/*
 * The editor: where in the memory its text is, what it shows of it, and
 * how it saves it.
 *
 * Translated from VCEDIT.INC.
 *
 * The text of the file is in the memory behind the program, from the
 * segment ViewSeg on, EdLen bytes of it, and may be longer than a segment.
 * A place in it is an offset from its start, 32 bits, or the address of
 * the byte: a segment, and an offset of less than 16 where the program has
 * made the address from an offset. The line that the cursor is in is
 * edited in a buffer of its own, EditBuf, which holds EdSizLn bytes of it,
 * and is put back into the text when the cursor leaves it.
 *
 * What is still assembly of the editor calls these for the addresses, for
 * the column of the cursor, for a line on the screen and to save the
 * file. The viewer has its window where the editor has, and clears and
 * scrolls it with T70_00 and T71_00.
 */
#include "vc.h"

#define LenEdit LenMenu                 /* Length of edit buffer */
#define EditBuf md.MenuBuf              /* Edit line buffer */

#pragma pack(push, 1)

/* What the editor keeps, in the main data from TmpBuf+100 on: EdData of
   the assembly, from EdOffst. What has no name here is the viewer's. */
struct EdData {
    u32 EdOffst;                        /* Offset of begin of the current line */
    u32 EdEndBf;                        /* Offset of end of part of current line */
    u16 EdSizLn;                        /* Length of part of line in buffer */
    u8  EdUpdat;                        /* Update line buffer */
    u8  EdWhole;                        /* The whole line allocates buffer */
    u16 EdCursr;                        /* Offset of the cursor in line buffer */
    u16 EdCol1;                         /* Cursor position for Up & Down */
    u8  EdUpDwn;                        /* Up & Down flag */
    u8  EdHex1;                         /* Cursor digit pointer for Up & Down */
    u8  EdScrn;                         /* Output all edit window */
    u8  EdCurLn;                        /* It doesn't need to output line */
    u16 EdUndo;                         /* Length of line in undo buffer */
    u8  Reserved1[77];
    u8  EdWrite;                        /* File was written */
    u16 EdSpace;                        /* Edit buffer size in paragraphs */
    u8  Reserved2;
    u8  EdStat;                         /* Status line */
    u8  EdHexAs;                        /* Hex / ASCII field in HEX mode */
    u8  EdHxCur;                        /* Cursor points 1 or 2 digit in HEX */
    u32 EdLen;                          /* Size of file in edit buffer */
    u16 EdCol;                          /* Cursor position */
    u16 EdLine;                         /* Cursor line */
    u16 Reserved3;
    u8  EdTxHex;                        /* Text / Hex mode */
    u8  EdChng;                         /* File was changed */
    u32 Reserved4;
    u16 EdCol0;                         /* First column displayed on the screen */
    u16 EdLin0;                         /* First line displayed on the screen */
    u16 Reserved5;
    u16 EdBase;                         /* The base of the edit window */
    u8  EdSizeX;                        /* The size of the edit window: */
    u8  EdSizeY;                        /* its columns and its rows */
    u32 EdFirst;                        /* Offset of first line on the screen */
    u16 EdShape;                        /* Original cursor shape */
};

/* Where a box is on the screen, as DialBox hands it back: in the order
   in which its entry for C stores BX, DX and SI. */
struct Box {
    u16 begin;                          /* the row in the high byte */
    u16 size;                           /* the rows in the high byte */
    u16 saved;                          /* where what it covers is kept */
};

#pragma pack(pop)

LAID_OUT(EdData, 138);

#define ed (*(struct EdData *)(md.TmpBuf + 100))

u32 T57_00(void);
u32 T57_01(u16 limit);
u16 T57_02(void);
u32 T60_00(u16 segment, u16 offset);
u32 T61_00(u16 high, u16 low);
u32 T62_00(void);
u16 T63_01(u16 high, u16 low);
u32 T70_00(u16 line);
void T71_00(u16 function, u16 place, u16 step);
void T72_00(u16 line, u16 segment, u16 from, u16 end);
void T73_00(void);
u32 T73_10(void);
void T74_00(void);
void T75_00(u16 number, u16 *screen, u16 color);
void T76_00(u16 place, u16 segment, u16 text, u16 colors);
u16 T80_00(void);

