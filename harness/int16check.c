/*
 * The lines that harness/int16.py is held against, every time it runs.
 *
 * It is no part of any program and is never compiled: clang reads it as
 * it reads a file of the portable C, for a machine whose int has 32
 * bits, and the finder has to find in each line the kinds that the line
 * names behind "finds:", and nothing in a line that names none. A line
 * is here for what the two builds make of it, worked out by hand: the
 * comment of a group says how.
 *
 * a, b and x are numbers of 16 bits, c and d bytes, w and y numbers of
 * 32 bits, s a byte with a sign.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;           /* as vc.h has it where an int has 32 */

enum { FIRST, SECOND = 5, BELOW = -2 };

#define HALF(number) ((number) >> 1)
#define FAR_OFF 40000               /* a long on DOS */
#define HIGH 0x9C40                 /* the same number, and no long there */

extern u16 takes16(u16 number);
extern u32 takes32(u32 number);
extern u8 bytes[];

/* A word as the words of the addresses in vc.h are, and named among
   them in harness/int16.py: what is written in it is no place, and
   what it is handed is read. */
#define PLACE(pointer) \
    ((u16)(((pointer) - bytes) >> 4))

u16 same(u16 a, u16 b, u8 c, u8 d, u32 w, u8 *p, signed char s)
{
    u16 x;
    u32 y;

    /* Put back into 16 bits before anything looks at it. */
    x = a + b;
    x = a - b;
    x = a * b + c;
    x = takes16(a + b);
    x = (u16)(a + b) >> 1;
    x = (u16)(a - b) / 3;
    x = (u16)~a >> 1;
    x = c << 8 | d;
    x = sizeof(u32) - a;
    x = p - bytes;
    x += a - b;
    if ((u16)(a + b) < a)
        x = 0;
    if ((u16)(a - b) < c)
        x = 0;

    /* Nothing has left 16 bits. */
    x = HALF(a);
    x = a >> 1;
    x = (a | b) >> 1;
    x = (a ^ 0xFFFF) >> 1;
    x = a / b;
    if (a > SECOND)
        x = 0;
    if (a == 0xFFFF)
        x = 0;
    if (a > sizeof(u32))
        x = 0;
    x = p[a];
    p -= a;
    y = w + a;
    y = (u32)a * b;
    y = (u32)a << 16;
    y = (a & 0xFF) << 8;
    y = takes32(a);

    /* A mask of 16 bits or fewer cuts the rest off. */
    x = a & (b - 1);
    if ((a - b & 0xFF) < c)
        x = 0;
    y = a + b & 0xFFFF;
    x = (a & 0xFF00 | a + c & 0xFF) >> 8;

    /* Whether a number is 0, and whether two are the same number, when
       they cannot be 65,536 apart. */
    if (a - b)
        x = 0;
    if (a - b == 0)
        x = 0;
    if (a != b)
        x = 0;
    x = a - c ? 1 : 2;

    /* Bytes are ints with a sign in both builds, and fit one of 16
       bits unless they are multiplied or shifted up. */
    if (c - d < 0)
        x = 0;
    if (c + d > 255)
        x = 0;
    if (c > s)
        x = 0;
    y = c - d;
    x = (c + d) >> 1;
    x = (u16)(signed char)c + a;
    /* On DOS the two bytes are an int that may be below 0, and are
       made a number without a sign for the other side; the same
       number where an int has 32 bits. */
    if ((c << 8 | d) > a)
        x = 0;

    /* A variable that a loop counts with is known inside the loop:
       from where it begins to below the limit, or below the greatest
       number that the limit can be. */
    for (x = 0; x < 13; x++)
        bytes[x] = bytes[100 + x];
    for (x = 1; x <= 11; x++)
        if (bytes[x - 1] == 100 + x)
            break;
    for (x = 0; x != 80; x++)
        y = (x + 1) * 800;
    for (x = 0; x < c; x++)
        y = x + 0xFF00;
    for (x = 40; x != 0; x--)
        y = x - 1;

    /* 40,000 written in decimal is a long on DOS: the sum is made in
       32 bits there too. */
    y = a + FAR_OFF;

    /* What is written in the second body of a word. */
    x = PLACE(p);

    (void)(a - b, c);
    return a - b;
}

