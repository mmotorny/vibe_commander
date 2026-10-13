/*
 * The colours of the setup: the list of the 39 colours of a palette, F5,
 * F6 and F7 of the setup's menu, and the picture in which the colour of
 * one of them is chosen from a grid of 16 by 16. The picture is not
 * here yet: Enter and a second click on a line do nothing.
 *
 * Translated from VCSETUP.ASM of the original's setup program: F05_00 to
 * F07_57, F07_60 to F07_85 and Col00 to Col55. A function has St in front
 * of the label that it begins at.
 *
 * How it is laid out here. In the assembly the two screens are one
 * stretch of code that is entered at many labels and left by jumps, with
 * what it knows of the list in five registers through every jump: BX and
 * DX, where the box of the list begins and how large it is, BP, the
 * palette, and SI and DI, the cursor and the first line shown, on their
 * way to F07_10; and in six variables, ColCur, ColFrst, ColNum, BackC,
 * HistC and HstCuC. Here the six variables are the setup's, of struct
 * StData of vc.h, and the box and the palette are `list`, a variable of
 * this file: nothing has to be in a register, and no function is entered
 * twice at a time. Every label that the assembly jumps to with a
 * new cursor is a function that works the cursor out and calls StF07_10,
 * which is the one place that moves it; and every jump back to where the
 * list or the picture waits for a key, F07_20 and Col10, is a return to
 * the loop of StF07_01 or of StCol00. What a key or the mouse makes of
 * the screen besides, leaving it or going on into the picture, is what
 * the function returns: a WHAT_ below.
 *
 * Two tables of the assembly are switch statements here, Color_T and
 * ColSt_T: the keys of the list and of the picture, with the label that
 * Case jumps to for each. Case takes no key without a scan code, and a
 * key of its table without one, Enter and Esc, with any.
 *
 * The mouse is asked where it is as often as the original asks, and in
 * the same places: a button of the tests is held for so many askings.
 */
#include "vc.h"

/* Of other files. ColAddr is where the colour table has a colour of the
   palette that is set.
   MvsBw writes characters in a colour, the colour in the high byte. */
u8 *ColAddr(u16 number);                                /* screen.c */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);

/* The tables, of stcolor_t.c, in the segment that AsmSeg returns: see
   there. */
extern const u8 __far StColor_S[];      /* " Color Setup " */
extern const u8 __far StMode_S[];       /* "Mode: " */
extern const u8 __far StPalet_S[];      /* "Palette: " */
extern const u8 __far StBwPal_S[];      /* the names of the palettes, */
extern const u8 __far StLapTp_S[];
extern const u8 __far StColMd_S[];      /* which is a display's too, */
extern const u8 __far StMonMd_S[];      /* as "Monochrome" is */
extern const u8 __far StColrs_S[];      /* the names of the 39 items */
extern const u8 __far StOn_S[];         /* "on" and a colour */
extern const u8 __far StColNm_S[];      /* the names of the colours */
extern const u8 __far StColNm_P[];      /* the colour of such a name */
extern const u8 __far StDfCol_B[];      /* "set default colors?" */

/* The setup without a setup file, of stmain_t.c: the default palettes
   first, as the colour table has them. */
extern const u8 __far StPalets[];

/* What the setup program has of its own: struct StData of vc.h. */
#define sd (*(struct StData *)md.MenuBuf)

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define Hist_C  23                      /* the colour of a list, */
#define HstCu_C 24                      /* and of the line of its cursor */
#define ShadFl  38                      /* how shadows are made: the last
                                           item */
#define LenColr 40                      /* the colours of a palette */

#define CROSS 'x'                       /* the pointer while the middle
                                           button leaves */

