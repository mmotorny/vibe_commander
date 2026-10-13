/*
 * The program and DOS around it: where the program begins and where it
 * ends, the commands of DOS that it runs, and its watch on DOS.
 *
 * In the original, and in the C translation after it, this is the part
 * of the program that stays in the memory while a command runs, which
 * the file has its name from: one segment with the PSP, a stack and
 * copies of the setup and the panels, which gives the rest of the
 * program up for the command and loads it again afterwards. The portable
 * C stays in the memory whole (see DEVIATIONS.md), and this is a file
 * of it like any other: DS is on the program's data, md is the main
 * data, and its variables are variables.
 *
 * Three things are here. The program begins here, at Alloc0: DOS enters
 * it there.
 *
 * The second is the commands: Exec runs those that the command line
 * holds, DosBuf, by COMMAND.COM or by the interrupt that COMMAND.COM has
 * for that; and what the program does when it leaves, or when DOS ends
 * it.
 *
 * The third is the program's watch on DOS. While the program runs, and
 * while a command runs from it, every call of DOS comes by New21: it notes
 * which program is being run, and when one stays in the memory, by DOS's
 * call for that or by INT 27h, it writes down who it is and which
 * interrupts it has taken, in a record behind the program's memory. With
 * those records the map of the memory, Alt+F5, lists the programs, and
 * Release takes one out again. These routines are entered from other
 * programs, on their stacks: nothing here takes the stack to be the
 * program's own, and a pointer to a local variable is declared STACK,
 * as everywhere.
 */
#include "vc.h"

#define LenCmd  124                     /* Length of command line */
#define VcApiVr 42

/*----------------------- The data -----------------------
   First the constants: the name that COMMAND.COM has when the
   environment does not say, and what the commands' code says for
   itself. */
const u8 Command[] = "COMMAND.COM";
static const u8 NoRun_S[] =
    "There is not enough memory to run this command\015\012";
static const u8 FnFil_S[] = "\015\012" "Can't find the file: ";
static const u8 Retr_S[] =
    "\015\012" "Press ENTER to try again or ESC to abort\015\012";
static const u8 Aloc_S[] = "Program too big to fit in memory\015\012";

/* The segment of the PSP, the 256 bytes that DOS keeps of a program in
   front of it: the command tail is there, the segment of the
   environment, where the memory ends, and where DOS goes on when the
   program has ended. The original is a .COM file and has all its
   segment registers there. */
u16 PspSeg;

/* What the watch keeps: the interrupts as they were when a program was
   last run, with what the BIOS keeps at 40h:A8h; the name of that
   program; and the programs that have stayed in the memory, how many
   and each by the segment of its record. Then where INT 22h and INT 27h
   went before the program took them. */
static u32 IntTab[256];
static u32 SavePtr;
static u8 NameBuf[12];
u16 Counter;
u16 PrgList[MaxProg];
static u32 Vect22;
static u32 Vect27;

/* Where INT 21h went before the program took it. This one is of the
   code's segment, not of the data: New21 passes a call of DOS on with
   no register to find the data by, and CS is all that it has. */
static u32 CODE Vect21;

/* A text of main_t.c, for a DOS that is too old. */
extern const u8 __far Vers_S[];

/* The PSP: where the program goes on when DOS has ended it, and the
   segment of its environment. */
#define PSP_EXIT (*(u32 __far *)AT(PspSeg, 0x0A))
#define PSP_ENV  (*(u16 __far *)AT(PspSeg, 0x2C))

#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))

/* Where a routine is, as an address for DOS and for a return. */
#define ROUTINE(function) ((const u8 __far *)(void __far *)(function))

#define DOS_DRIVE    0x1900             /* the current drive, to AL */
#define DOS_VECTOR   0x2500             /* an interrupt, AL, to DS:DX */
#define DOS_VERSION  0x3000             /* which DOS it is, to AL and AH */
#define DOS_BREAK    0x3300             /* whether DOS looks for Ctrl-C at
                                           every call, to DL, and with AL 1
                                           from DL */
#define DOS_PATH     0x4700             /* the current directory of the
                                           drive in DL, to DS:SI */
#define DOS_FREE     0x4900             /* free the block in ES */
#define DOS_RESIZE   0x4A00             /* make the block in ES BX
                                           paragraphs long */