/* How the assembly passes arguments to these, and what it expects back
   (see the stubs in port/asm/VCEDIT.INC). */
#pragma aux T57_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T57_01 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux T57_02 "*_c" __far value [ax] modify exact [ax dx]
#pragma aux T60_00 "*_c" __far parm [ax] [si] value [dx ax] modify exact [ax dx]
#pragma aux T61_00 "*_c" __far parm [dx] [bx] value [dx ax] modify exact [ax dx]
#pragma aux T62_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T63_01 "*_c" __far parm [dx] [bx] value [ax] modify exact [ax]
#pragma aux T70_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux T71_00 "*_c" __far parm [ax] [bx] [dx] modify exact [ax bx cx dx si di]
#pragma aux T72_00 "*_c" __far parm [ax] [dx] [si] [bx] modify exact [ax]
#pragma aux T73_00 "*_c" __far modify exact [ax]
#pragma aux T73_10 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T74_00 "*_c" __far modify exact [ax cx si di]
#pragma aux T75_00 "*_c" __far parm [cx] [di] [ax] modify exact [ax]
#pragma aux T76_00 "*_c" __far parm [bx] [dx] [si] [ax] modify exact [ax]
#pragma aux T80_00 "*_c" __far value [ax] modify exact [ax bx cx dx si di]

/* Routines of the editor that are still assembly, by their entries for C.
   T50_00 fills the line buffer from the text and T51_00 puts it back into
   the text, if it has been changed. T53_10 returns where the line ends
   that the byte at an address is in, in the same segment: behind the two
   characters that end it, in hex mode behind its 16 bytes, or where the
   text ends; if it does not end in that segment, where it began. */
void T50_00(void);
void T51_00(void);
u16 T53_10(u16 segment, u16 offset);
#pragma aux T50_00 "*_a" __far modify exact [es]
#pragma aux T51_00 "*_a" __far modify exact [es]
#pragma aux T53_10 "*_a" __far parm [dx] [si] value [si] modify exact [si es]

/* And routines of the rest of the program. DialBox puts a box on the
   screen with its middle at `place`: `box` is where the assembly
   describes it, an offset in its code segment. Dialog does that and takes
   the keys; the low byte of what it returns is 0 for Enter on the first
   answer. `include` is the text that ^I stands for in a line of the box,
   another such offset. ResWin puts back in the screen buffer what a box
   covers, Window shows it. ChgDisk asks for the disk of a drive that
   shares the floppy drive with another, and is not 0 if the user would
   not change it. Int10m calls the BIOS for the screen, with the mouse
   pointer out of the way. */
void DialBox(u16 place, u16 box, struct Box STACK *where);
u16 Dialog(u16 box, u16 include);
void ResWin(u16 begin, u16 size, u16 saved);
u16 ChgDisk(const u8 *path);
void Int10m(u16 ax, u16 bx, u16 cx, u16 dx);
#pragma aux DialBox "*_a" __far parm [bx] [ax] [di] modify exact [bx dx si es]
#pragma aux Dialog  "*_a" __far parm [ax] [dx] value [ax] modify exact [ax es]
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux ChgDisk "*_a" __far parm [dx] value [ax] modify exact [ax es]
#pragma aux Int10m  "*_a" __far parm [ax] [bx] [cx] [dx] modify exact [ax bx cx dx es]

/* C routines of other files that vc.h does not have. */
u16 __far *HexByt(u16 __far *screen, u16 byte);         /* digits.c */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
void PathLin(u8 __far *text);                           /* string.c */
#pragma aux HexByt  "*_c" __far parm [es di] [ax] value [es di] modify exact [di es]
#pragma aux MvsBw   "*_c" __far parm [es di] [dx si] [cx] [ax] value [es di] modify exact [di es]
#pragma aux PathLin "*_c" __far parm [es si] modify exact [ax]

