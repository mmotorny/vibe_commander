/*
 * The editor: where in the memory its text is, the lines of it, what it
 * shows of it, and how it saves it.
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
 * A line ends with a carriage return and a line feed, in either order; in
 * hex mode a line is 16 bytes, and the number of a line there is that of
 * its segment, counted from ViewSeg.
 *
 * What is still assembly of the editor, which is what it does with a key,
 * calls these for the addresses, for the line buffer, to get from a line
 * to the next and to the one before, for the column of the cursor, for a
 * line on the screen and to save the file. The viewer has its window where
 * the editor has, and clears and scrolls it with T70_00 and T71_00.
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

/* A line of the text, as the assembly has it in DS:SI and DI: the address
   of its first byte and its number, counted from 0. In the order in which
   the stubs of T53_00 and T54_00 have them on the stack. */
struct Where {
    u16 offset;
    u16 segment;
    u16 line;
};

#pragma pack(pop)

LAID_OUT(EdData, 138);

#define ed (*(struct EdData *)(md.TmpBuf + 100))

void T50_00(void);
void T51_00(void);
u16 T53_00(struct Where STACK *where);
u32 T53_10(u16 segment, u16 offset);
u16 T54_00(struct Where STACK *where);
u16 T55_00(u16 cursor);
void T56_00(u16 high, u16 low, u16 line, u16 target_high, u16 target_low);
u32 T57_00(void);
u32 T57_01(u16 limit);
u16 T57_02(void);
u32 T60_00(u16 segment, u16 offset);
u32 T61_00(u16 high, u16 low);
u32 T62_00(void);
u16 T63_01(u16 high, u16 low);
u16 T64_00(const u8 __far *name, const u8 *text, u16 count);
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
#pragma aux T50_00 "*_c" __far modify exact [ax]
#pragma aux T51_00 "*_c" __far modify exact [ax]
#pragma aux T53_00 "*_c" __far parm [bx] value [ax] modify exact [ax]
#pragma aux T53_10 "*_c" __far parm [dx] [si] value [dx ax] modify exact [ax dx]
#pragma aux T54_00 "*_c" __far parm [bx] value [ax] modify exact [ax]
#pragma aux T55_00 "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux T56_00 "*_c" __far parm [dx] [bx] [di] [cx] [ax] modify exact [ax bx cx dx si di]
#pragma aux T57_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T57_01 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux T57_02 "*_c" __far value [ax] modify exact [ax dx]
#pragma aux T60_00 "*_c" __far parm [ax] [si] value [dx ax] modify exact [ax dx]
#pragma aux T61_00 "*_c" __far parm [dx] [bx] value [dx ax] modify exact [ax dx]
#pragma aux T62_00 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T63_01 "*_c" __far parm [dx] [bx] value [ax] modify exact [ax]
#pragma aux T64_00 "*_c" __far parm [dx si] [di] [cx] value [ax] modify exact [ax]
#pragma aux T70_00 "*_c" __far parm [ax] value [dx ax] modify exact [ax dx]
#pragma aux T71_00 "*_c" __far parm [ax] [bx] [dx] modify exact [ax bx cx dx si di]
#pragma aux T72_00 "*_c" __far parm [ax] [dx] [si] [bx] modify exact [ax]
#pragma aux T73_00 "*_c" __far modify exact [ax]
#pragma aux T73_10 "*_c" __far value [dx ax] modify exact [ax dx]
#pragma aux T74_00 "*_c" __far modify exact [ax cx si di]
#pragma aux T75_00 "*_c" __far parm [cx] [di] [ax] modify exact [ax]
#pragma aux T76_00 "*_c" __far parm [bx] [dx] [si] [ax] modify exact [ax]
#pragma aux T80_00 "*_c" __far value [ax] modify exact [ax bx cx dx si di]