#define DOS_EXEC     0x4B00             /* run the program at DS:DX, with
                                           what ES:BX says */
#define DOS_EXIT     0x4C00             /* end the program, with AL */
#define DOS_LISTS    0x5200             /* where DOS keeps its lists, to
                                           ES:BX */
#define DOS_STRATEGY 0x5800             /* how DOS looks for memory: AL 0
                                           to ask, 1 to say, 2 and 3 the
                                           same for whether the upper
                                           memory is in the chain */
#define DOS_PSP_OLD  0x5100             /* the current program, to BX: as
                                           6200h, and of DOS 2 too */
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
#define FLAG_DIRECTION 0x0400
#define FLAG_OVERFLOW  0x0800

/* The six that an instruction leaves of what it has worked out. */
#define FLAGS_RESULT (FLAG_CARRY | FLAG_PARITY | FLAG_AUX | FLAG_ZERO \
                      | FLAG_SIGN | FLAG_OVERFLOW)

/* Where things are in the original, as it was shipped: what its
   registers hold when it calls the BIOS with them unset, and the BIOS
   takes them for a page of the screen and a colour. What the original
   keeps of the main data while a command runs, ComnIni, is at 25E5h
   there and laid out as the main data is; the part of it that stays in
   the memory is 31Ah paragraphs. */
#define COMNINI_WAS 0x25E5
#define WAS(address) ((u16)(address) - (u16)&md + COMNINI_WAS)
#define EPB_WAS     WAS(&md.EPB)
#define RESPARS_WAS 0x031A

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

/* The return address that an interrupt leaves on the stack, behind the
   registers that an interrupt function of the compiler saves there:
   struct Saved of vc.h. */
struct Frame {
    u16 ip, cs, flags;
};

/* DX of a call of DOS that is no address. */
#define DX_OF(regs) (*(u16 STACK *)&(regs).name)

static u16 GetStat(void);
static void SetStat(u16 state);

/*----------------------- The flags -----------------------
   The assembly goes on in DOS, and in the program that started this
   one, with the flags as its last instruction of arithmetic left them.
   These say what that is, so that the C can leave the same. */

/* What a result of 16 bits leaves of them: zero, sign, and parity, which
   is of its low byte. */
static u16 Result(u16 result)
{
    u8 bits = (u8)result;
    u16 flags = 0;

    bits ^= bits >> 4;
    bits ^= bits >> 2;
    bits ^= bits >> 1;
    if (!(bits & 1))
        flags |= FLAG_PARITY;
    if (result == 0)
        flags |= FLAG_ZERO;
    if (result & 0x8000)
        flags |= FLAG_SIGN;
    return flags;
}

/* ADD: `a` and `b` added. */
static u16 Added(u16 a, u16 b)
{
    u16 sum = a + b;
    u16 flags = Result(sum);

    if (sum < a)
        flags |= FLAG_CARRY;
    if ((a ^ b ^ sum) & 0x10)
        flags |= FLAG_AUX;
    if ((a ^ sum) & (b ^ sum) & 0x8000)
        flags |= FLAG_OVERFLOW;
    return flags;
}

/* SUB and CMP: `b` taken from `a`. */
static u16 Taken(u16 a, u16 b)
{
    u16 rest = a - b;
    u16 flags = Result(rest);

    if (a < b)
        flags |= FLAG_CARRY;
    if ((a ^ b ^ rest) & 0x10)
        flags |= FLAG_AUX;
    if ((a ^ b) & (a ^ rest) & 0x8000)
        flags |= FLAG_OVERFLOW;
    return flags;
}

/* SHL by one: `value` doubled. The processor leaves the auxiliary flag
   undefined; it is clear here. */
static u16 Doubled(u16 value)
{
    u16 twice = value << 1;
    u16 flags = Result(twice);

    if (value & 0x8000)
        flags |= FLAG_CARRY;
    if ((value ^ twice) & 0x8000)
        flags |= FLAG_OVERFLOW;
    return flags;
}

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
   Returns the flags as the assembly leaves them, with interrupts let in
   and the direction flag clear. The carry flag says that the program is
   to end instead: the list is full, the bell sounds and the interrupts
   are put back as they were, and the other flags are those of a
   register cleared, XOR, of which the processor leaves the auxiliary
   flag undefined: clear here. Or the paragraphs with the record's are
   more than 16 bits hold. Without that the flags are those of the
   addition that made them.

   Ported as it is: if DOS will not make the program's block longer for
   the record, the program stays without one and is not listed. The flags
   are then those of the paragraphs asked for, as they were added up,
   with the carry flag clear: DOS sets it and leaves the others, and the
   assembly turns it over. */
