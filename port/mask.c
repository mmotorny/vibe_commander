/*
 * File names in the form they are compared in, and masks.
 *
 * Translated from VCSUB1.INC.
 *
 * In that form a name is 11 characters, as in a directory of DOS: eight
 * of the name and three of the extension, in capitals, each part filled up
 * with spaces. A mask is the same, with a question mark where any
 * character will do; an asterisk has become question marks to the end of
 * its part.
 */
#include "vc.h"

void Convert(const u8 __far *name, u8 *buffer);
u8 __far *UnConv(u8 __far *name, const u8 __far *buffer);
u16 TstMas(const u8 __far *name, const u8 *mask);

/* How the assembly passes arguments to these, and what it expects back.
   The name is where the assembly has DS, the buffer or the mask in the
   main data. */
#pragma aux Convert "*_c" __far parm [dx si] [di] modify exact [ax]
#pragma aux UnConv  "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]
#pragma aux TstMas  "*_c" __far parm [dx si] [di] value [ax] modify exact [ax]

/* Fill one part of a name in its fixed form, `length` characters at
   `field`, from the text at `name`, which ends with a zero byte or with
   `end`. Returns the address of what follows the part in the text: the
   end, or the asterisk that stands for the rest. Conv10 in the assembly. */
static const u8 __far *Part(const u8 __far *name, u8 *field, u16 length,
                            u16 end)
{
    u16 c;

    do {
        c = *name++;
        if (c == 0 || c == end || c == '*') {
            c = c == '*' ? '?' : ' ';
            do
                *field++ = (u8)c;
            while (--length);
            return name - 1;
        }
        *field++ = (u8)UpCase(c);
    } while (--length);
    return name;
}

/* Put a file name, or a mask, into its fixed form: 11 characters at
   `buffer`. What a part has too much is left out. */
void Convert(const u8 __far *name, u8 *buffer)
{
    u8 c;

    name = Part(name, buffer, 8, '.');
    do
        c = *name++;
    while (c != '.' && c != 0);
    if (c == 0)
        name--;
    Part(name, buffer + 8, 3, 0);
}

/* Make a file name of its fixed form: the name without its spaces, a full
   stop, and the extension without its spaces, or neither if there is no
   extension. Returns the address of the zero byte that ends it. */
u8 __far *UnConv(u8 __far *name, const u8 __far *buffer)
{
    u8 __far *end = name;               /* behind the last that is no space */
    u16 i;
    u8 c;

    for (i = 0; i < 11; i++) {
        if (i == 8) {
            name = end;
            *name++ = '.';
        }
        c = buffer[i];
        *name++ = c;
        if (c != ' ')
            end = name;
    }
    *end = 0;
    return end;
}

/* Whether a file name fits a mask in its fixed form: 1 if it does, 0 if
   not. The name is put into its fixed form at TmpBuf+20. */
u16 TstMas(const u8 __far *name, const u8 *mask)
{
    u8 *fixed = md.TmpBuf + 20;
    u16 i;

    Convert(name, fixed);
    for (i = 0; i < 11; i++)
        if (mask[i] != '?' && mask[i] != fixed[i])
            return 0;
    return 1;
}
