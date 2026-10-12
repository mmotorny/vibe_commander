/*
 * The viewer: the lines of the file it shows, the part of the file that
 * is in the memory, what the screen shows of it, and a text looked for.
 *
 * Translated from VCVIEW.INC.
 *
 * The viewer has a segment of up to 0FFE0h bytes, ViSpace, for the file.
 * A file that is larger is there a part at a time: ViOffset is where in
 * the file the part begins and ViLoaded how long it is. Where a line
 * begins on the screen is kept in a table at the top of the same segment,
 * a word for each line, growing down from ViSpace towards the text:
 * the first line, at the top, begins at 0, and ViEndTab is the address of
 * the last. A line ends behind CR LF or LF CR, or after 80 columns. Not
 * wrapped, a line on the screen is what stands between two such pairs,
 * however many entries of the table that is. Where the table would meet
 * the text, the part ends.
 *
 * A place in the text and the entry of the table that goes with it travel
 * together, in SI and BX in the assembly and in struct Line here. In hex
 * there is no table: a line is 16 bytes.
 */
#include "vc.h"

/* What the viewer keeps while it shows a file: ViewDat, which lies in
   TmpBuf from its byte 200 on. The editor has its own names for some of
   it. */
#pragma pack(push, 1)
struct ViewDat {
    u16 ViSpace;                        /* Edit buffer size */
    u8  ViewEdit;                       /* View / Edit */
    u8  ViStatus;                       /* Output status line */
    u16 ViLoaded;                       /* Bytes loaded in buffer */
    u32 ViLen;                          /* Size of file */
    u32 ViOffset;                       /* Offset of first loaded byte in buffer */
    u16 ViKeyBar;                       /* Saved KeyBar & Clock */
    u8  ViTxHex;                        /* Text / Hex mode */
    u8  ViWrap;                         /* Wrap / Unwrap */
    u16 ViFirst;                        /* Offset of first line of screen */
    u16 ViHandle;                       /* File handle */
    u16 ViCol;                          /* First column displayed on the screen */
    u16 ViPoint;                        /* Point to address of first line */
    u16 ViEndTab;                       /* Point to end of line table */
    u16 ViBase;                         /* View window base */
    u16 ViSize;                         /* View window size */
    u8  ViSrPan;                        /* String was found in Hex/ASCII panel */
    u8  ViSrLen;                        /* Length of the found string */
    u16 ViTmp1;                         /* First byte of search string */
    u16 ViTmp2;                         /* Last byte of search string */
    u32 ViSearch;                       /* Saerch offset */
    u8  Vi100pc;                        /* 100% was displayed */
};
#pragma pack(pop)

LAID_OUT(ViewDat, 43);

#define VI ((struct ViewDat *)(md.TmpBuf + 200))

/* What is looked for, in HelpBuf: the text in capitals and its length,
   and, if the text is hex digits and the viewer shows hex, the bytes they
   stand for and how many those are. */
#define SRCH_TEXT 0
#define SRCH_LEN  (*(u16 *)(md.HelpBuf + 90))
#define SRCH_HEX  100
#define HexLen    (*(u16 *)(md.HelpBuf + 190))

#define Arrow_C 35                      /* the colour of the arrows at the
                                           ends of a line that goes on */

/* A place in the text, and the entry of the table of lines for it. The
   stubs of NextLin and PrevLin push BX and SI to make one. */
struct Line {
    u16 at;                             /* SI */
    u16 entry;                          /* BX */
};

u16 NextLin(u16 segment, struct Line STACK *line);
u16 PrevLin(u16 segment, struct Line STACK *line);
u16 V20_00(u16 segment);
void V25_00(void);
u16 V30_00(u16 segment, u16 dx);
void V31_00(u16 place);
void V32_00(u16 segment, u16 number, u16 at, u16 end, u16 dx);
u16 V40_00(void);
void V50_00(u16 segment);
void V51_00(u16 segment);
void V55_00(u16 segment, u16 to, u16 count, u16 handle);
void V60_00(u16 segment, u16 at, u16 table);
void V81_00(void);
u32 V82_00(u16 segment, u16 at, u16 count);

/* How the assembly passes arguments to these, and what it expects back.
   The segment of the text is what the assembly has in DS. */