/* Boxes and a text of the assembly, which DialBox and Dialog take by
   their offsets in its code segment: only those are used here. */
extern const u8 __far SavFi_P[];        /* "Saving the file" */
extern const u8 __far SavEr_B[];        /* "There was an error writing..." */
extern const u8 __far Prot_B[];         /* "... is a read-only file" */
extern const u8 __far EdFl_S[];         /* " Edit " */
#pragma aux SavFi_P "*"
#pragma aux SavEr_B "*"
#pragma aux Prot_B "*"
#pragma aux EdFl_S "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* The name of the main extension file. */
static const u8 CODE MainExt_F[] = "VC.EXT";

#define Arrow_C 35                      /* the colour of the arrows at the
                                           ends of a line that goes on */

#define TAB 9
#define CR_LF 0x0A0D                    /* the two that end a line, as a */
#define LF_CR 0x0D0A                    /* word, and the other way round */

/* Errors of DOS, the one that Intr21 makes of a drive that is not ready,
   and the one that the editor makes of a disk that is full. */
#define ERR_NO_FILE   0x02
#define ERR_NO_ACCESS 0x05
#define ERR_NOT_READY 0x53
#define ERR_DISK_FULL 0xFF

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))
#define PAIR(high, low) ((u32)(high) << 16 | (low))

/* The byte at an address, and the segment that the main data is in. */
#define AT(segment, offset) ((u8 __far *)PAIR(segment, offset))
#define MAIN_DATA() HIGH((u32)(const void __far *)md.TmpBuf)

/* The offset in the text of the byte at an address, and the address of
   the byte at an offset: its segment in the high word, and in the low one
   what is left, less than 16. Of the offset 20 bits count. */
#define OFFSET_OF(segment, offset) \
    (((u32)(u16)((segment) - md.ViewSeg) << 4) + (offset))
#define ADDRESS_OF(high, low) \
    PAIR((u16)(((low) >> 4 | (high) << 12) + md.ViewSeg), (low) & 0x0F)

/* The column that a number of characters of the line buffer end in, with
   tabs to every eighth, in the low word; in the high one how many of the
   characters were not counted, because the column `limit` was reached.
   The column is then the one that the character begins in which reaches
   it. */
static u32 Column(u16 count, u16 limit)
{
    const u8 *text = EditBuf;
    u16 column = 0;
    u16 before;

    if (count == 0)
        return 0;
    do {
        before = column;
        if (*text++ == TAB)
            column |= 7;
        column++;
        if (column >= limit)
            return PAIR(count, before);
    } while (--count);
    return column;
}

/* The column that the line in the buffer ends in, as far as the buffer
   has it, but no further than the last that the editor goes to; and
   T57_01, no further than `limit`. */
u32 T57_00(void)
{
    return Column(ed.EdSizLn, LenEdit - 100);
}

u32 T57_01(u16 limit)
{
    return Column(ed.EdSizLn, limit);
}

/* The column of the cursor, counted from 0: in hex mode the number of the
   byte in its line. */
u16 T57_02(void)
{
    if (ed.EdTxHex != 0)
        return ed.EdCursr;
    return LOW(Column(ed.EdCursr, 0xFFFF));
}

/* The offset in the text of the byte at an address. */
u32 T60_00(u16 segment, u16 offset)
{
    return OFFSET_OF(segment, offset);
}

/* The address of the byte at an offset in the text, the segment in the
   high word; and T62_00, of the first line on the screen. */
u32 T61_00(u16 high, u16 low)
{
    return ADDRESS_OF(high, low);
}

u32 T62_00(void)
{
    return ADDRESS_OF(HIGH(ed.EdFirst), LOW(ed.EdFirst));
}

