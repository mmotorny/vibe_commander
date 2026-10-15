/*
 * Lines of text with markup in them, which is how the program writes most
 * of what it shows.
 *
 * Translated from VCSUB1.INC.
 *
 * A line ends with a zero byte. In it,
 *
 *   ^^           stands for a caret,
 *   ^V and ^W    for the texts at TmpErr+1 and TmpErr+101, which the
 *                caller has put there,
 *   ^I           for the include line that the caller names,
 *   ^N ^R ^B ^U  switch to the normal, the reversed, the bold or the
 *                underlined colour, the first to the fourth of the
 *                caller's colours; naming the one that is on switches
 *                back to normal,
 *   ^Axx         switches to the colour xx, in hex,
 *   ^Cxx         stands for the character with the code xx, in hex,
 *   0FFh n       stands for n spaces, and 0FFh 80h+n c for n times c.
 *
 * The letter after a caret may be small. Any other letter means nothing.
 *
 * This is the markup of Norton Guides, whose help files the program's own
 * is made like. The original's NgLine has ^A and ^C the other way round,
 * unlike its LnStr and unlike Norton Guides; here they are put right (see
 * harness/deviations.py).
 */
#include "vc.h"

i32 LnStr(const struct LineArgs *args);

/* struct LineArgs, NgLine and CntLine are in vc.h. */

/* The length of a text that is put into a line, as LnStr counts it: it
   looks at 80 characters, and a text without an end among them counts as
   79 long. */
static i32 Included(const u8 *text)
{
    i32 length = 0;

    while (length < 80 && text[length] != 0)
        length++;
    return length < 80 ? length : 79;
}

/* The number of cells that a line takes on the screen. `args->include` is
   the include line; nothing else of `args` but the line is looked at.

   The count is kept as the assembly keeps it: what a run of characters or
   a text put in adds to it is added to its low byte only. */
i32 LnStr(const struct LineArgs *args)
{
    const u8 *line = args->line;
    i32 length = 0;
    u16 first, second;
    i32 more;

    for (;;) {
        first = line[0];
        second = line[1];
        line += 2;
        if (first == 0)
            break;
        if (first == '^') {
            if (second == '^') {
                length++;
                continue;
            }
            second = UpCase(second) & 0xFF;
            more = 0;
            if (second == 'V') {
                more = Included(md.TmpErr + 1);
            } else if (second == 'W') {
                more = Included(md.TmpErr + 101);
            } else if (second == 'I') {
                more = Included(args->include);
            } else if (second == 'A') {
                line += 2;
            } else if (second == 'C') {
                length++;
                line += 2;
            }
        } else if (first == 0xFF) {
            /* The count less one, and one more unless a character follows
               it, which is then counted like any other. It wraps: a
               count of 0 less one is 0FFFFh, of which the low byte and
               the highest bit are read. */
            more = (u16)((second << 8 | (second & 0x7F)) - 1);
            more = (more & 0xFF) + (more & 0x8000 ? 0 : 1);
        } else {
            line--;
            length++;
            continue;
        }
        length = (length & ~0xFF) | (length + more & 0xFF);
    }
    return length;
}

/* Write a line to the screen buffer at `args->screen`, in at most as many
   cells as the low byte of `args->value` says; what does not fit is left
   out. Moves `args->line` behind the line's end and `args->screen` behind
   what was written. */
void NgLine(struct LineArgs *args)
{
    const u8 *line = args->line;
    const u8 *text;
    u16 *screen = args->screen;
    i32 room = args->value & 0xFF;
    i32 style = 0;                      /* which of the colours is on */
    u16 color = (u16)(Color(args->colors[0]) << 8);
    u16 c;
    u16 code;                           /* a character by its number */
    i32 count, high;

#define PUT(character) \
    if (room) { \
        *screen++ = color | (character); \
        room--; \
    } else (void)0

    for (;;) {
        c = *line++;
        if (c == 0)
            break;
        if (c == 0xFF) {
            count = *line++;
            c = ' ';
            if (count & 0x80)
                c = *line++;
            for (count &= 0x7F; count; count--)
                PUT(c);
            continue;
        }
        if (c == '^') {
            c = *line++;
            if (c != '^') {
                c = UpCase(c) & 0xFF;
                if (c == 'V' || c == 'W' || c == 'I') {
                    text = c == 'V' ? md.TmpErr + 1
                         : c == 'W' ? md.TmpErr + 101
                         : args->include;
                    while ((c = *text++) != 0)
                        PUT(c);
                    continue;
                }
                if (c == 'N' || c == 'R' || c == 'B' || c == 'U') {
                    count = c == 'N' ? 0 : c == 'R' ? 1 : c == 'B' ? 2 : 3;
                    if (count == style)
                        count = 0;
                    style = count;
                    color = (u16)(Color(args->colors[style]) << 8);
                    continue;
                }
                if (c != 'A' && c != 'C')
                    continue;
                high = TstHex(*line++);
                code = *line++;
                if (high > 0xFF)
                    continue;
                count = TstHex(code);
                if (count > 0xFF)
                    continue;
                code = (high << 4 | count) & 0xFF;
                if (c == 'A') {
                    color = (u16)(code << 8);
                    style = 0;
                    continue;
                }
                c = code;
            }
        }
        PUT(c);
    }
#undef PUT
    args->line = line;
    args->screen = screen;
}

/* Write a line to the screen buffer with its middle at the row and the
   column in `args->value`, the row in the high byte. Moves `args->line`
   behind the line's end. */
void CntLine(struct LineArgs *args)
{
    struct LineArgs line = *args;
    i32 length = LnStr(args);

    line.screen = BasAdr(args->value) - ((length + 1) >> 1);
    /* NgLine reads the low byte of it and no more, the cells that it
       may write in. */
    line.value = (u16)length;
    NgLine(&line);
    args->line = line.line;
}
