/* What a program that this one runs finds of the extender's memory: a
   second copy of the toy, the program of the portable C, a command of
   DOS. The toy takes the largest block that the extender has, gives
   back as much of it as the keys say, and runs something by INT 21h
   function 4Bh in real mode, as the program of the portable C does.
   DIARY.md, "Write down what the extender's memory does on the bench,
   the piece X", has what it showed. By keys:

   k  take two blocks of 64 KB and keep them to the end, as the two
      panels of the program would be kept while a command runs
   t  take the largest block and keep it
   1  make the kept block 4 KB shorter, by 0503h
   s  make it 64 KB shorter
   S  make it 1 MB shorter
   b  give it back
   f  what 0500h says is free
   c  run a copy of the toy, D:\VC\VC.EXE with a tail: it says where it
      is, what 0500h tells it, takes the largest block and says where
      that is, and ends with 7
   p  run D:\VC.EXE, which run.py puts there with `--extra
      VC.EXE=FILE`: the program of the portable C, which the driver of
      the tests then takes through the steps that follow
   m  run `mem` through the shell
   a  run D:\VC\ABOVE.COM, above.asm, which run.py puts there with
      `--extra VC/ABOVE.COM=build/trial32/above.com`
   e  run D:\VC\ENDED.COM, ended.asm, put there with `--extra`, which
      runs D:\VC\T.EXE and has DOS end that at its first idle call
   q  the end

   Under the name T.EXE the toy takes the largest block and idles, and
   does not end: so that ended.asm can have DOS end it by another's
   hand with a block of the extender's kept.

   Behind each run it says what DOS answered and the code that the
   program ended with. Its texts go through DOS, so that the screen
   scrolls. */
#include "toy.h"

static struct R r;
static struct Real real;
static u32 info[12];
static u32 heldAt, heldName, heldSize;

static void Copy(u8 *to, const char *from)
{
    while ((*to++ = *from++) != 0) ;
}

static u32 Free(void)
{
    Zero(&r, sizeof r); r.eax = 0x500; r.edi = (u32)info; Int31(&r);
    return info[0];
}

static void SayFree(void)
{
    Put("free "); Hex(Free(), 8); Put(", ");
    Dec(info[0] >> 10); Put(" KB"); Line();
}

static void Take(void)
{
    heldSize = Free();
    Zero(&r, sizeof r);
    r.eax = 0x501; r.ebx = heldSize >> 16; r.ecx = heldSize & 0xFFFF;
    Int31(&r);
    if (r.flags & CARRY) { Put("refused"); Line(); heldSize = 0; return; }
    heldAt = (r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF);
    heldName = (r.esi & 0xFFFF) << 16 | (r.edi & 0xFFFF);
    Put("took "); Hex(heldSize, 8); Put(" bytes at "); Hex(heldAt, 8);
    Put("; ");
}

static void Shorter(u32 by)
{
    if (heldSize <= by) { Put("no block that long is kept"); Line(); return; }
    heldSize -= by;
    Zero(&r, sizeof r);
    r.eax = 0x503; r.ebx = heldSize >> 16; r.ecx = heldSize & 0xFFFF;
    r.esi = heldName >> 16; r.edi = heldName & 0xFFFF;
    Int31(&r);
    if (r.flags & CARRY) { Put("refused"); Line(); return; }
    heldAt = (r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF);
    heldName = (r.esi & 0xFFFF) << 16 | (r.edi & 0xFFFF);
    Put("kept "); Hex(heldSize, 8); Put(" bytes at "); Hex(heldAt, 8);
    Put("; ");
}

/* Run the file with the tail, in real mode, with the toy's own
   environment named: see Run of the portable C's sys_dos.c for why. */
static void Run(u8 *low, u16 segment, u16 environment,
                const char *name, const char *tail)
{
    u32 size = 0;

    while (tail[size]) size++;
    Copy(low, name);
    Copy(low + 0x41, tail); low[0x40] = (u8)size; low[0x41 + size] = '\r';
    Zero(low + 0x80, 14);
    *(u16 *)(low + 0x80) = environment;
    *(u16 *)(low + 0x82) = 0x40; *(u16 *)(low + 0x84) = segment;
    Zero(&real, sizeof real);
    real.eax = 0x4B00; real.ds = real.es = segment;
    real.edx = 0; real.ebx = 0x80;
    RealInt(0x21, &real);
    Put("came back: carry "); Hex(real.flags & CARRY, 1);
    Put(" AX "); Hex(real.eax, 4);
    r.eax = 0x4D00; Int21(&r);
    Put(", ended with "); Hex(r.eax, 4); Put("; "); SayFree();
}

void Main(void)
{
    const u8 *psp = (const u8 *)SelBase((u16)Entry[E_ES]);
    const u8 *name;
    u16 segment, environment;
    u8 *low;
    u8 key;

    ViaDos = 1;
    environment = (u16)(SelBase(*(const u16 *)(psp + 0x2C)) >> 4);
    /* The toy's own file is named behind the end of the environment. */
    name = (const u8 *)((u32)environment << 4);
    while (name[0] || name[1]) name++;
    name += 4;
    while (*name) name++;
    if (name[-5] == 'T' && name[-6] == '\\') {
        Put("  T: code at "); Hex((u32)Main, 8); Put("; "); SayFree();
        Put("  "); Take(); SayFree();
        for (;;) { Zero(&r, sizeof r); Int28(&r); }
    }
    if (psp[0x80] > 1) {                /* a tail: the copy */
        Put("  a copy: code at "); Hex((u32)Main, 8); Put("; ");
        SayFree();
        Put("  "); Take(); SayFree();
        Exit(7);
    }
    r.eax = 0x0200; r.ebx = 0; r.edx = 0x0100; Int10(&r);
    Put("code at "); Hex((u32)Main, 8); Put("; "); SayFree();
    segment = LowBlock(0x40); low = (u8 *)((u32)segment << 4);
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        key = (u8)r.eax;
        switch (key) {
        case 'k':
            for (key = 0; key < 2; key++) {
                Zero(&r, sizeof r); r.eax = 0x501; r.ebx = 1; Int31(&r);
                Put("64 KB at ");
                Hex((r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF), 8); Put("; ");
            }
            SayFree();
            break;
        case 't': Take(); SayFree(); break;
        case '1': Shorter(0x1000); SayFree(); break;
        case 's': Shorter(0x10000); SayFree(); break;
        case 'S': Shorter(0x100000); SayFree(); break;
        case 'b':
            Zero(&r, sizeof r);
            r.eax = 0x502; r.esi = heldName >> 16; r.edi = heldName & 0xFFFF;
            Int31(&r);
            heldSize = 0;
            Put("gave it back; "); SayFree();
            break;
        case 'f': SayFree(); break;
        case 'c':
            Put("c: a copy"); Line();
            Run(low, segment, environment, "D:\\VC\\VC.EXE", " 2");
            break;
        case 'p':
            Put("p: the program"); Line();
            Run(low, segment, environment, "D:\\VC.EXE", "");
            break;
        case 'm':
            Put("m: mem"); Line();
            Run(low, segment, environment, "A:\\FREEDOS\\BIN\\COMMAND.COM",
                "/C MEM");
            break;
        case 'a':
            Put("a: above"); Line();
            Run(low, segment, environment, "D:\\VC\\ABOVE.COM", "");
            break;
        case 'e':
            Put("e: ended"); Line();
            Run(low, segment, environment, "D:\\VC\\ENDED.COM", "");
            break;
        case 'q': Exit(0);
        }
    }
}
