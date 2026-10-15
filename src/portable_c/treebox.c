/*
 * The tree of directories: reading it from the disk, keeping it in a file
 * there, and the box in which a directory is chosen from it.
 *
 * Translated from VCSUB3.INC.
 *
 * The tree is the list in TreeBuf that tree.c describes. It is kept
 * in the file TREEINFO.NCD in the root of its disk, which is TreeBuf as it
 * is in the memory: five bytes that say what the file is, the number of
 * directories, the sum of those seven bytes, the list, and the sum of the
 * bytes of the list. A file that is not like that, or is not there, has
 * the disk read instead, directory by directory, and the file written, if
 * there are five entries or more; with fewer the file is deleted.
 *
 * While the disk is read the first two bytes of TreeBuf, which the file
 * has its name in, are used for other things: the first says how the
 * reading ended, the second on which level the directory is that is being
 * read. And every entry has in the last byte of its name whether its own
 * directories are still to be read.
 */
#include "vc.h"

/* ReDir of read.c calls ReadTre, and the tree in its box and Copy and
   Rename call ChTree: both return what the routines answered in the
   carry flag. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte. SavWin keeps what the screen buffer has where
   a window is to be, ResWin puts it back; MinBox draws a box with a shadow
   there, growing if windows are to zoom. Scroll moves a window of the
   screen by a line, with the mouse pointer out of the way.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo is asked
   about a click on the line `line` of a list: 2 for the first click on
   it, 1 for the second, 0 for a second one that ended elsewhere. ClrKbd
   empties the keyboard buffer if the setup has it so.

   PgUp and PgDn turn a list by a page of `lines`: they return the line of
   the cursor and the first line shown.

   ChkSum adds up `length` bytes at `at` in the main data. OncZoom
   puts the box `box` on the screen, as the assembly describes it, with
   its middle at `place`; only the first box since ZoomOne was cleared
   grows. */

/* The tables of the tree, in treebox_t.c: the name of the tree
   file, which DelTr0 has too, and the boxes that are shown while the
   disk is read and while the file is written, which OncZoom takes a
   pointer to. */
extern const u8 Tree_F[];               /* "\TREEINFO.NCD" */
extern const struct BOXTOP Tree_P;      /* "Scanning the directory..." */
extern const struct BOXTOP TrSav_P;     /* "Saving the directory..." */

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define Box_C   16                      /* the colour of a box that reports */
#define Conf_C  19                      /* the colours of a box that asks: */
#define CnfRe_C 20                      /* its text, reversed, */
#define CnfBo_C 21                      /* bold */
#define CnfUn_C 22                      /* and underlined */

#define TREE ((struct DIRTREE *)md.TreeBuf)
#define LIST TRELIST

/* What the reading of the disk keeps where the file has its name. */
#define TreeErr TreHead[0]              /* how it ended: see below */
#define TreeLev TreHead[1]              /* the level of the directory read */
#define READ_FULL    0                  /* all of it, or no room for more */
#define READ_NO_DISK 1                  /* the drive is not ready */
#define READ_STOPPED 2                  /* a key stopped it */

/* Not 0 in an entry whose own directories are still to be read: the last
   byte of its name, behind twelve characters. */
#define DirRecu DirName[12]

/* The path whose directories are asked for, with the mask behind it. */
#define SCANNED (md.MenuBuf + 40)

/* What a tree file begins with. */
static const u8 TreBeg[] = "PNCI";

/* The colours of the box that counts the directories read, and of the
   box in which one is chosen. */
static const u8 Scan_P[] = { Box_C, 0, 0, 0 };
static const u8 ChTr_P[] = { Conf_C, CnfRe_C, CnfBo_C, CnfUn_C };

static const u8 ChTr0_S[] = " Choose Directory ";
/* Two lines, one behind the other: what the line of the path says until
   the tree is there, and the line of the speed search. */
static const u8 ChTr1_S[] =
    "Reading directory tree...\0Speed search: ^r\xFF\x0C";
static const u8 Empty_S[] = "";

#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on the
                                           left and on the right */

/* Where the speed search keeps its text in MenuBuf, and in the word
   before it the column of its cursor. */
#define SPEED 230

/* Where the path of the cursor is kept while the box is open. */
#define KEPT 250

/* RdTr40, where ReadTre ends: the cursor of the tree on the current
   directory, and whether the drive was no longer ready. */