/* Routines of the rest of the program that are still assembly, by their
   entries for C. ResWin puts back in the screen buffer what a box
   covers, Window shows it. ChgDisk asks for the disk of a drive that
   shares the floppy drive with another, and is not 0 if the user would
   not change it. Int10m calls the BIOS for the screen, with the mouse
   pointer out of the way. CmpSym is not 0 for a character that stands
   between words: a space, a tab or one of +:.,><|\ */
void ResWin(u16 begin, u16 size, u16 saved);
u16 ChgDisk(const u8 *path);
void Int10m(u16 ax, u16 bx, u16 cx, u16 dx);
u16 CmpSym(u16 symbol);
#pragma aux ResWin  "*_a" __far parm [bx] [dx] [si] modify exact [es]
#pragma aux ChgDisk "*_a" __far parm [dx] value [ax] modify exact [ax es]
#pragma aux Int10m  "*_a" __far parm [ax] [bx] [cx] [dx] modify exact [ax bx cx dx es]
#pragma aux CmpSym  "*_a" __far parm [ax] value [ax] modify exact [ax]

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
#define ENDS_LINE(word) ((word) == CR_LF || (word) == LF_CR)

#define PIECE 0xFF00                    /* bytes moved at a time, */
#define PIECE_SEGMENTS 0x0FF0           /* and that in segments */

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

/* Fill the line buffer from the text: with the line at EdOffst, without
   the two characters that end it, or with as much of it as the editor
   keeps of a line, 3900 characters. EdEndBf is where in the text the
   buffer ends. Not in hex mode, which has no use for the buffer.

   The buffer gets 3900 bytes of the text whatever the length of the
   line. */
void T50_00(void)
{
    u32 address, end;
    const u8 __far *text;
    u16 count, size;

    if (ed.EdTxHex != 0)
        return;
    address = ADDRESS_OF(HIGH(ed.EdOffst), LOW(ed.EdOffst));
    text = AT(HIGH(address), LOW(address));
    for (count = 0; count < LenEdit - 100; count++)
        EditBuf[count] = text[count];
    ed.EdWhole = 1;
    size = 0;
    if (T63_01(HIGH(ed.EdOffst), LOW(ed.EdOffst)) == 0) {
        end = T53_10(HIGH(address), LOW(address));
        if (HIGH(end) == 0) {
            size = LOW(end) - LOW(address);
            if (size >= 2
                && ENDS_LINE(*(const u16 __far *)
                             AT(HIGH(address), LOW(end) - 2)))
                size -= 2;
        }
        if (HIGH(end) != 0 || size > LenEdit - 100) {
            size = LenEdit - 100;
            ed.EdWhole = 0;
        }
    }
    ed.EdEndBf = ed.EdOffst + size;
    ed.EdSizLn = size;
    ed.EdUpdat = 0;
}

/* Copy bytes of the text to another place in it, which may be more than
   a segment: upwards from the addresses given, or downwards, a piece at
   a time. */
static void T51_30(u16 from_segment, u16 from, u16 to_segment, u16 to,
                   u32 count, u16 down)
{
    const u8 __far *source;
    u8 __far *target;
    u16 piece;

    for (;;) {
        piece = count >= PIECE ? PIECE : LOW(count);
        source = AT(from_segment, from);
        target = AT(to_segment, to);
        if (down) {
            for (; piece; piece--)
                *target-- = *source--;
        } else {
            for (; piece; piece--)
                *target++ = *source++;
        }
        if (count < PIECE)
            break;
        count -= PIECE;
        if (down) {
            from_segment -= PIECE_SEGMENTS;
            to_segment -= PIECE_SEGMENTS;
        } else {
            from_segment += PIECE_SEGMENTS;
            to_segment += PIECE_SEGMENTS;
        }
    }
}

