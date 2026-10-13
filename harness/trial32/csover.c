/* The fault of QEMU that stops CauseWay, without CauseWay. In a code
   segment whose base is not 0, while DS, ES and SS all begin at 0, an
   instruction with 32-bit addressing that reads memory through cs: reads
   at the same offset from 0 instead. A processor reads through CS.
   CauseWay's entry of every interrupt is such code; a flat program has
   CS at 0 and is not touched by it.

   Three pieces of code are run in a segment made for each, with the
   data segment's selector at their offset 18h. Each prints what it read:
   that selector if the machine is right, the word at linear 18h if not.
   The second and the third load what they read into GS, which faults in
   a machine that is wrong, so only the first is run unless Count says
   more. Run under DOS/32A, which is not affected. */
#include "toy.h"

u32 Count = 1;

u32 FarCall(void *pointer);
#pragma aux FarCall = "db 0FFh, 18h" /* call far [eax] */ \
    parm [eax] value [eax] modify [eax ebx ecx edx gs];

static u8 Code[3][0x20] = {
    /* mov eax,0 ; o16 mov ax,cs:[18h] ; retf */
    { 0xB8, 0, 0, 0, 0, 0x66, 0x2E, 0x8B, 0x05, 0x18, 0, 0, 0, 0xCB },
    /* mov eax,0 ; mov gs,cs:[18h] ; mov ax,gs ; retf */
    { 0xB8, 0, 0, 0, 0, 0x2E, 0x8E, 0x2D, 0x18, 0, 0, 0,
      0x66, 0x8C, 0xE8, 0xCB },
    /* mov eax,0 ; o16 mov gs,cs:[18h] ; mov ax,gs ; retf */
    { 0xB8, 0, 0, 0, 0, 0x66, 0x2E, 0x8E, 0x2D, 0x18, 0, 0, 0,
      0x66, 0x8C, 0xE8, 0xCB },
};

void Main(void)
{
    static const char *const names[3] = {
        "o16 mov ax,cs:[18h]", "mov gs,cs:[18h]", "o16 mov gs,cs:[18h]" };
    struct { u32 offset; u16 selector; } pointer;
    struct R r;
    u16 selector;
    u32 i;

    Put("the word at linear 18h is "); Hex(*(u16 *)0x18, 4);
    Put("; each piece has "); Hex(GetDS(), 4); Put(" at its 18h"); Line();
    for (i = 0; i < Count && i < 3; i++) {
        *(u16 *)(Code[i] + 0x18) = GetDS();
        Zero(&r, sizeof r);
        r.eax = 0; r.ecx = 1; Int31(&r);            /* a descriptor */
        selector = (u16)r.eax;
        r.eax = 7; r.ebx = selector;                /* where it begins */
        r.ecx = (u32)Code[i] >> 16; r.edx = (u32)Code[i] & 0xFFFF;
        Int31(&r);
        r.eax = 8; r.ebx = selector; r.ecx = 0; r.edx = 0x1F; Int31(&r);
        r.eax = 9; r.ebx = selector;                /* code, of 32 bits */
        r.ecx = 0x409A | (GetCS() & 3) << 5; Int31(&r);
        pointer.offset = 0; pointer.selector = selector;
        Put(names[i]); Put(" in a segment at "); Hex(SelBase(selector), 8);
        Put(" read "); Hex(FarCall(&pointer), 4); Line();
    }
    Exit(0);
}
