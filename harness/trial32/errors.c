/* DOS's critical error and Ctrl-Break, taken by handlers in protected
   mode that are set with DPMI's function 0205h. The error is a file made
   on A:, the diskette that the bench boots from, which cannot be
   written: the handler writes down what it is called with and answers 3,
   Fail. Then the toy goes round for keys and writes the three counts
   behind each: a is any key, Ctrl+Break is the BIOS's INT 1Bh, and
   behind w the toy leaves the keys to DOS until DOS has seen a Ctrl+c
   and called INT 23h. q ends it, with the vectors given back: left as
   they are, DOS/32A writes a warning for each as the program ends. */
#include "toy.h"

u16 DataSelector;
u32 Count24, Count23, Count1B, Seen24[8];

__declspec(naked) void Handler24(void)
{
    _asm {
        push ds
        push eax
        mov ax, word ptr cs:DataSelector
        mov ds, ax
        pop eax
        inc dword ptr Count24
        mov dword ptr Seen24, eax
        mov dword ptr Seen24+4, edi
        mov dword ptr Seen24+8, esi
        mov dword ptr Seen24+12, ebp
        mov dword ptr Seen24+16, esp
        mov word ptr Seen24+20, ss
        mov eax, 3
        pop ds
        iretd
    }
}

__declspec(naked) void Handler23(void)
{
    _asm {
        push ds
        push eax
        mov ax, word ptr cs:DataSelector
        mov ds, ax
        inc dword ptr Count23
        pop eax
        pop ds
        iretd
    }
}

__declspec(naked) void Handler1B(void)
{
    _asm {
        push ds
        push eax
        mov ax, word ptr cs:DataSelector
        mov ds, ax
        inc dword ptr Count1B
        pop eax
        pop ds
        iretd
    }
}

static struct R r;
static u32 Before[3][2];
static int Taken;

static void Take(u32 number, void (*handler)(void))
{
    r.eax = 0x204; r.ebx = number; Int31(&r);
    Before[Taken][0] = r.ecx; Before[Taken][1] = r.edx; Taken++;
    Put("vector "); Hex(number, 2); Put(" was "); Hex(r.ecx, 4); Put(":");
    Hex(r.edx, 8);
    r.eax = 0x205; r.ebx = number; r.ecx = GetCS(); r.edx = (u32)handler;
    Int31(&r);
    Put(r.flags & CARRY ? ", not set" : ", set");
    r.eax = 0x200; r.ebx = number; Int31(&r);
    Put("; in real mode it is "); Hex(r.ecx, 4); Put(":"); Hex(r.edx, 4);
    Line();
}

static void GiveBack(u32 number, int which)
{
    r.eax = 0x205; r.ebx = number;
    r.ecx = Before[which][0]; r.edx = Before[which][1];
    Int31(&r);
}

static void Counts(void)
{
    Put("INT 24h "); Dec(Count24); Put(" INT 23h "); Dec(Count23);
    Put(" INT 1Bh "); Dec(Count1B); Line();
}

void Main(void)
{
    u32 before;

    DataSelector = GetDS();
    Row = 1;
    Take(0x24, Handler24); Take(0x23, Handler23); Take(0x1B, Handler1B);
    r.eax = 0x3C00; r.ecx = 0; r.edx = (u32)"A:\\X.TMP"; Int21(&r);
    Put("3C on A: carry "); Hex(r.flags & CARRY, 1);
    Put(" AX "); Hex(r.eax, 4);
    r.eax = 0x5900; r.ebx = 0; Int21(&r);
    Put("; 59: AX "); Hex(r.eax, 4); Line();
    Counts();
    Put("the handler was called with AX "); Hex(Seen24[0], 8);
    Put(" DI "); Hex(Seen24[1], 8); Put(" BP:SI "); Hex(Seen24[3], 8);
    Put(":"); Hex(Seen24[2], 8); Line();
    Put("  on the stack "); Hex(Seen24[5], 4); Put(":"); Hex(Seen24[4], 8);
    Line();
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        Put("key "); Hex(r.eax, 4); Put(": "); Counts();
        if ((u8)r.eax == 'w') {
            before = Count23;
            while (Count23 == before) {
                Zero(&r, sizeof r); Int28(&r);
                Zero(&r, sizeof r); r.eax = 0x0B00; Int21(&r);
            }
            Put("DOS saw it: "); Counts();
        }
        if ((u8)r.eax == 'q') break;
    }
    GiveBack(0x24, 0); GiveBack(0x23, 1); GiveBack(0x1B, 2);
    Exit(0);
}
