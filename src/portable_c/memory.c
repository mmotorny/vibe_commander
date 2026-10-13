/*
 * The map of the memory, Alt+F5: what DOS has given to whom, which
 * interrupt vectors point into it, and the programs that were started
 * from here and stay in the memory, to be released.
 *
 * Translated from VCMEM.INC.
 *
 * DOS keeps its memory as a chain of blocks, each with a header of a
 * paragraph in front of it that names its owner. F65_00 first lists the
 * blocks, BlkList, as pairs of words: where a block begins and whose it
 * is. From that it makes the lines of the map, MemList: one for every
 * owner, with the number of its blocks and their size, and one for every
 * stretch of free memory. What the program itself gives up while a
 * command runs counts as free.
 */
#include "vc.h"

#define MaxVect 8                       /* vectors that a line has room for */
#define SegDOS  0x70                    /* where DOS begins */

#pragma pack(push, 1)

/* A line of the map. A line whose address is 0 ends the list. */
struct MemLine {
    u8  flags;                          /* see below */
    u16 address;                        /* the segment it begins at */
    u16 blocks;
    u16 size;                           /* in paragraphs */
    u8  name[12];
    u8  hooked;                         /* vectors that point into it */
    u8  vectors[MaxVect];               /* the first of them */
};

#pragma pack(pop)

LAID_OUT(MemLine, 7 + 12 + 1 + MaxVect);

#define CAN_GO   1                      /* the program can be released */
#define SELECTED 2                      /* and is to be */

/* Where the lists are, and what the map keeps while it is shown: MemData
   of the assembly, in TmpBuf. */
#define BlkList md.WinBuf
#define MemList ((struct MemLine *)md.MenuBuf)
#define MemCur  (*(u16 *)(md.TmpBuf + 100))     /* the line of the cursor */
#define MemFrst (*(u16 *)(md.TmpBuf + 102))     /* the first line shown */
#define MemMenu md.TmpBuf[104]          /* the button chosen */
#define MemMous md.TmpBuf[105]          /* the buttons of the mouse */
#define MemTmp  (md.TmpBuf + 106)       /* a name of eight characters */

/* The marks a program has that can be released, and one that is to be. */
#define BULLET 7
#define TICK   0xFB

#define LINE       0xC4                 /* a single line */
#define LEFT_RIGHT 0xC7B6               /* where one meets the frame */

#define KEY_MOUSE 0xFC                  /* the high byte of a "key" that
                                           says the mouse did something */

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))
#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define NEAR(pointer) ((u16 *)(u16)(u32)(pointer))

/* F65_00 is declared in vc.h: see "The commands" there. */

/* The routines of the screen, the mouse and the keyboard that this file
   calls are C, of vc.h, and so is Int21d, by which DOS is asked for the
   map without Intr21, as in the original: see Dos5802 to Dos2523 below.

   MemRun returns the segment of the part of the program that goes while
   a command runs, and ResSeg that of the part that stays, where Counter,
   PrgList and Command are: both of vc.h. Release, of that part
   (resident.c), releases a program that has stayed in the memory:
   `record` is what PrgList has of it. It is called far, at its offset in
   that part's segment, and loads its own DS. */
typedef void __far ReleaseFn(u16 record);
#pragma aux ReleaseFn parm [ax] modify exact [ax]
extern const u8 __far Release[];
#pragma aux Release "*"

/* C routines of other files that vc.h does not have. MvsBw writes `count`
   characters to the screen buffer in the colour in the high byte of
   `color`; HexDec returns the two digits of a number below 100, the tens
   in the low byte. */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
u16 HexDec(u16 number);                                 /* format.c */

/* The buttons of the map and their colours, of memory_t.c, which
   MinMnu takes by their offsets; and what the part of the program that
   stays has for the map: the number of programs that have stayed too
   and for each the segment where its vectors are noted, of struct Stays
   there, and the name of the command processor. */
