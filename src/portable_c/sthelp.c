/*
 * The help of the setup: its pages read from the help file when the
 * setup is entered, and one of them in a window, with the keys and the
 * mouse that move its text.
 *
 * Translated from VCSTHLP.INC, of the original's setup program, whose
 * tables are in sthelp_t.c. It is another help than the main program's,
 * help.c: one page in a window with the button Ok, no index to choose
 * from, and the pages all in the memory. A routine here has the name of
 * the assembly with St in front, as everything of the setup program has.
 *
 * The help file is the main program's, VC.HLP in the program's
 * directory, and help.c says how it is laid out. The setup's pages are
 * seven that follow one another in it, from the 55th line of the index
 * on. StRdHelp reads the index, for where the first of them is and where
 * the page behind the last, and then what lies between the two as one
 * piece, or as much of it as there is room for. StHelp finds a page in
 * that piece by going from entry to entry, by the 26 bytes in front of
 * each text, which say how long it is: the pages are numbered from 1 by
 * where they lie, and the index is not asked again.
 *
 * The room. The original reads into HelpBuf, the end of its data, as far
 * as FEE0h of its one segment, below its stack. HelpBuf is at DF03h in
 * the program that was shipped: 100h, the 24,278 bytes of its file, and
 * 7F2Dh of data that the file does not hold. The first word there is how
 * many bytes were read, so the pages have 8,155 bytes, and the seven of
 * the help file that was shipped are 8,628: the sixth and the seventh,
 * Country info and Exit, do not fit, and "Help contents is not
 * available" is what F1 says for them. To show the same, this goes by
 * the same two numbers. It goes by where HelpBuf is, and not only by the
 * length, because the original looks for a page with addresses of 16
 * bits, which go round at the end of the segment: see StHelp.
 *
 * Which page: the menu of the setup and its dialogs say in HlpPage, by
 * the number of the page, from 1; FFh says that the help is on the
 * screen already.
 */
#include "vc.h"

/* The tables: of help_t.c, which the setup program has byte for byte,
   the title of the window and the numbers of the colours of the button;
   of sthelp_t.c the button, for MinMnu, and the box that says there is
   no such page, which Dialog takes a pointer to; and of the setup's
   other tables the text of the button as the window has it. */
extern const u8 Help_S[];               /* " Help " */
extern const u8 HlpMn_P[];
extern const u8 StHlpMn_D[];            /* Ok */
extern const struct BOXHD StHlpEr_B;    /* "Help contents is not available" */
extern const u8 StOk_S[];               /* "[ Ok ]" */

#define Help_C  27                      /* the colours of the help: its text, */
#define HlpBo_C 28                      /* bold, */
#define HlpRe_C 29                      /* reversed, */
#define HlpUn_C 30                      /* and underlined */

#define BegHlp  55                      /* the first page of the setup, as
                                           the index counts, from 1, */
#define NumHlp  7                       /* and how many it has */

/* The name of the help file, and behind it what its header has from the
   ninth byte on. They are one text because MainFil takes 13 bytes for a
   name: the first six of the second are copied with the first, as they
   are in the assembly. */
static const u8 StHelp_F[] = "VC.HLP\0VC 4.05 Help";
#define HlpHead 7                       /* where in it the second begins */
#define LenHead 13                      /* and its length, with its end */

/* The colours of a line of the help, for NgLine, and the line that it is
   to put in for ^I: none. */
static const u8 StHelp_P[] = { Help_C, HlpRe_C, HlpBo_C, HlpUn_C };
static const u8 StEmpty_S[] = "";

#pragma pack(push, 1)

/* The 26 bytes in front of the text of an entry of the help file:
   HlpData of the assembly. The text is behind them, HlpInfo there. */
struct StHlpData {
    u16 HlpMode;                        /* 0 - the index */
    u16 HlpSize;                        /* the length of its text */
    u16 HlpLins;                        /* its lines, with the title */
    u16 Reserved1;
    u16 HlpBkLn;
    u16 HlpOffsLo;                      /* where the index is in the file */
    u16 HlpOffsHi;
    u16 HlpFrst;                        /* the first line shown: the window
                                           keeps it here */
    u16 HlpCur;
    u8  Reserved2[8];
};

#pragma pack(pop)

LAID_OUT(StHlpData, 26);