/* Put the line buffer back into the text, if it has been changed: in
   place of what the text has from EdOffst to EdEndBf. What follows that
   in the text is moved down or up first, if the buffer holds less or more
   than was there.

   If the memory has no room for more, the editor beeps, and the text ends
   with the last byte but one of the memory. Ported as it is: what follows
   is moved up by as much as was asked for all the same, so that its end
   is lost and not the end of the line; of the line only what would go
   beyond the memory by itself is cut off. */
void T51_00(void)
{
    u32 old, less, end, address, tail, source;
    const u8 *buffer;
    u8 __far *text;
    u16 size, more, grown, segment, offset, limit, count;

    if (ed.EdTxHex != 0 || ed.EdUpdat == 0)
        return;
    ed.EdUpdat = 0;
    ed.EdChng = 1;
    old = ed.EdEndBf - ed.EdOffst;
    size = ed.EdSizLn;
    if (HIGH(old) != 0 || LOW(old) > size) {
        less = old - size;
        address = ed.EdOffst + size;
        address = ADDRESS_OF(HIGH(address), LOW(address));
        source = ADDRESS_OF(HIGH(ed.EdEndBf), LOW(ed.EdEndBf));
        T51_30(HIGH(source), LOW(source), HIGH(address), LOW(address),
               ed.EdLen - ed.EdEndBf, 0);
        ed.EdLen -= less;
    } else if (LOW(old) < size) {
        more = size - LOW(old);
        grown = more;
        end = ed.EdLen + more;
        address = ADDRESS_OF(HIGH(end), LOW(end));
        segment = HIGH(address);
        offset = LOW(address);
        limit = ed.EdSpace + md.ViewSeg;
        if (segment >= limit) {
            ed.EdScrn = 1;
            Beep();
            segment = limit - 1;
            offset = 0x0F;
            end = OFFSET_OF(segment, offset);
            if (HIGH(end) == HIGH(ed.EdOffst)
                && (u16)(LOW(end) - LOW(ed.EdOffst)) < ed.EdSizLn)
                ed.EdSizLn = LOW(end) - LOW(ed.EdOffst);
            grown = LOW(end) - LOW(ed.EdLen);
        }
        /* The last byte to write and the last to read, as addresses with
           offsets high enough to count down from by a piece. */
        tail = end - more;
        if (tail >= ed.EdEndBf) {
            source = ADDRESS_OF(HIGH(tail), LOW(tail));
            T51_30(HIGH(source) - 0x0FFF, LOW(source) + 0xFFF0 - 1,
                   segment - 0x0FFF, offset - 0x10 - 1,
                   tail - ed.EdEndBf, 1);
        }
        ed.EdLen += grown;
    }
    address = ADDRESS_OF(HIGH(ed.EdOffst), LOW(ed.EdOffst));
    text = AT(HIGH(address), LOW(address));
    buffer = EditBuf;
    for (count = ed.EdSizLn; count; count--)
        *text++ = *buffer++;
}

/* Where the line ends that the byte at an address is in, as an offset in
   the same segment: behind the two characters that end it, or where the
   text ends; in hex mode behind its 16 bytes, or where the text ends. The
   high word is 0 then. It is 1, with the offset that was given, if the
   address is at the end of the text or behind it, or if the line does not
   end in that segment. */
u32 T53_10(u16 segment, u16 offset)
{
    u32 place = OFFSET_OF(segment, offset);
    u32 left;
    u16 start = offset;
    u16 limit, word;

    if (T63_01(HIGH(place), LOW(place)))
        return PAIR(1, start);
    if (ed.EdTxHex != 0) {
        offset = (offset | 0x0F) + 1;
        if (offset == 0)
            return PAIR(1, start);
        place = OFFSET_OF(segment, offset);
        if (T63_01(HIGH(place), LOW(place)))
            offset -= LOW(place) - LOW(ed.EdLen);
        return offset;
    }
    /* As far as the text goes, or the segment. */
    left = ed.EdLen - place;
    limit = 0xFFFE;
    if (HIGH(left) == 0 && (u16)(LOW(left) + offset) >= offset
        && (u16)(LOW(left) + offset) < 0xFFFE)
        limit = LOW(left) + offset;
    while (offset < limit) {
        word = *(const u16 __far *)AT(segment, offset);
        offset += 2;
        if (offset <= limit && ENDS_LINE(word))
            return offset;
        offset--;
    }
    place = OFFSET_OF(segment, offset);
    if (T63_01(HIGH(place), LOW(place)))
        return offset;
    return PAIR(1, start);
}