/* Where an offset is in the text: 0 before its end, 1 at the end and 2
   behind it. */
u16 T63_01(u16 high, u16 low)
{
    if (high != HIGH(ed.EdLen))
        return high < HIGH(ed.EdLen) ? 0 : 2;
    if (low != LOW(ed.EdLen))
        return low < LOW(ed.EdLen) ? 0 : 2;
    return 1;
}

/* Clear a line of the window in the screen buffer; only the low byte of
   `line` counts. Returns where the line is in the buffer, and in the high
   word its row and column on the screen. */
u32 T70_00(u16 line)
{
    u16 place = ed.EdBase + (line << 8);
    u16 *screen = BasAdr(place);
    u16 *cell = screen;
    u16 blank = Color(Back_C) << 8 | ' ';
    u16 count;

    for (count = ed.EdSizeX; count; count--)
        *cell++ = blank;
    return PAIR(place, (u16)screen);
}

/* Move the lines of the window up or down by one, in the screen buffer
   and on the screen: those from the line `line`, which is the low byte of
   `function`, to the last but one. `place` is where the first of them
   goes to, and `step` the way from one to the next in the buffer, in
   bytes: 160 to move them up, and then `place` is the line `line`, or
   -160 to move them down, and `place` the last line of the window. The
   high byte of `function` is what the BIOS is asked to do with the screen,
   6 to scroll up and 7 to scroll down; the line that comes free is blank
   there. Nothing is done if `line` is the last line. */
void T71_00(u16 function, u16 place, u16 step)
{
    u16 *to = BasAdr(place);
    u16 *from = (u16 *)((u8 *)to + step);
    u16 count = (u8)(ed.EdSizeY - function) - 1;
    u16 cells, column;

    if (count == 0)
        return;
    do {
        /* As many cells as the window has columns; the high byte of
           the count of lines is on top of that, which is 0. */
        cells = count & 0xFF00 | ed.EdSizeX;
        for (column = 0; column != cells; column++)
            to[column] = from[column];
        from = (u16 *)((u8 *)from + step);
        to = (u16 *)((u8 *)to + step);
    } while (--count);
    Int10m(function & 0xFF00 | 1,
           Color(Back_C) << 8 | place & 0xFF,
           ed.EdBase + ((function & 0xFF) << 8),
           *(u16 *)&ed.EdSizeX + ed.EdBase - 0x101);
}

/* Write a line of the text to the line `line` of the window in the
   screen buffer, of which the low byte counts: the bytes in the segment
   `segment` from `from` to `end`.

   As text: without the two characters that end a line, if it has them, a
   carriage return and a line feed in either order; with tabs to every
   eighth column; from the column EdCol0 on, and as much as the window is
   wide. Where it goes on, the last column has an arrow, and the first one
   has, if columns are left out there.

   In hex: the offset of the first byte, eight digits, and then at most 16
   bytes, as two digits each, in fours, and once more as characters, a
   zero as a dot.

   Ported as it is: the arrow in the first column is drawn for a line of
   no bytes too, which T73_10 does not hand over at all. */
