/* What mode the timer's counter 0 is in once the extender has started
   the program, and how it counts: bits 1 to 3 of its status are the
   mode. In mode 3 it counts down by 2, twice to a tick, so a count read
   there is no place in the tick by itself. The toys measure by the ACPI
   timer for that reason. */
#include "toy.h"

u8 Status(void);
#pragma aux Status = "pushfd" "cli" "mov al,0E2h" "out 43h,al" \
    "in al,40h" "popfd" value [al] modify exact [eax];
u16 Count(void);
#pragma aux Count = "pushfd" "cli" "mov al,0" "out 43h,al" "in al,40h" \
    "mov ah,al" "in al,40h" "xchg al,ah" "popfd" value [ax] \
    modify exact [ax];

void Main(void)
{
    u16 last, now;
    int i;

    Put("status of counter 0: "); Hex(Status(), 2);
    Put("; at entry, with the count: "); Hex(Entry[E_TIMER], 8); Line();
    last = Count();
    for (i = 0; i < 10; i++) {
        now = Count();
        Hex(now, 4); Put(" -"); Hex((u16)(last - now), 4); Put("  ");
        last = now;
    }
    Line();
    Exit(0);
}
