/* The calls of DOS that the extender does not translate, made in real
   mode by DPMI's function 0300h with a buffer below the megabyte: a full
   path, a name parsed into a file control block, the country, and a file
   deleted by a file control block. Then the screen and the mouse from
   flat code: the BIOS's calls, the screen's memory and the BIOS's data.
   And what a call costs: the instructions of the machine for one, of
   64 made in a row, to hold against what rmcalls.asm writes for the same
   calls from real mode. */
#include "toy.h"

static struct R r;
static struct Real real;

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

static void Copy(u8 *to, const char *from)
{
    while ((*to++ = *from++) != 0) ;
}

static void Cost(const char *name, void (*call)(struct R *), u32 ax, u32 bx)
{
    u32 began, i;

    began = AcpiNow();
    for (i = 0; i < 64; i++) {
        r.eax = ax; r.ebx = bx; r.ecx = r.edx = 0;
        call(&r);
    }
    began = (AcpiNow() - began) & 0xFFFFFF;
    /* A count of that timer is 279.4 ns, an instruction 256 ns. */
    Put(name); Put(" "); Dec(began * 2794 / 2560 / 64); Put("  ");
}

static void C10(struct R *q) { Int10(q); }
static void C16(struct R *q) { Int16(q); }
static void C21(struct R *q) { Int21(q); }
static void C33(struct R *q) { Int33(q); }
static void C31(struct R *q) { Int31(q); }
static void None(struct R *q) { q->eax = 0; }
static void Real10(struct R *q)
{
    Zero(&real, sizeof real); real.eax = q->eax;
    RealInt(0x10, &real);
}

void Main(void)
{
    u16 segment = LowBlock(0x40);
    u8 *low = (u8 *)((u32)segment << 4);

    Row = 0;
    Put("a block of DOS's at segment "); Hex(segment, 4); Line();
    Zero(&real, sizeof real); Copy(low, "HELLO.TXT");
    real.eax = 0x6000; real.ds = real.es = segment;
    real.esi = 0; real.edi = 0x80;
    RealInt(0x21, &real);
    Put("60: flags "); Hex(real.flags, 4); Put(" AX "); Hex(real.eax, 4);
    Put(" "); Text(low + 0x80, 40); Line();
    Zero(&real, sizeof real); Copy(low, "README.TXT"); Zero(low + 0x80, 64);
    real.eax = 0x2901; real.ds = real.es = segment;
    real.esi = 0; real.edi = 0x80;
    RealInt(0x21, &real);
    Put("29: AX "); Hex(real.eax, 4); Put(" SI "); Hex(real.esi, 4);
    Put(" FCB "); Text(low + 0x81, 11); Line();
    Zero(&real, sizeof real); Zero(low, 64);
    real.eax = 0x3800; real.ds = segment; real.edx = 0;
    RealInt(0x21, &real);
    Put("38: flags "); Hex(real.flags, 4); Put(" BX "); Hex(real.ebx, 4);
    Put(" date "); Hex(low[0], 2); Put(" separators ");
    Text(low + 7, 1); Text(low + 9, 1); Text(low + 11, 1); Text(low + 13, 1);
    Put("capitals at "); Hex(*(u32 *)(low + 0x12), 8); Line();
    r.eax = 0x3C00; r.ecx = 0; r.edx = (u32)"X1.TMP"; Int21(&r);
    r.ebx = r.eax; r.eax = 0x3E00; Int21(&r);
    Zero(&real, sizeof real); Zero(low, 64); Copy(low + 1, "X1      TMP");
    real.eax = 0x1300; real.ds = segment; real.edx = 0;
    RealInt(0x21, &real);
    Put("13: AL "); Hex(real.eax & 255, 2);
    r.eax = 0x4300; r.edx = (u32)"X1.TMP"; Int21(&r);
    Put("; then 43 of the file: carry "); Hex(r.flags & CARRY, 1);
    Put(" AX "); Hex(r.eax, 4); Line();

    r.eax = 0x0F00; Int10(&r);
    Put("10/0F mode "); Hex(r.eax, 4);
    r.eax = 0x0300; r.ebx = 0; Int10(&r);
    Put("  03 cursor "); Hex(r.edx, 4); Put(" shape "); Hex(r.ecx, 4);
    r.eax = 0x0200; r.ebx = 0; r.edx = 0x0A05; Int10(&r);
    r.eax = 0x0900 | 'X'; r.ebx = 0x1E; r.ecx = 3; Int10(&r);
    r.eax = 0x0800; r.ebx = 0; Int10(&r);
    Put("  08 cell "); Hex(r.eax, 4);
    r.eax = 0x0100; r.ecx = 0x0607; Int10(&r);
    Put("  at 450h "); Hex(*(u16 *)0x450, 4);
    Put(" at 460h "); Hex(*(u16 *)0x460, 4); Line();
    r.eax = 0x1130; r.ebx = 0x0600; r.ecx = r.edx = 0; Int10(&r);
    Put("10/1130: CX "); Hex(r.ecx, 4); Put(" DL "); Hex(r.edx & 255, 2);
    Put(" ES "); Hex(GetES(), 4); Put(", as DS "); Hex(GetDS(), 4);
    Zero(&real, sizeof real); real.eax = 0x1130; real.ebx = 0x0600;
    RealInt(0x10, &real);
    Put("; in real mode ES:BP "); Hex(real.es, 4); Put(":");
    Hex(real.ebp, 4); Line();
    r.eax = 0x1200; r.ebx = 0x10; Int10(&r);
    Put("10/12 BX "); Hex(r.ebx, 4); Put(" CX "); Hex(r.ecx, 4);
    r.eax = 0x150B; r.ebx = 0; r.ecx = 2; Int2F(&r);
    Put("  2F/150B AX "); Hex(r.eax, 4); Put(" BX "); Hex(r.ebx, 4); Line();

    r.eax = 0; Int33(&r);
    Put("33/00 AX "); Hex(r.eax, 4); Put(" buttons "); Hex(r.ebx, 4);
    r.eax = 4; r.ecx = 80; r.edx = 40; Int33(&r);
    r.eax = 3; Int33(&r);
    Put("  03 at "); Hex(r.ecx, 4); Put(","); Hex(r.edx, 4);
    Put(" buttons "); Hex(r.ebx, 4);
    r.eax = 10; r.ebx = 0; r.ecx = 0xFFFF; r.edx = 0x7700; Int33(&r);
    r.eax = 1; Int33(&r);
    Put("  01, the cell at 10,5 "); Hex(*(u16 *)(0xB8000 + 810 * 2), 4);
    r.eax = 2; Int33(&r);

    Row = 12; Col = 0;
    Put("Instructions a call: ");
    Cost("no call", None, 0, 0);
    Cost("16/11", C16, 0x1100, 0); Cost("16/02", C16, 0x0200, 0);
    Cost("10/03", C10, 0x0300, 0); Cost("10/0F", C10, 0x0F00, 0); Line();
    Cost("33/03", C33, 3, 0); Cost("21/2C", C21, 0x2C00, 0);
    Cost("21/19", C21, 0x1900, 0); Cost("21/0B", C21, 0x0B00, 0);
    Cost("21/62", C21, 0x6200, 0); Line();
    Cost("31/0006", C31, 6, GetDS());
    Cost("10/03 by 0300h", Real10, 0x0300, 0); Line();
    Exit(0);
}
