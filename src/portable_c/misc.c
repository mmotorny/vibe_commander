/*
 * Small routines that many parts of the program share and that belong to
 * none of them: the end of a line, a switch turned over, a sum of bytes,
 * and a page up or down in a list; and where the parts of the program
 * are in the memory.
 *
 * Translated from VCSUB1.INC.
 */
#include "vc.h"

/* In the part of the program that stays in the memory: the segment that
   the part which goes is counted from, and how many paragraphs behind
   it that part begins. */
extern const u8 __far RunPar[];
extern const u8 __far TrnPrW[];
#pragma aux RunPar "*"
#pragma aux TrnPrW "*"

#define OFFSET(object) ((u16)(u32)(const void __far *)(object))
#define WORD_AT(segment, offset) \
    (*(const u16 __far *)((u32)(segment) << 16 | (u16)(offset)))

/* The segment of the program's constants. The original is one segment,
   and has its texts and tables at offsets in it, which its code reads
   through CS. Here they are a group of their own, and what is known by
   its offset, a box, a dialog, a text of one, is read at that offset in
   this segment: the segment of any of them, a number that is set when
   the program is loaded. */
u16 AsmSeg(void)
{
    return (u16)((u32)(const void __far *)Title_S >> 16);
}

/* The segment of the part of the program that stays in the memory while
   a command runs. It begins with the PSP and ends with the stack, so it
   is the segment of anything that is on the stack: SS. */
u16 ResSeg(void)
{
    u16 here;

    return (u16)((u32)(void __far *)&here >> 16);
}

/* The segment of the block of memory that this code is in: the part of
   the program that the original gives back to DOS while a command runs,
   and loads again. */
u16 MemRun(void)
{
    u16 stays = ResSeg();

    return WORD_AT(stays, OFFSET(RunPar)) + WORD_AT(stays, OFFSET(TrnPrW));
}

/* Is `word` the two bytes that end a line, in either order? */
u16 IsCrLf(u16 word)
{
    return word == 0x0D0A || word == 0x0A0D;
}

/* Turn a switch over: a byte that is 0 becomes 1, and any other 0.
   Returns what it is then. */
u16 InvVar(u8 __far *variable)
{
    return *variable = *variable == 0;
}

/* The sum of `length` bytes of the main data, each a number of its own.
   No bytes are 65536 of them. */
u16 ChkSum(const u8 *data, u16 length)
{
    u16 sum = 0;

    do
        sum += *data++;
    while (--length);
    return sum;
}

/* A page up in a list of which `rows` lines show: `cursor` is the number
   of the line the cursor is on and `first` that of the first line shown.
   The list moves by a line less than show, or as far as it goes, and the
   cursor stays in what shows; with the top of the list showing, the
   cursor goes there. Returns the cursor and the first line then. */
struct Paged PgUp(u16 cursor, u16 first, u16 rows)
{
    struct Paged paged;

    if (first == 0)
        cursor = 0;
    else {
        rows--;
        first = first >= rows ? first - rows : 0;
        rows += first;
        if (cursor > rows)
            cursor = rows;
    }
    paged.cursor = cursor;
    paged.first = first;
    return paged;
}

/* A page down in such a list of `lines` lines. The list moves by a line
   less than show, no further than to where its last lines fill the
   window, and the cursor stays in what shows; with the end of the list
   showing, the cursor goes to the last line. */
struct Paged PgDn(u16 cursor, u16 first, u16 rows, u16 lines)
{
    struct Paged paged;
    u16 last = lines >= rows ? lines - rows : 0;
    u16 next;

    rows--;
    if (first >= last)
        cursor = lines != 0 ? lines - 1 : 0;
    else {
        next = first + rows;
        first = next < first || next > last ? last : next;
        if (cursor < first)
            cursor = first;
    }
    paged.cursor = cursor;
    paged.first = first;
    return paged;
}