extern const u8 __far Memo1_P[];
extern const u8 __far Memo_D[];
extern const u8 __far Command[];
#pragma aux Command "*"
#define Counter STAYS(Counter)
#define PrgList STAYS(PrgList)

/* What a program that has stayed is noted with, in a segment of its own:
   the segment of the program, its name, the number of vectors it took
   and, five bytes each, their numbers with what they were before. */
#define PRG_PSP     0
#define PRG_NAME    2
#define PRG_VECTORS 14
#define PRG_VECTOR(number) (15 + 5 * (number))

#define Conf_C  19                      /* the numbers of the colours: the */
#define CnfRe_C 20                      /* window, the cursor, a line that */
#define CnfBo_C 21                      /* is selected, and the cursor on */
#define CnfUn_C 22                      /* such a line */

static const u8 CODE Mem0_S[] = " Memory Info ";
static const u8 CODE Mem1_S[] = {
    'A', 'd', 'd', 'r', 'e', 's', 's', ' ', 'B', 'l', 'o', 'c', 'k', 's',
    0xFF, 3, 'S', 'i', 'z', 'e', 0xFF, 4, 'P', 'r', 'o', 'g', 'r', 'a', 'm',
    0xFF, 8, 'H', 'o', 'o', 'k', 'e', 'd', ' ', 'v', 'e', 'c', 't', 'o',
    'r', 's', 0
};
static const u8 CODE Mem2_S[] = {
    '^', 'r', '[', ' ', 'R', 'e', 'l', 'e', 'a', 's', 'e', ' ', ']', '^',
    'r', 0xFF, 3, '[', ' ', 'C', 'a', 'n', 'c', 'e', 'l', ' ', ']', 0
};
static const u8 CODE DosMe_S[] = "DOS ";
static const u8 CODE FrMem_S[] = "free memory";
static const u8 CODE Syst_S[] = "system";
static const u8 CODE Unknw_S[] = "unknown";
static const u8 CODE Memo_P[] = {Conf_C, CnfBo_C, CnfRe_C, CnfUn_C};

/* Set address: the line with the number `number`. */
static struct MemLine *MemAddr(u16 number)
{
    return MemList + number;
}

/* Find end: the number of the first line behind `number` that is no
   line any more. */
static u16 MemEnd(u16 number)
{
    do
        number++;
    while (MemAddr(number)->address != 0);
    return number;
}

/* Set color: what a line begins with, in the low byte, a space, or the
   mark of a program that can be released or is to be, and in the high
   byte its colour: `which` is 0 for a line and 2 for the line of the
   cursor. */
static u16 MemColr(const struct MemLine *line, u16 which)
{
    u16 mark = ' ';

    if (line->flags & CAN_GO) {
        mark = BULLET;
        if (line->flags & SELECTED) {
            which++;
            mark = TICK;
        }
    }
    return Color(Memo_P[which]) << 8 | mark;
}

/* Set/Reset cursor: give the line `cursor` its mark and its colour on the
   screen, that of the cursor if `which` is 2 and its own if 0. `first`
   is the first line shown, `begin` and `size` are the window. */
static void MemCurs(u16 which, u16 cursor, u16 first, u16 begin, u16 size)
{
    u16 place = ((begin & 0xFF) | (u16)(u8)((begin >> 8) + cursor - first) << 8)
                + 0x405;
    u16 width = (u8)(size - 12);
    u16 cell = MemColr(MemAddr(cursor), which);
    u16 *screen = BasAdr(place);

    *(u8 *)screen = (u8)cell;
    MvsColr(screen, width, cell & 0xFF00 | cell >> 8);
    Window(place, 0x100 | width);
}

/* Output item: write a line of the map to the screen buffer at `place`,
   an empty one for what is no line. Returns the line behind it, or the
   same if it was none.

   Ported as it is: when all eight vectors that a line has room for are
   written and no more point into it, the last is still replaced by three
   full stops, as if there were more. The count is taken down in a
   register whose other half holds the colour, and the test is for all of
   it being 0. */
