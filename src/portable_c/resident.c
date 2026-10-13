/*
 * The part of the program that stays in the memory while a command runs.
 *
 * It begins with the PSP and is one segment, as the program was when it
 * was a .COM file: the room of the PSP, the code here, the data behind
 * it, which is of this file too, and the stack at the end. The compiler
 * has the code and the data in one group for that, by two options of
 * the build, -nd=R and -g=R_GROUP. DS is that segment in every
 * function of this file, where vc.h says that DS is the main data: the
 * main data is not in the memory while a command runs, and neither is
 * the C code of the rest of the program. So nothing here uses `md` or
 * calls a function of another file but those of dos.c, of which the
 * build compiles a second copy into this part (see RESIDENT in vc.h).
 *
 * Three things are here. The program begins here, at Alloc0: DOS enters
 * it there.
 *
 * The second is the outermost loop of the program, Stay:
 * it calls the part that goes, which comes back with commands to run or to
 * leave; it runs the commands, Exec, by COMMAND.COM or by the interrupt
 * that COMMAND.COM has for that, brings the other part back into the
 * memory if that was given up for them, by running the program's own
 * file as a second copy, and calls it again.
 *
 * The third is the program's watch on DOS. While the program runs, and
 * while a command runs from it, every call of DOS comes by New21: it notes
 * which program is being run, and when one stays in the memory, by DOS's
 * call for that or by INT 27h, it writes down who it is and which
 * interrupts it has taken, in a record behind the program's memory. With
 * those records the map of the memory, Alt+F5, lists the programs, and
 * Release takes one out again.
 */
#include "vc.h"

/* Where the PSP is, the 256 bytes that DOS keeps of a program in front
   of it: this part begins with them, and the file is the program as it
   lies in the memory, so it begins with room for them, which
   tools/mkexe.c takes off. The room is of the code segment, the first of
   the program, and has to be the first thing of that: this file is
   linked first, and nothing of the code segment stands in front of this
   line. That the compiler then writes it first rests on the compiler,
   not on the language; tools/mkexe.c stops the build if it is not at 0.
   It is the main data to the linker as well: see md in vc.h. */
const u8 CODE Psp[0x100] = { 0 };
#pragma aux Psp "*"

#define LenStk  0x1000                  /* Stack size */
#define LenCmd  124                     /* Length of command line */
#define VcApiVr 42

/*----------------------- The data -----------------------
   The data of this part is of this file, in a group of the compiler's
   that only this part has, R_GROUP (the option -nd=R of the build). The
   code of this part is of the same group (the option -g=R_GROUP), and
   is its first segment, so the group begins at the PSP: DS is on it, and
   SS, and an offset here is one from the PSP. It lies behind the code in
   the order of the compiler's segments: the constants, what has a value
   when the program is loaded, and last what has none, which is the stack
   and nothing else. tools/mkexe.c stops the build if that segment holds
   anything more, or if anything stands behind the structure that the
   file does not have as zeros.

   First the constants: the name that COMMAND.COM has when the
   environment does not say, and what this part says for itself. */
const u8 Command[] = "COMMAND.COM";
const u8 NoRun_S[] =
    "There is not enough memory to run this command\015\012";
const u8 FnFil_S[] = "\015\012" "Can't find the file: ";
const u8 BadFl_S[] = "\015\012" "Bad or missing ";
const u8 Retr_S[] =
    "\015\012" "Press ENTER to try again or ESC to abort\015\012";
const u8 Aloc_S[] = "Program too big to fit in memory\015\012";
#pragma aux Command "*"
#pragma aux NoRun_S "*"
#pragma aux FnFil_S "*"
#pragma aux BadFl_S "*"
#pragma aux Retr_S "*"
#pragma aux Aloc_S "*"

/* The numbers that tools/mkexe.c writes into the file, each of which it
   finds by its name in the linker's map. All are in paragraphs from the
   PSP but the counts: how long the file is, ImgEnd; where the part that
   goes begins, TrnPrW; how long this part is, ResPrW, and where the
   file has nothing but zeros for it, DatPrW, which is where Stays
   begins; where the linker's relocations are, RelTab, how many of them
   are of the part that goes, RelCnt, and how many, behind those, of
   this part, ResCnt. They are given a value here, 0, for the compiler
   to put them where the file has contents. */
