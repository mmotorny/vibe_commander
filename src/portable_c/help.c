/*
 * The help: the file it is read from, the index and the pages in their
 * window, and the keys and the mouse that move in them.
 *
 * Translated from VCHELP.INC, whose tables are in help_t.c.
 *
 * The help file is VC.HLP in the program's directory. It begins with a
 * header that holds its version, and at 194h has the place of its first
 * entry, the index. An entry is 26 bytes that describe it and then its
 * text; all of that, and the place at 194h, is kept with 1Ah added to
 * every byte without carry. Of the 26 bytes the program goes by the mode,
 * 0 for the index and not 0 for a page, the length of the text, the
 * number of its lines, the line of the index that a page belongs to, and
 * the place in the file of the index, which is FFFFFFFFh in the index
 * itself.
 *
 * The text of a page is its lines one after the other, each ended by a
 * zero byte, with the markup of NgLine in them; the first is its title
 * and is not counted. The text of the index begins with six bytes for
 * each of its lines, of which the last four are the place in the file of
 * the page that the line names.
 *
 * One entry at a time is in the memory, in HelpBuf, behind what the
 * program keeps about it. The dialogs and the menus say which page is
 * theirs in HlpPage, by its number in the index, from 1; 0 asks for the
 * index, and FFh says that the help is on the screen already.
 */
#include "vc.h"

#pragma pack(push, 1)

/* What the help keeps, in the main data from HelpBuf on: HlpData of the
   assembly. The name of the file is made there before it is opened, and
   the 13 bytes of the header that hold the version are read to where
   HlpFrst is. */
struct HlpData {
    u16 HlpHndl;                        /* the file */
    /* The 26 bytes that describe an entry, as they are read. */
    u16 HlpMode;                        /* 0 - the index */
    u16 HlpSize;                        /* the length of its text */
    u16 HlpLins;                        /* its lines, without a title */
    u16 Reserved1;
    u16 HlpBkLn;                        /* the line of the index for it */
    u16 HlpOffsLo;                      /* where the index is in the file, */
    u16 HlpOffsHi;                      /* or the entry to be read next */
    u16 HlpFrst;                        /* the first line shown */
    u16 HlpCur;                         /* the line of the cursor */
    u8  Reserved2[8];
    /* Behind them. */
    u8  Reserved3[2];
    u16 HlpBkFr;                        /* the first line shown of the index */
    u8  HlpInfo[LenHelp - 32];          /* the text */
};

#pragma pack(pop)

LAID_OUT(HlpData, LenHelp);

#define hd (*(struct HlpData *)md.HelpBuf)

void Help00(void);

/* This is Help. Its name is from when the assembly called it, under the
   name that the linker had for it then: the assembler took Help_c for
   Help_C, the number of a colour. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte, and GetCur says where it is. ChgDisk asks for
   the disk of a drive that shares the floppy drive with another, and is
   not 0 if the user would not change it. SavWin keeps what the screen
   buffer has where a window is to be, ResWin puts it back; MinBox draws a
   box with a shadow there, growing if windows are to zoom. Scroll moves
   a window of the screen by a line, with the mouse pointer out of the
   way.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo is asked
   about a click on the line `line` of a list: 2 for the first click on
   it, 1 for the second, 0 for a second one that ended elsewhere. ClrKbd
   empties the keyboard buffer if the setup has it so.

   PgUp and PgDn turn a list by a page of `lines`: they return the line of
   the cursor and the first line shown. */

/* C routines of other files that vc.h does not have. */
u16 *MvsBw(u16 *screen, const u8 *text, u16 count,
           u16 color);                                  /* screen.c */
u8 *MainFil(u8 *to, const u8 *name);                    /* string.c */

/* The tables of the help, in help_t.c: the title of the window, the
   box of the error, which Dialog takes a pointer to, and for MinMnu the
   buttons under the index and under a page and the numbers of their
   colours. */
extern const u8 Help_S[];               /* " Help " */
extern const struct BOXHD HlpEr_B;      /* "There was error reading data..." */
extern const u8 HlMn1_D[];              /* Help, Cancel */
extern const u8 HlMn2_D[];              /* Next, Previous, Index, Cancel */
extern const u8 HlpMn_P[];

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define Help_C  27                      /* the colours of the help: its text, */
#define HlpBo_C 28                      /* bold, */
#define HlpRe_C 29                      /* reversed, which the cursor is too, */
#define HlpUn_C 30                      /* and underlined */