static struct MemLine *MemItem(struct MemLine *line, u16 place, u16 size)
{
    u16 *screen = BasAdr(place);
    u16 cell = MemColr(line, 0);
    u16 count, color, left, i;
    u8 *dots;

    for (i = 0; i < (u8)(size - 12); i++)
        screen[i] = cell & 0xFF00 | ' ';
    if (line->address == 0)
        return line;
    *screen = cell;
    screen += 2;
    screen = NEAR(HexByt(screen, line->address >> 8));
    screen = NEAR(HexByt(screen, line->address));
    Number(md.TmpBuf, 0, line->blocks, NUM_PADDED);
    /* The size in bytes: 20 bits. */
    Number(md.TmpBuf + 20, line->size >> 12, line->size << 4,
           NUM_PADDED | NUM_SEPARATED);
    color = (u16)((u8 *)screen)[1] << 8;
    screen = NEAR(MvsBw(screen, md.TmpBuf + 4, 6, color));
    screen = NEAR(MvsBw(screen, md.TmpBuf + 23, 10, color));
    screen += 3;
    screen = NEAR(MvsBw(screen, line->name, 12, color));
    left = line->hooked;
    screen += 2;
    for (count = 0; count < MaxVect; count++) {
        if (left == 0)
            return line + 1;
        screen++;
        screen = NEAR(HexByt(screen, line->vectors[count]));
        left--;
    }
    if ((color | left) != 0) {
        dots = (u8 *)(screen - 2);
        dots[0] = '.';
        dots[2] = '.';
        dots[4] = '.';
    }
    return line + 1;
}

/* Copy a name of at most `count` characters to a line of the map, and
   fill with spaces what it leaves of them. */
static void F65_29(u8 *to, const u8 __far *name, u16 count)
{
    u8 c;

    for (; count; count--) {
        c = *name;
        if (c == 0)
            c = ' ';
        else
            name++;
        *to++ = c;
    }
}

/* Whether the upper memory is linked into the chain of blocks of DOS: 0
   or 1, or 0FFFFh if DOS does not know of it. */
static u16 Dos5802(void)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x5802;
    if (Int21d((struct DosRegs STACK *)&regs))
        return 0xFFFF;
    return regs.ax & 0xFF;
}

/* Link it, or not: `link` is 1 or 0. */
static void Dos5803(u16 link)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.bx = link;
    regs.ax = 0x5803;
    Int21d((struct DosRegs STACK *)&regs);
}

/* The segment of the first header of that chain: DOS has it in the word
   before the list that its function 52h names in ES:BX. */
static u16 Dos52(void)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x5200;
    Int21d((struct DosRegs STACK *)&regs);
    return WORD_AT(regs.es, regs.bx - 2);
}

/* What DR DOS says its version is, in a byte: 0 for another DOS, which
   does not know the call. */
static u16 Dos4452(void)
{
    struct DosRegs regs;

    regs.name = (const u8 __far *)&md;
    regs.ax = 0x4452;
    if (Int21d((struct DosRegs STACK *)&regs))
        return 0;
    return regs.ax & 0xFF;
}

/* Give Ctrl-C of DOS, interrupt 23h, to the routine at `routine` in the
   part of the program that stays in the memory. */
static void Dos2523(u16 routine)
{
    struct DosRegs regs;

    regs.name = AT(ResSeg(), routine);
    regs.ax = 0x2523;
    Int21d((struct DosRegs STACK *)&regs);
}

/* MCB analisis: list the blocks of DOS in BlkList. Before the first
   stands the place where DOS itself begins, and the first block, which
   DOS has for its own data, is given to that. A block of the program
   itself that goes while a command runs, its main data and, if Memory
   allocation is on, all that is not the part that stays, is listed as
   free. The list ends with where the memory ends, as a block that
   nobody has, and a word 0. */
