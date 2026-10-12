/*
 * Texts that end with a zero byte, and the paths and file names among
 * them.
 *
 * Translated from VCSUB1.INC.
 *
 * A text may be in any segment: the main data, a file panel, the code. So
 * these take far pointers, and where one hands back a place in a text it
 * was given, that is in the same segment.
 */
#include "vc.h"

void FilZero(u8 __far *text, u16 length);
u16 StrLen(const u8 __far *text);
const u8 __far *FndLet(const u8 __far *text);
void NormStr(u8 __far *text);
u8 __far *MainFil(u8 __far *to, const u8 __far *name);

/* How the assembly passes arguments to these, and what it expects back.
   The assembly has a text at DS:SI or ES:DI; DS cannot carry an argument
   here, so its stubs in port/asm/VCSUB1.INC move that elsewhere. AddFile,
   MvzLine, PathLin, LetLin and ShrtLin are in vc.h. */
#pragma aux FilZero "*_c" __far parm [es si] [cx] modify exact [ax]
#pragma aux StrLen  "*_c" __far parm [es di] value [cx] modify exact [cx]
#pragma aux FndLet  "*_c" __far parm [es si] value [es si] modify exact [si es]
#pragma aux NormStr "*_c" __far parm [es si] modify exact [ax]
#pragma aux MainFil "*_c" __far parm [es di] [dx si] value [es di] modify exact [di es]

/* End a path with a backslash, unless it has one: `end` is where the path
   ends. Returns where it ends then. Nothing is written behind it. */
u8 __far *AddFile(u8 __far *end)
{
    if (end[-1] != '\\')
        *end++ = '\\';
    return end;
}

/* Copy `length` bytes that hold a text, the text and whatever follows it.
   Returns where the text ends in the copy, the address of its zero byte.

   A text without an end among these bytes is taken to end with the last
   of them, and if there are no bytes at all, with the one before. */
u8 __far *MvzLine(u8 __far *to, const u8 __far *from, u16 length)
{
    u16 i;

    for (i = 0; i < length; i++)
        to[i] = from[i];
    for (i = 0; i < length; i++)
        if (to[i] == 0)
            return to + i;
    return to + length - 1;
}

/* Fill what follows a text with zeros, in a field of `length` bytes. */
void FilZero(u8 __far *text, u16 length)
{
    while (length) {
        length--;
        if (*text++ == 0)
            break;
    }
    for (; length; length--)
        *text++ = 0;
}

/* The length of a text, with its zero byte. */
u16 StrLen(const u8 __far *text)
{
    u16 length = 0;

    do
        length++;
    while (*text++ != 0 && length != 0xFFFF);
    return length;
}

/* The small letter for a capital, by the country's table for the letters
   beyond ASCII. Only the low byte of `letter` counts. */
u16 LowCase(u16 letter)
{
    letter &= 0xFF;
    if (letter >= 'A' && letter <= 'Z')
        return letter + ('a' - 'A');
    if (letter >= 0x80)
        return md.LowTab[letter - 0x80];
    return letter;
}

/* Skip spaces and tabs. Returns the address of what follows them. */
const u8 __far *FndLet(const u8 __far *text)
{
    while (*text == ' ' || *text == '\t')
        text++;
    return text;
}

/* The file name in a path: what follows its last backslash or colon. */
const u8 __far *CutPath(const u8 __far *path)
{
    const u8 __far *name = path;
    u8 c;

    while ((c = *path++) != 0)
        if (c == ':' || c == '\\')
            name = path;
    return name;
}

/* Take the spaces off both ends of a text. */
void NormStr(u8 __far *text)
{
    const u8 __far *from = text;
    u8 __far *end;                      /* behind the last that is no space */
    u8 c;

    while (*from == ' ')
        from++;
    do {
        end = text;
        do {
            c = *from++;
            *text++ = c;
        } while (c == ' ');
    } while (c != 0);
    *end = 0;
}

/* Turn a text into capitals. */
void PathLin(u8 __far *text)
{
    u8 c;

    do {
        c = (u8)UpCase(*text);
        *text++ = c;
    } while (c != 0);
}

/* Write a path the way the program shows one: in capitals, but for the
   file name, which is in small letters. */
void LetLin(u8 __far *text)
{
    u8 c;

    PathLin(text);
    text = (u8 __far *)CutPath(text);
    do {
        c = (u8)LowCase(*text);
        *text++ = c;
    } while (c != 0);
}

/* Shorten a path to `limit` characters by leaving out its start, with
   three dots in its place. A drive and a backslash that it begins with
   are kept in front of the dots, whatever the path is then left with.

   The length is looked for in 256 bytes, and a text without an end among
   them is taken to be 255 long. A limit too small for the drive, the
   backslash and the dots is not checked for: the count of what remains
   to be moved up then passes zero, and 64K are moved. */
void ShrtLin(u8 __far *text, u16 limit)
{
    const u8 __far *from;
    u16 length = 0;

    while (length < 256 && text[length] != 0)
        length++;
    if (length == 256)
        length = 255;
    if (length <= limit)
        return;
    from = text + (length - limit);
    if (text[1] == ':') {
        text += 2;
        from += 2;
        limit -= 2;
    }
    if (text[0] == '\\') {
        text++;
        from++;
        limit--;
    }
    *text++ = '.';
    *text++ = '.';
    *text++ = '.';
    from += 3;
    /* What is left of the limit, and the zero byte. */
    for (limit -= 2; limit; limit--)
        *text++ = *from++;
}

/* Make the path of a file that the program keeps in its own directory:
   NcPath, and the 13 bytes at `name`. Returns the address behind them. */
u8 __far *MainFil(u8 __far *to, const u8 __far *name)
{
    u16 i;

    to = MvzLine(to, md.NcPath, LenPath);
    for (i = 0; i < 13; i++)
        *to++ = name[i];
    return to;
}
