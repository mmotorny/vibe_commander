/*
 * mkexe - turn the linker's VC.EXE into the one that is shipped.
 *
 * Volkov Commander was written as a .COM program: it expects to start at
 * offset 100h of the segment that holds its PSP, and it moves and reloads
 * most of itself at run time. Compiled, it no longer fits the 64K of a
 * .COM file, and its code no longer sits in one segment. This tool makes
 * an .EXE that still suits it:
 *
 *   - The program is loaded at PSP:100h, like a .COM file, and entered
 *     with CS on the PSP, by giving the header a code segment of -10h:
 *     at the address that the linker has for the start, which is that of
 *     a function of the part that stays in the memory (Alloc0 in
 *     src/c/resident.c).
 *   - Its stack is its own from the first instruction: the header names
 *     the PSP for the stack segment too, and the end of the part of the
 *     program that stays in the memory while a command runs, which is
 *     where that part has its stack.
 *   - The file is the program as it lies in the memory: that part first,
 *     behind the PSP, with its data and its stack, which are nothing but
 *     room, then a paragraph for the header that DOS keeps in front of a
 *     block of memory, then the part that goes while a command runs. So
 *     that part is loaded where it runs, and the program has only to make
 *     a block of it. The linker's file begins with room for the PSP, 256
 *     bytes of the code of the part that stays (Psp of src/c/resident.c),
 *     which are taken off here: DOS has the PSP there.
 *   - DOS relocates nothing. A program that moves itself has to do that on
 *     its own, so the linker's relocation table is appended to the program,
 *     which applies it to every copy of itself that it loads (Reloc in
 *     src/c/resident.c). The relocations of the part that stays come
 *     behind the others in the table: that part does not move, but it
 *     takes a check sum of the file as it was loaded, to know the file
 *     again when it loads it after a command, and so applies its own
 *     relocations itself too, once, when it has the sum.
 *   - The numbers the program needs for this are written into words of
 *     the part that stays, which src/c/resident.c has for them: each is
 *     found by its name in the linker's map. So are the things that the
 *     numbers say: where the part that stays has nothing but zeros in the
 *     file, from Stays on; its stack, DataSav, which is all of the
 *     segment R_BSS, the last of that part; and where the part that goes
 *     begins. That is behind the first whole paragraph of TrnBeg, 32
 *     bytes of nothing which src/c/transit_t.c has and the linker puts
 *     behind the stack, on no paragraph of their own: the header of the
 *     block is in that paragraph.
 *
 * What the linker's map has to show for that, which the build arranges by
 * the compiler's options and the linker's command and nothing in the C
 * language promises (see docs/translation.md, "What rests on Open Watcom"): the
 * code and the data of the part that stays are one group, R_GROUP, which
 * begins at the PSP; the room of the PSP is the first thing of it; the
 * stack is alone in the last segment of it; and no code of the part that
 * goes is within a page of the stack. The build stops here if one of
 * these does not hold.
 *
 * It also checks the object files of the compiled code, if they are named.
 * One of the part that goes may hold code, or data of the compiler's
 * group, and not both. The program keeps its data in a segment that it
 * allocates when it starts, and has DS there; the constants that its code
 * reads by their names are in the code segment. Static data of the
 * compiler's making, a string literal for example, would be linked to
 * where the program does not look for it. The tables that the program
 * names by their offsets, boxes and dialogs with their texts, are
 * constants of the compiler's own group, DGROUP: a file
 * of those has no code that could read them through DS (see
 * src/c/tables.h), and may have a variable for that reason, with a value,
 * so that it is in the file. One of the part that stays has that part's
 * data besides its code, in the group that the compiler makes for it,
 * R_GROUP, on which DS is there: the four segments that the option -nd=R
 * names, and no other.
 *
 * And it checks the linker's map: no segment of compiled code may reach
 * into the last 4K of its 64K. Between the end of a segment and its
 * beginning a near call or jump goes the short way, around the end of the
 * 64K, and the linker makes such calls: near ones out of the far calls
 * between two files of a segment. A processor does them. QEMU, on which
 * the tests run, does not where the call and the place it would go to
 * without wrapping are in one 4K page of the machine's memory: it leaves
 * the wrapping out, and the program goes on 64K off (gen_jmp_rel in QEMU's
 * target/i386/tcg/translate.c, as of 11.1). A segment starts on a
 * paragraph, so the end of its 64K is at most FF0h into a page, and such a
 * call has one end at F010h or beyond.
 *
 * And that no code of the part that goes lies within 4K of the stack,
 * which is the end of the part that stays: QEMU is many times slower at a
 * write to a page of the memory that has code which it has run, and a
 * push is a write. Constants stand between the two, the texts and the
 * tables of the C code.
 *
 * Usage: mkexe <linked.exe> <linked.map> <output.exe> [<compiled.obj>...]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    PSP_SIZE    = 0x100,
    HEADER_SIZE = 0x20,         /* of the output file */
    CODE_END    = 0xF000,       /* where compiled code ends, at the most */
    PAGE        = 0x1000,       /* of the machine's memory */

    /* Records of an object file (OMF). */
    LNAMES      = 0x96,         /* a list of names */
    SEGDEF      = 0x98          /* a segment: attributes, length, name */
};

