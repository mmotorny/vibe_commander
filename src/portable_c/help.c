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

/* Fields of what the help keeps that are read from the file and decoded
   as one block, as they are in the file: the bytes of HelpBuf from the
   first field's place. */
#define HD_BYTES(field) (md.HelpBuf + offsetof(struct HlpData, field))

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
   of 0 is one of 65536, as the original's LOOP has it, and is handed
   over by no caller: the original's takes an entry whose text has no
   bytes, and goes round all of its segment, where in a flat memory that
   would be 64K behind HlpInfo, past the end of the main data. Such a
   file is refused, here and in sthelp.c (DEVIATIONS.md, 23). Not static:
   the help of the setup calls it for its own pages, as the setup program
   has the routine word for word. */
void DeCode(u8 *at, u16 count)
{
    do
        *at++ ^= 0x1A;
    while (--count);
}

/* Go to a place in the help file. True if DOS would not, or went
   elsewhere. */
static bool SeekDsk(u16 high, u16 low)
{
    u32 place = (u32)high << 16 | low;
    struct FilePlace sought = FileSeek(hd.HlpHndl, place);

    if (sought.failed)
        return true;
    return sought.place != place;
}

/* Read `count` bytes of the help file, fewer than 2,968. True if DOS
   would not, or read fewer. */
static bool ReadDsk(u8 *to, i32 count)
{
    struct Moved read = FileRead(hd.HlpHndl, to, count);

    if (read.failed)
        return true;
    return read.count != count;
}

/* Read the entry at HlpOffs. For a page that follows the index, the first
   line of the index is kept and the page is shown from its start; for the
   index, or a page that follows a page, the cursor goes to the line that
   the entry before named and the first line is the one kept. True if the
   entry could not be read: what is in the memory is neither then.

   An entry whose text is too long for the room is refused as one that
   could not be read, and here one whose text has no bytes too, which the
   original takes and decodes the 64K of its main data for
   (DEVIATIONS.md, 23). */
static bool RdEntry(void)
{
    i32 cursor = hd.HlpBkLn;
    i32 first = hd.HlpBkFr;

    if (hd.HlpMode == 0) {
        hd.HlpBkFr = hd.HlpFrst;
        cursor = 0;
        first = 0;
    }
    if (SeekDsk(hd.HlpOffsHi, hd.HlpOffsLo))
        return true;
    if (ReadDsk(HD_BYTES(HlpMode), 26))
        return true;
    DeCode(HD_BYTES(HlpMode), 26);
    if (hd.HlpSize == 0 || hd.HlpSize >= LenHelp - 0x20)
        return true;
    if (ReadDsk(hd.HlpInfo, hd.HlpSize))
        return true;
    DeCode(hd.HlpInfo, hd.HlpSize);
    hd.HlpLins -= hd.HlpMode;
    hd.HlpFrst = (u16)first;
    hd.HlpCur = (u16)cursor;
    return false;
}

/* Take from the index, which is in the memory, where the page of its line
   `line` is in the file. */
static void Help70(i32 line)
{
    u8 *entry;

    line += line;
    /* Six bytes for each line, which wraps as the original's offset of
       16 bits from a line of 10,923 on: only an index that is not true
       to its text has such lines. */
    entry = hd.HlpInfo + 2 + (u16)(line + line + line);
    hd.HlpOffsLo = *(u16 *)entry;
    hd.HlpOffsHi = *(u16 *)(entry + 2);
}

/* Where the text of a line is: of the index behind its six bytes for
   every line, of a page behind its title. The end of a text is looked for
   in 65535 bytes, and taken to be there if it is not. */