/* The name of the help file, and behind it what its header has from the
   ninth byte on. They are one text because MainFil takes 13 bytes for a
   name: the first six of the second are copied with the first, as they
   are in the assembly. */
static const u8 Help_F[] = "VC.HLP\0VC 4.05 Help";
#define HlpHead 7                       /* where in it the second begins */
#define LenHead 13                      /* and its length, with its end */

/* The lines of the buttons, as they are written under the index and under
   a page. */
static const u8 HlMn1_S[] = "\xFF\x15[ Help ]\xFF\x03[ Cancel ]";
static const u8 HlMn2_S[] =
    "\xFF\x07[ Next ]\xFF\x03[ Previous ]\xFF\x03[ Index ]\xFF\x03[ Cancel ]";

/* The colours of a line of the help, for NgLine, and the line that it is
   to put in for ^I: none. */
static const u8 Help_P[] = { Help_C, HlpRe_C, HlpBo_C, HlpUn_C };
static const u8 Empty_S[] = "";

#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on the
                                           left and on the right */
#define WIDTH       62                  /* the columns of a line of text */

/* The window: where it begins, the row in the high byte, and from there
   its title, its first line and the lines of the text. */
#define BEGIN       0x204
#define AT_TITLE    0x205
#define AT_TEXT     0x405

/* True if the entry in the memory has no index to go back to: it is the
   index. */
#define NO_INDEX() ((hd.HlpOffsLo & hd.HlpOffsHi) == 0xFFFF)

/* The lines of text that the window shows: 13 or more. */
#define SHOWN() (md.Lines - 12)

/* Take the mark off `count` bytes that were read from the file. A count
   of 0 is one of 65536. Not static: the help of the setup, sthelp.c,
   calls it for its own pages, as the setup program has the routine word
   for word. */
void DeCode(u8 *at, u16 count)
{
    do
        *at++ ^= 0x1A;
    while (--count);
}

/* Go to a place in the help file. Not 0 if DOS would not, or went
   elsewhere. */
static u16 SeekDsk(u16 high, u16 low)
{
    u32 place = (u32)high << 16 | low;
    struct FilePlace sought = FileSeek(hd.HlpHndl, 0, place);

    if (sought.failed)
        return 1;
    return sought.place != place;
}

/* Read `count` bytes of the help file. Not 0 if DOS would not, or read
   fewer. */
static u16 ReadDsk(u8 *to, u16 count)
{
    struct Answer read = FileRead(hd.HlpHndl, to, count);

    if (read.failed)
        return 1;
    return read.value != count;
}

/* Read the entry at HlpOffs. For a page that follows the index, the first
   line of the index is kept and the page is shown from its start; for the
   index, or a page that follows a page, the cursor goes to the line that
   the entry before named and the first line is the one kept. Not 0 if the
   entry could not be read: what is in the memory is neither then. */
static u16 RdEntry(void)
{
    u16 cursor = hd.HlpBkLn;
    u16 first = hd.HlpBkFr;

    if (hd.HlpMode == 0) {
        hd.HlpBkFr = hd.HlpFrst;
        cursor = 0;
        first = 0;
    }
    if (SeekDsk(hd.HlpOffsHi, hd.HlpOffsLo))
        return 1;
    if (ReadDsk((u8 *)&hd.HlpMode, 26))
        return 1;
    DeCode((u8 *)&hd.HlpMode, 26);
    if (hd.HlpSize >= LenHelp - 0x20)
        return 1;
    if (ReadDsk(hd.HlpInfo, hd.HlpSize))
        return 1;
    DeCode(hd.HlpInfo, hd.HlpSize);
    hd.HlpLins -= hd.HlpMode;
    hd.HlpFrst = first;
    hd.HlpCur = cursor;
    return 0;
}

/* Take from the index, which is in the memory, where the page of its line
   `line` is in the file. */
static void Help70(u16 line)
{
    u8 *entry;

    line += line;
    entry = hd.HlpInfo + 2 + (u16)(line + line + line);
    hd.HlpOffsLo = *(u16 *)entry;
    hd.HlpOffsHi = *(u16 *)(entry + 2);
}

/* Where the text of a line is: of the index behind its six bytes for
   every line, of a page behind its title. The end of a text is looked for
   in 65535 bytes, and taken to be there if it is not. */
