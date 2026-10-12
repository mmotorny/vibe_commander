/*
 * The command line at the panels: what a key does to it, Esc, the names
 * of the selected files and the paths of the panels put into it, the
 * history of commands in its box, and the speed search, which is typed
 * where the command line is.
 *
 * Translated from VCFUNC.INC.
 *
 * The command line is DosBuf, a text of up to 123 characters, with the
 * column of its cursor in DosCol. The history is HstBuf: the commands
 * that were run, each with its zero byte, the latest first, behind a zero
 * byte that stands for the empty line; HstAdr is where in it Ctrl+E and
 * Ctrl+X have got to.
 */
#include "vc.h"

/* The commands of this file are declared in vc.h: see "The commands"
   there. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte, and Cursor1 at its place in the command line.
   PutMnu draws the menu bar, if that is always shown, with an item of it
   marked, and PutKey the key bar. SavWin keeps what the screen buffer has
   where a window is to be, ResWin puts it back; MinBox draws a box with a
   shadow there. SftEGA moves a place down on a screen of more than 25
   lines, by half of what it has more.

   GetMous returns the buttons of the mouse that are down. Input waits for
   a key, with the key bar `bar` on the screen, and shows the help for F1;
   the high byte of what it returns is FCh for a button of the mouse.

   EdLin has a key act on a line that is typed into, `length` characters
   at `buffer` in the main data with the cursor in `column`: it returns
   the column, and in the high word something that is not 0 if the line
   or the column has changed. A key without a scan code is a character,
   whatever it is. DosCom draws the command line.

   PutMnu, PutKey, Input, EdLin, Search and DosCom are C, of vc.h. */

/* C routines of other files that vc.h does not have. */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
u32 TotSel(u16 panel);                                  /* files.c */
u16 F32_70(u16 begin, u16 size);                        /* usermenu.c */
#pragma aux MvsBw  "*_c" __far parm [es di] [dx si] [cx] [ax] value [es di] modify exact [di es]
#pragma aux TotSel "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux F32_70 "*_c" __far parm [bx] [dx] value [ax] modify exact [ax]

/* The mask that stands behind the name of a directory in the command
   line: the assembly's, which other C code has too. */
extern const u8 __far Mask_S[];         /* "\*.*" */
#pragma aux Mask_S "*"

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define LenCmd 124                      /* Length of command line */

#define Box_C   16                      /* the colour of a box that reports */
#define BoxCu_C 18                      /* and of what is typed into it */
#define Hist_C  23                      /* the colour of the history */
#define HstCu_C 24                      /* and of the line of its cursor */

static const u8 CODE Hist_S[9] = " History ";

/* The box of the speed search: its two colours, with the two bytes that
   follow them in the assembly, and its line. */
static const u8 CODE AltSr_P[] = { Box_C, BoxCu_C, 0x78, 0x79 };
static const u8 CODE AltSr_S[] = "Search: ^r\xFF\x0C^r";

/* The scan codes of Alt with a digit, a hyphen, an equals sign or a
   letter, as the assembly has them too, where the main loop reads
   them, and what each stands for in the speed search. */
static const u8 CODE AltSr_T[38] = {
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32
};
static const u8 CODE Ascii_T[38] = "1234567890-=qwertyuiopasdfghjklzxcvbnm";

/* Where the speed search keeps its text in MenuBuf, and in the word
   before it the column of its cursor. */
#define SPEED 230

/* Have a key act on the command line, and draw the line if that has
   changed it. */
void F73_00(u16 key)
{
    u32 pair = EdLin(key, md.DosBuf, md.DosCol, LenCmd - 1);

    if (HIGH(pair)) {
        md.DosCol = (u8)pair;
        DosCom();
    }
}

/* Esc: empty the command line. Returns 1 instead if there is nothing in
   it and Esc is to switch the panels off and on. */
u16 F74_00(void)
{
    if (md.DosBuf[0] == 0 && md.EscBar != 0)
        return 1;
    F73_00(0x1519);                     /* Ctrl+Y */
    return 0;
}

/* Ctrl+I: put the names of the selected files of the panel `panel` into
   the command line, where its cursor is, or the name of the file under
   the cursor of the panel, each with a space behind it, and "\*.*"
   behind the name of a directory; as many as there is room for. What
   was selected is not afterwards, and is marked as having been. Returns
   0 if the panel shows no files or has none to put there. */
