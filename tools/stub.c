/*
 * stub - switch off the banner of DOS/32A in a program that has it for
 * its stub.
 *
 *   stub VC.EXE
 *
 * The program of 32 bits begins with the extender, DOS/32A, which the
 * linker copies in front of it from the Open Watcom that the build
 * fetches (binw/dos32a.exe). As it comes, the extender writes two lines
 * of its name and its copyright when it starts, and the program's first
 * screen would have them. This clears the one bit that makes it do so,
 * in the file that the linker wrote.
 *
 * Where the bit is. The extender keeps its settings in its own file,
 * behind its header, in a block that begins with the letters ID32:
 *
 *   49 44 33 32  3F 40 02 10 00 01  08 08  20 00  20 00
 *   FF FF FF FF  3F  09  00 02  0C 09  00 00
 *
 * The banner is bit 3 of the byte 15h of that block, the 09h. It was
 * found on the bench of the tests, not read anywhere: the pinned Open
 * Watcom has no description of the block, so each bit of it was cleared
 * in turn in a toy and the toy run (TODO.md, "The portable C in 32 bits:
 * what a trial on the bench showed", and harness/trial32/). With this
 * bit clear DOS/32A writes nothing as it starts and leaves the cursor
 * where it is; nothing else of what it does was seen to change.
 *
 * What the bit is was read since, for another matter, in the code of
 * binw/dos32a.exe itself, where it takes the options of its environment
 * variable, DOS32A, whose names are in the file: it is the bit that the
 * option NOC clears, the banner and nothing else. The other bit that is
 * set in that byte, bit 0, is whether that variable is read at all. The
 * byte in front, 14h, has what the extender does when the program ends,
 * the interrupts of real mode put back and a line of warning for each
 * that was not as it was, and where the program is loaded. TODO.md has
 * both bytes bit by bit, with how they were read: "L, a program that
 * has stayed in the memory". Nothing of the block is changed but the
 * banner's bit.
 *
 * The bit was found for the DOS/32A of that download, which answers
 * 090Ch when a program asks it for its version (INT 31h with EEFFh):
 * the two bytes at 18h of the block are 0Ch and 09h, and are taken for
 * that number here. A stub with another number there is another
 * extender, of which nothing is known: this stops then, and the build
 * with it, instead of clearing a bit that may mean something else.
 *
 * The linker writes the stub's header anew, 32 bytes longer, so the
 * block is not where it is in dos32a.exe: it is looked for, in the first
 * 512 bytes of the file, and has to be there once.
 */
#include <stdio.h>
#include <string.h>

#define SEARCHED 512                    /* bytes of the file looked at */
#define BLOCK    0x1A                   /* bytes of the block gone by */
#define FLAGS    0x15                   /* the byte with the banner's bit */
#define BANNER   0x08
#define VERSION  0x18                   /* the extender's number, low byte
                                           first: 090Ch */
#define KNOWN_LOW  0x0C
#define KNOWN_HIGH 0x09

int main(int argc, char **argv)
{
    static const char letters[4] = { 'I', 'D', '3', '2' };
    unsigned char head[SEARCHED];
    size_t length, at, found = 0, count = 0;
    FILE *file;

    if (argc != 2) {
        fprintf(stderr, "usage: stub PROGRAM\n");
        return 2;
    }
    file = fopen(argv[1], "r+b");
    if (!file) {
        perror(argv[1]);
        return 1;
    }
    length = fread(head, 1, sizeof head, file);
    for (at = 0; at + BLOCK <= length; at++)
        if (!memcmp(head + at, letters, sizeof letters)) {
            found = at;
            count++;
        }
    if (count != 1) {
        fprintf(stderr, "%s: the settings of DOS/32A, which begin with "
                "ID32, are in its first %d bytes %d times, not once\n",
                argv[1], SEARCHED, (int)count);
        return 1;
    }
    if (head[found + VERSION] != KNOWN_LOW
            || head[found + VERSION + 1] != KNOWN_HIGH) {
        fprintf(stderr, "%s: its stub is DOS/32A %02X%02Xh, and the bit of "
                "the banner is known for %02X%02Xh alone: see tools/stub.c\n",
                argv[1], head[found + VERSION + 1], head[found + VERSION],
                KNOWN_HIGH, KNOWN_LOW);
        return 1;
    }
    head[found + FLAGS] &= (unsigned char)~BANNER;
    if (fseek(file, (long)(found + FLAGS), SEEK_SET)
            || fputc(head[found + FLAGS], file) == EOF || fclose(file)) {
        perror(argv[1]);
        return 1;
    }
    return 0;
}
