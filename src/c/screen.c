/*
 * Colours, and characters and colours in the screen buffer.
 *
 * Translated from VCSUB1.INC.
 *
 * The program draws into ScrBuf, a copy of the screen in the main data,
 * and another routine puts that on the screen. A cell is a word: the
 * character in the low byte and its colour in the high one.
 */
#include "vc.h"

#define ShadFl 38                       /* the colour that says how shadows
                                           are made */

u8 *ColAddr(u16 number);
u16 *Shadow(u16 *cell);
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color);

/* How the assembly passes arguments to these, and what it expects back;
   Color, BasAdr, MvsColr and MovDz0 are in vc.h. */
#pragma aux ColAddr "*_c" __far parm [ax] value [di] modify exact [di]
#pragma aux Shadow  "*_c" __far parm [di] value [di] modify exact [di]
#pragma aux MvsBw   "*_c" __far parm [es di] [dx si] [cx] [ax] value [es di] modify exact [di es]

/* Where in the colour table a colour is, for the palette that is set up
   and the display. Only the low byte of `number` counts.

   A colour has six bytes in the table: for each of the three palettes,
   black and white, colour and laptop, one for a colour display and one for
   a monochrome one. A palette that is none of the three counts as colour.

   This is written out twice, and without a multiplication, because Color
   is called from the bottom of the deepest calls and the program's stack
   is short: a function shared by the two, or a register saved around a
   multiplication, costs bytes of it. */
#define IN_TABLE(number, palette) \
    (md.ColTab + ((((number) & 0xFF) << 1) + ((number) & 0xFF) + (palette) \
                  << 1) + (md.ScrMod == 7 ? 1 : 0))

u8 *ColAddr(u16 number)
{
    u16 palette = md.ColTyp;

    if (palette >= 3)
        palette = 1;
    return IN_TABLE(number, palette);
}

/* A colour of the program's, by its number, as the attribute byte of a
   cell. Only the low byte of `number` counts. */
u16 Color(u16 number)
{
    u16 palette = md.ColTyp;

    if (palette >= 3)
        palette = 1;
    return *IN_TABLE(number, palette);
}

/* Put a cell into the shadow of a box, by changing its colour, and return
   the address of the next cell. The colour ShadFl says how: 0 leaves the
   colour alone, 1 takes the background and the blinking off it and turns
   its brightness round, and anything else makes it grey on black. */
u16 *Shadow(u16 *cell)
{
    u8 *attribute = (u8 *)cell + 1;
    u16 how = Color(ShadFl);

    if (how == 1)
        *attribute = *attribute & 0x8F ^ 8;
    else if (how != 0)
        *attribute = 7;
    return cell + 1;
}

/* Write `count` characters to the screen buffer in the colour in the high
   byte of `color`, and return the address of the cell behind them. A count
   of 0 is one of 65536. This is MvsBwc in the assembly, where the
   characters are in the code segment, and MvsBwe, where they are with the
   screen buffer. */
u16 __far *MvsBw(u16 __far *screen, const u8 __far *text, u16 count,
                 u16 color)
{
    color &= 0xFF00;
    do
        *screen++ = color | *text++;
    while (--count);
    return screen;
}

/* Give `count` cells of the screen buffer the colour in the low byte of
   `color`, and return the address of the cell behind them. A count of 0
   is one of 65536. */
u16 __far *MvsColr(u16 __far *screen, u16 count, u16 color)
{
    do {
        ((u8 __far *)screen)[1] = (u8)color;
        screen++;
    } while (--count);
    return screen;
}

/* Write a text to the screen buffer, leaving the colours as they are, and
   return the address of the cell behind it. MovDz in the assembly is this
   at a row and a column, which it makes an address of with BasAdr. */
u16 __far *MovDz0(u16 __far *screen, const u8 __far *text)
{
    u8 c;

    while ((c = *text++) != 0) {
        *(u8 __far *)screen = c;
        screen++;
    }
    return screen;
}

/* The cell of the screen buffer at a row and a column, the row in the high
   byte of `place`. */
u16 *BasAdr(u16 place)
{
    u16 row = place >> 8;

    /* 80 cells to a row, without a multiplication: see IN_TABLE. */
    return md.ScrBuf + (row << 6) + (row << 4) + (place & 0xFF);
}
