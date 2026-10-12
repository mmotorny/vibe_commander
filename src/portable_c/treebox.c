/*
 * The tree of directories: reading it from the disk, keeping it in a file
 * there, and the box in which a directory is chosen from it.
 *
 * Translated from VCSUB3.INC.
 *
 * The tree is the list in TreeBuf that src/c/tree.c describes. It is kept
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

u16 ReadTre(void);
u16 ChTree(u16 column);

/* ReDir of read.c calls ReadTre, and the tree in its box and Copy and
   Rename call ChTree: both return what the routines answered in the
   carry flag. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   HidCur puts the cursor below the screen, Cursor at a row and a column,
   the row in the high byte. SavWin keeps what the screen buffer has where
   a window is to be, ResWin puts it back; MinBox draws a box with a shadow
   there, growing if windows are to zoom. Int10m calls the BIOS for the
   screen, with the mouse pointer out of the way.

   GetMous returns the buttons of the mouse that are down, and in the high
   word where the mouse is, or `place` as it was if there is no mouse.
   TypMous gives the mouse pointer the look of a character, or its own for
   0. InWind is not 0 if a place is outside a window. DoublMo is asked
   about a click on the line `line` of a list: 2 for the first click on
   it, 1 for the second, 0 for a second one that ended elsewhere. ClrKbd
   empties the keyboard buffer if the setup has it so.

   PgUp and PgDn turn a list by a page of `lines`: they return the line of
   the cursor, and in the high word the first line shown.

   ChkSum adds up `length` bytes at `at` in the main data. OncZoom
   puts the box on the screen that the assembly describes at `box`, an
   offset in its code segment, with its middle at `place`; only the first
   box since ZoomOne was cleared grows. */

/* C routines of other files that vc.h does not have. */
void FilZero(u8 __far *text, u16 length);               /* string.c */
void TreeCur(void);                                     /* tree.c */
void TrCr20(u16 lines);

/* The tables of the tree, in src/c/treebox_t.c: the name of the tree
   file, which DelTr0 has too, and the boxes that are shown while the
   disk is read and while the file is written, which OncZoom takes by
   their offsets. */
extern const u8 __far Tree_F[];         /* "\TREEINFO.NCD" */
extern const u8 __far Tree_P[];         /* "Scanning the directory..." */
extern const u8 __far TrSav_P[];        /* "Saving the directory..." */
#pragma aux Tree_F "*"
#pragma aux Tree_P "*"
#pragma aux TrSav_P "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

#define Box_C   16                      /* the colour of a box that reports */
#define Conf_C  19                      /* the colours of a box that asks: */
#define CnfRe_C 20                      /* its text, reversed, */
#define CnfBo_C 21                      /* bold */
#define CnfUn_C 22                      /* and underlined */

#define TREE ((struct DIRTREE *)md.TreeBuf)
#define LIST ((struct DIRENT *)TRELIST)

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
static const u8 CODE TreBeg[] = "PNCI";

/* The colours of the box that counts the directories read, and of the
   box in which one is chosen. */
static const u8 CODE Scan_P[] = { Box_C, 0, 0, 0 };
static const u8 CODE ChTr_P[] = { Conf_C, CnfRe_C, CnfBo_C, CnfUn_C };

static const u8 CODE ChTr0_S[] = " Choose Directory ";
/* Two lines, one behind the other: what the line of the path says until
   the tree is there, and the line of the speed search. */
static const u8 CODE ChTr1_S[] =
    "Reading directory tree...\0Speed search: ^r\xFF\x0C";
static const u8 CODE Empty_S[] = "";

#define LINE        0xC4                /* a single line */
#define LEFT_RIGHT  0xC7B6              /* where one meets the frame, on the
                                           left and on the right */

/* Where the speed search keeps its text in MenuBuf, and in the word
   before it the column of its cursor. */
#define SPEED 230