static bool RdTr40(void)
{
    TreeCur();
    return TREE->TreeErr == READ_NO_DISK;
}

/* RdTr01: no disk, and the tree is the root alone. */
static bool RdTr01(void)
{
    struct DIRENT *entry = LIST;
    i32 i;

    for (i = 0; i < (i32)sizeof *entry; i++)
        ((u8 *)entry)[i] = 0;
    entry->DirName[0] = '\\';
    entry->DirShift = 1;
    TREE->TreeLen = 1;
    TREE->TreeErr = READ_FULL;
    return RdTr40();
}

/* Of RdTr02: whether the tree file that has been read into TreeBuf,
   `done` bytes of it, is right. Where it is not, the disk is read. */
static bool TreeFileRight(i32 done)
{
    i32 count, i;

    if (done >= (i32)sizeof md.TreeBuf)         /* longer than there is room
                                                   for */
        return false;
    for (i = 0; i < 5; i++)
        if (md.TreeBuf[i] != TreBeg[i])
            return false;
    if (ChkSum(md.TreeBuf, 7) != TREE->TreSum1)
        return false;
    /* A file that says it has no entry, or 4,096 or more, is taken for a
       broken one, and the disk is read. The original counts the bytes of
       the list in 16 bits, so it holds a file of 4,096 entries or more
       to the low twelve bits of its count, and takes one of none: it
       then shows the entries that the count says, of TreeBuf and of what
       lies behind it, or an empty tree. A bug of the original, fixed on
       the user's word (DEVIATIONS.md, 21). */
    if (TREE->TreeLen == 0)
        return false;
    count = TREE->TreeLen << 4;
    if (done - 11 != count)             /* beyond TreeBuf for 4,096 */
        return false;
    return ChkSum((const u8 *)TRELIST, (u16)count)      /* in TreeBuf */
           == *(const u16 *)((const u8 *)TRELIST + count);
}

/* RdTr18: the reading ends as it is, with `error`, and no entry has
   directories still to be read. */
static void RdTr18(i32 error)
{
    u8 *end = md.TreeBuf + sizeof md.TreeBuf;           /* BP */
    u8 *text;

    TREE->TreeErr = (u8)error;
    /* The byte of every entry from the root to the end of TreeBuf: the
       main data as bytes. */
    text = (u8 *)LIST + offsetof(struct DIRENT, DirRecu);
    do {
        *text = 0;
        text += sizeof(struct DIRENT);
    } while (text < end);
}

/* RdTr10: the directories of the disk read into the tree, with a box
   that counts them from the fifth on, `*box`, which OncZoom puts on the
   screen. The reading ends at RdTr26 of ReadTre. */