/* What the list knows besides its variables, which are the setup's
   (`sd`): ColCur, the item of the cursor; ColFrst, the first one shown,
   FFF0h while none is; ColNum, how many there are; BackC, the colour of
   the panels as it was when the list came up, which the right of the
   screen is written in; HistC, the colour of the list, and HstCuC, of its
   cursor. `begin` and `size` are the box, as Window takes them, the row
   in the high byte: BX and DX of the assembly. `palette` is BP: 0 for
   Black & White, 1 for Color, 2 for Laptop. */
static struct {
    u16 begin;
    u16 size;
    u16 palette;
} list;

/* What a key or the mouse has made of the list or of the picture. */
enum {
    WHAT_STAY,                          /* nothing more: wait for the next */
    WHAT_LEAVE,                         /* F07_57 and Col55 */
    WHAT_ENTER                          /* Col00, and in the picture Col50:
                                           the colour is taken */
};

/* The lines of the list that show: F07_60 of the assembly, which hands
   the cursor and the first line over as well. */
static u16 Lines(void)
{
    return (u8)((list.size >> 8) - 5);
}

/* The text number `number` of texts that stand one behind the other,
   each with its zero: F07_66, and F07_65 for the names of the items. */
static const u8 __far *StF07_66(const u8 __far *text, u16 number)
{
    for (; number != 0; number--)
        while (*text++ != 0)
            ;
    return text;
}

/* Write the name of an item into the screen buffer at `place`, behind a
   space and with spaces up to the width of the list. Returns the name
   behind it. */
static const u8 __far *StF07_70(u16 place, const u8 __far *text)
{
    u16 *screen = BasAdr(place);
    u16 room = (u8)(list.size - 12);
    u16 color = (u16)sd.HistC << 8;
    u16 character = ' ';

    for (;;) {
        *screen++ = color | character;
        if (--room == 0)
            break;
        character = *text;
        if (character == 0)
            character = ' ';
        else
            text++;
    }
    return text + 1;
}

/* Exchange the colour of the panels, in the palette that is set, with
   the one that the list keeps: around what is written on the right of
   the screen, which stays in the colour that it was drawn in when the
   palette that is edited is the one in use, and in the picture, where
   the palette that is edited is set. F07_85 of the assembly is the
   exchange, for any colour and any place, and nothing calls it but
   this. */
static void StF07_80(void)
{
    u8 *color = ColAddr(Back_C);
    u8 was = *color;

    *color = sd.BackC;
    sd.BackC = was;
}

/* Show a colour on the right of the screen: the cursor on its cell of
   the grid, the foreground by the column and the background by the row,
   and its name below the grid, "Yellow" and "on Blue". For the item
   Shadow the name is what the colour means there: No shadow for 0, Inv
   brightness on Black for 1, and White on Black for any other. */
static void StF07_75(u16 color)
{
    u16 front = color & 0x0F, behind = color >> 4 & 0x0F;
    u16 cell, i;
    u16 *screen;
    struct LineArgs line;

    Cursor(0x043D + (behind << 8 | front));
    if ((u8)sd.ColCur == ShadFl) {
        if (behind == 0 && front == 0) {
            front = 16;
            behind = 0xFF;
        } else if (behind == 0 && front == 1) {
            front = 17;
        } else {
            front = 7;
            behind = 0;
        }
    }
    StF07_80();
    cell = Color(StColNm_P[0]) << 8 | ' ';
    screen = BasAdr(0x153A);
    for (i = 0; i < 22; i++)
        screen[i] = cell;
    line.value = 0x1545;
    line.line = StF07_66(StColNm_S, front);
    line.colors = StColNm_P;
    line.include = line.line;           /* no name has ^I in it */
    CntLine((struct LineArgs STACK *)&line);
    Window(0x153A, 0x0116);
    screen = BasAdr(0x163A);
    for (i = 0; i < 22; i++)
        screen[i] = cell;
    if (behind != 0xFF) {
        line.value = 0x1645;
        line.line = StOn_S;
        line.colors = StColNm_P;
        line.include = StF07_66(StColNm_S, behind);
        CntLine((struct LineArgs STACK *)&line);
    }
    Window(0x163A, 0x0116);
    StF07_80();
}