#pragma aux NextLin "*_c" __far parm [dx] [bx] value [ax] modify exact [ax]
#pragma aux PrevLin "*_c" __far parm [dx] [bx] value [ax] modify exact [ax]
#pragma aux V20_00  "*_c" __far parm [ax] value [si] modify exact [si]
#pragma aux V25_00  "*_c" __far modify exact [ax]
#pragma aux V30_00  "*_c" __far parm [ax] [dx] value [ax] modify exact [ax]
#pragma aux V31_00  "*_c" __far parm [bx] modify exact [ax]
#pragma aux V32_00  "*_c" __far parm [bx] [ax] [si] [cx] [dx] modify exact [ax]
#pragma aux V40_00  "*_c" __far value [ax] modify exact [ax]
#pragma aux V50_00  "*_c" __far parm [ax] modify exact [ax]
#pragma aux V51_00  "*_c" __far parm [ax] modify exact [ax]
#pragma aux V55_00  "*_c" __far parm [ax] [dx] [cx] [bx] modify exact [ax]
#pragma aux V60_00  "*_c" __far parm [ax] [si] [di] modify exact [ax]
#pragma aux V81_00  "*_c" __far modify exact [ax]
#pragma aux V82_00  "*_c" __far parm [bx] [si] [cx] value [dx ax] modify exact [ax dx]

/* The editor's routine that clears a line of the window, by its number:
   still assembly. */
void T70_00(u16 number);
#pragma aux T70_00 "*_a" __far parm [ax] modify exact [bx cx di es]

/* The text in a segment, and a word of it or of the table of lines. */
#define TEXT(segment) ((u8 __far *)((u32)(segment) << 16))
#define WORD_AT(text, at) (*(u16 __far *)((text) + (at)))

/* Whether two bytes end a line: CR LF, or LF CR. */
#define IS_CRLF(word) ((word) == 0x0A0D || (word) == 0x0D0A)

/* Find where the line behind the one at `line` begins. Returns 0 and
   leaves it there, or 1 if the part in the memory has no line behind
   this one: the place then stays and the entry is the first of the
   table. */
u16 NextLin(u16 segment, struct Line STACK *line)
{
    const u8 __far *text = TEXT(segment);
    u16 at = line->at;
    u16 entry = line->entry;
    u16 word;

    if (VI->ViTxHex != 0) {
        at = (at & 0xFFF0) + 0x10;
        if (VI->ViLoaded <= at)
            goto none;
        line->at = at;
        return 0;
    }
    entry += 2;
    if (at < WORD_AT(text, entry - 2))
        entry = VI->ViSpace;
    do {
        if (entry <= VI->ViEndTab)
            goto none;
        entry -= 2;
    } while (at >= WORD_AT(text, entry));
    at = WORD_AT(text, entry);
    if (VI->ViWrap == 0) {
        for (;;) {
            at = WORD_AT(text, entry);
            word = WORD_AT(text, at - 2);
            if (IS_CRLF(word))
                break;
            entry -= 2;
            if (entry < VI->ViEndTab)
                goto none;
        }
    }
    if (VI->ViLoaded < at)
        goto none;
    line->at = at;
    line->entry = entry;
    return 0;
none:
    line->entry = VI->ViSpace - 2;
    return 1;
}

/* Find where the line before the one at `line` begins. Returns as
   NextLin does. */
u16 PrevLin(u16 segment, struct Line STACK *line)
{
    const u8 __far *text = TEXT(segment);
    u16 at = line->at;
    u16 entry = line->entry;
    u16 word;

    if (VI->ViTxHex != 0) {
        at &= 0xFFF0;
        if (at == 0)
            goto none;
        line->at = at - 0x10;
        return 0;
    }
    entry += 2;
    if (at < WORD_AT(text, entry - 2))
        entry = VI->ViSpace;
    for (;;) {
        if (entry <= VI->ViEndTab)
            goto none;
        entry -= 2;
        word = WORD_AT(text, entry);
        if (at == word)
            break;
        if (at < word) {
            entry += 2;
            break;
        }
    }
    if (WORD_AT(text, entry) == 0)
        goto none;
    entry += 2;
    at = WORD_AT(text, entry);
    if (VI->ViWrap == 0) {
        /* Up to where the line that ends here begins: past the entries
           that do not follow the end of a line, twice. */
        entry -= 2;
        for (;;) {
            at = WORD_AT(text, entry);
            if (at == 0)
                goto found;
            word = WORD_AT(text, at - 2);
            if (IS_CRLF(word))
                break;
            entry += 2;
        }
        for (;;) {
            entry += 2;
            at = WORD_AT(text, entry);
            if (at == 0)
                break;
            word = WORD_AT(text, at - 2);
            if (IS_CRLF(word))
                break;
        }
    }
found:
    line->at = at;
    line->entry = entry;
    return 0;
none:
    line->entry = VI->ViSpace - 2;
    return 1;
}

