/* The extender's blocks above the megabyte, DPMI's functions 0500h to
   0503h: how much there is, where a block is, whether one moves when it
   is made longer or shorter, what is in one when it is given, and what
   becomes of it. TODO.md, "X, the extender's memory on the bench", has
   what it showed. By keys, each of which writes a screen:

   f  what 0500h says, all of its numbers
   l  the largest block that 0501h gives, found by halving, with its
      address and the extender's name for it; what 0500h and DOS say
      while it is held; it is given back
   a  blocks of 1, 17, 4,096, 4,097 and 65,536 bytes, one behind the
      other: where each is, its name, and what 0500h says behind it
   r  0503h: a block made longer and shorter with nothing behind it, and
      with a block behind it; where it then is and whether it holds what
      it held
   x  a block with nothing behind it made as long as it goes: by what
      0500h says is free and the 16 bytes that the extender keeps in
      front of a block, and one byte more
   g  0502h and 0501h again: is a block that was given back given again
      at the same place; and what is in it then, of what was written
   z  what is in the largest block as it is given: how many of its bytes
      are not zero, and where the first and the last of them are. Before
      any other key, that is the memory as the toy found it: `l` leaves
      the extender's 16 bytes wherever a block of its halving ended
   d  the largest block filled with A5h and kept: the toy ends with it,
      and whatever runs next finds it or not
   h  the largest block taken and kept, for the end
   w  a write behind the end of a block, 16 bytes, where the extender
      has what it knows of the next block: what the extender then says
      to 0502h, 0501h and 0500h, and when the toy ends
   q  the end

   In front of what a key does and behind it the toy asks DOS for its
   version, INT 21h with 30h, with the key in BX: calls.asm, which
   writes down every call that DOS gets, then shows whether DOS is asked
   anything between the two. The toy writes to the screen's memory and
   reads its keys from the BIOS. */
#include "toy.h"

static struct R r;
static u32 info[12];

static void Clear(void)
{
    u16 *screen = (u16 *)0xB8000;
    int i;

    for (i = 0; i < 80 * 25; i++) screen[i] = 0x0720;
    Row = Col = 0;
}

static u32 Free(void)
{
    Zero(&r, sizeof r); r.eax = 0x500; r.edi = (u32)info; Int31(&r);
    return info[0];
}

struct Block { u32 at, name; int failed; };

static struct Block Take(u32 size)
{
    struct Block block;

    Zero(&r, sizeof r);
    r.eax = 0x501; r.ebx = size >> 16; r.ecx = size & 0xFFFF;
    Int31(&r);
    block.failed = (int)(r.flags & CARRY);
    block.at = (r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF);
    block.name = (r.esi & 0xFFFF) << 16 | (r.edi & 0xFFFF);
    return block;
}

static int Give(u32 name)
{
    Zero(&r, sizeof r);
    r.eax = 0x502; r.esi = name >> 16; r.edi = name & 0xFFFF;
    Int31(&r);
    return (int)(r.flags & CARRY);
}

static struct Block Resize(u32 name, u32 size)
{
    struct Block block;

    Zero(&r, sizeof r);
    r.eax = 0x503; r.ebx = size >> 16; r.ecx = size & 0xFFFF;
    r.esi = name >> 16; r.edi = name & 0xFFFF;
    Int31(&r);
    block.failed = (int)(r.flags & CARRY);
    block.at = (r.ebx & 0xFFFF) << 16 | (r.ecx & 0xFFFF);
    block.name = (r.esi & 0xFFFF) << 16 | (r.edi & 0xFFFF);
    return block;
}

static void Say(const char *what, struct Block block)
{
    Put(what);
    if (block.failed) { Put(": refused, AX "); Hex(r.eax, 4); }
    else { Put(": at "); Hex(block.at, 8); Put(", name "); Hex(block.name, 8); }
    Put("; free "); Hex(Free(), 8); Line();
}

/* The largest block that is given, to the byte. */
static u32 Largest(void)
{
    u32 low = 0, high = Free() + 0x10000, middle;
    struct Block block;

    while (low < high) {
        middle = low + (high - low + 1) / 2;
        block = Take(middle);
        if (block.failed) high = middle - 1;
        else { Give(block.name); low = middle; }
    }
    return low;
}

