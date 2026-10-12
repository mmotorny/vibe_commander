/*
 * The part of the program that stays in the memory while a command runs.
 *
 * It begins with the PSP and is one segment, as the program was when it
 * was a .COM file: the code here, the data, which port/asm/VC.ASM lays
 * out behind it, and the stack at the end. DS is that segment in every
 * function of this file, where vc.h says that DS is the main data: the
 * main data is not in the memory while a command runs, and neither is
 * the C code of the rest of the program. So nothing here uses `md` or
 * calls a function of another file but those of dos.c, of which the
 * build compiles a second copy into this part (see RESIDENT in vc.h).
 *
 * What is here is the program's watch on DOS. While the program runs, and
 * while a command runs from it, every call of DOS comes by New21: it notes
 * which program is being run, and when one stays in the memory, by DOS's
 * call for that or by INT 27h, it writes down who it is and which
 * interrupts it has taken, in a record behind the program's memory. With
 * those records the map of the memory, Alt+F5, lists the programs, and
 * Release takes one out again.
 */
#include "vc.h"

#define MaxProg 18                      /* Maximal resident quantaty */
#define VcApiVr 42

/* The data of this part, in RDATA of port/asm/VC.ASM: the interrupts as
   they were when a program was last run, with what the BIOS keeps at
   40h:A8h behind them; where INT 21h and INT 27h went before the program
   took them; the programs that have stayed, each by the segment of its
   record; the name of the program run last; and what is kept of the
   panels and of the main data while a command runs. Of ComnIni only as
   much is there as the main data has before DosLin. */
extern u32 __near IntTab[256];
extern u32 __near SavePtr;
extern u32 __near Vect21;
extern u32 __near Vect27;
extern u16 __near Counter;
extern u16 __near PrgList[MaxProg];
extern u8 __near NameBuf[12];
extern u8 __near WCB1sav[];
extern u8 __near WCB2sav[];
extern struct MainDat __near ComnIni;
#pragma aux IntTab "*"
#pragma aux SavePtr "*"
#pragma aux Vect21 "*"
#pragma aux Vect27 "*"
#pragma aux Counter "*"
#pragma aux PrgList "*"
#pragma aux NameBuf "*"
#pragma aux WCB1sav "*"
#pragma aux WCB2sav "*"
#pragma aux ComnIni "*"

/* This segment, as a number: DS. */
#define RES_SEG ((u16)((u32)(void __far *)&Counter >> 16))

#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define DOS_FREE     0x4900             /* DOS: free the block in ES */
#define DOS_RESIZE   0x4A00             /* make the block in ES BX
                                           paragraphs long */
#define DOS_LISTS    0x5200             /* where DOS keeps its lists, to
                                           ES:BX */
#define DOS_STRATEGY 0x5800             /* how DOS looks for memory: AL 0
                                           to ask, 1 to say, 2 and 3 the
                                           same for whether the upper
                                           memory is in the chain */
#define DOS_PSP      0x6200             /* the current program, to BX */

#define SAVE_PTR 0x4A8                  /* where the BIOS keeps the address
                                           of its tables for the screen */

#define FLAG_CARRY     0x0001
#define FLAG_PARITY    0x0004
#define FLAG_AUX       0x0010
#define FLAG_ZERO      0x0040
#define FLAG_SIGN      0x0080
#define FLAG_TRAP      0x0100
#define FLAG_INTERRUPT 0x0200
#define FLAG_OVERFLOW  0x0800

/* A record of a program that has stayed in the memory, in a segment of
   its own behind what the program keeps: the program's PSP, its name,
   how many interrupts it has taken, then for each its number and where
   it went before, five bytes, then what the BIOS had at SAVE_PTR, and
   the sum of all these bytes. */
#define REC_PSP     0
#define REC_NAME    2
#define REC_VECTORS 14
#define REC_FIRST   15
#define REC_PARAS   ((2 + 12 + 1 + 5 * 256 + 4 + 2 + 15) >> 4)

/* The registers as the compiler's interrupt functions save them on the
   stack, and the return address that an interrupt leaves there. */
struct Saved {
    u16 gs, fs, es, ds, di, si, bp, sp, bx, dx, cx, ax;
};
struct Frame {
    u16 ip, cs, flags;
};

/* What the assembly of this part still calls: near, with DS on this
   segment, and every register kept but AX. */
