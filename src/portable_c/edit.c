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
 * The editor has no procedure of its own in the assembly: the viewer's
 * code jumps to T00_00 once it has opened the file, and the editor jumps
 * back there when it is left. T00_00 here is that code, as far as the
 * last of what a key does: it reads the file, draws the status line and
 * the help of a menu or extension file, and then takes keys, with the
 * screen brought up to date before each. What a key does is a function
 * of the name that its code has in the assembly, and so is every place
 * there that others jump to; a function is over where the assembly goes
 * back for the next key. The viewer has its window where the editor has,
 * and clears and scrolls it with T70_00 and T71_00.
 */
#include "vc.h"

#define LenEdit LenMenu                 /* Length of edit buffer */
#define EditBuf md.MenuBuf              /* Edit line buffer */
#define UndoBuf ((u8 *)md.ErrBuf)       /* Undo buffer */

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
   the assembly had them on the stack where it called T53_00 and
   T54_00. */
struct Where {
    u16 offset;
    u16 segment;
    u16 line;
};

#pragma pack(pop)

LAID_OUT(EdData, 138);

/* The address of a byte of the text. One that AddressOf has made of an
   offset in the text has an offset of less than 16. */
struct Address {
    u16 offset;
    u16 segment;
};

/* What T53_10 returns: where a line ends. */
struct Ending {
    u16 at;                             /* behind the line: an offset in
                                           the segment of the address that
                                           was given; with none, the offset
                                           that was given */
    u16 none;                           /* 1 if the line does not end in
                                           that segment, or the address is
                                           at the end of the text or behind
                                           it */
};

/* What Column, T57_00 and T57_01 return: how far characters of the line
   buffer go, counted up to a column that is the limit. */
struct Reached {
    u16 column;                         /* the column that they end in; if
                                           they reach the limit, the one
                                           that the character begins in
                                           which reaches it */
    u16 left;                           /* how many of them were not
                                           counted, because the limit was
                                           reached: that character and
                                           those behind it */
};

/* What T73_10 returns: the line that it has written, as Window takes a
   part of the screen. */
struct Drawn {
    u16 begin;                          /* its row, in the high byte, and
                                           its column */
    u16 size;                           /* one row, and the columns of the
                                           window */
};

#define ed (*(struct EdData *)(md.TmpBuf + 100))

/* Which help the editor shows below its window: 1 for the menu file, 2
   for an extension file. A name for the first byte of EdOffst, which is
   set when the help has been drawn. */
#define EdHelp (*(u8 *)&ed.EdOffst)

void T50_00(void);
void T51_00(void);
u16 T53_00(struct Where STACK *where);
struct Ending T53_10(u16 segment, u16 offset);
u16 T54_00(struct Where STACK *where);
u16 T55_00(u16 cursor);
void T56_00(u16 high, u16 low, u16 line, u16 target_high, u16 target_low);
struct Reached T57_00(void);
struct Reached T57_01(u16 limit);
u16 T57_02(void);
struct Address T62_00(void);
u16 T63_01(u16 high, u16 low);
u16 T64_00(const u8 __far *name, const u8 *text, u16 count);
void T71_00(u16 function, u16 place, u16 step);
void T72_00(u16 line, u16 segment, u16 from, u16 end);
void T73_00(void);
struct Drawn T73_10(void);
void T74_00(void);
void T75_00(u16 number, u16 *screen, u16 color);
void T76_00(u16 place, u16 segment, u16 text, u16 colors);
u16 T80_00(void);

/* T00_00, the editor itself, is declared in vc.h. Of the others the
   viewer calls T70_00 and T71_00. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   ResWin puts back in the screen buffer what a box covers, Window shows
   it. ChgDisk asks for the disk of a drive that shares the floppy drive
   with another, and is not 0 if the user would not change it. Int10m calls
   the BIOS for the screen, with the mouse pointer out of the way.

   HidCur hides the cursor and Cursor puts it at a row, in the high byte,
   and a column; ChgCur gives it the editor's shape and returns the one it
   had, which CurShp gives it back. ClrKbd empties the keyboard buffer, if
   the setup has it so. GetMous returns the buttons of the mouse that are down
   and where it is; what it is given comes back for that if there is no
   mouse. InWind is not 0 if that is outside a window. AsmSeg
   is the segment of the program's constants. */

/* C routines of other files that vc.h does not have. */
u16 __far *HexByt(u16 __far *screen, u16 byte);         /* digits.c */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);                            /* screen.c */
void PathLin(u8 __far *text);                           /* string.c */
/* The viewer's search: V80_00 shows the box "Searching for" under the
   title `include`, says where it is, and makes ready what is looked for,
   SrchStr; V82_00, of vc.h, looks for it in `count` bytes, and returns
   where the search ended, behind what it found, and 1 for the text
   found, 0 for the bytes that it names in hex, and 100h for neither. */
void V80_00(u16 include, struct Box STACK *where);      /* view.c */

/* How long the text is that V80_00 has made ready. */
#define SrchLen (*(u16 *)(md.HelpBuf + 90))

/* Boxes, dialogs and texts of edit_t.c and view_t.c, and
   three of other files: Prot_B, Menu_F and Ext_F. DialBox and Dialog
   take theirs by their offsets in the segment that AsmSeg returns, and
   only those are used here: a text is read at its offset in AsmSeg. */
extern const u8 __far SavFi_P[];        /* "Saving the file" */
extern const u8 __far SavEr_B[];        /* "There was an error writing..." */
extern const u8 __far Prot_B[];         /* "... is a read-only file" */
extern const u8 __far EdFl_S[];         /* " Edit " */
extern const u8 __far ViFl_S[];         /* " View " */
extern const u8 __far BigFi_B[];        /* "... is too large for Edit" */
extern const u8 __far NewFi_B[];        /* "Can't find the file" */
extern const u8 __far SavAs_B[];        /* "Save this file as" */
extern const u8 __far SavCn_B[];        /* "You've made changes ..." */
extern const u8 __far Srch_B[];         /* "Search for the string" */
extern const u8 __far SrEr_B[];         /* "Could not find the string" */
extern const u8 __far Menu_F[];         /* the names of the menu file and */
extern const u8 __far Ext_F[];          /* of the three extension files */
extern const u8 __far ViewE_F[];
extern const u8 __far EditE_F[];
extern const u8 __far Line_S[];         /* "Line", "Col" and "Free" of the */
extern const u8 __far Col_S[];          /* status line */
extern const u8 __far FrBuf_S[];
extern const u8 __far EdtH_P[];         /* the colour of the help, */
extern const u8 __far EdtH_S[];         /* its six lines on the right, */
extern const u8 __far EdtH1_S[];        /* and for the menu file and for */
extern const u8 __far EdtH2_S[];        /* an extension file the title
                                           and six lines on the left */

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

/* The name of the main extension file. */
static const u8 CODE MainExt_F[] = "VC.EXT";

#define Menu_C  7                       /* the colours of the status line, */
#define HlBrd_C 25                      /* of the rows around the help, */
#define Arrow_C 35                      /* and of the arrows at the ends of
                                           a line that goes on */

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

/* The offset in the text of the byte at an address. */
#define OFFSET_OF(segment, offset) \
    (((u32)(u16)((segment) - md.ViewSeg) << 4) + (offset))

/* The address of the byte at an offset in the text: its segment, and for
   the offset what is left, less than 16. Of the offset in the text 20
   bits count. */
static struct Address AddressOf(u32 place)
{
    struct Address address;
    u16 high = HIGH(place);
    u16 low = LOW(place);

    address.segment = (low >> 4 | high << 12) + md.ViewSeg;
    address.offset = low & 0x0F;
    return address;
}

/* The column that `count` characters of the line buffer end in, with
   tabs to every eighth, and how many of them were not counted, because
   the column `limit` was reached: none, if it was not. */
static struct Reached Column(u16 count, u16 limit)
{
    struct Reached reached;
    const u8 *text = EditBuf;
    u16 column = 0;
    u16 before;

    reached.column = 0;
    reached.left = 0;
    if (count == 0)
        return reached;
    do {
        before = column;
        if (*text++ == TAB)
            column |= 7;
        column++;
        if (column >= limit) {
            reached.column = before;
            reached.left = count;
            return reached;
        }
    } while (--count);
    reached.column = column;
    return reached;
}

/* The column that the line in the buffer ends in, as far as the buffer
   has it, but no further than the last that the editor goes to, and how
   many of its characters are behind that; and T57_01, no further than
   `limit`. */
