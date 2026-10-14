/* Lines that harness/width.py is held against, every time it runs: what
   it is to find in each is in the comment at the end of the line, the
   kinds of the cuts behind `cuts:` or what the line declares behind
   `number:`, and nothing where there is no such comment. It is never
   compiled into anything. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef _Bool bool;

struct Thing {
    u16 word;                           /* number: field */
    u8 bytes[4];                        /* number: field */
    u32 wide;
    bool flag;
    u16 *pointer;
};

u16 Takes(u16 word, u8 byte);
u32 Wide(u32 number);

u16 Check(u16 a, u8 b, u32 c)           /* number: parameter, parameter, returns */
{
    u16 kept = a;                       /* number: variable */
    u8 text[13];                        /* number: variable */
    u32 wide = c;
    bool flag = a != 0;
    struct Thing thing;
    u16 i;                              /* number: variable */

    kept = b;
    kept = a + b;                       /* cuts: store */
    kept = (u16)(a + b);                /* cuts: cast */
    kept = a & 0xFF;
    kept = c;                           /* cuts: store */
    kept = c >> 16;
    kept = (u16)c;                      /* cuts: cast */
    kept = (u8)a;                       /* cuts: cast */
    kept = (u16)b;
    kept = flag;
    kept = a == 3;
    wide = a + b;
    wide = c + 1;
    wide++;
    kept++;                             /* cuts: step */
    --kept;                             /* cuts: step */
    kept += 2;                          /* cuts: step */
    kept -= b;                          /* cuts: step */
    kept &= 3;
    kept |= c;                          /* cuts: store */
    kept <<= 1;                         /* cuts: store */
    text[0] = a;                        /* cuts: store */
    text[1] = b;
    thing.word = c;                     /* cuts: store */
    thing.wide = c;
    thing.flag = c;
    for (i = 0; i < 13; i++)
        text[i] = i;
    for (i = 0; i < a; i++)
        kept = i + 1;
    for (i = 0; i < c; i++)             /* cuts: step */
        ;
    Takes(a, b);
    Takes(a + 1, b);                    /* cuts: handed */
    Takes(a, a);                        /* cuts: handed */
    Takes(c, 0);                        /* cuts: handed */
    Wide(a + b);
    kept = Wide(a);                     /* cuts: store */
    kept = Takes(a, b);
    if (a)
        return c;                       /* cuts: returned */
    if (b)
        return a + 1;                   /* cuts: returned */
    return kept;
}
