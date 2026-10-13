/*
 * The screen itself: its mode and its cursor, and the windows of the
 * screen buffer that are kept, put back and put on the screen.
 *
 * Translated from VCSUB1.INC.
 *
 * The program draws into ScrBuf, in the main data (see screen.c), and
 * Window copies a part of that to the cells of the screen, which the
 * program reads once, when it starts and after every command: SavScr.
 * The cells, the cursor and the mode are the system's, sys.h. In the
 * assembly the registers go from one call of the BIOS to the next: what
 * a call here takes of the one before, the page that is shown, it is
 * handed.
 */
#include "vc.h"

/* Can the program draw on the screen as it is: 80 columns, in mode 2, 3
   or 7? Returns 1 if it can. `mode` comes with BX as the caller has it
   and gets AX and BX as the BIOS leaves them, the mode without its
   highest bit where there are 80 columns: the code that starts the
   program goes on with the mode, the columns and the page. */
u16 ChkMode(struct VideoMode STACK *mode)
{
    struct Screen screen = ScreenState();
    u16 ok = 0;

    if (screen.columns == 80) {
        screen.mode &= 0x7F;
        ok = screen.mode == 2 || screen.mode == 3 || screen.mode == 7;
    }
    mode->ax = screen.columns << 8 | screen.mode;
    mode->bx = screen.page << 8 | mode->bx & 0xFF;
    return ok;
}

/* Put the cursor at a column and a row of the page that is shown: the
   row is the high byte of `place`. The BIOS is asked for the page
   first. */
void Cursor(u16 place)
{
    CursorTo(ScreenState().page, place);
}

/* Put the cursor where the command line has it. */
void Cursor1(void)
{
    Cursor((u16)md.DosLin << 8
           | (u8)(md.DosCol - md.OutCol + md.BegCol));
}

/* Give the cursor the shape it has in a line that is typed into: two
   lines at the bottom of the cell, of a colour display or of a
   monochrome one, and from the top of the cell down with the palette of
   a laptop. Returns the shape it had. */
u16 ChgCur(void)
{
    u16 was = CursorWhere(0).shape;
    u16 shape = md.ScrMod == 7 ? 0x0B0C : 0x0607;

    if (md.ColTyp == 2)
        shape &= 0xFF;
    CursorShape(shape);
    return was;
}

/* Give the cursor the shape `shape`, as the BIOS takes it. */
void CurShp(u16 shape)
{
    CursorShape(shape);
}

/* Where the cursor is on the page that is shown, the row in the high
   byte. */
u16 GetCur(void)
{
    return CursorWhere(ScreenState().page).place;
}

/* Put the cursor where it does not show: in row 127. */
void HidCur(void)
{
    Cursor(0x7F00);
}

/* Have the BIOS scroll a window with the mouse pointer hidden, which it
   would move along with the rest. The arguments are the registers of
   the BIOS's call still, for the callers' sake: the high byte of `ax`
   is 6 to scroll up and 7 to scroll down, and its low byte the lines;
   the high byte of `bx` is the colour of the lines that come in; `cx`
   is the upper left corner of the window and `dx` the lower right one.
   Returns the corners: the assembly goes on with CX and DX as the BIOS
   leaves them, which is as they went. */
struct Corners Int10m(u16 ax, u16 bx, u16 cx, u16 dx)
{
    struct Corners corners;

    CurMous(0);
    ScreenScroll(ax >> 8 == 7, ax, bx >> 8, cx, dx);
    CurMous(1);
    corners.lower = dx;
    corners.upper = cx;
    return corners;
}

/* Note where the cells of the screen are: in the memory of the display
   that the mode is of, from where the page begins that is shown, which
   is at 0 for the first page and for another where the BIOS says.
   Returns that.

   The original notes the page's place here and no more, in the low word
   of Screen, and its start-up puts the display's segment above it, B000h
   for the mode of one colour and B800h for the others. Both are the
   system's to know: ScreenMemory, which keeps the address in Screen
   still. */
u16 ScrOffs(void)
{
    u16 offset = 0;

    if (ScreenState().page != 0)
        offset = ScreenPageStart();
    ScreenMemory(md.ScrMod, offset);
    return offset;
}

