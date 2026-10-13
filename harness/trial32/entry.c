/* Does a flat 32-bit program run on the bench, and what is it handed?
   The registers where the extender enters it, where its segments begin,
   what the extender's DPMI and the extender say of themselves, the PSP
   as DOS answers it, and the privilege level. */
#include "toy.h"

u32 Cr0(void);
#pragma aux Cr0 = "mov eax,cr0" value [eax] modify exact [eax];

void Main(void)
{
    struct R r;
    u16 psp = (u16)Entry[E_ES];

    Put("entered at tick "); Hex(Entry[E_TICKS], 8);
    Put(" ESP "); Hex(Entry[E_ESP], 8);
    Put(" flags "); Hex(Entry[E_FLAGS], 8); Line();
    Put("CS "); Hex(Entry[E_CS], 4); Put(" DS "); Hex(Entry[E_DS], 4);
    Put(" ES "); Hex(Entry[E_ES], 4); Put(" SS "); Hex(Entry[E_SS], 4);
    Put(" FS "); Hex(Entry[E_FS], 4); Put(" GS "); Hex(Entry[E_GS], 4);
    Put(" Main at "); Hex((u32)Main, 8);
    Put(" stack from "); Hex((u32)Stack, 8); Line();
    Put("base of CS "); Hex(SelBase(GetCS()), 8);
    Put(" DS "); Hex(SelBase(GetDS()), 8);
    Put(" ES, the PSP "); Hex(SelBase(psp), 8); Line();
    r.eax = 0x400; Int31(&r);
    Put("DPMI 0400h: version "); Hex(r.eax, 4); Put(" flags "); Hex(r.ebx, 4);
    Put(" processor "); Hex(r.ecx & 255, 2);
    Put(" carry "); Hex(r.flags & CARRY, 1); Line();
    r.eax = 0x6200; Int21(&r);
    Put("INT 21h 62h: PSP "); Hex(r.ebx, 8);
    Put(", which begins at "); Hex(SelBase((u16)r.ebx), 8); Line();
    /* DOS/32A's own call: EAX "D32A", CH 0 raw, 1 XMS, 2 VCPI, 3 DPMI. */
    Zero(&r, sizeof r); r.eax = 0xEEFF; Int31(&r);
    Put("INT 31h EEFFh: carry "); Hex(r.flags & CARRY, 1);
    Put(" EAX "); Hex(r.eax, 8);
    Put(" kind of system "); Hex(r.ecx >> 8 & 255, 2);
    Put(" version "); Hex(r.edx, 4); Line();
    Put("privilege level "); Hex(GetCS() & 3, 1);
    if ((GetCS() & 3) == 0) { Put(" CR0 "); Hex(Cr0(), 8); }
    Line();
    Exit(0);
}
