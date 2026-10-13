/* Running a command from a 32-bit program, and what DOS has for it. The
   first screen has where the program is, DOS's largest free block and
   DOS's chain of blocks, each as its segment, its length in paragraphs
   and whose it is. Then, by keys:

   1  COMMAND.COM with /C through INT 21h function 4Bh in protected
      mode, with a block of parameters as DOS/4GW has it, which is a
      guess: the pinned Open Watcom does not describe DOS/32A's. It
      does not deliver the tail: COMMAND.COM comes up for keys and the
      script is over
   2  the same in real mode, by DPMI's function 0300h, with the name,
      the tail and the block in a block of DOS's
   3  as 2, with this program for the command: a second copy of it runs
      inside a command of the first, says where it is and ends with 7
   4  COMMAND.COM's INT 2Eh, through a few instructions of real mode in
      a block of DOS's that put the stack back behind it, called with
      DPMI's function 0301h
   m  the first screen's lines again
   v  where INT 22h goes in real mode: a command run by 4Bh leaves it
      changed, and DOS/32A writes a warning for that as the program ends
   r  put it back as it was when the program started
   q  the end

   Its texts go through DOS, so that the screen scrolls. */
#include "toy.h"

static struct R r;
static struct Real real;

static void Copy(u8 *to, const char *from)
{
    while ((*to++ = *from++) != 0) ;
}

static void Chain(void)
{
    u32 segment;
    const u8 *block;
    char name[9];
    int i;

    Zero(&real, sizeof real); real.eax = 0x5200; RealInt(0x21, &real);
    segment = *(u16 *)(((u32)real.es << 4) + (real.ebx & 0xFFFF) - 2);
    Put("DOS's blocks: ");
    for (;;) {
        block = (const u8 *)(segment << 4);
        Hex(segment + 1, 4); Put(" "); Hex(*(u16 *)(block + 3), 4);
        if (*(u16 *)(block + 1) == 0)
            Put(" free");
        else {
            Put(" of "); Hex(*(u16 *)(block + 1), 4);
            if (*(u16 *)(block + 1) == segment + 1) {
                for (i = 0; i < 8 && block[8 + i] >= ' '; i++)
                    name[i] = block[8 + i];
                name[i] = 0;
                Put(" "); Put(name);
            }
        }
        Put("; ");
        if (block[0] != 'M') break;
        segment += *(u16 *)(block + 3) + 1;
    }
    Line();
}

static void Free(void)
{
    r.eax = 0x100; r.ebx = 0xFFFF; Int31(&r);
    Put("largest free block of DOS: "); Dec((r.ebx & 0xFFFF) * 16);
    Put(" bytes"); Line();
}

/* int 2Eh, and the stack as it was before: the interrupt comes back
   with SS and SP as COMMAND.COM left them. */
static const u8 Quick[] = {
    0x2E, 0x89, 0x26, 0x30, 0x00,       /* mov cs:[30h],sp */
    0x2E, 0x8C, 0x16, 0x32, 0x00,       /* mov cs:[32h],ss */
    0xCD, 0x2E,                         /* int 2Eh */
    0xFA,                               /* cli */
    0x2E, 0x8E, 0x16, 0x32, 0x00,       /* mov ss,cs:[32h] */
    0x2E, 0x8B, 0x26, 0x30, 0x00,       /* mov sp,cs:[30h] */
    0xFB, 0xFC, 0xCB };                 /* sti ; cld ; retf */

void Main(void)
{
    static u8 tail[64];
    struct { u32 environment; u16 its; u32 tail; u16 tails;
             u32 first; u16 firsts; u32 second; u16 seconds; } block;
    const u8 *psp = (const u8 *)SelBase((u16)Entry[E_ES]);
    u16 segment;
    u8 *low;
    u32 i, was[2];
    u8 key;

    r.eax = 0x0200; r.ebx = 0; r.edx = 0x0100; Int10(&r);
    ViaDos = 1;
    if (psp[0x80] > 1) {                /* a tail: the second copy */
        Put("  a second copy runs: PSP at "); Hex((u32)psp, 8);
        Put(", code at "); Hex((u32)Main, 8); Line();
        Put("  "); Free();
        Exit(7);
    }
    Put("PSP at "); Hex((u32)psp, 8); Put(", code at "); Hex((u32)Main, 8);
    Line();
    Free(); Chain();
    segment = LowBlock(0x40); low = (u8 *)((u32)segment << 4);
    r.eax = 0x200; r.ebx = 0x22; Int31(&r);
    was[0] = r.ecx; was[1] = r.edx;
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        key = (u8)r.eax;
        switch (key) {
        case '1':
            Put("1: 4Bh in protected mode"); Line();
            Copy(tail + 1, "/C ECHO a command ran\r"); tail[0] = 21;
            Zero(&block, sizeof block);
            block.tail = (u32)tail; block.tails = GetDS();
            r.eax = 0x4B00; r.ebx = (u32)&block;
            r.edx = (u32)"A:\\FREEDOS\\BIN\\COMMAND.COM";
            Int21(&r);
            Put("came back: carry "); Hex(r.flags & CARRY, 1);
            Put(" AX "); Hex(r.eax, 4); Line();
            break;
        case '2': case '3':
            Put(key == '2' ? "2: 4Bh in real mode"
                           : "3: this program in a command");
            Line();
            Copy(low, "A:\\FREEDOS\\BIN\\COMMAND.COM");
            if (key == '2') {
                Copy(low + 0x41, "/C ECHO a command ran\r"); low[0x40] = 21;
            } else {
                Copy(low + 0x41, "/C D:\\VC\\VC.EXE 2\r"); low[0x40] = 17;
            }
            Zero(low + 0x80, 14);
            *(u16 *)(low + 0x82) = 0x40; *(u16 *)(low + 0x84) = segment;
            Zero(&real, sizeof real);
            real.eax = 0x4B00; real.ds = real.es = segment;
            real.edx = 0; real.ebx = 0x80;
            RealInt(0x21, &real);
            Put("came back: flags "); Hex(real.flags, 4);
            Put(" AX "); Hex(real.eax, 4);
            r.eax = 0x4D00; Int21(&r);
            Put(" exit code "); Hex(r.eax, 4); Line();
            break;
        case '4':
            Put("4: INT 2Eh"); Line();
            for (i = 0; i < sizeof Quick; i++) low[0x100 + i] = Quick[i];
            Copy(low + 0x41, "ECHO quick\r"); low[0x40] = 10;
            Zero(&real, sizeof real);
            real.ds = real.es = segment; real.esi = 0x40;
            real.cs = segment + 0x10; real.ip = 0;
            r.eax = 0x301; r.ebx = 0; r.ecx = 0; r.edi = (u32)&real;
            Int31(&r);
            Put("came back: carry "); Hex(r.flags & CARRY, 1);
            Put(" AX "); Hex(real.eax, 4); Line();
            break;
        case 'm': Free(); Chain(); break;
        case 'v':
            r.eax = 0x200; r.ebx = 0x22; Int31(&r);
            Put("INT 22h goes to "); Hex(r.ecx, 4); Put(":"); Hex(r.edx, 4);
            Put(", and went to "); Hex(was[0], 4); Put(":"); Hex(was[1], 4);
            Line();
            break;
        case 'r':
            r.eax = 0x201; r.ebx = 0x22; r.ecx = was[0]; r.edx = was[1];
            Int31(&r);
            Put("put back"); Line();
            break;
        case 'q': Exit(0);
        }
    }
}
