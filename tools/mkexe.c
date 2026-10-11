/*
 * mkexe - turn the linker's VC.EXE into the one that is shipped.
 *
 * Volkov Commander is written as a .COM program: it expects to start at
 * offset 100h of the segment that holds its PSP, and it moves and reloads
 * most of itself at run time. With compiled code linked in it no longer
 * fits the 64K of a .COM file, and its code no longer sits in one segment.
 * This tool makes an .EXE that still suits it:
 *
 *   - The program is loaded at PSP:100h and entered there, like a .COM
 *     file, by giving the header a code segment of -10h.
 *   - DOS relocates nothing. A program that moves itself has to do that on
 *     its own, so the linker's relocation table is appended to the program,
 *     which applies it to every copy of itself that it loads (Reloc in
 *     port/asm/VC.ASM).
 *   - The numbers the program needs for this are written into the block
 *     that port/asm/VC.ASM reserves at 103h.
 *
 * It also checks the object files of the compiled code, if they are named:
 * they may hold code only. The program keeps its data in a segment that it
 * allocates when it starts, and its constants in the code segment; static
 * data of the compiler's making, a string literal for example, would be
 * linked to where the program does not look for it.
 *
 * Usage: mkexe <linked.exe> <output.exe> [<compiled.obj>...]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    PSP_SIZE    = 0x100,
    HEADER_SIZE = 0x20,         /* of the output file */
    STACK_SIZE  = 0x200,        /* until the program sets up its own */

    /* The block at 103h. The first three words are written here. */
    INFO        = 0x103,
    IMAGE_END   = INFO + 0,     /* end of the program, paragraphs from PSP */
    TABLE       = INFO + 2,     /* relocation table, paragraphs from PSP */
    COUNT       = INFO + 4,     /* number of relocations */
    TRANSIT     = INFO + 6,     /* in: where the part that moves begins */
    INFO_END    = INFO + 8,

    /* Records of an object file (OMF). */
    LNAMES      = 0x96,         /* a list of names */
    SEGDEF      = 0x98          /* a segment: attributes, length, name */
};

static void fail(const char *message)
{
    fprintf(stderr, "mkexe: %s\n", message);
    exit(1);
}

static unsigned get16(const unsigned char *p)
{
    return p[0] | p[1] << 8;
}

static void put16(unsigned char *p, unsigned long value)
{
    p[0] = value & 0xFF;
    p[1] = value >> 8 & 0xFF;
}

