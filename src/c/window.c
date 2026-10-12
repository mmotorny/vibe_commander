/*
 * The screen itself: its mode and its cursor, and the windows of the
 * screen buffer that are kept, put back and put on the screen.
 *
 * Translated from VCSUB1.INC.
 *
 * The program draws into ScrBuf, in the main data (see screen.c), and
 * Window copies a part of that to the memory of the display, which the
 * program reads once, when it starts and after every command: SavScr.
 * Where the display's memory is, Screen, was found out when the program
 * started. It is written through far pointers. The cursor and the mode
 * are the BIOS's: Int10 of dos.c, which takes the registers and hands
 * them back as the BIOS leaves them. They go from one call to the next
 * here as they did in the assembly.
 */
#include "vc.h"

#define BIOS_PAGE_OFFSET ((const u16 __far *)0x0040004EUL)  /* where in the
                                           display's memory the page that
                                           is shown begins, as the BIOS
                                           keeps it */
#define CGA_STATUS 0x3DA                /* the port whose lowest bit says
                                           that a CGA is not drawing */

/* What is at an offset in a segment. */
#define AT(segment, offset) \
    ((u16 __far *)((u32)(segment) << 16 | (u16)(offset)))

/* Can the program draw on the screen as it is: 80 columns, in mode 2, 3
   or 7? Returns 1 if it can. `mode` comes with BX as the caller has it
   and gets AX and BX as the BIOS leaves them, the mode without its
   highest bit where there are 80 columns: the code that starts the
   program goes on with the mode, the columns and the page. */
u16 ChkMode(struct VideoMode STACK *mode)
{
    struct BiosRegs regs;
    u16 ok = 0;

    regs.ax = 0x0F00;
    regs.bx = mode->bx;
    regs.cx = 0;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
    if (regs.ax >> 8 == 80) {
        regs.ax &= 0xFF7F;
        ok = (regs.ax & 0xFF) == 2 || (regs.ax & 0xFF) == 3
             || (regs.ax & 0xFF) == 7;
    }
    mode->ax = regs.ax;
    mode->bx = regs.bx;
    return ok;
}

/* Put the cursor at a column and a row of the page that is shown: the
   row is the high byte of `place`. The BIOS is asked for the page
   first. */
void Cursor(u16 place)
{
    struct BiosRegs regs;

    regs.ax = 0x0F00 | place & 0xFF;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = place;
    Int10((struct BiosRegs STACK *)&regs);
    regs.ax = 0x0200 | regs.ax & 0xFF;
    Int10((struct BiosRegs STACK *)&regs);
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
    struct BiosRegs regs;
    u16 was;

    regs.ax = 0x0300;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
    was = regs.cx;
    regs.cx = md.ScrMod == 7 ? 0x0B0C : 0x0607;
    if (md.ColTyp == 2)
        regs.cx &= 0xFF;
    regs.ax = 0x0100 | regs.ax & 0xFF;
    Int10((struct BiosRegs STACK *)&regs);
    return was;
}

/* Give the cursor the shape `shape`, as the BIOS takes it. */
void CurShp(u16 shape)
{
    struct BiosRegs regs;

    regs.ax = 0x0100;
    regs.bx = 0;
    regs.cx = shape;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
}

/* Where the cursor is on the page that is shown, the row in the high
   byte. */
u16 GetCur(void)
{
    struct BiosRegs regs;

    regs.ax = 0x0F00;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
    regs.ax = 0x0300 | regs.ax & 0xFF;
    Int10((struct BiosRegs STACK *)&regs);
    return regs.dx;
}

/* Put the cursor where it does not show: in row 127. */
void HidCur(void)
{
    Cursor(0x7F00);
}

/* Call the BIOS for the screen with the mouse pointer hidden, which it
   would move along with the rest: the program does so to scroll a
   window. Returns DX in the low word and CX in the high one, as the
   BIOS leaves them. */
u32 Int10m(u16 ax, u16 bx, u16 cx, u16 dx)
{
    struct BiosRegs regs;

    regs.ax = ax;
    regs.bx = bx;
    regs.cx = cx;
    regs.dx = dx;
    CurMous(0);
    Int10((struct BiosRegs STACK *)&regs);
    CurMous(1);
    return (u32)regs.cx << 16 | regs.dx;
}

/* Note where in the display's memory the page that is shown begins: at
   0 for the first page, and for another where the BIOS says. Returns
   that. */
u16 ScrOffs(void)
{
    struct BiosRegs regs;
    u16 offset = 0;

    regs.ax = 0x0F00;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    Int10((struct BiosRegs STACK *)&regs);
    if (regs.bx >> 8 != 0)
        offset = *BIOS_PAGE_OFFSET;
    *(u16 *)&md.Screen = offset;
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

/* Copy `rows` rows of 80 cells, a number of one byte, seven rows at a
   time. On a display that shows snow when its memory is written while
   it draws, which is one for which `snow` is 0 in its low byte, each
   seven are copied when the display has begun not to draw for long
   enough, with the machine kept from interrupting: after the line it
   was at, ten looks in a row have to find it not drawing, which is so
   only between two screens. */
static void MovScr(u16 __far *to, const u16 __far *from, u16 rows, u16 snow)
{
    u16 some = 7, count, looks;

    do {
        if (rows < some)
            some = rows;
        count = some * 80;
        if ((snow & 0xFF) == 0) {
            while (InPort(CGA_STATUS) & 1)
                ;
            do {
                IntOn();
                looks = 10;
                IntOff();
                while (InPort(CGA_STATUS) & 1)
                    if (--looks == 0)
                        break;
            } while (looks != 0);
        }
        for (; count != 0; count--)
            *to++ = *from++;
        IntOn();
        rows -= some;
    } while (rows != 0);
}

/* Put on the screen what the screen buffer has in a window: where it
   begins, the row in the high byte, and its rows and columns likewise.
   A window of one row is copied cell by cell, on a display that shows
   snow each cell when the display has just finished a line; one of more
   rows as whole rows of the screen, whatever its columns are. The mouse
   pointer is hidden meanwhile. Under TopView or DESQview the copy goes
   into their buffer, and they are told what has changed.

   They are told so with ES on the main data, where the call wants the
   segment of the buffer. Ported as it is. */
void Window(u16 begin, u16 size)
{
    const u16 *from = BasAdr(begin);
    u16 offset = (u16)((const u8 *)from - (const u8 *)md.ScrBuf)
                 + (u16)md.Screen;
    u16 segment = (u16)(md.Screen >> 16);
    u16 rows = size >> 8;
    u16 __far *to;
    u16 count, left;

    CurMous(0);
    if (rows != 1) {
        from -= begin & 0xFF;
        offset -= (begin & 0xFF) << 1;
        MovScr(AT(segment, offset), from, rows, md.Snow);
        count = rows * 80;
    } else {
        count = size & 0xFF;
        to = AT(segment, offset);
        left = count;
        do {
            if (md.Snow == 0) {
                while (InPort(CGA_STATUS) & 1)
                    ;
                IntOff();
                while ((InPort(CGA_STATUS) & 1) == 0)
                    ;
            }
            *to++ = *from++;
            IntOn();
        } while (--left);
    }
    if (md.TopView != 0)
        TopView(AT(MAIN_SEG, offset), count);
    CurMous(1);
}

/* Read the screen into the screen buffer, all its lines. */
void SavScr(void)
{
    MovScr((u16 __far *)md.ScrBuf, (const u16 __far *)md.Screen, md.Lines,
           md.Snow);
}
