/*
 * Digits: a byte as two hex digits on the screen, and the value of a hex
 * digit or of a number that has been typed.
 *
 * Translated from VCSUB1.INC.
 */
#include "vc.h"

u16 __far *HexByt(u16 __far *screen, u16 byte);
const u8 __far *TxtNum(const u8 __far *text, u16 STACK *number);

/* How the assembly passes arguments to these, and what it expects back;
   TstHex is in vc.h. */
#pragma aux HexByt "*_c" __far parm [es di] [ax] value [es di] modify exact [di es]
#pragma aux TxtNum "*_c" __far parm [es si] [bx] value [es si] modify exact [si es]

/* Write the low byte of `byte` to the screen buffer as two hex digits,
   leaving the colours as they are, and return the address of the cell
   behind them. HexCod in the assembly is one digit of this. */
u16 __far *HexByt(u16 __far *screen, u16 byte)
{
    u16 shift = 4;
    u16 digit;

    for (;;) {
        digit = (byte >> shift & 0x0F) + '0';
        if (digit > '9')
            digit += 'A' - '9' - 1;
        *(u8 __far *)screen = (u8)digit;
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
u16 TstHex(u16 digit)
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
const u8 __far *TxtNum(const u8 __far *text, u16 STACK *number)
{
    u16 value = 0;
    u16 digit;

    for (;;) {
        digit = (u16)*text - '0';
        if (digit > 9)
            break;
        text++;
        /* Ten times more than 6553 does not fit, nor does 65530 and a
           digit above 5; and once at 0FFFFh it stays there. */
        if (value > 6553) {
            value = 0xFFFF;
        } else {
            value *= 10;
            value = (u16)(value + digit) < value ? 0xFFFF : value + digit;
        }
    }
    *number = value;
    return text;
}