#define HlpInfo sizeof(struct StHlpData)

/* Where the original has HelpBuf in its segment, and where what it reads
   into it ends: see the top. What is here in place of HelpBuf is as long
   as from there to the end of that segment, so that every address which
   the original can come to in looking for a page is a place in it. */
#define HELPBUF 0xDF03
#define LIMIT   0xFEE0
#define LEN_BUF (0x10000UL - HELPBUF)

#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on the
                                           left and on the right */
#define WIDTH       62                  /* the columns of a line of text */

/* The window: where it begins, the row in the high byte, and from there
   its title and the lines of the text. */
#define BEGIN       0x204
#define AT_TITLE    0x205
#define AT_TEXT     0x405

/* Where the pages are: in place of HelpBuf of the original, LEN_BUF bytes
   that are the setup's for as long as it is open. They are in TreeBuf of
   the main data, the list of a tree, which nothing uses meanwhile: a
   panel that shows the tree reads it anew when the setup has been left,
   as after a command. */
HOLDS(the_pages_of_the_help_fit_TreeBuf,
      LEN_BUF <= sizeof ((struct MainDat *)0)->TreeBuf);

static u8 *StHelpBuf(void)
{
    return md.TreeBuf;
}

/* Go to a place in the help file. Not 0 if DOS would not, or went
   elsewhere. */
static bool SeekDsk(u16 handle, u32 place)
{
    struct FilePlace sought = FileSeek(handle, place);

    if (sought.failed)
        return true;
    return sought.place != place;
}

/* Read `count` bytes of the help file. Not 0 if DOS would not, or read
   fewer. */
static bool ReadDsk(u16 handle, u8 *to, i32 count)
{
    struct Moved read = FileRead(handle, to, count);

    if (read.failed)
        return true;
    return read.count != count;
}

/* What StRdHelp reads of the file that it has opened, as far as a part
   cannot be read or is not what it is to be: the header, the index and
   the setup's pages. The assembly jumps to RdHlp1 for each of those, and
   StRdHelp closes the file there. */
static void StRdPages(u16 handle, u8 *buffer)
{
    struct StHlpData *index = (struct StHlpData *)buffer;
    struct Moved read;
    i32 count, lines, i;
    u32 first, between;

    if (SeekDsk(handle, 8) || ReadDsk(handle, buffer + 16, LenHead))
        return;                         /* the header: */
    for (i = 0; i < LenHead; i++)       /* is it a help file of this */
        if (buffer[16 + i] != StHelp_F[HlpHead + i])    /* version? */
            return;
    if (SeekDsk(handle, 0x194)          /* where the index is */
        || ReadDsk(handle, (u8 *)&index->HlpOffsLo, 4))
        return;
    DeCode((u8 *)&index->HlpOffsLo, 4);
    if (SeekDsk(handle, (u32)index->HlpOffsHi << 16 | index->HlpOffsLo)
        || ReadDsk(handle, buffer, HlpInfo))
        return;                         /* what the index begins with, */
    DeCode(buffer, HlpInfo);
    count = index->HlpSize;             /* and its text, if it fits */
    if (count == 0 || (u16)(LIMIT - HELPBUF) < count)
        return;
    if (ReadDsk(handle, buffer + HlpInfo, count))
        return;
    DeCode(buffer + HlpInfo, (u16)count);
    if (index->HlpMode != 0)
        return;
    lines = index->HlpLins;             /* has it pages for the setup? */
    if (lines <= BegHlp - 1)
        return;
    lines -= BegHlp - 1;
    /* Six bytes for every line of the index, of which the last four are
       where its page is in the file. */
    first = *(u32 *)(buffer + HlpInfo + 2 + 6 * (BegHlp - 1));
    if (SeekDsk(handle, first))
        return;
    /* As much as there is room for: or, if the index has a page behind
       the setup's and that is nearer, as far as there. */
    count = LIMIT - (HELPBUF + 2);
    if (lines > NumHlp) {
        between = *(u32 *)(buffer + HlpInfo + 2
                           + 6 * (BegHlp - 1 + NumHlp)) - first;
        if (between >> 16 == 0 && count > (u16)between)
            count = (u16)between;
        if (count == 0)                 /* the next page is the first */
            return;
    }
    read = FileRead(handle, buffer + 2, count);
    if (read.failed)
        return;
    *(u16 *)buffer = (u16)read.count;   /* below the room's LIMIT */
    /* The mark comes off as many bytes as were asked for, read or not. */
    DeCode(buffer + 2, (u16)count);
}