/* Put the cursor of the list on the item `cursor`, with `first` the
   first line to show if that shows the cursor: else the lines move as
   far as it takes. If nothing changes by that, nothing is drawn. Else
   the line that the cursor was on is written anew; the lines, if they
   have moved; the cursor's line in its colour; and the colour of the
   item on the right of the screen.

   Where the lines have moved by one, the BIOS moves what the screen
   shows of them and the one new line is put on the screen, 49 cells wide
   where a line is 41: what is behind it on its row, the frame and the
   shadow, as the screen has them. */
static void StF07_10(u16 cursor, u16 first)
{
    u16 lines = Lines();
    u16 moved, was, top, place, size, count, upper, lower;
    const u8 __far *text;

    if (cursor < first)
        first = cursor;
    if (cursor >= lines + first)
        first = cursor - lines + 1;
    moved = first - sd.ColFrst;
    was = sd.ColCur;
    top = sd.ColFrst;
    sd.ColCur = cursor;
    sd.ColFrst = first;
    if (moved == 0 && was == cursor)
        return;

    if (top != 0xFFF0) {                /* F07_13: where the cursor was */
        place = list.begin + ((was - top & 0xFF) << 8) + 0x0205;
        StF07_70(place, StF07_66(StColrs_S, was));
        Window(place, 0x0100 | (u8)(list.size - 12));
    }
    if (moved != 0) {                   /* F07_14: the lines */
        text = StF07_66(StColrs_S, sd.ColFrst);
        place = list.begin + 0x0205;
        count = lines;
        do {
            text = StF07_70(place, text);
            place += 0x0100;
        } while (--count != 0 && *text != 0);
        place = list.begin + 0x0205;
        size = list.size - 0x050C;
        if (moved == 1 || moved == 0xFFFF) {
            upper = place;
            lower = upper + (size - 0x0101);
            Scroll(moved != 1, sd.HistC, upper, lower);
            if (moved == 1)
                place += (u16)(u8)((size >> 8) - 1) << 8;
            size = 0x0100 | (u8)(lower + 1);
        }
        Window(place, size);
    }
    /* F07_18: the cursor, and its colour. */
    place = list.begin + ((sd.ColCur - sd.ColFrst & 0xFF) << 8) + 0x0205;
    size = 0x0100 | (u8)(list.size - 12);
    MvsColr(BasAdr(place), size & 0xFF, sd.HstCuC);
    Window(place, size);
    StF07_75(md.ColTab[sd.ColCur * 6 + list.palette * 2
                       + (md.ScrMod == 7 ? 1 : 0)]);
}

/* Up, and the mouse held above the lines. */
static void StF07_40(void)
{
    StF07_10(sd.ColCur != 0 ? sd.ColCur - 1 : 0, sd.ColFrst);
}

/* Down, and the mouse held below the lines. */
static void StF07_41(void)
{
    u16 cursor = sd.ColCur + 1;

    if (cursor >= sd.ColNum)
        cursor--;
    StF07_10(cursor, sd.ColFrst);
}

/* PgUp: the lines a page back, which is one line less than show, with
   the cursor where it is if it still shows, else on the last line; and
   from the first page to the first item. */
static void StF07_44(void)
{
    u16 cursor = sd.ColCur, first = sd.ColFrst;
    u16 page = Lines() - 1;

    if (first == 0)
        cursor = 0;
    else {
        first = first >= page ? first - page : 0;
        if (cursor > page + first)
            cursor = page + first;
    }
    StF07_10(cursor, first);
}

/* PgDn: the lines a page on, as far as the last page, with the cursor
   where it is if it still shows, else on the first line; and from the
   last page to the last item. A list that has room for all its items is
   on its last page. */