static void F65_01(void)
{
    u16 *list = BlkList;
    u16 linked, header, block, owner, next, last;
    u16 gone = MemRun();

    linked = Dos5802();
    if (linked != 0xFFFF)
        Dos5803(1);
    *list++ = SegDOS;
    *list++ = SegDOS;
    header = Dos52();
    block = header + 1;
    owner = SegDOS;
    for (;;) {
        *list++ = block;
        *list++ = owner;
        next = header + WORD_AT(header, 3) + 1;
        last = *AT(header, 0) != 'M';
        header = next;
        block = header + 1;
        if (last)
            break;
        owner = WORD_AT(header, 1);
        if (block == MAIN_SEG)
            owner = 0;
        if (md.LoadFl != 0 && block == gone)
            owner = 0;
    }
    *list++ = block;
    *list++ = 0;
    *list++ = 0;
    if (linked != 0xFFFF)
        Dos5803(linked);
}

/* Program analisis: make the lines of the map from the list of blocks.
   A block that is its own owner is a program, and gets the blocks that
   name it as theirs; blocks that nobody has are free memory, those that
   follow one another one line; a block of an owner that has no block
   here is a line by itself, "system" if the owner is a number up to 8.
   Such a block is noted as its own owner in the list, for the vectors.

   A program is named after its file, which stands behind its
   environment, or after what the header of its block says; one that was
   started from here and has stayed, after what was noted of it then. */
static void F65_05(void)
{
    u16 *blocks = BlkList;
    u16 *pair = blocks;
    u16 *other;
    struct MemLine *line = MemList;
    const u8 __far *name;
    u8 *to;
    u16 address, owner, count, size, digits, room, programs, program;
    u16 environment, header, place, i;

    do {                                                /* F65_06 */
        if ((u8 *)line >= md.MenuBuf + LenMenu - sizeof(struct MemLine))
            break;
        address = pair[0];
        owner = pair[1];
        pair += 2;
        if (owner == 0) {                               /* Free block */
            count = 1;
            while (pair[1] == 0 && pair[2] != 0) {
                pair += 2;
                count++;
            }
            size = pair[0] - address - 1;
        } else {
            count = 0;
            size = 0;
            if (owner != address) {
                for (other = blocks; ; ) {
                    if (owner == other[0])
                        goto F65_08;    /* a block of a program here */
                    other += 2;
                    if (other[2] == 0)
                        break;
                }
                count = 1;
                size = pair[0] - address - 1;
            }
            other = blocks;                             /* F65_13 */
            do {
                if (address == other[1]) {
                    count++;
                    size += other[2] - other[0] - 1;
                }
                other += 2;
            } while (other[2] != 0);
        }
        line->flags = 0;                                /* F65_15 */
        line->address = address;
        line->blocks = count;
        line->size = size;
        line->hooked = 0;
        to = line->name;

        /* Searching for name */
        if (pair[-1] == 0) {
            F65_29(to, FrMem_S, 12);
            goto F65_30;
        }
        if (address == SegDOS) {
            for (i = 0; i < 4; i++)
                *to++ = DosMe_S[i];
            room = 12 - 4 - 4;
            digits = HexDec(md.VerDOS);
            if ((u8)digits != '0') {
                *to++ = (u8)digits;
                room--;
            }
            *to++ = (u8)(digits >> 8);
            *to++ = '.';
            digits = HexDec(md.VerDOS >> 8);
            *to++ = (u8)digits;
            *to++ = (u8)(digits >> 8);
            F65_29(to, DosMe_S + 4, room);
            goto F65_30;
        }
        /* Releasable program */
        programs = WORD_AT(ResSeg(), OFFSET(Counter));
        for (i = 0; i < programs; i++) {
            program = WORD_AT(ResSeg(), OFFSET(PrgList) + 2 * i);
            if (address == WORD_AT(program, PRG_PSP)) {
                line->flags |= CAN_GO;
                line->hooked = *AT(program, PRG_VECTORS);
                for (i = 0; i < MaxVect; i++)
                    line->vectors[i] = *AT(program, PRG_VECTOR(i));
                F65_29(to, AT(program, PRG_NAME), 12);
                goto F65_30;
            }
        }
        owner = pair[-1];                               /* System area */
        pair[-1] = address;
        if (owner <= 8) {
            F65_29(to, Syst_S, 12);
            goto F65_30;
        }
        if (owner != address)
            goto F65_25;
        /* Other programs: check environment */
        environment = WORD_AT(address, 0x2C);
        if (environment == 0 || environment == address)
            goto F65_24;
        for (other = blocks; ; ) {
            if (other[2] == 0)
                goto F65_24;
            other += 2;
            if (other[-2] == environment)
                break;
        }
        header = environment - 1;
        if (address != WORD_AT(header, 1))
            goto F65_24;
        /* Get name from environment: behind its two zero bytes and a
           word that is 1. */
        place = 0x10;
        while (WORD_AT(header, place) != 0)
            place++;
        place += 4;
        if (WORD_AT(header, place - 2) == 1) {
            name = AT(header, place);
        } else {
            if ((u8)md.VerDOS >= 4 || address != WORD_AT(address, 0x16))
                goto F65_24;
            name = AT(ResSeg(), OFFSET(Command));
        }
        name = CutPath(name);
        if (*name != 0) {
            F65_29(to, name, 12);
            goto F65_30;
        }
F65_24:
        /* Check DOS version: from 4 on the header has the name. */
        if ((u8)md.VerDOS >= 4 || Dos4452() >= 0x65) {
            for (i = 0; i < 8; i++)
                MemTmp[i] = *AT(address - 1, 8 + i);
            MemTmp[8] = 0;
            F65_29(to, MemTmp, 12);
            goto F65_30;
        }
F65_25:
        F65_29(to, Unknw_S, 12);                        /* Unknown program */
F65_30:
        line++;
F65_08: ;
    } while (pair[2] != 0);
    line->flags = 0;                                    /* F65_09 */
    line->address = 0;
}

