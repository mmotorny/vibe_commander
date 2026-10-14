/* What harness/cut.py holds the compiler to, and its own finder: a cut
   to 16 bits that Open Watcom loses, and the form that it keeps. See
   the top of that file, and "A byte with a sign made a word" in
   src/portable_c/vc.h.

   Each function has in front of it the instructions that the build's
   compiler makes of it with the build's options, from its first one to
   its first `ret` or jump. The test compiles this file so and fails for
   a function that comes out otherwise. A function called Lost has the
   form that is wrong, and its instructions are the wrong ones: where
   C says 0000FFFEh for the byte FEh, `movsx eax,al` makes FFFFFFFEh. A
   function called Kept has the form that the tree is written in, or
   another cut that the tree rests on, and its instructions are right.

   The finder of the test, which reads the tree for the form that is
   wrong, has to find every function called Lost here and no other. */

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef int            i32;
typedef unsigned long long u64;

extern u32 Takes(u32 number);
extern u32 Number;
extern const u32 Table[];

/* The form that is wrong, where the number goes on into 32 bits. */

/* is: movsx eax,al; ret */
u32 LostReturned(u8 byte)
{
    return (u16)(signed char)byte;
}

/* is: movsx eax,al; call Takes_; inc eax; ret */
u32 LostHanded(u8 byte)
{
    return Takes((u16)(signed char)byte) + 1;
}

/* is: movsx eax,al; add eax,edx; ret */
u32 LostAdded(u8 byte, u32 to)
{
    u16 line = (u16)(signed char)byte;

    return line + to;
}

/* is: movsx eax,al; cmp eax,edx; setb al; movzx eax,al; ret */
u32 LostCompared(u8 byte, u32 with)
{
    u16 line = (u16)(signed char)byte;

    return line < with;
}

/* Cut to a byte behind it, and wrong all the same: the division is one
   of numbers with a sign. */
/* is: push ecx; push edx; movsx eax,al; mov ecx,0x00000003; cdq; idiv ecx; pop edx; pop ecx; ret */
u8 LostDivided(u8 byte)
{
    return (u8)((u16)(signed char)byte / 3);
}

/* By way of an int, which changes nothing. */
/* is: inc al; movsx eax,al; ret */
u32 LostByInt(u8 byte)
{
    return (u16)(int)(signed char)(byte + 1);
}

/* Kept in a number of 32 bits, as a variable that step 2 of the road
   has widened is. */
/* is: movsx eax,al; lea eax,[eax+eax*4]; mov _Number,eax; ret */
void LostKept(u8 byte)
{
    i32 line = (u16)(signed char)byte;

    Number = line * 5;
}

/* With no cast written, which is the same conversion. */
/* is: movsx eax,al; lea eax,0x7[eax+eax*2]; ret */
u32 LostUnwritten(signed char byte)
{
    u16 line = byte;

    return line * 3 + 7;
}

/* The same fault one size up: a word with a sign made 32 bits without
   one, and that made 64. `cdq` fills the upper half with the sign, where
   C says zeros. The tree has no number of 64 bits, and the finder finds
   any that is written, since it does not look for this form. */
/* is: cwde; cdq; ret */
u64 LostWide(short word)
{
    return (u32)word;
}

/* The form that the tree is written in, in the same places. */

/* is: cbw; movzx eax,ax; ret */
u32 KeptReturned(u8 byte)
{
    return (u16)(short)(signed char)byte;
}

/* is: cbw; movzx eax,ax; jmp L */
u32 KeptHanded(u8 byte)
{
    return Takes((u16)(short)(signed char)byte) + 1;
}

/* is: cbw; movzx eax,ax; add eax,edx; ret */
u32 KeptAdded(u8 byte, u32 to)
{
    u16 line = (u16)(short)(signed char)byte;

    return line + to;
}

/* is: cbw; movzx eax,ax; jmp L */
u32 KeptCompared(u8 byte, u32 with)
{
    u16 line = (u16)(short)(signed char)byte;

    return line < with;
}

/* is: push ecx; push edx; cbw; movzx eax,ax; jmp L */
u8 KeptDivided(u8 byte)
{
    return (u8)((u16)(short)(signed char)byte / 3);
}

/* is: inc al; cbw; movzx eax,ax; ret */
u32 KeptByInt(u8 byte)
{
    return (u16)(short)(signed char)(byte + 1);
}

/* is: cbw; movzx eax,ax; jmp L */
void KeptKept(u8 byte)
{
    i32 line = (u16)(short)(signed char)byte;

    Number = line * 5;
}

/* is: cbw; movzx eax,ax; lea eax,0x7[eax+eax*2]; ret */
u32 KeptUnwritten(signed char byte)
{
    u16 line = (u16)(short)byte;

    return line * 3 + 7;
}

/* is: cbw; movzx eax,ax; mov eax,_Table[eax*4]; ret */
u32 KeptIndexed(u8 byte)
{
    return Table[(u16)(short)(signed char)byte];
}

/* is: movsx eax,al; jmp L */
u32 LostIndexed(u8 byte)
{
    return Table[(u16)(signed char)byte];
}

/* Every other cast that narrows, which the tree writes wherever a
   number is put into fewer bits, and the one that widens a byte with its
   sign: each handed to a function that takes 32 bits, added to a number
   of 32 bits, compared with one, and made an index. These the compiler
   keeps: the cut is in the instructions of each, `movzx` from the
   register of 16 or 8 bits, or the arithmetic is done in that register. */

/* A number of 32 bits with a sign cut to 16 bits. */

/* is: movzx eax,ax; jmp L */
u32 KeptIntWordHanded(i32 number)
{
    return Takes((u16)number) + 1;
}

/* is: movzx eax,ax; add eax,edx; ret */
u32 KeptIntWordAdded(i32 number, u32 to)
{
    return (u16)number + to;
}