void T72_00(u16 line, u16 segment, u16 from, u16 end)
{
    const u8 __far *text = AT(segment, 0);
    u16 *screen = (u16 *)LOW(T70_00(line));
    u16 __far *cell;
    u8 *ascii;
    u32 offset;
    u16 column, place, c, count;

    if (ed.EdTxHex == 0) {
        column = 0;
        if (end >= 2) {
            c = *(const u16 __far *)(text + (u16)(end - 2));
            if (c == CR_LF || c == LF_CR)
                end -= 2;
        }
        while (from < end) {
            c = text[from++];
            if (c == TAB) {
                column |= 7;
                c = ' ';
            }
            place = column++;
            if (place < ed.EdCol0)
                continue;
            place -= ed.EdCol0;
            if ((u8)place >= ed.EdSizeX) {
                screen[(u16)(ed.EdSizeX - 1)] = Color(Arrow_C) << 8 | 0x1A;
                break;
            }
            *(u8 *)(screen + place) = (u8)c;
        }
        if (ed.EdCol0 != 0)
            *screen = Color(Arrow_C) << 8 | 0x1B;
        return;
    }
    offset = OFFSET_OF(segment, from);
    cell = (u16 __far *)(screen + 1);
    cell = HexByt(cell, (u16)(offset >> 24));
    cell = HexByt(cell, (u16)(offset >> 16));
    cell = HexByt(cell, (u16)offset >> 8);
    cell = HexByt(cell, (u16)offset);
    cell += 2;
    ascii = (u8 *)((u16 *)(u16)(u32)cell + 52);
    for (count = 16; count; count--) {
        if (from >= end)
            break;
        c = text[from++];
        *ascii = c == 0 ? '.' : (u8)c;
        ascii += 2;
        cell = HexByt(cell, c) + 1;
        if ((count & 3) == 1)
            cell++;
    }
}

/* Write the line that the cursor is in to the screen buffer. Returns
   where it is on the screen, and in the high word its size, one row.

   As text it is the line buffer that is written. If the buffer does not
   hold the whole line and has fewer characters behind the cursor than
   the window has columns to the right of it, it is first put back into
   the text and filled again. A line of no characters is cleared and has
   no arrow in its first column. */
u32 T73_10(void)
{
    u16 line = ed.EdLine - ed.EdLin0;
    u16 place = ed.EdBase + (line << 8);
    u32 address;
    u16 from, end;

    if (ed.EdTxHex == 0) {
        if ((u16)(ed.EdSizLn - ed.EdCursr)
                < (u16)((signed char)ed.EdSizeX + ed.EdCol0 - ed.EdCol)
            && ed.EdWhole == 0) {
            T51_00();
            T50_00();
        }
        from = (u16)EditBuf;
        end = from + ed.EdSizLn;
        T70_00(line);
        if (from < end)
            T72_00(line, MAIN_DATA(), from, end);
    } else {
        address = ADDRESS_OF(HIGH(ed.EdOffst), LOW(ed.EdOffst));
        end = T53_10(HIGH(address), LOW(address));
        T72_00(line, HIGH(address), LOW(address), end);
    }
    return PAIR(0x100 | ed.EdSizeX, place);
}

/* Write the line that the cursor is in, and show it. */
void T73_00(void)
{
    u32 where = T73_10();

    Window(LOW(where), HIGH(where));
}

/* Put the name of the file into the status line, the first row of the
   screen buffer: in 31 columns from the seventh on, shortened to fit. */
void T74_00(void)
{
    u8 *name = md.TmpBuf;
    u16 *screen = md.ScrBuf + 6;
    u16 blank, count;

    for (count = 0; count < LenStr + 1; count++)
        name[count] = md.ViewFil[count];
    ShrtLin(name, 31);
    LetLin(name);
    blank = *screen & 0xFF00 | ' ';
    for (count = 0; count < 31; count++)
        screen[count] = blank;
    MovDz0(screen, name);
}

/* Write a number counted from 0 as one counted from 1, at the left of
   five cells of the screen buffer, in the colour in the high byte of
   `color`. */
void T75_00(u16 number, u16 *screen, u16 color)
{
    u8 *end = Number(md.TmpBuf, 0, number + 1, 0);
    u16 count;

    for (count = 5; count; count--)
        *end++ = ' ';
    MvsBw(screen, md.TmpBuf, 5, color);
}

/* Write six lines of the help that the editor shows for a menu file or
   an extension file, one below the other from `place` on: texts one
   behind the other at `text` in the segment `segment`, where `colors`
   are too. */
void T76_00(u16 place, u16 segment, u16 text, u16 colors)
{
    struct LineArgs line;
    u16 count;

    line.value = 0xFF;                  /* no limit to the width */
    line.line = AT(segment, text);
    line.colors = AT(segment, colors);
    line.include = line.line;           /* none of the lines has ^I */
    for (count = 6; count; count--) {
        line.screen = BasAdr(place);
        NgLine((struct LineArgs STACK *)&line);
        place += 0x100;
    }
}