static void RdTr10(struct Box *box)
{
    struct LineArgs line;
    struct DIRENT *entry;               /* DI: where the next one goes */
    u8 *end = md.TreeBuf + sizeof md.TreeBuf;           /* BP */
    u8 *text;
    struct Answer found;
    i32 count, i;
    bool first;

    for (i = 0; i < (i32)sizeof md.TreeBuf; i++)
        md.TreeBuf[i] = 0;
    md.ZoomOne = 0;
    SCANNED[0] = '\\';
    SCANNED[1] = '*';
    SCANNED[2] = '.';
    SCANNED[3] = '*';
    SCANNED[4] = 0;
    LIST->DirName[0] = '\\';
    LIST->DirShift = 1;
    entry = LIST + 1;

    for (;;) {                          /* RdTr11: one more: with five, a
                                           box that counts them */
        count = ++TREE->TreeLen;
        if (count >= 5) {
            if (count == 5) {
                *(u16 *)md.MenuBuf = md.DosPth[0];  /* and the end */
                OncZoom(0x728, &Tree_P, 0, box);
            }
            Number(md.MenuBuf, 0, (u16)count, NUM_SEPARATED);  /* at most
                                                   the list's length */
            line.value = (box->begin + 0x400 & 0xFF00) | 0x28;
            line.line = md.MenuBuf;
            line.colors = Scan_P;
            line.include = Empty_S;
            CntLine(&line);
            Window(box->begin + 0x400, 0x100 | (box->size & 0xFF));
        }
        if (TREE->TreeLen == 1) {       /* RdTr15: the first */
            SetDTA();
            first = true;
        } else                          /* RdTr14: the next entry of the
                                           directory */
            first = false;

        for (;;) {                      /* RdTr16 */
            if (TstBrk()) {
                RdTr18(READ_STOPPED);
                return;
            }
            if (first)
                found = FindFirst(SCANNED, 0x37);
            else
                found = FindNext();
            if (found.failed) {
                if (found.value == 0x53) {
                    RdTr18(READ_NO_DISK);
                    return;
                }
                /* RdTr22: no more in this directory: on with the first one
                   found in it, or else the one behind. */
                for (;;) {
                    if (entry[-1].DirRecu == 0) {
                        if (entry->DirRecu == 0)
                            return;     /* RdTr26 */
                    } else {
                        do
                            entry--;
                        while (entry[-1].DirRecu != 0);
                    }
                    entry->DirRecu = 0;
                    TREE->TreeLev = entry->DirLevel;
                    /* Its name behind the backslash of its level, which is
                       looked for in 20 bytes at a time, and the mask
                       behind the name. */
                    text = SCANNED;
                    count = entry->DirLevel;
                    do {
                        for (i = 20; i; i--)
                            if (*text++ == '\\')
                                break;
                    } while (--count);
                    text = MvzLine(text, entry->DirName, 13);
                    text[0] = '\\';
                    text[1] = '*';
                    text[2] = '.';
                    text[3] = '*';
                    text[4] = 0;
                    entry++;
                    /* Too long a path is not read: the next. */
                    if (!(text + 1 > SCANNED + LenPath))
                        break;
                }
                SetDTA();               /* RdTr15: its first */
                first = true;
                continue;
            }
            if ((md.DTA.FilAttr & ATTR_DIRECTORY) == 0
                || (md.HidDirs == 0 && (md.DTA.FilAttr & ATTR_HIDDEN) != 0)
                || md.DTA.FilName[0] == '.') {
                first = false;          /* RdTr14 */
                continue;
            }
            if ((u16)(sizeof md.TreeBuf - sizeof(struct DIRTREE)
                      - sizeof(struct DIRENT)) >> 4 > TREE->TreeLen)
                break;                  /* RdTr20 */
            Beep();                     /* no room for it */
            RdTr18(READ_FULL);
            return;
        }

        /* RdTr20: a directory: its entry goes in here, before what
           follows. */
        if ((u8 *)entry < end - sizeof *entry)
            for (text = end - 1; text >= (u8 *)(entry + 1); text--)
                *text = text[-(int)sizeof *entry];
        for (i = 0; i < 12; i++)
            entry->DirName[i] = md.DTA.FilName[i];
        FilZero(entry->DirName, 12);
        entry->DirRecu = 0xFF;
        entry->DirLevel = TREE->TreeLev + 1;
        if (entry->DirLevel == entry[-1].DirLevel) {
            entry[-1].DirCont = 1;
            entry->DirShift = 0;
        } else {
            entry->DirShift = 1;
        }
        entry->DirCont = 0;
        entry++;
    }
}

/* Read the tree of the current disk: from its file, or from the disk if
   the file is not there or not right, and then write the file. The cursor
   of the tree is put on the current directory.

   Esc, F10 or Ctrl+C while the disk is read stops that: the tree is what
   has been read, and no file is written. So it is when the tree has no
   room for another directory, and when the drive is no longer ready: only
   then is what this returns not 0. A drive that is not ready to begin
   with, the disk of a drive that stands for two not put in, or a file
   that cannot be read leave a tree of the root alone, and that returns
   0. */
