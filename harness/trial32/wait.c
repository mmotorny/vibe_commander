/* A key waited for inside the BIOS, INT 16h function 10h, with no idle
   call: the driver hands the next key over when the wait begins. q ends
   it. */
#include "toy.h"

void Main(void)
{
    struct R r;

    for (;;) {
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        Put("key "); Hex(r.eax, 4); Put(" at tick "); Hex(Ticks(), 8);
        Line();
        if ((u8)r.eax == 'q') break;
    }
    Exit(0);
}
