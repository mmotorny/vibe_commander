/* How much of the program's own stack is written below the stack
   pointer while a call goes through the extender: the extender is
   entered on the program's stack, and what it keeps there of a call
   that is under way is where nothing else may go on meanwhile, the
   routine of the critical error least of all (STACK_KEPT of
   src/portable_c/sys_dos.c). The stack is filled below the stack pointer,
   a call is made, and the lowest byte that is another then is how deep
   it went: written in decimal behind each call, with the 8 bytes that
   the toy's own instructions push. The last line is of the timer's
   interrupts while the toy goes round in protected mode, and of 2,000
   calls one behind the other. */
#include "toy.h"

#define DEEP 3000
#define TOP 32

static u8 *Low;

static void Mark(u32 esp)
{
    u8 *at;

    Low = (u8 *)esp - DEEP;
    for (at = Low; at < (u8 *)esp - TOP; at++)
        *at = 0xA5;
}

static u32 Used(u32 esp)
{
    u8 *at;

    for (at = Low; at < (u8 *)esp - TOP; at++)
        if (*at != 0xA5)
            return (u8 *)esp - at;
    return TOP;
}

static struct R r;
static struct Real real;
static char name[] = "C:\\WORK\\HELLO.TXT";
static u8 buffer[20000];

static void Show(const char *what, u32 esp)
{
    Put(what); Put(" "); Dec(Used(esp)); Put("; ");
}

void Main(void)
{
    u32 esp = GetESP();
    u32 i;
    u16 handle;

    Put("stack at "); Hex(esp, 8);
    Put("; the segment of the selector 18h, which the extender has its "
        "stack in where it enters a routine, begins at ");
    Hex(SelBase(0x18), 8); Line();
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x3000; Int21(&r); Show("30h", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x3D00; r.edx = (u32)name; Int21(&r); Show("3Dh", esp);
    handle = (u16)r.eax;
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x3F00; r.ebx = handle; r.ecx = 20000; r.edx = (u32)buffer; Int21(&r); Show("3Fh", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x3E00; r.ebx = handle; Int21(&r); Show("3Eh", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x4E00; r.edx = (u32)"*.*"; Int21(&r); Show("4Eh", esp);
    Line();
    Mark(esp); Zero(&real, sizeof real); real.eax = 0x3000; RealInt(0x21, &real); Show("30h by 0300h", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x0300; Int10(&r); Show("INT 10h 03h", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r); Show("INT 16h 11h", esp);
    Mark(esp); Zero(&r, sizeof r); r.eax = 3; Int33(&r); Show("INT 33h 3", esp);
    Mark(esp); Zero(&r, sizeof r); Int28(&r); Show("INT 28h", esp);
    Line();
    /* A long while with interrupts of the timer, in protected mode. */
    Mark(esp);
    for (i = Ticks(); Ticks() - i < 20; ) ;
    Show("20 ticks in protected mode", esp);
    Mark(esp);
    for (i = 0; i < 2000; i++) { Zero(&r, sizeof r); r.eax = 0x2C00; Int21(&r); }
    Show("2000 calls of 2Ch", esp);
    Line();
    Exit(0);
}