/* Find the first line of the screen: where the line begins that ViFirst
   is in, which is returned, and its entry of the table, which goes to
   ViPoint. */
u16 V20_00(u16 segment)
{
    const u8 __far *text = TEXT(segment);
    u16 at = VI->ViFirst;
    u16 entry, word;

    if (VI->ViTxHex != 0) {
        VI->ViPoint = VI->ViSpace - 2;
        return at & 0xFFF0;
    }
    entry = VI->ViPoint + 2;
    if (at < WORD_AT(text, entry - 2))
        entry = VI->ViSpace;
    for (;;) {
        if (entry <= VI->ViEndTab)
            break;
        entry -= 2;
        if (at < WORD_AT(text, entry)) {
            entry += 2;
            break;
        }
    }
    at = WORD_AT(text, entry);
    if (VI->ViWrap == 0) {
        for (;;) {
            at = WORD_AT(text, entry);
            if (at == 0)
                break;
            word = WORD_AT(text, at - 2);
            if (IS_CRLF(word))
                break;
            entry += 2;
        }
    }
    VI->ViPoint = entry;
    return at;
}

/* Write the status line: the first column that is shown, and how far
   into the file the screen begins, in per cent. */
void V25_00(void)
{
    u16 *screen;
    u16 cell, count, percent, high;
    u32 position, length, low;

    VI->Vi100pc = 0;
    if (VI->ViStatus == 0)
        return;
    count = 0;
    if (VI->ViTxHex == 0)
        count = VI->ViCol;
    Number(md.TmpBuf, 0, count, NUM_SEPARATED);
    screen = md.ScrBuf + 44;
    cell = *screen & 0xFF00 | ' ';
    for (count = 7; count; count--)
        *screen++ = cell;
    MovDz0(md.ScrBuf + 44, md.TmpBuf);

    /* A hundred times the place, in 48 bits, and how often the length of
       the file goes into that: by adding and subtracting, as the assembly
       does. */
    position = VI->ViOffset + VI->ViFirst;
    low = 0;
    high = 0;
    for (count = 100; count; count--) {
        low += position;
        if (low < position)
            high++;
    }
    length = VI->ViLen;
    percent = 0;
    if (length != 0) {
        percent = 0xFFFF;
        for (;;) {
            percent++;
            if (low < length) {
                low -= length;
                if (high == 0)
                    break;
                high--;
            } else {
                low -= length;
            }
        }
    }
    Number(md.TmpBuf, 0, percent, NUM_SEPARATED | NUM_PADDED);
    MovDz0(BasAdr(75), md.TmpBuf + 10);
    Window(0, 0x150);
}

/* Write the screen: the window cleared, and a line of the text in each
   of its rows, from the line that ViFirst is in. Returns 1 if the part in
   the memory ends before the window is full, and else 0. `dx` is what
   the assembly has in DX, which V32_00 may store. */
u16 V30_00(u16 segment, u16 dx)
{
    const u8 __far *text = TEXT(segment);
    struct Line line;
    u16 number, count, at, end, result;
    u8 last;

    number = 0;
    count = VI->ViSize >> 8;
    do {
        T70_00(number);
        number++;
    } while (--count);
    number = 0;
    count = VI->ViSize >> 8;
    line.at = V20_00(segment);
    line.entry = VI->ViPoint;
    for (;;) {
        at = line.at;
        if (NextLin(segment, (struct Line STACK *)&line))
            break;
        V32_00(segment, number, at, line.at, dx);
        number++;
        if (--count == 0) {
            result = 0;
            goto show;
        }
    }
    /* The last line there is: without the first byte of a pair that ends
       it, where the second may be in the part still to be read. */
    end = VI->ViLoaded;
    if (V40_00() && at < end) {
        last = text[end - 1];
        if (last == 0x0D || last == 0x0A)
            end--;
    }
    V32_00(segment, number, at, end, dx);
    result = 1;
show:
    Window(VI->ViBase, VI->ViSize);
    return result;
}

