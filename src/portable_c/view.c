/*
 * The viewer: opening a file for it or for the editor, the lines of the
 * file it shows, the part of the file that is in the memory, what the
 * screen shows of it, its keys, and a text looked for.
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

/* A place in the text, and the entry of the table of lines for it: what
   the original has in SI and BX for NextLin and PrevLin. */
struct Line {
    u16 at;                             /* SI */
    u16 entry;                          /* BX */
};

void V80_00(u16 include, struct Box *where);
u16 NextLin(u16 segment, struct Line *line);
u16 PrevLin(u16 segment, struct Line *line);
u16 V20_00(u16 segment);
void V25_00(void);
u16 V30_00(u16 segment);
void V31_00(u16 place);
void V32_00(u16 segment, u16 number, u16 at, u16 end);
u16 V40_00(void);
void V50_00(u16 segment);
void V51_00(u16 segment);
void V55_00(u16 segment, u16 to, u16 count, u16 handle);
void V60_00(u16 segment, u16 at, u16 table);
void V81_00(void);

/* V00_02, V00_10, V13_00 and V13_01, which the main loop goes on in, are
   declared in vc.h, with struct Open, which V00_02 fills. The others are
   called by the keys of the viewer, which are looked at here: V80_00 and
   V82_00 by the editor too, V82_00 by the file search. The segment of the
   text is what the assembly has in DS. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   ClrKbd empties the keyboard buffer if the setup has it so; the bars and
   the key that is waited for are C, in vc.h. GetMous returns the buttons
   of the mouse that are down and where the mouse is, or `place` as it
   was if there is no mouse. InWind is not 0 if a place is
   outside a window. ChgDisk asks for the disk of a drive that shares the
   floppy drive with another, and is not 0 if the user would not change it.
   ResWin puts back what a box covers. AsmSeg returns the segment of the
   program's constants, where the boxes and texts are. */

/* C routines of other files that vc.h does not have: a panel's frame,
   the path over it and what it lists, as files or as the tree; characters
   written to the screen buffer in a colour; and the editor's routines
   that clear a line of the window, by its number, and move the lines of
   the window up or down by one (see each). */
void Panel(struct WCBdef *panel);                       /* panel.c */
void PutPath(struct WCBdef *panel);
void PutFls(struct WCBdef *panel);
void PutTre(struct WCBdef *panel);                      /* tree.c */
u16 *MvsBw(u16 *screen, const u8 *text, u16 count,
           u16 color);                                  /* screen.c */
void T71_00(u16 function, u16 place, u16 step);         /* edit.c */

/* Boxes, dialogs and texts of view_t.c, which DialBox and Dialog
   take by their offsets in the segment that AsmSeg returns. */
extern const u8 ViFl_S[];               /* " View " */
extern const u8 EdFl_S[];               /* " Edit " */
extern const u8 Col_S[];                /* "Col" */
extern const u8 RdFil_P[];              /* "Reading the file" */
extern const u8 FndEr_B[];              /* "Can't find the file" */
extern const u8 Srch_B[];               /* "Search for the string" */
extern const u8 SrFor_P[];              /* "Searching for" */
extern const u8 SrEr_B[];               /* "Could not find the string" */

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

/* What the status line says behind the size of the file. */
static const u8 Byts_S[6] = " Bytes";

#define Menu_C 7                        /* the colour of the menu bar, and of
                                           the status line */

/* An error of DOS: no such file; and the one of a call that the user
   gave up when a drive was not ready (sys.h). */
#define ERR_NO_FILE   0x02
#define ERR_NOT_READY 0x53

/* The text in a segment, and a word of it or of the table of lines. The
   place of the word is cut to 16 bits: the text begins where its segment
   does, so a place that has wrapped round, two before 0, is the word
   that it is on DOS, at the segment's end. */
#define TEXT(segment) AT(segment, 0)
#define WORD_AT(text, at) (*(u16 *)((text) + (u16)(at)))

/* Whether two bytes end a line: CR LF, or LF CR. */
#define IS_CRLF(word) ((word) == 0x0A0D || (word) == 0x0D0A)

/* Open the file ViewFil for the viewer or, if `edit` is not 0, for the
   editor, with the box "Reading the file" on the screen: `menu` is the
   item of the menu bar to mark, and `whole` is not 0 for a window that
   is all of the screen between the status line and the key bar, where
   else the window is set already, for the dirinfo of an Info panel.

   Returns 0 if there is nothing to show: the file is not there, which a
   box says, its disk is not put in, or a key has stopped it. The box is
   off the screen then. Else `open` has the file and where the box is,
   ViLen how long the file is, and 1 is returned for the editor and 2 for
   the viewer. A file that is not there is a new one to the editor, with
   0FFFFh for a handle.

   A key that stops it, Esc, leaves the file open, as the assembly
   does. */
u16 V00_02(u16 menu, u16 edit, u16 whole, struct Open *open)
{
    u32 length;
    struct Answer file;
    struct FilePlace end;
    u16 include, handle;

    if (whole) {
        VI->ViBase = 0x100;
        VI->ViSize = (u16)(u8)(md.Lines - 2) << 8 | 80;
    }
    md.WinMous = 0;
    PutMnu(menu);
    VI->ViewEdit = (u8)edit;
    include = OFFSET(ViFl_S);
    if ((u8)edit != 0)
        include = OFFSET(EdFl_S);
    DialBox(0x528, OFFSET(RdFil_P), include, &md, &open->box);
    Window(open->box.begin, open->box.size);
    if (ChgDisk(md.ViewFil))
        goto none;
    file = OpenFil(md.ViewFil, 0x40);
    handle = file.value;
    if (file.failed != 0) {
        if (handle == ERR_NOT_READY)
            goto none;
        include = OFFSET(ViFl_S);
        if (VI->ViewEdit != 0) {
            if (handle == ERR_NO_FILE) {
                handle = 0xFFFF;
                goto opened;
            }
            include = OFFSET(EdFl_S);
        }
        Dialog(OFFSET(FndEr_B), include, 0, 0, &md);
        goto none;
    }
opened:
    if (TstBrk())
        goto none;
    VI->ViHandle = handle;
    length = 0;
    if (handle != 0xFFFF) {
        end = FileSeek(handle, 2, 0);
        if (!end.failed)
            length = end.place;
    }
    VI->ViLen = length;
    FileSeek(handle, 0, 0);
    VI->ViKeyBar = (u16)md.KeyBar << 8 | md.Clock;
    open->handle = handle;
    if (VI->ViewEdit != 0)
        return 1;
    return 2;
none:
    ResWin(open->box.begin, open->box.size, open->box.saved);
    Window(open->box.begin, open->box.size);
    return 0;
}