/* Read the setup's pages of the help file, when the setup is entered:
   into StHelpBuf, how many bytes of them in its first word and the pages
   behind that. If that fails, the setup goes on without them, and
   nothing is said: the original's setup program, which reads them when
   it starts, prints a line through DOS then, HlpEr_S of the assembly,
   "Error reading data from the help file.", and the setup of the one
   program has none of what that program printed at its start (see
   DEVIATIONS.md). From there on the two are the same: F1 shows what
   this memory then gives.

   The index is read into the same memory first, and is still there if
   something then fails: the first word is its mode in that case, 0 for a
   real one, which StHelp takes for the length of the pages. The
   original leaves what its memory held where it has read nothing; here
   that is zeros, so that what F1 then shows does not hang on what was
   in the memory before.

   A count of no bytes is refused as a file that cannot be read: the
   index's text, where its entry says it has none, and the setup's pages,
   where the page of the index behind them begins where they do. The
   original hands its DeCode the 0, which goes round all of its one
   segment, its own code with it (DEVIATIONS.md, 23). */
void StRdHelp(void)
{
    u8 *buffer = StHelpBuf();
    struct Answer answer;
    u16 handle;
    i32 i;

    for (i = 0; i < (i32)LEN_BUF; i++)
        buffer[i] = 0;
    MainFil(md.TmpBuf, StHelp_F);
    answer = FileOpen(md.TmpBuf, 0x40);
    if (answer.failed)
        return;
    handle = answer.value;
    StRdPages(handle, buffer);
    /* RdHlp1: read or not, the file is closed */
    FileClose(handle);
}

/* Write a line of text into the window at a row and a column, over
   spaces: HlpLine of the assembly. Returns where the text of the next
   line begins. */
static const u8 *StHlpLine(u16 place, const u8 *text)
{
    struct LineArgs line;
    u16 cell = (u16)(Color(StHelp_P[0]) << 8 | ' ');
    i32 i;

    line.screen = BasAdr(place);
    for (i = 0; i < WIDTH; i++)
        line.screen[i] = cell;
    md.TmpErr[1] = 0;
    md.TmpErr[101] = 0;
    line.value = WIDTH;
    line.line = text;
    line.colors = StHelp_P;
    line.include = StEmpty_S;
    NgLine(&line);
    return line.line;
}

/* Where the window goes on from a key or the mouse: shown again from the
   line `first`, a line up or down, the next key, or closed. The assembly
   jumps to each of those. */
enum {
    HELP_SHOW,                          /* Help10 */
    HELP_UP,                            /* Help30 */
    HELP_DOWN,                          /* Help32 */
    HELP_KEY,                           /* Help20 */
    HELP_LEAVE                          /* Help40 */
};

/* The page that the help of the setup is to show: as many entries on as
   its number says, none of them beyond what was read. No page, 0, is one
   of 65536. Answers 0 where there is none, which is Help02 of the
   assembly. */
static struct StHlpData *StPage(u8 *buffer, u8 page)
{
    struct StHlpData *entry;            /* BP: the page */
    u16 at, next, end, count;

    count = page;
    end = HELPBUF + 2 + *(u16 *)buffer;
    next = HELPBUF + 2;
    do {
        at = next;
        next = at + HlpInfo;
        if (next < at || next > end)
            return 0;
        entry = (struct StHlpData *)(buffer + (at - HELPBUF));
        if ((u16)(next + entry->HlpSize) < next)
            return 0;
        next += entry->HlpSize;
        if (next > end)
            return 0;
    } while (--count);
    if (entry->HlpMode != 0)
        return entry;                   /* on to Help04 */
    return 0;
}

/* The mouse, from Help22 of the assembly: a button is down, and
   `pointer` has the buttons and where the mouse is. Where it goes on to
   the window, the first line is in `first`. */