static u16 AddProg(struct Saved STACK *r)
{
    volatile u16 table = 0;
    u32 __far *vector = (u32 __far *)AT(table, 0);
    struct DosRegs regs;
    struct BiosRegs bios;
    u8 __far *record;
    u8 __far *to;
    const u8 __far *byte;
    u16 psp, segment, i, changed, length, sum, more, flags;

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
        return FLAG_ZERO | FLAG_PARITY | FLAG_CARRY;
    }
    regs.ax = DOS_PSP;                  /* Get current PSP */
    Int21e(&regs);
    psp = regs.bx;
    FreeBlk(psp);
    regs.ax = DOS_RESIZE;
    regs.bx = r->dx + REC_PARAS;
    regs.es = psp;
    if (Int21e(&regs))
        return Added(r->dx, REC_PARAS) & ~FLAG_CARRY;
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
    flags = Added(r->dx, more);
    r->dx += more;
    return flags;
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
static u16 GetStat(void)
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
static void SetStat(u16 state)
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
   An interrupt that the program has taken and deals with comes to four
   instructions of assembly: room on the stack for one more return
   address, with the flags as they are on top of it, and a jump to an
   interrupt function of the compiler's. That function saves the
   registers and finds DS, and has them as `r`, to read and to change;
   `on` is the room, and `back` what the interrupt left on the stack, the
   place it came from. It ends as every interrupt function does, by
   taking `on` off the stack and going there, and that is how it goes on
   in the handler that had the interrupt before: with the registers as
   they are in `r`, the flags of `on`, and the stack as the interrupt
   left it. To go back where the interrupt came from instead, it goes on
   in CtrlC, which does nothing but return.

   The flags of `on` are made what the assembly had when it jumped to
   that handler: those of its last instruction of arithmetic, with
   interrupts let in and the direction flag clear where it had done so.
   Nothing that is known reads them there; they are the same all the
   same. */

/* Ctrl-C of DOS and Ctrl-Break of the BIOS, while the program has them:
   nothing. */
void __interrupt __far CtrlC(void)
{
}

void __interrupt __far Int21(struct Saved r, struct Frame on,
                             struct Frame back);
void __interrupt __far Int27(struct Saved r, struct Frame on);

/* INT 21h arrives here, every call of DOS that any program makes while
   this one is in the memory. Of nearly all of them the program wants
   nothing, and for those it takes nothing either: no register changed
   and not a byte of the caller's stack, which may be short. They go on
   in the handler that had the interrupt before, with the flags of the
   comparison that sent them there, as in the assembly. Only the four
   that the program deals with go on to Int21: AH 4Bh, 31h, 49h, and
   2Bh with 'VC' in CX and 'OM' in DX.

   Why this is assembly, thirteen instructions in front of the four that
   every entry has: what the compiler makes saves registers before it
   compares, on the caller's stack, and cannot jump to another handler
   with the stack as it found it; a function without that, naked, is
   given no instruction for a statement of C at all; an ordinary one
   with the call in AX, nothing to change and the jump as a call that
   does not return makes itself room on the stack, changes AL to
   compare, reads the old address through the caller's DS or through an
   ES that it loads, and jumps with BP still pushed (tried, with this
   compiler). And the flags that the handler before is entered with are
   those of these comparisons, which C cannot promise of anything. The
   old address is read through CS, which is why it is a variable of the
   code's segment. */
__declspec(naked) void __far New21(void)
{
    _asm {
        cmp ah,4Bh
        je deal
        cmp ah,31h
        je deal
        cmp ah,49h
        je deal
        cmp ah,2Bh
        jne old
        cmp cx,5643h
        jne old
        cmp dx,4F4Dh
        je deal
    old:
        jmp dword ptr cs:Vect21
    deal:
        pushf
        push ax
        push ax
        jmp Int21
    }
}

/* INT 27h arrives here. The program deals with every one, and the
   assembly took of the caller's stack for each as well. */
__declspec(naked) void __far New27(void)
{
    _asm {
        pushf
        push ax
        push ax
        jmp Int27
    }
}

