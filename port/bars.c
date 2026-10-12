/*
 * The lines of the screen that are not a panel's: the key bar at the
 * bottom, the menu bar at the top, and the command line with its prompt.
 *
 * Translated from VCSUB2.INC.
 *
 * All of them are drawn into the screen buffer and put on the screen from
 * there, a whole row at a time.
 */
#include "vc.h"

#define KeyNu_C 5                       /* the number of a key in the bar */
#define KeyTi_C 6                       /* what the key does */
#define Menu_C  7                       /* the menu bar */
#define MnCur_C 8                       /* the menu that is open */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   Int16 calls the BIOS for the keyboard, with `ax` as INT 16h takes it,
   and returns AX as it comes back. */

/* Which names the key bar shows: for each of the bars that the program
   has, and for none of Shift, Ctrl and Alt held, Shift, Ctrl and Alt, a
   row of KeyC_T. A bar that is not here has the row of spaces. */
static const u8 CODE KeyB_T[15][4] = {
    { 1,  2,  3,  4},                   /*  0 */
    { 7,  0,  0,  0},                   /*  1 */
    { 8,  0,  0,  0},                   /*  2 */
    { 9, 12,  0,  0},                   /*  3 */
    {10, 12,  0,  0},                   /*  4 */
    {11, 12,  0,  0},                   /*  5 */
    {13, 16,  0,  0},                   /*  6 */
    {14, 16,  0,  0},                   /*  7 */
    { 6,  0,  0, 17},                   /*  8 */
    { 1,  2,  3,  5},                   /*  9 */
    {15, 16,  0,  0},                   /* 10 */
    { 1,  1,  1,  1},                   /* 11 */
    { 2,  2,  2,  2},                   /* 12 */
    { 4,  4,  4,  4},                   /* 13 */
    {18,  0,  0,  0},                   /* 14 */
};

/* The names of the ten keys in each row, as numbers of Key_S. */
static const u8 CODE KeyC_T[19][10] = {
    { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0},   /*  0 */
    { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10},   /*  1 */
    { 0, 31,  3,  4,  5,  6,  7,  8, 11, 12},   /*  2 */
    {13, 14,  0, 32,  5,  6,  0,  8,  0,  0},   /*  3 */
    {13, 14, 15, 16, 30, 17, 18, 19, 20, 21},   /*  4 */
    {13, 14, 15, 16, 30, 17, 18, 19,  0, 21},   /*  5 */
    { 1,  0,  0,  0,  0,  0,  0,  0,  0, 21},   /*  6 */
    { 1,  0,  0,  0,  0,  0,  0,  0,  0, 10},   /*  7 */
    { 0,  0,  0,  0,  0,  0,  0,  0,  0, 10},   /*  8 */
    { 1,  0, 10, 22,  0,  0, 26,  0,  0, 10},   /*  9 */
    { 1, 24, 10, 23,  0,  0, 26,  0,  0, 10},   /* 10 */
    { 1, 25, 10, 23,  0,  0, 26,  0,  0, 10},   /* 11 */
    { 0,  0,  0,  0,  0,  0, 26,  0,  0,  0},   /* 12 */
    { 1, 27,  0, 22,  0,  0, 26,  0,  0, 10},   /* 13 */
    { 1, 27,  0, 23,  0,  0, 26,  0,  0, 10},   /* 14 */
    { 1, 27,  0,  0,  0,  0, 26,  0,  0, 10},   /* 15 */
    { 0, 28,  0,  0,  0,  0, 26,  0,  0, 29},   /* 16 */
    { 0,  0,  0,  0,  0,  0,  0,  0,  0, 21},   /* 17 */
    { 1, 33,  0,  0,  0,  0,  0,  0,  0, 10},   /* 18 */
};

