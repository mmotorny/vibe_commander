/*
 * The editor: where in the memory its text is, the lines of it, what it
 * shows of it, and how it saves it.
 *
 * Translated from VCEDIT.INC.
 *
 * The text of the file is in the room, from Room of vc.h on, the
 * segment ViewSeg of the original, EdLen bytes of it.
 * A place in it is an offset from its start, 32 bits, and the byte there
 * is Room + place. The assembly has the address of the byte besides, a
 * segment and an offset, which it makes of a place with an offset of
 * less than 16, and what it does by the segment of an address is said
 * where it is done. The line that the cursor is in is
 * edited in a buffer of its own, EditBuf, which holds EdSizLn bytes of it,
 * and is put back into the text when the cursor leaves it.
 *
 * A line ends with a carriage return and a line feed, in either order; in
 * hex mode a line is 16 bytes, and the number of a line there is a
 * sixteenth of its place: that of its segment, counted from the text's.
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
   the assembly, from EdOffst. What has no name here is the viewer's.

   Its last 38 bytes are the first 38 of the viewer's ViewDat, and six
   things there are the two's in common, which stay where they are and as
   wide as they are: EdLen, the viewer's ViLen; EdBase, EdSizeX and
   EdSizeY, its ViBase and ViSize; EdTxHex, its ViTxHex; and Reserved2,
   Reserved3 and Reserved4, which the viewer reads behind the editor and
   the editor does not write. The structure cannot grow either: it ends
   18 bytes before TmpBuf does. So each field that counts or is a place
   in the text is a variable of this file beside it, of 32 bits, of the
   same name, and its bytes here are not written. The flags are
   booleans, a byte as before. */
struct EdData {
    u32 EdOffst;                        /* the variable EdOffst */
    u32 EdEndBf;                        /* the variable EdEndBf */
    u16 EdSizLn;                        /* the variable EdSizLn */
    bool EdUpdat;                       /* Update line buffer */
    bool EdWhole;                       /* The whole line allocates buffer */
    u16 EdCursr;                        /* the variable EdCursr */
    u16 EdCol1;                         /* the variable EdCol1 */
    bool EdUpDwn;                       /* Up & Down flag */
    bool EdHex1;                        /* Cursor digit for Up & Down: the
                                           second, as EdHxCur */
    bool EdScrn;                        /* Output all edit window */
    bool EdCurLn;                       /* It doesn't need to output line */
    u16 EdUndo;                         /* the variable EdUndo */
    u8  Reserved1[77];
    bool EdWrite;                       /* File was written */
    u16 EdSpace;                        /* the variable EdSpace */
    u8  Reserved2;
    bool EdStat;                        /* Status line */
    bool EdHexAs;                       /* Hex / ASCII field in HEX mode */
    bool EdHxCur;                       /* Cursor points 1 or 2 digit in HEX:
                                           true for the second */
    i32 EdLen;                          /* Size of file in edit buffer: the
                                           viewer's ViLen, the length of the
                                           file, which T00_00 holds against
                                           the room as such, and from there
                                           on the length of the text */
    u16 EdCol;                          /* the variable EdCol */
    u16 EdLine;                         /* the variable EdLine */
    u16 Reserved3;
    bool EdTxHex;                       /* Text / Hex mode */
    bool EdChng;                        /* File was changed */
    u32 Reserved4;
    u16 EdCol0;                         /* the variable EdCol0 */
    u16 EdLin0;                         /* the variable EdLin0 */
    u16 Reserved5;
    u16 EdBase;                         /* The base of the edit window */
    u8  EdSizeX;                        /* The size of the edit window: */
    u8  EdSizeY;                        /* its columns and its rows */
    u32 EdFirst;                        /* the variable EdFirst */
    u16 EdShape;                        /* Original cursor shape */
};

#pragma pack(pop)

LAID_OUT(EdData, 138);

/* A line of the text: the place of its first byte and its number,
   counted from 0. DS:SI and DI of the assembly, which has the address of
   the byte where this has its place. */
struct Where {
    i32 place;
    i32 line;
};

/* What T53_10 returns: where a line ends. */
struct Ending {
    i32 at;                             /* the place behind the line; with
                                           none, the place that was
                                           given */
    bool none;                          /* that place is at the end of the
                                           text or behind it */
};

/* What Column, T57_00 and T57_01 return: how far characters of the line
   buffer go, counted up to a column that is the limit. */