bool ReadTre(void)
{
    struct Box box;
    struct Answer opened;
    struct Moved moved;
    struct FileAttr had;
    i32 count, size, i;

    box.begin = 0;
    box.size = 0;

    if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
        return RdTr01();
    opened = OpenFil(Tree_F, 0x40);
    if (opened.failed) {
        if (opened.value == 0x53)
            return RdTr01();
    } else {                            /* RdTr02: the file */
        moved = FileRead(opened.value, md.TreeBuf, sizeof md.TreeBuf);
        if (FileClose(opened.value).failed)
            return RdTr01();
        if (moved.failed)
            return RdTr01();
        if (TreeFileRight(moved.count))
            return RdTr40();
    }

    RdTr10(&box);                       /* the disk */

    /* RdTr26 */
    if (TREE->TreeLen >= 5) {
        ResWin(box.begin, box.size, md.ErrBuf);
        Window(box.begin, box.size);
    }
    if (TREE->TreeErr != READ_FULL)
        return RdTr40();
    if (TREE->TreeLen < 5) {            /* too few for a file */
        FileDelete(Tree_F);
        return RdTr40();
    }
    if (TstBrk())
        return RdTr40();

    /* The file. */
    OncZoom(0x728, &TrSav_P, 0, &box);
    for (i = 0; i < 5; i++)
        md.TreeBuf[i] = TreBeg[i];
    count = TREE->TreeLen << 4;
    TREE->TreSum1 = ChkSum(md.TreeBuf, 7);
    *(u16 *)((u8 *)TRELIST + count) = ChkSum((const u8 *)TRELIST,
                                             (u16)count);   /* in TreeBuf */
    size = count + 11;
    had = FileAttributes(Tree_F);       /* with the attributes it has */
    if (had.failed && had.value == 2)   /* none yet */
        had.attributes = 0;
    if (!had.failed || had.value == 2) {
        md.BlocEr = 1;
        opened = FileCreate(Tree_F, had.attributes | 0x20);
        if (opened.failed) {
            if (opened.value == 5)
                Beep();
        } else {
            moved = FileWrite(opened.value, md.TreeBuf, size);  /* in
                                                   TreeBuf */
            if (!FileClose(opened.value).failed && !moved.failed
                && moved.count != size)
                Beep();
        }
    }

    /* RdTr37 */
    md.BlocEr = 0;
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    return RdTr40();
}

/* The address of the entry with a number: NumAd1 of the assembly. An
   address is an offset in the list here, in bytes, TREE_OFFSET of vc.h,
   where the assembly has a place of the main data. The box reckons with
   them as numbers with a sign, where the original has addresses of 16
   bits that wrap round: a line that has moved up by a line is at an
   offset below 0, which the original has as one that has wrapped round,
   and all of it is differences and comparisons within the list. The
   number of an entry, TreNum, is the pointer's own, and a difference of
   two pointers into the list, which is in TreeBuf of the main data,
   shorter than 64K: an i32 holds it. */
#define ADDRESS(number) ((number) << 4)
#define NUMBER(entry) ((i32)((entry) - TRELIST))

/* Show a line of the list in the box that begins at `begin`: the one
   that is `offset` bytes of the list behind the first one shown, which
   is -16 for the line above it. */
static void ChTr50(u16 begin, i32 offset)
{
    i32 row = (begin >> 8) + (offset >> 4);

    Window((u16)((row << 8 | (begin & 0xFF)) + 0x205), 0x126);    /* in the
                                                   box, on the screen */
}

/* Where the table of the keys stands in the original, behind its entries
   for F2 and for Ctrl+R: see ChTr55 below. */
#define AFTER_F2     0xE45A
#define AFTER_CTRL_R 0xE45E

/* ChTr12: the cursor of the tree at the entry `cursor` and the first
   line shown at `first`, both addresses, within the lines of the box
   that begins at `begin` and has the size `size`, and what has changed
   of them shown. The box goes on by TreeCursor and TreeFirst. */
static void ChTr12(u16 begin, u16 size, i32 cursor, i32 first)
{
    u16 *screen;
    u16 place, inner, cell;
    i32 was, shown, moved, row, count, n, i;

    if (cursor < first)
        first = cursor;
    n = ((size >> 8) - 9) << 4;
    if (cursor - first > n)
        first = cursor - n;
    was = TREE_OFFSET(TreeCursor);
    shown = TREE_OFFSET(TreeFirst);
    moved = first - shown;
    TreeCursor = TREE_ENTRY(cursor);
    TreeFirst = TREE_ENTRY(first);
    if (moved == 0 && was == cursor)
        return;                         /* to ChTr20 */

    /* The lines. */
    screen = BasAdr(begin) + 165;
    cell = (u16)(Color(ChTr_P[0]) << 8 | ' ');
    count = (size >> 8) - 8;
    do {
        for (i = 0; i < 38; i++)
            screen[i] = cell;
        screen += 80;
    } while (--count);
    PutTree(CurrentPanel, begin + 0x104, size - 0x40A, ChTr_P);
    place = begin + 0x205;
    inner = size - 0x80C;
    if (moved != 0 && moved != (i32)sizeof(struct DIRENT)
        && moved != -(i32)sizeof(struct DIRENT)) {
        Window(place, inner);           /* on the screen: all of them */
    } else {
        if (moved != 0) {
            /* On the screen: the screen moved by a line, and the one that
               is new. */
            if (moved == (i32)sizeof(struct DIRENT))
                row = (inner >> 8) - 1; /* the line that is new */
            else
                row = 0;
            was -= moved;               /* the line that had the cursor has
                                           moved too */
            Scroll(moved < 0, Color(ChTr_P[0]), place, inner + place - 0x101);
            ChTr50(begin, row << 4);
        }
        ChTr50(begin, was - shown);     /* the line that had the cursor */
        ChTr50(begin, TREE_OFFSET(TreeCursor) - TREE_OFFSET(TreeFirst));  /*
                                           and the one that has */
    }
    /* ChTr19: the path of the cursor */
    Window(begin + (size & 0xFF00) - 0x500 + 5, 0x126);
}