static unsigned char *read_file(const char *path, long *size)
{
    FILE *f = fopen(path, "rb");
    unsigned char *contents;

    if (!f) {
        fprintf(stderr, "mkexe: cannot open %s\n", path);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    rewind(f);
    contents = malloc(*size + 1);
    if (!contents || fread(contents, 1, *size, f) != (size_t)*size)
        fail("cannot read a file");
    fclose(f);
    return contents;
}

/* Fail if the object file at `path` has anything in a segment that is not
   code. The compiler names its code segments by class, here CCODE. */
static void check_object(const char *path)
{
    long size, at = 0;
    unsigned char *object = read_file(path, &size);
    char names[64][32];
    int count = 1;                      /* names are numbered from 1 */

    while (at + 3 <= size) {
        int type = object[at];
        long end = at + 3 + get16(object + at + 1) - 1;    /* the check sum */
        const unsigned char *p = object + at + 3;

        if (end > size)
            break;
        if (type == LNAMES) {
            while (p < object + end && count < 64) {
                int length = *p++ < 31 ? p[-1] : 31;
                memcpy(names[count], p, length);
                names[count++][length] = 0;
                p += p[-1];
            }
        } else if (type == SEGDEF) {
            unsigned attributes = *p++, length, name, class;
            if (attributes >> 5 == 0)
                p += 3;                 /* an absolute segment's address */
            length = get16(p);
            name = p[2];
            class = p[3];
            if (name >= (unsigned)count || class >= (unsigned)count)
                fail("an object file is not as expected");
            if ((length || attributes & 2) && strcmp(names[class], "CCODE")) {
                fprintf(stderr, "mkexe: %s has %u bytes of static data in "
                        "segment %s; compiled code may hold none, not even "
                        "a string literal\n", path, length, names[name]);
                exit(1);
            }
        }
        at = end + 1;
    }
    free(object);
}

int main(int argc, char **argv)
{
    FILE *f;
    unsigned char *in, *image, header[HEADER_SIZE] = {0};
    long in_size, image_size, table_at, total, i;
    unsigned relocations, header_size;
    const unsigned char *entry;

    if (argc < 3)
        fail("usage: mkexe <linked.exe> <output.exe> [<compiled.obj>...]");
    for (i = 3; i < argc; i++)
        check_object(argv[i]);
    in = read_file(argv[1], &in_size);

    if (in_size < 0x1C || get16(in) != 0x5A4D)
        fail("the input is not an MZ executable");
    relocations = get16(in + 6);
    header_size = get16(in + 8) * 16u;
    entry = in + get16(in + 0x18);
    if (get16(in + 0x14) != PSP_SIZE || get16(in + 0x16) != 0)
        fail("the program does not start at 0000:0100");
    if ((long)header_size + INFO_END > in_size
            || entry + relocations * 4 > in + header_size)
        fail("the input is truncated");

    /* The linker's image starts where the PSP will be: 100h bytes of
       nothing, then the program. Round it up to whole paragraphs, since the
       program measures, sums and moves itself in paragraphs, and make room
       for the relocation table behind it. */
    image_size = in_size - header_size;
    table_at = (image_size + 15) / 16 * 16;
    total = (table_at + relocations * 4L + 15) / 16 * 16;
    image = calloc(total, 1);
    if (!image)
        fail("out of memory");
    memcpy(image, in + header_size, image_size);
    for (i = 0; i < PSP_SIZE; i++)
        if (image[i])
            fail("the program has contents below 100h");

    /* Only the part of the program that moves is relocated, at its new
       place; nothing below it may need relocating. */
    for (i = 0; i < relocations; i++) {
        unsigned offset = get16(entry + i * 4);
        unsigned segment = get16(entry + i * 4 + 2);
        long address = segment * 16L + offset;
        if (address < get16(image + TRANSIT) || address + 2 > image_size)
            fail("a relocation lies outside the transit part");
        put16(image + table_at + i * 4, offset);
        put16(image + table_at + i * 4 + 2, segment);
    }
    put16(image + IMAGE_END, total / 16);
    put16(image + TABLE, table_at / 16);
    put16(image + COUNT, relocations);

    /* DOS loads the file's contents 10h paragraphs above the PSP and adds
       that segment to the header's CS and SS. With CS = -10h and IP = 100h
       the program starts at PSP:100h; the stack goes just above it. */
    total -= PSP_SIZE;
    put16(header + 0x00, 0x5A4D);
    put16(header + 0x02, (HEADER_SIZE + total) % 512);
    put16(header + 0x04, (HEADER_SIZE + total + 511) / 512);
    put16(header + 0x08, HEADER_SIZE / 16);
    put16(header + 0x0A, STACK_SIZE / 16);              /* minimum memory */
    put16(header + 0x0C, 0xFFFF);                       /* maximum memory */
    put16(header + 0x0E, total / 16);                   /* SS */
    put16(header + 0x10, STACK_SIZE);                   /* SP */
    put16(header + 0x14, PSP_SIZE);                     /* IP */
    put16(header + 0x16, 0x10000UL - PSP_SIZE / 16);    /* CS */
    put16(header + 0x18, 0x1C);

    if (!(f = fopen(argv[2], "wb")))
        fail("cannot create the output file");
    if (fwrite(header, 1, HEADER_SIZE, f) != HEADER_SIZE
            || fwrite(image + PSP_SIZE, 1, total, f) != (size_t)total
            || fclose(f))
        fail("cannot write the output file");
    return 0;
}
