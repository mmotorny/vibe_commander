/* The functions that the toys of the trial share: see toy.h. */
#include "toy.h"

u32 Row = 2, Col = 0;
int ViaDos;

void Put(const char *text)
{
    u8 *screen = (u8 *)0xB8000;

    if (ViaDos) {
        struct R r;
        u32 size = 0;

        while (text[size]) size++;
        r.eax = 0x4000; r.ebx = 1; r.ecx = size; r.edx = (u32)text;
        Int21(&r);
        return;
    }
    for (; *text; text++) {
        if (Col >= 80) Line();
        screen[(Row * 80 + Col) * 2] = *text;
        screen[(Row * 80 + Col) * 2 + 1] = 0x07;
        Col++;
    }
}

void Line(void)
{
    if (ViaDos) { Put("\r\n"); return; }
    Col = 0;
    if (Row < 24) Row++;
}

void Hex(u32 value, int digits)
{
    char text[9];
    int at;

    for (at = digits - 1; at >= 0; at--, value >>= 4)
        text[at] = "0123456789ABCDEF"[value & 15];
    text[digits] = 0;
    Put(text);
}

void Dec(u32 value)
{
    char text[11];
    int at = 10;

    text[10] = 0;
    do { text[--at] = (char)('0' + value % 10); value /= 10; } while (value);
    Put(text + at);
}

void Exit(int code)
{
    struct R r;

    r.eax = 0x4C00 | (code & 255);
    Int21(&r);
}

void Zero(void *at, u32 size)
{
    u8 *to = at;

    while (size--) *to++ = 0;
}

u32 Ticks(void) { return *(volatile u32 *)0x46C; }

u32 SelBase(u16 selector)
{
    struct R r;

    r.eax = 6; r.ebx = selector;
    Int31(&r);
    if (r.flags & CARRY) return 0xFFFFFFFFUL;
    return (r.ecx & 0xFFFF) << 16 | (r.edx & 0xFFFF);
}

u32 Acpi(void);
#pragma aux Acpi = "mov edx,608h" "in eax,dx" value [eax] \
    modify exact [eax edx];
u32 AcpiNow(void) { return Acpi() & 0xFFFFFF; }

int RealInt(int number, struct Real *real)
{
    struct R r;

    real->sp = real->ss = 0;            /* a stack of the extender's */
    r.eax = 0x300; r.ebx = number; r.ecx = 0; r.edi = (u32)real;
    r.edx = r.esi = 0;
    Int31(&r);
    return r.flags & CARRY;
}

u16 LowBlock(u32 paragraphs)
{
    struct R r;

    Zero(&r, sizeof r);
    r.eax = 0x100; r.ebx = paragraphs;
    Int31(&r);
    return r.flags & CARRY ? 0 : (u16)r.eax;
}

/* The stack. The linker wants a segment of the class STACK, makes it as
   long as `option stack=` says, and has the header of the file name its
   end: the extender enters the program on it. Only these 16 bytes of it
   are in the file. */
#pragma data_seg("STACK", "STACK")
u8 Stack[16] = { 0 };
#pragma data_seg()

u32 Entry[12];
extern void Main(void);

/* Where the extender enters, which the linker is told with `option
   start=Start_`: CS, DS and SS are flat, ES is the PSP's selector. What
   was handed over is kept, ES is put on the data, where compiled code
   takes it to be, and the toy goes on in Main. This is all that a C
   library's start-up would have had to do. */
__declspec(naked) void Start(void)
{
    _asm {
        mov dword ptr Entry, esp
        mov word ptr Entry+4, cs
        mov word ptr Entry+8, ds
        mov word ptr Entry+12, es
        mov word ptr Entry+16, ss
        mov word ptr Entry+20, fs
        mov word ptr Entry+24, gs
        mov dword ptr Entry+32, ebx
        mov edx, 608h
        in eax, dx
        mov dword ptr Entry+28, eax
        pushfd
        pop eax
        mov dword ptr Entry+36, eax
        cli
        mov al, 0C2h
        out 43h, al
        in al, 40h
        mov byte ptr Entry+46, al
        in al, 40h
        mov ah, al
        in al, 40h
        xchg al, ah
        mov word ptr Entry+44, ax
        mov eax, dword ptr ds:[46Ch]
        mov dword ptr Entry+40, eax
        sti
        push ds
        pop es
        cld
        jmp Main
    }
}