/* Go on in the handler at `handler`. */
static void Old(struct Frame STACK *on, u32 handler)
{
    on->ip = (u16)handler;
    on->cs = (u16)(handler >> 16);
}

/* Go on with what an instruction has left in the six flags, `result`,
   and with the direction flag clear. */
static void Left(struct Frame STACK *on, u16 result)
{
    on->flags = on->flags & ~(FLAGS_RESULT | FLAG_DIRECTION) | result;
}

/* INT 21h, the four calls that New21 does not pass on. A program run (AH
   4Bh) is noted, one that stays in the memory (31h) gets its record, or
   ends instead (4Ch) when the list is full, and a block of memory given
   back (49h) takes the program that it was off the list. And another
   program asks for this one's data, the program's own call (AH 2Bh, CX
   'VC', DX 'OM'): it gets in ES:BX the first panel for AL 0, the second
   for 1 and the main data for anything else, and the number of this
   interface in AH. The original hands out what it keeps of the three
   while a command runs, copies in the part of it that stays in the
   memory: of the main data what it has before DosLin, of a panel what
   it has before ExtBuf and behind that 13 bytes of every file. Here it
   is the panel and the main data themselves, which begin as those
   copies do; the files of a panel are where a panel has them, 24 bytes
   each (see DEVIATIONS.md).

   The flags that DOS is entered with, as the assembly leaves them. For
   a program run: those of the zero byte that ends its name, compared
   with zero, and interrupts let in. For one that stays: those that
   AddProg returns. For a block given back: those of the count of the
   list less the programs taken off it, or, with the list empty, of the
   comparison that found AH to be 49h, equal; interrupts are still off.

   Ported as it is: the program's own call returns with the flags as the
   handler has them, not as its caller had them. The carry flag is
   clear, which is the answer; interrupts are still off, as every
   interrupt begins; and the other flags are those of AL compared with
   1. */
void __interrupt __far Int21(struct Saved r, struct Frame on,
                             struct Frame back)
{
    u8 function = (u8)(r.ax >> 8);

    if (function == 0x4B) {
        ExeProg(AT(r.ds, r.dx));
        Left(&on, FLAG_ZERO | FLAG_PARITY);
        on.flags |= FLAG_INTERRUPT;
    } else if (function == 0x31) {
        u16 flags = AddProg(&r);

        if (flags & FLAG_CARRY)
            r.ax = 0x4C00 | r.ax & 0xFF;
        Left(&on, flags);
        on.flags |= FLAG_INTERRUPT;
    } else if (function == 0x49) {
        u16 count = Counter;

        FreeBlk(r.es);
        if (count)
            Left(&on, Taken(count, count - Counter));
        else
            Left(&on, FLAG_ZERO | FLAG_PARITY);
    } else {
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
        r.bx = 0;
        if (which < 1)
            r.es = md.WCB1seg;
        else if (which == 1)
            r.es = md.WCB2seg;
        else {
            r.es = MAIN_SEG;
            r.bx = (u16)&md;
        }
        r.ax = VcApiVr * 0x100;
        Old(&on, (u32)ROUTINE(CtrlC));
        return;
    }
    Old(&on, Vect21);
}

/* INT 27h: a program stays in the memory, the old way, with the bytes
   up to DX of its segment. It gets its record, or ends instead, by
   function 0 of DOS, when the list is full. The handler before is
   entered with interrupts let in and with the flags of the last shift
   that makes bytes of the paragraphs again, DOS, where the program
   ends, with those that AddProg returns. */
void __interrupt __far Int27(struct Saved r, struct Frame on)
{
    u16 flags;

    r.dx = (u16)(r.dx + 0x0F) >> 4;
    flags = AddProg(&r);
    if (flags & FLAG_CARRY) {
        r.ax &= 0x00FF;
        Old(&on, Vect21);
    } else {
        flags = Doubled(r.dx << 3);
        r.dx <<= 4;
        Old(&on, Vect27);
    }
    Left(&on, flags);
    on.flags |= FLAG_INTERRUPT;
}

/*------------------- Execute commands ------------------*/

/* Write the text at `text` on the screen as a teletype would, in grey,
   on the page in the high byte of `bx`. Returns where the text ends. A
   text of no characters is written as its zero byte. */
