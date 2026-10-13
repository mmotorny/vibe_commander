/* CauseWay stops with exception 0Dh at the first software interrupt of
   any program on the bench, at 00EF:000000B5, in `mov ds,cs:[0Ch]`. This
   reads CauseWay's descriptor tables before any interrupt is made, to
   show that the word it loads there is a good selector: so the toy calls
   nothing, and does not end; run.py prints the screen it is stuck on
   when its --limit is over. See csover.c for the fault itself. */
#include "toy.h"

void Sgdt(u8 *six);
#pragma aux Sgdt = "sgdt [eax]" parm [eax] modify exact [];
u16 Sldt(void);
#pragma aux Sldt = "sldt ax" value [ax] modify exact [ax];

static u32 Base(const u8 *table, u32 selector)
{
    const u8 *d = table + (selector & ~7UL);

    return d[2] | d[3] << 8 | (u32)d[4] << 16 | (u32)d[7] << 24;
}

static void Descriptor(const char *name, const u8 *table, u32 selector)
{
    const u8 *d = table + (selector & ~7UL);
    int i;

    Put(name); Put(" "); Hex(selector, 4); Put(": ");
    for (i = 0; i < 8; i++) { Hex(d[i], 2); Put(" "); }
    Put(" begins at "); Hex(Base(table, selector), 8); Line();
}

void Main(void)
{
    u8 gdtr[6];
    const u8 *gdt, *ldt, *api;
    u16 ldtr;
    int i;

    Sgdt(gdtr);
    gdt = (const u8 *)(*(u32 *)(gdtr + 2));
    ldtr = Sldt();
    Put("GDT at "); Hex((u32)gdt, 8); Put(" LDTR "); Hex(ldtr, 4);
    Put(" CS "); Hex(GetCS(), 4); Line();
    ldt = (const u8 *)Base(gdt, ldtr);
    Descriptor("this program's CS", ldt, GetCS());
    Descriptor("CauseWay's code", ldt, 0xEF);
    api = (const u8 *)Base(ldt, 0xEF);
    Put("the word at 00EF:000C, which it loads into DS: ");
    Hex(api[0xC] | api[0xD] << 8, 4); Line();
    Descriptor("that selector", gdt, api[0xC] | api[0xD] << 8);
    Put("the word at linear 0000000C, which QEMU loads: ");
    Hex(*(u16 *)0xC, 4); Line();
    Put("at 00EF:00AB ");
    for (i = 0; i < 18; i++) { Hex(api[0xAB + i], 2); Put(" "); }
    Line();
    for (;;) ;
}