/* Find where the line behind the one at `line` begins. Returns 0 and
   leaves it there, or 1 if the part in the memory has no line behind
   this one: the place then stays and the entry is the first of the
   table. */
u16 NextLin(u16 segment, struct Line *line)
{
    const u8 *text = TEXT(segment);
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
u16 PrevLin(u16 segment, struct Line *line)
{
    const u8 *text = TEXT(segment);
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
    const u8 *text = TEXT(segment);
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
   the memory ends before the window is full, and else 0. */
u16 V30_00(u16 segment)
{
    const u8 *text = TEXT(segment);
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
        if (NextLin(segment, &line))
            break;
        V32_00(segment, number, at, line.at);
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
    V32_00(segment, number, at, end);
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

   Where nothing found is in this part of the file, ViTmp1 says so and
   ViTmp2 stays as it is. The assembly stores there what DX happens to
   hold. Nothing reads it: a place is held against ViTmp2 only if it is
   not below ViTmp1, 0FFFFh then, and no place is that far, the part
   being 0FFE0h bytes at the most; and the editor, which has the shape of
   the cursor in the same word, EdShape, sets that when it opens a
   file. */
void V32_00(u16 segment, u16 number, u16 at, u16 end)
{
    const u8 *text = TEXT(segment);
    u8 *screen = (u8 *)BasAdr(VI->ViBase + (number << 8));
    u8 *chars;
    u32 found, position;
    u16 first, column, place, behind, word, color, count;
    u8 c;

    first = 0xFFFF;
    found = VI->ViSearch + 1 - VI->ViOffset;
    if ((u16)(found >> 16) == 0) {
        first = (u16)found;
        VI->ViTmp2 = first + VI->ViSrLen - 1;
    }
    VI->ViTmp1 = first;

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
    if (FileSeek(VI->ViHandle, 0, position).place == position)
        V55_00(segment, to, count, VI->ViHandle);
}

/* Read the next part: the second half of what is in the memory becomes
   the first, with the lines of the table that are in it, and the file is
   read on behind it, as far as the table leaves room. */
void V50_00(u16 segment)
{
    u8 *text = TEXT(segment);
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
    u8 *text = TEXT(segment);
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
    if ((u16)(space - 2) > VI->ViEndTab)
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
    u8 *text = TEXT(segment) + to;
    u8 *at = text;
    u16 left;

    for (left = count; left; left--)
        *at++ = 0;
    FileRead(handle, text, count);
}

/* Make the table of lines for the text from `at` on: `table` is the
   entry above the one that the line at `at` gets. Where the table meets
   the text, the part that is in the memory ends. */
void V60_00(u16 segment, u16 at, u16 table)
{
    u8 *text = TEXT(segment);
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

/* Write the status line of the viewer but for the column and the per
   cent, which V25_00 writes: the name of the file and its size. */
static void Status(void)
{
    u16 *screen;
    u16 cell = Color(Menu_C) << 8 | ' ';
    u16 code = AsmSeg();
    u16 count;

    for (count = 0; count != 80; count++)
        md.ScrBuf[count] = cell;
    screen = MvsBw(md.ScrBuf, AT(code, OFFSET(ViFl_S) + 1), 4, cell);
    *screen = cell & 0xFF00 | ':';
    MvsBw(md.ScrBuf + 40, AT(code, OFFSET(Col_S)), 3, cell);
    for (count = 0; count != LenPath + 13; count++)
        md.TmpBuf[count] = md.ViewFil[count];
    LetLin(md.TmpBuf);
    ShrtLin(md.TmpBuf, 33);
    MovDz0(md.ScrBuf + 6, md.TmpBuf);
    Number(md.TmpBuf, HIGH(VI->ViLen), LOW(VI->ViLen),
           NUM_SEPARATED | NUM_PADDED);
    screen = MovDz0(md.ScrBuf + 51, md.TmpBuf);
    count = 6;
    if (VI->ViLen == 1)
        count--;
    MvsBw(screen, Byts_S, count, cell);
    *(u8 *)(md.ScrBuf + 78) = '%';
}

/* The first line of the screen, as V20_00 finds it, the line behind
   `line` and the one before it. */
#define FIRST() (line.at = V20_00(segment), line.entry = VI->ViPoint)
#define NEXT()  NextLin(segment, &line)
#define PREV()  PrevLin(segment, &line)

/* The viewer: read the file `handle`, whose length is in ViLen, show it
   in the window ViBase and ViSize, with a status line if that is as wide
   as the screen, and do what the keys and the mouse ask for, until Esc,
   F3 or F10 leave it. Then the file is closed and the panels are on the
   screen again.

   A button of the mouse held in the upper or the lower third of the
   window is Up or Down, and in the left or the right quarter of what is
   between them Left or Right. */
void V00_10(u16 handle)
{
    u16 segment = SEG_OF(Room);
    const u8 *text = TEXT(segment);
    struct Line line;
    struct Box box;
    struct Answer read;
    u32 rest, position;
    struct Sought sought;
    struct Mouse pointer;
    u16 count, key, bar, step, column, at, end, number, begin, size;
    u16 starts, panel, length, behind, in_file, result, edge;
    u16 mouse = 0;
    u8 third, quarter;

    count = RoomParas();
    if (count > 0xFFE)
        count = 0xFFE;
    count <<= 4;
    VI->ViSpace = count;
    V55_00(segment, 0, count, handle);
    if (HIGH(VI->ViLen) == 0)
        count = LOW(VI->ViLen);
    VI->ViLoaded = count;
    VI->ViOffset = 0;
    VI->ViFirst = 0;
    VI->ViCol = 0;
    VI->ViTxHex = 0;
    md.Clock = 0;
    md.KeyBar = 1;
    VI->ViWrap = 1;
    VI->ViSearch = 0xFFFFFFF0;
    md.HlpPage = 3;
    if ((u8)VI->ViSize < 79) {
        VI->ViStatus = 0;
    } else {
        Status();
        VI->ViStatus = 1;
    }

V00_15:                                 /* the part in the memory is new */
    V60_00(segment, 0, VI->ViSpace);
V00_20:                                 /* write the screen */
    V25_00();
    if (!V30_00(segment))
        goto V00_30;
V00_21:                                 /* the text ends on the screen */
    if (!V40_00())
        goto V00_30;
    if (VI->ViFirst < VI->ViSpace >> 1)
        goto V00_30;
    V50_00(segment);
V00_22:
    V30_00(segment);

V00_30:                                 /* the mouse */
    pointer = GetMous(mouse);
    mouse = pointer.place;
    if (pointer.buttons == 0) {
        md.WinMous = 0;
        goto V00_40;
    }
    ClrKbd();
    begin = VI->ViBase;
    size = VI->ViSize;
    if (md.WinMous == 0 && InWind(mouse, begin, size))
        goto V00_30;
    md.WinMous = 1;
    third = (u8)((u16)(signed char)(size >> 8) / 3);
    if ((u8)(mouse >> 8) < (u8)((begin >> 8) + third))
        goto V05_00;
    if ((u8)(mouse >> 8) >= (u8)((begin >> 8) + (size >> 8) - third))
        goto V06_00;
    quarter = (u8)size >> 2;
    if ((u8)mouse < (u8)(begin + quarter))
        goto V01_00;
    if ((u8)mouse >= (u8)(begin + size - quarter))
        goto V03_00;

V00_40:                                 /* a key */
    bar = 3;
    if (VI->ViTxHex == 0) {
        bar = 4;
        if (VI->ViWrap != 0)
            bar = 5;
    }
    key = Input(bar);
    ClrKbd();
    /* The mouse, or a key without a scan code: Case takes none. */
    if (key >> 8 == 0xFC || key >> 8 == 0)
        goto V00_30;
    switch (key & 0xFF) {
    case 0x1B:                          /* Esc */
        goto leave;
    case 0x13:                          /* ^S */
        goto V01_00;
    case 0x04:                          /* ^D */
        goto V03_00;
    case 0x01:                          /* ^A */
        goto V02_00;
    case 0x06:                          /* ^F */
        goto V04_00;
    case 0x05:                          /* ^E */
        goto V05_00;
    case 0x18:                          /* ^X */
        goto V06_00;
    case 0x12:                          /* ^R */
        goto V07_00;
    case 0x03:                          /* ^C */
        goto V08_00;
    case 0x0D:                          /* Enter */
        goto V11_00;
    }
    switch (key) {
    case 0x3D00:                        /* F3 */
    case 0x4400:                        /* F10 */
        goto leave;
    case 0x4B00:                        /* Left */
        goto V01_00;
    case 0x4D00:                        /* Right */
        goto V03_00;
    case 0x7300:                        /* ^Left */
        goto V02_00;
    case 0x7400:                        /* ^Right */
        goto V04_00;
    case 0x4800:                        /* Up */
        goto V05_00;
    case 0x5000:                        /* Down */
        goto V06_00;
    case 0x4900:                        /* PgUp */
        goto V07_00;
    case 0x5100:                        /* PgDn */
        goto V08_00;
    case 0x4700:                        /* Home, ^Home, ^PgUp */
    case 0x7700:
    case 0x8400:
        goto V09_00;
    case 0x4F00:                        /* End, ^End, ^PgDn */
    case 0x7500:
    case 0x7600:
        goto V10_00;
    case 0x3E00:                        /* F4 */
        goto V11_00;
    case 0x3C00:                        /* F2 */
        goto V12_00;
    case 0x4100:                        /* F7 */
        goto V14_00;
    case 0x5A00:                        /* Shift-F7 */
        goto V15_00;
    }
    goto V00_30;

V01_00:                                 /* Left */
    step = 1;
    goto V02_01;
V02_00:                                 /* ^Left */
    step = 40;
V02_01:
    if (VI->ViTxHex != 0)
        goto V00_30;
    column = VI->ViCol;
    if (column == 0)
        goto V00_30;
    if (column < step)
        column = 0;
    else
        column -= step;
V02_02:
    if (column == VI->ViCol)
        goto V00_30;
    VI->ViCol = column;
    goto V00_20;

V03_00:                                 /* Right */
    step = 1;
    goto V04_01;
V04_00:                                 /* ^Right */
    step = 40;
V04_01:
    if (VI->ViTxHex != 0)
        goto V00_30;
    column = VI->ViCol;
    if (column >= 0xFFFF - 80)
        goto V00_30;
    column += step;
    if (column >= 0xFFFF - 80)
        column = 0xFFFF - 80;
    goto V02_02;

V05_00:                                 /* Up */
    FIRST();
    if (line.at == 0 && VI->ViOffset == 0)
        goto V00_30;
    if (PREV())
        goto V05_01;
    if (line.at != 0)
        goto V05_02;
    if (VI->ViSpace >> 1 <= VI->ViFirst)
        goto V05_02;
V05_01:                                 /* the part before, if there is one */
    if (VI->ViOffset == 0)
        goto V05_02;
    V51_00(segment);
    FIRST();
    PREV();
    VI->ViFirst = line.at;
    VI->ViPoint = line.entry;
    V25_00();
    goto V00_22;
V05_02:                                 /* the window a line down, and the
                                           new line in its first row */
    VI->ViFirst = line.at;
    VI->ViPoint = line.entry;
    V25_00();
    begin = VI->ViBase;
    T71_00(0x700, begin + ((VI->ViSize >> 8) - 1 << 8), (u16)-(2 * 80));
    V31_00(begin);
    FIRST();
    at = line.at;
    end = VI->ViLoaded;
    if (!NEXT())
        end = line.at;
    V32_00(segment, 0, at, end);
    Window(VI->ViBase, 0x100 | (u8)VI->ViSize);
    /* Whether the window is full: from the first line again, with the
       entry of the table that the line behind it has left. */
    line.at = at;
    count = VI->ViSize >> 8;
    do {
        if (NEXT())
            goto V06_05;
    } while (--count);
    goto V00_30;

V06_00:                                 /* Down */
    if (VI->Vi100pc != 0)
        goto V00_30;
    FIRST();
    if (NEXT()) {
        V50_00(segment);
        VI->ViFirst = 0;
        VI->ViPoint = VI->ViSpace - 2;
        V25_00();
        goto V00_22;
    }
    VI->ViFirst = line.at;
    VI->ViPoint = line.entry;
    V25_00();
    /* The window a line up, and the new line in its last row. */
    begin = VI->ViBase;
    T71_00(0x600, begin, 2 * 80);
    V31_00(begin + ((VI->ViSize >> 8) - 1 << 8));
    number = (u16)(signed char)(VI->ViSize >> 8) - 1;
    count = number;
    FIRST();
    do {
        if (NEXT())
            goto V06_05;
    } while (--count);
    at = line.at;
    end = VI->ViLoaded;
    result = NEXT();
    if (!result)
        end = line.at;
    V32_00(segment, number, at, end);
    Window(VI->ViBase + (number << 8), 0x100 | (u8)VI->ViSize);
    if (!result)
        goto V00_30;
V06_05:
    V40_00();
    goto V00_21;

V07_00:                                 /* PgUp */
    FIRST();
    if (line.at == 0 && VI->ViOffset == 0)
        goto V00_30;
    count = (VI->ViSize >> 8) - 2;
    do {
        if (PREV())
            goto V07_02;
    } while (--count);
    if (line.at != 0)
        goto V07_04;
V07_02:                                 /* the part before, if there is one */
    if (VI->ViSpace >> 1 <= VI->ViFirst)
        goto V07_04;
    if (VI->ViOffset == 0)
        goto V07_04;
    V51_00(segment);
    FIRST();
    count = (u8)(md.Lines - 4);
    do {
        if (PREV())
            break;
    } while (--count);
V07_04:
    VI->ViFirst = line.at;
    VI->ViPoint = line.entry;
    V25_00();
    goto V00_22;

V08_00:                                 /* PgDn */
    if (VI->Vi100pc != 0)
        goto V00_30;
    FIRST();
    /* Where the lines below begin goes to TmpBuf, a word for each: the
       last of them is the first line of the next screen. */
    starts = MD_PLACE(md.TmpBuf) - 2;
    count = (VI->ViSize >> 8) - 2;
    do {
        if (NEXT())
            goto V08_06;
        starts += 2;
        *(u16 *)MD_POINTER(starts) = line.at;
    } while (--count);
    /* And a screen from there: where it is not full and the file ends,
       back by as many lines as it lacks. */
    count = (VI->ViSize >> 8) - 1;
    do {
        if (NEXT())
            goto V08_03;
    } while (--count);
    goto V08_05;
V08_03:
    if (VI->ViOffset + VI->ViLoaded < VI->ViLen)
        goto V08_05;
    count += count;
    if (starts < count)
        goto V00_30;
    starts -= count;
    if (starts < MD_PLACE(md.TmpBuf))
        goto V00_30;
V08_05:
    VI->ViFirst = *(u16 *)MD_POINTER(starts);
    goto V00_20;
V08_06:                                 /* the part ends: the next one */
    V50_00(segment);
    VI->ViFirst = 0;
    VI->ViPoint = VI->ViSpace - 2;
    goto V00_20;

V09_00:                                 /* Home */
    if (VI->ViOffset == 0) {
        if ((VI->ViFirst | VI->ViCol) == 0)
            goto V00_30;
        VI->ViFirst = 0;
        VI->ViCol = 0;
        goto V00_20;
    }
    VI->ViOffset = 0;
    VI->ViFirst = 0;
    VI->ViCol = 0;
    count = VI->ViSpace;
    if (HIGH(VI->ViLen) == 0 && count > LOW(VI->ViLen))
        count = LOW(VI->ViLen);
    VI->ViLoaded = count;
    FileSeek(VI->ViHandle, 0, 0);
    V55_00(segment, 0, count, VI->ViHandle);
    goto V00_15;

V10_00:                                 /* End */
    if (VI->Vi100pc != 0)
        goto V00_30;
    rest = VI->ViLen - VI->ViOffset;
    if (HIGH(rest) != 0 || LOW(rest) > VI->ViLoaded) {
        /* The end of the file is not in the memory: half a part of it,
           from an offset that is a multiple of 16. */
        count = (VI->ViSpace >> 1) - 0x10 + (LOW(VI->ViLen) & 0xF);
        VI->ViOffset = VI->ViLen - count;
        VI->ViLoaded = count;
        VI->ViFirst = 0;
        Load(segment, VI->ViOffset, 0, VI->ViLoaded);
        V60_00(segment, 0, VI->ViSpace);
    }
    line.entry = VI->ViEndTab;
    line.at = WORD_AT(text, line.entry);
    count = (VI->ViSize >> 8) - 1;
    if (VI->ViTxHex != 0)
        line.at = VI->ViLoaded - 1;
    do {
        if (PREV())
            break;
    } while (--count);
    VI->ViFirst = line.at;
    VI->ViPoint = line.entry;
    goto V00_20;

V11_00:                                 /* F4 - ASCII / Hex */
    VI->ViTxHex = VI->ViTxHex == 0;
    goto V00_20;

V12_00:                                 /* F2 - Wrap / Unwrap */
    if (VI->ViTxHex != 0)
        goto V00_30;
    VI->ViWrap = VI->ViWrap == 0;
    goto V00_20;

V14_00:                                 /* F7 - search */
    if ((u8)Dialog(OFFSET(Srch_B), OFFSET(ViFl_S), 0, 0, &md) != 0)
        goto V00_30;
V15_00:                                 /* Continue search */
    if (md.SrchStr[0] == 0)
        goto V00_30;
    V80_00(OFFSET(ViFl_S), &box);
    /* From the first line of the screen, or from the line behind it if
       something has been found before. */
    at = VI->ViFirst;
    if (HIGH(VI->ViSearch) != 0xFFFF) {
        line.at = at;
        line.entry = VI->ViPoint;
        at = VI->ViLoaded;
        if (!NEXT())
            at = line.at;
    }
    VI->ViSearch = VI->ViOffset + at;
    if (VI->ViLoaded < at)
        goto V15_04;
    /* In what is in the memory. */
    count = VI->ViLoaded - at;
    sought = V82_00(segment, at, count);
    if (sought.found & 0x100)
        goto V15_06;
    behind = sought.end - at;
    panel = sought.found;
    in_file = 0;
    goto V15_10;
V15_04:                                 /* and on in the file, a piece of
                                           it at a time in MenuBuf */
    if (TstBrk()) {
        result = 2;
        goto V15_22;
    }
    if (VI->ViLen <= VI->ViSearch)
        goto V15_07;
    rest = VI->ViLen - VI->ViSearch;
    count = LenMenu;
    if (HIGH(rest) == 0 && count > LOW(rest))
        count = LOW(rest);
    if (FileSeek(VI->ViHandle, 0, VI->ViSearch).place != VI->ViSearch)
        goto V15_07;
    read = FileRead(VI->ViHandle, md.MenuBuf, count);
    count = read.value;
    if (read.failed) {
        result = 2;
        goto V15_22;
    }
    sought = V82_00(MAIN_SEG, MD_PLACE(md.MenuBuf), count);
    behind = sought.end - MD_PLACE(md.MenuBuf);
    panel = sought.found;
    in_file = 1;
    if (!(sought.found & 0x100))
        goto V15_10;
V15_06:                                 /* not in these `count` bytes: on
                                           from where it may begin */
    if (count < SRCH_LEN)
        goto V15_07;
    count = count - SRCH_LEN + 1;
    VI->ViSearch += count;
    goto V15_04;
V15_07:
    Dialog(OFFSET(SrEr_B), OFFSET(ViFl_S), 0, 0, &md);
    result = 3;
    goto V15_22;
V15_10:                                 /* found, as text or as bytes */
    VI->ViSrPan = (u8)panel;
    length = SRCH_LEN;
    if ((u8)panel == 0)
        length = HexLen;
    VI->ViSrLen = (u8)length;
    behind -= length;
    VI->ViSearch += behind;
    if (in_file) {
        /* The part of the file that begins 2000 bytes before it, at a
           multiple of 16. */
        position = 0;
        if (VI->ViSearch >= 2000)
            position = VI->ViSearch - 2000;
        position &= 0xFFFFFFF0;
        VI->ViOffset = position;
        rest = VI->ViLen - position;
        count = VI->ViSpace;
        if (HIGH(rest) == 0 && count > LOW(rest))
            count = LOW(rest);
        VI->ViLoaded = count;
        Load(segment, VI->ViOffset, 0, VI->ViLoaded);
        V60_00(segment, 0, VI->ViSpace);
    }
    count = LOW(VI->ViSearch) - LOW(VI->ViOffset);
    VI->ViFirst = count;
    if (VI->ViTxHex != 0)
        goto V15_21;
    /* The column it is in, and the line moved so that it shows. */
    at = V20_00(segment);
    if (count < at)
        goto V15_21;
    count -= at;
    column = 0;
    for (; count; count--) {
        if (text[at++] == 9)
            column |= 7;
        column++;
    }
    if (column > VI->ViCol) {
        column += VI->ViSrLen;
        edge = VI->ViCol + (u8)VI->ViSize;
        if (column < edge)
            goto V15_21;
        column = column - (u8)VI->ViSize + 1;
    } else if (column != 0) {
        column--;
    }
    VI->ViCol = column;
V15_21:
    result = 0;
V15_22:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    if (result == 0)
        goto V00_20;
    VI->ViSearch = 0xFFFFFFF0;
    goto V00_30;

leave:
    V13_00();
}

/* Close the file of the viewer, or of an editor that could not read it,
   and go back to the panels. */
void V13_00(void)
{
    FileClose(VI->ViHandle);
    V13_01(0);
}

/* What PutFls of the original does: a panel shows files, the tree or
   the Info. */
static void Files(struct WCBdef *panel)
{
    if (panel->WinTyp == 0)
        PutFls(panel);
    else if (panel->WinTyp == 1)
        PutTre(panel);
    else
        PutInf(panel);
}

/* Go back to the panels from the viewer or the editor: what DOS had on
   the screen, the clock and the key bar as they were, the panels that
   are visible, the prompt, the menu bar and the key bar. `changed` is
   not 0 in its low byte if a file has been written, so that the panels
   are read again. */
void V13_01(u16 changed)
{
    struct WCBdef *panel;

    Quit();
    md.Clock = (u8)VI->ViKeyBar;
    md.KeyBar = (u8)(VI->ViKeyBar >> 8);
    panel = LeftPanel;
    if (panel->Visible != 0) {
        Panel(panel);
        PutPath(panel);
        Files(panel);
    }
    panel->ChgFlg |= (u8)changed;
    panel = RightPanel;
    if (panel->Visible != 0) {
        Panel(panel);
        PutPath(panel);
        Files(panel);
    }
    panel->ChgFlg |= (u8)changed;
    PutPrm(panel);
    PutMnu(0);
    PutKey(0xFF);
    Window(0, (u16)md.Lines << 8 | 80);
    md.WinMous = 0;
}

/* Show the box "Searching for" with the text that is looked for, and
   prepare the search: for the viewer and for the editor. `include` is
   the title of the box, an offset in the assembly, and `where` gets
   where the box is. */
void V80_00(u16 include, struct Box *where)
{
    DialBox(0x528, OFFSET(SrFor_P), include, &md, where);
    Window(where->begin, where->size);
    V81_00();
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

/* Look for what V81_00 has prepared in `count` bytes of a text. Returns
   where the search ended, behind what was found, and 1 if the text was
   found, capitals and small letters alike, 0 if the bytes were, and 100h
   if neither. */
static struct Sought Sought(u16 found, u16 end)
{
    struct Sought sought;

    sought.found = found;
    sought.end = end;
    return sought;
}

struct Sought V82_00(u16 segment, u16 at, u16 count)
{
    const u8 *text = TEXT(segment);
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
                return Sought(1, at);
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
                    return Sought(0, at);
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
    return Sought(0x100, at);
}

/*
 * The commands that view or edit a file: F3 and F4, Alt-F3 and Alt-F4,
 * which are the program's own viewer and editor whatever the setup, and
 * Shift-F3 and Shift-F4, which ask for a name.
 *
 * They are commands, as in keys.c and enter.c: functions that get the
 * panel which the original has in DS and return where the program goes
 * on, struct Onward of vc.h. NEXT_VIEW is V00_00 above for the file
 * ViewFil, NEXT_EDIT the same for the editor, and NEXT_DIRINFO is V00_02
 * for the editor in the window of an Info panel; the main loop goes on
 * from there to the editor, or back here: V00_03 of main.c.
 *
 * Where the setup has the viewer or the editor be another program, that
 * is looked for in VCVIEW.EXT or VCEDIT.EXT, of the current directory or
 * else of the program's: the line for files of this kind there is
 * commands to run, or nothing but the name of a program. Such a program
 * is run here, with the memory of the viewer given up for it and behind
 * the name of the file what such a program reads of the setup.
 */

/* The commands are declared in vc.h, and so are F34_30 and F34_40. */

/* More C routines of other files that vc.h does not have. */
const u8 *FndLet(const u8 *text);                       /* string.c */
u8 *MainFil(u8 *to, const u8 *name);
u16 Run_00(u16 text, u16 view);                         /* usermenu.c */

/* More of them, and two names of other files, those of the menu file
   and of the file that an Info panel shows. The names of the extension
   files of the viewer and of the editor are copied with what follows
   them, as those two are, and "Try rebooting." is too: in view_t.c
   each is written as long as what is copied. Then the dialog that asks
   for a name; the boxes shown while a program is loaded as the viewer
   or as the editor; and the box that says it could not be run, with
   its texts. */
extern const u8 ViewE_F[];              /* "VCVIEW.EXT" */
extern const u8 EditE_F[];              /* "VCEDIT.EXT" */
extern const u8 Menu_F[];               /* "VC.MNU" */
extern const u8 DrIn_F[];               /* "DIRINFO" */
extern const u8 ViFil_B[];              /* "View the file:" */
extern const u8 Viewr_P[];              /* "Loading the viewer:" */
extern const u8 Editr_P[];              /* "Loading the editor:" */
extern const u8 RunEr_B[];
extern const u8 RunEr_S[];              /* "Not enough memory to load" */
extern const u8 Run1_S[];               /* "Can't run commands for some
                                           reason." */
extern const u8 Run2_S[];               /* "Try rebooting." */


#define EOF_CHAR 0x1A

/* What a command returns, struct Onward of vc.h: the place where the
   program goes on, a NEXT_ there. None of these places takes a panel. */
static struct Onward To(u16 place)
{
    struct Onward next;

    next.place = (u8)place;
    next.item = 0;
    next.panel = 0;
    return next;
}

/* On in the program's own viewer, or in its editor if `edit` is not 0. */
static struct Onward Own(u16 edit)
{
    return To(edit != 0 ? NEXT_EDIT : NEXT_VIEW);
}

/* What a program that is run as the viewer or the editor finds behind
   the name of the file, in the tail of its command line: ViewPar, with
   36 zero bytes behind it. The palette is put into its byte 9, and what
   there is to know of the mouse into bytes 11 to 13. */
static const u8 ViewPar[9] = { 0, 'S', 'o', 'c', 'h', 'a', 0, 0x1F, 1 };

/* Look for the next command of a line of an extension file or a menu
   file: F34_30. `text` is where in MenuBuf to look from, in a command or
   behind one. Commands are lines that begin with a space or a tab; empty
   lines and lines that begin with an apostrophe are passed over.

   Returns where the command begins; or that there is none, and where
   the line begins that is no command, or the place behind the character
   that ends the file. */
static struct Next Next(u16 none, const u8 *at)
{
    struct Next next;

    next.none = none;
    next.at = MD_PLACE(at);
    return next;
}

struct Next F34_30(u16 text)
{
    const u8 *at = MD_POINTER(text);
    u16 word;

    for (;;) {
        at = NEAR(FndLet(at));
        word = *(const u16 *)at;
        at += 2;
        if (!IS_CRLF(word)) {
            at--;
            if ((u8)word == EOF_CHAR)
                break;
            at--;
            return Next(0, at);
        }
        for (;;) {                      /* behind the end of a line */
            word = *(const u16 *)at;
            if ((u8)word == ' ' || (u8)word == '\t')
                break;
            if (!IS_CRLF(word) && (u8)word != '\'')
                return Next(1, at);
            do {                        /* to the end of this one */
                word = *(const u16 *)at;
                at += 2;
                if (IS_CRLF(word))
                    break;
                at--;
            } while ((u8)word != EOF_CHAR);
            if (!IS_CRLF(word))
                return Next(1, at);
        }
    }
    return Next(1, at);
}

/* Is the command at `text` in MenuBuf nothing but the name of a file,
   and the last of its line: F34_40. For a viewer or an editor, `view`
   not 0, that is a name with an extension of three characters, and for a
   menu one that ends in ".MNU". A drive in front of it has to be a
   letter, and a "!" makes it a command.

   If so, the path of the file is at TmpBuf: the
   name as it is if it has a path of its own, and else in the directory
   of the extension file or the menu file, whose path is at TmpBuf. Else
   what is returned has the place before the command. The
   character behind the name is made a zero byte before the length of
   the name is found too great. */
struct Named F34_40(u16 text, u16 view)
{
    struct Named named;
    u16 code = AsmSeg();
    const u8 *suffix = AT(code, OFFSET(Menu_F) + 1);
    const u8 *from;
    u8 *to;
    u8 *at = MD_POINTER(text);
    u8 *end;
    u16 word, count;
    u8 c;

    word = *(const u16 *)at;
    if (word >> 8 == ':') {
        c = (u8)UpCase(word & 0xFF);
        if (c < 'A' || c > 'Z')
            goto command;
        at += 2;
    }
    for (;;) {
        word = *(const u16 *)at;
        at++;
        c = (u8)word;
        if (c == '!')
            goto command;
        if (IS_CRLF(word) || c == EOF_CHAR || c == ' ' || c == '\t')
            break;
    }
    at--;                               /* the character behind the name */
    end = at;
    at -= 4;
    count = 1;
    if (view == 0)
        count = 4;
    do {
        c = (u8)UpCase(*at++);
        suffix++;
        if (c != *suffix)
            goto command;
    } while (--count);
    if (F34_30(MD_PLACE(end)).none == 0)
        goto command;
    *end = 0;
    if ((u16)(end - MD_POINTER(text)) >= LenPath + 12)
        goto command;
    to = md.TmpBuf;
    if (MD_PLACE(NEAR(CutPath(MD_POINTER(text)))) == text)
        to = (u8 *)CutPath(md.TmpBuf);
    from = MD_POINTER(text);
    for (count = LenPath + 13; count != 0; count--)
        *to++ = *from++;
    named.file = 1;
    named.at = text + LenPath + 13;
    return named;
command:
    named.file = 0;
    named.at = text - 1;
    return named;
}

/* Run the program at TmpBuf as the viewer of the file ViewFil, or as the
   editor: Aexec0. The box that says so stays until the program is done,
   and the screen is then written as it was, which leaves nothing of what
   the program has shown; the panels are read again. A program that DOS
   will not run is said in a box. In the original this is the end of a
   command, which jumps to where the loop draws the panels: the command
   that calls it says so. */
static void Aexec0(u16 edit)
{
    u16 code = AsmSeg();
    struct Box where;
    struct Answer ran;
    struct Block all;
    const u8 *from;
    u8 *tail;
    u16 box, include, error, i;

    tail = NEAR(MvzLine(md.TmpBuf + 101, md.ViewFil, LenStr + 1));
    md.TmpBuf[100] = (u8)((u16)(tail - (md.TmpBuf + 101)) + 45);
    box = OFFSET(Viewr_P);
    if (edit != 0)
        box = OFFSET(Editr_P);
    DialBox(0x528, box, MD_PLACE(md.TmpBuf), &md, &where);
    Window(where.begin, where.size);
    *(u16 *)(md.TmpBuf + 95) = where.begin;
    *(u16 *)(md.TmpBuf + 97) = where.size;
    *(u16 *)(md.TmpBuf + 93) = CursorWhere(0).shape;
    for (i = 0; i < sizeof ViewPar; i++)
        tail[i] = ViewPar[i];
    for (; i < 45; i++)
        tail[i] = 0;
    tail[45] = 0x0D;
    tail[9] = md.ColTyp;
    if (md.Mouse != 0) {
        tail[11] = 0xFF;
        tail[12] = md.LftMous;
        tail[13] = 1;
    }
    /* The panels keep their memory, and the viewer as much as it needs
       at the least: the room is made that long. In the original the
       panels and the room are one block, which begins with the main
       data. */
    MemResize(SEG_OF(Room), ROOM_LEAST);
    LetLin(md.TmpBuf);
    ran = ProgramRun(md.TmpBuf, md.TmpBuf + 100);
    error = ran.value;
    /* All the memory that there is, again: DOS says how much that is
       when it is asked for more. */
    all = MemResize(SEG_OF(Room), 0xFFFF);
    MemResize(SEG_OF(Room), all.largest);
    HidCur();
    SavMous();
    IniMous();
    if (ran.failed && error != ERR_NOT_READY) {
        include = OFFSET(RunEr_S);
        if (error != 8 && error != 7) {
            include = OFFSET(Run1_S);
            from = AT(code, OFFSET(Run2_S));
            for (i = 0; i < 80; i++)
                md.TmpBuf[i] = from[i];
        }
        Dialog(OFFSET(RunEr_B), include, 0, 0, &md);
    }
    ResWin(*(u16 *)(md.TmpBuf + 95), *(u16 *)(md.TmpBuf + 97),
           MD_PLACE(md.WinBuf));
    Window(0, (u16)md.Lines << 8 | 80);
    CurShp(*(u16 *)(md.TmpBuf + 93));
    FncEx1();
}

/* View or edit the file ViewFil: F34_11. `how` is 1 in its low byte for
   the editor, and in its high byte for Alt-F3 and Alt-F4. Those are the
   program's own viewer and editor if the setup has the keys without Alt
   be something else, and the other way round if it has not.

   What else the viewer is, the extension file says, VCVIEW.EXT or
   VCEDIT.EXT. One that is not there, or has nothing for files of this
   kind, leaves the program's own; one that cannot be read, nothing. */
static struct Onward F34_11(u16 how)
{
    u16 code = AsmSeg();
    const u8 *name;
    const u8 *from;
    struct Named named;
    struct Next next;
    struct Answer file, read, found;
    u16 edit = how & 0xFF;
    u16 own = how >> 8;
    u16 count, command, i;

    if ((edit != 0 ? md.AltEdit : md.AltView) == 0)
        own = own == 0;
    if (own != 0)
        return Own(edit);
    HidCur();
    PutMnu(2);
    name = AT(code, OFFSET(ViewE_F));
    if (edit != 0)
        name = AT(code, OFFSET(EditE_F));
    from = name;
    for (i = 0; i < 13; i++)
        md.TmpBuf[i] = from[i];
    if (ChgDisk(md.TmpBuf))
        return To(NEXT_FUNC04);
    file = OpenFil(md.TmpBuf, 0x40);
    if (file.failed) {
        if ((u8)file.value != ERR_NO_FILE)
            return To(NEXT_FUNC04);
        MainFil(md.TmpBuf, name);
        if (ChgDisk(md.TmpBuf))
            return Own(edit);
        file = OpenFil(md.TmpBuf, 0x40);
        if (file.failed) {
            if ((u8)file.value <= 3)
                return Own(edit);
            return To(NEXT_FUNC04);
        }
    }
    read = FileRead(file.value, md.MenuBuf, LenMenu - 1);
    count = read.value;
    if (FileClose(file.value).failed)
        return To(NEXT_FUNC04);
    if (read.failed)
        return To(NEXT_FUNC04);
    md.MenuBuf[count] = EOF_CHAR;
    command = FindExt(CutPath(md.ViewFil));
    if (command == 0)
        return Own(edit);
    next = F34_30(command);
    if (next.none)
        return Own(edit);
    named = F34_40(next.at, 1);
    if (named.file == 0) {
        command = named.at;
        *MD_POINTER(command) = ' ';
        if (Run_00(command, 1))
            return F21_30();
        return To(NEXT_FUNC04);
    }
    /* A program: is it there? */
    if (ChgDisk(md.TmpBuf))
        return To(NEXT_FUNC04);
    found = FindFirst(md.TmpBuf, 0x27);
    if (found.failed) {
        if (found.value == ERR_NOT_READY)
            return To(NEXT_FUNC04);
        return Own(edit);
    }
    Aexec0(edit);
    return To(NEXT_INIT50);
}

/* Ask for the name of the file to view or to edit: F44_01. */
static struct Onward F44_01(u16 how)
{
    u16 include = OFFSET(ViFl_S) + 1;

    if ((how & 0xFF) != 0)
        include = OFFSET(EdFl_S) + 1;
    md.HlpPage = (u8)(how + 18);
    if ((u8)Dialog(OFFSET(ViFil_B), include, 0, 0, &md) != 0)
        return To(NEXT_FUNC04);
    if (md.ViewFil[0] == 0)
        return To(NEXT_FUNC04);
    return F34_11(how);
}

/* The editor on the file dirinfo of the directory that an Info panel
   shows, in the lower part of the panel, under the word Edit: F34_10,
   where a click of the mouse in that part comes in as well. */
struct Onward F34_10(struct WCBdef *panel)
{
    const u8 *from = AT(AsmSeg(), OFFSET(DrIn_F));
    u16 begin = BegBX(panel);
    u16 size = SizDX();
    u16 i;

    for (i = 0; i < 13; i++)
        md.ViewFil[i] = from[i];
    LetLin(md.ViewFil);
    begin += 0xB02;
    size -= 0xC04;
    VI->ViBase = begin;
    VI->ViSize = size;
    begin = ADD_LOW(begin - 0x100, 15);
    MvsBw(BasAdr(begin), AT(AsmSeg(), OFFSET(EdFl_S)), 6,
          Color(Slct_C) << 8);
    return To(NEXT_DIRINFO);
}

/* View or edit what the cursor of the panel is on, a file, or for the
   editor the dirinfo of an Info panel: F34_01. With the panels off, in a
   tree, and for the viewer in an Info panel, the name is asked for. */
static struct Onward F34_01(struct WCBdef *panel, u16 how)
{
    const struct FILENT *file;
    u16 i;

    if (panel->Visible == 0)
        return F44_01(how);
    if (panel->WinTyp == 0) {
        if (panel->CurAdr >= panel->EndBuf)
            return To(NEXT_FUNC04);
        file = ENTRY(panel, panel->CurAdr);
        if (file->AttrF & ATTR_DIRECTORY)
            return To(NEXT_FUNC04);
        for (i = 0; i < 13; i++)
            md.ViewFil[i] = file->NameF[i];
        return F34_11(how);
    }
    if ((how & 0xFF) == 0 || panel->WinTyp != 2)
        return F44_01(how);
    return F34_10(panel);
}

struct Onward F43_00(void)              /* Shift-F3: View... */
{
    return F44_01(0);
}

struct Onward F44_00(void)              /* Shift-F4: Edit... */
{
    return F44_01(1);
}

struct Onward F63_00(struct WCBdef *panel) /* Alt-F3 */
{
    return F34_01(panel, 0x100);
}

struct Onward F64_00(struct WCBdef *panel) /* Alt-F4 */
{
    return F34_01(panel, 0x101);
}

struct Onward F33_00(struct WCBdef *panel) /* F3 */
{
    return F34_01(panel, 0);
}

struct Onward F34_00(struct WCBdef *panel) /* F4 */
{
    return F34_01(panel, 1);
}