/* Vectors: note with each line the interrupt vectors that point into its
   blocks, the first eight of them, and how many they are. A program that
   can be released has those that were noted of it. */
static void F65_31(void)
{
    u16 *blocks = BlkList;
    u16 *pair;
    struct MemLine *line;
    u16 vector, target, owner, address;
    /* The segment of the table of vectors. Written as a constant, the
       compiler takes a far pointer into segment 0 for a near one. */
    volatile u16 table = 0;

    for (vector = 0; vector < 256; vector++) {
        target = (WORD_AT(table, 4 * vector) >> 4)
                 + WORD_AT(table, 4 * vector + 2);
        pair = blocks;
        if (target < pair[0])
            continue;
        do {                                            /* F65_33 */
            if (pair[2] == 0)
                goto F65_35;
            pair += 2;
        } while (target >= pair[0]);
        owner = pair[-1];
        line = MemList;
        do {                                            /* F65_34 */
            address = line->address;
            line++;
            if (address == 0)
                goto F65_35;
        } while (address != owner);
        line--;
        if (line->flags & CAN_GO)
            continue;
        line->hooked++;
        if (line->hooked < MaxVect + 1)
            (&line->hooked)[line->hooked] = (u8)vector;
F65_35: ;
    }
}

/* Alt F5 - memory info: the map in a window, with the keys that move the
   cursor, Ins, Grey + and Grey - to select programs that can be released,
   and two buttons: Release, for the selected ones or the one under the
   cursor, and Cancel. The mouse does the same.

   A program is released only if no program that came after it and stays
   has taken a vector that it took. */