/* What a key does, in the six cells that the bar has for it. */
static const u8 CODE Key_S[34][6] = {
    {' ', ' ', ' ', ' ', ' ', ' '},     /*  0 */
    {'H', 'e', 'l', 'p', ' ', ' '},     /*  1 */
    {'M', 'e', 'n', 'u', ' ', ' '},     /*  2 */
    {'V', 'i', 'e', 'w', ' ', ' '},     /*  3 */
    {'E', 'd', 'i', 't', ' ', ' '},     /*  4 */
    {'C', 'o', 'p', 'y', ' ', ' '},     /*  5 */
    {'R', 'e', 'n', 'M', 'o', 'v'},     /*  6 */
    {'M', 'k', 'd', 'i', 'r', ' '},     /*  7 */
    {'D', 'e', 'l', 'e', 't', 'e'},     /*  8 */
    {'P', 'u', 'l', 'l', 'D', 'n'},     /*  9 */
    {'Q', 'u', 'i', 't', ' ', ' '},     /* 10 */
    {'S', 'a', 'v', 'S', 'e', 't'},     /* 11 */
    {'P', 'u', 'l', 'l', 'D', 'n'},     /* 12 */
    {'L', 'e', 'f', 't', ' ', ' '},     /* 13 */
    {'R', 'i', 'g', 'h', 't', ' '},     /* 14 */
    {'V', 'i', 'e', 'w', '.', '.'},     /* 15 */
    {'E', 'd', 'i', 't', '.', '.'},     /* 16 */
    {'D', 'i', 'r', 'S', 'i', 'z'},     /* 17 */
    {'F', 'i', 'n', 'd', ' ', ' '},     /* 18 */
    {'H', 'i', 's', 't', 'r', 'y'},     /* 19 */
    {'E', 'G', 'A', ' ', 'L', 'n'},     /* 20 */
    {'T', 'r', 'e', 'e', ' ', ' '},     /* 21 */
    {'A', 'S', 'C', 'I', 'I', ' '},     /* 22 */
    {'H', 'e', 'x', ' ', ' ', ' '},     /* 23 */
    {'W', 'r', 'a', 'p', ' ', ' '},     /* 24 */
    {'U', 'n', 'w', 'r', 'a', 'p'},     /* 25 */
    {'S', 'e', 'a', 'r', 'c', 'h'},     /* 26 */
    {'S', 'a', 'v', 'e', ' ', ' '},     /* 27 */
    {'S', 'a', 'v', 'e', '.', '.'},     /* 28 */
    {'Q', '&', 'S', 'a', 'v', 'e'},     /* 29 */
    {'M', 'e', 'm', 'o', 'r', 'y'},     /* 30 */
    {'M', 'a', 'i', 'n', ' ', ' '},     /* 31 */
    {'L', 'a', 'b', 'e', 'l', ' '},     /* 32 */
    {'R', 'e', 's', 'c', 'a', 'n'},     /* 33 */
};

/* The menu bar: the names of its five menus, each behind four spaces, and
   spaces to the end of the row. It is a constant of port/bars_t.c, which
   the menu reads too. */
extern const u8 __far PtMnu_S[];
#pragma aux PtMnu_S "*"
/* Its colour, as NgLine takes the colours of a line. The line switches to
   no other, so the one is all that is read. */
static const u8 CODE PtMnu_B[] = { Menu_C };

/* Fill a row of the screen buffer with spaces in the colour that DOS
   writes in, and return the address of the row below: `place` has the row
   in its high byte. The colour is read at CrntCol in the segment
   `segment`, which is that of the main data, MAIN_SEG, for every caller
   but one: Run_00 of port/usermenu.c. */
u16 *ClsLine(u16 place, u16 segment)
{
    u16 *screen = BasAdr(place & 0xFF00);
    u16 cell = *(const u8 __far *)((u32)segment << 16
                                   | (u16)(u8 *)&md.CrntCol) << 8 | ' ';
    u16 count;

    for (count = 80; count != 0; count--)
        *screen++ = cell;
    return screen;
}

/* Draw the key bar `bar` as it is with none of Shift, Ctrl and Alt held,
   0, or with Shift, Ctrl or Alt, 1, 2 and 3 for `shift`: the numbers of
   the ten keys and what each does. Nothing is drawn if the bar shows that
   already. Only the low bytes of both count. */
void PutKey0(u16 bar, u16 shift)
{
    u16 index = ((bar << 2 & 0xFF) | shift) & 0xFF;
    u16 row = 0;
    u16 place, title, number, key, name, i;
    u16 *screen;

    if (index < sizeof KeyB_T)
        row = KeyB_T[index >> 2][index & 3];
    if (row == md.BarPrev)
        return;
    md.BarPrev = (u8)row;
    place = (u16)(u8)(md.Lines - 1) << 8;
    screen = BasAdr(place);
    title = Color(KeyTi_C) << 8;
    number = Color(KeyNu_C) << 8;
    for (key = 0; key < 10; key++) {
        if (key == 9) {
            *screen++ = number | '1';
            *screen++ = number | '0';
        } else {
            *screen++ = number | ('1' + key);
        }
        name = KeyC_T[row][key];
        for (i = 0; i < 6; i++)
            *screen++ = title | Key_S[name][i];
        if (key != 9)
            *screen++ = number | ' ';
    }
    Window(place, 0x150);
}