/* Go to the next line of the text. Returns 0, with `where` on it, or 1
   and `where` as it was if there is none: at the end of the text, and
   after a line that the text ends in without the two characters. In hex
   mode the next line is the next 16 bytes, if the text has that many.

   A line that does not end in the segment of its address is looked at
   again from the address with the lowest offset, and from there on a
   piece further on every time. */
u16 T53_00(struct Where STACK *where)
{
    u16 segment = where->segment;
    u16 offset = where->offset;
    u32 place, start, end, address;

    if (where->line == 0xFFFF)
        return 1;
    place = OFFSET_OF(segment, offset);
    if (T63_01(HIGH(place), LOW(place)))
        return 1;
    if (ed.EdTxHex != 0) {
        end = T53_10(segment, offset);
        if (HIGH(end) != 0) {
            address = ADDRESS_OF(HIGH(place), LOW(place));
            segment = HIGH(address);
            end = T53_10(segment, LOW(address));
        }
        offset = LOW(end);
        if (offset & 0x0F)
            return 1;
    } else {
        start = place;
        for (;;) {
            end = T53_10(segment, offset);
            if (HIGH(end) == 0)
                break;
            place = OFFSET_OF(segment, offset);
            address = ADDRESS_OF(HIGH(place), LOW(place));
            segment = HIGH(address);
            offset = LOW(address);
            end = T53_10(segment, offset);
            if (HIGH(end) == 0)
                break;
            segment += PIECE_SEGMENTS;
        }
        offset = LOW(end);
        place = OFFSET_OF(segment, offset);
        if (place - start <= 1)
            return 1;
        if (!ENDS_LINE(*(const u16 __far *)AT(segment, offset - 2)))
            return 1;
    }
    where->segment = segment;
    where->offset = offset;
    where->line++;
    return 0;
}

/* Go to the line before. Returns 0, with `where` on it, or 1 and `where`
   as it was if there is none: in the first line. In hex mode the line
   before is 16 bytes back.

   As text, the bytes before the line are looked through backwards, at
   most 32K at a time, for the two characters that end a line before the
   two that end the line before this one. Lines of nothing but those two
   may follow one another there, and are passed over as long as they do;
   so from where that stopped the lines are taken forwards again until the
   next one would be this line. */
u16 T54_00(struct Where STACK *where)
{
    struct Where line;
    u16 segment = where->segment;
    u16 offset = where->offset;
    u32 place, from, next, address;
    u16 found = 0, checked, pair, word, back;

    if (where->line == 0)
        return 1;
    if (ed.EdTxHex != 0) {
        if (segment <= md.ViewSeg)
            return 1;
        where->segment = segment - 1;
        where->line--;
        return 0;
    }
    if (segment <= md.ViewSeg && offset == 0)
        return 1;
    place = OFFSET_OF(segment, offset);
    checked = offset > 2;
    offset -= 2;
    for (;;) {
        if (!checked || offset < 2) {
            /* The segment has no more: take one that is lower. */
            back = segment - md.ViewSeg;
            if (back == 0) {
                offset = 0;
                break;
            }
            if (back > 0x800)
                back = 0x800;
            segment -= back;
            offset += back << 4;
        }
        checked = 1;
        word = *(const u16 __far *)AT(segment, offset - 2);
        if (!found) {
            if (ENDS_LINE(word)) {
                found = 1;
                pair = word;
            }
        } else {
            pair = pair << 8 | pair >> 8;
            if (word != pair) {
                offset++;
                break;
            }
        }
        offset--;
    }
    from = OFFSET_OF(segment, offset);
    for (;;) {
        address = ADDRESS_OF(HIGH(from), LOW(from));
        line.segment = HIGH(address);
        line.offset = LOW(address);
        line.line = 0;
        T53_00((struct Where STACK *)&line);
        next = OFFSET_OF(line.segment, line.offset);
        if (next >= place)
            break;
        from = next;
    }
    address = ADDRESS_OF(HIGH(from), LOW(from));
    where->segment = HIGH(address);
    where->offset = LOW(address);
    where->line--;
    return 0;
}