void F65_00(void)
{
    struct LineArgs text;
    struct MemLine *line;
    u16 begin, size;                    /* BX and DX: the window */
    u16 cursor, first;                  /* BP and DI */
    u16 color, place, rows, was, moved, old, key, flags, n, i;
    u16 buttons, mouse, pressed, last;
    u16 programs, program, release;
    u16 *screen;
    u32 pair;
    struct Paged paged;
    struct Mouse pointer;

    HidCur();
    md.HlpPage = 30;
    F65_01();
    F65_05();
    F65_31();

    /* Output window */
    PutMnu(3);
    begin = 0x203;
    size = (u16)(u8)(md.Lines - 3) << 8 | 76;
    SavWin(begin, size, (u16)md.WinBuf);
    color = Color(Memo_P[0]) << 8;
    MinBox(begin, size, color);
    text.value = begin + 0x100 & 0xFF00 | 40;
    text.line = Mem0_S;
    text.colors = Memo_P;
    text.include = Mem0_S;              /* no line has ^I */
    CntLine((struct LineArgs STACK *)&text);
    screen = BasAdr(begin) + 165;
    text.value = (u8)(size - 12);
    text.screen = screen;
    text.line = Mem1_S;
    NgLine((struct LineArgs STACK *)&text);
    place = begin + (size & 0xFF00) - 0x500;
    BoxLine(screen + 75, color | LINE, LEFT_RIGHT, 3, (u8)(size - 10));
    BoxLine(BasAdr(place), color | LINE, LEFT_RIGHT, 3, (u8)(size - 10));
    text.value = place + 0x100 & 0xFF00 | 40;
    text.line = Mem2_S;
    CntLine((struct LineArgs STACK *)&text);
    Window(begin, size);
    cursor = 0;
    first = 0;
    MemCur = 2;
    MemFrst = 2;
    MemMenu = 0;
    MemMous = 0;
    md.WinMous = 0;
    md.MousEnt = 0;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;
    mouse = 0;

F65_40:                                 /* Test parameters */
    was = first;
    if (cursor < first)
        first = cursor;
    rows = (u16)(signed char)(u8)((size >> 8) - 9);
    if (cursor >= (u16)(rows + first))
        first = cursor - rows + 1;
    if (MemMous & 2) {
        /* The right button selects what the mouse goes over, or takes
           the selection off, as it did for the first line. */
        line = MemAddr(cursor);
        if (line->flags & CAN_GO) {
            if (!(md.WinMous & 2)) {
                md.WinMous |= 2;
                if (!(line->flags & SELECTED))
                    md.WinMous |= 4;
            }
            flags = line->flags & CAN_GO;
            if (md.WinMous & 4)
                flags |= SELECTED;
            if (flags != line->flags) {
                line->flags = (u8)flags;
                if (first == was)
                    MemCurs(2, cursor, first, begin, size);
            }
        }
    } else
        md.WinMous &= 1;
    moved = first - MemFrst;                            /* Mous54 */
    old = MemCur;
    was = MemFrst;
    MemCur = cursor;
    MemFrst = first;
    if (moved != 0 || old != cursor) {
        MemCurs(0, old, was, begin, size);              /* Reset cursor */
        if (moved != 0) {
            /* Output lines */
            line = MemAddr(MemFrst);
            place = begin + 0x405;
            n = (u8)((size >> 8) - 9);
            do {
                line = MemItem(line, place, size);
                place += 0x100;
            } while (--n);
            /* Output screen: by a line, the screen is scrolled and the
               line that comes in written; else all of them. */
            place = begin + 0x405;
            rows = (u8)((size >> 8) - 9);
            key = 0x600 | (u8)(rows - 1);
            n = rows << 8;
            if (moved == 1 || moved == 0xFFFF) {
                if (moved == 0xFFFF)
                    key = 0x700;
                place = Int10m(
                    key & 0xFF00 | 1, Color(Memo_P[0]) << 8 | place & 0xFF,
                    place, ((u16)(u8)(rows - 1) << 8 | 63) + place).upper;
                place += (key & 0xFF) << 8;
                n = 0x100;
            }
            Window(place, n | 64);
        }
        MemCurs(2, MemCur, MemFrst, begin, size);       /* Set cursor */
    }

F65_48:                                 /* Input */
    pointer = GetMous(mouse);
    if (pointer.buttons != 0)
        goto Mous01;
    md.WinMous = 0;
    md.MousEnt = 0;
F65_49:
    pair = MinMnu(OFFSET(Memo_D),
                  (u16)(u8)((begin >> 8) + (size >> 8) - 4) << 8
                  | (u8)(begin + 25),
                  0x118, OFFSET(Memo1_P), 1, MemMenu);
    MemMenu = (u8)HIGH(pair);
    key = LOW(pair);
    cursor = MemCur;
    first = MemFrst;
    if (key >> 8 == KEY_MOUSE)
        goto Mous00;
    MemMous = 0;
    if (key >> 8 == 0)                  /* Case takes none without a scan code */
        goto F65_48;
    switch (key) {
    case 0x4800:                        /* Up */
        goto F65_50;
    case 0x5000:                        /* Down */
        goto F65_52;
    case 0x4700:                        /* Home */
        cursor = 0;
        goto F65_40;
    case 0x4F00:                        /* End */
        cursor = MemEnd(cursor) - 1;
        goto F65_40;
    case 0x4900:                        /* PgUp */
        paged = PgUp(cursor, first, (u16)(signed char)(u8)((size >> 8) - 9));
        cursor = paged.cursor;
        first = paged.first;
        goto F65_40;
    case 0x5100:                        /* PgDn */
        paged = PgDn(cursor, first, (u16)(signed char)(u8)((size >> 8) - 9),
                     MemEnd(cursor));
        cursor = paged.cursor;
        first = paged.first;
        goto F65_40;
    case 0x5200:                        /* Ins */
        line = MemAddr(cursor);
        if (line->flags & CAN_GO) {
            line->flags ^= SELECTED;
            MemCurs(2, cursor, first, begin, size);
        }
        if (md.InsDwn != 0)
            goto F65_52;
        goto F65_40;
    case 0x4E2B:                        /* Grey + */
    case 0x4A2D:                        /* Grey - */
        line = MemList;
        do {
            if (line->flags & CAN_GO) {
                line->flags &= ~SELECTED;
                if (key == 0x4E2B)
                    line->flags |= SELECTED;
            }
            line++;
        } while (line->address != 0);
        /* All lines are written again: as if the first were another. */
        MemFrst += 2;
        MemCur = MemFrst;
        goto F65_40;
    case 0x4400:                        /* F10 */
        goto F65_83;
    }
    if ((key & 0xFF) == 0x0D)           /* Enter */
        goto F65_70;
    if ((key & 0xFF) == 0x1B)           /* Esc */
        goto F65_83;
    goto F65_48;

Mous00:                                 /* Mouse support */
    pointer = GetMous(mouse);
Mous01:
    buttons = pointer.buttons;
    mouse = pointer.place;
    ClrKbd();
    pressed = mouse;
    if (buttons & 4) {
        /* The middle button leaves, unless another is down with it. */
        TypMous('x');
        do {
            pointer = GetMous(mouse);
            buttons = pointer.buttons;
            mouse = pointer.place;
            ClrKbd();
        } while (buttons & 4);
        TypMous(0);
        if (!(buttons & 3))
            goto F65_83;
    }
    MemMous = (u8)buttons;
    if (!InWind(mouse, begin + 0x404, size - 0x90A))
        goto Mous10;
    if (md.WinMous != 0) {
        /* A button held since it was pressed on a line: above the lines
           and below them the cursor goes on. */
        cursor = MemCur;
        first = MemFrst;
        if ((u8)(mouse >> 8) < (u8)((begin >> 8) + 4))
            goto F65_50;
        if ((u8)(mouse >> 8) < (u8)((begin >> 8) + (size >> 8) - 5))
            goto F65_48;
        goto F65_52;
    }
    if (!(buttons & 1))
        goto F65_48;
    if (!InWind(mouse, (u16)(u8)((begin >> 8) + (size >> 8) - 4) << 8 | 28,
                0x118))
        goto F65_49;                    /* on the buttons */
    /* Let go outside the window, it leaves. */
    pointer = GetMous(mouse);
    if (pointer.buttons != 0)
        goto Mous01;
    mouse = pressed;
    if (InWind(mouse, begin, size - 0x102))
        goto F65_83;
    goto F65_48;

Mous10:                                 /* on a line */
    md.WinMous |= 1;
    first = MemFrst;
    cursor = (u8)((mouse >> 8) - (begin >> 8) - 4) + first;
    last = MemEnd(0) - 1;
    if (cursor > last) {
        cursor = last;
        goto F65_40;
    }
    if (buttons & 2)
        goto F65_40;
    n = DoublMo(mouse & 0xFF00 | (u8)(begin + 4), 0x100 | (u8)(size - 10),
                cursor);
    if (n == 2)
        goto F65_40;
    if (n == 0)
        goto F65_48;
    /* A second click on the line releases its program, and no other. */
    md.MousEnt = 0;
    ((u16 *)&md.MousTim)[1] = 0xFFFF;
    TypMous(0);
    if (!(MemAddr(cursor)->flags & CAN_GO))
        goto F65_48;
    for (line = MemList; line->address != 0; line++)
        line->flags &= CAN_GO;
    goto F65_73;

F65_50:                                 /* Up */
    if (cursor != 0)
        cursor--;
    goto F65_40;

F65_52:                                 /* Down */
    cursor++;
    if (MemAddr(cursor)->address == 0)
        cursor--;
    goto F65_40;

F65_70:                                 /* Enter */
    if (MemMenu != 0)
        goto F65_83;
    for (line = MemList; line->address != 0; line++)
        if (line->flags & SELECTED)
            goto F65_74;
F65_73:
    line = MemAddr(cursor);
    if (!(line->flags & CAN_GO))
        goto F65_40;
    line->flags |= SELECTED;
F65_74:
    /* ErrBuf says of every vector whether a program that stays has it. */
    for (i = 0; i < 256; i++)
        ((u8 *)md.ErrBuf)[i] = 0;
    programs = WORD_AT(ResSeg(), OFFSET(Counter));
    if (programs == 0) {
        first = (u16)md.ErrBuf + 256;   /* what the assembly has in DI */
        goto F65_40;
    }
    SavMous();                          /* Save mouse cursor */
    CurMous(0);
    RestVct();                          /* Restore vectors */
    do {                                /* Release, the last first */
        program = WORD_AT(ResSeg(), OFFSET(PrgList) + 2 * (programs - 1));
        for (line = MemList; ; line++) {
            if (line->address == 0)
                goto F65_82;
            if (line->address == WORD_AT(program, PRG_PSP))
                break;
        }
        n = *AT(program, PRG_VECTORS);
        release = line->flags & SELECTED;
        if (release)
            for (i = 0; i < n; i++)
                if (((u8 *)md.ErrBuf)[*AT(program, PRG_VECTOR(i))] != 0) {
                    release = 0;
                    break;
                }
        if (release)
            ((ReleaseFn *)((u32)ResSeg() << 16 | OFFSET(Release)))(program);
        else
            for (i = 0; i < n; i++)
                ((u8 *)md.ErrBuf)[*AT(program, PRG_VECTOR(i))] = 1;
F65_82: ;
    } while (--programs);
    Dos2523(HookVct());                 /* Set vectors */
    IniMous();                          /* Restore mouse */

F65_83:                                 /* Exit */
    ResWin(begin, size, (u16)md.WinBuf);
    Window(0, (u16)md.Lines << 8);
}
