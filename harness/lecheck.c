/* A flat program of 32 bits that harness/lecheck.py builds, to hold the
   harness's reader of such files, harness/le.py, to: it has one of each
   thing that the reader has to get right, where the test can say from
   this text what the reader must find.

   The test builds it and does not run it. Run on the bench, it writes
   what it finds of itself in the memory onto the screen, from row 10
   on, which is what the reader says is there: see the top of
   lecheck.py. It is compiled twice, the second time with SECOND
   defined, for a second file with a static variable of the same name. */
typedef unsigned char u8;
typedef unsigned long u32;

#ifdef SECOND

static u32 Kept = 0x5EC0D5ECUL;

u32 Other(void) { return Kept; }
void SetOther(u32 to) { Kept = to; }

#else

static u32 Kept = 0xF125F125UL;

/* What the file does not hold: it has no value, and the linker's order
   has it last. */
#define NONE 70000
u8 None[NONE];

/* A constant, and code that names it: B8h, its address, C3h. */
const u8 Text_S[] = "the reader of linear executables";

const u8 *Gives(void) { return Text_S; }

/* 4,096 addresses of five bytes each with the number behind them, one
   behind the other: so they lie across four or five ends of a page of
   the file, each at another byte of an entry, and three of the addresses lie
   across one, cut behind their first, second and third byte. Entry n
   names None + 17 * n: from below 64K of the data to beyond, where the
   linker writes a relocation longer. */
#define ENTRIES 4096
#define STEP 17
#pragma pack(push, 1)
struct Entry { const u8 *names; u8 number; };
#pragma pack(pop)
#define E1(n) { None + STEP * (n), (u8)(n) }
#define E4(n) E1(n), E1((n) + 1), E1((n) + 2), E1((n) + 3)
#define E16(n) E4(n), E4((n) + 4), E4((n) + 8), E4((n) + 12)
#define E64(n) E16(n), E16((n) + 16), E16((n) + 32), E16((n) + 48)
#define E256(n) E64(n), E64((n) + 64), E64((n) + 128), E64((n) + 192)
struct Entry Table[ENTRIES] = {
    E256(0), E256(256), E256(512), E256(768),
    E256(1024), E256(1280), E256(1536), E256(1792),
    E256(2048), E256(2304), E256(2560), E256(2816),
    E256(3072), E256(3328), E256(3584), E256(3840)
};

/* An address in front of the object that has what it names, and one
   behind it: the megabyte that the portable C counts its segments in
   begins 64K in front of its main data. */
#define FRONT 0x10000
#define BEHIND 0x345678
const u8 *Outside[2] = { (const u8 *)Table - FRONT, None + NONE + BEHIND };

/* Data that names code. */
void Start(void);
typedef void Function(void);
Function *Calls[2] = { (Function *)Gives, Start };

/* Code that takes the segment of something: a selector, which has a
   relocation of its own kind. */
u32 Selector(void);
#pragma aux Selector = "mov ax,seg Table" value [eax] modify exact [eax];

u32 Loads(void) { return Selector(); }

/* The stack, which the linker wants: see harness/trial32/lib.c. */
#pragma data_seg("STACK", "STACK")
u8 Stack[16] = { 0 };
#pragma data_seg()

/* Onto the screen, where the bench has it, and no cursor moved. */
#define SCREEN ((u8 *)0xB8000)
#define ROW 10
static u32 Row = ROW, Column = 0;

static void Put(const char *text)
{
    while (*text) SCREEN[160 * Row + 2 * Column++] = (u8)*text++;
}

static void Hex(u32 value)
{
    u32 digit;

    for (digit = 0; digit < 8; digit++, value <<= 4)
        SCREEN[160 * Row + 2 * Column++] =
            (u8)"0123456789ABCDEF"[value >> 28];
}

static void Line(void) { Row++; Column = 0; }

void Leave(void);
#pragma aux Leave = "mov eax,4C00h" "int 21h" aborts;

/* Where the extender enters. */
void Start(void)
{
    u32 n, wrong = 0, sum = 0;

    Put("code "); Hex((u32)Gives); Put(" text "); Hex((u32)Text_S);
    Put(" table "); Hex((u32)Table); Put(" none "); Hex((u32)None);
    Line();
    for (n = 0; n < ENTRIES; n++) {
        if (Table[n].names != None + STEP * n || Table[n].number != (u8)n)
            wrong++;
        sum += (u32)Table[n].names;
    }
    Put("entries that do not name what they should: "); Hex(wrong);
    Line();
    Put("their sum: "); Hex(sum); Line();
    Put("outside "); Hex((u32)Outside[0]); Put(" "); Hex((u32)Outside[1]);
    if (Outside[0] != (const u8 *)Table - FRONT
        || Outside[1] != None + NONE + BEHIND) Put(", wrong");
    Line();
    Put("calls "); Hex((u32)Calls[0]); Put(" "); Hex((u32)Calls[1]);
    Put(" kept "); Hex(Kept); Line();
    Leave();
}

#endif