/* Whether the line buffer has a character at `cursor`. If it has not,
   and does not hold the whole line, it is first put back into the text
   and filled again. */
static u16 T55_10(u16 cursor)
{
    if (cursor >= ed.EdSizLn && ed.EdWhole == 0) {
        T51_00();
        T50_00();
    }
    return cursor < ed.EdSizLn;
}

/* Where the next word is, from the character `cursor` of the line buffer
   on: behind the characters that stand between words, if it begins with
   such, and behind those that do not, up to the next one that does or to
   the end of what the buffer has. The first character itself is not
   looked at. */
u16 T55_00(u16 cursor)
{
    for (;;) {
        if (!T55_10(cursor))
            return cursor;
        cursor++;
        if (!CmpSym(EditBuf[cursor]))
            break;
    }
    for (;;) {
        if (!T55_10(cursor))
            return cursor;
        cursor++;
        if (CmpSym(EditBuf[cursor]))
            return cursor;
    }
}

/* Put the cursor on the byte of the text at the offset `target`, given a
   line that begins at or before it: where it begins and its number. The
   lines are taken one after the other from there, as far as they begin
   at or before the byte; the last of them is the cursor's, and goes into
   the line buffer. In it the cursor is no further than the editor goes
   in a line, and in the last character that it keeps of one if the byte
   is 64K or more into the line. */
void T56_00(u16 high, u16 low, u16 line, u16 target_high, u16 target_low)
{
    struct Where where;
    u32 place = PAIR(high, low);
    u32 target = PAIR(target_high, target_low);
    u32 address = ADDRESS_OF(high, low);
    u16 cursor, last;

    where.segment = HIGH(address);
    where.offset = LOW(address);
    where.line = line;
    for (;;) {
        ed.EdOffst = place;
        ed.EdLine = where.line;
        if (T53_00((struct Where STACK *)&where))
            break;
        place = OFFSET_OF(where.segment, where.offset);
        if (place > target)
            break;
    }
    T50_00();
    target -= ed.EdOffst;
    cursor = HIGH(target) != 0 ? 0xFFFF : LOW(target);
    last = ed.EdSizLn - HIGH(T57_00());
    if (cursor > last)
        cursor = last;
    ed.EdCursr = cursor;
}

/* Whether `count` bytes at `name` and at `text` are the same. */
u16 T64_00(const u8 __far *name, const u8 *text, u16 count)
{
    for (; count; count--)
        if (*name++ != *text++)
            return 0;
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
        end = LOW(T53_10(HIGH(address), LOW(address)));
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
    /* None of its lines has ^I: no include line. */
    DialBox(0x528, OFFSET(SavFi_P), 0, MAIN_SEG, (struct Box STACK *)&box);
    Window(box.begin, box.size);
    if (ChgDisk(md.ViewFil))
        goto done;
    answer = T80_01();
    if (HIGH(answer) == 0) {
        if (LOW(answer) & 1) {
            T80_Name();
            if ((u8)Dialog(OFFSET(Prot_B), OFFSET(EdFl_S), 0, 0,
                           MAIN_SEG) != 0)
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
    Dialog(OFFSET(SavEr_B), 0, 0, 0, MAIN_SEG);
done:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    T80_14();
    return failed;
}