u16 ImgEnd = 0;
u16 RelTab = 0;
u16 RelCnt = 0;
u16 TrnPrW = 0;
u16 ResPrW = 0;
u16 DatPrW = 0;
u16 ResCnt = 0;
#pragma aux ImgEnd "*"
#pragma aux RelTab "*"
#pragma aux RelCnt "*"
#pragma aux TrnPrW "*"
#pragma aux ResPrW "*"
#pragma aux DatPrW "*"
#pragma aux ResCnt "*"

/* The segment that the part that goes runs in, which is so many
   paragraphs below its block of memory as that part begins behind the
   PSP in the file, TrnPrW; the check sum of the program's file as it
   was loaded; and the command tail by which a second copy of the
   program knows that it is one, with room for the segment of the
   first. */
u16 RunPar = 0;
u16 ChkFile = 0;
u8 OvlPrm[12] = { 10, 'V', 'V', 'V', 0, 0, 0, 0, 0, 0, 0, 0x0D };
#pragma aux RunPar "*"
#pragma aux ChkFile "*"
#pragma aux OvlPrm "*"

/* What this part keeps: struct Stays of vc.h, which says what is in it.
   Of ComnIni only as much is there as the main data has before DosLin.
   It is one structure for the order of its members, which the compiler
   keeps of nothing else; so they have no names of their own to the
   linker, and are named here as the assembly names them. It is all
   zeros in the file, and written out as that: without a value it would
   be of the segment that is to hold the stack alone. It is the last
   that has a value, so that this part is nothing but zeros from here
   on. */
struct Stays Stays = { { 0 } };
#pragma aux Stays "*"
#define IntTab  Stays.IntTab
#define SavePtr Stays.SavePtr
#define Vect22  Stays.Vect22
#define Vect27  Stays.Vect27
#define Counter Stays.Counter
#define PrgList Stays.PrgList
#define NameBuf Stays.NameBuf
#define WCB1sav Stays.WCB1sav
#define WCB2sav Stays.WCB2sav
#define ComnIni (*(struct MainDat *)Stays.ComnIni)

/* The stack, which is the end of this part: the header of the file names
   it, so the program is on it from its first instruction. The original
   has 100h bytes of it. Compiled code uses more than the assembly did,
   so the C has sixteen times that, and its use needs no care.
   It is an object of its own, not the last member of Stays, for the name:
   by that tools/mkexe.c and the tests find where it begins, and it ends
   where its segment does, R_BSS, of which it is all.

   That it is the end rests on this compiler and not on the language.
   Open Watcom has what is given no value in a segment of its own behind
   what is given one, and this is the one object of the file without:
   C promises no order of two objects. tools/mkexe.c stops the build
   where that does not hold. For another compiler the stack would have
   to be put last by what that one has for it, a section that its
   linker is told to place behind the others, and tools/mkexe.c and
   harness/coverage.py told the name of that in place of R_BSS; or it
   goes into Stays as the last member, which the language does keep
   in its place, and the two are told how long it is some other way
   than by a name in the map. */
u8 DataSav[LenStk];
#pragma aux DataSav "*"

/* Where INT 21h went before the program took it. This one is of the
   code's segment, not of the data: New21 passes a call of DOS on with
   no register to find the data by, and CS is all that it has. It is
   given a value, 0, which it needs for nothing else: the compiler
   writes what has none in front of what has one, in the code segment
   too, and this would stand where the PSP's room is to. */
u32 CODE Vect21 = 0;
#pragma aux Vect21 "*"

/* Where the part that goes is entered, when the program starts and when
   it has run a command: TrnMain of main.c, which vc.h declares as
   the function that it is, and which is named here as an object, for
   its address. And a text of that part, which is where the file has it
   when the program begins. */
extern const u8 __far TrnMain_s[];
extern const u8 __far Vers_S[];
#pragma aux TrnMain_s "TrnMain_c"

/* The segment of something of the part that goes, as the linker has it:
   in paragraphs from the PSP. Every other word of the program that
   holds a segment has the place of the program added when that is
   loaded. This one is of the part that stays and names the part that
   goes, which is somewhere else after every command: tools/mkexe.c
   leaves it as it is, and what uses it adds where that part is: RunPar,
   or the PSP when the program begins and the part is where the file has
   it. That the high word of a far pointer is that number is this
   compiler's and its linker's doing, not the language's. */
#define TRN_SEG(object) ((u16)((u32)(const void __far *)(object) >> 16))

/* The PSP, which this segment begins with: where the program goes on
   when DOS has ended it, and the segment of its environment. */