u16 F80_00(u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    struct FILENT __far *file;
    u32 selected;
    u16 any, at, length, used, column, i, c;

    if (!FilePan(panel))
        return 0;
    selected = TotSel(panel);
    any = HIGH(selected) | LOW(selected);
    at = wcb->CurAdr;
    if (any == 0) {
        if (at >= wcb->EndBuf)
            return 0;
        if (ENTRY(wcb, at)->NameF[0] == '.')
            return 0;
    }
    for (i = DSKBUF; i < wcb->EndBuf; i += sizeof(struct FILENT))
        ENTRY(wcb, i)->AttrF &= 0x7F;
    if (any == 0)
        goto F80_05;
    at = DSKBUF;
F80_04:
    if (at >= wcb->EndBuf)
        goto F80_10;
    if (!(ENTRY(wcb, at)->AttrF & ATTR_SELECTED))
        goto F80_12;
F80_05:                                 /* is there room for it */
    file = ENTRY(wcb, at);
    for (length = 0; file->NameF[length] != 0; length++)
        ;
    length++;
    for (used = 0; md.DosBuf[used] != 0; used++)
        ;
    used++;
    if (file->AttrF & ATTR_DIRECTORY)
        length += 4;
    if (length > (u16)(LenCmd - used))
        goto F80_10;
    column = md.DosCol;
    for (i = 0; (c = file->NameF[i]) != 0; i++)
        column = LOW(EdLin(c, md.DosBuf, column, LenCmd - 1));
    if (file->AttrF & ATTR_DIRECTORY)
        for (i = 0; (c = Mask_S[i]) != 0; i++)
            column = LOW(EdLin(c, md.DosBuf, column, LenCmd - 1));
    column = LOW(EdLin(' ', md.DosBuf, column, LenCmd - 1));
    md.DosCol = (u8)column;
    if (any == 0)
        goto F80_10;
    file->AttrF &= 0xBF;
    file->AttrF |= 0x80;
F80_12:
    at += sizeof(struct FILENT);
    goto F80_04;

F80_10:
    DosCom();
    return 1;
}

/* Type a text into the command line, from the column `column` on, each
   character as a key with the scan code in the high byte of `scan`, until
   one is not taken; and behind it the character `end`, if that is not 0
   and the text does not end in it. Draws the line and returns the column
   of the cursor. */
static u16 F68_25(const u8 __far *text, u16 column, u16 scan, u16 end)
{
    u32 pair;
    u16 c;

    for (;;) {
        c = *text;
        if (c == 0)
            goto F68_90;
        text++;
        pair = EdLin(scan | c, md.DosBuf, column, LenCmd - 1);
        column = LOW(pair);
        if (!HIGH(pair))
            goto F68_27;
    }
F68_90:
    if ((end & 0xFF) != 0 && (end & 0xFF) != text[-1])
        column = LOW(EdLin(end, md.DosBuf, column, LenCmd - 1));
F68_27:
    md.DosCol = (u8)column;
    DosCom();
    Cursor1();
    return column;
}

/* Ctrl+[ and Ctrl+]: put the path of the panel `panel` into the command
   line, with a backslash behind it, or the directory that the cursor of
   the tree is on. `key` is the key, for its scan code. */
void F82_01(u16 panel, u16 key)
{
    struct WCBdef __far *wcb = PANEL(panel);
    const u8 __far *path = wcb->WinPath;

    if (wcb->WinTyp > 1)
        return;
    if (wcb->WinTyp == 1)
        path = md.CrnPath;
    F68_25(path, md.DosCol, key & 0xFF00, '\\');
}

/* Give the row `row` of the screen, where a line of the history is, the
   colour whose number is `color`. */
static void F68_30(u16 color, u16 begin, u16 row, u16 size)
{
    u16 place = (row & 0xFF) << 8 | (u8)(begin + 4);
    u16 width = (u8)(size - 10);

    MvsColr(BasAdr(place), width, Color(color));
    Window(place, 0x100 | width);
}

/* Alt+F8: the history in a box, to choose a command from. Enter puts the
   command into the command line and Ctrl+Enter does only that; a letter
   goes to the next command that begins with it. Returns 1 if the command
   is to be run. */
