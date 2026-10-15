/*
 * Digits: a byte as two hex digits on the screen, and the value of a hex
 * digit or of a number that has been typed.
 *
 * Translated from VCSUB1.INC.
 */
#include "vc.h"

/* HexByt, TstHex and TxtNum are in vc.h. */

/* Write the low byte of `byte` to the screen buffer as two hex digits,
   leaving the colours as they are, and return the address of the cell
   behind them. HexCod in the assembly is one digit of this. `byte` is
   bits, of which eight are read, and has no sign for that. */
u16 *HexByt(u16 *screen, u32 byte)
{
    i32 shift = 4;
    u16 digit;

    for (;;) {
        digit = (byte >> shift & 0x0F) + '0';
        if (digit > '9')
            digit += 'A' - '9' - 1;
        *(u8 *)screen = (u8)digit;
        screen++;
        if (shift == 0)
            return screen;
        shift = 0;
    }
}

/* The value of a hex digit, or a number above 0FFh if `digit` is none.
   Only the low byte of `digit` counts. A letter may be small.

   What is above 0FFh has in its low byte what the assembly leaves in AL
   for a character that is no digit: the character if it comes before '0',
   and else its capital. */
i32 TstHex(u16 digit)
{
    digit &= 0xFF;
    if (digit < '0')
        return 0xFF00 | digit;
    if (digit <= '9')
        return digit - '0';
    digit = UpCase(digit) & 0xFF;
    if (digit < 'A' || digit > 'F')
        return 0xFF00 | digit;
    return digit - ('A' - 10);
}

/* Read a number in decimal digits from a text, store it at `number`, and
   return the address of what follows the digits: `text` itself, if it
   begins with none. A number too large for 16 bits is read as 0FFFFh. */
const u8 *TxtNum(const u8 *text, u16 *number)
{
    i32 value = 0;
    i32 digit;

    for (;;) {
        /* It wraps: a character below '0' is a number above 9. */
        digit = (u16)(*text - '0');
        if (digit > 9)
            break;
        text++;
        /* Ten times more than 6553 does not fit, nor does 65530 and a
           digit above 5; and once at 0FFFFh it stays there. The sum
           wraps in 16 bits, as the original finds by its carry that it
           does not fit. */
        if (value > 6553) {
            value = 0xFFFF;
        } else {
            value *= 10;
            value = (u16)(value + digit) < value ? 0xFFFF : value + digit;
        }
    }
    *number = (u16)value;
    return text;
}