/* The words of src/c/resident.c that are written here, by their names in
   the linker's map. All are in paragraphs from the PSP but the counts. */
#define IMAGE_END   "ImgEnd"    /* end of the program */
#define TABLE       "RelTab"    /* relocation table */
#define COUNT       "RelCnt"    /* number of relocations */
#define TRANSIT     "TrnPrW"    /* where the part that goes begins */
#define RESIDENT    "ResPrW"    /* where the part that stays ends, which is
                                   the top of its stack */
#define DATA        "DatPrW"    /* from where the part that stays is
                                   nothing but zeros in the file */
#define STAYING     "ResCnt"    /* number of relocations of that part,
                                   which follow the others in the table */

/* What the words say, by the names that the map has for it. */
#define PSP_AT      "Psp"       /* the room of the PSP, the first of all */
#define TRANSIT_AT  "TrnBeg"    /* the room of a block's header, behind
                                   which the part that goes begins */
#define DATA_AT     "Stays"     /* what the part that stays keeps */
#define STACK_AT    "DataSav"   /* its stack, */
#define STACK_IN    "R_BSS"     /* which is all of this segment, */
#define DATA_CLASS  "FAR_DATA"  /* of this class, */
#define DATA_GROUP  "R_GROUP"   /* in the group of that part's data */

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

/* Fail if the object file at `path` has code and data that the code
   would not find. The compiler names its code segments by class, here
   CCODE, and RCODE for the part of the program that stays in the memory.
   Of its own group it has constants in the segment CONST2 and variables
   that have a value in _DATA: for a file without code. The data of the
   part that stays is of the class FAR_DATA, in the four segments that
   the compiler makes of its own with the option -nd=R: for a file whose
   code is of that part. */
static void check_object(const char *path)
{
    long size, at = 0;
    unsigned char *object = read_file(path, &size);
    char names[64][32];
    int count = 1;                      /* names are numbered from 1 */
    int code = 0, stays = 0, constants = 0, kept = 0;

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
            if (!length && !(attributes & 2))
                ;
            else if (!strcmp(names[class], "CCODE"))
                code = 1;
            else if (!strcmp(names[class], "RCODE"))
                code = stays = 1;
            else if (!strcmp(names[name], "CONST2")
                    || !strcmp(names[name], "_DATA"))
                constants = 1;
            else if (!strcmp(names[class], DATA_CLASS)
                    && (!strcmp(names[name], "RCONST")
                        || !strcmp(names[name], "RCONST2")
                        || !strcmp(names[name], "R_DATA")
                        || !strcmp(names[name], STACK_IN)))
                kept = 1;
            else {
                fprintf(stderr, "mkexe: %s has %u bytes of static data in "
                        "segment %s; compiled code may hold none, not even "
                        "a string literal\n", path, length, names[name]);
                exit(1);
            }
        }
        at = end + 1;
    }
    if (code && constants) {
        fprintf(stderr, "mkexe: %s has code and data of the compiler's "
                "group; the code would read that through DS, which is on "
                "the main data, or on the part that stays. A constant "
                "that only code reads is declared CODE; one that is named "
                "by its offset goes to a file of constants (see "
                "src/c/tables.h)\n", path);
        exit(1);
    }
    if (kept && !stays) {
        fprintf(stderr, "mkexe: %s has data of the part that stays in "
                "the memory, and no code of that part\n", path);
        exit(1);
    }
    free(object);
}

