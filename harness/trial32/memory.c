/* What memory there is, and by which call: what DPMI's function 0500h
   says is free above the megabyte, INT 21h function 48h, which under an
   extender is not DOS's own, a block of DOS's memory by DPMI's function
   0100h, and three blocks by DPMI's function 0501h, whose addresses are
   linear ones that flat code uses as they are. */
#include "toy.h"

void Main(void)
{
    struct R r;
    u32 info[12];
    int i;

    Zero(&r, sizeof r); r.eax = 0x500; r.edi = (u32)info; Int31(&r);
    Put("DPMI 0500h: the largest free block "); Hex(info[0], 8);
    Put(" bytes; pages in all "); Hex(info[4], 8); Line();
    Zero(&r, sizeof r); r.eax = 0x4800; r.ebx = 0xFFFF; Int21(&r);
    Put("INT 21h 48h for FFFFh paragraphs: carry "); Hex(r.flags & CARRY, 1);
    Put(" AX "); Hex(r.eax, 4); Put(", the largest "); Hex(r.ebx, 4); Line();
    Zero(&r, sizeof r); r.eax = 0x4800; r.ebx = 0x100; Int21(&r);
    Put("INT 21h 48h for 100h: carry "); Hex(r.flags & CARRY, 1);
    Put(" AX "); Hex(r.eax, 4);
    Put(", a selector of a segment at "); Hex(SelBase((u16)r.eax), 8); Line();
    Zero(&r, sizeof r); r.eax = 0x100; r.ebx = 0x100; Int31(&r);
    Put("DPMI 0100h for 100h: carry "); Hex(r.flags & CARRY, 1);
    Put(" segment "); Hex(r.eax, 4); Put(" selector "); Hex(r.edx, 4); Line();
    Zero(&r, sizeof r); r.eax = 0x100; r.ebx = 0xFFFF; Int31(&r);
    Put("DPMI 0100h for FFFFh: carry "); Hex(r.flags & CARRY, 1);
    Put(", the largest "); Hex(r.ebx, 4); Put(" paragraphs"); Line();
    for (i = 0; i < 3; i++) {
        Zero(&r, sizeof r);
        r.eax = 0x501; r.ebx = 0x20 >> i; r.ecx = 0;
        Int31(&r);
        Put("DPMI 0501h for "); Hex(0x200000 >> i, 6);
        Put(" bytes: carry "); Hex(r.flags & CARRY, 1); Put(", at ");
        Hex((r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF), 8); Line();
    }
    Zero(&r, sizeof r); r.eax = 0x500; r.edi = (u32)info; Int31(&r);
    Put("then the largest free block is "); Hex(info[0], 8); Line();
    Exit(0);
}