void Release(u16 record);
u16 GetStat(void);
void SetStat(u16 state);
#pragma aux Release "*_r" __near parm [ax] modify exact [ax]
#pragma aux GetStat "*_r" __near value [ax] modify exact [ax]
#pragma aux SetStat "*_r" __near parm [ax] modify exact [ax]

/*---------------------- Check sum ----------------------
   Whether the record in the segment `record` is whole: 1 if its sum is
   right. */
static u16 ChkBlk(u16 record)
{
    const u8 __far *byte = AT(record, 0);
    u16 length = (u16)byte[REC_VECTORS] * 5 + (2 + 12 + 1 + 4);
    u16 sum = 0;

    do
        sum += *byte++;
    while (--length);
    return sum == *(const u16 __far *)byte;
}

/*------------------ Free memory block ------------------
   Take off the list the program whose PSP is the block `block`, which is
   being freed, and every record that is no longer whole. */
static void FreeBlk(u16 block)
{
    u16 count = Counter, from, to = 0;

    if (count == 0)
        return;
    for (from = 0; from < count; from++) {
        u16 record = PrgList[from];

        if (ChkBlk(record) && block != WORD_AT(record, REC_PSP))
            PrgList[to++] = record;
    }
    Counter -= from - to;
}

/*------------- Save vectors & program name -------------
   Note the interrupts as they are, and the name of the file `name`
   without its path, for a program that DOS is about to run. Twelve
   bytes are taken from where the name begins, however long it is. */
static void ExeProg(const u8 __far *name)
{
    /* The segment of the table of vectors. Written as a constant, the
       compiler takes a far pointer into segment 0 for a near one. */
    volatile u16 table = 0;
    const u32 __far *vector = (const u32 __far *)AT(table, 0);
    const u8 __far *begin;
    u16 i;

    IntOff();
    for (i = 0; i < 256; i++)
        IntTab[i] = vector[i];
    SavePtr = *(const u32 __far *)AT(table, SAVE_PTR);
    IntOn();
    for (begin = name; ; ) {
        u8 c = *name++;

        if (c == ':' || c == '\\')
            begin = name;
        else if (c == 0)
            break;
    }
    for (i = 0; i < 12; i++)
        NameBuf[i] = begin[i];
}

/*------------- Add resident program to list ------------
   A program is about to stay in the memory with as many paragraphs as
   `r` has in DX. Make its record behind them, with the interrupts that
   have changed since it was run, and add the record's paragraphs to DX.
   Returns 1 if the program is to end instead: the list is full, the
   bell sounds and the interrupts are put back as they were.

   Ported as it is: if DOS will not make the program's block longer for
   the record, the program stays without one and is not listed. */
static u16 AddProg(struct Saved STACK *r)
{
    volatile u16 table = 0;
    u32 __far *vector = (u32 __far *)AT(table, 0);
    struct DosRegs regs;
    struct BiosRegs bios;
    u8 __far *record;
    u8 __far *to;
    const u8 __far *byte;
    u16 psp, segment, i, changed, length, sum, more;

    IntOn();
    if (Counter >= MaxProg) {
        bios.ax = 0x0E07;
        bios.bx = r->bx;
        bios.cx = r->cx;
        bios.dx = r->dx;
        Int10(&bios);
        IntOff();
        for (i = 0; i < 256; i++)
            vector[i] = IntTab[i];
        *(u32 __far *)AT(table, SAVE_PTR) = SavePtr;
        IntOn();
        return 1;
    }
    regs.ax = DOS_PSP;                  /* Get current PSP */
    Int21e(&regs);
    psp = regs.bx;
    FreeBlk(psp);
    regs.ax = DOS_RESIZE;
    regs.bx = r->dx + REC_PARAS;
    regs.es = psp;
    if (Int21e(&regs))
        return 0;
    segment = psp + r->dx;
    record = AT(segment, 0);
    IntOff();
    *(u16 __far *)record = psp;         /* PSP -> PrgSeg */
    to = record + REC_NAME;
    for (i = 0; i < 12; i++)            /* File name -> PrgSeg */
        *to++ = NameBuf[i];
    to++;
    changed = 0;                        /* Changed vectors -> PrgSeg */
    for (i = 0; i < 256; i++) {
        if (IntTab[i] == vector[i] || i == 0x22)
            continue;
        *to++ = (u8)i;                  /* Save vector */
        *(u32 __far *)to = IntTab[i];
        to += 4;
        changed++;
    }
    record[REC_VECTORS] = (u8)changed;
    *(u32 __far *)to = SavePtr;         /* SAVE PTR -> PrgSeg */
    to += 4;
    length = (u16)(u32)to;              /* check sum -> PrgSeg */
    sum = 0;
    for (byte = record, i = length; i; i--)
        sum += *byte++;
    *(u16 __far *)to = sum;
    PrgList[Counter] = segment;         /* Save PrgSeg */
    Counter++;
    IntOn();
    more = ((length + 1) >> 4) + 1;     /* Add PrgSeg size */
    r->dx += more;
    return r->dx < more;
}