struct Reached {
    i32 column;                         /* the column that they end in; if
                                           they reach the limit, the one
                                           that the character begins in
                                           which reaches it */
    i32 left;                           /* how many of them were not
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

/* The fields of EdData that count, as variables. The room for the text,
   in bytes: EdSpace counted paragraphs in the assembly. Places in the
   text: where the line in the buffer begins and where the part of it
   that the buffer holds ends, and where the first line of the window
   begins. In the line buffer: how much of the line it holds, where the
   cursor is in it, and how long the line in the undo buffer is. The
   column of the cursor, and the one that Up and Down keep to; the line
   of the cursor and the first line and column shown. Each is set before
   it is read, when the editor begins (T00_00) or the cursor first comes
   to the screen (T00_20). */
static i32 EdSpace;                     /* Edit buffer size */
static i32 EdOffst;                     /* Offset of begin of the current line */
static i32 EdEndBf;                     /* Offset of end of part of current line */
static i32 EdFirst;                     /* Offset of first line on the screen */
static i32 EdSizLn;                     /* Length of part of line in buffer */
static i32 EdCursr;                     /* Offset of the cursor in line buffer */
static i32 EdUndo;                      /* Length of line in undo buffer */
static i32 EdCol;                       /* Cursor position */
static i32 EdCol1;                      /* Cursor position for Up & Down */
static i32 EdLine;                      /* Cursor line */
static i32 EdCol0;                      /* First column displayed on the screen */
static i32 EdLin0;                      /* First line displayed on the screen */

void T50_00(void);
void T51_00(void);
bool T53_00(struct Where *where);
struct Ending T53_10(i32 place);
bool T54_00(struct Where *where);
i32 T55_00(i32 cursor);
void T56_00(i32 place, i32 line, i32 target);
struct Reached T57_00(void);
struct Reached T57_01(i32 limit);
i32 T57_02(void);
i32 T63_01(i32 place);
bool T64_00(const u8 *name, const u8 *text, i32 count);
void T71_00(u16 function, u16 place, i32 step);
void T72_00(i32 line, const u8 *from, const u8 *end, const u8 *base);
void T73_00(void);
struct Drawn T73_10(void);
void T74_00(void);
i32 T75_00(i32 number, u16 *screen, u16 color);
void T76_00(u16 place, const u8 *text, const u8 *colors);
bool T80_00(void);

/* T00_00, the editor itself, is declared in vc.h. Of the others the
   viewer calls T70_00 and T71_00. */

/* Routines of the rest of the program that this file calls: all are C,
   of vc.h.

   ResWin puts back in the screen buffer what a box covers, Window shows
   it. ChgDisk asks for the disk of a drive that shares the floppy drive
   with another, and is not 0 if the user would not change it. Scroll
   moves a window of the screen by a line, with the mouse pointer out of
   the way.

   HidCur hides the cursor and Cursor puts it at a row, in the high byte,
   and a column; ChgCur gives it the editor's shape and returns the one it
   had, which CurShp gives it back. ClrKbd empties the keyboard buffer, if
   the setup has it so. GetMous returns the buttons of the mouse that are down
   and where it is; what it is given comes back for that if there is no
   mouse. InWind is not 0 if that is outside a window. */

/* C routines of other files that vc.h does not have. */
u16 *HexByt(u16 *screen, u32 byte);                     /* digits.c */
u16 *MvsBw(u16 *screen, const u8 *text, u16 count,
           u16 color);                                  /* screen.c */
void PathLin(u8 *text);                                 /* string.c */
/* The viewer's search: V80_00 shows the box "Searching for" under the
   title `include`, says where it is, and makes ready what is looked for,
   SrchStr; V82_00, of vc.h, looks for it in `count` bytes, and returns
   where the search ended, behind what it found, and 1 for the text
   found, 0 for the bytes that it names in hex, and 100h for neither. */
void V80_00(const u8 *include, struct Box *where);      /* view.c */

/* How long the text is that V80_00 has made ready. */
#define SrchLen (*(u16 *)(md.HelpBuf + 90))

/* Boxes, dialogs and texts of edit_t.c and view_t.c, and
   three of other files: Prot_B, Menu_F and Ext_F. DialBox and Dialog
   take a pointer to theirs: a box and a dialog are declared here as
   what each begins with, as vc.h has it at the two. */
extern const struct BOXTOP SavFi_P;     /* "Saving the file" */
extern const struct BOXHD SavEr_B;      /* "There was an error writing..." */
extern const struct BOXHD Prot_B;       /* "... is a read-only file" */
extern const u8 EdFl_S[];               /* " Edit " */
extern const u8 ViFl_S[];               /* " View " */
extern const struct BOXHD BigFi_B;      /* "... is too large for Edit" */
extern const struct BOXHD NewFi_B;      /* "Can't find the file" */
extern const struct BOXHD SavAs_B;      /* "Save this file as" */
extern const struct BOXHD SavCn_B;      /* "You've made changes ..." */
extern const struct BOXHD Srch_B;       /* "Search for the string" */
extern const struct BOXHD SrEr_B;       /* "Could not find the string" */
extern const u8 Menu_F[];               /* the names of the menu file and */
extern const u8 Ext_F[];                /* of the three extension files */
extern const u8 ViewE_F[];
extern const u8 EditE_F[];
extern const u8 Line_S[];               /* "Line" and "Col" of the status */
extern const u8 Col_S[];                /* line */
extern const u8 EdtH_P[];               /* the colour of the help, */
extern const u8 EdtH_S[];               /* its six lines on the right, */
extern const u8 EdtH1_S[];              /* and for the menu file and for */
extern const u8 EdtH2_S[];              /* an extension file the title
                                           and six lines on the left */

/* The name of the main extension file. */
static const u8 MainExt_F[] = "VC.EXT";

#define Menu_C  7                       /* the colours of the status line, */
#define HlBrd_C 25                      /* of the rows around the help, */
#define Arrow_C 35                      /* and of the arrows at the ends of
                                           a line that goes on */

#define TAB 9
#define CR_LF 0x0A0D                    /* the two that end a line, as a */
#define LF_CR 0x0D0A                    /* word, and the other way round */
#define ENDS_LINE(word) ((word) == CR_LF || (word) == LF_CR)

#define PIECE 0xFF00                    /* bytes read or written at a time */

/* Errors of DOS, the one of a call that the user gave up when a drive
   was not ready (sys.h), and the one that the editor makes of a disk
   that is full. */
#define ERR_NO_FILE   0x02
#define ERR_NO_ACCESS 0x05
#define ERR_NOT_READY 0x53
#define ERR_DISK_FULL 0xFF


/* The column that `count` characters of the line buffer end in, with
   tabs to every eighth, and how many of them were not counted, because
   the column `limit` was reached: none, if it was not. */
static struct Reached Column(i32 count, i32 limit)
{
    struct Reached reached;
    const u8 *text = EditBuf;
    i32 column = 0;
    i32 before;

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
    return Column(EdSizLn, LenEdit - 100);
}

struct Reached T57_01(i32 limit)
{
    return Column(EdSizLn, limit);
}

/* The column of the cursor, counted from 0: in hex mode the number of the
   byte in its line. */
i32 T57_02(void)
{
    if (ed.EdTxHex != 0)
        return EdCursr;
    /* No limit: the assembly's is 0FFFFh, which the line buffer's 3900
       characters at eight columns each do not reach. */
    return Column(EdCursr, 0xFFFF).column;
}

/* Where an offset is in the text: 0 before its end, 1 at the end and 2
   behind it. */
i32 T63_01(i32 place)
{
    if (place != ed.EdLen)
        return place < ed.EdLen ? 0 : 2;
    return 1;
}

/* Fill the line buffer from the text: with the line at EdOffst, without
   the two characters that end it, or with as much of it as the editor
   keeps of a line, 3900 characters. EdEndBf is where in the text the
   buffer ends. Not in hex mode, which has no use for the buffer.

   The buffer gets 3900 bytes of the text whatever the length of the
   line.

   The assembly takes a line that does not end in the segment of its
   address, 64K from there less up to 17 bytes, for one that the buffer
   does not hold, without looking how long it is: which a line of that
   length is. */
void T50_00(void)
{
    struct Ending end;
    const u8 *text;
    i32 length, count, size;

    if (ed.EdTxHex != 0)
        return;
    text = Room + EdOffst;
    for (count = 0; count < LenEdit - 100; count++)
        EditBuf[count] = text[count];
    ed.EdWhole = 1;
    size = 0;
    if (T63_01(EdOffst) == 0) {
        end = T53_10(EdOffst);
        /* From a place before the end of the text to one that is no
           further than its end: a length of the room, which is not
           below 0. */
        length = (i32)(end.at - EdOffst);
        if (length >= 2 && ENDS_LINE(*(const u16 *)(Room + end.at - 2)))
            length -= 2;
        if (length > LenEdit - 100) {
            size = LenEdit - 100;
            ed.EdWhole = 0;
        } else {
            size = length;
        }
    }
    EdEndBf = EdOffst + size;
    EdSizLn = size;
    ed.EdUpdat = 0;
}

/* Copy bytes of the text to another place in it: upwards from the two
   bytes given, or downwards. The assembly does it a piece of 0FF00h
   bytes at a time, each from a segment that much further on or back, a
   byte after the other in the same direction all through. */
static void T51_30(const u8 *source, u8 *target, i32 count, bool down)
{
    if (down) {
        for (; count; count--)
            *target-- = *source--;
    } else {
        for (; count; count--)
            *target++ = *source++;
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
   beyond the memory by itself is cut off.

   The assembly counts the room in paragraphs, EdSpace, and holds the
   segment of where the text would end against it, a sixteenth of the
   place: which is the place held against the room in bytes, the room
   being whole paragraphs. */
void T51_00(void)
{
    i32 old, less, end, tail;
    const u8 *buffer;
    u8 *text;
    i32 size, more, grown, count;

    if (ed.EdTxHex != 0 || ed.EdUpdat == 0)
        return;
    ed.EdUpdat = 0;
    ed.EdChng = 1;
    old = EdEndBf - EdOffst;
    size = EdSizLn;
    if (old > size) {
        less = old - size;
        T51_30(Room + EdEndBf, Room + EdOffst + size,
               ed.EdLen - EdEndBf, false);
        ed.EdLen -= less;
    } else if (old < size) {
        more = size - old;
        grown = more;
        end = ed.EdLen + more;
        if (end >= EdSpace) {
            ed.EdScrn = 1;
            Beep();
            end = EdSpace - 1;
            /* The assembly holds the high word of the end's place
               against that of the line's, and the low words apart:
               a line whose place is in another 64K than the end is
               not cut, though the end be in it. */
            if (end >> 16 == EdOffst >> 16
                && (u16)(end - EdOffst) < EdSizLn)
                EdSizLn = end - EdOffst;
            grown = end - ed.EdLen;
        }
        /* From the last byte to read to the last to write, downwards. */
        tail = end - more;
        if (tail >= EdEndBf)
            T51_30(Room + tail - 1, Room + end - 1, tail - EdEndBf, true);
        ed.EdLen += grown;
    }
    text = Room + EdOffst;
    buffer = EditBuf;
    for (count = EdSizLn; count; count--)
        *text++ = *buffer++;
}

/* What T53_10 returns: a line that ends at a place, or none, with the
   place that T53_10 was given. */
static struct Ending EndsAt(i32 place)
{
    struct Ending ending;

    ending.at = place;
    ending.none = 0;
    return ending;
}

static struct Ending NoEnd(i32 start)
{
    struct Ending ending;

    ending.at = start;
    ending.none = 1;
    return ending;
}

/* Where the line ends that the byte at a place is in: behind the two
   characters that end it, or where the text ends; in hex mode behind its
   16 bytes, or where the text ends. There is no such place, and the
   place that was given comes back, if it is at the end of the text or
   behind it.

   The assembly is given an address and looks no further than its
   segment goes, 0FFFEh, and has no end either for a line that goes on
   beyond that. Whoever asked it then asked again, from the address with
   the lowest offset and from there a piece of 0FF00h bytes further on
   every time, each time for a line that begins there: which finds the
   same two characters, the first from the place on, or the end of the
   text. What the two did otherwise, for a line of more than 64K less 17
   bytes, is said at T50_00 and at T00_20. */
struct Ending T53_10(i32 place)
{
    i32 start = place;
    u16 word;

    if (T63_01(place))
        return NoEnd(start);
    if (ed.EdTxHex != 0) {
        place = (place | 0x0F) + 1;
        if (T63_01(place))
            place = ed.EdLen;
        return EndsAt(place);
    }
    /* As far as the text goes. The last word that is looked at has its
       second byte behind the text, and ends no line. */
    while (place < ed.EdLen) {
        word = *(const u16 *)(Room + place);
        place += 2;
        if (place <= ed.EdLen && ENDS_LINE(word))
            return EndsAt(place);
        place--;
    }
    return EndsAt(place);
}

/* Go to the next line of the text. Returns false, with `where` on it,
   or true and `where` as it was if there is none: at the end of the
   text, and after a line that the text ends in without the two
   characters. In hex mode the next line is the next 16 bytes, if the
   text has that many.

   The assembly counts the lines in 16 bits and has no next line from
   the line 0FFFFh on, in text and in hex: the 65,536th line is the
   last that it reaches. Here a line is counted in 32 bits and every
   line of the text is reached (src/portable_c/DEVIATIONS.md, 25). */
bool T53_00(struct Where *where)
{
    i32 place = where->place;
    struct Ending end;

    if (T63_01(place))
        return true;
    end = T53_10(place);
    if (ed.EdTxHex != 0) {
        if (end.at & 0x0F)
            return true;
    } else {
        if (end.at - place <= 1)
            return true;
        if (!ENDS_LINE(*(const u16 *)(Room + end.at - 2)))
            return true;
    }
    where->place = end.at;
    where->line++;
    return false;
}

/* Go to the line before. Returns false, with `where` on it, or true and
   `where` as it was if there is none: in the first line. In hex mode the line
   before is 16 bytes back, and there is none for a line in the first 16
   bytes whatever its number.

   As text, the bytes before the line are looked through backwards for
   the two characters that end a line before the two that end the line
   before this one. Lines of nothing but those two may follow one another
   there, and are passed over as long as they do; so from where that
   stopped the lines are taken forwards again until the next one would be
   this line. The assembly looks through at most 32K at a time, and then
   takes a segment that is lower. */
bool T54_00(struct Where *where)
{
    struct Where line;
    i32 place = where->place;
    i32 from, next;
    i32 at;
    bool found = false;
    u16 pair, word;

    if (where->line == 0)
        return true;
    if (ed.EdTxHex != 0) {
        if (place < 16)
            return true;
        where->place = place - 16;
        where->line--;
        return false;
    }
    if (place == 0)
        return true;
    /* Two bytes behind the word that is looked at: with a sign, since
       from the second byte of the text that is the byte before the
       text. */
    at = place - 2;
    for (;;) {
        if (at < 2) {
            /* The text has no more. */
            at = 0;
            break;
        }
        word = *(const u16 *)(Room + at - 2);
        if (!found) {
            if (ENDS_LINE(word)) {
                found = true;
                pair = word;
            }
        } else {
            pair = (u16)(pair << 8 | pair >> 8);
            if (word != pair) {
                at++;
                break;
            }
        }
        at--;
    }
    from = at;
    for (;;) {
        line.place = from;
        line.line = 0;
        T53_00(&line);
        next = line.place;
        if (next >= place)
            break;
        from = next;
    }
    where->place = from;
    where->line--;
    return false;
}

/* Whether the line buffer has a character at `cursor`. If it has not,
   and does not hold the whole line, it is first put back into the text
   and filled again. */
static bool T55_10(i32 cursor)
{
    if (cursor >= EdSizLn && ed.EdWhole == 0) {
        T51_00();
        T50_00();
    }
    return cursor < EdSizLn;
}

/* Where the next word is, from the character `cursor` of the line buffer
   on: behind the characters that stand between words, if it begins with
   such, and behind those that do not, up to the next one that does or to
   the end of what the buffer has. The first character itself is not
   looked at. */
i32 T55_00(i32 cursor)
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
void T56_00(i32 place, i32 line, i32 target)
{
    struct Where where;
    i32 cursor, last;

    where.place = place;
    where.line = line;
    for (;;) {
        EdOffst = place;
        EdLine = where.line;
        if (T53_00(&where))
            break;
        place = where.place;
        if (place > target)
            break;
    }
    T50_00();
    /* The assembly takes 0FFFFh for a byte 64K or more into the line,
       which is beyond the last character too. */
    cursor = target - EdOffst;
    last = EdSizLn - T57_00().left;
    if (cursor > last)
        cursor = last;
    EdCursr = cursor;
}

/* Whether `count` bytes at `name` and at `text` are the same. */
bool T64_00(const u8 *name, const u8 *text, i32 count)
{
    for (; count; count--)
        if (*name++ != *text++)
            return false;
    return true;
}

/* Clear a line of the window in the screen buffer; only the low byte of
   `line` counts. Returns where the line is in the buffer, and its row
   and column on the screen. */
struct Cleared T70_00(i32 line)
{
    struct Cleared cleared;
    /* Of the row the low byte alone counts: the rest is shifted out of
       the place's 16 bits. */
    u16 place = (u16)(ed.EdBase + (line << 8));
    u16 *screen = BasAdr(place);
    u16 *cell = screen;
    u16 blank = (u16)(Color(Back_C) << 8 | ' ');
    i32 count;

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
void T71_00(u16 function, u16 place, i32 step)
{
    u16 *to = BasAdr(place);
    /* So many bytes on or back, two to a cell: the assembly adds the
       step to an offset, as a number of 16 bits, FF60h for 160 back. */
    i32 stride = step / 2;
    u16 *from = to + stride;
    /* The lines below `line` but the last: `line` is never below the
       last, so this is not below 0. */
    i32 count = (u8)(ed.EdSizeY - function) - 1;
    i32 cells, column;

    if (count == 0)
        return;
    do {
        /* As many cells as the window has columns; the high byte of
           the count of lines is on top of that, which is 0. */
        cells = count & 0xFF00 | ed.EdSizeX;
        for (column = 0; column != cells; column++)
            to[column] = from[column];
        from += stride;
        to += stride;
    } while (--count);
    Scroll(function >> 8 == 7, Color(Back_C),
           (u16)(ed.EdBase + ((function & 0xFF) << 8)),
           *(u16 *)&ed.EdSizeX + ed.EdBase - 0x101);
}

/* Write a line of the text to the line `line` of the window in the
   screen buffer, of which the low byte counts: the bytes from `from` up
   to `end`.

   As text: without the two characters that end a line, if it has them, a
   carriage return and a line feed in either order; with tabs to every
   eighth column; from the column EdCol0 on, and as much as the window is
   wide. Where it goes on, the last column has an arrow, and the first one
   has, if columns are left out there.

   In hex: the place of the first byte, eight digits, and then at most 16
   bytes, as two digits each, in fours, and once more as characters, a
   zero as a dot.

   Ported as it is: the arrow in the first column is drawn for a line of
   no bytes too, which T73_10 does not hand over at all. And the two
   characters are looked for in the two bytes before `end` whether the
   line has two or not: the assembly has `from` and `end` as offsets in a
   segment, and looks there wherever `end` is 2 or more. `base` is where
   that segment begins. So of a line of one byte that makes the two
   characters with the byte before it, which a text can end in, nothing
   is written, but where it stands less than two bytes into its
   segment; and the two bytes before a line that is in the buffer are
   always looked at. */
void T72_00(i32 line, const u8 *from, const u8 *end, const u8 *base)
{
    u16 *screen = T70_00(line).screen;
    u16 *cell;
    u8 *ascii;
    i32 offset, column, place, count;
    u16 c;

    if (ed.EdTxHex == 0) {
        column = 0;
        if (end - base >= 2) {
            c = *(const u16 *)(end - 2);
            if (c == CR_LF || c == LF_CR)
                end -= 2;
        }
        while (from < end) {
            c = *from++;
            if (c == TAB) {
                column |= 7;
                c = ' ';
            }
            place = column++;
            if (place < EdCol0)
                continue;
            place -= EdCol0;
            /* Nothing is left out in front of the window or more than
               seven columns of a tab: the window's width is passed here
               by less than 8, below 256, where the assembly holds the
               low byte against it. */
            if (place >= ed.EdSizeX) {
                screen[(u16)(ed.EdSizeX - 1)] = (u16)(Color(Arrow_C) << 8 | 0x1A);
                break;
            }
            *(u8 *)(screen + place) = (u8)c;
        }
        if (EdCol0 != 0)
            *screen = (u16)(Color(Arrow_C) << 8 | 0x1B);
        return;
    }
    /* In hex the line is of the text, and no place in it is below 0. */
    offset = from - Room;
    cell = screen + 1;
    cell = HexByt(cell, (u16)(offset >> 24));
    cell = HexByt(cell, (u16)(offset >> 16));
    cell = HexByt(cell, (u16)offset >> 8);
    cell = HexByt(cell, (u16)offset);
    cell += 2;
    ascii = (u8 *)(cell + 52);
    for (count = 16; count; count--) {
        if (from >= end)
            break;
        c = *from++;
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
    i32 line = EdLine - EdLin0;         /* a row of the window */
    u16 place = (u16)(ed.EdBase + (line << 8));
    const u8 *from;
    const u8 *end;

    if (ed.EdTxHex == 0) {
        if ((u16)(EdSizLn - EdCursr)
                < (u16)((signed char)ed.EdSizeX + EdCol0 - EdCol)
            && ed.EdWhole == 0) {
            T51_00();
            T50_00();
        }
        from = EditBuf;
        end = from + EdSizLn;
        T70_00(line);
        /* The segment of the buffer is the main data in the assembly,
           where the buffer is far from the start. */
        if (from < end)
            T72_00(line, from, end, (const u8 *)&md);
    } else {
        from = Room + EdOffst;
        end = Room + T53_10(EdOffst).at;
        T72_00(line, from, end, Room);
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
    u16 blank;
    i32 count;

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
   five cells of the screen buffer, or in as many as it has digits where
   that is more, in the colour in the high byte of `color`. Returns how
   many cells it has written.

   The assembly counts in 16 bits, shows the 65,536th line as 0, and
   writes five cells whatever the number. Here the number is of 32 bits
   and a line of more than five digits has its cells, T00_20 moving the
   column behind it (src/portable_c/DEVIATIONS.md, 25). A column has
   five digits at the most: 3900 characters of tabs. */
i32 T75_00(i32 number, u16 *screen, u16 color)
{
    u8 *end = Number(md.TmpBuf, (u16)((number + 1) >> 16),
                     (u16)(number + 1), 0);
    i32 width = (i32)(end - md.TmpBuf);
    i32 count;

    for (count = 5; count; count--)
        *end++ = ' ';
    if (width < 5)
        width = 5;
    MvsBw(screen, md.TmpBuf, (u16)width, color);
    return width;
}

/* Write six lines of the help that the editor shows for a menu file or
   an extension file, one below the other from `place` on: texts one
   behind the other at `text`, in the colours `colors`. */
void T76_00(u16 place, const u8 *text, const u8 *colors)
{
    struct LineArgs line;
    i32 count;

    line.value = 0xFF;                  /* no limit to the width */
    line.line = text;
    line.colors = colors;
    line.include = text;                /* none of the lines has ^I */
    for (count = 6; count; count--) {
        line.screen = BasAdr(place);
        NgLine(&line);
        place += 0x100;
    }
}

/* What T80_01, T80_02 and T80_03 return, as struct Answer of vc.h has
   it: what was asked for, or an error. */
static struct Answer Done(u16 value)
{
    struct Answer answer;

    answer.value = value;
    answer.failed = false;
    return answer;
}

static struct Answer Failed(u16 error)
{
    struct Answer answer;

    answer.value = error;
    answer.failed = true;
    return answer;
}

/* Ask DOS for the attributes of the file. Returns them, with Archive
   set; for a file that is not there, Archive alone. Anything else that
   DOS says is an error, and that is returned. */
static struct Answer T80_01(void)
{
    struct FileAttr asked = FileAttributes(md.ViewFil);

    if (asked.failed && asked.value != ERR_NO_FILE)
        return Failed(asked.value);
    /* A file that is not there has none: 0, which FileAttributes answers
       with where it failed. */
    return Done(asked.attributes & 0x3F | 0x20);
}

/* The name of the file, as the box about a read-only file shows it. */
static void T80_Name(void)
{
    u8 *name = md.TmpBuf + 130;
    i32 count;

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

   The file is made with the attributes less Read-only: those that it
   has just been given, or its own, which have none then. */
static struct Answer T80_02(u16 attributes)
{
    struct Answer answer;
    struct Moved wrote;
    struct Device device;
    const u8 *piece;
    i32 left, count;
    u16 handle, error;
    bool failed;

    if (attributes & 1) {
        answer = FileSetAttributes(md.ViewFil, attributes & 0xFFFE);
        if (answer.failed)
            return Failed(answer.value);
        ed.EdWrite = 1;
    }
    answer = FileCreate(md.ViewFil, attributes & 0xFFFE);
    if (answer.failed)
        return Failed(answer.value);
    ed.EdWrite = 1;
    handle = answer.value;
    device = FileDevice(handle);
    failed = device.failed;
    error = device.value;
    if (!failed) {
        if (device.kind & 0x80) {
            if (device.kind & 2)
                goto close;
            attributes = 0xFFFF;
        }
        left = ed.EdLen;
        piece = Room;
        do {
            count = PIECE;
            if (left < count)
                count = left;
            wrote = FileWrite(handle, piece, count);
            failed = wrote.failed;
            error = wrote.error;
            if (failed)
                goto close;
            if (wrote.count != count) {
                failed = wrote.count < count;
                error = ERR_DISK_FULL;
                goto close;
            }
            piece += PIECE;
            left -= count;
        } while (left != 0);
    }
close:
    answer = FileClose(handle);
    if (answer.failed)
        return Failed(answer.value);
    if (failed)
        return Failed(error);
    if (attributes == 0xFFFF)
        return Done(0);
    answer = FileSetAttributes(md.ViewFil, attributes);
    if (answer.failed && answer.value != ERR_NO_ACCESS)
        return Failed(answer.value);
    return Done(0);
}

/* Delete the file. Returns 0, or the error. */
static struct Answer T80_03(void)
{
    struct Answer deleted = FileDelete(md.ViewFil);

    if (deleted.failed)
        return Failed(deleted.value);
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
    const u8 *name = md.ViewFil;
    const u8 *file;
    const u8 *text;
    u8 *to;
    u16 letter, c;
    i32 count, i;

    letter = *(const u16 *)name;
    if (letter >> 8 == ':')
        name += 2;
    else
        letter = letter & 0xFF00 | md.DosPth[0];
    letter = UpCase(letter);
    md.TmpBuf[0] = (u8)letter;
    md.TmpBuf[1] = ':';
    md.TmpBuf[2] = '\\';
    if (DirCurrent(letter - ('A' - 1), md.TmpBuf + 3).failed)
        return;
    to = md.TmpBuf + 2;
    if (*name != '\\') {
        count = LenPath + 13;
        do
            c = *to++;
        while (--count && c != 0);
        to = AddFile(to - 1);
    }
    if (md.TmpBuf + LenPath + 13 <= to)
        return;
    for (count = md.TmpBuf + LenPath + 13 - to; count; count--)
        *to++ = *name++;
    PathLin(md.TmpBuf);
    file = CutPath(md.TmpBuf);
    count = file - md.TmpBuf;
    for (i = 0; i < count; i++)
        if (md.TmpBuf[i] != md.NcPath[i])
            return;
    if (md.NcPath[count] != 0)
        return;
    for (i = 0; i < (i32)sizeof MainExt_F; i++)
        if (file[i] != MainExt_F[i])
            return;
    count = LenExt - 1;
    if (count > ed.EdLen)
        count = ed.EdLen;
    text = Room;
    for (i = 0; i < count; i++)
        md.ExtMain[i] = text[i];
    md.ExtMain[count] = 0x1A;
}

/* Save the text as the file ViewFil. Returns false if it is saved.

   With a box on the screen that says so: the line buffer goes back into
   the text, the user is asked whether to overwrite a file that is
   read-only, and the file is written. If that fails, another box says
   so, unless it is the drive that is not ready, which has had its own;
   and of a disk that is full the file is deleted again, with the box
   only if that goes well.

   Ported as it is: what follows the saving of the file, T80_14, is done
   whether it was saved or not. */
bool T80_00(void)
{
    struct Box box;
    struct Answer answer;
    bool failed = true;

    T51_00();
    T50_00();
    /* None of its lines has ^I: no include line. */
    DialBox(0x528, &SavFi_P, 0, &md, &box);
    Window(box.begin, box.size);
    if (ChgDisk(md.ViewFil))
        goto done;
    answer = T80_01();
    if (!answer.failed) {
        if (answer.value & 1) {
            T80_Name();
            if ((u8)Dialog(&Prot_B, EdFl_S, 0, 0, &md) != 0)
                goto done;
        }
        answer = T80_02(answer.value);
        if (!answer.failed) {
            failed = false;
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
    Dialog(&SavEr_B, 0, 0, 0, &md);
done:
    ResWin(box.begin, box.size, box.saved);
    Window(box.begin, box.size);
    T80_14();
    return failed;
}

/* The line that the cursor is in, as T53_00 and T54_00 take one. T60_00
   and T61_00 of the assembly, which make an offset of an address and an
   address of an offset, and T62_00, the address of the first line on the
   screen, are nothing here: a line is known by its place. */
static void T60_10(struct Where *where)
{
    where->place = EdOffst;
    where->line = EdLine;
}

/* Make the line `where` the one that the cursor is in. */
static void T60_20(const struct Where *where)
{
    EdLine = where->line;
    EdOffst = where->place;
}

/* Put the cursor in the column `limit` of the line, counted from 1, or
   at the end of what the buffer has of the line if that is shorter. The
   buffer is filled first. */
static void T14_12(i32 limit)
{
    T50_00();
    EdCursr = EdSizLn - T57_01(limit).left;
}

/* End, where the line is one that the cursor has just come to: as text,
   to the last column that the editor goes to or to the end of the line;
   in hex, to the last byte of the 16, or behind the last one there is. */
static void T14_11(void)
{
    struct Ending end;
    i32 cursor;

    if (ed.EdTxHex == 0) {
        T14_12(LenEdit - 100);
        return;
    }
    ed.EdHxCur = 0;                     /* T14_00 */
    end = T53_10(EdOffst);
    /* With no end, the assembly has the offset of the line's address
       here, which is less than 16. */
    cursor = end.at & 0x0F;
    if (!end.none && cursor == 0) {
        cursor = 0x0F;
        ed.EdHxCur++;
    }
    EdCursr = cursor;
}

/* End. As text the buffer is put back into the text first, and filled
   again, if it holds a part of the line that ends before the last column
   the editor goes to. */
static void T14_00(void)
{
    i32 left;

    if (ed.EdTxHex != 0) {
        T14_11();
        return;
    }
    left = T57_00().left;
    if (left == 0 && EdSizLn != 0 && ed.EdWhole == 0) {
        T51_00();
        T14_11();
        return;
    }
    EdCursr = EdSizLn - left;
}

/* Keep the cursor from the columns behind the last one the editor goes
   to. */
static void T08_01(void)
{
    i32 last = EdSizLn - T57_00().left;

    if (last < EdCursr)
        EdCursr = last;
}

/* Go to the start of the next line, if there is one. */
static void T06_14(void)
{
    struct Where where;

    T51_00();
    T60_10(&where);
    if (T53_00(&where) == 0) {
        T60_20(&where);
        EdCursr = 0;
    }
    T50_00();
}

/* Right, as text: `mouse` is not 0 for a click in the last column of the
   window, which does not go on to the next line. */
static void T06_10(bool mouse)
{
    if (EdCursr >= EdSizLn) {
        if (ed.EdWhole != 0) {
            if (!mouse)                 /* T06_13 */
                T06_14();
            return;
        }
        T51_00();
        T50_00();
    }
    if (T57_02() < LenEdit - 100 - 1)   /* T06_11 */
        EdCursr++;
}

/* Right. In hex from the first digit of a byte to the second, where the
   cursor is among the digits, else to the next byte, or the next line;
   but not from behind the last byte. */
static void T06_00(void)
{
    i32 place, cursor;

    if (ed.EdTxHex == 0) {
        T06_10(false);
        return;
    }
    cursor = EdCursr;
    place = EdOffst + cursor;
    if (T63_01(place) != 0)
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
    EdCursr = cursor;
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
    i32 count;

    if (EdSizLn >= LenEdit) {
        T51_00();
        T50_00();
    }
    EdSizLn++;                       /* T01_04 */
    to = EditBuf + LenEdit - 1;
    for (count = LenEdit - 1 - EdCursr; count; count--) {
        *to = to[-1];
        to--;
    }
    EditBuf[EdCursr] = (u8)key;      /* T01_05 */
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
    i32 place = EdOffst + EdCursr;
    u8 *byte = Room + place;
    u8 value = (u8)key;
    i32 digit;

    if (ed.EdHexAs == 0) {
        digit = TstHex(key);
        if (digit > 0xFF)
            return;
        if (ed.EdHxCur == 0)
            value = (u8)(digit << 4 | *byte & 0x0F);
        else
            value = (u8)(digit | *byte & 0xF0);
    }
    *byte = value;                      /* T01_12 */
    if (T63_01(place) != 0) {
        if (ed.EdHexAs == 0)
            *byte &= 0xF0;
        /* The last byte of the room: the assembly asks for an offset
           of 0Fh in its last paragraph. */
        if (place + 1 >= EdSpace) {     /* T01_13 */
            Beep();
        } else {
            ed.EdLen++;                 /* T01_14 */
            if ((ed.EdLen & 0x0F) == 0
                && (u8)(EdLine - EdLin0 + 1) < ed.EdSizeY) {
                /* The place of the line below, as the assembly makes it:
                   it steps the low word of EdOffst alone, so the place
                   of a line within 16 bytes of a multiple of 64K wraps
                   64K back, and the line drawn is that one. */
                EdLine++;
                EdOffst = EdOffst & ~0xFFFF | (u16)(EdOffst + 0x10);
                T73_00();
                EdOffst = EdOffst & ~0xFFFF | (u16)(EdOffst - 0x10);
                EdLine--;
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
    i32 row;

    T50_00();
    T73_00();
    row = EdLine - EdLin0 + 2;
    if ((u8)row < ed.EdSizeY)
        T71_00(0x700 | (u8)(row - 1),
               (u16)(ed.EdBase + ((u16)(u8)(ed.EdSizeY - 1) << 8)), -160);
    T06_14();
}

/* Enter: the two characters that end a line, at the cursor. A buffer
   that has no room for them is put back into the text first and filled
   again. */
static void T02_00(void)
{
    u8 *to;
    i32 count;

    if (EdSizLn >= LenEdit - 1) {
        T51_00();
        T50_00();
    }
    EdSizLn += 2;                    /* T02_01 */
    to = EditBuf + LenEdit - 1;
    for (count = LenEdit - 2 - EdCursr; count; count--) {
        *to = to[-2];
        to--;
    }
    *(u16 *)(EditBuf + EdCursr) = CR_LF;     /* T02_02 */
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

    T60_10(&where);
    if (T54_00(&where) != 0)
        return;
    T51_00();
    T60_20(&where);
    T14_11();
}

/* Left in hex, by a byte: `digit` is the digit of that byte to be on,
   where the cursor is among the digits. */
static void T05_02(bool digit)
{
    if (EdCursr != 0) {
        ed.EdHxCur = digit;
        EdCursr--;                   /* T05_03 */
        return;
    }
    if (EdOffst == 0)                /* T05_05 */
        return;
    ed.EdHxCur = 1;
    T05_11();
}

/* Left. In hex from the second digit of a byte to the first, where the
   cursor is among the digits, else to the byte before. */
static void T05_00(void)
{
    if (ed.EdTxHex == 0) {
        if (EdCursr != 0)            /* T05_10 */
            EdCursr--;
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
static void T10_02(bool down, i32 count)
{
    struct Where where;
    struct Ending end;
    bool failed;
    i32 cursor;

    T51_00();
    T60_10(&where);
    if (count != 0) {
        do {                            /* T10_03 */
            if (!down)
                failed = T54_00(&where);
            else
                failed = T53_00(&where);
            if (failed)
                break;
        } while (--count);
    }
    T60_20(&where);                             /* T10_06 */
    ed.EdUpDwn = 1;
    if (ed.EdTxHex == 0) {
        T14_12(EdCol1 + 1);
        return;
    }
    EdCursr = EdCol1;             /* T10_10 */
    ed.EdHxCur = ed.EdHex1;
    end = T53_10(EdOffst);
    if (!end.none && (end.at & 0x0F) == 0)
        return;
    cursor = end.at & 0x0F;            /* T10_11 */
    if (cursor > EdCursr)
        return;
    EdCursr = cursor;
    ed.EdHxCur = 0;
}

/* PgUp, if `down` is 0, or PgDn: to the first or the last line of the
   window, or from there by a line less than the window has; as far as
   there are lines. */
static void T12_01(bool down)
{
    struct Where where;
    i32 count, row;
    bool failed;

    T51_00();
    T60_10(&where);
    count = ed.EdSizeY - 1;
    row = EdLine - EdLin0;
    if (!down) {
        if (row != 0)
            count = row;
    } else if (count > row) {           /* T12_02 */
        count -= row;
    }
    do {                                /* T12_03 */
        if (!down)
            failed = T54_00(&where);
        else
            failed = T53_00(&where);
        if (failed)
            break;
    } while (--count);
    if (where.line != EdLine) {      /* T12_06 */
        T60_20(&where);
        EdCursr = 0;
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
        T60_10(&where);
        while (T53_00(&where) == 0)
            ;
        T60_20(&where);
    } else {
        EdLine = ed.EdLen >> 4;       /* T16_10 */
        EdOffst = ed.EdLen & ~0x0F;
    }
    T14_11();                           /* T16_11 */
}

/* Take the characters of the buffer from `from` up to `end` out of the
   line, and keep them to be put back. */
static void T20_02(u8 *from, u8 *end)
{
    u8 *undo = UndoBuf;
    i32 count = end - from;
    i32 cut = count;
    const u8 *source = from;

    EdUndo = count;
    for (; count; count--)
        *undo++ = *source++;
    if (EditBuf + EdSizLn > end)
        for (count = EditBuf + EdSizLn - end; count; count--)
            *from++ = *end++;
    EdSizLn -= cut;                  /* T20_03 */
    ed.EdUpdat = 1;
    ed.EdChng = 1;
}

/* Put the buffer into the text in place of the line as far as the offset
   `end`, and fill it again: a line gone, or joined to the one before.
   With `drawn`, the screen follows: the line is drawn, those below it move
   up by one, and the line that comes into the last row of the window is
   drawn there. Where the text has no line for that row, the row is
   cleared. */
static void T20_06(i32 end, bool drawn)
{
    struct Where where;
    i32 place, offset, line, row, count;

    EdEndBf = end;
    ed.EdUpdat = 1;
    T51_00();
    T50_00();
    if (!drawn)
        return;
    T73_00();
    line = EdLine;
    row = line - EdLin0;
    if ((u8)(row + 1) < ed.EdSizeY)
        T71_00(0x600 | (u8)(row + 1),
               (u16)(ed.EdBase + ((u16)(u8)(row + 1) << 8)), 160);
    count = ed.EdSizeY - 1;             /* T20_07 */
    if (count <= row)
        return;
    count -= row;
    T60_10(&where);
    where.line = line;
    place = ed.EdLen;
    for (;;) {                          /* T20_08 */
        if (T53_00(&where) != 0)
            break;
        if (--count == 0) {
            place = where.place;
            break;
        }
    }
    offset = EdOffst;                /* T20_09 */
    EdOffst = place;
    EdLine = where.line + count;
    T50_00();
    T73_00();
    EdOffst = offset;
    EdLine = line;
    T50_00();
}

/* Join the next line to the one in the buffer, if there is a next line:
   Del at the end of a line. The two characters between them are what is
   kept to be put back. */
static void T20_05(bool drawn)
{
    struct Where where;

    T60_10(&where);
    /* The assembly has in DI, for the number of the line, whatever was
       left there, and its T53_00 refuses the next line where that is
       0FFFFh, which nothing leaves. Nothing reads the number here. */
    where.line = 0;
    if (T53_00(&where) != 0)
        return;
    EdUndo = 2;
    *(u16 *)UndoBuf = CR_LF;
    T20_06(where.place, drawn);
}

/* Del: the character under the cursor, or at the end of the line the
   two that end it. */
static void T20_00(void)
{
    u8 *at;

    if (ed.EdTxHex != 0)
        return;
    if (EdCursr >= EdSizLn) {
        T20_05(true);
        return;
    }
    at = EditBuf + EdCursr;          /* T20_01 */
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
    i32 place, cursor, row;
    u8 *at;

    if (ed.EdTxHex != 0) {
        place = EdOffst + EdCursr;
        if (place == 0)
            return;
        if (T63_01(place) == 0) {
            T05_00();                   /* T21_02 */
            return;
        }
        ed.EdChng = 1;
        ed.EdLen--;
        if ((place & 0x0F) == 0)
            Window(T70_00(EdLine - EdLin0).place,
                   0x100 | ed.EdSizeX);
        T05_02(false);                  /* T21_01 */
        return;
    }
    cursor = EdCursr;                /* T21_10 */
    if (cursor != 0) {
        cursor--;
        EdCursr = cursor;
        at = EditBuf + cursor;
        T20_02(at, at + 1);
        return;
    }
    T60_10(&where);                             /* T21_11 */
    if (T54_00(&where) != 0)
        return;
    T51_00();
    T60_20(&where);
    T50_00();
    EdCursr = EdSizLn - T57_00().left;
    row = EdLine + 1 - EdLin0;
    if (row == 0) {
        EdLin0 = EdLine;
        EdFirst = EdOffst;
    }
    T20_05(row != 0);                   /* T21_12 */
}

/* Ctrl-K: cut the line off at the cursor. Of a line that the buffer
   holds a part of, the rest goes too, and is not kept to be put back. */
static void T24_00(void)
{
    struct Where where;
    i32 end;

    if (ed.EdTxHex != 0)
        return;
    if (ed.EdWhole == 0) {
        T51_00();
        T50_00();
        T60_10(&where);
        where.line = 0;                 /* see T20_05 */
        end = ed.EdLen;
        if (T53_00(&where) == 0)
            end = where.place - 2;
        EdEndBf = end;               /* T24_01 */
        ed.EdWhole = 1;
    }
    if (EdCursr >= EdSizLn)       /* T24_02 */
        return;
    T20_02(EditBuf + EdCursr, EditBuf + EdSizLn);
}

/* Ctrl-Y: cut the line. What the buffer has of it is kept to be put
   back, as a line. */
static void T25_00(void)
{
    struct Where where;
    i32 end, count, size;

    if (ed.EdTxHex != 0)
        return;
    if (ed.EdWhole == 0) {
        T51_00();
        T50_00();
    }
    size = EdSizLn;                  /* T25_01 */
    for (count = 0; count < size; count++)
        UndoBuf[count] = EditBuf[count];
    EdUndo = size + 2;
    *(u16 *)(UndoBuf + size) = CR_LF;
    T60_10(&where);
    where.line = 0;                     /* see T20_05 */
    end = ed.EdLen;
    if (T53_00(&where) == 0)
        end = where.place;
    EdSizLn = 0;                     /* T25_02 */
    EdCursr = 0;
    T20_06(end, true);
}

/* Ctrl-U: put back at the cursor what was last taken out. What is no
   line and fits the buffer goes into the buffer. A line, and what does
   not fit, goes into the text: the buffer is put back first, and what is
   put in is written through the buffer as if it were a line of its own
   at the cursor. */
static void T26_00(void)
{
    i32 offset;
    i32 count = EdUndo;
    bool line = false;
    i32 i;
    i32 left;
    u8 *from, *to;

    if (ed.EdTxHex != 0 || count == 0)
        return;
    if (count >= 2 && ENDS_LINE(*(const u16 *)(UndoBuf + count - 2)))
        line = true;
    else if (EdSizLn < LenEdit + 1 - count) {
        EdSizLn += count;            /* T26_01 */
        to = EditBuf + LenEdit - 1;
        from = to - count;
        /* The bytes from the cursor to `from`, both of them counted:
           none, if the cursor is behind that. */
        left = (i32)(from - (EditBuf + EdCursr - 1));
        if (left > 0)
            for (; left; left--)
                *to-- = *from--;
        EdCursr += count;            /* T26_02 */
        to = from + 1;
        for (i = 0; i < count; i++)
            to[i] = UndoBuf[i];
        ed.EdUpdat = 1;
        T08_01();
        return;
    }
    T51_00();                           /* T26_04 */
    offset = EdOffst;
    EdOffst = offset + EdCursr;
    EdEndBf = EdOffst;
    for (i = 0; i < count; i++)
        EditBuf[i] = UndoBuf[i];
    EdSizLn = count;
    ed.EdUpdat = 1;
    T51_00();
    EdOffst = offset;
    if (line) {
        T02_03();
        return;
    }
    T50_00();                           /* T26_05 */
    EdCursr += count;
    T08_01();
}

/* F4: from text to hex and back, with the cursor on the same byte. In
   hex the window begins with the line of the cursor. Not in a window
   narrower than the screen. */
static void T30_00(void)
{
    i32 place;

    if (ed.EdSizeX < 80)
        return;
    T51_00();
    EdCol0 = 0;
    ed.EdScrn = 1;
    ed.EdTxHex = ed.EdTxHex == 0;
    if (ed.EdTxHex != 0) {
        ed.EdHxCur = 0;
        place = EdOffst + EdCursr;
        EdCursr = (u16)(place & 0x0F);
        EdLine = place >> 4;
        EdLin0 = EdLine;
        EdOffst = place & ~0x0F;
    } else {
        place = EdOffst + EdCursr;        /* T30_10 */
        T56_00(0, 0, place);
        EdLin0 = EdLine;
    }
    EdFirst = EdOffst;            /* T30_11 */
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
    i32 place, left, base, count;
    struct Sought sought;

    if (md.SrchStr[0] == 0)
        return;
    T51_00();
    V80_00(EdFl_S, &box);
    place = EdOffst + EdCursr;
    for (;;) {                          /* T32_01 */
        if (TstBrk())
            goto done;
        if (ed.EdLen < place)
            goto none;
        left = ed.EdLen - place;
        count = 0x8000;
        if (left < count)
            count = left;
        /* T32_02. The viewer's search takes the paragraph of the place
           and an offset from there, and answers with one: so the
           assembly had them, a segment and an offset of less than 16. */
        base = place & ~0x0F;
        sought = V82_00(Room + base, place - base, count);
        if (sought.found < 0x100)
            break;
        if (count < SrchLen)
            goto none;
        place += count - SrchLen + 1;
    }
    if (TstBrk())                       /* T32_05 */
        goto done;
    place = base + sought.end;
    if (ed.EdTxHex != 0) {
        ed.EdHexAs = sought.found != 0;
        ed.EdHxCur = 0;
        EdOffst = place & ~0x0F;
        EdLine = place >> 4;
        EdCursr = place & 0x0F;
    } else {
        T56_00(EdOffst, EdLine, place);               /* T32_06 */
    }
    goto done;
none:
    Dialog(&SrEr_B, EdFl_S, 0, 0, &md);    /* T32_03 */
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

/* What the editor does with a key. Returns true if that is to leave. */
static bool T00_83(u16 key)
{
    u16 scan = key >> 8;
    u16 c = key & 0xFF;
    i32 cursor, answer;

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
            cursor = EdSub1(EditBuf, EdCursr);
            if (cursor == EdCursr)
                return 0;
            T20_02(EditBuf + cursor, EditBuf + EdCursr);
            EdCursr = cursor;
            return 0;
        case 0x14:                      /* ^T: T23_00 */
            if (ed.EdTxHex != 0)
                return 0;
            cursor = T55_00(EdCursr);
            if (cursor == EdCursr)
                return 0;
            T20_02(EditBuf + EdCursr, EditBuf + cursor);
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
                EdCursr = EdSub1(EditBuf, EdCursr);
            return 0;
        case 0x74:                      /* ^Right: T08_00 */
word_right:
            if (ed.EdTxHex == 0) {
                EdCursr = T55_00(EdCursr);
                T08_01();
            }
            return 0;
        case 0x48:                      /* Up: T09_00 */
up:
            T10_02(false, 1);
            return 0;
        case 0x50:                      /* Down: T10_00 */
down:
            T10_02(true, 1);
            return 0;
        case 0x49:                      /* PgUp: T11_00 */
page_up:
            T12_01(false);
            return 0;
        case 0x51:                      /* PgDn: T12_00 */
page_down:
            T12_01(true);
            return 0;
        case 0x47:                      /* Home: T13_00 */
            EdCursr = 0;
            ed.EdHxCur = 0;
            return 0;
        case 0x4F:                      /* End */
            T14_00();
            return 0;
        case 0x84:                      /* ^PgUp */
        case 0x77:                      /* ^Home: T15_00 */
            T51_00();
            EdOffst = 0;
            T50_00();
            EdCursr = 0;
            EdLine = 0;
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
            if ((u8)Dialog(&Srch_B, EdFl_S, 0, 0, &md) != 0)
                return 0;
            T32_00();
            return 0;
        case 0x5A:                      /* Shift F7 */
            T32_00();
            return 0;
        case 0x55:                      /* Shift F2: T33_00 */
            if ((u8)Dialog(&SavAs_B, 0, 0, 0, &md) != 0)
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
            answer = (u8)Dialog(&SavCn_B, 0, 0, 0, &md);
            if (answer == 1)
                return 1;
            if (answer > 1)
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
   `mouse`, the row in the high byte. Returns false if that is nowhere, and
   the mouse is to be asked again.

   With the button held since a click in the window: up a line with the
   mouse above the window, down a line with it below. In the window, the
   cursor goes to the line of the mouse and the column, in hex to the
   byte and the digit; a click in the last column of the cursor's own
   line, as text, is Right. */
static bool T00_71(u16 mouse)
{
    u16 base = ed.EdBase;
    u16 size = *(u16 *)&ed.EdSizeX;
    i32 row, count, column, where;
    bool down;

    ClrKbd();
    ed.EdCurLn = 1;
    if (md.WinMous != 0) {
        if (mouse >> 8 < base >> 8) {
            T10_02(false, 1);           /* T09_00 */
            return 1;
        }
        if (mouse >> 8 >= (u8)((base >> 8) + (size >> 8))) {    /* T00_72 */
            T10_02(true, 1);            /* T10_00 */
            return 1;
        }
    }
    if (InWind(mouse, base, size))      /* T00_73 */
        return 0;
    md.WinMous = 1;
    ed.EdHex1 = 0;
    mouse -= base;
    row = mouse >> 8;
    column = (u8)mouse;
    if (ed.EdTxHex == 0) {
        where = column + EdCol0;
        if (row == (u8)(EdLine - EdLin0)
            && (u8)mouse >= (u8)(size - 1)) {
            T06_10(true);
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
            ed.EdHex1 = column % 3 != 0;   /* 2, the space behind
                                                   the byte, as 1 */
            where = column / 3 | where;
        }
    } else {                            /* T00_76 */
        ed.EdHexAs = 1;
        where = (u8)column - 63;
        if (where >= 16)
            where = 15;
    }
    EdCol1 = where;                  /* T00_77 */
    count = row - (EdLine - EdLin0);
    down = true;
    if (count < 0) {
        count = -count;
        down = false;
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
    struct Ending end;
    const u8 *base;
    i32 place, start;
    i32 column, first_column, first_line, line, old_column, old_line;
    i32 delta, row, count, size, state, i;
    u16 color;
    u16 *saved, *cell;

    column = T57_02();
    EdCol = column;
    if (ed.EdUpDwn == 0) {
        EdCol1 = column;
        ed.EdHex1 = ed.EdHxCur;
    }
    ed.EdUpDwn = 0;                     /* T00_21 */
    line = EdLine;
    first_column = EdCol0;
    first_line = EdLin0;
    if (ed.EdTxHex == 0) {
        if (column < first_column
            || column == first_column && first_column != 0) {
            first_column = column;      /* T00_22 */
            if (column != 0)
                first_column--;
        } else if (column > first_column) {
            size = ed.EdSizeX - 1;      /* T00_23 */
            if (column >= size + first_column)
                first_column = column - (size - 1);
        }
    }
    if (line < first_line) {            /* T00_24 */
        first_line = line;
    } else {
        size = ed.EdSizeY;              /* T00_25 */
        if (line >= size + first_line) {
            size--;
            first_line = line < size ? 0 : line - size;
        }
    }
    old_column = EdCol0;             /* T00_26 */
    EdCol0 = first_column;
    old_line = EdLin0;
    EdLin0 = first_line;
    delta = first_line - old_line;
    if (delta != 0) {
        /* Where the first line of the window is now: so many lines on
           from where it was, or back, or back from the cursor's line if
           that is nearer. */
        row = EdLine - EdLin0;
        T60_10(&where);
        where.line = 0;
        place = 0;
        if (first_line >= old_line) {
            count = delta;
            if (count <= row) {
                where.place = EdFirst;
                do                      /* T00_27 */
                    T53_00(&where);
                while (--count);
                goto first;
            }
        } else {
            if (EdLin0 == 0)         /* T00_28 */
                goto top;
            count = -delta;
            if (count <= row) {
                row = count;
                where.place = EdFirst;
            }
        }
        count = row;                    /* T00_29 */
        if (count != 0) {
            where.line--;
            do                          /* T00_30 */
                T54_00(&where);
            while (--count);
        }
first:
        place = where.place;            /* T00_31 */
top:
        EdFirst = place;             /* T00_32 */
    }
    if (ed.EdScrn == 0) {               /* T00_33 */
        if (old_column == EdCol0) {
            if (delta == 1) {
                T71_00(0x600, ed.EdBase, 160);
                ed.EdCurLn = 0;
            } else if (delta == -1) {           /* T00_34 */
                T71_00(0x700,
                       (u16)(ed.EdBase + ((u16)(u8)(ed.EdSizeY - 1) << 8)),
                       -160);
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
    /* The assembly has the lines as offsets in the segment of the first
       of them, the paragraph that EdFirst is in, and T72_00 goes by how
       far into that segment a line ends. */
    start = EdFirst;
    base = Room + (start & ~0x0F);
    do {                                /* T00_41 */
        state = T63_01(start);
        if (state == 1 && ed.EdTxHex == 0) {
            /* The end of the text: a line of nothing, if the text ends
               with the two characters. */
            if (start < 2)              /* T00_45 */
                break;
            if (!ENDS_LINE(*(const u16 *)(Room + start - 2)))
                break;
            /* Nothing before it is looked at: the assembly hands it
               over as ending at the start of a segment. */
            T72_00(row, Room + start, Room + start, Room + start);
            break;
        }
        if (state != 0) {
            /* In hex, the end of the text or what is behind it: a line
               of no bytes, if a line begins there. */
            if (ed.EdTxHex != 0 && (start & 0x0F) == 0)     /* T00_46 */
                T72_00(row, Room + start, Room + start, base);
            break;
        }
        /* Of a line that does not end within 64K of its address the
           assembly hands over what its segment has, T00_42, and looks
           for where it ends a piece at a time. What the window shows of
           a line is at most its first 32K: as many bytes as the columns
           before the window and in it, and the cursor is no further
           than 3900 characters into a line, tabs among them. */
        end = T53_10(start);
        T72_00(row, Room + start, Room + end.at, base); /* T00_43 */
        start = end.at;
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
        /* The line from 46 on, a space, Col and a space, and the
           column: where the assembly has them, at 52 and 56, for a line
           of five digits or fewer, and further on by as many cells as
           the line has more (src/portable_c/DEVIATIONS.md, 25). What is
           behind the column is blank to the end of the line, where a
           longer line had the column before. */
        cell = md.ScrBuf + 46 + T75_00(EdLine, md.ScrBuf + 46, color);
        *cell++ = color & 0xFF00 | ' ';
        cell = MvsBw(cell, Col_S, 3, color);
        *cell++ = color & 0xFF00 | ' ';
        cell += T75_00(EdCol, cell, color);
        while (cell < md.ScrBuf + 80)
            *cell++ = color & 0xFF00 | ' ';
        /* The assembly writes here, T00_54, what is free of the room
           in seven cells from 62 on, behind "Free" at 70: a figure of
           free memory, which goes (src/portable_c/DEVIATIONS.md, 24). */
        for (i = 0; i < 80; i++)
            if (saved[i] != md.ScrBuf[i]) {
                Window(0, 0x150);
                break;
            }
    }

    column = (u8)(EdCol - EdCol0);        /* T00_60 */
    if (ed.EdTxHex != 0) {
        if (ed.EdHexAs != 0) {
            column += 63;
        } else {
            column = column * 3 + (column >> 2) + 11;
            if (ed.EdHxCur != 0)
                column++;
        }
    }
    Cursor((u16)(((u16)(u8)(EdLine - EdLin0) << 8 | (u8)column)
                 + ed.EdBase));
}

/* The loop of keys of the editor: the screen, the mouse, a key and what
   it does, until a key leaves the editor. */
static void T00_19(void)
{
    struct Mouse mouse;
    u16 key;
    i32 bar;

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
        ClrKbd();                       /* not under DESQview, in the
                                           original */
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

/* How the editor was left, struct EditEnd of vc.h: an EDIT_ there, and
   with EDIT_DONE whether a file was written. */
static struct EditEnd Ends(i32 how, bool written)
{
    struct EditEnd end;

    end.how = (u8)how;
    end.written = written;
    return end;
}

/* The editor: T00_00 of the assembly, to which the viewer's code jumps
   once it has opened the file, with the file's handle, or 0FFFFh for a
   file that is not there, and its size in EdLen. The box "Reading the
   file" is on the screen: `begin` and `size` say where. Where what it
   covers is kept, which the assembly has in SI, nothing reads.

   Returns how it was left: EDIT_VIEW if the file is to be viewed
   instead, EDIT_CLOSE if it is to be closed and nothing more, and
   EDIT_DONE when it has been edited and is closed, with EdWrite, which
   V13_01 of the viewer takes: not 0 if the file was written.

   The text goes into the memory behind the program, EdSpace paragraphs:
   a file that is not shorter than that is too large, and a box asks
   whether to view it. A new file is asked about, if the window is the
   whole screen. The file is read in pieces of 0FF00h bytes; one that
   gives fewer bytes than its size says is as long as it gives. A window
   as wide as the screen has a status line above it, and the menu file
   and the extension files have ten lines of help below theirs. */
struct EditEnd T00_00(u16 handle, u16 begin, u16 size)
{
    struct Moved read;
    struct LineArgs line;
    const u8 *name;
    u8 *end;
    u16 *cell;
    u16 *screen;
    u8 *piece;
    i32 left;
    const u8 *file;
    i32 help, count, i;
    u16 place, color;

    /* The room in bytes, as whole paragraphs, which the lines of hex
       mode, of 16 bytes each, go by. A file that is not shorter is too
       large: its length is the viewer's, of 32 bits without a sign,
       and from here on below the room. */
    EdSpace = RoomBytes() & ~0x0F;
    if ((u32)ed.EdLen >= (u32)EdSpace) {
        /* T00_01. The assembly leaves in DX for the text that ^I stands
           for, and in BX and SI for a box that is on the screen, what the
           box before had there. Nothing reads the three: the box has no
           ^I in its lines, and the dialog puts its box up itself. */
        if ((u8)Dialog(&BigFi_B, 0, 0, 0, &md) != 0)
            goto closed;
        place = (begin & 0xFF00) + 0x100 | 37;  /* T00_02 */
        MovDz0(BasAdr(place), ViFl_S);
        Window(place, 0x106);
        return Ends(EDIT_VIEW, 0);
    }

    /* T00_03: the menu file, or an extension file? */
    name = CutPath(md.ViewFil);
    end = MvzLine(md.TmpBuf, name, 13);
    *end++ = 0;
    PathLin(md.TmpBuf);
    count = end - md.TmpBuf;
    help = 1;
    file = Menu_F;
    if (!T64_00(file, md.TmpBuf, count)) {
        help = 2;
        file = Ext_F;
        if (!T64_00(file, md.TmpBuf, count)) {
            file = ViewE_F;
            if (!T64_00(file, md.TmpBuf, count)) {
                file = EditE_F;
                if (!T64_00(file, md.TmpBuf, count))
                    help = 0;
            }
        }
    }
    if (help != 0)                      /* T00_04 */
        ed.EdSizeY -= 10;               /* the rows of the help */
    if (handle == 0xFFFF) {
        /* A new file. */
        if ((u8)(md.Lines - 2) <= ed.EdSizeY
            && (u8)Dialog(&NewFi_B, 0, 0, 0, &md) != 0)
            goto closed;
    } else {
        left = ed.EdLen;                /* T00_06 */
        piece = Room;
        for (;;) {
            count = PIECE;              /* T00_07 */
            if (left < count)
                count = left;
            read = FileRead(handle, piece, count);      /* T00_08 */
            if (read.failed)
                goto closed;            /* T00_09 */
            if (TstBrk())
                goto closed;
            if (read.count != count) {
                left -= read.count;     /* T00_11 */
                ed.EdLen -= left;
                break;
            }
            piece += PIECE;
            left -= count;
            if (left == 0)
                break;
        }
        FileClose(handle);              /* T00_12 */
    }
    ResWin(begin, size, md.WinBuf);    /* T00_13 */
    Window(begin, size);
    ed.EdStat = 0;
    if (ed.EdSizeX >= 79) {
        /* The status line. */
        color = (u16)(Color(Menu_C) << 8 | ' ');
        for (i = 0; i < 80; i++)
            md.ScrBuf[i] = color;
        cell = MovDz0(BasAdr(0), EdFl_S + 1);
        *(u8 *)(cell - 1) = ':';
        MovDz0(BasAdr(41), Line_S);
        MovDz0(BasAdr(52), Col_S);
        /* "Free" at 70 goes with the figure in front of it, T00_54. */
        T74_00();
        ed.EdStat = 1;
    }
    if (help != 0) {                    /* T00_14 */
        /* The help below the window: a box between two rows of half
           blocks, its title, and six lines in each of two columns. */
        place = (u16)(ed.EdBase + ((u16)ed.EdSizeY << 8));
        screen = BasAdr(place);
        color = (u16)(Color(HlBrd_C) << 8);
        for (i = 0; i < 80; i++)
            *screen++ = color | 0xDC;
        screen = BoxDubl(screen, (u16)(Color(*EdtH_P) << 8 | 0xDC), 6, 1, 76);
        for (i = 0; i < 80; i++)
            *screen++ = color | 0xDF;
        file = EdtH1_S;
        if (help != 1)
            file = EdtH2_S;
        line.value = (place & 0xFF00) + 0x100 | 40;     /* T00_15 */
        line.screen = screen;
        line.line = file;
        line.colors = EdtH_P;
        /* None of the lines has ^I; the assembly leaves in DX what it
           had for BoxDubl, 76, which names no text. */
        line.include = 0;
        CntLine(&line);
        T76_00((place & 0xFF00) + 0x200 | 3, line.line, EdtH_P);
        T76_00((place & 0xFF00) + 0x200 | 46, EdtH_S, EdtH_P);
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
    EdLine = 0;
    EdCol0 = 0;
    EdLin0 = 0;
    EdFirst = 0;
    EdOffst = 0;
    EdCursr = 0;
    EdUndo = 0;
    md.Clock = 0;
    md.KeyBar = 1;
    ed.EdScrn = 1;
    md.HlpPage = 5;
    T50_00();
    ed.EdShape = ChgCur();
    T00_19();
    CurShp(ed.EdShape);                 /* T35_02 */
    return Ends(EDIT_DONE, ed.EdWrite);
closed:
    ResWin(begin, size, md.WinBuf);    /* T00_10 */
    Window(begin, size);
    return Ends(EDIT_CLOSE, 0);
}