static u16 OutStr(const u8 __far *text, u16 bx)
{
    struct BiosRegs regs;

    do {
        regs.ax = 0x0E00 | *text++;
        regs.bx = bx & 0xFF00 | 7;
        regs.cx = regs.dx = 0;
        Int10(&regs);
    } while (*text != 0);
    return (u16)(u32)text;
}

/* The same for a text of the program's data.

   Ported as it is: the assembly sets no page where it writes these and
   the commands, and hands the BIOS whatever BH holds, half of an
   address of its own or of a number that DOS answered. Each place that
   writes has in `bx` what the assembly has in BX there, an address as
   it is in the original. Most BIOSes, that of the bench among them,
   write to the page that shows, whatever they are told. */
static void OutStr0(const u8 *text, u16 bx)
{
    OutStr((const u8 __far *)text, bx);
}

/* Say that the file `name` is missing, after `text`, and wait for Enter
   or Esc: 1 for Esc, to give up. The assembly has the address of
   ComnIni.EPB in BX where it calls, which DOS was to run the file
   with. */
static u16 NotFind(const u8 *text, const u8 __far *name)
{
    OutStr0(text, EPB_WAS);
    OutStr(name, EPB_WAS);
    OutStr0(Retr_S, EPB_WAS);
    for (;;) {
        u8 key = (u8)Int16(0).key;

        if (key == 0x0D)
            return 0;
        if (key == 0x1B)
            return 1;
    }
}

/* Note whether DOS looks for Ctrl-C at every call, have it not do so,
   and take Ctrl-C: when the program starts, and after every command. */
void SetBrk(void)
{
    struct DosRegs regs;

    regs.name = 0;
    regs.ax = DOS_BREAK;
    Int21e(&regs);
    md.BreakFl = (u8)DX_OF(regs);
    DX_OF(regs) = 0;
    regs.ax = DOS_BREAK + 1;
    Int21e(&regs);
    regs.name = ROUTINE(CtrlC);
    regs.ax = DOS_VECTOR + 0x23;
    Int21e(&regs);
}

/* Leave: have DOS look for Ctrl-C as it did, release the programs that
   have stayed in the memory, and give INT 21h and INT 27h back. Then end
   the program with `code`, unless DOS has ended it already and this is
   where it went on: then return.

   The last program of the list is released as many times as the list is
   long; releasing it takes it off the list, by New21 (see FreeBlk).

   `r` and `flags` are the registers and the flags that this was entered
   with, and they are left as the assembly leaves its own, for the
   program that this returns to when DOS has ended this one: ES on the
   segment of this program's PSP; CX 0, the count of the programs run
   down; SI twice the
   length of the list when the last program was released, and DS on its
   record, if there was one; DS:DX the address that INT 27h went to
   before, which DOS was told last, if the program had taken the
   interrupts, and else DL as DOS was told to look for Ctrl-C. AX, BX,
   DI and BP are as they came. The flags are those of the segment that
   the program goes on in, compared with 0; the direction flag is clear
   if a program was released, and interrupts are let in if its record
   was whole. */
static void Exit02(u16 code, struct Saved STACK *r, u16 STACK *flags)
{
    struct DosRegs regs;
    u16 count;

    r->es = PspSeg;                     /* Exit01 */
    r->dx = r->dx & 0xFF00 | md.BreakFl;
    regs.name = 0;
    DX_OF(regs) = md.BreakFl;
    regs.ax = DOS_BREAK + 1;
    Int21e(&regs);
    for (count = Counter; count; count--) {
        r->si = Counter << 1;
        r->ds = *(PrgList + Counter - 1);
        *flags &= ~FLAG_DIRECTION;
        if (ChkBlk(r->ds))
            *flags |= FLAG_INTERRUPT;
        Release(r->ds);
    }
    r->cx = 0;
    if (md.ChkTSR != 0) {               /* Exit03 */
        regs.name = (const u8 __far *)Vect21;
        regs.ax = DOS_VECTOR + 0x21;
        Int21e(&regs);
        regs.name = (const u8 __far *)Vect27;
        regs.ax = DOS_VECTOR + 0x27;
        Int21e(&regs);
        r->ds = (u16)(Vect27 >> 16);
        r->dx = (u16)Vect27;
    }
    if ((u16)(Vect22 >> 16) == 0) {     /* Exit04 */
        regs.ax = DOS_EXIT | code & 0xFF;
        Int21e(&regs);
    }
    *flags = *flags & ~FLAGS_RESULT | Taken((u16)(Vect22 >> 16), 0);
}