u32 differs(u16 a, u16 b, u8 c, u8 d, u32 w, u8 *p, signed char s)
{
    u16 x;
    u32 y;
    u32 z = a - 1;                      /* finds: widen */
    u8 e = a + b >> 8;                  /* finds: shift */

    /* A sum or a difference of 16 bits, shifted or divided. */
    x = (a + b) >> 1;                   /* finds: shift */
    x = HALF(a + b);                    /* finds: shift */
    x = ~a >> 1;                        /* finds: shift */
    x >>= b - 1;                        /* finds: shift */
    x = a << (b - 1);                   /* finds: shift */
    x = (a - b) / 3;                    /* finds: divide */
    x = (a - b) % c;                    /* finds: divide */
    x = a / (b + 1);                    /* finds: divide */
    x /= a - b;                         /* finds: divide */
    x = (p - bytes) >> 1;               /* finds: shift */

    /* Held against another number. */
    if (a + b < a)                      /* finds: compare */
        x = 0;
    if (a - b < c)                      /* finds: compare */
        x = 0;
    if (a - b >= 0)                     /* finds: compare */
        x = 0;
    if (a + b == 0)                     /* finds: compare */
        x = 0;
    if (a - b == 1)                     /* finds: compare */
        x = 0;
    if (a == b + 1)                     /* finds: compare */
        x = 0;
    if (~a == b)                        /* finds: compare */
        x = 0;
    if (-a < b)                         /* finds: compare */
        x = 0;
    if (a << 4 > b)                     /* finds: compare */
        x = 0;
    if (a - 1 != b)                     /* finds: compare */
        x = 0;
    if (a + 1 > sizeof(u32))            /* finds: compare */
        x = 0;
    if (p - bytes < a)                  /* finds: compare */
        x = 0;
    if ((s ? a - b : 0) < c)            /* finds: compare */
        x = 0;
    for (x = 0; x - a < b; x++)         /* finds: compare */
        ;
    switch (a - b) {                    /* finds: compare */
    case 1:
        break;
    }

    /* A loop whose variable is not known: one that is changed inside
       it, one whose limit is no constant and may be anything, one that
       ends at an unequal limit which may never be met, and the
       variable behind the loop. */
    for (x = 0; x < 13; x++) {
        y = x + 0xFFFA;                 /* finds: widen */
        x += c;
    }
    for (x = 0; x < a; x++)
        y = x + 2;                      /* finds: widen */
    for (x = 0; x != a; x++)
        y = x + 1;                      /* finds: widen */
    for (x = 0; x < 13; x++)
        ;
    y = x + 0xFFFA;                     /* finds: widen */
    for (x = 0; x < 13; x++) {
entered:
        y = x + 0xFFFA;                 /* finds: widen */
    }
    if (c)
        goto entered;

    /* Asked whether it is 0, where it can be 65,536. */
    if (a + b)                          /* finds: truth */
        x = 0;
    if (!(a * b))                       /* finds: truth */
        x = 0;
    if (a + b && c)                     /* finds: truth */
        x = 0;
    x = a + b ? 1 : 2;                  /* finds: truth */
    while (a << 4)                      /* finds: truth */
        a--;
    do {
        x++;
    } while (a + x);                    /* finds: truth */

    /* Made a number of 32 bits. */
    y = a + b;                          /* finds: widen */
    y = a * b;                          /* finds: widen */
    y = (u32)(a * b);                   /* finds: widen */
    y = w + (a - b);                    /* finds: widen */
    y += a + 1;                         /* finds: widen */
    y = takes32(a + b);                 /* finds: widen */
    y = a << 16;                        /* finds: widen */
    y = (a & 0xFFF) << 8;               /* finds: widen */
    y = a ? b + 1 : 0;                  /* finds: widen */
    y = a + HIGH;                       /* finds: widen */
    /* A mask of more than 16 bits is a long on DOS, and the sum is made
       one for it. */
    if ((a + b & 0x1FFFF) < c)          /* finds: widen */
        x = 0;
    y = sizeof(u32) - a;                /* finds: widen */
    y = p - bytes;                      /* finds: widen */

    /* Bytes multiplied or shifted up: an int that has a sign on DOS
       from 8000h on. */
    y = c << 8;                         /* finds: widen */
    y = c * d;                          /* finds: widen */
    if (c * d > 1000)                   /* finds: compare */
        x = 0;
    if ((c << 8) > 1000)                /* finds: compare */
        x = 0;

    /* A number with a sign that may be below 0, against one of 16 bits
       without. */
    if (a == -1)                        /* finds: signed */
        x = 0;
    if (a > BELOW)                      /* finds: signed */
        x = 0;
    if (a > c - d)                      /* finds: signed */
        x = 0;
    if (a > s)                          /* finds: signed */
        x = 0;
    x = (c - d) / a;                    /* finds: signed */

    /* Added to a pointer, or made one. */
    x = p[a - 1];                       /* finds: back */
    x = *(p + (a + b));                 /* finds: index */
    p += a - b;                         /* finds: back */
    p += a + c;                         /* finds: index */
    x = p[a * 2];                       /* finds: index */
    x = PLACE(p + (a - 1));             /* finds: back */
    p = (u8 *)(a + 1);                  /* finds: address */

    /* A number written below 0, kept in 16 bits without a sign: the
       same number in both builds, 65,376, and found for where it goes.
       One that is worked out is not found, nor one that is put back
       into a byte or added where it stands. */
    x = (u16)-160;                      /* finds: minus */
    x = -160;                           /* finds: minus */
    x = takes16(-160);                  /* finds: minus */
    x = takes16((u16)-(2 * 80));        /* finds: minus */
    x = BELOW;                          /* finds: minus */
    if (a == (u16)-2)                   /* finds: minus */
        x = 0;
    x = -a;
    x = a - 160;
    x = a + -160;
    c = -1;
    y = takes32(-160);
    x = 0xFF60;

    (void)(e + z + y + x);
    return a - b;                       /* finds: widen */
}