/* The linker's map, as text, and a line of it at a time. */
static char *map;

static const char *next_line(const char *line, char *copy, size_t room)
{
    size_t length = strcspn(line, "\n");

    if (!*line)
        return NULL;
    memcpy(copy, line, length < room ? length : room - 1);
    copy[length < room ? length : room - 1] = 0;
    return line + length + (line[length] == '\n');
}

/* Where the map has the symbol `name`, counted from the PSP. A line of
   its list of symbols is an address, a mark or none, and the name. */
static unsigned long symbol(const char *name)
{
    const char *at = map;
    char line[256], found[64];
    unsigned segment, offset;
    unsigned long address = 0;
    int count = 0;

    while ((at = next_line(at, line, sizeof line)) != NULL)
        if (sscanf(line, "%4x:%4x%*[*+s] %63s", &segment, &offset, found) == 3
                || sscanf(line, "%4x:%4x %63s", &segment, &offset, found)
                    == 3)
            if (!strcmp(found, name)) {
                address = segment * 16UL + offset;
                count++;
            }
    if (count != 1) {
        fprintf(stderr, "mkexe: the map has %s %d times, and the program "
                "is laid out by where that is\n", name, count);
        exit(1);
    }
    return address;
}

/* Fail if the map has another symbol than `name` from `begin` to `end`. */
static void alone(const char *name, unsigned long begin, unsigned long end)
{
    const char *at = map;
    char line[256], found[64];
    unsigned segment, offset;

    while ((at = next_line(at, line, sizeof line)) != NULL)
        if ((sscanf(line, "%4x:%4x%*[*+s] %63s", &segment, &offset, found)
                    == 3
                || sscanf(line, "%4x:%4x %63s", &segment, &offset, found)
                    == 3)
                && segment * 16UL + offset >= begin
                && segment * 16UL + offset < end && strcmp(found, name)) {
            fprintf(stderr, "mkexe: %s is in the segment of the stack, "
                    "which is to be the end of the part that stays: a "
                    "variable of that part needs a value, if only 0, to "
                    "be of another segment\n", found);
            exit(1);
        }
}

/* Where the map has the segment `wanted` of the class `of` begin, and
   how long it is, in `length`. With `wanted` a null pointer: where the
   first segment of that class begins that is not empty. A line of the
   list of segments is name, class, group, address and size. */
static unsigned long segment_at(const char *wanted, const char *of,
                                unsigned long *length)
{
    const char *at = map;
    char line[256], name[32], class[32], group[32];
    unsigned segment, offset;
    unsigned long size, begin = 0xFFFFFFFFUL;

    while ((at = next_line(at, line, sizeof line)) != NULL)
        if (sscanf(line, "%31s %31s %31s %4x:%4x %8lx", name, class, group,
                   &segment, &offset, &size) == 6
                && !strcmp(class, of)
                && (wanted ? !strcmp(name, wanted) : size != 0)
                && segment * 16UL + offset < begin) {
            begin = segment * 16UL + offset;
            *length = size;
        }
    if (begin == 0xFFFFFFFFUL) {
        fprintf(stderr, "mkexe: the map has no segment %s of the class "
                "%s\n", wanted ? wanted : "with contents", of);
        exit(1);
    }
    return begin;
}

/* Where the map has the group `name` begin. A line of the list of groups
   is name, address and size. */
static unsigned long group_at(const char *name)
{
    const char *at = map;
    char line[256], found[32];
    unsigned segment, offset;
    unsigned long size;

    while ((at = next_line(at, line, sizeof line)) != NULL)
        if (sscanf(line, "%31s %4x:%4x %8lx", found, &segment, &offset,
                   &size) == 4 && !strcmp(found, name))
            return segment * 16UL + offset;
    fprintf(stderr, "mkexe: the map has no group %s\n", name);
    exit(1);
}

/* Fail if the linker's map has a segment of compiled code that ends
   beyond CODE_END, counted from where its segment register points.
   Returns where the first code behind `from` begins, counted from the
   PSP. */
