/* Calls of DOS that take addresses, made from protected mode with every
   address above the megabyte: which does the extender translate? A line
   for each group of calls, each call as its number, a C behind it if it
   came back with the carry flag set, and what it left in AX; then what
   it was to fill in. A call that the extender does not translate comes
   back wrong, or right with nothing filled in: 60h, 29h and 38h under
   DOS/32A. realcalls.c makes those in real mode. */
#include "toy.h"

static struct R r;
static u8 Buffer[0x12000];
static u8 Dta[128];

static void Show(const char *name)
{
    Put(name); Put(r.flags & CARRY ? ":C" : ":");
    Hex(r.eax, r.eax > 0xFFFF ? 8 : 4); Put(" ");
}

static void Text(const u8 *text, int most)
{
    char one[2];

    one[1] = 0;
    Put("'");
    for (; *text && most--; text++) {
        one[0] = *text < 32 ? '.' : *text;
        Put(one);
    }
    Put("' ");
}

static void Dos(u32 ax, u32 bx, u32 cx, u32 dx, u32 si, u32 di)
{
    r.eax = ax; r.ebx = bx; r.ecx = cx; r.edx = dx; r.esi = si; r.edi = di;
    Int21(&r);
}

static u8 Pattern(u32 at) { return (u8)(at * 7 + (at >> 8)); }

