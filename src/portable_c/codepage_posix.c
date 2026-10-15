/*
 * The code page of the program for Linux, 437: the characters of Unicode
 * that its bytes are, what the VGA's font shows for each, DOS's capitals
 * of its bytes, and UTF-8. See emulate_posix.h.
 */
#include <string.h>

#include "emulate_posix.h"

/* The characters of Unicode of the bytes 80h to FFh, by Unicode's
   mapping file, VENDORS/MICSFT/PC/CP437.TXT, which is what Python's codec
   cp437 is made of: the table was written from that codec. */
static const u16 Unicode437[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,   /* 80h */
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,   /* 88h */
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,   /* 90h */
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,   /* 98h */
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,   /* A0h */
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,   /* A8h */
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,   /* B0h */
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,   /* B8h */
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,   /* C0h */
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,   /* C8h */
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,   /* D0h */
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,   /* D8h */
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,   /* E0h */
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,   /* E8h */
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,   /* F0h */
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0,   /* F8h */
};
/* DOS's capitals of the bytes 80h to FFh for the names of files, the
   table that DOS answers INT 21h 6504h with: of FreeDOS 1.4's COUNTRY.SYS,
   the bench's, for the country 1 with the code page 437, its table
   "FUCASE" there. The bench of the United States names no country, and
   its DOS then has the table of the country 1 in itself. */
static const u8 Upper437[128] = {
    0x80, 0x9A, 0x45, 0x41, 0x8E, 0x41, 0x8F, 0x80,   /* 80h */
    0x45, 0x45, 0x45, 0x49, 0x49, 0x49, 0x8E, 0x8F,   /* 88h */
    0x90, 0x92, 0x92, 0x4F, 0x99, 0x4F, 0x55, 0x55,   /* 90h */
    0x59, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,   /* 98h */
    0x41, 0x49, 0x4F, 0x55, 0xA5, 0xA5, 0xA6, 0xA7,   /* A0h */
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,   /* A8h */
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,   /* B0h */
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,   /* B8h */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,   /* C0h */
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,   /* C8h */
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,   /* D0h */
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,   /* D8h */
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,   /* E0h */
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,   /* E8h */
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,   /* F0h */
    0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,   /* F8h */
};
u32 CodePageToUnicode(u8 byte)
{
    if (byte < 0x80)
        return byte;
    return Unicode437[byte - 0x80];
}

/* What the VGA's font shows for the bytes below 20h, where Unicode's
   mapping of the code page has the controls of ASCII; 0 is a blank. */
static const u16 Glyphs[0x20] = {
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC
};
#define HOUSE 0x2302                    /* what the font has at 7Fh */

u32 CodePageGlyph(u8 byte)
{
    if (byte < 0x20)
        return Glyphs[byte];
    if (byte == 0x7F)
        return HOUSE;
    return CodePageToUnicode(byte);
}

i32 CodePageFromUnicode(u32 character)
{
    i32 i;

    if (character < 0x80)
        return (i32)character;
    for (i = 0; i < 128; i++)
        if (Unicode437[i] == character)
            return 0x80 + i;
    return -1;
}

u8 CodePageUpper(u8 byte)
{
    if (byte >= 'a' && byte <= 'z')
        return (u8)(byte - ('a' - 'A'));
    if (byte < 0x80)
        return byte;
    return Upper437[byte - 0x80];
}

/* The bits of a byte of UTF-8 that carry the character: those below the
   first 0 bit. */
static u32 Bits(char byte, u32 mask)
{
    return (u32)(u8)byte & mask;
}

static bool Continues(char byte)
{
    return ((u8)byte & 0xC0) == 0x80;
}

i32 Utf8Decode(const char *text, i32 length, u32 *character)
{
    u8 first;
    i32 count;
    u32 found;
    u32 least;
    i32 i;

    if (length <= 0)
        return 0;
    first = (u8)text[0];
    if (first < 0x80) {
        *character = first;
        return 1;
    }
    if ((first & 0xE0) == 0xC0) {
        count = 2;
        found = Bits(text[0], 0x1F);
        least = 0x80;
    } else if ((first & 0xF0) == 0xE0) {
        count = 3;
        found = Bits(text[0], 0x0F);
        least = 0x800;
    } else if ((first & 0xF8) == 0xF0) {
        count = 4;
        found = Bits(text[0], 0x07);
        least = 0x10000;
    } else
        return 0;
    if (length < count)
        return 0;
    for (i = 1; i < count; i++) {
        if (!Continues(text[i]))
            return 0;
        found = found << 6 | Bits(text[i], 0x3F);
    }
    if (found < least || found > 0x10FFFF
        || (found >= 0xD800 && found <= 0xDFFF))
        return 0;
    *character = found;
    return count;
}

i32 Utf8Encode(u32 character, char *to)
{
    if (character < 0x80) {
        to[0] = (char)character;
        return 1;
    }
    if (character < 0x800) {
        to[0] = (char)(0xC0 | character >> 6);
        to[1] = (char)(0x80 | (character & 0x3F));
        return 2;
    }
    if (character < 0x10000) {
        to[0] = (char)(0xE0 | character >> 12);
        to[1] = (char)(0x80 | (character >> 6 & 0x3F));
        to[2] = (char)(0x80 | (character & 0x3F));
        return 3;
    }
    to[0] = (char)(0xF0 | character >> 18);
    to[1] = (char)(0x80 | (character >> 12 & 0x3F));
    to[2] = (char)(0x80 | (character >> 6 & 0x3F));
    to[3] = (char)(0x80 | (character & 0x3F));
    return 4;
}

u32 UnicodeSmall(u32 character)
{
    if ((character >= 'A' && character <= 'Z')
        || (character >= 0x00C0 && character <= 0x00DE
            && character != 0x00D7)
        || (character >= 0x0391 && character <= 0x03A9
            && character != 0x03A2))
        return character + 0x20;
    return character;
}

i32 CodePageToUtf8(const u8 *from, i32 length, bool small, char *to,
                   i32 room)
{
    u32 character;
    char bytes[4];
    i32 written = 0;
    i32 count;
    i32 i;

    for (i = 0; i < length; i++) {
        character = CodePageToUnicode(from[i]);
        count = Utf8Encode(small ? UnicodeSmall(character) : character,
                           bytes);
        if (written + count >= room)
            return -1;
        memcpy(to + written, bytes, (size_t)count);
        written += count;
    }
    if (written >= room)
        return -1;
    to[written] = '\0';
    return written;
}