static unsigned long check_map(unsigned long from)
{
    const char *at = map;
    char line[256], name[32], class[32], group[32];
    unsigned segment, offset;
    unsigned long size, first = 0xFFFFFFFFUL;
    int found = 0;

    /* A line of the list of segments: name, class, group, address, size. */
    while ((at = next_line(at, line, sizeof line)) != NULL) {
        if (sscanf(line, "%31s %31s %31s %4x:%4x %8lx", name, class, group,
                   &segment, &offset, &size) != 6)
            continue;
        if (size && segment * 16UL + offset >= from
                && segment * 16UL + offset < first
                && !strcmp(class, "CCODE"))
            first = segment * 16UL + offset;
        if (strcmp(class, "CCODE"))
            continue;
        found = 1;
        if (offset + size > CODE_END) {
            fprintf(stderr, "mkexe: segment %s ends at %lXh, and compiled "
                    "code may not reach beyond %Xh: a call from there to "
                    "the beginning of the segment goes around the end of "
                    "its 64K, which the machine of the tests gets wrong. "
                    "Have a file compiled into another segment: see "
                    "add_c_tree in CMakeLists.txt\n",
                    name, offset + size, (unsigned)CODE_END);
            exit(1);
        }
    }
    if (!found)
        fail("the map lists no segment of compiled code");
    return first;
}