/* Draw the key bar `bar` for the keys among Shift, Ctrl and Alt that are
   held, if the setup has a key bar. Without one, the next bar that is
   asked for is drawn whatever was there. */
void PutKey(u16 bar)
{
    u16 keys;

    if (md.KeyBar == 0) {
        md.BarPrev = 0xFF;
        return;
    }
    keys = (u16)Int16(0x200);
    PutKey0(bar, keys & 8 ? 3 : keys & 4 ? 2 : keys & 3 ? 1 : 0);
}

/* Draw the menu bar with the menu `item` marked as open: 1 to 5, anything
   else for none. Only the low byte of `item` counts. */
void PtMnu1(u16 item)
{
    struct LineArgs line;
    u16 i = 0;
    u16 column = 0xFE;                  /* two cells before the name */
    u16 width = 3;                      /* and as many behind it */
    u8 c;

    item &= 0xFF;
    line.value = 80;
    line.screen = BasAdr(0);
    line.line = PtMnu_S;
    line.colors = PtMnu_B;
    /* The line has no ^I, so the include line is not read: the assembly
       left there whatever its caller had in DX. */
    line.include = PtMnu_S;
    NgLine((struct LineArgs STACK *)&line);
    if (item != 0 && item <= 5) {
        /* The column is counted in a byte, as the assembly counts it. */
        do {
            do {
                c = PtMnu_S[i++];
                column++;
            } while (c != 0xFF);
            column += PtMnu_S[i++] - 1;
        } while (--item);
        do {
            c = PtMnu_S[i++];
            width++;
        } while (c != 0xFF);
        MvsColr(BasAdr(column & 0xFF), width, Color(MnCur_C));
    }
    Window(0, 0x150);
}

/* Draw the menu bar if the setup has it always visible. */
void PutMnu(u16 item)
{
    if (md.MenuVs != 0)
        PtMnu1(item);
}

/* Draw the command line from where it begins on the screen, as much of
   the command as there is room for, with the cursor's column among it:
   the line moves so that the cursor has five characters before it, where
   the command has as many, and stays on the screen. */
void DosCom(void)
{
    u16 cursor = md.DosCol;
    u16 first = 0;
    u16 width = (u8)(79 - md.BegCol);
    u16 color = md.CrntCol << 8;
    u16 count;
    const u8 *text;
    u16 *screen;
    u8 c;

    if (cursor >= 5) {
        first = md.OutCol;
        if (cursor - 5 < first)
            first = cursor - 5;
    }
    if (cursor >= width && cursor - width > first)
        first = cursor - width;
    md.OutCol = (u8)first;
    text = md.DosBuf + first;
    screen = BasAdr(md.DosLin << 8 | md.BegCol);
    for (count = width + 1; count != 0; count--) {
        c = *text;
        if (c == 0)
            c = ' ';
        else
            text++;
        *screen++ = color | c;
    }
    Window(md.DosLin << 8, 0x150);
}

/* Draw the prompt of the command line, the current directory or, if the
   setup has no path in it, the drive alone, and the command behind it.
   The line is the one that DOS has its cursor in, or the one below the
   panel `panel` if that is further down and the active panel is on. */
void PutPrm(u16 panel)
{
    u16 below = (BegBX(panel) >> 8) + (SizDX() >> 8) & 0xFF;
    u16 line = md.DosLin;
    u16 color = md.CrntCol << 8;
    u16 column = 1;
    const u8 *path = md.DosPth;
    u16 *screen;
    u8 c;

    if (line < below && PANEL(md.WCBcrn)->Visible != 0) {
        line = below;
        md.DosLin = (u8)line;
    }
    screen = BasAdr(line << 8);
    *screen++ = color | *path++;
    if (md.Prompt != 0)
        while ((c = *path++) != 0) {
            *screen++ = color | c;
            column++;
        }
    if (md.PathEr != 0) {
        *screen++ = color | '?';
        column++;
    }
    *screen++ = color | '>';
    column++;
    md.BegCol = (u8)column;
    DosCom();
}