/* Leave the program with `code`: the PSP gets back the address at which
   the program that started this one goes on, and DOS ends this one.
   Nobody gets the registers that Exit02 leaves. */
void Exit00(u16 code)
{
    struct Saved r;
    u16 flags;

    PSP_EXIT = Vect22;
    Vect22 &= 0xFFFF;
    Exit02(code, &r, &flags);
}

/* While the program runs, its PSP names Exit01 as the place to go on
   when the program has ended: so if DOS ends it, by whatever means, DOS
   goes on there, on the stack of the program that started this one, and
   what Exit02 does is done all the same. Then on to where that program
   goes on, with the stack as it is, and with the registers and the
   flags as Exit02 leaves them. That is done as for an interrupt that
   goes on in the handler before it, see New21: four instructions of
   assembly for room on the stack, and an interrupt function that
   returns into that room. The assembly took of that program's stack
   here too. */
void __interrupt __far Exit05(struct Saved r, struct Frame on);

__declspec(naked) void __far Exit01(void)
{
    _asm {
        pushf
        push ax
        push ax
        jmp Exit05
    }
}

void __interrupt __far Exit05(struct Saved r, struct Frame on)
{
    Exit02(r.ax, &r, &on.flags);
    on.ip = (u16)Vect22;
    on.cs = (u16)(Vect22 >> 16);
}

/* Have the program go on in Exit01 when DOS ends it, and, if it is set
   up to know of the programs that stay in the memory, take DOS's own
   interrupts, 21h and 27h: when the program has started. */
void TakeDos(void)
{
    struct DosRegs regs;
    /* The segment of the table of vectors. Written as a constant, the
       compiler takes a far pointer into segment 0 for a near one. */
    volatile u16 table = 0;

    Vect22 = PSP_EXIT;
    PSP_EXIT = (u32)ROUTINE(Exit01);
    if (md.ChkTSR == 0)
        return;
    regs.ax = 0x3521;                   /* where an interrupt goes */
    regs.bx = 0;
    regs.cx = 0;
    regs.name = (const u8 __far *)&md;
    regs.si = 0;
    regs.di = 0;
    Int21d(&regs);
    Vect21 = (u32)regs.es << 16 | regs.bx;
    IntOff();
    *(u32 __far *)AT(table, 4 * 0x21) = (u32)ROUTINE(New21);
    IntOn();
    regs.ax = 0x3527;
    Int21d(&regs);
    Vect27 = (u32)regs.es << 16 | regs.bx;
    regs.ax = DOS_VECTOR + 0x27;        /* have one go somewhere */
    regs.name = ROUTINE(New27);
    Int21d(&regs);
}

/* No memory: say so and leave. `bx` is what the assembly has in BX
   where it jumps here, a number of DOS's or an address. */
void AlocEr(u16 bx)
{
    OutStr0(Aloc_S, bx);
    Exit00(1);
}

/* COMMAND.COM's interrupt for a command: `command` is its length, its
   text and a carriage return. It comes back with nothing as it was but
   where it comes back to: no stack, no DS. So this does not return. It
   is the one place where the program has to set its stack by hand, and
   it goes on in Exec2E, on a stack that is empty again.

   Why this is assembly, all eight instructions of it: no compiled
   instruction can run before SS and SP are set, since any of them may
   use the stack; and the function that called cannot be gone back
   into, with its place on the stack lost, so the jump is of it too.
   Interrupts are off meanwhile and the direction flag is clear
   afterwards, as the compiler has it. Exec2E finds DS itself, as a
   function that is entered from elsewhere does. */
static void __loadds Exec2E(void);
#pragma aux Exec2E aborts

__declspec(naked) static void Int2E(const u8 *command)
{
    (void)command;                      /* in SI, where the interrupt
                                           takes it */
    _asm {
        int 2Eh
        cli
        mov ax,seg Stack
        mov ss,ax
        mov sp,offset Stack + LenStk
        sti
        cld
        jmp Exec2E
    }
}
#pragma aux Int2E parm [si] aborts