/* Where the path of the cursor is kept while the box is open. */
#define KEPT 250

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
u16 ReadTre(void)
{
    struct DosRegs regs;
    struct LineArgs line;
    struct Box box;
    struct DIRENT *entry;               /* DI: where the next one goes */
    u8 *end = md.TreeBuf + sizeof md.TreeBuf;           /* BP */
    u8 *text;
    u32 opened;
    u16 count, done, failed, error, size, i;

    box.begin = 0;
    box.size = 0;
    regs.bx = 0;
    regs.cx = 0;
    regs.si = 0;
    regs.di = 0;
    regs.name = md.MenuBuf;

    if (Fantom((u8)(md.DosPth[0] - ('A' - 1))))
        goto RdTr01;
    opened = OpenFil(Tree_F, 0x40);
    if (!HIGH(opened))
        goto RdTr02;
    if (LOW(opened) != 0x53)
        goto RdTr10;

RdTr01:                                 /* no disk: the root alone */
    entry = LIST;
    for (i = 0; i < sizeof *entry; i++)
        ((u8 *)entry)[i] = 0;
    entry->DirName[0] = '\\';
    entry->DirShift = 1;
    TREE->TreeLen = 1;
    TREE->TreeErr = READ_FULL;
    goto RdTr40;

RdTr02:                                 /* the file */
    regs.ax = 0x3F00 | LOW(opened) & 0xFF;
    regs.bx = LOW(opened);
    regs.cx = sizeof md.TreeBuf;
    regs.name = md.TreeBuf;
    failed = Intr21((struct DosRegs STACK *)&regs);
    done = regs.ax;
    regs.ax = 0x3E00 | done & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs))
        goto RdTr01;
    if (failed)
        goto RdTr01;
    if (done >= sizeof md.TreeBuf)      /* longer than there is room for */
        goto RdTr10;
    for (i = 0; i < 5; i++)
        if (md.TreeBuf[i] != TreBeg[i])
            goto RdTr10;
    count = TREE->TreeLen;
    if (ChkSum(md.TreeBuf, 7) != TREE->TreSum1)
        goto RdTr10;
    count <<= 4;
    if ((u16)(done - 11) != count)
        goto RdTr10;
    if (ChkSum((const u8 *)TRELIST, count) != *(u16 *)(u16)(TRELIST + count))
        goto RdTr10;
    goto RdTr40;

RdTr10:                                 /* the disk */
    for (i = 0; i < sizeof md.TreeBuf; i++)
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

RdTr11:                                 /* one more: with five, a box that
                                           counts them */
    count = ++TREE->TreeLen;
    if (count >= 5) {
        if (count == 5) {
            *(u16 *)md.MenuBuf = (u16)(signed char)md.DosPth[0];
            OncZoom(0x728, OFFSET(Tree_P), 0, (struct Box STACK *)&box);
        }
        Number(md.MenuBuf, 0, count, NUM_SEPARATED);
        line.value = box.begin + 0x400 & 0xFF00 | 0x28;
        line.line = md.MenuBuf;
        line.colors = Scan_P;
        line.include = Empty_S;
        CntLine((struct LineArgs STACK *)&line);
        Window(box.begin + 0x400, 0x100 | box.size & 0xFF);
    }
    if (TREE->TreeLen == 1)
        goto RdTr15;

RdTr14:                                 /* the next entry of the directory */
    regs.ax = 0x4F00;
    goto RdTr16;

RdTr15:                                 /* the first */
    SetDTA();
    regs.name = SCANNED;
    regs.cx = 0x37;
    regs.ax = 0x4E00;

RdTr16:
    error = READ_STOPPED;
    if (TstBrk())
        goto RdTr18;
    regs.ax |= 2;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax != 0x53)
            goto RdTr22;
        error = READ_NO_DISK;
        goto RdTr18;
    }
    if ((md.DTA.FilAttr & ATTR_DIRECTORY) == 0)
        goto RdTr14;
    if (md.HidDirs == 0 && (md.DTA.FilAttr & ATTR_HIDDEN) != 0)
        goto RdTr14;
    if (md.DTA.FilName[0] == '.')
        goto RdTr14;
    if ((u16)(sizeof md.TreeBuf - sizeof(struct DIRTREE)
              - sizeof(struct DIRENT)) >> 4 > TREE->TreeLen)
        goto RdTr20;
    Beep();                             /* no room for it */
    error = READ_FULL;