static u8 *Help72(u16 line)
{
    u8 *text = hd.HlpInfo;
    u16 count;

    line++;
    if (hd.HlpMode == 0) {
        line--;
        count = hd.HlpLins << 1;
        text += count;
        text += count;
        text += count;
    }
    for (; line; line--) {
        count = 0xFFFF;
        do
            count--;
        while (*text++ != 0 && count != 0);
    }
    return text;
}

/* Write a line of text into the window at a row and a column, over
   spaces. Returns where the text of the next line begins. */
static const u8 *Help77(u16 place, const u8 *text)
{
    struct LineArgs line;
    u16 cell = Color(Help_P[0]) << 8 | ' ';
    u16 i;

    line.screen = BasAdr(place);
    for (i = 0; i < WIDTH; i++)
        line.screen[i] = cell;
    md.TmpErr[1] = 0;
    md.TmpErr[101] = 0;
    line.value = WIDTH;
    line.line = text;
    line.colors = Help_P;
    line.include = Empty_S;
    NgLine(&line);
    return line.line;
}

/* Show the entry that is in the memory in a window, and go on from there:
   move in it, and read and show others, until a key or the mouse leaves
   the help or an entry cannot be read. */
static void Help05(void)
{
    const u8 *text;
    u16 size = (u16)(u8)(md.Lines - 3) << 8 | 74;
    u16 item = 0;                       /* CX: the button with the cursor */
    u16 cursor, first;                  /* SI and DI: where to go */
    u16 was, shown;                     /* and where the screen is */
    u16 *screen;
    const u8 *control;                  /* the buttons */
    u16 color, place, moved, count, key, buttons, mouse, held;
    i32 lines, third;
    struct Buttons menu;
    struct Paged paged;
    struct Mouse pointer;

    SavWin(BEGIN, size, MD_PLACE(md.ErrBuf1));
    color = Color(Help_C) << 8;
    MinBox(BEGIN, size, color);
    screen = BasAdr(BEGIN);
    MvsBw(screen + 80 + 33, Help_S, 6, color);
    BoxLine(BasAdr(BEGIN + (size & 0xFF00) - 0x500), color | LINE,
            LEFT_RIGHT, 3, 64);
    BoxLine(screen + 3 * 80, color | LINE, LEFT_RIGHT, 3, 64);
    Window(BEGIN, size);
    hd.HlpBkFr = 0;
    md.HlpPage = 0xFF;
    md.WinMous = 0;
    md.MousEnt = 0;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;

Help06:                                 /* the title and the buttons */
    if (hd.HlpMode != 0)
        text = hd.HlpInfo;
    else
        text = Title_S;
    Help77(BEGIN + AT_TITLE, text);
    Window(BEGIN + AT_TITLE, 0x100 | WIDTH);
    if (NO_INDEX()) {
        item &= 0xFF00;
        text = HlMn1_S;
    } else {
        text = HlMn2_S;
    }
    Help77(BEGIN + AT_TITLE + (size & 0xFF00) - 0x600, text);
    cursor = hd.HlpCur;
    first = hd.HlpFrst;
    hd.HlpFrst = 0xFFF0;                /* nothing is shown yet */

Help10:                                 /* the cursor within the window */
    if (cursor < first)
        first = cursor;
    lines = (size >> 8) - 9;
    if (cursor >= (u16)(lines + first))
        first = cursor - lines + 1;
    was = hd.HlpCur;
    hd.HlpCur = cursor;
    shown = hd.HlpFrst;
    hd.HlpFrst = first;
    moved = first - shown;
    if (moved != 0 || hd.HlpMode == 0 && was != hd.HlpCur) {
        if (shown != 0xFFF0) {          /* the line that had the cursor */
            place = BEGIN + (was - shown << 8) + AT_TEXT;
            Help77(place, Help72(was));
            Window(place, 0x100 | WIDTH);
        }
        if (moved != 0) {               /* the lines */
            text = Help72(hd.HlpFrst);
            place = BEGIN + AT_TEXT;
            count = (u8)((size >> 8) - 9);
            do {
                /* Behind the text there are only empty lines. */
                if ((u16)(MD_PLACE(NEAR(text)) - MD_PLACE(hd.HlpInfo))
                        >= hd.HlpSize)
                    text--;
                text = Help77(place, text);
                place += 0x100;
            } while (--count);
            /* On the screen: all of them, or the screen moved by a line
               and the one that is new. */
            place = BEGIN + AT_TEXT;
            count = SHOWN();
            if (moved == 1 || moved == 0xFFFF) {
                key = moved == 1 ? 0x600 | count - 1 & 0xFF : 0x700;
                Scroll(key >> 8 == 7, Color(Help_C), place,
                       ((count - 1 & 0xFF) << 8 | WIDTH - 1) + place);
                place += key << 8;
                count = 1;
            }
            Window(place, count << 8 | WIDTH);
        }
        if (hd.HlpMode == 0) {          /* the cursor */
            place = BEGIN + (hd.HlpCur - hd.HlpFrst << 8) + AT_TEXT;
            MvsColr(BasAdr(place), WIDTH, Color(HlpRe_C));
            Window(place, 0x100 | WIDTH);
        }
    }

Help25:                                 /* a key */
    if (GetMous(item).buttons != 0)
        goto Help30;
    md.WinMous = 0;
    md.MousEnt = 0;
    if (NO_INDEX())
        control = HlMn1_D;
    else
        control = HlMn2_D;
    menu = MinMnu(control, BEGIN + (size & 0xFF00) - 0x400 + 12, 0x130,
                  HlpMn_P, 2, item);
    key = menu.key;
    item = menu.chosen;
    if (key >> 8 == 0xFC)
        goto Help30;
    if (key >> 8 == 0)                  /* Case takes none without a scan code */
        goto Help25;
    if ((key & 0xFF) == 0x0D)
        goto Help60;
    if ((key & 0xFF) == 0x1B)
        goto Help66;
    if ((key & 0xFF) != 0)
        goto Help25;
    switch (key >> 8) {
    case 0x48: goto Help50;
    case 0x50: goto Help53;
    case 0x47:                          /* Home */
        /* The assembly goes on with the first line as a register happens
           to have it, and that is made 0 at Help10 whatever it is. */
        cursor = 0;
        first = 0;
        goto Help10;
    case 0x4F:                          /* End */
        first = 0;
        cursor = hd.HlpLins;
        if (cursor != 0)
            cursor--;
        goto Help10;
    case 0x49:                          /* PgUp */
        paged = PgUp(hd.HlpCur, hd.HlpFrst, SHOWN());
        cursor = paged.cursor;
        first = paged.first;
        goto Help10;
    case 0x51:                          /* PgDn */
        paged = PgDn(hd.HlpCur, hd.HlpFrst, SHOWN(), hd.HlpLins);
        cursor = paged.cursor;
        first = paged.first;
        goto Help10;
    case 0x44: goto Help66;             /* F10 */
    }
    goto Help25;

Help30:                                 /* the mouse */
    pointer = GetMous(item);
    buttons = pointer.buttons;
    mouse = pointer.place;
Help31:
    ClrKbd();
    held = mouse;
    if (buttons & 4) {                  /* the middle button, until it is */
        TypMous('x');                   /* let go: leaves, unless another */
        do {                            /* is down too */
            pointer = GetMous(mouse);
            buttons = pointer.buttons;
            mouse = pointer.place;
            ClrKbd();
        } while (buttons & 4);
        TypMous(0);
        if ((buttons & 3) == 0)
            goto Help66;
    }
    if (InWind(mouse, BEGIN + 0x404, size - 0x90A)) {
        /* Not where the text is. */
        if (md.WinMous == 0) {
            /* And the button did not go down there: leaves when it is
               let go outside the window. */
            if ((buttons & 1) == 0)
                goto Help25;
            pointer = GetMous(mouse);
            buttons = pointer.buttons;
            mouse = pointer.place;
            if (buttons != 0)
                goto Help31;
            if (!InWind(held, BEGIN, size - 0x102))
                goto Help25;
            goto Help66;
        }
        third = 0;
        if (hd.HlpMode != 0)
            third = ((size >> 8) - 9) / 3;
    } else {
        md.WinMous |= 1;
        if (hd.HlpMode == 0) {          /* a line of the index */
            first = hd.HlpFrst;
            cursor = (u8)((mouse >> 8) - (BEGIN >> 8) - 4) + first;
            count = hd.HlpLins - 1;
            if (cursor > count) {
                cursor = count;
                goto Help10;
            }
            key = DoublMo(mouse & 0xFF00 | (u8)(BEGIN + 4),
                          0x100 | (u8)(size - 10), cursor);
            if (key == 2)
                goto Help10;
            if (key == 0)
                goto Help25;
            md.MousEnt = 0;
            ((u16 *)&md.MousTim)[1] = 0xFFFF;
            TypMous(0);
            goto Help60;
        }
        third = ((size >> 8) - 9) / 3;
    }
    /* Held above the text, or in its first third on a page: up; below it
       or in the last third: down. */
    if ((u8)(mouse >> 8) < (u8)((BEGIN >> 8) + 4 + third))
        goto Help50;
    if ((u8)(mouse >> 8) >= (u8)((BEGIN >> 8) + (size >> 8) - 5 - third))
        goto Help53;
    goto Help25;

Help50:                                 /* Up */
    cursor = hd.HlpCur;
    first = hd.HlpFrst;
    count = hd.HlpMode != 0 ? first : cursor;
    if (count != 0)
        cursor = count - 1;
    goto Help10;

Help53:                                 /* Down */
    cursor = hd.HlpCur;
    first = hd.HlpFrst;
    count = SHOWN() + first;
    if (hd.HlpMode == 0)
        count = cursor + 1;
    if (count < hd.HlpLins)
        cursor = count;
    goto Help10;

Help60:                                 /* Enter */
    if (NO_INDEX()) {
        if ((item & 0xFF) != 0)         /* Cancel */
            goto Help66;
        if (hd.HlpMode != 0) {
            cursor = hd.HlpCur;
            first = hd.HlpFrst;
            goto Help10;
        }
        count = hd.HlpCur;              /* the page of the line */
    } else {
        *(u8 *)&hd.HlpMode = 1;
        if ((item & 0xFF) == 2)         /* Index */
            goto Help64;
        if ((item & 0xFF) > 2)          /* Cancel */
            goto Help66;
        if (RdEntry())                  /* the index, for the page behind */
            goto Help65;                /* this one in it or before it */
        count = hd.HlpCur;
        if ((item & 0xFF) == 0) {
            count++;
            if (count >= hd.HlpLins)
                count = 0;
        } else if (count-- == 0) {
            count = hd.HlpLins - 1;
        }
    }
    Help70(count);
Help64:
    if (!RdEntry())
        goto Help06;

Help65:                                 /* it could not be read */
    Dialog(&HlpEr_B, 0, 0, 0, &md);

Help66:                                 /* Esc */
    ResWin(BEGIN, size, MD_PLACE(md.ErrBuf1));
    Window(BEGIN, size);
}