/* Clear a line of the window: the one that begins at `place`, the row in
   the high byte. */
void V31_00(u16 place)
{
    u16 *screen = BasAdr(place);
    u16 count = VI->ViSize & 0xFF;
    u16 cell = Color(Back_C) << 8 | ' ';

    for (; count; count--)
        *screen++ = cell;
}

/* Write a line of the text into row `number` of the window: the bytes
   from `at` up to `end`. As text, a tab goes on to the next column that
   is a multiple of eight, what ends the line is left out, the columns
   before ViCol are not shown, and an arrow at either end says that the
   line goes on there. In hex it is the place in the file, the bytes, four
   to a group, and the same bytes as characters. What was found last has
   the colour of the cursor.

   `dx` is what the assembly has in DX: it is stored for the last byte of
   what was found where nothing found is in this part of the file. Nothing
   reads it then. */
void V32_00(u16 segment, u16 number, u16 at, u16 end, u16 dx)
{
    const u8 __far *text = TEXT(segment);
    u8 *screen = (u8 *)BasAdr(VI->ViBase + (number << 8));
    u8 *chars;
    u32 found, position;
    u16 first, column, place, behind, word, color, count;
    u8 c;

    first = 0xFFFF;
    found = VI->ViSearch + 1 - VI->ViOffset;
    if ((u16)(found >> 16) == 0) {
        first = (u16)found;
        dx = first + VI->ViSrLen - 1;
    }
    VI->ViTmp1 = first;
    VI->ViTmp2 = dx;

    if (VI->ViTxHex == 0) {
        column = 0;
        if (end >= at && end - at >= 2) {
            word = WORD_AT(text, end - 2);
            if (IS_CRLF(word))
                end -= 2;
        }
        while (at < end) {
            c = text[at++];
            behind = at;
            place = column;
            column++;
            if (c == 9) {
                c = ' ';
                if (column & 7)
                    at--;
            }
            if (place < VI->ViCol)
                continue;
            place -= VI->ViCol;
            if ((u8)place >= (u8)VI->ViSize) {
                *(u16 *)(screen + ((VI->ViSize & 0xFF) << 1) - 2) =
                    Color(Arrow_C) << 8 | 0x1A;
                break;
            }
            place <<= 1;
            screen[place] = c;
            if (behind >= VI->ViTmp1 && behind <= VI->ViTmp2
                && VI->ViSrPan != 0)
                screen[place + 1] = (u8)Color(Cursr_C);
        }
        if (VI->ViCol != 0)
            *(u16 *)screen = Color(Arrow_C) << 8 | 0x1B;
        return;
    }

    position = VI->ViOffset + at;
    screen += 2;
    screen = (u8 *)HexByt((u16 *)screen, (u16)(position >> 24));
    screen = (u8 *)HexByt((u16 *)screen, (u16)(position >> 16));
    screen = (u8 *)HexByt((u16 *)screen, (u16)position >> 8);
    screen = (u8 *)HexByt((u16 *)screen, (u16)position);
    screen += 4;
    chars = screen + 2 * 52;
    color = Color(Cursr_C);
    for (count = 16; count; count--) {
        if (at >= end)
            break;
        c = text[at++];
        *chars = c != 0 ? c : '.';
        chars += 2;
        screen = (u8 *)HexByt((u16 *)screen, c) + 2;
        if (at >= VI->ViTmp1 && at <= VI->ViTmp2) {
            if (VI->ViSrPan != 0) {
                chars[-1] = (u8)color;
            } else {
                screen[-3] = (u8)color;
                screen[-5] = (u8)color;
                if (at != VI->ViTmp1 && count < 16) {
                    screen[-7] = (u8)color;
                    screen[-9] = (u8)color;
                }
            }
        }
        if ((count & 3) == 1)
            screen += 2;
    }
}

