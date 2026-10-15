/*
 * The frame of a box in the screen buffer, with its shadow, and the box
 * growing on the screen.
 *
 * Translated from VCSUB1.INC.
 *
 * A box has a margin of three cells to the left and to the right of its
 * frame, in its own colour, and an empty row above it; its shadow is two
 * cells wide to the right and a row deep below. A size has the rows in
 * its high byte and the columns in its low one, the margins counted, and
 * a place the row and the column likewise.
 */
#include "vc.h"

#define ShadFl  38                      /* the colour that says how shadows
                                           are made */
#define Diskret 5                       /* the steps in which a box grows */

#define SPACE       0x20
#define SPACES      0x2020
#define DOUBLE      0xCD                /* a double line */
#define TOP_CORNERS 0xC9BB              /* the corners of a double frame, */
#define SIDES       0xBABA              /* its sides */
#define BOTTOM_CORNERS 0xC8BC           /* and its lower corners */

u8 *ColAddr(u16 number);                                /* screen.c */
u16 *Shadow(u16 *cell);

/* Write a line of a box to the screen buffer, and return the address of
   the cell behind it: `margin` spaces, the character in the high byte of
   `ends`, `width` times the cell `cell`, the character in the low byte of
   `ends`, and the spaces again, all in the colour of `cell`. */
static u16 *BoxLn0(u16 *screen, u16 cell, u16 ends, i32 margin, i32 width)
{
    u16 color = cell & 0xFF00;
    i32 count;

    for (count = margin; count != 0; count--)
        *screen++ = color | SPACE;
    *screen++ = color | ends >> 8;
    for (count = width; count != 0; count--)
        *screen++ = cell;
    *screen++ = color | (ends & 0xFF);
    for (count = margin; count != 0; count--)
        *screen++ = color | SPACE;
    return screen;
}

/* The same, and the address of the line below comes back. */
u16 *BoxLine(u16 *screen, u16 cell, u16 ends, i32 margin, i32 width)
{
    BoxLn0(screen, cell, ends, margin, width);
    return screen + 80;
}

/* The same with the two cells behind the line put into the shadow. */
static u16 *ShadLin(u16 *screen, u16 cell, u16 ends, i32 margin, i32 width)
{
    Shadow(Shadow(BoxLn0(screen, cell, ends, margin, width)));
    return screen + 80;
}

/* Write a box with a double frame and `rows` empty lines in it, in the
   colour in the high byte of `color`, and return the address of the line
   below it. `rows` is at least 1: every caller has it so. */
u16 *BoxDubl(u16 *screen, u16 color, i32 rows, i32 margin, i32 width)
{
    color &= 0xFF00;
    screen = BoxLine(screen, color | DOUBLE, TOP_CORNERS, margin, width);
    do
        screen = BoxLine(screen, color | SPACE, SIDES, margin, width);
    while (--rows);
    return BoxLine(screen, color | DOUBLE, BOTTOM_CORNERS, margin, width);
}

/* Write a box into the screen buffer at `begin`, of the size `size`, in
   the colour in the high byte of `color`: an empty row, the frame with
   as many rows in it as the size has beyond five, an empty row below it,
   and the shadow. The width is a byte: a box of fewer than ten columns
   has 256 more between its sides. */
static void ShadBox(u16 begin, u16 size, u16 color)
{
    u16 *screen = BasAdr(begin);
    i32 rows = size >> 8;
    /* A box of fewer than ten columns wraps round to 256 more, as the
       original's byte does. */
    i32 width = (u8)(size - 10);
    u16 count;

    color &= 0xFF00;
    screen = BoxLine(screen, color | SPACE, SPACES, 3, width);
    screen = ShadLin(screen, color | DOUBLE, TOP_CORNERS, 3, width);
    if (rows > 5) {
        rows -= 5;
        do
            screen = ShadLin(screen, color | SPACE, SIDES, 3, width);
        while (--rows);
    }
    screen = ShadLin(screen, color | DOUBLE, BOTTOM_CORNERS, 3, width);
    screen = ShadLin(screen, color | SPACE, SPACES, 3, width);
    screen += 2;
    count = (u8)(width + 8);
    do
        screen = Shadow(screen);
    while (--count);
}

/* Wait until the BIOS's count of timer ticks is no longer `tick` in its
   low word, and return what it is then. */
static u16 MinBx3(u16 tick)
{
    u16 now;

    do {
        IntOn();
        now = (u16)Timer();
    } while (now == tick);
    return now;
}

/* A length of a box that is growing, and where it begins: `size` and
   `begin`, bytes, at the step `step`, of which the fourth is the first
   shown and the smallest. What the box has beyond `least` is halved, and
   as many quarters of that as the step says are taken off at either end.
   Returns the length in the low byte and the beginning in the high
   one. */
static u16 MinBx5(i32 size, i32 begin, i32 step, i32 least)
{
    i32 less = (u8)(((u16)(u8)(size - least) >> 1) * step / (Diskret - 1));

    return (u16)((u16)(u8)(begin + less) << 8 | (u8)(size - less - less));
}

/* Draw a box with its shadow into the screen buffer, at `begin` and of
   the size `size`, in the colour in the high byte of `color`. If windows
   are to zoom the box grows on the screen first, in four steps of a
   timer tick each, from one of eleven columns and five rows in its
   middle, without a shadow; the last one, of full size, is left for the
   caller to put on the screen. The next character typed is no longer
   taken as it is. */
void MinBox(u16 begin, u16 size, u16 color)
{
    u8 *shadow;
    u8 shadows;
    u16 tick, across, down;
    i32 step;

    md.Quote = 0;
    if (md.ZoomWin != 0) {
        shadow = ColAddr(ShadFl);
        shadows = *shadow;
        *shadow = 0;
        tick = MinBx3(0);
        for (step = Diskret - 1; step != 0; step--) {
            across = MinBx5(size & 0xFF, begin & 0xFF, step, 11);
            down = MinBx5(size >> 8, begin >> 8, step, 5);
            /* The sizes: the cast cuts off the beginning that `down` has
               in its high byte. */
            ShadBox((down & 0xFF00) | across >> 8,
                    (u16)(down << 8 | (across & 0xFF)), color);
            tick = MinBx3(tick);
            Window((down & 0xFF00) | across >> 8,
                   (u16)(down << 8 | (across & 0xFF)));
        }
        MinBx3(tick);
        *shadow = shadows;
    }
    ShadBox(begin, size, color);
}

/* Write a line of a box that MinBox has drawn, at `place`: between the
   characters of `ends` the cell `cell`, over the width that the low byte
   of `size` has for the box. Returns the place a row below. */
u16 StdLine(u16 place, u16 cell, u16 ends, u16 size)
{
    /* Fewer than ten columns wrap round, as in ShadBox. */
    BoxLine(BasAdr(place), cell, ends, 3, (u8)(size - 10));
    return place + 0x100;
}

/* Put a box of the assembly on the screen, with its middle at `place`,
   growing only if no box has since the program last waited for a key at
   the panels: `box` and `include` are as DialBox takes them, and where
   the box is comes back in `where`. */
void OncZoom(u16 place, const struct BOXTOP *box, const u8 *include,
             struct Box *where)
{
    u8 zoom = md.ZoomWin;

    if (md.ZoomOne != 0)
        md.ZoomWin = 0;
    DialBox(place, box, include, &md, where);
    Window(where->begin, where->size);
    md.ZoomWin = zoom;
    md.ZoomOne = 1;
}