static u8 *Help72(i32 line)
{
    u8 *text = hd.HlpInfo;
    i32 count;

    /* Both wrap as the original's words of 16 bits, which only an entry
       that is not true to its text gets to: the six bytes of an index
       of 32,768 lines or more, and the line behind 65,535, which is
       the title. */
    if (hd.HlpMode == 0) {
        count = (u16)(hd.HlpLins << 1);
        text += count;
        text += count;
        text += count;
    } else {
        line = (u16)(line + 1);
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
    u16 cell = (u16)(Color(Help_P[0]) << 8 | ' ');
    i32 i;

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

/* Where the window goes on from a key or the mouse: shown again from
   the line that the cursor is to be on and the first line, a line up or
   down, Enter on the button with the cursor, the next key, the title
   and the buttons anew for an entry that has been read, the box for one
   that could not be, or out. The assembly jumps to each of those. */
enum {
    HELP_SHOW,                          /* Help10 */
    HELP_UP,                            /* Help50 */
    HELP_DOWN,                          /* Help53 */
    HELP_ENTER,                         /* Help60 */
    HELP_KEY,                           /* Help25 */
    HELP_PAGE,                          /* Help06 */
    HELP_FAILED,                        /* Help65 */
    HELP_LEAVE                          /* Help66 */
};

/* The mouse, Help30 of the assembly: a button is down. Where it goes
   on to the window, the cursor and the first line are in `at`. */
static i32 Help30(u16 item, u16 size, struct Paged *at)
{
    u16 buttons, mouse, held;
    i32 count, third, clicked;
    bool outside;
    struct Mouse pointer;

    pointer = GetMous(item);
    buttons = pointer.buttons;
    mouse = pointer.place;
    for (;;) {                          /* Help31 */
        ClrKbd();
        held = mouse;
        if (buttons & 4) {              /* the middle button, until it is */
            TypMous('x');               /* let go: leaves, unless another */
            do {                        /* is down too */
                pointer = GetMous(mouse);
                buttons = pointer.buttons;
                mouse = pointer.place;
                ClrKbd();
            } while (buttons & 4);
            TypMous(0);
            if ((buttons & 3) == 0)
                return HELP_LEAVE;
        }
        outside = InWind(mouse, BEGIN + 0x404, size - 0x90A);
        if (!outside || md.WinMous != 0)
            break;
        /* Not where the text is, and the button did not go down there:
           leaves when it is let go outside the window. */
        if ((buttons & 1) == 0)
            return HELP_KEY;
        pointer = GetMous(mouse);
        buttons = pointer.buttons;
        mouse = pointer.place;
        if (buttons == 0) {
            if (!InWind(held, BEGIN, size - 0x102))
                return HELP_KEY;
            return HELP_LEAVE;
        }
    }
    if (outside) {
        /* Not where the text is. */
        third = 0;
        if (hd.HlpMode != 0)
            third = ((size >> 8) - 9) / 3;
    } else {
        md.WinMous |= 1;
        if (hd.HlpMode == 0) {          /* a line of the index */
            at->first = hd.HlpFrst;
            /* Both wrap, marked: the last line of an index without
               lines is 65,535. */
            at->cursor = (u16)((u8)((mouse >> 8) - (BEGIN >> 8) - 4)
                               + at->first);
            count = (u16)(hd.HlpLins - 1);
            if (at->cursor > count) {
                at->cursor = count;
                return HELP_SHOW;
            }
            clicked = DoublMo((mouse & 0xFF00) | (u8)(BEGIN + 4),
                              0x100 | (u8)(size - 10), at->cursor);
            if (clicked == 2)
                return HELP_SHOW;
            if (clicked == 0)
                return HELP_KEY;
            md.MousEnt = 0;
            ((u16 *)&md.MousTim)[1] = 0xFFFF;
            TypMous(0);
            return HELP_ENTER;
        }
        third = ((size >> 8) - 9) / 3;
    }
    /* Held above the text, or in its first third on a page: up; below it
       or in the last third: down. */
    if ((u8)(mouse >> 8) < (u8)((BEGIN >> 8) + 4 + third))
        return HELP_UP;
    if ((u8)(mouse >> 8) >= (u8)((BEGIN >> 8) + (size >> 8) - 5 - third))
        return HELP_DOWN;
    return HELP_KEY;
}

/* A key, Help25 of the assembly, or the mouse, until one of them moves
   the window, chooses a button or leaves. The button with the cursor is
   in `item`; where the window goes on to, in `at`. */
static i32 Help25(u16 size, u16 *item, struct Paged *at)
{
    const u8 *control;                  /* the buttons */
    u16 key;
    i32 go;
    struct Buttons menu;

    for (;;) {                          /* Help25 */
        key = 0xFC00;                   /* the mouse, as MinMnu answers it */
        if (GetMous(*item).buttons == 0) {
            md.WinMous = 0;
            md.MousEnt = 0;
            if (NO_INDEX())
                control = HlMn1_D;
            else
                control = HlMn2_D;
            menu = MinMnu(control, BEGIN + (size & 0xFF00) - 0x400 + 12,
                          0x130, HlpMn_P, 2, *item);
            key = menu.key;
            *item = menu.chosen;
        }
        if (key >> 8 == 0xFC) {
            go = Help30(*item, size, at);
            if (go != HELP_KEY)
                return go;
            continue;
        }
        if (key >> 8 == 0)              /* Case takes none without */
            continue;                   /* a scan code */
        if ((key & 0xFF) == 0x0D)
            return HELP_ENTER;
        if ((key & 0xFF) == 0x1B)
            return HELP_LEAVE;
        if ((key & 0xFF) != 0)
            continue;
        switch (key >> 8) {
        case 0x48: return HELP_UP;
        case 0x50: return HELP_DOWN;
        case 0x47:                      /* Home */
            /* The assembly goes on with the first line as a register
               happens to have it, and that is made 0 at Help10 whatever
               it is. */
            at->cursor = 0;
            at->first = 0;
            return HELP_SHOW;
        case 0x4F:                      /* End */
            at->first = 0;
            at->cursor = hd.HlpLins;
            if (at->cursor != 0)
                at->cursor--;
            return HELP_SHOW;
        case 0x49:                      /* PgUp */
            *at = PgUp(hd.HlpCur, hd.HlpFrst, SHOWN());
            return HELP_SHOW;
        case 0x51:                      /* PgDn */
            *at = PgDn(hd.HlpCur, hd.HlpFrst, SHOWN(), hd.HlpLins);
            return HELP_SHOW;
        case 0x44: return HELP_LEAVE;   /* F10 */
        }
    }
}

/* Enter on a button, Help60 of the assembly: the page of the line, the
   page in front or behind, the index, or Cancel. Where it goes on to the
   window, the cursor and the first line are in `at`. */
static i32 Help60(u16 item, struct Paged *at)
{
    i32 count;

    if (NO_INDEX()) {
        if ((item & 0xFF) != 0)         /* Cancel */
            return HELP_LEAVE;
        if (hd.HlpMode != 0) {
            at->cursor = hd.HlpCur;
            at->first = hd.HlpFrst;
            return HELP_SHOW;
        }
        Help70(hd.HlpCur);              /* the page of the line */
    } else {
        *(u8 *)&hd.HlpMode = 1;
        if ((item & 0xFF) != 2) {       /* Index: on to Help64 */
            if ((item & 0xFF) > 2)      /* Cancel */
                return HELP_LEAVE;
            if (RdEntry())              /* the index, for the page behind */
                return HELP_FAILED;     /* this one in it or before it */
            count = hd.HlpCur;
            if ((item & 0xFF) == 0) {
                count++;
                if (count >= hd.HlpLins)
                    count = 0;
            } else if (count-- == 0) {
                count = (u16)(hd.HlpLins - 1);  /* wraps, marked */
            }
            Help70(count);
        }
    }
    if (RdEntry())                      /* Help64 */
        return HELP_FAILED;
    return HELP_PAGE;
}

/* Show the entry that is in the memory in a window, and go on from there:
   move in it, and read and show others, until a key or the mouse leaves
   the help or an entry cannot be read.
   The lines are the entry's, which the file and HlpData have in 16 bits.
   Where a line is made of them that would pass 65,535, it wraps as the
   original's word does: an entry whose numbers are not true to its text
   gets there, one that names a line of the index far behind its last.
   The cuts that say so are marked. */
static void Help05(void)
{
    const u8 *text;
    u16 size = (u16)((u8)(md.Lines - 3) << 8 | 74);
    u16 item = 0;                       /* CX: the button with the cursor */
    struct Paged at;                    /* SI and DI: where to go */
    i32 was, shown, moved;              /* and where the screen is */
    u16 *screen;
    u16 color, place, key;
    i32 count, lines, go;

    SavWin(BEGIN, size, md.ErrBuf1);
    color = (u16)(Color(Help_C) << 8);
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

    /* Help06: the title and the buttons */
    do {
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
        at.cursor = hd.HlpCur;
        at.first = hd.HlpFrst;
        hd.HlpFrst = 0xFFF0;            /* nothing is shown yet */

        /* Help10: the cursor within the window */
        do {
            if (at.cursor < at.first)
                at.first = at.cursor;
            lines = (size >> 8) - 9;
            if (at.cursor >= (u16)(lines + at.first))  /* wraps, marked */
                at.first = at.cursor - lines + 1;
            was = hd.HlpCur;
            hd.HlpCur = (u16)at.cursor; /* both of 16 bits, by the cuts */
            shown = hd.HlpFrst;
            hd.HlpFrst = (u16)at.first;
            moved = at.first - shown;
            if (moved != 0 || (hd.HlpMode == 0 && was != hd.HlpCur)) {
                if (shown != 0xFFF0) {  /* the line that had the cursor */
                    place = (u16)(BEGIN + ((was - shown) << 8) + AT_TEXT);
                    Help77(place, Help72(was));
                    Window(place, 0x100 | WIDTH);
                }
                if (moved != 0) {       /* the lines */
                    text = Help72(hd.HlpFrst);
                    place = BEGIN + AT_TEXT;
                    count = (u8)((size >> 8) - 9);
                    do {
                        /* Behind the text there are only empty lines. How
                           far into it the line is wraps at 16 bits, as the
                           original's difference of two offsets, marked: it
                           is never below 0, even for a text of no bytes,
                           since Help77 answers at least a byte behind what
                           it is handed, which is at most a byte in front. */
                        if ((u16)(text - hd.HlpInfo) >= hd.HlpSize)
                            text--;
                        text = Help77(place, text);
                        place += 0x100;
                    } while (--count);
                    /* On the screen: all of them, or the screen moved by a
                       line and the one that is new. */
                    place = BEGIN + AT_TEXT;
                    count = SHOWN();
                    if (moved == 1 || moved == -1) {
                        key = moved == 1 ? 0x600 | (count - 1 & 0xFF) : 0x700;
                        Scroll(key >> 8 == 7, Color(Help_C), place,
                               (u16)(((count - 1 & 0xFF) << 8 | WIDTH - 1)
                                     + place));
                        place += key << 8;
                        count = 1;
                    }
                    Window(place, (u16)(count << 8 | WIDTH));
                }
                if (hd.HlpMode == 0) {  /* the cursor */
                    place = (u16)(BEGIN + ((hd.HlpCur - hd.HlpFrst) << 8)
                                  + AT_TEXT);
                    MvsColr(BasAdr(place), WIDTH, Color(HlpRe_C));
                    Window(place, 0x100 | WIDTH);
                }
            }

            go = Help25(size, &item, &at);
            switch (go) {
            case HELP_UP:               /* Help50: Up */
                at.cursor = hd.HlpCur;
                at.first = hd.HlpFrst;
                count = hd.HlpMode != 0 ? at.first : at.cursor;
                if (count != 0)
                    at.cursor = count - 1;
                break;
            case HELP_DOWN:             /* Help53: Down */
                at.cursor = hd.HlpCur;
                at.first = hd.HlpFrst;
                count = (u16)(SHOWN() + at.first);      /* both wrap, marked */
                if (hd.HlpMode == 0)
                    count = (u16)(at.cursor + 1);
                if (count < hd.HlpLins)
                    at.cursor = count;
                break;
            case HELP_ENTER:            /* Help60 */
                go = Help60(item, &at);
                break;
            }
        } while (go == HELP_SHOW || go == HELP_UP || go == HELP_DOWN);
    } while (go == HELP_PAGE);

    if (go == HELP_FAILED)              /* Help65: it could not be read */
        Dialog(&HlpEr_B, 0, 0, 0, &md);

    /* Help66: Esc */
    ResWin(BEGIN, size, md.ErrBuf1);
    Window(BEGIN, size);
}

/* Read the header of the help file that HelpFil has opened, where its
   index is, the index and the page that HlpPage names: true where the
   file is not a help file of this version or cannot be read. */
static bool HelpBad(void)
{
    i32 i;

    if (SeekDsk(0, 8) || ReadDsk(md.HelpBuf + 16, LenHead))
        return true;
    for (i = 0; i < LenHead; i++)
        if (md.HelpBuf[16 + i] != Help_F[HlpHead + i])
            return true;
    if (SeekDsk(0, 0x194) || ReadDsk(HD_BYTES(HlpOffsLo), 4))
        return true;
    DeCode(HD_BYTES(HlpOffsLo), 4);
    hd.HlpBkLn = 0;
    if (RdEntry())                      /* the index */
        return true;
    if (md.HlpPage != 0 && hd.HlpMode == 0) {
        Help70(md.HlpPage - 1);
        if (RdEntry())
            return true;
    }
    return false;
}

/* Open the help file and show the page that HlpPage names, or the index;
   say so if the file is not a help file of this version or cannot be
   read. Without the file there is no help, and nothing is said. */
static void HelpFil(void)
{
    struct Answer opened;

    HidCur();
    MainFil(md.HelpBuf, Help_F);
    if (ChgDisk(md.HelpBuf))
        return;
    opened = OpenFil(md.HelpBuf, 0x40);
    if (opened.failed)
        return;
    hd.HlpHndl = opened.value;
    if (HelpBad())
        Dialog(&HlpEr_B, 0, 0, 0, &md); /* Help01 */
    else
        Help05();
    FileClose(hd.HlpHndl);              /* Help67 */
}


/* The help, for F1: on the page that HlpPage names. While it is shown
   DOS's critical errors bring up no box, and F1 does nothing. */
void Help00(void)
{
    u16 place = GetCur();
    u8 page = md.HlpPage;
    u8 errors = md.BlocEr;

    md.BlocEr = 1;
    if (page != 0xFF)
        HelpFil();
    md.HlpPage = page;
    md.BlocEr = errors;
    Cursor(place);
}
