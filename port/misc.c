/*
 * Small routines that many parts of the program share and that belong to
 * none of them: the end of a line, a switch turned over, a sum of bytes,
 * and a page up or down in a list.
 *
 * Translated from VCSUB1.INC.
 */
#include "vc.h"

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
   cursor goes there. Returns the cursor in the low word and the first
   line in the high one. */
u32 PgUp(u16 cursor, u16 first, u16 rows)
{
    if (first == 0)
        cursor = 0;
    else {
        rows--;
        first = first >= rows ? first - rows : 0;
        rows += first;
        if (cursor > rows)
            cursor = rows;
    }
    return (u32)first << 16 | cursor;
}

/* A page down in such a list of `lines` lines. The list moves by a line
   less than show, no further than to where its last lines fill the
   window, and the cursor stays in what shows; with the end of the list
   showing, the cursor goes to the last line. */
u32 PgDn(u16 cursor, u16 first, u16 rows, u16 lines)
{
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
    return (u32)first << 16 | cursor;
}
