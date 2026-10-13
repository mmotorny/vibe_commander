/* What the driver reads of the program: AutoTim and Clock, which are in
   the toy's data, above the megabyte, where the driver's DS:[offset]
   does not reach: the idle call arrives in real mode with DS 0.

   t has the toy do something by itself nine ticks later, as the program
   goes to a directory when the cursor of a tree has rested, with AutoTim
   set meanwhile. c switches a clock on that blinks. q ends it.

   With the driver as it is the step "t" ends at once, before the toy has
   done its thing. With DRIVERX.COM told where the two are (run.py
   --lauto, --lclock, with the addresses that the toy's first line has),
   it ends when "done by itself" is on the screen, and a step behind "c"
   takes the 20 ticks for which the driver watches what blinks.

   That was the trial. The driver of the tests reads above the megabyte
   itself since, and harness/drivercheck.py holds it to this toy: as it
   is here, and once more with the two variables in a main data, `md`,
   at the offsets that the original has them at, which that script
   defines in front of this file. */
#include "toy.h"

#ifndef AutoTim
u32 AutoTim = 0xFFFFFFFFUL;
u8 Clock = 0;
#endif

void Main(void)
{
    struct R r;
    u32 began = 0, blinked = 0;
    u8 *cell = (u8 *)0xB8000 + 20 * 160;

    Put("AutoTim at "); Hex((u32)&AutoTim, 8); Put(" Clock at ");
    Hex((u32)&Clock, 8); Line();
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        if ((u16)AutoTim != 0xFFFF && Ticks() - began >= 9) {
            Put("done by itself at "); Hex(Ticks(), 8); Line();
            AutoTim = 0xFFFFFFFFUL;
        }
        if (Clock && Ticks() - blinked >= 9) {
            *cell ^= ':' ^ ' ';
            blinked = Ticks();
        }
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        Put("key "); Hex(r.eax, 4); Put(" at "); Hex(Ticks(), 8); Line();
        if ((u8)r.eax == 't') { began = Ticks(); AutoTim = began + 9; }
        if ((u8)r.eax == 'c') { Clock = 1; *cell = ':'; blinked = Ticks(); }
        if ((u8)r.eax == 'q') break;
    }
    Exit(0);
}