/* Whether the part in the memory goes to the end of the file: 0 if it
   does, with 100 written to the status line for the per cent, and 1 if it
   does not. */
u16 V40_00(void)
{
    if (VI->ViOffset + VI->ViLoaded < VI->ViLen)
        return 1;
    VI->Vi100pc = 1;
    if (VI->ViStatus != 0) {
        *(u8 *)(md.ScrBuf + 75) = '1';
        *(u8 *)(md.ScrBuf + 76) = '0';
        *(u8 *)(md.ScrBuf + 77) = '0';
        Window(75, 0x103);
    }
    return 0;
}

/* Go to `position` in the file and, if DOS says it is there, read
   `count` bytes to `to` in the text. */
static void Load(u16 segment, u32 position, u16 to, u16 count)
{
    struct DosRegs regs;

    regs.ax = 0x4200;
    regs.bx = VI->ViHandle;
    regs.cx = (u16)(position >> 16);
    regs.name = TEXT(segment) + (u16)position;          /* DX */
    Intr21((struct DosRegs STACK *)&regs);
    if (regs.ax == (u16)position
        && (u16)(u32)regs.name == (u16)(position >> 16))
        V55_00(segment, to, count, VI->ViHandle);
}

/* Read the next part: the second half of what is in the memory becomes
   the first, with the lines of the table that are in it, and the file is
   read on behind it, as far as the table leaves room. */
void V50_00(u16 segment)
{
    u8 __far *text = TEXT(segment);
    u16 half = VI->ViSpace >> 1;
    u16 from, to, at, count, kept;
    u32 rest;

    VI->ViOffset += half;
    VI->ViFirst -= half;
    from = to = VI->ViSpace - 2;
    WORD_AT(text, to) = 0;
    while (from >= VI->ViEndTab) {
        at = WORD_AT(text, from);
        from -= 2;
        if (at <= half)
            continue;
        to -= 2;
        WORD_AT(text, to) = at - half;
    }
    VI->ViEndTab = to;
    rest = VI->ViLen - VI->ViOffset;
    if ((u16)(rest >> 16) == 0 && to > (u16)rest)
        to = (u16)rest;
    count = VI->ViLoaded - half;
    VI->ViLoaded = to;
    from = half;
    for (kept = 0; kept != count; kept++)
        text[kept] = text[from++];
    Load(segment, VI->ViOffset + kept, kept, VI->ViEndTab - kept);
    to = VI->ViEndTab;
    V60_00(segment, WORD_AT(text, to), to + 2);
}

/* Read the part before: what is in the memory moves up by half of the
   room, or by as much as the file has before it, the file is read into
   the room that leaves, and the table of lines is made again: from the
   start, and from where the second line began before, so that the lines
   behind it are the same as they were. */
void V51_00(u16 segment)
{
    u8 __far *text = TEXT(segment);
    u16 space = VI->ViSpace;
    u16 shift = space >> 1;
    u16 loaded, second, from, to, count;

    if ((u16)(VI->ViOffset >> 16) == 0 && shift > (u16)VI->ViOffset)
        shift = (u16)VI->ViOffset;
    VI->ViOffset -= shift;
    VI->ViFirst += shift;
    loaded = VI->ViLoaded + shift;
    if (loaded < shift || loaded >= space)
        loaded = space;
    VI->ViLoaded = loaded;
    second = 0;
    if (space - 2 > VI->ViEndTab)
        second = WORD_AT(text, space - 4);
    second += shift;
    from = space - shift;
    to = space;
    for (count = from; count; count--)
        text[--to] = text[--from];
    Load(segment, VI->ViOffset, 0, shift);
    V60_00(segment, 0, space);
    to = space;
    for (;;) {
        if (to <= VI->ViEndTab)
            return;
        to -= 2;
        from = WORD_AT(text, to);
        if (second == from)
            return;
        if (second < from)
            break;
    }
    V60_00(segment, second, to + 2);
}

/* Read `count` bytes of the file `handle` to `to` in the text, which is
   filled with zeros first. */
void V55_00(u16 segment, u16 to, u16 count, u16 handle)
{
    struct DosRegs regs;
    u8 __far *text = TEXT(segment) + to;
    u16 left;

    regs.name = text;
    for (left = count; left; left--)
        *text++ = 0;
    regs.ax = 0x3F00;
    regs.bx = handle;
    regs.cx = count;
    Intr21((struct DosRegs STACK *)&regs);
}