/* ChTr11: as ChTr12, and the speed search begins anew where the cursor
   is to go elsewhere. */
static void ChTr11(u16 begin, u16 size, i32 cursor, i32 first)
{
    if (cursor != TREE_OFFSET(TreeCursor)) {
        *(u16 *)(md.MenuBuf + SPEED - 2) = 0;
        md.MenuBuf[SPEED] = 0;
    }
    ChTr12(begin, size, cursor, first);
}

/* ChTr10: as ChTr11, to the entries with the numbers `cursor` and
   `first`. */
static void ChTr10(u16 begin, u16 size, i32 cursor, i32 first)
{
    ChTr11(begin, size, ADDRESS(cursor), ADDRESS(first));
}

/* ChTr46, Esc, where ChTree ends: the current directory as it was, the
   box off the screen, and 1 unless `key` is Enter. */
static bool ChTr46(u16 begin, u16 size, u16 key)
{
    i32 i;

    HidCur();
    for (i = 0; i < LenPath; i++)
        md.CrnPath[i] = md.MenuBuf[KEPT + i];
    ResWin(begin, size, md.WinBuf);
    Window(begin, size);
    return (key & 0xFF) != 0x0D;
}

/* ChTr45, Enter: the path of the cursor to TmpBuf, and ChTr46. */
static bool ChTr45(u16 begin, u16 size, u16 key)
{
    i32 i;

    for (i = 0; i < LenPath; i++)
        md.TmpBuf[i] = md.CrnPath[i];
    md.TmpBuf[LenPath] = 0;
    return ChTr46(begin, size, key);
}

/* Show the tree of the current disk in a box that begins in the column
   `column`, of which only the low byte counts, and have a directory
   chosen in it: with the keys that move the cursor, by typing how its
   name begins, or with the mouse. F2 and Ctrl+R read the disk again.
   Returns 0 for Enter, with the path of the directory in TmpBuf, and 1
   if the box was left otherwise.

   The cursor and the first line shown are those of the tree, TreeCursor
   and TreeFirst, which a panel that shows the tree has too: such a panel
   is marked as changed.

   Up, ChTr35, and Down, ChTr37, which the keys and the mouse go to, are
   a line each where they are: they go to ChTr10 with the cursor a line
   further, or to ChTr20 where there is none. */
