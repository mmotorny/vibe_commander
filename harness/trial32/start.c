/* When is the program entered? The ACPI timer where the extender enters
   it, to hold against what rmstart.asm writes for a program of real mode
   on the same bench: the difference is what the extender takes to start
   and to load the file. The extender sets the timer's counter 0 anew, so
   that one cannot be gone by: its status is written too, 36h for mode 3
   where the BIOS leaves 34h, mode 2.

   Linked with big.c or fat.c besides (see big.py), the file is longer,
   with or without relocations: build.sh start d32 big.c. */
#include "toy.h"

void Main(void)
{
    Put("entered at tick "); Hex(Entry[E_TICKS], 8);
    Put(", ACPI timer "); Hex(Entry[E_ACPI] & 0xFFFFFF, 6);
    Put(", counter 0 with its status "); Hex(Entry[E_TIMER], 8); Line();
    Exit(0);
}