/* Ask DOS for the attributes of the file. Returns them, with Archive
   set, and 0 in the high word; for a file that is not there, Archive
   alone. Anything else that DOS says is an error: that, and 1 in the
   high word. */
static u32 T80_01(void)
{
    struct DosRegs regs;

    regs.ax = 0x4300;
    regs.name = md.ViewFil;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax != ERR_NO_FILE)
            return PAIR(1, regs.ax);
        regs.cx = 0;
    }
    return regs.cx & 0x3F | 0x20;
}

/* The name of the file, as the box about a read-only file shows it. */
static void T80_Name(void)
{
    u8 *name = md.TmpBuf + 130;
    u16 count;

    for (count = 0; count < LenStr + 1; count++)
        name[count] = md.ViewFil[count];
    LetLin(name);
}

/* Write the file, with the attributes `attributes`. Returns 0, or the
   error and 1 in the high word.

   A read-only file is made writable first. The file is made anew, or
   emptied. If it is a device, nothing is done about attributes later; if
   that is the console, nothing is written, and that is no error. The
   text is written in pieces of at most 0FF00h bytes, an empty one as one
   piece of none, and a piece that DOS writes less of is the error of a
   disk that is full. The file is closed whatever happened, and gets its
   attributes back; that DOS denies access for that is no error.

   The registers go from one call of DOS to the next as it leaves them,
   but for AL where DOS does not read it. */
static u32 T80_02(u16 attributes)
{
    struct DosRegs regs;
    u32 left;
    u16 segment, error, failed;

    regs.name = md.ViewFil;
    regs.cx = attributes;
    if (attributes & 1) {
        regs.cx = attributes & 0xFFFE;
        regs.ax = 0x4301;
        if (Intr21((struct DosRegs STACK *)&regs))
            return PAIR(1, regs.ax);
        ed.EdWrite = 1;
    }
    regs.ax = 0x3C00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    ed.EdWrite = 1;
    regs.bx = regs.ax;
    regs.ax = 0x4400;
    failed = Intr21((struct DosRegs STACK *)&regs);
    error = regs.ax;
    if (!failed) {
        if ((u16)(u32)regs.name & 0x80) {
            if ((u16)(u32)regs.name & 2)
                goto close;
            attributes = 0xFFFF;
        }
        left = ed.EdLen;
        segment = md.ViewSeg;
        do {
            regs.cx = 0xFF00;
            if (HIGH(left) == 0 && LOW(left) < regs.cx)
                regs.cx = LOW(left);
            regs.name = AT(segment, 0);
            regs.ax = 0x4000;
            failed = Intr21((struct DosRegs STACK *)&regs);
            error = regs.ax;
            if (failed)
                goto close;
            if (regs.ax != regs.cx) {
                failed = regs.ax < regs.cx;
                error = ERR_DISK_FULL;
                goto close;
            }
            segment += 0xFF0;
            left -= regs.cx;
        } while (left != 0);
    }
close:
    regs.ax = 0x3E00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    if (failed)
        return PAIR(1, error);
    if (attributes == 0xFFFF)
        return 0;
    regs.cx = attributes;
    regs.name = md.ViewFil;
    regs.ax = 0x4301;
    if (Intr21((struct DosRegs STACK *)&regs)
        && regs.ax != ERR_NO_ACCESS)
        return PAIR(1, regs.ax);
    return 0;
}

/* Delete the file. Returns 0, or the error and 1 in the high word. */
static u32 T80_03(void)
{
    struct DosRegs regs;

    regs.ax = 0x4100;
    regs.name = md.ViewFil;
    if (Intr21((struct DosRegs STACK *)&regs))
        return PAIR(1, regs.ax);
    return 0;
}