#define PSP_EXIT (*(u32 __near *)0x0A)
#define PSP_ENV  (*(u16 __near *)0x2C)

/* This segment, as a number: DS. */
#define RES_SEG ((u16)((u32)(void __far *)&Counter >> 16))

#define AT(segment, offset) \
    ((u8 __far *)((u32)(segment) << 16 | (u16)(offset)))
#define WORD_AT(segment, offset) (*(u16 __far *)AT(segment, offset))
#define OFFSET(object) ((u16)(u32)(const void __far *)(object))

#define DOS_END      0x0000             /* DOS: end the program, the old
                                           way */
#define DOS_DRIVE    0x1900             /* the current drive, to AL */
#define DOS_VECTOR   0x2500             /* an interrupt, AL, to DS:DX */
#define DOS_VERSION  0x3000             /* which DOS it is, to AL and AH */
#define DOS_BREAK    0x3300             /* whether DOS looks for Ctrl-C at
                                           every call, to DL, and with AL 1
                                           from DL */
#define DOS_PATH     0x4700             /* the current directory of the
                                           drive in DL, to DS:SI */
#define DOS_ALLOC    0x4800             /* a block of BX paragraphs, to AX */
#define DOS_FREE     0x4900             /* free the block in ES */
#define DOS_RESIZE   0x4A00             /* make the block in ES BX
                                           paragraphs long */
#define DOS_EXEC     0x4B00             /* run the program at DS:DX, with
                                           what ES:BX says */
#define DOS_EXIT     0x4C00             /* end the program, with AL */
#define DOS_CODE     0x4D00             /* what the last program ended
                                           with, to AX */
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
#define FLAG_DIRECTION 0x0400
#define FLAG_OVERFLOW  0x0800

/* The six that an instruction leaves of what it has worked out. */
#define FLAGS_RESULT (FLAG_CARRY | FLAG_PARITY | FLAG_AUX | FLAG_ZERO \
                      | FLAG_SIGN | FLAG_OVERFLOW)

/* Where things are in the original, as it was shipped: what its
   registers hold when it calls the BIOS with them unset, and the BIOS
   takes them for a page of the screen and a colour. ComnIni is at 25E5h
   there and laid out as here; the part that stays is 31Ah paragraphs. */
#define COMNINI_WAS 0x25E5
#define WAS(address) ((u16)(address) - (u16)&ComnIni + COMNINI_WAS)
#define EPB_WAS     WAS(&ComnIni.EPB)
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

/* What the map of the memory calls, of the part that goes: far, with
   DS anywhere, as SetBrk is. Every register is kept but AX. */
void __far __loadds Release(u16 record);
#pragma aux Release "*" parm [ax] modify exact [ax]

static u16 GetStat(void);
static void SetStat(u16 state);
static void Stay(void);
#pragma aux Stay aborts

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
void __far __loadds Release(u16 record)
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
#pragma aux CtrlC "*"

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
   old address is read through CS: the data of this part is of the
   segment that its code is in. */
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
#pragma aux New21 "*"

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
#pragma aux New27 "*"

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
   'VC', DX 'OM'): it gets in ES:BX what is kept of the first panel for
   AL 0, of the second for 1 and of the main data for anything else, and
   the number of this interface in AH.

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

/* The same for a text of this segment.

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
   or Esc: 1 for Esc, to give up. Both places that call have the address
   of ComnIni.EPB in BX, which DOS was to run the file with. */
static u16 NotFind(const u8 *text, const u8 __far *name)
{
    OutStr0(text, EPB_WAS);
    OutStr(name, EPB_WAS);
    OutStr0(Retr_S, EPB_WAS);
    for (;;) {
        u8 key = (u8)Int16(0);

        if (key == 0x0D)
            return 0;
        if (key == 0x1B)
            return 1;
    }
}

/* Note whether DOS looks for Ctrl-C at every call, have it not do so,
   and take Ctrl-C: when the program starts, and after every command.
   The code that starts the program calls this through a far pointer,
   with DS anywhere. */
void __far __loadds SetBrk(void)
{
    struct DosRegs regs;

    regs.name = 0;
    regs.ax = DOS_BREAK;
    Int21e(&regs);
    ComnIni.BreakFl = (u8)DX_OF(regs);
    DX_OF(regs) = 0;
    regs.ax = DOS_BREAK + 1;
    Int21e(&regs);
    regs.name = AT(RES_SEG, OFFSET(CtrlC));
    regs.ax = DOS_VECTOR + 0x23;
    Int21e(&regs);
}
#pragma aux SetBrk "*" modify exact [ax dx]