static void Fill(u32 at, u32 size, u8 with)
{
    u8 *to = (u8 *)at;
    while (size--) *to++ = with;
}

/* How many bytes are not `with`, and where the first and the last of
   them are. */
static void Held(const char *what, u32 at, u32 size, u8 with)
{
    const u8 *from = (const u8 *)at;
    u32 count = 0, first = 0, last = 0, i;

    for (i = 0; i < size; i++)
        if (from[i] != with) {
            if (count++ == 0) first = at + i;
            last = at + i;
        }
    Put(what); Put(": of "); Hex(size, 8); Put(" bytes "); Hex(count, 8);
    Put(" are not "); Hex(with, 2);
    if (count) {
        Put(", from "); Hex(first, 8); Put(" to "); Hex(last, 8);
        Put(": "); Hex(*(u32 *)first, 8);
    }
    Line();
}

static void Marker(u8 key, u8 which)
{
    Zero(&r, sizeof r); r.eax = 0x3000; r.ebx = (u32)which << 8 | key;
    Int21(&r);
}

void Main(void)
{
    static const u32 sizes[5] = { 1, 17, 4096, 4097, 65536 };
    struct Block block, behind, small[5];
    u32 size;
    u8 key;
    int i;

    Clear();
    Put("the toy's code at "); Hex((u32)Main, 8);
    Put(", its stack at "); Hex((u32)Stack, 8);
    Put("; free "); Hex(Free(), 8); Line();
    for (;;) {
        Zero(&r, sizeof r); Int28(&r);
        Zero(&r, sizeof r); r.eax = 0x1100; Int16(&r);
        if (r.flags & ZERO) continue;
        Zero(&r, sizeof r); r.eax = 0x1000; Int16(&r);
        key = (u8)r.eax;
        Clear();
        Marker(key, 0xB0);
        switch (key) {
        case 'f':
            Free();
            Put("0500h: the largest free block "); Hex(info[0], 8); Line();
            Put("pages: most not locked "); Hex(info[1], 8);
            Put(", most locked "); Hex(info[2], 8); Line();
            Put("linear space in pages "); Hex(info[3], 8);
            Put(", not locked in all "); Hex(info[4], 8); Line();
            Put("free pages "); Hex(info[5], 8);
            Put(", physical pages "); Hex(info[6], 8); Line();
            Put("free linear space in pages "); Hex(info[7], 8);
            Put(", the paging file "); Hex(info[8], 8); Line();
            break;
        case 'l':
            Put("0500h says "); Hex(Free(), 8); Line();
            size = Largest();
            Put("the largest that 0501h gives is "); Hex(size, 8);
            Put(" bytes; one more is refused"); Line();
            block = Take(size);
            Say("taken", block);
            Zero(&r, sizeof r); r.eax = 0x100; r.ebx = 0xFFFF; Int31(&r);
            Put("DOS's largest block meanwhile, in paragraphs: ");
            Hex(r.ebx, 4); Line();
            Say("a block of 1 byte besides", Take(1));
            Give(block.name);
            Put("given back; 0500h says "); Hex(Free(), 8); Line();
            block = Take(size);
            Say("taken again", block);
            Give(block.name);
            break;
        case 'a':
            for (i = 0; i < 5; i++) {
                small[i] = Take(sizes[i]);
                Hex(sizes[i], 5); Say(" bytes", small[i]);
            }
            for (i = 0; i < 5; i++) {
                Give(small[i].name);
                Put("the "); Dec(i + 1); Put(". given back; free ");
                Hex(Free(), 8); Line();
            }
            break;
        case 'r':
            block = Take(0x100000);
            Say("1 MB", block);
            Fill(block.at, 0x100000, 0x5A);
            block = Resize(block.name, 0x200000);
            Say("made 2 MB, nothing behind it", block);
            Held("its first megabyte", block.at, 0x100000, 0x5A);
            block = Resize(block.name, 0x80000);
            Say("made 512 KB", block);
            Held("it", block.at, 0x80000, 0x5A);
            behind = Take(0x1000);
            Say("4 KB behind it", behind);
            block = Resize(block.name, 0x100000);
            Say("made 1 MB, with that behind it", block);
            Held("its first 512 KB", block.at, 0x80000, 0x5A);
            block = Resize(block.name, 0x40000);
            Say("made 256 KB", block);
            Held("it", block.at, 0x40000, 0x5A);
            block = Resize(block.name, 0x40001);
            Say("made one byte longer", block);
            size = Free() + 0x100000;
            small[0] = Resize(block.name, size);
            Say("made a megabyte longer than there is", small[0]);
            Held("it, where it was", block.at, 0x40000, 0x5A);
            small[0] = Resize(block.name, 0x40002);
            Say("made two bytes longer than it was", small[0]);
            Give(small[0].name);
            Put("given back; free "); Hex(Free(), 8); Line();
            Give(behind.name);
            Put("gave back twice: carry "); Hex(Give(behind.name), 1);
            Put(" AX "); Hex(r.eax, 4); Line();
            break;
        case 'x':
            small[0] = Take(0x10000);
            Say("64 KB", small[0]);
            block = Take(0x100000);
            Say("1 MB behind it", block);
            Fill(block.at, 0x100000, 0x5A);
            size = 0x100000 + Free() + 16;
            behind = Resize(block.name, size + 1);
            Say("made as long as what is free, 17 bytes more", behind);
            block = Resize(block.name, size);
            Say("made as long as what is free, 16 bytes more", block);
            Held("its first megabyte", block.at, 0x100000, 0x5A);
            block = Resize(block.name, 0x2000);
            Say("made 8 KB", block);
            Give(block.name);
            Put("given back; free "); size = Free(); Hex(size, 8); Line();
            block = Take(size);
            Say("all of that taken", block);
            Held("its first megabyte", block.at, 0x100000, 0x5A);
            Give(block.name); Give(small[0].name);
            break;
        case 'g':
            block = Take(0x100000);
            Say("1 MB", block);
            Fill(block.at, 0x100000, 0xC3);
            behind = Take(0x100000);
            Say("1 MB behind it", behind);
            Give(block.name);
            Put("the first given back; free "); Hex(Free(), 8); Line();
            block = Take(0x80000);
            Say("512 KB", block);
            Held("it", block.at, 0x80000, 0xC3);
            small[0] = Take(0x80000 - 16);
            Say("512 KB less 16 bytes", small[0]);
            small[1] = Take(0x100000);
            Say("1 MB", small[1]);
            Give(block.name); Give(small[0].name); Give(small[1].name);
            Give(behind.name);
            Put("all given back; free "); Hex(Free(), 8); Line();
            break;
        case 'z':
            size = Free();
            block = Take(size);
            Say("the largest", block);
            Held("it", block.at, size, 0);
            Give(block.name);
            break;
        case 'd':
            size = Free();
            block = Take(size);
            Say("the largest", block);
            Fill(block.at, size, 0xA5);
            Put("filled with A5, and kept"); Line();
            break;
        case 'w':
            /* The cursor further down, for what the extender may say. */
            Zero(&r, sizeof r); r.eax = 0x200; r.edx = 0x1400; Int10(&r);
            block = Take(0x1000);
            Say("4 KB", block);
            behind = Take(0x1000);
            Say("4 KB behind it", behind);
            small[0] = Take(0x1000);
            Say("and 4 KB behind that", small[0]);
            Put("in front of the second: ");
            for (i = 0; i < 4; i++) {
                Hex(*(u32 *)(behind.at - 16 + 4 * i), 8); Put(" ");
            }
            Line();
            Fill(block.at + 0x1000, 16, 0xFF);
            Put("16 bytes of FF written behind the first"); Line();
            Put("0500h: free "); Hex(Free(), 8); Line();
            i = Give(small[0].name);
            Put("the third given back: carry "); Hex(i, 1);
            Put(" AX "); Hex(r.eax, 4); Line();
            Say("4 KB more", Take(0x1000));
            i = Give(behind.name);
            Put("the second given back: carry "); Hex(i, 1);
            Put(" AX "); Hex(r.eax, 4); Line();
            i = Give(block.name);
            Put("the first given back: carry "); Hex(i, 1);
            Put(" AX "); Hex(r.eax, 4); Put("; free "); Hex(Free(), 8);
            Line();
            break;
        case 'h':
            size = Free();
            block = Take(size);
            Say("the largest, kept", block);
            break;
        case 'q':
            Marker(key, 0xE0);
            Exit(0);
        }
        Marker(key, 0xE0);
    }
}
