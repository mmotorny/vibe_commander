/* Is the memory that the file does not hold zero when the program is
   entered, and where does the extender put the two objects of the file,
   the code and the data? Linked without the `order` of build.sh, the
   200,000 bytes are in the file, as zeros, since the stack is behind
   them and has a value. */
#include "toy.h"

static u8 Big[200000];
static u32 Mark = 0x12345678;

void Main(void)
{
    u32 i, set = 0, first = 0;

    for (i = 0; i < sizeof Big; i++)
        if (Big[i]) { if (!set) first = i; set++; }
    Put("of 200000 bytes that are not in the file, "); Dec(set);
    Put(" are not 0");
    if (set) { Put(", the first at "); Dec(first); }
    Line();
    Put("code at "); Hex((u32)Main, 8); Put(", data at "); Hex((u32)&Mark, 8);
    Put(", those bytes at "); Hex((u32)Big, 8);
    Put(", the stack "); Hex((u32)Stack, 8);
    Put(" to "); Hex(Entry[E_ESP], 8); Line();
    Exit(0);
}