static i32 StHelp22(struct StHlpData *entry, u16 size, struct Mouse pointer,
                    i32 *first)
{
    u16 buttons, mouse, held;
    i32 third;

    for (;;) {                          /* Help22 */
        buttons = pointer.buttons;
        mouse = pointer.place;
        ClrKbd();
        held = mouse;
        if (buttons & 4) {              /* the middle button, until it is */
            TypMous('x');               /* let go: closes, unless another */
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
        if (!InWind(mouse, BEGIN + 0x404, size - 0x90A) || md.WinMous != 0)
            break;
        /* Not where the text is, and the button did not go down there:
           the left one closes when it is let go outside the window. */
        if ((buttons & 1) == 0)
            return HELP_KEY;
        pointer = GetMous(mouse);
        if (pointer.buttons == 0) {
            if (!InWind(held, BEGIN, size - 0x102))
                return HELP_KEY;
            return HELP_LEAVE;
        }
    }
    /* In the upper third of the text, or above it: up; in the lower
       third or below it: down. */
    md.WinMous |= 1;
    *first = entry->HlpFrst;
    third = ((size >> 8) - 9) / 3;
    if ((u8)(mouse >> 8) < (u8)((BEGIN >> 8) + 4 + third))
        return HELP_UP;
    if ((u8)(mouse >> 8) >= (u8)((BEGIN >> 8) + (size >> 8) - 5 - third))
        return HELP_DOWN;
    return HELP_KEY;
}

/* A key, Help20 of the assembly, or the mouse, until one of them moves
   the window or closes it. Where it goes on to the window, the first
   line is in `first`. */
static i32 StHelp20(struct StHlpData *entry, u16 size, i32 *first)
{
    struct Mouse pointer;
    u16 key;
    i32 step, last, go;

    for (;;) {                          /* Help20 */
        pointer = GetMous(0);
        if (pointer.buttons == 0) {
            md.WinMous = 0;
            md.MousEnt = 0;
            key = MinMnu(StHlpMn_D, BEGIN + (size & 0xFF00) - 0x400 + 13,
                         0x130, HlpMn_P, 2, 0).key;
            if (key >> 8 != 0xFC) {
                *first = entry->HlpFrst;
                /* Help_T of the assembly, for Case, which takes no key
                   without a scan code. */
                if (key >> 8 == 0)
                    continue;
                if ((key & 0xFF) == 0x0D || (key & 0xFF) == 0x1B)
                    return HELP_LEAVE;  /* Enter, Esc */
                if ((key & 0xFF) != 0)
                    continue;
                switch (key >> 8) {
                case 0x48: return HELP_UP;
                case 0x50: return HELP_DOWN;
                case 0x47:              /* Help35: Home */
                    *first = 0;
                    return HELP_SHOW;
                case 0x4F:              /* End */
                    *first = entry->HlpLins;
                    step = md.Lines - 11;
                    break;
                case 0x49:              /* PgUp */
                    if (*first == 0)
                        return HELP_SHOW;
                    step = md.Lines - 13;
                    break;
                case 0x51:              /* PgDn */
                    step = md.Lines - 12;
                    /* wraps: a page of no lines has 65,535 as the
                       original's word */
                    last = (u16)(entry->HlpLins - 1);
                    if (last < step)
                        return HELP_SHOW;
                    last -= step;
                    if (*first >= last)
                        return HELP_SHOW;
                    *first += step - 1;
                    if (*first > last)
                        *first = last;
                    return HELP_SHOW;
                case 0x44: return HELP_LEAVE;   /* F10 */
                default: continue;
                }
                /* Help34: back by `step` lines, or to the first, as Home
                   goes, Help35 */
                if (*first >= step)
                    *first -= step;
                else
                    *first = 0;
                return HELP_SHOW;
            }
            pointer = GetMous(0);       /* Help21: the mouse */
        }
        go = StHelp22(entry, size, pointer, first);
        if (go != HELP_KEY)
            return go;
    }
}

/* The window with the page, Help04 of the assembly, until a key or the
   mouse closes it. */
static void StHelp04(struct StHlpData *entry, u16 size)
{
    const u8 *lines;                    /* its text, with the title */
    const u8 *text;
    i32 first;                          /* DI: the first line to show */
    i32 shown, left, go;
    u16 count, moved, color, how, where;
    u16 *screen, *bottom;

    lines = (const u8 *)entry + HlpInfo;
    SavWin(BEGIN, size, md.ErrBuf);
    color = (u16)(Color(Help_C) << 8);
    MinBox(BEGIN, size, color);
    screen = BasAdr(BEGIN);
    MvsBw(screen + 80 + 33, Help_S, 6, color);
    bottom = BasAdr(BEGIN + (size & 0xFF00) - 0x500);
    MvsBw(bottom + 80 + 33, StOk_S, 6, color);
    BoxLine(bottom, color | LINE, LEFT_RIGHT, 3, 64);
    BoxLine(screen + 3 * 80, color | LINE, LEFT_RIGHT, 3, 64);
    StHlpLine(BEGIN + AT_TITLE, lines);
    Window(BEGIN, size);
    md.WinMous = 0;
    md.MousEnt = 0;
    md.HlpPage = 0xFF;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;
    entry->HlpFrst = 0xFFF0;            /* nothing is shown yet */
    first = 0;

    /* Help10: the text from the line `first` */
    do {
        shown = entry->HlpFrst;
        entry->HlpFrst = (u16)first;    /* a line of 16 bits: see Help32 */
        moved = (u16)(first - shown);   /* wraps: a difference of two lines
                                           as the original's word has it */
        if (moved != 0) {
            /* Where that line begins: behind the title and the lines
               before it, each of which ends in a zero byte. The end of a
               line is looked for in FF00h bytes and the low byte of the
               count of lines, and taken to be there if it is not. */
            text = lines;
            count = entry->HlpFrst + 1;
            do {
                left = 0xFF00 | (count & 0xFF);
                do
                    left--;
                while (*text++ != 0 && left != 0);
            } while (--count);
            where = BEGIN + AT_TEXT;
            count = (u8)((size >> 8) - 9);
            do {
                /* Behind the text there are only empty lines. */
                if ((u16)(text - lines) >= entry->HlpSize)
                    text--;
                text = StHlpLine(where, text);
                where += 0x100;
            } while (--count);
            /* On the screen: all of them, or the screen moved by a line
               and the one that is new. */
            where = BEGIN + AT_TEXT;
            count = (u8)(md.Lines - 12);
            if (moved == 1 || moved == 0xFFFF) {
                how = moved == 1 ? 0x600 | (count - 1 & 0xFF) : 0x700;
                Scroll(how >> 8 == 7, Color(Help_C), where,
                       (u16)(((count - 1 & 0xFF) << 8 | WIDTH - 1) + where));
                where += how << 8;
                count = 1;
            }
            Window(where, (u16)(count << 8 | WIDTH));
        }

        go = StHelp20(entry, size, &first);
        if (go == HELP_UP) {            /* Help30: Up */
            if (first != 0)
                first--;
        } else if (go == HELP_DOWN) {   /* Help32: Down */
            if ((u16)(md.Lines - 11 + first) < entry->HlpLins)
                first = (u16)(first + 1);       /* wraps: from line 65,535
                                                   to 0, as the original's
                                                   word, where the sum above
                                                   has gone round */
        }
    } while (go != HELP_LEAVE);

    /* Help40: Esc */
    ResWin(BEGIN, size, md.ErrBuf);
    Window(BEGIN, size);
}

/* The help of the setup, for F1: the page that HlpPage names in a
   window, until a key or the mouse closes it; or a box that says there
   is no such page, with a beep. While it is shown F1 does nothing.

   The page is looked for as the original looks for it, with addresses
   of 16 bits that begin where the original has its pages, HELPBUF + 2:
   whether an entry ends beyond what was read, and whether adding the 26
   bytes in front of a text or its length went round at 64 KB. Where
   what was read ends is added up without such a look. With the pages
   read that is all the same; behind a read that failed, where the first
   word is no length, it decides what shows. */
void StHelp(void)
{
    u8 *buffer = StHelpBuf();
    struct StHlpData *entry;            /* BP: the page */
    u16 place = GetCur();
    u8 page = md.HlpPage;
    u16 size = (u16)((u8)(md.Lines - 3) << 8 | 74);

    HidCur();
    if (page != 0xFF) {                 /* else on to Help41 */
        entry = StPage(buffer, page);
        if (entry == 0)
            Dialog(&StHlpEr_B, 0, 0, 0, &md);   /* Help02: there is none */
        else
            StHelp04(entry, size);
    }

    /* Help41 */
    md.HlpPage = page;
    Cursor(place);
}