int main(int argc, char **argv)
{
    static const char *const words[] = {
        IMAGE_END, TABLE, COUNT, TRANSIT, RESIDENT, DATA, STAYING
    };
    FILE *f;
    unsigned char *in, *image, header[HEADER_SIZE] = {0};
    long in_size, image_size, table_at, total, i, map_size;
    unsigned long transit, resident, data, stack, kept, code, length;
    unsigned relocations, header_size;
    unsigned moving = 0, staying = 0, start;
    const unsigned char *entry;

    if (argc < 4)
        fail("usage: mkexe <linked.exe> <linked.map> <output.exe> "
             "[<compiled.obj>...]");
    /* What an earlier build left is not the program of one that fails. */
    remove(argv[3]);
    for (i = 4; i < argc; i++)
        check_object(argv[i]);
    in = read_file(argv[1], &in_size);
    map = (char *)read_file(argv[2], &map_size);
    map[map_size] = 0;

    if (in_size < 0x1C || get16(in) != 0x5A4D)
        fail("the input is not an MZ executable");
    relocations = get16(in + 6);
    header_size = get16(in + 8) * 16u;
    entry = in + get16(in + 0x18);
    start = get16(in + 0x14);
    if (get16(in + 0x16) != 0)
        fail("the program does not start in the segment of the PSP");
    if ((long)header_size + PSP_SIZE > in_size
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

    /* The part that stays: its code, then its data, which begins with
       what has contents in the file and ends with the stack. CS, DS and
       SS are on the PSP there, and it is read by offsets from its
       group. */
    if (group_at(DATA_GROUP) != 0)
        fail("the resident part is not in a group that begins at the "
             "PSP");
    if (symbol(PSP_AT) != 0)
        fail("the room of the PSP is not the first thing of the program");
    code = segment_at(NULL, "RCODE", &length);
    if (start < code || start >= code + length)
        fail("the program does not start in the code of the resident "
             "part");
    kept = segment_at(NULL, DATA_CLASS, &length);
    data = symbol(DATA_AT);
    stack = segment_at(STACK_IN, DATA_CLASS, &length);
    resident = stack + length;
    if (symbol(STACK_AT) != stack)
        fail("the stack is not where its segment begins");
    alone(STACK_AT, stack, resident);
    if (kept < code || data < kept || data > stack)
        fail("the resident part is not its code, its data and its stack, "
             "in this order");
    for (i = 0; i < (long)(sizeof words / sizeof *words); i++)
        if (symbol(words[i]) < kept || symbol(words[i]) + 2 > data) {
            fprintf(stderr, "mkexe: %s is not in front of %s, where the "
                    "resident part has contents in the file\n", words[i],
                    DATA_AT);
            exit(1);
        }

    /* The part that goes begins on a paragraph, one paragraph behind the
       end of the part that stays: that one is for the header of its block
       of memory. The room for it is behind the stack, 32 bytes that
       begin on no paragraph, and the first whole paragraph of them is
       the one. */
    transit = (symbol(TRANSIT_AT) + 15) / 16 * 16 + 16;
    if (symbol(TRANSIT_AT) < resident
            || transit / 16 != (resident + 15) / 16 + 1
            || transit + 2 > (unsigned long)image_size)
        fail("the transit part does not begin a paragraph behind the "
             "resident part");
    /* The program takes its check sum of all but what the part that
       stays keeps, which is in use by then, the stack in it: it has to be
       nothing in the file. */
    for (i = data; i < (long)transit; i++)
        if (image[i])
            fail("the data of the resident part has contents");
    if (check_map(transit) < resident + (unsigned long)PAGE)
        fail("the transit part has code within 4K of the stack, which "
             "makes the machine of the tests many times slower: have "
             "constants stand in front of the code");

    /* A relocation is a word of the program that holds a segment, counted
       from the PSP. The part that moves is relocated at its new place,
       and what its words name moves with it. So they cannot name the
       part that stays, which is where it was: its segment is the PSP's,
       0 here, and the program has that from SS (ResSeg of
       src/c/misc.c). */
    for (i = 0; i < relocations; i++) {
        unsigned offset = get16(entry + i * 4);
        unsigned segment = get16(entry + i * 4 + 2);
        long address = segment * 16L + offset;
        if (address + 2 > image_size)
            fail("a relocation lies outside the program");
        if (address < (long)transit)
            continue;
        if (get16(image + address) == 0)
            fail("the transit part names the segment of the resident "
                 "part");
        put16(image + table_at + moving * 4, offset);
        put16(image + table_at + moving * 4 + 2, segment);
        moving++;
    }
    /* The part that stays is relocated where it is, once, and the table
       has each word of it by its offset from the PSP: they are in its
       code. That holds for a word that names that part itself, whose
       segment is the PSP's, 0 here. One that names a segment of the part
       that goes is left as the linker has it, in paragraphs from the PSP,
       and is not in the table: that part is somewhere else after every
       command, and the code adds where (TRN_SEG of src/c/resident.c). */
    for (i = 0; i < relocations; i++) {
        long address = get16(entry + i * 4 + 2) * 16L + get16(entry + i * 4);
        if (address >= (long)transit)
            continue;
        if (address + 2 > (long)kept)
            fail("a relocation lies in the data of the resident part");
        if (get16(image + address) != 0) {
            if (get16(image + address) < resident / 16)
                fail("the resident part names a segment within itself "
                     "that is not the PSP's");
            continue;
        }
        put16(image + table_at + (moving + staying) * 4, address);
        put16(image + table_at + (moving + staying) * 4 + 2, 0);
        staying++;
    }
    if (relocations * 4L > 0xFFFF)
        fail("the relocation table is longer than 64K");
    put16(image + symbol(IMAGE_END), total / 16);
    put16(image + symbol(TABLE), table_at / 16);
    put16(image + symbol(COUNT), moving);
    put16(image + symbol(STAYING), staying);
    put16(image + symbol(TRANSIT), transit / 16);
    put16(image + symbol(RESIDENT), (resident + 15) / 16);
    put16(image + symbol(DATA), (data + 15) / 16);

    /* DOS loads the file's contents 10h paragraphs above the PSP and adds
       that segment to the header's CS and SS. With CS = -10h the program
       starts in the segment of the PSP, at the offset that the linker has
       for it, and with SS = -10h its stack is in the same segment: it
       ends where the resident part does. The file holds all the memory
       that the program needs to start. */
    total -= PSP_SIZE;
    put16(header + 0x00, 0x5A4D);
    put16(header + 0x02, (HEADER_SIZE + total) % 512);
    put16(header + 0x04, (HEADER_SIZE + total + 511) / 512);
    put16(header + 0x08, HEADER_SIZE / 16);
    put16(header + 0x0A, 0);                            /* minimum memory */
    put16(header + 0x0C, 0xFFFF);                       /* maximum memory */
    put16(header + 0x0E, 0x10000UL - PSP_SIZE / 16);    /* SS */
    put16(header + 0x10, resident);                     /* SP */
    put16(header + 0x14, start);                        /* IP */
    put16(header + 0x16, 0x10000UL - PSP_SIZE / 16);    /* CS */
    put16(header + 0x18, 0x1C);

    if (!(f = fopen(argv[3], "wb")))
        fail("cannot create the output file");
    if (fwrite(header, 1, HEADER_SIZE, f) != HEADER_SIZE
            || fwrite(image + PSP_SIZE, 1, total, f) != (size_t)total
            || fclose(f))
        fail("cannot write the output file");
    return 0;
}