/*------------------- Release programs ------------------
   Take the program whose record is in the segment `record` out of the
   memory: give the interrupts that it took back to who had them before,
   and free every block of memory that is its own. */
void Release(u16 record)
{
    volatile u16 table = 0;
    struct DosRegs regs;
    const u8 __far *from;
    u16 state, count, psp, header, next;
    u8 last;

    state = GetStat();
    if (ChkBlk(record)) {               /* Check sum */
        from = AT(record, REC_VECTORS); /* Restore vectors */
        count = *from++;
        IntOff();
        for (; count; count--) {
            u8 number = *from++;

            *(u32 __far *)AT(table, 4 * (u16)number) =
                *(const u32 __far *)from;
            from += 4;
        }
        *(u32 __far *)AT(table, SAVE_PTR) = *(const u32 __far *)from;
        IntOn();                      /* Restore SAVE PTR */
        psp = WORD_AT(record, REC_PSP); /* Release blocks */
        regs.ax = DOS_LISTS;
        Int21e(&regs);
        header = WORD_AT(regs.es, regs.bx - 2);
        do {
            if (psp == WORD_AT(header, 1)) {
                regs.ax = DOS_FREE;
                regs.es = header + 1;
                Int21e(&regs);
            }
            next = header + 1 + WORD_AT(header, 3);
            last = *AT(header, 0);
            header = next;
        } while (last == 'M');
    }
    SetStat(state);
}

/* Have DOS look for memory in the upper memory first, with that memory
   in its chain of blocks. Returns how it looked before, for SetStat:
   the strategy in the high byte and whether the upper memory was in the
   chain in the low one, or 0FFFFh for a DOS that knows of neither. */
u16 GetStat(void)
{
    struct DosRegs regs;
    u16 state;

    regs.ax = DOS_STRATEGY + 2;
    if (Int21e(&regs))
        return 0xFFFF;
    state = 0xFF00 | (u8)regs.ax;
    regs.ax = DOS_STRATEGY;
    Int21e(&regs);
    state = (regs.ax & 0xFF) << 8 | (state & 0xFF);
    regs.ax = DOS_STRATEGY + 3;
    regs.bx = 1;
    Int21e(&regs);
    regs.ax = DOS_STRATEGY + 1;
    regs.bx = 0x80;
    Int21e(&regs);
    return state;
}

/* Have DOS look for memory as it did before GetStat. */
void SetStat(u16 state)
{
    struct DosRegs regs;

    if (state == 0xFFFF)
        return;
    regs.ax = DOS_STRATEGY + 1;
    regs.bx = state >> 8;
    Int21e(&regs);
    regs.ax = DOS_STRATEGY + 3;
    regs.bx = state & 0xFF;
    Int21e(&regs);
}

/*----------------------- The interrupts -----------------------
   An interrupt that the program has taken comes to a function of four
   instructions, New21 or New27, which is all the assembly there is to
   it: room on the stack for one more return address, with the flags as
   they came on top of it, and a jump to an interrupt function of the
   compiler's. That function saves the registers and finds DS, and has
   them as `r`, to read and to change; `on` is the room, and `back` what
   the interrupt left on the stack, the place it came from. It ends as
   every interrupt function does, by taking `on` off the stack and going
   there, and that is how it goes on in the handler that had the
   interrupt before: with the registers as they are in `r`, the flags of
   `on`, and the stack as the interrupt left it. To go back where the
   interrupt came from instead, it goes on in CtrlC, which does nothing
   but return. */