void Main(void)
{
    static u8 fcb[64];
    u32 handle, i, bad;

    Row = 0;
    /* A file opened, read, measured and closed. */
    Dos(0x3D00, 0, 0, (u32)"C:\\WORK\\HELLO.TXT", 0, 0); Show("3D");
    handle = r.eax;
    Zero(Buffer, 32);
    Dos(0x3F00, handle, 15, (u32)Buffer, 0, 0); Show("3F"); Text(Buffer, 13);
    Dos(0x4202, handle, 0, 0, 0, 0); Show("42"); Hex(r.edx, 4); Put(" ");
    Dos(0x5700, handle, 0, 0, 0, 0); Show("57");
    Hex(r.ecx, 4); Hex(r.edx, 4); Put(" ");
    Dos(0x3E00, handle, 0, 0, 0, 0); Show("3E"); Line();
    /* More than 64K written at one call, and read back. */
    for (i = 0; i < sizeof Buffer; i++) Buffer[i] = Pattern(i);
    Dos(0x3C00, 0, 0, (u32)"BIG.DAT", 0, 0); Show("3C"); handle = r.eax;
    Dos(0x4000, handle, sizeof Buffer, (u32)Buffer, 0, 0);
    Show("40 of 12000h");
    Dos(0x4202, handle, 0, 0, 0, 0); Show("42 end"); Hex(r.edx, 4); Put(" ");
    Dos(0x3E00, handle, 0, 0, 0, 0); Show("3E"); Line();
    Zero(Buffer, sizeof Buffer);
    Dos(0x3D00, 0, 0, (u32)"BIG.DAT", 0, 0); Show("3D"); handle = r.eax;
    Dos(0x3F00, handle, sizeof Buffer, (u32)Buffer, 0, 0);
    Show("3F of 12000h");
    for (i = bad = 0; i < sizeof Buffer; i++) bad += Buffer[i] != Pattern(i);
    Put("wrong bytes "); Dec(bad); Put(" ");
    Dos(0x3E00, handle, 0, 0, 0, 0); Show("3E"); Line();
    /* A directory searched, with the DTA above the megabyte. */
    Dos(0x1A00, 0, 0, (u32)Dta, 0, 0); Show("1A");
    Dos(0x2F00, 0, 0, 0, 0, 0); Show("2F");
    Put("BX "); Hex(r.ebx, 8); Put(" for a DTA at "); Hex((u32)Dta, 8);
    Line();
    Dos(0x4E00, 0, 0x37, (u32)"C:\\WORK\\*.*", 0, 0); Show("4E");
    Text(Dta + 0x1E, 13);
    for (i = 0; i < 3; i++) {
        Dos(0x4F00, 0, 0, 0, 0, 0); Show("4F"); Text(Dta + 0x1E, 13);
    }
    Line();
    /* Directories. */
    Zero(Buffer, 80);
    Dos(0x4700, 0, 0, 0, (u32)Buffer, 0); Show("47"); Text(Buffer, 20);
    Dos(0x3B00, 0, 0, (u32)"C:\\STAMPS", 0, 0); Show("3B");
    Dos(0x4700, 0, 0, 0, (u32)Buffer, 0); Show("47"); Text(Buffer, 20);
    Dos(0x3B00, 0, 0, (u32)"C:\\WORK", 0, 0); Show("3B"); Line();
    Dos(0x3900, 0, 0, (u32)"NEWDIR", 0, 0); Show("39");
    Dos(0x5600, 0, 0, (u32)"NEWDIR", 0, (u32)"NEWDIR2"); Show("56");
    Dos(0x4300, 0, 0, (u32)"NEWDIR2", 0, 0); Show("43");
    Hex(r.ecx, 4); Put(" ");
    Dos(0x3A00, 0, 0, (u32)"NEWDIR2", 0, 0); Show("3A");
    Dos(0x4301, 0, 1, (u32)"BIG.DAT", 0, 0); Show("4301");
    Dos(0x4300, 0, 0, (u32)"BIG.DAT", 0, 0); Show("43");
    Hex(r.ecx, 4); Put(" ");
    Dos(0x4301, 0, 0, (u32)"BIG.DAT", 0, 0); Show("4301");
    Dos(0x4100, 0, 0, (u32)"BIG.DAT", 0, 0); Show("41"); Line();
    /* A full path, a name parsed, the country. */
    Zero(Buffer, 128);
    Dos(0x6000, 0, 0, 0, (u32)"HELLO.TXT", (u32)Buffer); Show("60");
    Text(Buffer, 30); Line();
    Zero(fcb, sizeof fcb);
    Dos(0x2901, 0, 0, 0, (u32)"README.TXT", (u32)fcb); Show("29");
    Text(fcb + 1, 11); Line();
    Zero(Buffer, 64);
    Dos(0x3800, 0, 0, (u32)Buffer, 0, 0); Show("38");
    Put("BX "); Hex(r.ebx, 4); Put(" date "); Hex(Buffer[0], 2);
    Put(" separators "); Text(Buffer + 9, 1); Text(Buffer + 11, 1);
    Put("capitals at "); Hex(*(u32 *)(Buffer + 0x12), 8); Line();
    /* Calls that take no address. */
    Dos(0x3600, 0, 0, 3, 0, 0); Show("36"); Hex(r.ebx, 4); Put(" ");
    Dos(0x1900, 0, 0, 0, 0, 0); Show("19");
    Dos(0x3000, 0, 0, 0, 0, 0); Show("30");
    Dos(0x3306, 0, 0, 0, 0, 0); Show("3306"); Hex(r.ebx, 4); Put(" ");
    Dos(0x4408, 3, 0, 0, 0, 0); Show("4408");
    Dos(0x4409, 3, 0, 0, 0, 0); Show("4409"); Hex(r.edx, 4); Put(" ");
    Dos(0x440E, 1, 0, 0, 0, 0); Show("440E"); Line();
    Dos(0x3524, 0, 0, 0, 0, 0); Show("3524"); Put("EBX "); Hex(r.ebx, 8);
    Dos(0x5900, 0, 0, 0, 0, 0); Show(" 59"); Line();
    /* A text printed by DOS, where the cursor is: fourteen lines down. */
    Dos(0x0900, 0, 0, (u32)"\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "09: a text for DOS to print$", 0, 0);
    Dos(0x0600, 0, 0, '!', 0, 0);
    Exit(0);
}