/* Leave: have DOS look for Ctrl-C as it did, release the programs that
   have stayed in the memory, and give INT 21h and INT 27h back. Then end
   the program with `code`, unless DOS has ended it already and this is
   where it went on: then return.

   The last program of the list is released as many times as the list is
   long; releasing it takes it off the list, by New21 (see FreeBlk).

   `r` and `flags` are the registers and the flags that this was entered
   with, and they are left as the assembly leaves its own, for the
   program that this returns to when DOS has ended this one: ES on this
   segment; CX 0, the count of the programs run down; SI twice the
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

    r->es = RES_SEG;                    /* Exit01 */
    r->dx = r->dx & 0xFF00 | ComnIni.BreakFl;
    regs.name = 0;
    DX_OF(regs) = ComnIni.BreakFl;
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
    if (ComnIni.ChkTSR != 0) {          /* Exit03 */
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
static void Exit00(u16 code)
{
    struct Saved r;
    u16 flags;

    PSP_EXIT = Vect22;
    Vect22 &= 0xFFFF;
    Exit02(code, &r, &flags);
}
#pragma aux Exit00 aborts

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
#pragma aux Exit01 "*"

void __interrupt __far Exit05(struct Saved r, struct Frame on)
{
    Exit02(r.ax, &r, &on.flags);
    on.ip = (u16)Vect22;
    on.cs = (u16)(Vect22 >> 16);
}

/* No memory: say so and leave. `bx` is what the assembly has in BX
   where it jumps here, a number of DOS's or an address. */
static void AlocEr(u16 bx)
{
    OutStr0(Aloc_S, bx);
    Exit00(1);
}
#pragma aux AlocEr aborts

/* COMMAND.COM's interrupt for a command: `command` is its length, its
   text and a carriage return. It comes back with nothing as it was but
   where it comes back to: no stack, no DS. So this does not return. It
   is the one place where the program has to set its stack by hand, and
   it goes on in Exec2E, on a stack that is empty again.

   Why this is assembly, all nine instructions of it: no compiled
   instruction can run before SS, SP and DS are set, since any of them
   may use the stack or the data; they are set from CS, which is this
   segment and all that is left; and the function that called cannot be
   gone back into, with its place on the stack lost, so the jump is of
   it too. Interrupts are off meanwhile and the direction flag is clear
   afterwards, as the compiler has it. */
static void Exec2E(void);
#pragma aux Exec2E aborts

__declspec(naked) static void Int2E(const u8 *command)
{
    (void)command;                      /* in SI, where the interrupt
                                           takes it */
    _asm {
        int 2Eh
        cli
        mov ax,cs
        mov ss,ax
        mov sp,offset DataSav + LenStk
        sti
        mov ds,ax
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
        DX_OF(regs) = ComnIni.BreakFl;
        regs.ax = DOS_BREAK + 1;
        Int21e(&regs);
        command = (u8 *)ComnIni.BatAdr;
        before = command - 4;
        for (end = command; *end++ != 0; )
            ;
        ComnIni.BatAdr = (u16)end;
        for (end = command, count = LenCmd; count; count--)
            if (*end++ == 0)
                break;
        end--;
        *end = 0x0D;
        if (ComnIni.ExecTyp != 0) {
            before += 3;
            *before = (u8)(end - 1 - before);
            if (ComnIni.DosChg[1] != 0) {
                ComnIni.BatPoin = (u16)before;
                Int2E(ComnIni.DosChg);
            }
            Int2E(before);              /* Exec03 */
        }
        before[0] = (u8)(end - 1 - before);     /* Exec04 */
        before[1] = '/';
        before[2] = 'C';
        before[3] = ' ';
        ComnIni.EPB.CmdLine = (u32)RES_SEG << 16 | (u16)before;
        for (;;) {                      /* Exec05 */
            if (ComnIni.ComSpec == 0) {
                name = (const u8 __far *)Command;
            } else {
                name = AT(PSP_ENV, ComnIni.ComSpec);
                regs.name = name;
                regs.bx = (u16)&ComnIni.EPB;
                regs.es = RES_SEG;
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
        command = (u8 *)ComnIni.BatAdr;
        if (*command == 0)
            return;
        if (*command == '@') {
            ComnIni.BatAdr++;
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
        if (ComnIni.Prompt != 0) {
            *to++ = ':';
            *to++ = '\\';
            regs.name = AT(RES_SEG, (u8)(drive + 1));
            regs.si = (u16)to;
            regs.ax = DOS_PATH;
            Int21e(&regs);
            while (*to != 0)
                to++;
        }
        *to++ = '>';                    /* Exec09 */
        *to = 0;
        OutStr0((const u8 *)IntTab, WAS(ComnIni.BatAdr));
        OutStr0((const u8 *)ComnIni.BatAdr, WAS(ComnIni.BatAdr));
        bios.ax = 0x0E0D;
        bios.bx = WAS(ComnIni.BatAdr);
        bios.cx = bios.dx = 0;
        Int10(&bios);
        bios.ax = 0x0E0A;
        Int10(&bios);
    }
}

/*---------------- Relocate the transit part -------------
   The part that goes has just been loaded, into the block that RunPar
   goes with. The linker's relocations for it are behind it, where
   tools/mkexe.c has put them: each is the place of a word that holds a
   segment, as the linker counts them, from the PSP, and so is what the
   word holds. RunPar is what a segment of the file has become. DOS
   cannot do this: the program loads that part itself, time after
   time. */
static void Reloc(void)
{
    const u16 __far *entry = (const u16 __far *)AT(RunPar + RelTab, 0);
    u16 count;

    for (count = RelCnt; count; count--) {
        u16 offset = *entry++;
        u16 segment = *entry++;

        *(u16 __far *)AT(RunPar + segment, offset) += RunPar;
    }
}

/* The commands are done. If the part that goes was given up for them,
   bring it back: a block of memory for it, and the program's own file
   run as a second copy, with a command tail that says where the block
   is. The copy moves its part there and ends, and leaves the check sum
   of its file in the first two bytes of DosBuf: that has to be this
   file's, and the copy's answer 0. If not, or if the file cannot be
   run, the program says so and asks whether to try again; it ends if
   not. Returns when the part is there.

   Ported as it is: whether the block was to be had is taken from its
   address less the paragraphs of TrnPrW, not from what DOS said. DOS
   answers 8 or 7 when it has no block, which is less than those. */
static void Exec10(void)
{
    struct DosRegs regs;
    const u8 __far *file;
    u16 state, block;
    u8 was;

    was = ComnIni.LoadFl;
    ComnIni.LoadFl = ComnIni.LoadRes;
    if ((was & 0x7F) == 0)
        return;
    state = GetStat();                  /* Exec11: Load VC.EXE */
    regs.bx = ImgEnd - TrnPrW;
    regs.ax = DOS_ALLOC;
    Int21e(&regs);
    block = regs.ax;
    SetStat(state);
    RunPar = block - TrnPrW;
    if (block < TrnPrW) {
        /* BX is what setting DOS's way of looking for memory back has
           left there, whether the upper memory was in the chain; or,
           with a DOS that knows of none, the largest block that DOS
           said it has. */
        if (state == 0xFFFF)
            AlocEr(regs.bx);
        AlocEr(state & 0xFF);
    }
    for (;;) {                          /* Exec12 */
        *(u16 *)ComnIni.DosBuf = ImgEnd;
        *(u16 *)(OvlPrm + OvlPrm[0] - 1) = RES_SEG;
        ComnIni.EPB.CmdLine = (u32)RES_SEG << 16 | (u16)OvlPrm;
        file = AT(PSP_ENV, ComnIni.OvlPth);
        regs.name = file;
        regs.bx = (u16)&ComnIni.EPB;
        regs.es = RES_SEG;
        regs.ax = DOS_EXEC;
        if (!Int21e(&regs)) {
            regs.ax = DOS_CODE;         /* Exec14 */
            Int21e(&regs);
            if (regs.ax == 0 && *(u16 *)ComnIni.DosBuf == ChkFile) {
                Reloc();
                return;
            }
        } else if ((u8)regs.ax > 3) {
            AlocEr(EPB_WAS);
        }
        if (NotFind(BadFl_S, file))     /* Exec13: VC.EXE not found */
            Exit00(1);
    }
}

/* Run the commands of DosBuf, which the part that goes has left in what
   it keeps here of the main data, ComnIni. That part is given up first,
   its block of memory freed, if the program is set up so: LoadFl. */
static void Exec(void)
{
    struct DosRegs regs;
    u16 state;

    if (ComnIni.LoadFl & 0x7F) {
        state = GetStat();
        regs.es = RunPar + TrnPrW;
        regs.ax = DOS_FREE;
        Int21e(&regs);
        SetStat(state);
    }
    ComnIni.BatAdr = (u16)ComnIni.DosBuf;       /* Exec01 */
    Exec02(0);
    Exec10();
}

/*--------------------- The outermost loop ---------------------
   The part that goes is called far, at TrnMain, in the segment that
   the linker has for that and RunPar make, with something that is not 0
   when the program has just been
   loaded, and comes back when it has something for this part to do: a
   number in AX, the low word. With no memory for the main data it has
   in DX, the high word, the paragraphs that DOS had, as Init11 of
   start.c returns them: what the assembly has in BX there. It is a
   function of C, TrnMain of main.c, which has DS elsewhere meanwhile
   and puts it back. Where to call is kept in memory, TrnAdr, and Stay
   and Stay11 have no variables: that was needed while the part came
   back through the assembly, with no register as it was, BP among them,
   which the compiler takes to be kept whatever is said. */
typedef u32 __far Transit(u16 first);
#pragma aux Transit parm [ax] value [dx ax] \
                    modify exact [ax bx cx dx si di es]

#define TO_EXEC  0x100                  /* run the commands of DosBuf */
#define TO_ALLOC 0x200                  /* there is no memory for the main
                                           data: say so and leave */
                                        /* less: leave, with that for DOS */

#define TrnAdr (*(Transit * __near *)&Stays.TrnAdr)

static void Go(u32 back)
{
    u16 what = (u16)back;

    if (what == TO_EXEC)
        Exec();
    else if (what == TO_ALLOC)
        AlocEr((u16)(back >> 16));
    else
        Exit00(what);
}

/* Call the part that goes, where it sets itself up after a command,
   Init11 of the original, and do what it comes back with: for ever. */
static void Stay11(void)
{
    for (;;) {
        TrnAdr = (Transit *)AT(RunPar + TRN_SEG(TrnMain_s),
                               OFFSET(TrnMain_s));
        Go(TrnAdr(0));
    }
}
#pragma aux Stay11 aborts

/* The same from the start of the program, where that part reads the
   setup first: Init00 of the original. */
static void Stay(void)
{
    TrnAdr = (Transit *)AT(RunPar + TRN_SEG(TrnMain_s), OFFSET(TrnMain_s));
    Go(TrnAdr(1));
    Stay11();
}

/* Behind INT 2Eh, on an empty stack. If that was the command of DosChg,
   which changes the drive, the command itself is next, at BatPoin. Else
   a command of DosBuf has been run: on with those that follow it, and
   then as Exec and Stay go on. */
static void Exec2E(void)
{
    if (ComnIni.DosChg[1] != 0) {
        ComnIni.DosChg[1] = 0;
        Int2E((const u8 *)ComnIni.BatPoin);
    }
    SetBrk();                           /* SetBrk0 */
    Exec02(1);
    Exec10();
    Stay11();
}

/*---------------------- The entry ----------------------*/

/* Add the bytes of `count` paragraphs from the segment `from` on to
   `sum`. */
static u16 AllocS(u16 sum, u16 from, u16 count)
{
    do {
        const u8 __far *byte = AT(from, 0);
        u16 i;

        for (i = 16; i; i--)
            sum += *byte++;
        from++;
    } while (--count);
    return sum;
}

/* Move `count` paragraphs from the segment `from` to the segment `to`,
   the last first. */
static void MovPars(u16 from, u16 to, u16 count)
{
    while (count) {
        const u8 __far *source;
        u8 __far *target;
        u16 i;

        count--;
        source = AT(from + count, 16);
        target = AT(to + count, 16);
        for (i = 16; i; i--)
            *--target = *--source;
    }
}

/* No memory when the program begins: say so and end. */
static void Alloc9(void)
{
    struct DosRegs regs;

    OutStr0(Aloc_S, RESPARS_WAS);       /* BX: the paragraphs of the part
                                           that stays */
    SetStat(0);
    regs.ax = DOS_EXIT + 1;
    Int21e(&regs);
}
#pragma aux Alloc9 aborts

/* The program begins here: DOS has loaded the file and enters it with DS
   on the PSP, which is this segment, and with the stack that the header
   of the file names, which is this part's own (see tools/mkexe.c). The
   file is the program as it lies in the memory, so the part that goes
   is where it will run, a paragraph behind this one.

   First the check sum of the file as it is loaded, by which the program
   knows it again after a command: all of it but the data of this part,
   where the stack is, and in use.

   If the command tail is the one that Exec10 gives a second copy, this
   is one: it moves its part that goes into the block that the first
   copy has for it, leaves its check sum there, takes the length that
   the first has for the file, and ends, with 1 if that is not the
   length of its own.

   Else the program is starting. The compiled code of this part names
   its segment, the PSP's, in a few places: the linker's relocations for
   that are applied now, when the sum has been taken, and until then
   nothing here calls what needs them. DOS has to be 3.20 at the least.
   The block of memory that DOS has given the program is made as long as
   the file, and of the part that goes a block of its own, by writing
   the header that DOS keeps in front of one. Then that part is
   relocated, and Stay calls it.

   Ported as it is: how DOS looks for memory is set to what it is when
   DOS starts, the lowest block that fits and no upper memory, whatever
   it was.

   Not in the C: the original cannot always make its block as long
   as it needs, since its file is shorter than the program in the memory,
   and then asks for a block elsewhere for the part that goes. The
   translation's block holds all of the file, and making it as long as the
   file cannot fail but with DOS's chain of blocks destroyed, where asking
   for another fails as well. So that way is left out, and the program
   ends as the original does when both fail. */
void Alloc0(void)
{
    struct DosRegs regs;
    const u8 *tail = (const u8 *)0x80;
    u16 psp = RES_SEG;
    u16 sum, i, block, rest;
    u8 __far *header;
    u8 kind;

    IntOn();                            /* for the direction flag, which
                                           the compiler wants clear */
    sum = AllocS(0, psp + 0x10, DatPrW - 0x10);
    sum = AllocS(sum, psp + TrnPrW, ImgEnd - TrnPrW);
    for (i = 0; i < (u16)OvlPrm[0] - 1; i++)
        if (tail[i] != OvlPrm[i])
            break;
    if (i == (u16)OvlPrm[0] - 1 && tail[i + 2] == 0x0D) {
        u16 first = *(const u16 *)(tail + i);
        u16 __far *size = (u16 __far *)AT(first, (u16)ComnIni.DosBuf);
        u16 length = *size;

        *size = sum;
        MovPars(psp + TrnPrW, WORD_AT(first, (u16)&RunPar) + TrnPrW,
                length - TrnPrW);
        regs.ax = DOS_EXIT + (length != ImgEnd);
        Int21e(&regs);                  /* Alloc2 */
    }
    ChkFile = sum;                      /* Alloc3 */
    {
        const u16 __far *entry =
            (const u16 __far *)AT(psp + RelTab, RelCnt * 4);

        for (i = ResCnt; i; i--) {
            *(u16 *)*entry += psp;
            entry += 2;
        }
    }
    regs.ax = DOS_VERSION;              /* Check DOS version */
    Int21e(&regs);
    if ((u8)regs.ax < 3) {
        regs.ax = DOS_END;              /* Alloc4 */
        Int21e(&regs);
    }
    if (((regs.ax & 0xFF) << 8 | regs.ax >> 8) < 0x314) {
        OutStr(AT(psp + TRN_SEG(Vers_S), OFFSET(Vers_S)),
               regs.bx);                /* BX: as DOS answered */
        regs.ax = DOS_EXIT + 2;
        Int21e(&regs);
    }
    regs.bx = ResPrW + (ImgEnd - TrnPrW) + 1;   /* Alloc5 */
    regs.es = psp;
    regs.ax = DOS_RESIZE;
    if (Int21e(&regs))
        Alloc9();
    block = psp + ResPrW + 1;           /* Alloc6 */
    IntOff();
    /* Split into resident and transit parts */
    header = AT(psp - 1, 0);
    kind = header[0];
    header[0] = 'M';
    rest = *(u16 __far *)(header + 3) - ResPrW - 1;
    *(u16 __far *)(header + 3) = ResPrW;
    header = AT(block - 1, 0);
    header[0] = kind;
    *(u16 __far *)(header + 1) = psp;
    *(u16 __far *)(header + 3) = rest;
    IntOn();
    RunPar = block - TrnPrW;            /* Alloc7 */
    SetStat(0);
    Reloc();
    Stay();
}
#pragma aux Alloc0 "*" aborts
