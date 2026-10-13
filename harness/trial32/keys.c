/* Idle and keys. The toy goes round as the program does: INT 28h, a look
   at the mouse, a look for a key; a key is read and written out with the
   shift state, a button of the mouse with where the pointer is. q ends
   it. Under the driver of the tests as it is, each step of a script
   should end with its key on the screen: the driver has then taken the
   idle call for the program's and the keys have come through the
   extender. AutoTim and Clock are here as the driver would look for
   them: it cannot see them, see auto.c. */
#include "toy.h"

u32 AutoTim = 0xFFFFFFFFUL;
u8 Clock = 0;

void Main(void)
{
    struct R r;
    u32 rounds = 0;
    u16 key;
    int held = 0;

    Put("AutoTim at "); Hex((u32)&AutoTim, 8); Put(" Clock at ");
    Hex((u32)&Clock, 8); Put(" ESP "); Hex(GetESP(), 8); Line();
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        rounds++;
        Zero(&r, sizeof r); r.eax = 3; Int33(&r);
        if (r.ebx & 7) {
            if (!held) {
                Put("mouse "); Hex(r.ebx, 2); Put(" at "); Dec(r.ecx / 8);
                Put(","); Dec(r.edx / 8); Line();
            }
            held = 1;
            continue;
        }
        held = 0;
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Put("waiting "); Hex(r.eax, 4);
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        key = (u16)r.eax;
        Put(" key "); Hex(key, 4);
        Zero(&r, sizeof r); r.eax = 0x0200; Int16(&r);
        Put(" shift "); Hex(r.eax & 255, 2);
        Put(" at 417h "); Hex(*(u8 *)0x417, 2);
        Put(" rounds "); Dec(rounds); Line();
        if ((u8)key == 'q') break;
    }
    Exit(0);
}
