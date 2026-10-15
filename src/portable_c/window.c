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
   or 7? Returns true if it can. `screen` gets how the screen is set, the
   mode without its highest bit where there are 80 columns: the code
   that starts the program goes on with the mode, the columns and the
   page. */
bool ChkMode(struct Screen *screen)
{
    bool ok = false;

    *screen = ScreenState();
    if (screen->columns == 80) {
        screen->mode &= 0x7F;
        ok = screen->mode == 2 || screen->mode == 3 || screen->mode == 7;
    }
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
    Cursor((u16)(md.DosLin << 8
                 | (u8)(md.DosCol - md.OutCol + md.BegCol)));
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

/* Move what a window of the screen shows a line up, or down if `down`,
   with the mouse pointer hidden, which would move along with the
   rest: `upper` is the upper left corner of the window and `lower` the
   lower right one, and the line that comes in is empty, of the colour
   `color`.

   Int10m of the original, which is the BIOS called for the screen with
   any registers, and called for nothing but this. The assembly goes on
   with CX and DX as the BIOS leaves them, the corners, which is as they
   went: a caller goes on with its own. */
void Scroll(bool down, u16 color, u16 upper, u16 lower)
{
    CurMous(0);
    ScreenScroll(down, 1, color, upper, lower);
    CurMous(1);
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

/* Keep what the screen buffer has in a window, at `buffer`, which is a
   buffer of the main data: `begin` is where the window begins, the row in the high byte,
   and `size` its rows and columns likewise. A window of no rows has
   65536 of them. A box names a buffer by its place in the main data,
   which may be odd, MenuBuf+500 for the copy's: a cell is kept as its
   two bytes, by PUT16, and put back by GET16. */
void SavWin(u16 begin, u16 size, void *buffer)
{
    const u16 *row = BasAdr(begin);
    u8 *to = buffer;
    u16 rows = size >> 8;
    i32 columns = size & 0xFF, column;

    do {
        for (column = 0; column != columns; column++) {
            PUT16(to, row[column]);
            to += 2;
        }
        row += 80;
    } while (--rows);
}

/* Put back what SavWin kept, and have the next character typed taken as
   it is no longer. */
void ResWin(u16 begin, u16 size, const void *buffer)
{
    u16 *row = BasAdr(begin);
    const u8 *from = buffer;
    u16 rows = size >> 8;
    i32 columns = size & 0xFF, column;

    do {
        for (column = 0; column != columns; column++) {
            row[column] = GET16(from);
            from += 2;
        }
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
    u16 rows = size >> 8;
    i32 columns = size & 0xFF, column;

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
static void MovScr(i32 cell, u16 *buffer, i32 rows, bool read)
{
    if (read)
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
    i32 cell = (i32)(from - md.ScrBuf);
    i32 rows = size >> 8;
    i32 columns = size & 0xFF;

    CurMous(0);
    if (rows != 1) {
        MovScr(cell - (begin & 0xFF), from - (begin & 0xFF), rows, false);
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
    MovScr(0, md.ScrBuf, md.Lines, true);
}
