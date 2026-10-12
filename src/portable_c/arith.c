/*
 * Arithmetic on numbers of 32 bits for the compiled code: what the
 * compiler calls where it multiplies two of them or divides one by
 * another. The C library has these, and the program has no C library.
 *
 * The compiler calls them by names of its own and with registers of its
 * own, which is what the two #pragma aux below say: they are no choice
 * of the program's. It calls them far, as it calls every function of
 * another file in the model that this tree is compiled in, so a file of
 * either code segment may multiply and divide such numbers.
 *
 * Neither may do what it is there for. A word times a word as a number
 * of 32 bits would be a call of __U4M, and any division of a number of
 * 32 bits one of __U4D: the functions would call themselves. What they
 * are made of needs no helper: a word times a word as a word, a word
 * divided by a word, and numbers of 32 bits added, subtracted, compared
 * and shifted by a constant. Read the listing,
 * build/portable_c/arith.lst, after a change: it must have no `call`.
 *
 * They are not the original's: it multiplies and divides where it needs
 * to, in registers. And no scenario would show one of them wrong for
 * some numbers only, so they are held against other arithmetic:
 * harness/arithcheck.py runs them, as the compiler made them, on a model
 * of the processor, with many pairs of numbers. Run it after a change
 * here. The two routines in assembly whose place they took passed it on
 * the same numbers; TODO.md has what else was done.
 */
#include "vc.h"

typedef unsigned long long u64;

/* The numbers come as the compiler has them, the first in DX:AX and the
   second in CX:BX, each word an argument of its own: a number of 32 bits
   in two registers has its halves where the compiler likes.

   The product comes back in DX:AX and nothing else changes, BX and CX
   neither. The quotient comes back in DX:AX and the remainder in CX:BX,
   which is one register more than a function returns a number of 32
   bits in. A number of 64 bits is returned in four, AX, BX, CX and DX
   from its highest word down, whichever order they are written in: U4D
   returns such a number, made of the four words as the registers want
   them. */
u32 U4M(u16 low, u16 high, u16 byLow, u16 byHigh);
u64 U4D(u16 low, u16 high, u16 byLow, u16 byHigh);
#pragma aux U4M "__U4M" __far parm [ax] [dx] [bx] [cx] value [dx ax] modify exact [ax dx]
#pragma aux U4D "__U4D" __far parm [ax] [dx] [bx] [cx] value [ax bx cx dx] modify exact [ax bx cx dx]

/* What U4D returns, as the words of a number of 64 bits from the lowest
   up, each named after the register that it comes back in. */
union Divided {
    u64 registers;
    struct {
        u16 dx;                         /* the quotient, its high word */
        u16 cx;                         /* the remainder, its high word */
        u16 bx;                         /* the remainder, its low word */
        u16 ax;                         /* the quotient, its low word */
    } word;
};

/* The product of two numbers, the low 32 bits of it. */
u32 U4M(u16 low, u16 high, u16 byLow, u16 byHigh)
{
    u16 a0 = low & 0xFF, a1 = low >> 8;
    u16 b0 = byLow & 0xFF, b1 = byLow >> 8;
    u32 product;

    /* The low words: byte by byte, since the product of two bytes is a
       word. */
    product = (u32)(a0 * b0)
              + ((u32)(a0 * b1) << 8)
              + ((u32)(a1 * b0) << 8)
              + ((u32)(a1 * b1) << 16);
    /* A low word by the other number's high one counts from bit 16 on,
       so its low word is all of it that the 32 bits have. The two high
       words together count from bit 32. */
    return product
           + ((u32)(u16)(low * (unsigned)byHigh + high * (unsigned)byLow)
              << 16);
}

/* A number divided by another: the quotient and the remainder. By 0 it
   is the processor's divide error, as it is where the program divides
   words. */
u64 U4D(u16 low, u16 high, u16 byLow, u16 byHigh)
{
    union Divided answer;
    u32 by = (u32)byHigh << 16 | byLow;
    u32 rest = high;
    u16 top = 0;
    u16 carry, i;

    /* The high word. Divided by a word it is a division of words. By
       more than a word its quotient is 0, and it is what is left. */
    if (byHigh == 0) {
        top = high / byLow;
        rest = high % byLow;
    }
    /* The low word, a bit at a time: what is left takes the next bit of
       it behind its own, and has the divisor taken off if it is no
       smaller then. It is smaller than the divisor before, so twice it
       may be more than 32 bits hold: `carry`. The bits of the quotient
       go into the low word from below, as its own leave at the top. */
    for (i = 16; i; i--) {
        carry = (u16)(rest >> 16) & 0x8000;
        rest = rest << 1 | low >> 15;
        low <<= 1;
        if (carry || rest >= by) {
            rest -= by;
            low |= 1;
        }
    }
    answer.word.dx = top;
    answer.word.ax = low;
    answer.word.cx = (u16)(rest >> 16);
    answer.word.bx = (u16)rest;
    return answer.registers;
}