static void StF07_48(void)
{
    u16 cursor = sd.ColCur, first = sd.ColFrst;
    u16 lines = Lines();
    u16 last = sd.ColNum >= lines ? sd.ColNum - lines : 0;

    if (first >= last) {
        cursor = sd.ColNum;
        if (cursor != 0)
            cursor--;
    } else {
        first += lines - 1;
        if (first > last)
            first = last;
        if (cursor < first)
            cursor = first;
    }
    StF07_10(cursor, first);
}

/* F8: ask whether to take the default colours of the palette, and take
   them. Nothing is drawn afterwards, as in the original, which goes back
   to F07_10 with the cursor where it is: the colour named on the right
   of the screen is the one from before until the cursor moves, and the
   cursor of the grid, which the box has hidden, comes back then too. */
static void StF07_53(void)
{
    u16 at, i;

    if ((u8)Dialog(OFFSET(StDfCol_B), 0, 0, 0, MAIN_SEG) != 0) {
        md.HlpPage = 4;
        return;
    }
    md.HlpPage = 4;
    at = list.palette * 2 + (md.ScrMod == 7 ? 1 : 0);
    for (i = 0; i < LenColr; i++)
        md.ColTab[at + i * 6] = StPalets[at + i * 6];
}

/* The mouse in the list: F07_30. The middle button leaves, once it is
   let go with no other held. A button on a line puts the cursor there,
   and a second click on the line is Enter; on a row below the last item,
   which a screen of 50 lines has, it is the last item. A button that
   went down on a line and is held above the lines or below them is Up or
   Down for as long as it is held. The left button that went down
   elsewhere and is let go outside the box leaves. */
static u16 StF07_30(void)
{
    struct Mouse mouse = GetMous(0);
    u16 held, item, row, click;

    for (;;) {                          /* F07_31 */
        ClrKbd();
        held = mouse.place;
        if (mouse.buttons & 4) {
            TypMous(CROSS);
            do {
                mouse = GetMous(mouse.place);
                ClrKbd();
            } while (mouse.buttons & 4);
            TypMous(0);
            if ((mouse.buttons & 3) == 0)
                return WHAT_LEAVE;
        }
        if (!InWind(mouse.place, list.begin + 0x0204, list.size - 0x050A))
            break;
        row = mouse.place >> 8;
        if (md.WinMous != 0) {
            if (row < (u8)((list.begin >> 8) + 2))
                StF07_40();
            else if (row >= (u8)((list.begin >> 8) + (list.size >> 8) - 3))
                StF07_41();
            return WHAT_STAY;
        }
        if ((mouse.buttons & 1) == 0)
            return WHAT_STAY;
        mouse = GetMous(mouse.place);
        if (mouse.buttons == 0)
            return InWind(held, list.begin, list.size - 0x0102)
                 ? WHAT_LEAVE : WHAT_STAY;
    }
    /* F07_37: on a line. */
    md.WinMous |= 1;
    item = (u8)((mouse.place >> 8) - (list.begin >> 8) - 2) + sd.ColFrst;
    if (item > sd.ColNum - 1) {
        StF07_10(sd.ColNum - 1, sd.ColFrst);
        return WHAT_STAY;
    }
    click = DoublMo(mouse.place & 0xFF00 | (u8)(list.begin + 4),
                    0x0100 | (u8)(list.size - 10), item);
    if (click == 2) {
        StF07_10(item, sd.ColFrst);
        return WHAT_STAY;
    }
    if (click == 0)
        return WHAT_STAY;
    md.MousEnt = 0;
    md.MousTim |= 0xFFFF0000UL;
    TypMous(0);
    return WHAT_ENTER;
}

/* The list of the colours of the palette `palette`, whose name is at
   `name`: F07_01. The right of the screen has the kind of the display,
   the name of the palette, the grid of the 256 colours, every cell an x
   in its colour, and below it the name of the colour that the item under
   the cursor has. It is cleared down to the key bar, or to the last row
   if there is none. Returns when the list is left: to the setup's
   menu. */