RdTr18:                                 /* that is all, as it is */
    TREE->TreeErr = (u8)error;
    text = &LIST->DirRecu;
    do {
        *text = 0;
        text += sizeof(struct DIRENT);
    } while (text < end);
    goto RdTr26;

RdTr20:                                 /* a directory: its entry goes in
                                           here, before what follows */
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
    goto RdTr11;

RdTr22:                                 /* no more in this directory: on
                                           with the first one found in it,
                                           or else the one behind */
    if (entry[-1].DirRecu == 0) {
        if (entry->DirRecu == 0)
            goto RdTr26;
    } else {
        do
            entry--;
        while (entry[-1].DirRecu != 0);
    }
    entry->DirRecu = 0;
    TREE->TreeLev = entry->DirLevel;
    /* Its name behind the backslash of its level, which is looked for in
       20 bytes at a time, and the mask behind the name. */
    text = SCANNED;
    count = entry->DirLevel;
    do {
        for (i = 20; i; i--)
            if (*text++ == '\\')
                break;
    } while (--count);
    text = (u8 *)(u16)(u32)MvzLine(text, entry->DirName, 13);
    text[0] = '\\';
    text[1] = '*';
    text[2] = '.';
    text[3] = '*';
    text[4] = 0;
    entry++;
    if (text + 1 > SCANNED + LenPath)   /* too long a path: not read */
        goto RdTr22;
    goto RdTr15;

RdTr26:
    if (TREE->TreeLen >= 5) {
        ResWin(box.begin, box.size, (u16)md.ErrBuf);
        Window(box.begin, box.size);
    }
    if (TREE->TreeErr != READ_FULL)
        goto RdTr40;
    if (TREE->TreeLen < 5) {            /* too few for a file */
        regs.ax = 0x4100;
        regs.name = Tree_F;
        Intr21((struct DosRegs STACK *)&regs);
        goto RdTr40;
    }
    if (TstBrk())
        goto RdTr40;

    /* The file. */
    OncZoom(0x728, OFFSET(TrSav_P), 0, (struct Box STACK *)&box);
    for (i = 0; i < 5; i++)
        md.TreeBuf[i] = TreBeg[i];
    count = TREE->TreeLen;
    TREE->TreSum1 = ChkSum(md.TreeBuf, 7);
    count <<= 4;
    *(u16 *)(u16)(TRELIST + count) = ChkSum((const u8 *)TRELIST, count);
    size = count + 11;
    regs.ax = 0x4300;                   /* with the attributes it has */
    regs.bx = size;
    regs.cx = count;
    regs.name = Tree_F;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax != 2)
            goto RdTr37;
        regs.cx = 0;
    }
    regs.cx |= 0x20;
    md.BlocEr = 1;
    regs.ax = 0x3C00 | regs.ax & 0xFF;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax == 5)
            Beep();
        goto RdTr37;
    }
    regs.cx = size;
    regs.bx = regs.ax;
    regs.name = md.TreeBuf;
    regs.ax = 0x4000 | regs.ax & 0xFF;
    failed = Intr21((struct DosRegs STACK *)&regs);
    done = regs.ax;
    regs.ax = 0x3E00 | done & 0xFF;
    if (!Intr21((struct DosRegs STACK *)&regs) && !failed && done != size)
        Beep();

RdTr37:
    md.BlocEr = 0;
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);

RdTr40:
    TreeCur();
    return TREE->TreeErr == READ_NO_DISK;
}

/* The number of the entry at an address of the list, and the address of
   the entry with a number: TreNum and NumAd1 of the assembly. */
#define NUMBER(address) ((u16)((address) - TRELIST) >> 4)
#define ADDRESS(number) ((u16)(((number) << 4) + TRELIST))

/* Show a line of the list in the box that begins at `begin`: the one
   that is `offset` bytes of the list behind the first one shown. */
static void ChTr50(u16 begin, u16 offset)
{
    Window((begin & 0xFF | (u16)(u8)((begin >> 8) + (offset >> 4)) << 8)
           + 0x205, 0x126);
}