u16 F68_00(void)
{
    const u8 *text;                     /* DI, then SI */
    u16 *screen;
    u16 begin, size;                    /* BX and DX: the box */
    u16 row;                            /* CL: the row of the cursor */
    u16 limit, length, color, width, lines, room, at, i, c, key, to;
    u16 half, answer, count;

    HidCur();
    PutMnu(3);
    size = 0x51F;
    text = md.HstBuf;
    limit = (u8)(md.Lines - 3);
F68_01:                                 /* how many and how wide */
    length = 0;
    do
        if (text[length++] == 0)
            break;
    while (length < 256);
    text += length;
    if ((u8)length > (u8)size) {
        size = size & 0xFF00 | (u8)length;
        if ((u8)size > 65 + 1)
            size = size & 0xFF00 | 65 + 1;
    }
    size += 0x100;
    if ((u8)(size >> 8) >= limit)
        goto F68_03;
    if (*text != 0)
        goto F68_01;
F68_03:
    size = ADD_LOW(size, 10 + 2 - 1);
    half = size + 0x100 - 1;
    begin = 0x0D28 - ((half >> 8) >> 1 << 8 | (half & 0xFF) >> 1);
    begin = SftEGA(begin);
    SavWin(begin, size, (u16)md.WinBuf);
    color = Color(Hist_C) << 8;
    MinBox(begin, size, color);
    MvsBw(BasAdr((begin + 0x100 & 0xFF00) | 35), Hist_S, 9, color);

    /* The commands, the latest at the bottom, above the empty line. */
    screen = BasAdr((u8)((begin >> 8) + (size >> 8) - 4) << 8
                    | (u8)(begin + 5));
    text = md.HstBuf;
    width = (u8)(size - 12);
    lines = (u8)((size >> 8) - 5);
    at = 0;
F68_04:
    if ((u16)text <= md.HstAdr)
        at++;
    room = width;
    i = 0;
    for (;;) {
        c = *text++;
        if (c == 0)
            break;
        if (room == 0)
            continue;
        screen[i++] = color | c;
        room--;
    }
    screen -= 80;
    if (--lines != 0)
        goto F68_04;
    Window(begin, size);
    md.HlpPage = 33;
    row = (u8)((begin >> 8) + (size >> 8) - 3 - at);
    md.MousTim = md.MousTim & 0xFFFF | 0xFFFF0000UL;
    md.MousEnt = 0;

F68_07:                                 /* the cursor */
    F68_30(HstCu_C, begin, row, size);
F68_08:
    if (LOW(GetMous(0)))
        goto F68_11;
    md.MousEnt = 0;
    key = Input(1);
    if (key >> 8 == 0xFC)
        goto F68_11;
    switch (key) {
    case 0x4800:
        goto F68_13;
    case 0x5000:
        goto F68_15;
    case 0x4700:
    case 0x4900:
        goto F68_16;
    case 0x4F00:
    case 0x5100:
        goto F68_14;
    case 0x1C0A:
    case 0x1C0D:
    case 0x4400:
        goto F68_20;
    }
    if ((key & 0xFF) == 0x1B && key >> 8 != 0)
        goto F68_20;
    /* A letter: the next command that begins with it, from the cursor
       down and round, but for the empty line. */
    c = UpCase(key) & 0xFF;
    i = row;
F68_09:
    i = (u8)(i + 1);
    if (i < (u8)((begin >> 8) + (size >> 8) - 4))
        goto F68_10;
    if (i == row)
        goto F68_08;
    i = (u8)((begin >> 8) + 2);
F68_10:
    if (i == row)
        goto F68_08;
    screen = BasAdr(i << 8 | (u8)(begin + 5));
    if ((UpCase(c << 8 | *(u8 *)screen) & 0xFF) != c)
        goto F68_09;
    to = i;
    goto F68_12;

F68_11:                                 /* the mouse */
    answer = F32_70(begin, size);
    if (answer >> 8 < 1)
        goto F68_08;
    if (answer >> 8 > 1) {
        key = answer & 0xFF00 | 0x0D;
        goto F68_20;
    }
    to = (u8)(answer + (begin >> 8) + 2);
F68_12:                                 /* the cursor goes elsewhere */
    if (to == row)
        goto F68_08;
    F68_30(Hist_C, begin, row, size);
    row = to;
    goto F68_07;

F68_13:                                 /* Up */
    to = (u8)(row - 1);
    if (to != (u8)((begin >> 8) + 1))
        goto F68_12;
F68_14:                                 /* End */
    to = (u8)((begin >> 8) + (size >> 8) - 4);
    goto F68_12;
F68_15:                                 /* Down */
    to = (u8)(row + 1);
    if (to != (u8)((begin >> 8) + (size >> 8) - 3))
        goto F68_12;
F68_16:                                 /* Home */
    to = (u8)((begin >> 8) + 2);
    goto F68_12;

F68_20:                                 /* the box goes */
    ResWin(begin, size, (u16)md.WinBuf);
    Window(begin, size);
    if (key >> 8 != 0x1C)
        return 0;
    count = (u16)(signed char)(u8)((begin >> 8) + (size >> 8) - 4 - row);
    text = md.HstBuf;
    for (; count != 0; count--) {
        length = 0;
        do
            if (text[length++] == 0)
                break;
        while (length < 256);
        text += length;
    }
    md.DosBuf[0] = 0;
    c = F68_25(text, 0, 0, 0);
    if ((key & 0xFF) != 0x0D)
        return 0;
    if ((c & 0xFF) == 0)
        return 0;
    return 1;
}