struct Reached T57_00(void)
{
    return Column(ed.EdSizLn, LenEdit - 100);
}

struct Reached T57_01(u16 limit)
{
    return Column(ed.EdSizLn, limit);
}

/* The column of the cursor, counted from 0: in hex mode the number of the
   byte in its line. */
u16 T57_02(void)
{
    if (ed.EdTxHex != 0)
        return ed.EdCursr;
    return Column(ed.EdCursr, 0xFFFF).column;
}

/* The address of the first line on the screen. T60_00 and T61_00 of the
   assembly, which make an offset of an address and an address of an
   offset, are OFFSET_OF and AddressOf. */
struct Address T62_00(void)
{
    return AddressOf(ed.EdFirst);
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
    struct Address address;
    struct Ending end;
    const u8 __far *text;
    u16 count, size;

    if (ed.EdTxHex != 0)
        return;
    address = AddressOf(ed.EdOffst);
    text = AT(address.segment, address.offset);
    for (count = 0; count < LenEdit - 100; count++)
        EditBuf[count] = text[count];
    ed.EdWhole = 1;
    size = 0;
    if (T63_01(HIGH(ed.EdOffst), LOW(ed.EdOffst)) == 0) {
        end = T53_10(address.segment, address.offset);
        if (!end.none) {
            size = end.at - address.offset;
            if (size >= 2
                && ENDS_LINE(*(const u16 __far *)
                             AT(address.segment, end.at - 2)))
                size -= 2;
        }
        if (end.none || size > LenEdit - 100) {
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
    u32 old, less, end, tail;
    struct Address address, source;
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
        address = AddressOf(ed.EdOffst + size);
        source = AddressOf(ed.EdEndBf);
        T51_30(source.segment, source.offset, address.segment,
               address.offset, ed.EdLen - ed.EdEndBf, 0);
        ed.EdLen -= less;
    } else if (LOW(old) < size) {
        more = size - LOW(old);
        grown = more;
        end = ed.EdLen + more;
        address = AddressOf(end);
        segment = address.segment;
        offset = address.offset;
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
            source = AddressOf(tail);
            T51_30(source.segment - 0x0FFF, source.offset + 0xFFF0 - 1,
                   segment - 0x0FFF, offset - 0x10 - 1,
                   tail - ed.EdEndBf, 1);
        }
        ed.EdLen += grown;
    }
    address = AddressOf(ed.EdOffst);
    text = AT(address.segment, address.offset);
    buffer = EditBuf;
    for (count = ed.EdSizLn; count; count--)
        *text++ = *buffer++;
}

/* What T53_10 returns: a line that ends at an offset, or one that does
   not, with the offset that T53_10 was given. */
static struct Ending EndsAt(u16 offset)
{
    struct Ending ending;

    ending.at = offset;
    ending.none = 0;
    return ending;
}

static struct Ending NoEnd(u16 start)
{
    struct Ending ending;

    ending.at = start;
    ending.none = 1;
    return ending;
}

/* Where the line ends that the byte at an address is in, as an offset in
   the same segment: behind the two characters that end it, or where the
   text ends; in hex mode behind its 16 bytes, or where the text ends.
   There is no such place, and the offset that was given comes back, if
   the address is at the end of the text or behind it, or if the line
   does not end in that segment. */
struct Ending T53_10(u16 segment, u16 offset)
{
    u32 place = OFFSET_OF(segment, offset);
    u32 left;
    u16 start = offset;
    u16 limit, word;

    if (T63_01(HIGH(place), LOW(place)))
        return NoEnd(start);
    if (ed.EdTxHex != 0) {
        offset = (offset | 0x0F) + 1;
        if (offset == 0)
            return NoEnd(start);
        place = OFFSET_OF(segment, offset);
        if (T63_01(HIGH(place), LOW(place)))
            offset -= LOW(place) - LOW(ed.EdLen);
        return EndsAt(offset);
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
            return EndsAt(offset);
        offset--;
    }
    place = OFFSET_OF(segment, offset);
    if (T63_01(HIGH(place), LOW(place)))
        return EndsAt(offset);
    return NoEnd(start);
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
    u32 place, start;
    struct Ending end;
    struct Address address;

    if (where->line == 0xFFFF)
        return 1;
    place = OFFSET_OF(segment, offset);
    if (T63_01(HIGH(place), LOW(place)))
        return 1;
    if (ed.EdTxHex != 0) {
        end = T53_10(segment, offset);
        if (end.none) {
            address = AddressOf(place);
            segment = address.segment;
            end = T53_10(segment, address.offset);
        }
        offset = end.at;
        if (offset & 0x0F)
            return 1;
    } else {
        start = place;
        for (;;) {
            end = T53_10(segment, offset);
            if (!end.none)
                break;
            place = OFFSET_OF(segment, offset);
            address = AddressOf(place);
            segment = address.segment;
            offset = address.offset;
            end = T53_10(segment, offset);
            if (!end.none)
                break;
            segment += PIECE_SEGMENTS;
        }
        offset = end.at;
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
    u32 place, from, next;
    struct Address address;
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
        address = AddressOf(from);
        line.segment = address.segment;
        line.offset = address.offset;
        line.line = 0;
        T53_00((struct Where STACK *)&line);
        next = OFFSET_OF(line.segment, line.offset);
        if (next >= place)
            break;
        from = next;
    }
    address = AddressOf(from);
    where->segment = address.segment;
    where->offset = address.offset;
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
    struct Address address = AddressOf(place);
    u16 cursor, last;

    where.segment = address.segment;
    where.offset = address.offset;
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
    last = ed.EdSizLn - T57_00().left;
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
   `line` counts. Returns where the line is in the buffer, and its row
   and column on the screen. */
struct Cleared T70_00(u16 line)
{
    struct Cleared cleared;
    u16 place = ed.EdBase + (line << 8);
    u16 *screen = BasAdr(place);
    u16 *cell = screen;
    u16 blank = Color(Back_C) << 8 | ' ';
    u16 count;

    for (count = ed.EdSizeX; count; count--)
        *cell++ = blank;
    cleared.screen = screen;
    cleared.place = place;
    return cleared;
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
    u16 *screen = T70_00(line).screen;
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
   where it is on the screen and its size, one row.

   As text it is the line buffer that is written. If the buffer does not
   hold the whole line and has fewer characters behind the cursor than
   the window has columns to the right of it, it is first put back into
   the text and filled again. A line of no characters is cleared and has
   no arrow in its first column. */
struct Drawn T73_10(void)
{
    struct Drawn drawn;
    u16 line = ed.EdLine - ed.EdLin0;
    u16 place = ed.EdBase + (line << 8);
    struct Address address;
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
        address = AddressOf(ed.EdOffst);
        end = T53_10(address.segment, address.offset).at;
        T72_00(line, address.segment, address.offset, end);
    }
    drawn.begin = place;
    drawn.size = 0x100 | ed.EdSizeX;
    return drawn;
}

/* Write the line that the cursor is in, and show it. */
void T73_00(void)
{
    struct Drawn drawn = T73_10();

    Window(drawn.begin, drawn.size);
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

/* What T80_01, T80_02 and T80_03 return, as struct Answer of vc.h has
   it: what was asked for, or an error. */
static struct Answer Done(u16 value)
{
    struct Answer answer;

    answer.value = value;
    answer.failed = 0;
    return answer;
}

static struct Answer Failed(u16 error)
{
    struct Answer answer;

    answer.value = error;
    answer.failed = 1;
    return answer;
}

/* Ask DOS for the attributes of the file. Returns them, with Archive
   set; for a file that is not there, Archive alone. Anything else that
   DOS says is an error, and that is returned. */
static struct Answer T80_01(void)
{
    struct DosRegs regs;

    regs.ax = 0x4300;
    regs.name = md.ViewFil;
    if (Intr21((struct DosRegs STACK *)&regs)) {
        if (regs.ax != ERR_NO_FILE)
            return Failed(regs.ax);
        regs.cx = 0;
    }
    return Done(regs.cx & 0x3F | 0x20);
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
   error.

   A read-only file is made writable first. The file is made anew, or
   emptied. If it is a device, nothing is done about attributes later; if
   that is the console, nothing is written, and that is no error. The
   text is written in pieces of at most 0FF00h bytes, an empty one as one
   piece of none, and a piece that DOS writes less of is the error of a
   disk that is full. The file is closed whatever happened, and gets its
   attributes back; that DOS denies access for that is no error.

   The registers go from one call of DOS to the next as it leaves them,
   but for AL where DOS does not read it. */
static struct Answer T80_02(u16 attributes)
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
            return Failed(regs.ax);
        ed.EdWrite = 1;
    }
    regs.ax = 0x3C00;
    if (Intr21((struct DosRegs STACK *)&regs))
        return Failed(regs.ax);
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
        return Failed(regs.ax);
    if (failed)
        return Failed(error);
    if (attributes == 0xFFFF)
        return Done(0);
    regs.cx = attributes;
    regs.name = md.ViewFil;
    regs.ax = 0x4301;
    if (Intr21((struct DosRegs STACK *)&regs)
        && regs.ax != ERR_NO_ACCESS)
        return Failed(regs.ax);
    return Done(0);
}

/* Delete the file. Returns 0, or the error. */
static struct Answer T80_03(void)
{
    struct DosRegs regs;