/* A place half as many rows further down as the screen has more than
   25: for a box that is to be in the middle of a screen of 43 or 50
   lines too. */
u16 SftEGA(u16 place)
{
    if (md.Lines > 25)
        place += (u16)((u8)(md.Lines - 25) >> 1) << 8;
    return place;
}

/* Keep what the screen buffer has in a window, at `buffer` in the main
   data: `begin` is where the window begins, the row in the high byte,
   and `size` its rows and columns likewise. A window of no rows has
   65536 of them. */
void SavWin(u16 begin, u16 size, u16 buffer)
{
    const u16 *row = BasAdr(begin);
    u16 *to = (u16 *)buffer;
    u16 rows = size >> 8, columns = size & 0xFF, column;

    do {
        for (column = 0; column != columns; column++)
            *to++ = row[column];
        row += 80;
    } while (--rows);
}

/* Put back what SavWin kept, and have the next character typed taken as
   it is no longer. */
void ResWin(u16 begin, u16 size, u16 buffer)
{
    u16 *row = BasAdr(begin);
    const u16 *from = (const u16 *)buffer;
    u16 rows = size >> 8, columns = size & 0xFF, column;

    do {
        for (column = 0; column != columns; column++)
            row[column] = *from++;
        row += 80;
    } while (--rows);
    md.Quote = 0;
}

/* Put into a window of the screen buffer what DOS had on the screen
   there, which the program keeps in UserScr. */
void ResUser(u16 begin, u16 size)
{
    u16 *row = BasAdr(begin);
    const u16 *from = md.UserScr + (row - md.ScrBuf);
    u16 rows = size >> 8, columns = size & 0xFF, column;

    do {
        for (column = 0; column != columns; column++)
            row[column] = from[column];
        row += 80;
        from += 80;
    } while (--rows);
}

/* Copy `rows` rows of 80 cells, a number of one byte, between the screen
   and `buffer`: to the screen from its cell `cell` on, or from the
   screen if `read` is not 0. The machine may interrupt the program
   afterwards, whether it might before or not.

   The original copies seven rows at a time, and on a display that shows
   snow when its memory is written while it draws, a CGA, each seven
   when the display has begun not to draw for long enough, with the
   machine kept from interrupting. Not here: see DEVIATIONS.md. What is
   left of that is that the machine may interrupt again, which the
   original says after every seven rows whatever the display is. DOS
   calls the program for a critical error with the machine kept from
   interrupting, and the box that the program shows then is put on the
   screen through here. */
static void MovScr(u16 cell, u16 *buffer, u16 rows, u16 read)
{
    if (read != 0)
        ScreenGet(cell, buffer, rows * 80);
    else
        ScreenPut(cell, buffer, rows * 80);
    IntOn();
}

/* Put on the screen what the screen buffer has in a window: where it
   begins, the row in the high byte, and its rows and columns likewise.
   A window of one row is copied cell by cell, as many as it has columns,
   and one of no columns as 65536 cells; one of more rows as whole rows
   of the screen, whatever its columns are. The mouse pointer is hidden
   meanwhile, and the machine may interrupt the program afterwards, as
   after MovScr: see there. On a display that shows snow the original
   copies each cell of a row when the display has just finished a line.

   The original then tells TopView or DESQview what has changed, where
   one of them shows a buffer in place of the display's memory. Not
   here: see DEVIATIONS.md. */
void Window(u16 begin, u16 size)
{
    u16 *from = BasAdr(begin);
    u16 cell = (u16)(from - md.ScrBuf);
    u16 rows = size >> 8;
    u16 columns = size & 0xFF;

    CurMous(0);
    if (rows != 1) {
        MovScr(cell - (begin & 0xFF), from - (begin & 0xFF), rows, 0);
    } else {
        if (columns != 0)
            ScreenPut(cell, from, columns);
        else {
            /* 65536 cells, as the original's loop counts none: twice
               round the 64K that the display's memory and the main data
               each have, which is the same 32768 cells twice. */
            ScreenPut(cell, from, 0x8000);
            ScreenPut(cell, from, 0x8000);
        }
        IntOn();
    }
    CurMous(1);
}

/* Read the screen into the screen buffer, all its lines. */
void SavScr(void)
{
    MovScr(0, md.ScrBuf, md.Lines, 1);
}