/* is: movzx eax,ax; jmp L */
u32 KeptIntWordCompared(i32 number, u32 with)
{
    return (u16)number < with;
}

/* is: movzx eax,ax; mov eax,_Table[eax*4]; ret */
u32 KeptIntWordIndexed(i32 number)
{
    return Table[(u16)number];
}

/* A number of 32 bits without a sign cut to 16 bits. */

/* is: movzx eax,ax; jmp L */
u32 KeptUnsignedWordHanded(u32 number)
{
    return Takes((u16)number) + 1;
}

/* is: movzx eax,ax; add eax,edx; ret */
u32 KeptUnsignedWordAdded(u32 number, u32 to)
{
    return (u16)number + to;
}

/* is: movzx eax,ax; jmp L */
u32 KeptUnsignedWordCompared(u32 number, u32 with)
{
    return (u16)number < with;
}

/* is: movzx eax,ax; mov eax,_Table[eax*4]; ret */
u32 KeptUnsignedWordIndexed(u32 number)
{
    return Table[(u16)number];
}

/* A number of 32 bits with a sign cut to a byte. */

/* is: movzx eax,al; jmp L */
u32 KeptIntByteHanded(i32 number)
{
    return Takes((u8)number) + 1;
}

/* is: movzx eax,al; add eax,edx; ret */
u32 KeptIntByteAdded(i32 number, u32 to)
{
    return (u8)number + to;
}

/* is: movzx eax,al; jmp L */
u32 KeptIntByteCompared(i32 number, u32 with)
{
    return (u8)number < with;
}

/* is: movzx eax,al; jmp L */
u32 KeptIntByteIndexed(i32 number)
{
    return Table[(u8)number];
}

/* A number of 32 bits without a sign cut to a byte. */

/* is: movzx eax,al; jmp L */
u32 KeptUnsignedByteHanded(u32 number)
{
    return Takes((u8)number) + 1;
}

/* is: movzx eax,al; add eax,edx; ret */
u32 KeptUnsignedByteAdded(u32 number, u32 to)
{
    return (u8)number + to;
}

/* is: movzx eax,al; jmp L */
u32 KeptUnsignedByteCompared(u32 number, u32 with)
{
    return (u8)number < with;
}

/* is: movzx eax,al; jmp L */
u32 KeptUnsignedByteIndexed(u32 number)
{
    return Table[(u8)number];
}

/* A number of 16 bits cut to a byte. */

/* is: movzx eax,al; jmp L */
u32 KeptWordByteHanded(u16 number)
{
    return Takes((u8)number) + 1;
}

/* is: movzx eax,al; add eax,edx; ret */
u32 KeptWordByteAdded(u16 number, u32 to)
{
    return (u8)number + to;
}

/* is: movzx eax,al; jmp L */
u32 KeptWordByteCompared(u16 number, u32 with)
{
    return (u8)number < with;
}

/* is: movzx eax,al; jmp L */
u32 KeptWordByteIndexed(u16 number)
{
    return Table[(u8)number];
}

/* A number of 16 bits with a sign cut to a byte. */

/* is: movzx eax,al; jmp L */
u32 KeptShortByteHanded(short number)
{
    return Takes((u8)number) + 1;
}

/* is: movzx eax,al; add eax,edx; ret */
u32 KeptShortByteAdded(short number, u32 to)
{
    return (u8)number + to;
}

/* is: movzx eax,al; jmp L */
u32 KeptShortByteCompared(short number, u32 with)
{
    return (u8)number < with;
}

/* is: movzx eax,al; jmp L */
u32 KeptShortByteIndexed(short number)
{
    return Table[(u8)number];
}

/* A number of 16 bits with a sign made one without. */

/* is: movzx eax,ax; jmp L */
u32 KeptShortWordHanded(short number)
{
    return Takes((u16)number) + 1;
}

/* is: movzx eax,ax; add eax,edx; ret */
u32 KeptShortWordAdded(short number, u32 to)
{
    return (u16)number + to;
}

/* is: movzx eax,ax; jmp L */
u32 KeptShortWordCompared(short number, u32 with)
{
    return (u16)number < with;
}

/* is: movzx eax,ax; mov eax,_Table[eax*4]; ret */
u32 KeptShortWordIndexed(short number)
{
    return Table[(u16)number];
}

/* A byte made 32 bits with its sign, which is meant: `movsx` is right here. */

/* is: movsx eax,al; call Takes_; inc eax; ret */
u32 KeptSignedHanded(u8 number)
{
    return Takes((i32)(signed char)number) + 1;
}

/* is: movsx eax,al; add eax,edx; ret */
u32 KeptSignedAdded(u8 number, i32 to)
{
    return (i32)(signed char)number + to;
}

/* is: movsx eax,al; cmp eax,edx; setl al; movzx eax,al; ret */
u32 KeptSignedCompared(u8 number, i32 with)
{
    return (i32)(signed char)number < with;
}

/* is: movsx eax,al; jmp L */
u32 KeptSignedIndexed(u8 number)
{
    return Table[(i32)(signed char)number];
}

/* A difference of two numbers cut, and a byte with a sign that is added
   to before it is cut: arithmetic, and no conversion of the byte itself,
   so the finder leaves it. */

/* is: sub eax,edx; movzx eax,ax; ret */
u32 KeptDifference(i32 a, i32 b)
{
    return (u16)(a - b);
}

/* is: sub al,dl; movzx eax,al; ret */
u32 KeptByteDifference(i32 a, i32 b)
{
    return (u8)(a - b);
}

/* is: cbw; add edx,eax; movzx eax,dx; ret */
u32 KeptSum(signed char byte, u16 to)
{
    return (u16)(byte + to);
}