static void StF07_01(const u8 __far *name, u16 palette)
{
    const u8 __far *mode;
    const u8 __far *text;
    u16 *screen;
    u16 size, rows, i, cell, shape, key, what, mouse;

    md.HlpPage = 4;
    mode = StColMd_S;
    if (md.ScrMod == 7)
        mode = StMonMd_S;
    list.palette = palette;
    size = (u16)md.Lines << 8 | 22;
    if (md.KeyBar != 0)
        size -= 0x0100;
    sd.BackC = (u8)Color(Back_C);
    screen = BasAdr(58);
    for (rows = size >> 8; rows != 0; rows--) {
        for (i = 0; i < 22; i++)
            screen[i] = (u16)sd.BackC << 8 | ' ';
        screen += 80;
    }
    screen = BasAdr(58);
    MovDz0(MovDz0(screen + 80 + 4, StMode_S), mode);
    MovDz0(MovDz0(screen + 160 + 1, StPalet_S), name);
    screen += 320 + 3;
    cell = 'x';
    for (rows = 16; rows != 0; rows--) {
        for (i = 0; i < 16; i++) {
            screen[i] = cell;
            cell += 0x0100;
        }
        screen += 80;
    }
    Window(58, size);

    list.begin = 0x0203;
    list.size = (u16)(u8)(md.Lines - 3) << 8 | 53;
    sd.HstCuC = (u8)Color(HstCu_C);
    sd.HistC = (u8)Color(Hist_C);
    MinBox(list.begin, list.size, (u16)sd.HistC << 8);
    MvsBw(BasAdr(list.begin) + 80 + 19, StColor_S, 13,
          (u16)sd.HistC << 8);
    Window(list.begin, list.size);
    sd.ColNum = 0;
    text = StColrs_S;
    do {
        sd.ColNum++;
        while (*text++ != 0)
            ;
    } while (*text != 0);
    sd.ColCur = 0;
    sd.ColFrst = 0xFFF0;
    md.WinMous = 0;
    md.MousEnt = 0;
    md.MousTim |= 0xFFFF0000UL;
    shape = ChgCur();
    StF07_10(0, 0);

    for (;;) {                          /* F07_20 */
        what = WHAT_STAY;
        mouse = GetMous(0).buttons != 0;
        if (!mouse) {
            md.WinMous = 0;
            md.MousEnt = 0;
            key = Input(2);
            mouse = key >> 8 == 0xFC;
        }
        if (mouse)
            what = StF07_30();
        else
            switch (key) {
            case 0x4800:
                StF07_40();
                break;
            case 0x5000:
                StF07_41();
                break;
            case 0x4700:                /* Home */
                StF07_10(0, 0);
                break;
            case 0x4F00:                /* End */
                StF07_10(sd.ColNum - 1, 0);
                break;
            case 0x4900:
                StF07_44();
                break;
            case 0x5100:
                StF07_48();
                break;
            case 0x4200:
                StF07_53();
                break;
            case 0x4400:                /* F10 */
                what = WHAT_LEAVE;
                break;
            default:
                if (key >> 8 == 0)
                    break;
                if ((u8)key == 0x1B)
                    what = WHAT_LEAVE;
                else if ((u8)key == 0x0D)
                    what = WHAT_ENTER;
            }
        if (what == WHAT_LEAVE)
            break;
        /* WHAT_ENTER is the picture of the item, Col00: not here yet. */
    }

    /* F07_57: the cursor as it was, and the screen as DOS left it. */
    HidCur();
    CursorShape(shape);
    StUser(0, md.Lines);
    Window(0, (u16)md.Lines << 8 | 80);
}

/* F5, F6 and F7 of the setup's menu: Set Black & White palette, Set
   Color palette and Set Laptop palette. */
void StF05_00(void)
{
    StF07_01(StBwPal_S, 0);
}

void StF06_00(void)
{
    StF07_01(StColMd_S, 1);
}

void StF07_00(void)
{
    StF07_01(StLapTp_S, 2);
}