/* Ctrl-C of DOS and Ctrl-Break of the BIOS, while the program has them:
   nothing. */
void __interrupt __far CtrlC(void)
{
}
#pragma aux CtrlC "*"

void __interrupt __far Int21(struct Saved r, struct Frame on,
                             struct Frame back);
void __interrupt __far Int27(struct Saved r, struct Frame on);

__declspec(naked) void __far New21(void)
{
    _asm {
        pushf
        push ax
        push ax
        jmp Int21
    }
}
#pragma aux New21 "*"

__declspec(naked) void __far New27(void)
{
    _asm {
        pushf
        push ax
        push ax
        jmp Int27
    }
}
#pragma aux New27 "*"

/* Go on in the handler at `handler`. */
static void Old(struct Frame STACK *on, u32 handler)
{
    on->ip = (u16)handler;
    on->cs = (u16)(handler >> 16);
}

/* The handlers below let interrupts in and clear the direction flag
   before they go on in DOS, where the assembly did both: when it noted a
   program, added one or looked through its list. */
#define FLAG_DIRECTION 0x0400
#define WORKED(on) \
    ((on).flags = (on).flags & ~FLAG_DIRECTION | FLAG_INTERRUPT)

/* INT 21h: DOS. A program run (AH 4Bh) is noted, one that stays in the
   memory (31h) gets its record, or ends instead (4Ch) when the list is
   full, and a block of memory given back (49h) takes the program that
   it was off the list. And another program asks for this one's data,
   the program's own call (AH 2Bh, CX 'VC', DX 'OM'): it gets in ES:BX
   what is kept of the first panel for AL 0, of the second for 1 and of
   the main data for anything else, and the number of this interface in
   AH.

   Ported as it is: that call returns with the flags as the handler has
   them, not as its caller had them. The carry flag is clear, which is
   the answer; interrupts are still off, as every interrupt begins; and
   the other flags are those of AL compared with 1. */
void __interrupt __far Int21(struct Saved r, struct Frame on,
                             struct Frame back)
{
    u8 function = (u8)(r.ax >> 8);

    if (function == 0x4B) {
        ExeProg(AT(r.ds, r.dx));
        WORKED(on);
    } else if (function == 0x31) {
        if (AddProg(&r))
            r.ax = 0x4C00 | r.ax & 0xFF;
        WORKED(on);
    } else if (function == 0x49) {
        FreeBlk(r.es);
        on.flags &= ~FLAG_DIRECTION;
    } else if (function == 0x2B && r.cx == 0x5643 && r.dx == 0x4F4D) {
        u8 which = (u8)r.ax;            /* VCapi */
        u8 less = which - 1;
        u8 bits = less;
        u16 flags = back.flags
            & ~(FLAG_CARRY | FLAG_PARITY | FLAG_AUX | FLAG_ZERO | FLAG_SIGN
                | FLAG_TRAP | FLAG_INTERRUPT | FLAG_OVERFLOW);

        bits ^= bits >> 4;
        bits ^= bits >> 2;
        bits ^= bits >> 1;
        if (!(bits & 1))
            flags |= FLAG_PARITY;
        if (!(which & 0x0F))
            flags |= FLAG_AUX;
        if (less == 0)
            flags |= FLAG_ZERO;
        if (less & 0x80)
            flags |= FLAG_SIGN;
        if (which == 0x80)
            flags |= FLAG_OVERFLOW;
        back.flags = flags;
        r.es = RES_SEG;
        if (which < 1)
            r.bx = (u16)WCB1sav;
        else if (which == 1)
            r.bx = (u16)WCB2sav;
        else
            r.bx = (u16)&ComnIni;
        r.ax = VcApiVr * 0x100;
        on.ip = OFFSET(CtrlC);
        on.cs = RES_SEG;
        return;
    }
    Old(&on, Vect21);
}

/* INT 27h: a program stays in the memory, the old way, with the bytes
   up to DX of its segment. It gets its record, or ends instead, by
   function 0 of DOS, when the list is full. */
void __interrupt __far Int27(struct Saved r, struct Frame on)
{
    r.dx = (u16)(r.dx + 0x0F) >> 4;
    if (AddProg(&r)) {
        r.ax &= 0x00FF;
        Old(&on, Vect21);
    } else {
        r.dx <<= 4;
        Old(&on, Vect27);
    }
    WORKED(on);
}