/* Where the table of the keys stands in the original, behind its entries
   for F2 and for Ctrl+R: see ChTr55 below. */
#define AFTER_F2     0xE45A
#define AFTER_CTRL_R 0xE45E

/* Show the tree of the current disk in a box that begins in the column
   `column`, of which only the low byte counts, and have a directory
   chosen in it: with the keys that move the cursor, by typing how its
   name begins, or with the mouse. F2 and Ctrl+R read the disk again.
   Returns 0 for Enter, with the path of the directory in TmpBuf, and 1
   if the box was left otherwise.

   The cursor and the first line shown are those of the tree, TreAdr and
   TreFrst, which a panel that shows the tree has too: such a panel is
   marked as changed. */
u16 ChTree(u16 column)
{
    struct LineArgs line;
    u16 begin = 0x200 | column & 0xFF;                  /* BX */
    u16 size = (u16)(u8)(md.Lines - 3) << 8 | 50;       /* DX */
    u16 color;                          /* AX: the colour in the high byte */
    u16 cursor, first;                  /* BP and DI: where to go, as
                                           numbers of entries from the keys
                                           and as addresses from ChTr11 */
    u16 was, shown;                     /* where the screen is */
    u16 key = 0;
    u16 path = 0;                       /* SI at ChTr55 */
    u16 state, mouse, held;             /* SI, CX and BP for the mouse */
    u16 moved, scroll, lines, place, count, cell, look, n, i;
    u16 *screen;
    u32 pair;

    HidCur();
    SavWin(begin, size, (u16)md.WinBuf);
    color = Color(ChTr_P[0]) << 8 | column & 0xFF;
    MinBox(begin, size, color);
    if (PANEL(md.WCB1seg)->WinTyp == 1)
        PANEL(md.WCB1seg)->ChgFlg = 1;
    if (PANEL(md.WCB2seg)->WinTyp == 1)
        PANEL(md.WCB2seg)->ChgFlg = 1;
    for (i = 0; i < LenPath; i++)
        md.MenuBuf[KEPT + i] = md.CrnPath[i];

ChTr59:                                 /* the box, and the tree read */
    line.value = begin + 0x118;
    line.line = ChTr0_S;
    line.colors = ChTr_P;
    line.include = Empty_S;
    CntLine((struct LineArgs STACK *)&line);
    screen = BoxLine(BasAdr(begin + (size & 0xFF00) - 0x600),
                     color & 0xFF00 | LINE, LEFT_RIGHT, 3,
                     (u8)(size - 10));
    line.value = 38;
    line.screen = screen + 5;
    line.line = ChTr1_S;
    NgLine((struct LineArgs STACK *)&line);
    line.screen = screen + 5 + 80;
    NgLine((struct LineArgs STACK *)&line);
    Window(begin, size);
    md.CrnPath[0] = 0;
    ReadTre();
    TrCr20((size >> 8) - 8);
    lines = (u16)(signed char)((size >> 8) - 8);
    n = TREE->TreeLen - lines;
    if (TREE->TreeLen < lines)
        n = 0;
    n = ADDRESS(n);
    if (n < md.TreFrst)
        md.TreFrst = n;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;
    md.MousEnt = 0;
    /* Nothing is shown yet: the screen is taken to be elsewhere. */
    cursor = md.TreAdr;
    first = md.TreFrst;
    md.TreAdr += 2 * sizeof(struct DIRENT);
    md.TreFrst += 2 * sizeof(struct DIRENT);
    goto ChTr11;

ChTr10:                                 /* to the entries with these numbers */
    cursor = ADDRESS(cursor);
    first = ADDRESS(first);

ChTr11:
    if (cursor != md.TreAdr) {          /* the speed search begins anew */
        *(u16 *)(md.MenuBuf + SPEED - 2) = 0;
        md.MenuBuf[SPEED] = 0;
    }

ChTr12:                                 /* the cursor within the lines */
    if (cursor < first)
        first = cursor;
    n = (u16)(u8)((size >> 8) - 9) << 4;
    if ((u16)(cursor - first) > n)
        first = cursor - n;
    moved = first - md.TreFrst;
    was = md.TreAdr;
    shown = md.TreFrst;
    md.TreAdr = cursor;
    md.TreFrst = first;
    if (moved == 0 && was == md.TreAdr)
        goto ChTr20;

    /* The lines. */
    screen = BasAdr(begin) + 165;
    cell = Color(ChTr_P[0]) << 8 | ' ';
    count = (u8)((size >> 8) - 8);
    do {
        for (i = 0; i < 38; i++)
            screen[i] = cell;
        screen += 80;
    } while (--count);
    PutTree(md.WCBcrn, begin + 0x104, size - 0x40A, ChTr_P);
    if (moved != 0) {
        /* On the screen: all of them, or the screen moved by a line and
           the one that is new. */
        place = begin + 0x205;
        n = size - 0x80C;
        if (moved == sizeof(struct DIRENT)) {
            scroll = 0x600 | (u8)((n >> 8) - 1);
        } else if (moved == (u16)-(int)sizeof(struct DIRENT)) {
            scroll = 0x700;
        } else {
            Window(place, n);
            goto ChTr19;
        }
        was -= moved;                   /* the line that had the cursor has
                                           moved too */
        Int10m(scroll & 0xFF00 | 1, Color(ChTr_P[0]) << 8 | place & 0xFF,
               place, n + place - 0x101);
        ChTr50(begin, (u16)(signed char)scroll << 4);
    }
    ChTr50(begin, was - shown);         /* the line that had the cursor */
    ChTr50(begin, md.TreAdr - md.TreFrst);      /* and the one that has */

ChTr19:                                 /* the path of the cursor */
    Window(begin + (size & 0xFF00) - 0x500 + 5, 0x126);

ChTr20:                                 /* the speed search, and a key */
    place = begin + (size & 0xFF00) - 0x400 + 19;
    if (md.WinMous == 0)
        InSt10(place, (u16)(md.MenuBuf + SPEED), 12, ChTr_P[1] << 8);
    Cursor(place + *(u16 *)(md.MenuBuf + SPEED - 2));
    cursor = NUMBER(md.TreAdr);
    first = NUMBER(md.TreFrst);
    if (LOW(GetMous(0)) != 0)
        goto ChTr60;
    md.MousEnt = 0;
    md.WinMous = 0;
    key = Input(14);
    if (key >> 8 == 0xFC)
        goto ChTr60;
    if (key >> 8 != 0) {                /* Case takes none without a scan
                                           code */
        switch (key & 0xFF) {
        case 0:
            switch (key >> 8) {
            case 0x48: goto ChTr35;
            case 0x50: goto ChTr37;
            case 0x49:                  /* PgUp */
                pair = PgUp(cursor, first,
                            (u16)(signed char)((size >> 8) - 8));
                cursor = LOW(pair);
                first = HIGH(pair);
                goto ChTr10;
            case 0x51:                  /* PgDn */
                pair = PgDn(cursor, first,
                            (u16)(signed char)((size >> 8) - 8),
                            TREE->TreeLen);
                cursor = LOW(pair);
                first = HIGH(pair);
                goto ChTr10;
            case 0x47:                  /* Home */
                cursor = 0;
                goto ChTr10;
            case 0x4F:                  /* End */
                cursor = TREE->TreeLen;
                goto ChTr35;
            case 0x44: goto ChTr46;     /* F10 */
            case 0x3C:                  /* F2 */
                path = AFTER_F2;
                goto ChTr55;
            }
            break;
        case 0x0D: goto ChTr45;
        case 0x1B: goto ChTr46;
        case 0x12:                      /* Ctrl+R */
            path = AFTER_CTRL_R;
            goto ChTr55;
        }
    }
    /* Another key is for the speed search. */
    n = Search(MAIN_SEG, md.TreAdr, TRELIST, ADDRESS(TREE->TreeLen),
               sizeof(struct DIRENT), key);
    if (n == md.TreAdr)
        goto ChTr20;
    cursor = n;
    first = md.TreFrst;
    goto ChTr12;

ChTr35:                                 /* Up */
    if (cursor == 0)
        goto ChTr20;
    cursor--;
    goto ChTr10;

ChTr37:                                 /* Down */
    cursor++;
    if (cursor >= TREE->TreeLen)
        goto ChTr20;
    goto ChTr10;

ChTr55:                                 /* the disk read again */
    /* The assembly comes here from Case with SI behind the entry of the
       key in its table of keys, an address in the code segment, and
       DelTr0 takes SI for the address of a path in the main data: the
       tree file it spoils is that of the drive named there, if the second
       byte there is a colon. In the original those two bytes are in
       TreeBuf, in the name of its 510th entry: the second and third
       character for F2, the sixth and seventh for Ctrl+R. No name that
       DOS gives has a colon, so it is the current drive unless a tree
       file has such a name. The addresses are the original's here, since
       the table is not in the C translation. */
    place = begin + 0x205;
    n = size - 0x80C;
    cell = Color(ChTr_P[0]) << 8 | ' ';
    count = n >> 8;
    do {
        screen = BasAdr(place);
        for (i = n & 0xFF; i; i--)
            *screen++ = cell;
        place += 0x100;
    } while (--count);
    DelTr0((const u8 *)path);
    goto ChTr59;

ChTr60:                                 /* the mouse, while a button is
                                           down */
    state = 0;
    mouse = TREE->TreeLen;
    held = cursor;
ChTr61:
    pair = GetMous(mouse);
    mouse = HIGH(pair);
    if (LOW(pair) == 0)
        goto ChTr65;
    held = mouse;
    ClrKbd();
    look = 'x';                         /* the middle button alone */
    if (LOW(pair) & 3)
        look = 0;
    n = state;
    state = look << 8 | LOW(pair);
    if (look != n >> 8)
        TypMous(look);
    if ((state & 3) == 0)
        goto ChTr61;
    if (!InWind(held, begin + 0x204, size - 0x80A))
        goto ChTr67;
    /* Not on a line. If the button went down on one, the cursor goes up
       while the mouse is above the lines and down while it is below. */
    if (md.WinMous == 0)
        goto ChTr61;
    cursor = first;
    if ((u8)(held >> 8) < (u8)((begin >> 8) + 2))
        goto ChTr35;
    if ((u8)(held >> 8) < (u8)((begin >> 8) + (size >> 8) - 6))
        goto ChTr61;
    cursor += (u16)(signed char)((size >> 8) - 9);
    goto ChTr37;

ChTr65:                                 /* let go */
    TypMous(0);
    key = state;
    if ((key & 0xFF) == 2)              /* the right button */
        goto ChTr20;
    if ((key & 0xFF) < 1)
        goto ChTr20;
    if ((key & 0xFF) != 1)              /* more than one, or the middle */
        goto ChTr46;
    if (md.WinMous != 0)
        goto ChTr20;
    if (InWind(held, begin, size - 0x102))      /* outside the box */
        goto ChTr46;
    goto ChTr20;

ChTr67:                                 /* on a line */
    md.WinMous = 1;
    n = (u16)(signed char)((u8)((u16)(held - begin) >> 8) - 2);
    key = DoublMo(begin + 0x204 + (n << 8), 0x100 | (u8)(size - 10), n);
    if (key == 2) {                     /* the cursor goes there */
        n += first;
        cursor = TREE->TreeLen - 1;
        if (cursor > n)
            cursor = n;
        goto ChTr10;
    }
    if (key == 0)
        goto ChTr20;
    TypMous(0);                         /* a second click: Enter */
    key = 0x1C0D;

ChTr45:                                 /* Enter */
    for (i = 0; i < LenPath; i++)
        md.TmpBuf[i] = md.CrnPath[i];
    md.TmpBuf[LenPath] = 0;

ChTr46:                                 /* Esc */
    HidCur();
    for (i = 0; i < LenPath; i++)
        md.CrnPath[i] = md.MenuBuf[KEPT + i];
    ResWin(begin, size, (u16)md.WinBuf);
    Window(begin, size);
    return (key & 0xFF) != 0x0D;
}