/* Make the table of lines for the text from `at` on: `table` is the
   entry above the one that the line at `at` gets. Where the table meets
   the text, the part that is in the memory ends. */
void V60_00(u16 segment, u16 at, u16 table)
{
    u8 __far *text = TEXT(segment);
    u16 loaded = VI->ViLoaded;
    u16 column, word;

    for (;;) {
        column = 0;
        if (at < table) {
            table -= 2;
            WORD_AT(text, table) = at;
            if (table < loaded)
                loaded = table;
        } else {
            loaded = table;
        }
        for (;;) {
            if (at >= loaded)
                goto done;
            word = WORD_AT(text, at);
            at += 2;
            if (at <= loaded && IS_CRLF(word))
                break;
            at--;
            if ((u8)word == 9)
                column |= 7;
            column++;
            if (column < 80)
                continue;
            if (column > 80)
                break;
            /* A line of 80 columns that ends here is not followed by an
               empty one. */
            word = WORD_AT(text, at);
            if (IS_CRLF(word))
                continue;
            if (at < loaded)
                break;
            goto done;
        }
    }
done:
    VI->ViLoaded = loaded;
    VI->ViEndTab = table;
    VI->ViPoint = VI->ViSpace - 2;
}

/* Make of the text that is looked for, SrchStr, what the search compares
   with: the text in capitals and, if the viewer or the editor shows hex,
   the bytes that the text names in hex digits: one or two digits to a
   byte, with spaces or a comma between bytes. A text that is not that
   names no bytes. */
void V81_00(void)
{
    const u8 *from;
    u8 *to;
    u16 digit;
    u8 c;

    to = (u8 *)MvzLine(md.HelpBuf, md.SrchStr, LenStr + 1);
    PathLin(md.HelpBuf);
    SRCH_LEN = to - md.HelpBuf;
    HexLen = 0;
    if (VI->ViTxHex == 0)
        return;
    from = md.SrchStr;
    to = md.HelpBuf + SRCH_HEX;
    for (;;) {
        do
            c = *from++;
        while (c == ' ');
        if (c == 0)
            return;
        digit = TstHex(c);
        if (digit > 0xFF)
            break;
        *to = (u8)digit;
        HexLen++;
        c = *from++;
        if (c == 0)
            return;
        digit = TstHex(c);
        if (digit <= 0xFF) {
            *to = (u8)((*to << 4) + digit);
            from++;
        }
        from--;
        to++;
        do
            c = *from++;
        while (c == ' ');
        if (c != ',')
            from--;
    }
    HexLen = 0;
}

/* Look for what V81_00 has prepared in `count` bytes of a text. The high
   word of what is returned is where the search ended: behind what was
   found. The low word is 1 if the text was found, capitals and small
   letters alike, 0 if the bytes were, and 100h if neither. */
u32 V82_00(u16 segment, u16 at, u16 count)
{
    const u8 __far *text = TEXT(segment);
    u16 more = SRCH_LEN - HexLen;       /* places at the end where only
                                           the bytes have room */
    u16 start, left, i;

    if (count >= SRCH_LEN) {
        count = count - SRCH_LEN + 1;
    } else {
        count -= SRCH_LEN;
        more += count;
        if (more >= count)              /* fewer bytes than are looked for */
            goto none;
        more++;
        count = 0;
        goto bytes_only;
    }
    for (;;) {
        start = at;
        i = SRCH_TEXT;
        left = SRCH_LEN;
        for (;;) {
            if ((u8)UpCase(text[at++]) != md.HelpBuf[i++])
                break;
            if (--left == 0)
                return (u32)at << 16 | 1;
        }
        at = start;
bytes:
        left = HexLen;
        if (left != 0) {
            i = SRCH_HEX;
            for (;;) {
                if (text[at++] != md.HelpBuf[i++])
                    break;
                if (--left == 0)
                    return (u32)at << 16;
            }
        }
        at = start + 1;
        if (--count)
            continue;
bytes_only:
        if (more-- == 0)
            break;
        count = 1;
        start = at;
        goto bytes;
    }
none:
    return (u32)at << 16 | 0x100;
}
