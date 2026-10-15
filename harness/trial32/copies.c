/* A copy of the program run from a command of the program, as a scenario
   does that types the program's name into its command line: each copy is
   elsewhere above the megabyte, with an AutoTim, a Clock and a stack of
   its own, and the driver of the tests has to read those of the copy
   that idles. harness/drivercheck.py holds it to this toy.

   The toy is auto.c with a key more. A copy writes into six rows of the
   screen that are its own, the first copy into the first six, and begins
   with its number, where its two variables are and its stack pointer.

   t  has the copy do something by itself nine ticks later, with its
      AutoTim set meanwhile
   c  switches its clock on, which blinks in the last cell of the copy's
      first row; o switches it off
   r  runs a copy of the toy through the shell, as a command; the copy
      that ran it goes on when that one has ended
   q  ends the copy

   The shell is run in real mode, as command.c runs it: the name, the
   tail and DOS's block in a block below the megabyte. The environment
   is named there, since the word of the PSP by which DOS would find it
   is a selector under the extender, and INT 22h is put back behind the
   command, or the extender writes a warning when the copy ends
   (DIARY.md, "Run a program and hand the shell a command from the
   program of 32 bits"). */
#include "toy.h"

#define ROWS 6                          /* of the screen, for a copy */

u32 AutoTim = 0xFFFFFFFFUL;
u8 Clock = 0;

static u32 First;                       /* the copy's first row */

static void Copy(u8 *to, const char *from)
{
    while ((*to++ = *from++) != 0) ;
}

/* On to the copy's next row, cleared, and round to its second behind its
   last. */
static void Next(void)
{
    u8 *screen = (u8 *)0xB8000;
    u32 i;

    Row = Row + 1 < First + ROWS ? Row + 1 : First + 1;
    Col = 0;
    for (i = 0; i < 79; i++) screen[(Row * 80 + i) * 2] = ' ';
}

void Main(void)
{
    struct R r;
    struct Real real;
    const u8 *psp = (const u8 *)SelBase((u16)Entry[E_ES]);
    u32 number = psp[0x80] > 1 ? (u32)(psp[0x82] - '0') : 1;
    u32 began = 0, blinked = 0, was[2];
    u16 segment = LowBlock(0x40);
    u8 *low = (u8 *)((u32)segment << 4);
    u8 *cell;
    u8 key;

    First = (number - 1) * ROWS;
    cell = (u8 *)0xB8000 + (First * 80 + 79) * 2;
    Row = First - 1; Next(); Row = First;
    Put("copy "); Dec(number); Put(" AutoTim at "); Hex((u32)&AutoTim, 8);
    Put(" Clock at "); Hex((u32)&Clock, 8); Put(" ESP ");
    Hex(GetESP(), 8);
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        if ((u16)AutoTim != 0xFFFF && Ticks() - began >= 9) {
            Next(); Put("done by itself at "); Hex(Ticks(), 8);
            AutoTim = 0xFFFFFFFFUL;
        }
        if (Clock && Ticks() - blinked >= 9) {
            *cell ^= ':' ^ ' ';
            blinked = Ticks();
        }
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        key = (u8)r.eax;
        Next(); Put("key "); Hex(r.eax, 4); Put(" at "); Hex(Ticks(), 8);
        switch (key) {
        case 't': began = Ticks(); AutoTim = began + 9; break;
        case 'c': Clock = 1; *cell = ':'; blinked = Ticks(); break;
        case 'o': Clock = 0; *cell = ' '; break;
        case 'r':
            r.eax = 0x200; r.ebx = 0x22; Int31(&r);
            was[0] = r.ecx; was[1] = r.edx;
            Copy(low, "A:\\FREEDOS\\BIN\\COMMAND.COM");
            Copy(low + 0x41, "/C D:\\VC\\VC.EXE 0\r"); low[0x40] = 17;
            low[0x40 + 17] = (u8)('0' + number + 1);
            Zero(low + 0x80, 14);
            *(u16 *)(low + 0x80) =
                (u16)(SelBase(*(const u16 *)(psp + 0x2C)) >> 4);
            *(u16 *)(low + 0x82) = 0x40; *(u16 *)(low + 0x84) = segment;
            Zero(&real, sizeof real);
            real.eax = 0x4B00; real.ds = real.es = segment;
            real.edx = 0; real.ebx = 0x80;
            RealInt(0x21, &real);
            r.eax = 0x201; r.ebx = 0x22; r.ecx = was[0]; r.edx = was[1];
            Int31(&r);
            Next(); Put("came back at "); Hex(Ticks(), 8);
            if (real.flags & CARRY) { Put(" refused "); Hex(real.eax, 4); }
            break;
        case 'q': Exit(0);
        }
    }
}