/* Run the commands that DosBuf holds from BatAdr on, one after the
   other: each ends with a zero byte, and behind the last there is one
   more. With `behind`, go on behind a command that has been run. A
   command that begins with `@`, behind another, is run without being
   shown first, with a prompt, as the others are. Returns when none is
   left, or when COMMAND.COM cannot be run.

   A command is at most 123 characters; what a longer one has beyond
   that is cut off by the carriage return that is written there.

   With ExecTyp, the commands are handed to the COMMAND.COM that is in
   the memory, by its INT 2Eh, and this does not return: see Int2E. If
   the current drive is another than when the program started, F21_31
   has left a command in DosChg that goes there, which is handed over
   first. Without ExecTyp, COMMAND.COM is run for each command, by the
   name that COMSPEC has in the environment, with /C and the command for
   its tail, which is written over the four bytes in front of the
   command. */
static void Exec02(u16 behind)
{
    struct DosRegs regs;
    struct BiosRegs bios;
    const u8 __far *name;
    u8 *command, *before, *end, *to;
    u16 count, failed;
    u8 drive;

    for (;;) {
        if (behind)
            goto Exec07;
        regs.name = 0;
        DX_OF(regs) = md.BreakFl;
        regs.ax = DOS_BREAK + 1;
        Int21e(&regs);
        command = (u8 *)md.BatAdr;
        before = command - 4;
        for (end = command; *end++ != 0; )
            ;
        md.BatAdr = (u16)end;
        for (end = command, count = LenCmd; count; count--)
            if (*end++ == 0)
                break;
        end--;
        *end = 0x0D;
        if (md.ExecTyp != 0) {
            before += 3;
            *before = (u8)(end - 1 - before);
            if (md.DosChg[1] != 0) {
                md.BatPoin = (u16)before;
                Int2E(md.DosChg);
            }
            Int2E(before);              /* Exec03 */
        }
        before[0] = (u8)(end - 1 - before);     /* Exec04 */
        before[1] = '/';
        before[2] = 'C';
        before[3] = ' ';
        md.EPB.CmdLine = (u32)MAIN_SEG << 16 | (u16)before;
        for (;;) {                      /* Exec05 */
            if (md.ComSpec == 0) {
                name = (const u8 __far *)Command;
            } else {
                name = AT(PSP_ENV, md.ComSpec);
                regs.name = name;
                regs.bx = (u16)&md.EPB;
                regs.es = MAIN_SEG;
                regs.ax = DOS_EXEC;
                failed = Int21e(&regs);
                SetBrk();
                if (!failed)
                    break;
                if (regs.ax > 3) {
                    OutStr0(NoRun_S, EPB_WAS);
                    return;
                }
            }
            if (NotFind(FnFil_S, name)) /* Exec06 */
                return;
        }
    Exec07:
        behind = 0;
        command = (u8 *)md.BatAdr;
        if (*command == 0)
            return;
        if (*command == '@') {
            md.BatAdr++;
            continue;
        }
        /* Exec08: the prompt, made where the interrupts are noted when a
           program is run, and the command behind it. BX is the address
           of the command, for the texts and for the end of the line. */
        to = (u8 *)IntTab;
        *to++ = 0x0D;
        *to++ = 0x0A;
        regs.ax = DOS_DRIVE;
        Int21e(&regs);
        drive = (u8)regs.ax;
        *to++ = drive + 'A';
        if (md.Prompt != 0) {
            *to++ = ':';
            *to++ = '\\';
            regs.name = AT(MAIN_SEG, (u8)(drive + 1));
            regs.si = (u16)to;
            regs.ax = DOS_PATH;
            Int21e(&regs);
            while (*to != 0)
                to++;
        }
        *to++ = '>';                    /* Exec09 */
        *to = 0;
        OutStr0((const u8 *)IntTab, WAS(md.BatAdr));
        OutStr0((const u8 *)md.BatAdr, WAS(md.BatAdr));
        bios.ax = 0x0E0D;
        bios.bx = WAS(md.BatAdr);
        bios.cx = bios.dx = 0;
        Int10(&bios);
        bios.ax = 0x0E0A;
        Int10(&bios);
    }
}

/* The commands are done: LoadFl is as the setup has it again, without
   the bit that said not to show a command.

   The original does more here, Exec10: if it gave most of itself up
   for the commands, it asks DOS for a block of memory and runs the
   program's own file as a second copy, which moves its code there and
   ends; it holds the check sum of that file against its own, and says
   "Bad or missing" with the name of the file and asks whether to try
   again when the file is not there or is another. None of that is in
   the portable C, which gives nothing up (see DEVIATIONS.md). */
