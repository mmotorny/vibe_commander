/*
 * An extension file or a menu file in memory: its entries, and the entry
 * for a file's extension.
 *
 * Translated from VCSUB2.INC.
 *
 * Such a file is text. A line that begins an entry has something other
 * than a space, a tab or an apostrophe first; the lines that follow it and
 * begin with a space or a tab belong to it, a line that begins with an
 * apostrophe is a comment, and empty lines count for nothing. In an
 * extension file an entry begins with up to three characters of an
 * extension, where ? stands for any character and * for any rest, and a
 * colon, behind which is the command. The program puts the byte 1Ah
 * behind what it has read of the file, and the file ends there.
 */
#include "vc.h"

#define EOF_CHAR 0x1A

/* A line ends with a carriage return and a line feed, in either order. */
#define IS_CRLF(pair) ((pair) == 0x0D0A || (pair) == 0x0A0D)

/* Where FndItem stopped, the offset in the low word, and 1 in the high
   one if that is the end of the file. */
#define ITEM(end, file) ((u32)(end) << 16 | (u16)(u32)(file))

/* Go to the entry at `file` or the next one behind it; with `rest`, to
   the next one behind the line that `file` is in. At the end of the file
   the place returned is that of the byte 1Ah if an entry was looked for
   there, and one behind it if a line was being skipped. */
static u32 Item(const u8 __far *file, u16 rest)
{
    u16 pair, c;

    for (;;) {
        if (!rest) {
            pair = *(const u16 __far *)file;
            c = pair & 0xFF;
            if (c != ' ' && c != '\t' && c != '\'' && !IS_CRLF(pair))
                return ITEM(c == EOF_CHAR, file);
        }
        rest = 0;
        for (;;) {                      /* FndIt1: to the end of the line */
            pair = *(const u16 __far *)file;
            if (IS_CRLF(pair)) {
                file += 2;
                break;
            }
            file++;
            if ((pair & 0xFF) == EOF_CHAR)
                return ITEM(1, file);
        }
    }
}

/* The entry at `file`, or the first one behind it: see Item. */
u32 FndItem(const u8 __far *file)
{
    return Item(file, 0);
}

/* Find the entry of the extension file in MenuBuf for the file `name`,
   and return where its command begins, behind the colon, as an offset in
   the main data; 0 if the file has no entry for it. The extension is
   left at TmpErr, three characters, with spaces for those it lacks. */
u16 FindExt(const u8 __far *name)
{
    const u8 *file;
    const u8 *wanted;
    u16 count, pair;
    u32 item;
    u8 c;

    do
        c = *name++;
    while (c != '.' && c != 0);
    if (c == 0)
        name--;
    for (count = 0; count < 3; count++) {
        c = *name;
        if (c == 0)
            c = ' ';
        else
            name++;
        md.TmpErr[count] = c;
    }
    item = FndItem(md.MenuBuf);
    for (;;) {
        if ((u16)(item >> 16) != 0)
            return 0;
        file = (const u8 *)(u16)item;
        wanted = md.TmpErr;             /* FnEx05 */
        count = 3;
        for (;;) {
            c = *file++;
            if (c == ':' || c == '\t') {
                /* Fewer characters than three: the rest of the file's
                   extension has to be spaces. */
                if (c == ':')
                    file--;
                while (count != 0 && *wanted == ' ') {
                    wanted++;
                    count--;
                }
                if (count != 0)
                    goto FnEx12;
                break;
            }
            if (c == '*')
                break;
            if (c != '?' && (u8)LowCase(c) != *wanted)
                goto FnEx12;
            wanted++;
            if (--count == 0)
                break;
        }
        for (;;) {                      /* FnEx09: it is this entry */
            pair = *(const u16 *)file;
            if (IS_CRLF(pair)) {
                /* No colon in its line: on with the next entry. */
                item = FndItem(file + 2);
                break;
            }
            file++;
            if ((pair & 0xFF) == EOF_CHAR)
                return 0;
            if ((pair & 0xFF) == ':')
                return (u16)file;
        }
        continue;
FnEx12:
        item = Item(file, 1);
    }
}