/* If the file that was saved is the main extension file, the one in the
   directory of the program, take what the program knows of it from the
   text: its first 511 bytes, with an end-of-file character behind them.

   The name of the file is made a whole path for that, in capitals: the
   drive it names or else the current one, the current directory of that
   drive unless the name begins with a backslash, and the name, as far as
   81 characters go. Its directory must be NcPath and its name VC.EXT.

   Nothing is done if the file was not written. */
static void T80_14(void)
{
    struct DosRegs regs;
    const u8 *name = md.ViewFil;
    const u8 *file;
    const u8 __far *text;
    u8 *to;
    u16 letter, count, c;

    letter = *(const u16 *)name;
    if (letter >> 8 == ':')
        name += 2;
    else
        letter = letter & 0xFF00 | md.DosPth[0];
    letter = UpCase(letter);
    md.TmpBuf[0] = (u8)letter;
    md.TmpBuf[1] = ':';
    md.TmpBuf[2] = '\\';
    regs.ax = 0x4700;
    regs.name = AT(MAIN_DATA(), (u8)(letter - ('A' - 1)));
    regs.si = (u16)(md.TmpBuf + 3);
    if (Intr21((struct DosRegs STACK *)&regs))
        return;
    to = md.TmpBuf + 2;
    if (*name != '\\') {
        count = LenPath + 13;
        do
            c = *to++;
        while (--count && c != 0);
        to = (u8 *)(u16)(u32)AddFile(to - 1);
    }
    if (md.TmpBuf + LenPath + 13 <= to)
        return;
    for (count = md.TmpBuf + LenPath + 13 - to; count; count--)
        *to++ = *name++;
    PathLin(md.TmpBuf);
    file = (const u8 *)(u16)(u32)CutPath(md.TmpBuf);
    count = file - md.TmpBuf;
    for (c = 0; c < count; c++)
        if (md.TmpBuf[c] != md.NcPath[c])
            return;
    if (md.NcPath[count] != 0)
        return;
    for (c = 0; c < sizeof MainExt_F; c++)
        if (file[c] != MainExt_F[c])
            return;
    count = LenExt - 1;
    if (HIGH(ed.EdLen) == 0 && count > LOW(ed.EdLen))
        count = LOW(ed.EdLen);
    text = AT(md.ViewSeg, 0);
    for (c = 0; c < count; c++)
        md.ExtMain[c] = text[c];
    md.ExtMain[count] = 0x1A;
}

/* Save the text as the file ViewFil. Returns 0 if it is saved.

   With a box on the screen that says so: the line buffer goes back into
   the text, the user is asked whether to overwrite a file that is
   read-only, and the file is written. If that fails, another box says
   so, unless it is the drive that is not ready, which has had its own;
   and of a disk that is full the file is deleted again, with the box
   only if that goes well.

   Ported as it is: what follows the saving of the file, T80_14, is done
   whether it was saved or not. */
u16 T80_00(void)
{
    struct Box box;
    u32 answer;
    u16 failed = 1;

    T51_00();
    T50_00();
    DialBox(0x528, OFFSET(SavFi_P), (struct Box STACK *)&box);
    Window(box.begin, box.size);
    if (ChgDisk(md.ViewFil))
        goto done;
    answer = T80_01();
    if (HIGH(answer) == 0) {
        if (LOW(answer) & 1) {
            T80_Name();
            if ((u8)Dialog(OFFSET(Prot_B), OFFSET(EdFl_S)) != 0)
                goto done;
        }
        answer = T80_02(LOW(answer));
        if (HIGH(answer) == 0) {
            failed = 0;
            goto done;
        }
    }
    if (LOW(answer) == ERR_DISK_FULL) {
        answer = T80_03();
        if (HIGH(answer) == 0)
            goto report;
    }
    if (LOW(answer) == ERR_NOT_READY)
        goto done;
report:
    /* None of its lines has ^I: no include line. */
    Dialog(OFFSET(SavEr_B), 0);
done:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    T80_14();
    return failed;
}