/* Open the help file and show the page that HlpPage names, or the index;
   say so if the file is not a help file of this version or cannot be
   read. Without the file there is no help, and nothing is said. */
static void HelpFil(void)
{
    struct Answer opened;
    u16 i;

    HidCur();
    MainFil(md.HelpBuf, Help_F);
    if (ChgDisk(md.HelpBuf))
        return;
    opened = OpenFil(md.HelpBuf, 0x40);
    if (opened.failed)
        return;
    hd.HlpHndl = opened.value;
    if (SeekDsk(0, 8) || ReadDsk(md.HelpBuf + 16, LenHead))
        goto Help01;
    for (i = 0; i < LenHead; i++)
        if (md.HelpBuf[16 + i] != Help_F[HlpHead + i])
            goto Help01;
    if (SeekDsk(0, 0x194) || ReadDsk((u8 *)&hd.HlpOffsLo, 4))
        goto Help01;
    DeCode((u8 *)&hd.HlpOffsLo, 4);
    hd.HlpBkLn = 0;
    if (RdEntry())                      /* the index */
        goto Help01;
    if (md.HlpPage != 0 && hd.HlpMode == 0) {
        Help70(md.HlpPage - 1);
        if (RdEntry())
            goto Help01;
    }
    Help05();
    goto Help67;

Help01:
    Dialog(&HlpEr_B, 0, 0, 0, &md);

Help67:
    FileClose(hd.HlpHndl);
}

/* The help, for F1: on the page that HlpPage names. While it is shown
   DOS's critical errors bring up no box, and F1 does nothing. */
void Help00(void)
{
    u16 place = GetCur();
    u16 page = md.HlpPage;
    u16 errors = md.BlocEr;

    md.BlocEr = 1;
    if (page != 0xFF)
        HelpFil();
    md.HlpPage = page;
    md.BlocEr = errors;
    Cursor(place);
}