static void Exec10(void)
{
    md.LoadFl = md.LoadRes;
}

/* Run the commands of DosBuf. The original first gives up all of itself
   but the part that this is of, if the program is set up so. With the
   commands handed to COMMAND.COM's own interrupt, this does not return:
   the program goes on in Exec2E. */
void Exec(void)
{
    md.BatAdr = (u16)md.DosBuf;         /* Exec01 */
    Exec02(0);
    Exec10();
}

/* Behind INT 2Eh, on an empty stack. If that was the command of DosChg,
   which changes the drive, the command itself is next, at BatPoin. Else
   a command of DosBuf has been run: on with those that follow it, and
   then as the program goes on behind Exec, which cannot be returned
   to: Again of main.c. */
static void __loadds Exec2E(void)
{
    if (md.DosChg[1] != 0) {
        md.DosChg[1] = 0;
        Int2E((const u8 *)md.BatPoin);
    }
    SetBrk();                           /* SetBrk0 */
    Exec02(1);
    Exec10();
    Again();
}

/*---------------------- The entry ----------------------*/

/* No memory when the program begins: say so and end. */
static void Alloc9(void)
{
    struct DosRegs regs;

    OutStr0(Aloc_S, RESPARS_WAS);       /* BX: the paragraphs of the part
                                           of the original that stays */
    SetStat(0);
    regs.ax = DOS_EXIT + 1;
    Int21e(&regs);
}
#pragma aux Alloc9 aborts

/* The program begins here: DOS has loaded the file, set the words of it
   that hold a segment, and enters this on the stack that the header of
   the file names, with DS on the PSP. It is told to load DS for itself,
   as a function is that is entered from elsewhere, and then asks DOS
   where the PSP is.

   DOS has to be 3.20 at the least. DOS has given the program all of the
   memory: its block is made as long as the program is, which ends where
   its stack does (see stack.c), and the rest is asked for again when the
   panels are made, Init11 of start.c. Then Main of main.c is the
   program.

   Ported as it is: how DOS looks for memory is set to what it is when
   DOS starts, the lowest block that fits and no upper memory, whatever
   it was.

   Not as the original, which is a .COM file (see DEVIATIONS.md): a DOS
   before 3 is left through the INT 20h that every PSP begins with,
   entered by a far call, where the original returns there. DOS wants CS
   on the PSP when a program ends that way, and nothing else of this
   program is in that segment. And the call that says where the PSP is
   is one of DOS 2 and later.

   Not in the portable C, which does not load itself again after a
   command: the original first takes the check sum of its file as it is
   loaded, to know the file again; and if its command tail is the one
   that it gives a second copy of itself, it is that copy, which moves
   its code into the block that the first copy has for it, leaves its
   check sum there, and ends. It also moves most of itself up when it
   starts, to lay the data of its part that stays over the code that
   has run, and applies its own relocations: the C translation's file
   is laid out for that by a tool, where the portable C's is as the
   linker writes it. */
void __loadds Alloc0(void)
{
    struct DosRegs regs;
    u32 end = (u32)(void __far *)Stack;

    IntOn();                            /* for the direction flag, which
                                           the compiler wants clear */
    regs.ax = DOS_PSP_OLD;
    Int21e(&regs);
    PspSeg = regs.bx;
    regs.ax = DOS_VERSION;              /* Check DOS version */
    Int21e(&regs);
    if ((u8)regs.ax < 3)                /* Alloc4 */
        ((void (__far *)(void))AT(PspSeg, 0))();
    if (((regs.ax & 0xFF) << 8 | regs.ax >> 8) < 0x314) {
        OutStr(Vers_S, regs.bx);        /* BX: as DOS answered */
        regs.ax = DOS_EXIT + 2;
        Int21e(&regs);
    }
    regs.bx = (u16)(end >> 16) + (((u16)end + LenStk + 15) >> 4)
              - PspSeg;                 /* Alloc5 */
    regs.es = PspSeg;
    regs.ax = DOS_RESIZE;
    if (Int21e(&regs))
        Alloc9();
    SetStat(0);                         /* Alloc7 */
    Main();
}
#pragma aux Alloc0 aborts