    regs.ax = 0x4100;
    regs.name = md.ViewFil;
    if (Intr21((struct DosRegs STACK *)&regs))
        return Failed(regs.ax);
    return Done(0);
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
    struct Answer answer;
    u16 failed = 1;

    T51_00();
    T50_00();
    /* None of its lines has ^I: no include line. */
    DialBox(0x528, OFFSET(SavFi_P), 0, MAIN_SEG, (struct Box STACK *)&box);
    Window(box.begin, box.size);
    if (ChgDisk(md.ViewFil))
        goto done;
    answer = T80_01();
    if (!answer.failed) {
        if (answer.value & 1) {
            T80_Name();
            if ((u8)Dialog(OFFSET(Prot_B), OFFSET(EdFl_S), 0, 0,
                           MAIN_SEG) != 0)
                goto done;
        }
        answer = T80_02(answer.value);
        if (!answer.failed) {
            failed = 0;
            goto done;
        }
    }
    if (answer.value == ERR_DISK_FULL) {
        answer = T80_03();
        if (!answer.failed)
            goto report;
    }
    if (answer.value == ERR_NOT_READY)
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

/* The line that the cursor is in, as T53_00 and T54_00 take one. */
static void T60_10(struct Where STACK *where)
{
    struct Address address = AddressOf(ed.EdOffst);

    where->segment = address.segment;
    where->offset = address.offset;
    where->line = ed.EdLine;
}

/* Make the line `where` the one that the cursor is in. */
static void T60_20(const struct Where STACK *where)
{
    ed.EdLine = where->line;
    ed.EdOffst = OFFSET_OF(where->segment, where->offset);
}

/* Put the cursor in the column `limit` of the line, counted from 1, or
   at the end of what the buffer has of the line if that is shorter. The
   buffer is filled first. */
static void T14_12(u16 limit)
{
    T50_00();
    ed.EdCursr = ed.EdSizLn - T57_01(limit).left;
}

/* End, where the line is one that the cursor has just come to: as text,
   to the last column that the editor goes to or to the end of the line;
   in hex, to the last byte of the 16, or behind the last one there is. */
static void T14_11(void)
{
    struct Address address;
    struct Ending end;
    u16 cursor;

    if (ed.EdTxHex == 0) {
        T14_12(LenEdit - 100);
        return;
    }
    ed.EdHxCur = 0;                     /* T14_00 */
    address = AddressOf(ed.EdOffst);
    end = T53_10(address.segment, address.offset);
    cursor = end.at;
    if (!end.none) {
        cursor &= 0x0F;
        if (cursor == 0) {
            cursor = 0x0F;
            ed.EdHxCur++;
        }
    }
    ed.EdCursr = cursor;
}

/* End. As text the buffer is put back into the text first, and filled
   again, if it holds a part of the line that ends before the last column
   the editor goes to. */
static void T14_00(void)
{
    u16 left;

    if (ed.EdTxHex != 0) {
        T14_11();
        return;
    }
    left = T57_00().left;
    if (left == 0 && ed.EdSizLn != 0 && ed.EdWhole == 0) {
        T51_00();
        T14_11();
        return;
    }
    ed.EdCursr = ed.EdSizLn - left;
}

/* Keep the cursor from the columns behind the last one the editor goes
   to. */
static void T08_01(void)
{
    u16 last = ed.EdSizLn - T57_00().left;

    if (last < ed.EdCursr)
        ed.EdCursr = last;
}

/* Go to the start of the next line, if there is one. */
static void T06_14(void)
{
    struct Where where;

    T51_00();
    T60_10((struct Where STACK *)&where);
    if (T53_00((struct Where STACK *)&where) == 0) {
        T60_20((struct Where STACK *)&where);
        ed.EdCursr = 0;
    }
    T50_00();
}

/* Right, as text: `mouse` is not 0 for a click in the last column of the
   window, which does not go on to the next line. */
static void T06_10(u16 mouse)
{
    if (ed.EdCursr >= ed.EdSizLn) {
        if (ed.EdWhole != 0) {
            if (mouse == 0)             /* T06_13 */
                T06_14();
            return;
        }
        T51_00();
        T50_00();
    }
    if (T57_02() < LenEdit - 100 - 1)   /* T06_11 */
        ed.EdCursr++;
}

/* Right. In hex from the first digit of a byte to the second, where the
   cursor is among the digits, else to the next byte, or the next line;
   but not from behind the last byte. */
static void T06_00(void)
{
    u32 place;
    u16 cursor;

    if (ed.EdTxHex == 0) {
        T06_10(0);
        return;
    }
    cursor = ed.EdCursr;
    place = ed.EdOffst + cursor;
    if (T63_01(HIGH(place), LOW(place)) != 0)
        return;
    if (ed.EdHexAs == 0 && ed.EdHxCur == 0) {
        ed.EdHxCur++;
        return;
    }
    ed.EdHxCur = 0;                     /* T06_01 */
    cursor++;
    if (cursor >= 0x10) {
        T06_14();
        return;
    }
    ed.EdCursr = cursor;
}

/* What follows a character typed: the text is changed, and the cursor
   goes right. */
static void T01_20(void)
{
    ed.EdChng = 1;
    T06_00();
}

/* A character typed into the text, at the cursor. A buffer that is full
   is put back into the text first and filled again. */
static void T01_03(u16 key)
{
    u8 *to;
    u16 count;

    if (ed.EdSizLn >= LenEdit) {
        T51_00();
        T50_00();
    }
    ed.EdSizLn++;                       /* T01_04 */
    to = EditBuf + LenEdit - 1;
    for (count = LenEdit - 1 - ed.EdCursr; count; count--) {
        *to = to[-1];
        to--;
    }
    EditBuf[ed.EdCursr] = (u8)key;      /* T01_05 */
    ed.EdUpdat = 1;
    T01_20();
}

/* A key typed in hex: among the digits a digit of the byte under the
   cursor, and nothing if the key is none; among the characters the byte.
   A byte behind the last one makes the text longer, if there is room; the
   editor beeps if there is not. With the sixteenth byte of a line the
   line below is drawn too. */
static void T01_10(u16 key)
{
    u32 place = ed.EdOffst + ed.EdCursr;
    struct Address address = AddressOf(place);
    u8 __far *byte = AT(address.segment, address.offset);
    u16 digit;

    if (ed.EdHexAs == 0) {
        digit = TstHex(key);
        if (digit > 0xFF)
            return;
        if (ed.EdHxCur == 0)
            key = digit << 4 | *byte & 0x0F;
        else
            key = digit | *byte & 0xF0;
    }
    *byte = (u8)key;                    /* T01_12 */
    if (T63_01(HIGH(place), LOW(place)) != 0) {
        if (ed.EdHexAs == 0)
            *byte &= 0xF0;
        if (address.offset >= 0x0F      /* T01_13 */
            && (u16)(address.segment - md.ViewSeg + 1) >= ed.EdSpace) {
            Beep();
        } else {
            ed.EdLen++;                 /* T01_14 */
            if ((LOW(ed.EdLen) & 0x0F) == 0
                && (u8)(ed.EdLine - ed.EdLin0 + 1) < ed.EdSizeY) {
                ed.EdLine++;
                *(u16 *)&ed.EdOffst += 0x10;
                T73_00();
                *(u16 *)&ed.EdOffst -= 0x10;
                ed.EdLine--;
            }
        }
    }
    T73_00();                           /* T01_15 */
    T01_20();
}

/* What follows a line that has been ended, where the cursor is still in
   it: the line is drawn, those below it move down by one if any are
   shown, and the cursor goes to the start of the next line. */
static void T02_03(void)
{
    u16 row;

    T50_00();
    T73_00();
    row = ed.EdLine - ed.EdLin0 + 2;
    if ((u8)row < ed.EdSizeY)
        T71_00(0x700 | (u8)(row - 1),
               ed.EdBase + ((u16)(u8)(ed.EdSizeY - 1) << 8), (u16)-160);
    T06_14();
}

/* Enter: the two characters that end a line, at the cursor. A buffer
   that has no room for them is put back into the text first and filled
   again. */
static void T02_00(void)
{
    u8 *to;
    u16 count;

    if (ed.EdSizLn >= LenEdit - 1) {
        T51_00();
        T50_00();
    }
    ed.EdSizLn += 2;                    /* T02_01 */
    to = EditBuf + LenEdit - 1;
    for (count = LenEdit - 2 - ed.EdCursr; count; count--) {
        *to = to[-2];
        to--;
    }
    *(u16 *)(EditBuf + ed.EdCursr) = CR_LF;     /* T02_02 */
    ed.EdUpdat = 1;
    T51_00();
    T02_03();
}

/* Go from the start of the line that the cursor is in to the line
   before, and to the end of that: Left in the first column, and in hex
   from the first byte of a line. Nothing if there is none. */
static void T05_11(void)
{
    struct Where where;

    T60_10((struct Where STACK *)&where);
    if (T54_00((struct Where STACK *)&where) != 0)
        return;
    T51_00();
    T60_20((struct Where STACK *)&where);
    T14_11();
}

/* Left in hex, by a byte: `digit` is the digit of that byte to be on,
   where the cursor is among the digits. */
static void T05_02(u16 digit)
{
    if (ed.EdCursr != 0) {
        ed.EdHxCur = (u8)digit;
        ed.EdCursr--;                   /* T05_03 */
        return;
    }
    if (ed.EdOffst == 0)                /* T05_05 */
        return;
    ed.EdHxCur = 1;
    T05_11();
}

/* Left. In hex from the second digit of a byte to the first, where the
   cursor is among the digits, else to the byte before. */
static void T05_00(void)
{
    if (ed.EdTxHex == 0) {
        if (ed.EdCursr != 0)            /* T05_10 */
            ed.EdCursr--;
        else
            T05_11();
        return;
    }
    if (ed.EdHexAs == 0 && ed.EdHxCur != 0) {
        ed.EdHxCur = 0;
        return;
    }
    T05_02(ed.EdHexAs == 0);            /* T05_01 */
}

/* Up, if `down` is 0, or down, by `count` lines or as many as there are,
   and into the column that the cursor was in before it last went up or
   down; in hex into the byte and the digit, or behind the last byte of a
   line that has not that many. */
static void T10_02(u16 down, u16 count)
{
    struct Where where;
    struct Address address;
    struct Ending end;
    u16 failed, cursor;

    T51_00();
    T60_10((struct Where STACK *)&where);
    if (count != 0) {
        do {                            /* T10_03 */
            if ((u8)down == 0)
                failed = T54_00((struct Where STACK *)&where);
            else
                failed = T53_00((struct Where STACK *)&where);
            if (failed)
                break;
        } while (--count);
    }
    T60_20((struct Where STACK *)&where);       /* T10_06 */
    ed.EdUpDwn = 1;
    if (ed.EdTxHex == 0) {
        T14_12(ed.EdCol1 + 1);
        return;
    }
    ed.EdCursr = ed.EdCol1;             /* T10_10 */
    ed.EdHxCur = ed.EdHex1;
    address = AddressOf(ed.EdOffst);
    end = T53_10(address.segment, address.offset);
    if (!end.none && (end.at & 0x0F) == 0)
        return;
    cursor = end.at & 0x0F;             /* T10_11 */
    if (cursor > ed.EdCursr)
        return;
    ed.EdCursr = cursor;
    ed.EdHxCur = 0;
}

/* PgUp, if `down` is 0, or PgDn: to the first or the last line of the
   window, or from there by a line less than the window has; as far as
   there are lines. */
static void T12_01(u16 down)
{
    struct Where where;
    u16 count, row, failed;

    T51_00();
    T60_10((struct Where STACK *)&where);
    count = (u16)ed.EdSizeY - 1;
    row = ed.EdLine - ed.EdLin0;
    if (down == 0) {
        if (row != 0)
            count = row;
    } else if (count > row) {           /* T12_02 */
        count -= row;
    }
    do {                                /* T12_03 */
        if (down == 0)
            failed = T54_00((struct Where STACK *)&where);
        else
            failed = T53_00((struct Where STACK *)&where);
        if (failed)
            break;
    } while (--count);
    if (where.line != ed.EdLine) {      /* T12_06 */
        T60_20((struct Where STACK *)&where);
        ed.EdCursr = 0;
        ed.EdHxCur = 0;
    }
    T50_00();                           /* T12_07 */
}

/* Ctrl-End: to the last line of the text, and its end. In hex that is
   the line that the byte behind the last one would be in. */
static void T16_00(void)
{
    struct Where where;

    if (ed.EdTxHex == 0) {
        T51_00();
        T60_10((struct Where STACK *)&where);
        while (T53_00((struct Where STACK *)&where) == 0)
            ;
        T60_20((struct Where STACK *)&where);
    } else {
        ed.EdLine = AddressOf(ed.EdLen).segment - md.ViewSeg;   /* T16_10 */
        ed.EdOffst = ed.EdLen & 0xFFFFFFF0UL;
    }
    T14_11();                           /* T16_11 */
}

/* Take the characters of the buffer from `from` up to `end` out of the
   line, and keep them to be put back. */
static void T20_02(u8 *from, u8 *end)
{
    u8 *undo = UndoBuf;
    u16 count = end - from;
    u16 cut = count;
    const u8 *source = from;

    ed.EdUndo = count;
    for (; count; count--)
        *undo++ = *source++;
    if ((u16)(EditBuf + ed.EdSizLn) > (u16)end)
        for (count = EditBuf + ed.EdSizLn - end; count; count--)
            *from++ = *end++;
    ed.EdSizLn -= cut;                  /* T20_03 */
    ed.EdUpdat = 1;
    ed.EdChng = 1;
}

/* Put the buffer into the text in place of the line as far as the offset
   `end`, and fill it again: a line gone, or joined to the one before.
   With `drawn`, the screen follows: the line is drawn, those below it move
   up by one, and the line that comes into the last row of the window is
   drawn there. Where the text has no line for that row, the row is
   cleared. */
static void T20_06(u32 end, u16 drawn)
{
    struct Where where;
    u32 place, offset;
    u16 line, row, count;

    ed.EdEndBf = end;
    ed.EdUpdat = 1;
    T51_00();
    T50_00();
    if (!drawn)
        return;
    T73_00();
    line = ed.EdLine;
    row = line - ed.EdLin0;
    if ((u8)(row + 1) < ed.EdSizeY)
        T71_00(0x600 | (u8)(row + 1),
               ed.EdBase + ((u16)(u8)(row + 1) << 8), 160);
    count = (u16)ed.EdSizeY - 1;        /* T20_07 */
    if (count <= row)
        return;
    count -= row;
    T60_10((struct Where STACK *)&where);
    where.line = line;
    place = ed.EdLen;
    for (;;) {                          /* T20_08 */
        if (T53_00((struct Where STACK *)&where) != 0)
            break;
        if (--count == 0) {
            place = OFFSET_OF(where.segment, where.offset);
            break;
        }
    }
    offset = ed.EdOffst;                /* T20_09 */
    ed.EdOffst = place;
    ed.EdLine = where.line + count;
    T50_00();
    T73_00();
    ed.EdOffst = offset;
    ed.EdLine = line;
    T50_00();
}

/* Join the next line to the one in the buffer, if there is a next line:
   Del at the end of a line. The two characters between them are what is
   kept to be put back. */
static void T20_05(u16 drawn)
{
    struct Where where;

    T60_10((struct Where STACK *)&where);
    /* The assembly has in DI, for the number of the line, whatever was
       left there. T53_00 goes by it only where it is 0FFFFh, which
       nothing leaves. */
    where.line = 0;
    if (T53_00((struct Where STACK *)&where) != 0)
        return;
    ed.EdUndo = 2;
    *(u16 *)UndoBuf = CR_LF;
    T20_06(OFFSET_OF(where.segment, where.offset), drawn);
}

/* Del: the character under the cursor, or at the end of the line the
   two that end it. */
static void T20_00(void)
{
    u8 *at;

    if (ed.EdTxHex != 0)
        return;
    if (ed.EdCursr >= ed.EdSizLn) {
        T20_05(1);
        return;
    }
    at = EditBuf + ed.EdCursr;          /* T20_01 */
    T20_02(at, at + 1);
}

/* BackSpace. As text the character before the cursor, or in the first
   column the two that end the line before, if there is one: the cursor is
   then at the end of that line, and if the window did not show the line,
   it begins there now, and nothing else is drawn. In hex it takes the
   last byte off the text if the cursor is behind it, and clears the line
   on the screen if that was its only byte; anywhere else it is Left. */
static void T21_00(void)
{
    struct Where where;
    u32 place;
    u16 cursor, row;
    u8 *at;

    if (ed.EdTxHex != 0) {
        place = ed.EdOffst + ed.EdCursr;
        if (place == 0)
            return;
        if (T63_01(HIGH(place), LOW(place)) == 0) {
            T05_00();                   /* T21_02 */
            return;
        }
        ed.EdChng = 1;
        ed.EdLen--;
        if ((LOW(place) & 0x0F) == 0)
            Window(T70_00(ed.EdLine - ed.EdLin0).place,
                   0x100 | ed.EdSizeX);
        T05_02(0);                      /* T21_01 */
        return;
    }
    cursor = ed.EdCursr;                /* T21_10 */
    if (cursor != 0) {
        cursor--;
        ed.EdCursr = cursor;
        at = EditBuf + cursor;
        T20_02(at, at + 1);
        return;
    }
    T60_10((struct Where STACK *)&where);       /* T21_11 */
    if (T54_00((struct Where STACK *)&where) != 0)
        return;
    T51_00();
    T60_20((struct Where STACK *)&where);
    T50_00();
    ed.EdCursr = ed.EdSizLn - T57_00().left;
    row = ed.EdLine + 1 - ed.EdLin0;
    if (row == 0) {
        ed.EdLin0 = ed.EdLine;
        ed.EdFirst = ed.EdOffst;
    }
    T20_05(row);                        /* T21_12 */
}

/* Ctrl-K: cut the line off at the cursor. Of a line that the buffer
   holds a part of, the rest goes too, and is not kept to be put back. */
static void T24_00(void)
{
    struct Where where;
    u32 end;

    if (ed.EdTxHex != 0)
        return;
    if (ed.EdWhole == 0) {
        T51_00();
        T50_00();
        T60_10((struct Where STACK *)&where);
        where.line = 0;                 /* see T20_05 */
        end = ed.EdLen;
        if (T53_00((struct Where STACK *)&where) == 0)
            end = OFFSET_OF(where.segment, where.offset) - 2;
        ed.EdEndBf = end;               /* T24_01 */
        ed.EdWhole = 1;
    }
    if (ed.EdCursr >= ed.EdSizLn)       /* T24_02 */
        return;
    T20_02(EditBuf + ed.EdCursr, EditBuf + ed.EdSizLn);
}

/* Ctrl-Y: cut the line. What the buffer has of it is kept to be put
   back, as a line. */
static void T25_00(void)
{
    struct Where where;
    u32 end;
    u16 count, size;

    if (ed.EdTxHex != 0)
        return;
    if (ed.EdWhole == 0) {
        T51_00();
        T50_00();
    }
    size = ed.EdSizLn;                  /* T25_01 */
    for (count = 0; count < size; count++)
        UndoBuf[count] = EditBuf[count];
    ed.EdUndo = size + 2;
    *(u16 *)(UndoBuf + size) = CR_LF;
    T60_10((struct Where STACK *)&where);
    where.line = 0;                     /* see T20_05 */
    end = ed.EdLen;
    if (T53_00((struct Where STACK *)&where) == 0)
        end = OFFSET_OF(where.segment, where.offset);
    ed.EdSizLn = 0;                     /* T25_02 */
    ed.EdCursr = 0;
    T20_06(end, 1);
}

/* Ctrl-U: put back at the cursor what was last taken out. What is no
   line and fits the buffer goes into the buffer. A line, and what does
   not fit, goes into the text: the buffer is put back first, and what is
   put in is written through the buffer as if it were a line of its own
   at the cursor. */
static void T26_00(void)
{
    u32 offset;
    u16 count = ed.EdUndo;
    u16 line = 0;
    u16 i, cursor;
    u8 *from, *to;

    if (ed.EdTxHex != 0 || count == 0)
        return;
    if (count >= 2 && ENDS_LINE(*(const u16 *)(UndoBuf + count - 2)))
        line = 1;
    else if (ed.EdSizLn < (u16)(LenEdit + 1 - count)) {
        ed.EdSizLn += count;            /* T26_01 */
        to = EditBuf + LenEdit - 1;
        from = to - count;
        cursor = (u16)(EditBuf + ed.EdCursr - 1);
        if ((u16)from > cursor)
            for (i = (u16)from - cursor; i; i--)
                *to-- = *from--;
        ed.EdCursr += count;            /* T26_02 */
        to = from + 1;
        for (i = 0; i < count; i++)
            to[i] = UndoBuf[i];
        ed.EdUpdat = 1;
        T08_01();
        return;
    }
    T51_00();                           /* T26_04 */
    offset = ed.EdOffst;
    ed.EdOffst = offset + ed.EdCursr;
    ed.EdEndBf = ed.EdOffst;
    for (i = 0; i < count; i++)
        EditBuf[i] = UndoBuf[i];
    ed.EdSizLn = count;
    ed.EdUpdat = 1;
    T51_00();
    ed.EdOffst = offset;
    if (line) {
        T02_03();
        return;
    }
    T50_00();                           /* T26_05 */
    ed.EdCursr += count;
    T08_01();
}

/* F4: from text to hex and back, with the cursor on the same byte. In
   hex the window begins with the line of the cursor. Not in a window
   narrower than the screen. */
static void T30_00(void)
{
    u32 place;
    struct Address address;

    if (ed.EdSizeX < 80)
        return;
    T51_00();
    ed.EdCol0 = 0;
    ed.EdScrn = 1;
    ed.EdTxHex = ed.EdTxHex == 0;
    if (ed.EdTxHex != 0) {
        ed.EdHxCur = 0;
        place = ed.EdOffst + ed.EdCursr;
        address = AddressOf(place);
        ed.EdCursr = address.offset;
        ed.EdLine = address.segment - md.ViewSeg;
        ed.EdLin0 = ed.EdLine;
        ed.EdOffst = OFFSET_OF(address.segment, 0);
    } else {
        place = ed.EdOffst + ed.EdCursr;        /* T30_10 */
        T56_00(0, 0, 0, HIGH(place), LOW(place));
        ed.EdLin0 = ed.EdLine;
    }
    ed.EdFirst = ed.EdOffst;            /* T30_11 */
}

/* Shift-F7: look for the text SrchStr again, from the cursor on, with a
   box that says so; the text 32K at a time, and each time from as many
   bytes before the end of the last as the text is long, less one. A key
   that stops stops it. If it is found, the cursor goes behind it: in hex
   among the digits if it was found as bytes, else among the characters.
   If it is not, a box says so. */
static void T32_00(void)
{
    struct Box box;
    u32 place, left;
    struct Address address;
    struct Sought sought;
    u16 count;

    if (md.SrchStr[0] == 0)
        return;
    T51_00();
    V80_00(OFFSET(EdFl_S), (struct Box STACK *)&box);
    place = ed.EdOffst + ed.EdCursr;
    for (;;) {                          /* T32_01 */
        if (TstBrk())
            goto done;
        if (ed.EdLen < place)
            goto none;
        left = ed.EdLen - place;
        count = 0x8000;
        if (HIGH(left) == 0 && LOW(left) < count)
            count = LOW(left);
        address = AddressOf(place);     /* T32_02 */
        sought = V82_00(address.segment, address.offset, count);
        if (sought.found < 0x100)
            break;
        if (count < SrchLen)
            goto none;
        place += (u16)(count - SrchLen + 1);
    }
    if (TstBrk())                       /* T32_05 */
        goto done;
    place = OFFSET_OF(address.segment, sought.end);
    if (ed.EdTxHex != 0) {
        ed.EdHexAs = (u8)sought.found;
        ed.EdHxCur = 0;
        ed.EdOffst = place & 0xFFFFFFF0UL;
        ed.EdLine = AddressOf(place).segment - md.ViewSeg;
        ed.EdCursr = LOW(place) & 0x0F;
    } else {
        T56_00(HIGH(ed.EdOffst), LOW(ed.EdOffst), ed.EdLine,   /* T32_06 */
               HIGH(place), LOW(place));
    }
    goto done;
none:
    Dialog(OFFSET(SrEr_B), OFFSET(EdFl_S), 0, 0, MAIN_SEG);    /* T32_03 */
done:
    T50_00();                           /* T32_07 */
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
}

/* F2: save the file, and show its name, which Shift-F2 has asked for. */
static void T34_00(void)
{
    if (T80_00() == 0)
        ed.EdChng = 0;
    if (ed.EdStat != 0)                 /* T34_01 */
        T74_00();
}

/* What the editor does with a key. Returns 1 if that is to leave. */
static u16 T00_83(u16 key)
{
    u16 scan = key >> 8;
    u16 c = key & 0xFF;
    u16 cursor;

    /* The table of keys that the assembly has for Case: a character with
       any scan code, or a key that has no character. A key without a
       scan code is none of them. */
    if (scan != 0 && c != 0) {
        switch (c) {
        case 0x09:                      /* Tab */
            if (ed.EdTxHex != 0) {      /* T03_00 */
                ed.EdHexAs = ed.EdHexAs == 0;
                return 0;
            }
            T01_03(key);
            return 0;
        case 0x11:                      /* ^Q: T04_00 */
            md.Quote = 1;
            return 0;
        case 0x13:                      /* ^S */
            goto left;
        case 0x04:                      /* ^D */
            goto right;
        case 0x01:                      /* ^A */
            goto word_left;
        case 0x06:                      /* ^F */
            goto word_right;
        case 0x05:                      /* ^E */
            goto up;
        case 0x18:                      /* ^X */
            goto down;
        case 0x12:                      /* ^R */
            goto page_up;
        case 0x03:                      /* ^C */
            goto page_down;
        case 0x07:                      /* ^G */
            goto delete;
        case 0x08:                      /* BackSpace */
            T21_00();
            return 0;
        case 0x7F:                      /* ^BS */
        case 0x17:                      /* ^W: T22_00 */
            if (ed.EdTxHex != 0)
                return 0;
            cursor = EdSub1(EditBuf, ed.EdCursr);
            if (cursor == ed.EdCursr)
                return 0;
            T20_02(EditBuf + cursor, EditBuf + ed.EdCursr);
            ed.EdCursr = cursor;
            return 0;
        case 0x14:                      /* ^T: T23_00 */
            if (ed.EdTxHex != 0)
                return 0;
            cursor = T55_00(ed.EdCursr);
            if (cursor == ed.EdCursr)
                return 0;
            T20_02(EditBuf + ed.EdCursr, EditBuf + cursor);
            return 0;
        case 0x0B:                      /* ^K */
            T24_00();
            return 0;
        case 0x19:                      /* ^Y */
            T25_00();
            return 0;
        case 0x15:                      /* ^U */
            T26_00();
            return 0;
        case 0x1B:                      /* Esc */
            goto quit;
        }
    } else if (scan != 0) {
        switch (scan) {
        case 0x4B:                      /* Left */
left:
            T05_00();
            return 0;
        case 0x4D:                      /* Right */
right:
            T06_00();
            return 0;
        case 0x73:                      /* ^Left: T07_00 */
word_left:
            if (ed.EdTxHex == 0)
                ed.EdCursr = EdSub1(EditBuf, ed.EdCursr);
            return 0;
        case 0x74:                      /* ^Right: T08_00 */
word_right:
            if (ed.EdTxHex == 0) {
                ed.EdCursr = T55_00(ed.EdCursr);
                T08_01();
            }
            return 0;
        case 0x48:                      /* Up: T09_00 */
up:
            T10_02(0, 1);
            return 0;
        case 0x50:                      /* Down: T10_00 */
down:
            T10_02(1, 1);
            return 0;
        case 0x49:                      /* PgUp: T11_00 */
page_up:
            T12_01(0);
            return 0;
        case 0x51:                      /* PgDn: T12_00 */
page_down:
            T12_01(1);
            return 0;
        case 0x47:                      /* Home: T13_00 */
            ed.EdCursr = 0;
            ed.EdHxCur = 0;
            return 0;
        case 0x4F:                      /* End */
            T14_00();
            return 0;
        case 0x84:                      /* ^PgUp */
        case 0x77:                      /* ^Home: T15_00 */
            T51_00();
            ed.EdOffst = 0;
            T50_00();
            ed.EdCursr = 0;
            ed.EdLine = 0;
            ed.EdHxCur = 0;
            return 0;
        case 0x76:                      /* ^PgDn */
        case 0x75:                      /* ^End */
            T16_00();
            return 0;
        case 0x53:                      /* Del */
delete:
            T20_00();
            return 0;
        case 0x3E:                      /* F4 */
            T30_00();
            return 0;
        case 0x41:                      /* F7: T31_00 */
            if ((u8)Dialog(OFFSET(Srch_B), OFFSET(EdFl_S), 0, 0,
                           MAIN_SEG) != 0)
                return 0;
            T32_00();
            return 0;
        case 0x5A:                      /* Shift F7 */
            T32_00();
            return 0;
        case 0x55:                      /* Shift F2: T33_00 */
            if ((u8)Dialog(OFFSET(SavAs_B), 0, 0, 0, MAIN_SEG) != 0)
                return 0;
            T34_00();
            return 0;
        case 0x3C:                      /* F2 */
            T34_00();
            return 0;
        case 0x44:                      /* F10: T35_00 */
quit:
            if (ed.EdChng == 0)
                return 1;
            c = (u8)Dialog(OFFSET(SavCn_B), 0, 0, 0, MAIN_SEG);
            if (c == 1)
                return 1;
            if (c > 1)
                return 0;
            return T80_00() == 0;       /* T35_01 */
        case 0x5D:                      /* Shift F10: T35_01 */
            return T80_00() == 0;
        }
    }
    if (c == 0)                         /* T01_00 */
        return 0;
    if (ed.EdTxHex != 0) {
        T01_10(key);
        return 0;
    }
    if (c == 0x0D) {
        T02_00();
        return 0;
    }
    if (scan == 0 || c >= ' ')          /* T01_01 */
        T01_03(key);
    return 0;
}

/* Where the mouse says that the cursor is to go, with a button down at
   `mouse`, the row in the high byte. Returns 0 if that is nowhere, and
   the mouse is to be asked again.

   With the button held since a click in the window: up a line with the
   mouse above the window, down a line with it below. In the window, the
   cursor goes to the line of the mouse and the column, in hex to the
   byte and the digit; a click in the last column of the cursor's own
   line, as text, is Right. */
static u16 T00_71(u16 mouse)
{
    u16 base = ed.EdBase;
    u16 size = *(u16 *)&ed.EdSizeX;
    u16 row, column, where, down, count;

    ClrKbd();
    ed.EdCurLn = 1;
    if (md.WinMous != 0) {
        if (mouse >> 8 < base >> 8) {
            T10_02(0, 1);               /* T09_00 */
            return 1;
        }
        if (mouse >> 8 >= (u8)((base >> 8) + (size >> 8))) {    /* T00_72 */
            T10_02(1, 1);               /* T10_00 */
            return 1;
        }
    }
    if (InWind(mouse, base, size))      /* T00_73 */
        return 0;
    md.WinMous = 1;
    ed.EdHex1 = 0;
    mouse -= base;
    row = mouse >> 8;
    column = (u16)(signed char)(u8)mouse;
    if (ed.EdTxHex == 0) {
        where = column + ed.EdCol0;
        if (row == (u8)(ed.EdLine - ed.EdLin0)
            && (u8)mouse >= (u8)(size - 1)) {
            T06_10(1);
            return 1;
        }
    } else if ((u8)column < 63) {       /* T00_74 */
        ed.EdHexAs = 0;
        where = 0;
        if ((u8)column >= 11) {
            column = (u8)column - 11;
            /* Thirteen columns to four bytes, three to a byte: which
               byte, and which of its columns. */
            where = column / 13 << 2;
            column %= 13;
            if (column >= 12)
                column--;
            ed.EdHex1 = (u8)(column % 3);
            where = (u16)(signed char)(u8)(column / 3 | where);
        }
    } else {                            /* T00_76 */
        ed.EdHexAs = 1;
        where = (u8)column - 63;
        if (where >= 16)
            where = 15;
    }
    ed.EdCol1 = where;                  /* T00_77 */
    count = row - (u16)(ed.EdLine - ed.EdLin0);
    down = 1;
    if (row < (u16)(ed.EdLine - ed.EdLin0)) {
        count = -count;
        down = 0;
    }
    T10_02(down, count);                /* T00_78 */
    return 1;
}

/* Bring the screen up to where the cursor is: the first column and the
   first line that the window shows follow the cursor, the window is
   drawn again as far as it has changed, the status line says where the
   cursor is, and the cursor is put there. */
static void T00_20(void)
{
    struct Where where;
    struct Address address;
    struct Ending end;
    u32 place, free;
    u16 column, first_column, first_line, line, old_column, old_line;
    u16 delta, row, count, size, segment, start, state, color, i;
    u16 *saved;

    column = T57_02();
    ed.EdCol = column;
    if (ed.EdUpDwn == 0) {
        ed.EdCol1 = column;
        ed.EdHex1 = ed.EdHxCur;
    }
    ed.EdUpDwn = 0;                     /* T00_21 */
    line = ed.EdLine;
    first_column = ed.EdCol0;
    first_line = ed.EdLin0;
    if (ed.EdTxHex == 0) {
        if (column < first_column
            || column == first_column && first_column != 0) {
            first_column = column;      /* T00_22 */
            if (column != 0)
                first_column--;
        } else if (column > first_column) {
            size = (u16)ed.EdSizeX - 1; /* T00_23 */
            if (column >= (u16)(size + first_column))
                first_column = column - (u16)(size - 1);
        }
    }
    if (line < first_line) {            /* T00_24 */
        first_line = line;
    } else {
        size = ed.EdSizeY;              /* T00_25 */
        if (line >= (u16)(size + first_line)) {
            size--;
            first_line = line < size ? 0 : line - size;
        }
    }
    old_column = ed.EdCol0;             /* T00_26 */
    ed.EdCol0 = first_column;
    old_line = ed.EdLin0;
    ed.EdLin0 = first_line;
    delta = first_line - old_line;
    if (delta != 0) {
        /* Where the first line of the window is now: so many lines on
           from where it was, or back, or back from the cursor's line if
           that is nearer. */
        row = ed.EdLine - ed.EdLin0;
        T60_10((struct Where STACK *)&where);
        where.line = 0;
        place = 0;
        if (first_line >= old_line) {
            count = delta;
            if (count <= row) {
                address = T62_00();
                where.segment = address.segment;
                where.offset = address.offset;
                do                      /* T00_27 */
                    T53_00((struct Where STACK *)&where);
                while (--count);
                goto first;
            }
        } else {
            if (ed.EdLin0 == 0)         /* T00_28 */
                goto top;
            count = -delta;
            if (count <= row) {
                row = count;
                address = T62_00();
                where.segment = address.segment;
                where.offset = address.offset;
            }
        }
        count = row;                    /* T00_29 */
        if (count != 0) {
            where.line--;
            do                          /* T00_30 */
                T54_00((struct Where STACK *)&where);
            while (--count);
        }
first:
        place = OFFSET_OF(where.segment, where.offset);     /* T00_31 */
top:
        ed.EdFirst = place;             /* T00_32 */
    }
    if (ed.EdScrn == 0) {               /* T00_33 */
        if (old_column == ed.EdCol0) {
            if (delta == 1) {
                T71_00(0x600, ed.EdBase, 160);
                ed.EdCurLn = 0;
            } else if (delta == 0xFFFF) {       /* T00_34 */
                T71_00(0x700,
                       ed.EdBase + ((u16)(u8)(ed.EdSizeY - 1) << 8),
                       (u16)-160);
                ed.EdCurLn = 0;
            } else if (delta != 0) {
                goto whole;
            }
            if (ed.EdCurLn == 0)        /* T00_36 */
                T73_00();
            goto shown;
        }
    } else {
        count = ed.EdSizeY;             /* T00_38 */
        row = 0;
        do
            T70_00(row++);
        while (--count);
    }
whole:
    ClrKbd();                           /* T00_40 */
    count = ed.EdSizeY;
    row = 0;
    address = T62_00();
    segment = address.segment;
    start = address.offset;
    do {                                /* T00_41 */
        place = OFFSET_OF(segment, start);
        state = T63_01(HIGH(place), LOW(place));
        if (state == 1 && ed.EdTxHex == 0) {
            /* The end of the text: a line of nothing, if the text ends
               with the two characters. */
            if (place < 2)              /* T00_45 */
                break;
            address = AddressOf(place - 2);
            if (!ENDS_LINE(*(const u16 __far *)
                           AT(address.segment, address.offset)))
                break;
            T72_00(row, address.segment, address.offset + 2, 0);
            break;
        }
        if (state != 0) {
            /* In hex, the end of the text or what is behind it: a line
               of no bytes, if a line begins there. */
            if (ed.EdTxHex != 0 && (start & 0x0F) == 0)     /* T00_46 */
                T72_00(row, segment, start, 0);
            break;
        }
        end = T53_10(segment, start);
        if (end.none) {
            address = AddressOf(place);
            segment = address.segment;
            start = address.offset;
            end = T53_10(segment, start);
            if (end.none) {
                /* A line that does not end within 64K: what the window
                   shows of it is at its start, and where it ends is
                   looked for a piece at a time. */
                T72_00(row, segment, start, 0xFFFF);
                do {                    /* T00_42 */
                    segment += PIECE_SEGMENTS;
                    end = T53_10(segment, 0);
                } while (end.none);
                start = end.at;
                goto next;
            }
        }
        T72_00(row, segment, start, end.at);    /* T00_43 */
        start = end.at;
next:
        row++;                          /* T00_44 */
    } while (--count);
    T73_10();                           /* T00_48 */
    Window(ed.EdBase, *(u16 *)&ed.EdSizeX);
shown:
    ed.EdScrn = 0;                      /* T00_49 */
    ed.EdCurLn = 0;

    if (ed.EdStat != 0) {
        saved = (u16 *)md.TmpErr;       /* T00_51 */
        for (i = 0; i < 80; i++)
            saved[i] = md.ScrBuf[i];
        color = ' ';
        if (ed.EdChng != 0)
            color = '*';
        *(u8 *)(md.ScrBuf + 38) = (u8)color;
        color = ' ';
        if (md.Quote != 0)
            color = '"';
        *(u8 *)(md.ScrBuf + 39) = (u8)color;
        color |= md.ScrBuf[0] & 0xFF00;
        T75_00(ed.EdLine, md.ScrBuf + 46, color);
        T75_00(ed.EdCol, md.ScrBuf + 56, color);
        /* What is free: the room less the text, and less what the line
           in the buffer has grown by. Nothing, if that is more than
           there is. */
        free = ((u32)ed.EdSpace << 4) - ed.EdLen;
        if (ed.EdTxHex == 0) {
            free += ed.EdEndBf - ed.EdOffst - ed.EdSizLn;
            if (HIGH(free) >= 0x10)
                free = 0;
        }
        Number(md.TmpBuf, HIGH(free), LOW(free),        /* T00_54 */
               NUM_SEPARATED | NUM_PADDED);
        MvsBw(md.ScrBuf + 62, md.TmpBuf + 6, 7, color);
        for (i = 0; i < 80; i++)
            if (saved[i] != md.ScrBuf[i]) {
                Window(0, 0x150);
                break;
            }
    }

    column = (u8)(ed.EdCol - ed.EdCol0);        /* T00_60 */
    if (ed.EdTxHex != 0) {
        if (ed.EdHexAs != 0) {
            column += 63;
        } else {
            column = column * 3 + (column >> 2) + 11;
            if (ed.EdHxCur != 0)
                column++;
        }
    }
    Cursor((u16)(((u16)(u8)(ed.EdLine - ed.EdLin0) << 8 | (u8)column)
                 + ed.EdBase));
}

/* The loop of keys of the editor: the screen, the mouse, a key and what
   it does, until a key leaves the editor. */
static void T00_19(void)
{
    struct Mouse mouse;
    u16 key, bar;

    for (;;) {
        T00_20();
        for (;;) {                      /* T00_70 */
            mouse = GetMous(0);
            if (mouse.buttons == 0) {
                md.WinMous = 0;
                break;
            }
            if (T00_71(mouse.place))
                goto again;
        }
        bar = 10;                       /* T00_80 */
        if (ed.EdStat != 0) {
            bar = 6;
            if (ed.EdTxHex == 0)
                bar = 7;
        }
        key = Input(bar);
        if (md.DesqVie == 0)
            ClrKbd();
        HidCur();                       /* T00_82 */
        if (key >> 8 == 0xFC)
            continue;
        if (md.Quote != 0) {
            /* The key behind ^Q is a character, whatever it is. */
            md.Quote = 0;
            if ((u8)key != 0) {
                if (ed.EdTxHex == 0)
                    T01_03(key);
                else
                    T01_10(key);
            }
            continue;
        }
        if (T00_83(key))
            return;
again:
        ;
    }
}

/* The editor: T00_00 of the assembly, to which the viewer's code jumps
   once it has opened the file, with the file's handle, or 0FFFFh for a
   file that is not there, and its size in EdLen. The box "Reading the
   file" is on the screen: `begin` and `size` say where, and `saved` where
   what it covers is kept.

   Returns 0 if the file is to be viewed instead, 1 if it is to be closed
   and nothing more, and 2 when it has been edited and is closed, with
   EdWrite in the high byte, which V13_01 of the viewer takes: not 0 if
   the file was written.

   The text goes into the memory behind the program, EdSpace paragraphs:
   a file that is not shorter than that is too large, and a box asks
   whether to view it. A new file is asked about, if the window is the
   whole screen. The file is read in pieces of 0FF00h bytes; one that
   gives fewer bytes than its size says is as long as it gives. A window
   as wide as the screen has a status line above it, and the menu file
   and the extension files have ten lines of help below theirs. */
u16 T00_00(u16 handle, u16 begin, u16 size, u16 saved)
{
    struct DosRegs regs;
    struct LineArgs line;
    const u8 __far *name;
    u8 __far *end;
    u16 __far *cell;
    u16 *screen;
    u32 left;
    u16 code = AsmSeg();
    u16 help, file, count, segment, place, color, i;

    ed.EdSpace = MAIN_SEG + md.FreMem2 - md.ViewSeg;
    if ((u32)ed.EdSpace << 4 <= ed.EdLen) {
        /* T00_01. The assembly leaves in DX for the text that ^I stands
           for, and in BX and SI for a box that is on the screen, what the
           box before had there; so does this. */
        if ((u8)Dialog(OFFSET(BigFi_B), size, begin, saved, MAIN_SEG) != 0)
            goto closed;
        place = (begin & 0xFF00) + 0x100 | 37;  /* T00_02 */
        MovDz0((u16 __far *)BasAdr(place), AT(code, OFFSET(ViFl_S)));
        Window(place, 0x106);
        return 0;
    }

    /* T00_03: the menu file, or an extension file? */
    name = CutPath(md.ViewFil);
    end = MvzLine(md.TmpBuf, name, 13);
    *end++ = 0;
    PathLin(md.TmpBuf);
    count = (u16)(u32)end - (u16)md.TmpBuf;
    help = 0x0A01;
    file = OFFSET(Menu_F);
    if (!T64_00(AT(code, file), md.TmpBuf, count)) {
        help = 0x0A02;
        file = OFFSET(Ext_F);
        if (!T64_00(AT(code, file), md.TmpBuf, count)) {
            file = OFFSET(ViewE_F);
            if (!T64_00(AT(code, file), md.TmpBuf, count)) {
                file = OFFSET(EditE_F);
                if (!T64_00(AT(code, file), md.TmpBuf, count))
                    help = 0;
            }
        }
    }
    EdHelp = (u8)help;                  /* T00_04 */
    ed.EdSizeY -= help >> 8;
    if (handle == 0xFFFF) {
        /* A new file. */
        if ((u8)(md.Lines - 2) <= ed.EdSizeY
            && (u8)Dialog(OFFSET(NewFi_B), size, begin, file,
                          MAIN_SEG) != 0)
            goto closed;
    } else {
        regs.bx = handle;               /* T00_06 */
        left = ed.EdLen;
        segment = md.ViewSeg;
        for (;;) {
            count = PIECE;              /* T00_07 */
            if (HIGH(left) == 0 && LOW(left) < count)
                count = LOW(left);
            regs.cx = count;            /* T00_08 */
            regs.name = AT(segment, 0);
            regs.ax = 0x3F00;
            if (Intr21((struct DosRegs STACK *)&regs))
                goto closed;            /* T00_09 */
            if (TstBrk())
                goto closed;
            if (regs.ax != count) {
                left -= regs.ax;        /* T00_11 */
                ed.EdLen -= left;
                break;
            }
            segment += PIECE_SEGMENTS;
            left -= count;
            if (left == 0)
                break;
        }
        regs.ax = 0x3E00;               /* T00_12 */
        Intr21((struct DosRegs STACK *)&regs);
    }
    ResWin(begin, size, (u16)md.WinBuf);        /* T00_13 */
    Window(begin, size);
    ed.EdStat = 0;
    if (ed.EdSizeX >= 79) {
        /* The status line. */
        color = Color(Menu_C) << 8 | ' ';
        for (i = 0; i < 80; i++)
            md.ScrBuf[i] = color;
        cell = MovDz0((u16 __far *)BasAdr(0), AT(code, OFFSET(EdFl_S) + 1));
        *(u8 __far *)(cell - 1) = ':';
        MovDz0((u16 __far *)BasAdr(41), AT(code, OFFSET(Line_S)));
        MovDz0((u16 __far *)BasAdr(52), AT(code, OFFSET(Col_S)));
        MovDz0((u16 __far *)BasAdr(70), AT(code, OFFSET(FrBuf_S)));
        T74_00();
        ed.EdStat = 1;
    }
    if (EdHelp != 0) {                  /* T00_14 */
        /* The help below the window: a box between two rows of half
           blocks, its title, and six lines in each of two columns. */
        place = ed.EdBase + ((u16)ed.EdSizeY << 8);
        screen = BasAdr(place);
        color = Color(HlBrd_C) << 8;
        for (i = 0; i < 80; i++)
            *screen++ = color | 0xDC;
        screen = BoxDubl(screen, Color(*AT(code, OFFSET(EdtH_P))) << 8 | 0xDC,
                         6, 1, 76);
        for (i = 0; i < 80; i++)
            *screen++ = color | 0xDF;
        file = OFFSET(EdtH1_S);
        if (EdHelp != 1)
            file = OFFSET(EdtH2_S);
        line.value = (place & 0xFF00) + 0x100 | 40;     /* T00_15 */
        line.screen = screen;
        line.line = AT(code, file);
        line.colors = AT(code, OFFSET(EdtH_P));
        /* None of the lines has ^I; the assembly leaves in DX what it
           had for BoxDubl. */
        line.include = AT(code, 76);
        CntLine((struct LineArgs STACK *)&line);
        T76_00((place & 0xFF00) + 0x200 | 3, code, (u16)(u32)line.line,
               OFFSET(EdtH_P));
        T76_00((place & 0xFF00) + 0x200 | 46, code, OFFSET(EdtH_S),
               OFFSET(EdtH_P));
        Window(place, 0x0A50);
    }
    md.Quote = 0;                       /* T00_16 */
    ed.EdWrite = 0;
    ed.EdHexAs = 0;
    ed.EdHxCur = 0;
    ed.EdTxHex = 0;
    ed.EdChng = 0;
    ed.EdUpDwn = 0;
    ed.EdCurLn = 0;
    ed.EdLine = 0;
    ed.EdCol0 = 0;
    ed.EdLin0 = 0;
    ed.EdFirst = 0;
    ed.EdOffst = 0;
    ed.EdCursr = 0;
    ed.EdUndo = 0;
    md.Clock = 0;
    md.KeyBar = 1;
    ed.EdScrn = 1;
    md.HlpPage = 5;
    T50_00();
    ed.EdShape = ChgCur();
    T00_19();
    CurShp(ed.EdShape);                 /* T35_02 */
    return (u16)ed.EdWrite << 8 | 2;
closed:
    ResWin(begin, size, (u16)md.WinBuf);        /* T00_10 */
    Window(begin, size);
    return 1;
}