bool ChTree(i32 column)
{
    struct LineArgs line;
    u16 begin = 0x200 | (column & 0xFF);                  /* BX */
    u16 size = (u16)((u8)(md.Lines - 3) << 8 | 50);     /* DX */
    u16 color;                          /* AX: the colour in the high byte */
    i32 cursor, first;                  /* BP and DI: where to go, as
                                           numbers of entries from the keys
                                           and as addresses from ChTr11 */
    u16 key = 0;
    u16 path = 0;                       /* SI at ChTr55 */
    u16 state, mouse, held;             /* SI, CX and BP for the mouse */
    u16 before;                         /* what state was */
    u8 look;
    u16 place, inner, cell;
    i32 count, click, n, i;
    i32 lines, under;
    u16 *screen;
    bool mousing;                       /* a button of the mouse is down:
                                           ChTr60 */
    struct Paged paged;
    struct Mouse pointer;

    HidCur();
    SavWin(begin, size, md.WinBuf);
    color = (u16)(Color(ChTr_P[0]) << 8 | (column & 0xFF));
    MinBox(begin, size, color);
    if (LeftPanel->WinTyp == 1)
        LeftPanel->ChgFlg = 1;
    if (RightPanel->WinTyp == 1)
        RightPanel->ChgFlg = 1;
    for (i = 0; i < LenPath; i++)
        md.MenuBuf[KEPT + i] = md.CrnPath[i];

    for (;;) {                          /* ChTr59: the box, and the tree
                                           read */
        line.value = begin + 0x118;
        line.line = ChTr0_S;
        line.colors = ChTr_P;
        line.include = Empty_S;
        CntLine(&line);
        screen = BoxLine(BasAdr(begin + (size & 0xFF00) - 0x600),
                         (color & 0xFF00) | LINE, LEFT_RIGHT, 3,
                         (u8)(size - 10));
        line.value = 38;
        line.screen = screen + 5;
        line.line = ChTr1_S;
        NgLine(&line);
        line.screen = screen + 5 + 80;
        NgLine(&line);
        Window(begin, size);
        md.CrnPath[0] = 0;
        ReadTre();
        TrCr20((size >> 8) - 8);
        lines = (size >> 8) - 8;
        n = TREE->TreeLen - lines;
        if (TREE->TreeLen < lines)
            n = 0;
        n = ADDRESS(n);
        if (n < TREE_OFFSET(TreeFirst))
            TreeFirst = TREE_ENTRY(n);
        ((u16 *)&md.MousTim)[1] = 0xFFFF;
        md.MousEnt = 0;
        /* Nothing is shown yet: the screen is taken to be elsewhere. */
        cursor = TREE_OFFSET(TreeCursor);
        first = TREE_OFFSET(TreeFirst);
        TreeCursor += 2;
        TreeFirst += 2;
        ChTr11(begin, size, cursor, first);

        for (;;) {                      /* ChTr20: the speed search, and a
                                           key */
            place = begin + (size & 0xFF00) - 0x400 + 19;
            if (md.WinMous == 0)
                InSt10(place, md.MenuBuf + SPEED, 12, ChTr_P[1] << 8);
            Cursor(place + *(u16 *)(md.MenuBuf + SPEED - 2));
            cursor = NUMBER(TreeCursor);
            first = NUMBER(TreeFirst);
            mousing = GetMous(0).buttons != 0;
            if (!mousing) {
                md.MousEnt = 0;
                md.WinMous = 0;
                key = Input(14);
                mousing = key >> 8 == 0xFC;
            }
            if (mousing) {
                /* ChTr60: the mouse, while a button is down. Each way out
                   of the loop goes on at ChTr20. */
                state = 0;
                mouse = TREE->TreeLen;
                held = (u16)cursor;     /* BP: read as a place only once
                                           a button has been down */
                for (;;) {              /* ChTr61 */
                    pointer = GetMous(mouse);
                    mouse = pointer.place;
                    if (pointer.buttons == 0) {
                        /* ChTr65: let go */
                        TypMous(0);
                        key = state;
                        if ((key & 0xFF) == 2)  /* the right button */
                            break;
                        if ((key & 0xFF) < 1)
                            break;
                        if ((key & 0xFF) != 1)  /* more than one, or the
                                                   middle */
                            return ChTr46(begin, size, key);
                        if (md.WinMous != 0)
                            break;
                        if (InWind(held, begin, size - 0x102))  /* outside
                                                   the box */
                            return ChTr46(begin, size, key);
                        break;
                    }
                    held = mouse;
                    ClrKbd();
                    look = 'x';         /* the middle button alone */
                    if (pointer.buttons & 3)
                        look = 0;
                    before = state;
                    state = (u16)(look << 8 | pointer.buttons);  /* three
                                                   bits of buttons */
                    if (look != before >> 8)
                        TypMous(look);
                    if ((state & 3) == 0)
                        continue;
                    if (!InWind(held, begin + 0x204, size - 0x80A)) {
                        /* ChTr67: on a line */
                        md.WinMous = 1;
                        under = (u8)((u16)(held - begin) >> 8) - 2;
                        click = DoublMo((u16)(begin + 0x204 + (under << 8)), /* on the
                                                   screen, by InWind */
                                        0x100 | (u8)(size - 10), under);
                        if (click == 2) {   /* the cursor goes there */
                            under += first;
                            /* the list has the root */
                            cursor = TREE->TreeLen - 1;
                            if (cursor > under)
                                cursor = under;
                            ChTr10(begin, size, cursor, first);
                            break;
                        }
                        if (click == 0)
                            break;
                        TypMous(0);     /* a second click: Enter */
                        return ChTr45(begin, size, 0x1C0D);
                    }
                    /* Not on a line. If the button went down on one, the
                       cursor goes up while the mouse is above the lines
                       and down while it is below. */
                    if (md.WinMous == 0)
                        continue;
                    cursor = first;
                    if ((u8)(held >> 8) < (u8)((begin >> 8) + 2)) {
                        if (cursor != 0)                /* ChTr35 */
                            ChTr10(begin, size, cursor - 1, first);
                        break;
                    }
                    if ((u8)(held >> 8) < (u8)((begin >> 8) + (size >> 8)
                                                - 6))
                        continue;
                    cursor += (size >> 8) - 9;
                    if (cursor + 1 < TREE->TreeLen)     /* ChTr37 */
                        ChTr10(begin, size, cursor + 1, first);
                    break;
                }
                continue;
            }
            if (key >> 8 != 0) {        /* Case takes none without a scan
                                           code */
                if (key == 0x3C00) {    /* F2 */
                    path = AFTER_F2;
                    break;
                }
                if ((key & 0xFF) == 0x12) {     /* Ctrl+R */
                    path = AFTER_CTRL_R;
                    break;
                }
                if ((key & 0xFF) == 0x0D)
                    return ChTr45(begin, size, key);
                if ((key & 0xFF) == 0x1B)
                    return ChTr46(begin, size, key);
                if ((key & 0xFF) == 0) {
                    switch (key >> 8) {
                    case 0x48:          /* Up: ChTr35 */
                        if (cursor != 0)
                            ChTr10(begin, size, cursor - 1, first);
                        continue;
                    case 0x50:          /* Down: ChTr37 */
                        if (cursor + 1 < TREE->TreeLen)
                            ChTr10(begin, size, cursor + 1, first);
                        continue;
                    case 0x49:          /* PgUp */
                        paged = PgUp(cursor, first, (size >> 8) - 8);
                        ChTr10(begin, size, paged.cursor, paged.first);
                        continue;
                    case 0x51:          /* PgDn */
                        paged = PgDn(cursor, first, (size >> 8) - 8,
                                     TREE->TreeLen);
                        ChTr10(begin, size, paged.cursor, paged.first);
                        continue;
                    case 0x47:          /* Home */
                        ChTr10(begin, size, 0, first);
                        continue;
                    case 0x4F:          /* End, as Up from behind the last */
                        cursor = TREE->TreeLen;
                        if (cursor != 0)
                            ChTr10(begin, size, cursor - 1, first);
                        continue;
                    case 0x44:          /* F10 */
                        return ChTr46(begin, size, key);
                    }
                }
            }
            /* Another key is for the speed search. */
            n = Search((const u8 *)TRELIST, TREE_OFFSET(TreeCursor), 0,
                       (u16)ADDRESS(TREE->TreeLen),     /* the list, in
                                                           TreeBuf */
                       sizeof(struct DIRENT), key);
            if (n != TREE_OFFSET(TreeCursor))
                ChTr12(begin, size, n, TREE_OFFSET(TreeFirst));
        }

        /* ChTr55: the disk read again. The assembly comes here from Case
           with SI behind the entry of the key in its table of keys, an
           address in the code segment, and DelTr0 takes SI for the
           address of a path in the main data: the tree file it spoils is
           that of the drive named there, if the second byte there is a
           colon. In the original those two bytes are in TreeBuf, in the
           name of its 510th entry: the second and third character for F2,
           the sixth and seventh for Ctrl+R. No name that DOS gives has a
           colon, so it is the current drive unless a tree file has such a
           name. The addresses are the original's here, since the table is
           not in the C. */
        place = begin + 0x205;
        inner = size - 0x80C;
        cell = (u16)(Color(ChTr_P[0]) << 8 | ' ');
        count = inner >> 8;
        do {
            screen = BasAdr(place);
            for (i = inner & 0xFF; i; i--)
                *screen++ = cell;
            place += 0x100;
        } while (--count);
        DelTr0(MD_POINTER(path));
    }
}