/* Have a key of the speed search act on what has been typed, look for
   the next name that begins with that, among the files of the panel
   `panel` or in the tree, and put the cursor there: F77_10. */
static void F77_10(u16 key, u16 panel)
{
    struct WCBdef __far *wcb = PANEL(panel);
    u16 segment, at, first, begin, end, step, found;

    if (wcb->WinTyp == 0) {
        segment = panel;
        at = wcb->CurAdr;
        first = wcb->First;
        end = wcb->EndBuf;
        begin = DSKBUF;
        step = sizeof(struct FILENT);
    } else {
        segment = MAIN_SEG;
        at = md.TreAdr;
        first = md.TreFrst;
        begin = TRELIST;
        end = (((struct DIRTREE *)md.TreeBuf)->TreeLen << 4) + begin;
        step = sizeof(struct DIRENT);
    }
    found = Search(segment, at, begin, end, step, key);
    if (found != at)
        F00_50(panel, found, first);
}

/* The speed search of a panel, which `key`, Alt with a letter or a digit,
   has begun: a box in the bottom line of the panel `panel`, where what
   is typed is shown, while the cursor of the panel goes to the next name
   that begins with it. A key that is no part of that ends it and is
   returned, to be acted on, 0 for Esc; the high word is where the box
   was. */
u32 F77_00(u16 key, u16 panel)
{
    struct LineArgs line;
    u16 *screen;
    u16 begin, size, color, bar, i;

    PutKey(11);
    begin = BegBX(panel);
    size = SizDX();
    begin = (u8)((begin >> 8) + (size >> 8) - 1) << 8 | (u8)(begin + 8);
    size = 0x318;
    SavWin(begin, size, (u16)md.WinBuf);
    screen = BasAdr(begin);
    color = Color(AltSr_P[0]) << 8;
    BoxDubl(screen, color | 11, 1, 0, 0x16);
    line.value = color | 50;
    line.screen = screen + 82;
    line.line = AltSr_S;
    line.colors = AltSr_P;
    line.include = AltSr_S;
    NgLine((struct LineArgs STACK *)&line);
    Window(begin, size);
    *(u16 *)(md.MenuBuf + SPEED - 2) = 0;
    md.MenuBuf[SPEED] = 0;
    bar = md.KeyBar;
    md.KeyBar = 0;
    goto F77_02;

F77_01:                                 /* what has been typed, and a key */
    InSt10(begin + 0x10A, (u16)(md.MenuBuf + SPEED), 12,
           (u16)AltSr_P[1] << 8);
    i = *(u16 *)(md.MenuBuf + SPEED - 2) + begin + 0x10A;
    Cursor(i);
    key = Input(i);
F77_02:
    if (key >> 8 == 0xFC)
        goto F77_20;
    if (key >> 8 >= 0xF0)
        goto F77_01;
    if (key >> 8 != 0)
        switch (key & 0xFF) {
        case 0x13:                      /* ^S */
        case 0x04:                      /* ^D */
        case 0x01:                      /* ^A */
        case 0x06:                      /* ^F */
        case 0x08:                      /* BackSpace */
        case 0x07:                      /* ^G */
        case 0x0B:                      /* ^K */
        case 0x19:                      /* ^Y */
        case 0x17:                      /* ^W */
        case 0x7F:                      /* ^BackSpace */
        case 0x14:                      /* ^T */
        case 0x11:                      /* ^Q */
        case 0x0A:                      /* ^Enter */
            goto F77_11;
        case 0x00:
            if (key == 0x5300)          /* Del */
                goto F77_11;
            break;
        case 0x1B:                      /* Esc */
            key = 0;
            goto F77_20;
        }
    if ((key & 0xFF) != 0)
        goto F77_03;
    for (i = 0; i < sizeof AltSr_T; i++)
        if (AltSr_T[i] == key >> 8)
            break;
    if (i == sizeof AltSr_T)
        goto F77_20;
    key = key & 0xFF00 | Ascii_T[i];
    goto F77_11;
F77_03:
    if ((key & 0xFF) < ' ')
        goto F77_20;
F77_11:
    F77_10(key, panel);
    goto F77_01;

F77_20:                                 /* the box goes */
    md.KeyBar = (u8)bar;
    ResWin(begin, size, (u16)md.WinBuf);
    Window(begin, size);
    Cursor1();
    return (u32)begin << 16 | key;
}
