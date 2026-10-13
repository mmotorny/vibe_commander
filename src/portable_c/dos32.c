/*
 * The system that the program runs on, for DOS on a PC of 32 bits: the
 * functions of sys.h for a program whose memory is one address after
 * another, compiled by Open Watcom's wcc386 in the flat model and
 * started by the DOS extender DOS/32A, which is the stub of its file.
 * dos.c is the same functions for the program of 16 bits, and the build
 * takes one of the two files. What is asked and in which order is the
 * same in both, a function at a time, since that is behaviour: read
 * dos.c beside this.
 *
 * DOS, the BIOS and the mouse driver are of real mode, and the program
 * is not: it calls them by the same software interrupts, and the
 * extender takes each call there and back. What it does on the way was
 * found out by toys on the test bench (TODO.md, "The portable C in 32
 * bits: what a trial on the bench showed", and harness/trial32/):
 *
 *   - A call that takes no address, or the BIOS's or the mouse
 *     driver's, goes over with its registers and comes back with them.
 *   - A call of DOS that takes the address of a name or of a buffer, and
 *     that the extender knows, is translated: the program hands its own
 *     pointer, in a register of 32 bits, and the extender copies what is
 *     there to a buffer of its own below the megabyte and back. Reading
 *     and writing take a count of 32 bits so.
 *   - A call of DOS that the extender does not know goes over with its
 *     registers as they are, and the address in them means nothing
 *     there: what goes by a file control block, AH 13h and 29h, the
 *     country, 38h, and the full path of a name, 60h. Those are made in
 *     real mode, by the extender's service for that, with what DOS
 *     reads and writes in a block below the megabyte that the program
 *     asks for when it starts.
 *   - Four calls that it translates it brings back with the AX that it
 *     was called with where they did not fail, and not with what DOS
 *     left there, which is what sys.h has an answer be: the search of a
 *     directory, AH 4Eh and 4Fh, the current directory, 47h, and the
 *     attributes set, AX 4301h. The toy of harness/trial32/sys.c showed
 *     it. So these are made in real mode here, by the extender's
 *     service for that, with the name and the record in a block below
 *     the megabyte that the program asks for when it starts, and what
 *     DOS wrote there copied to where the program wants it.
 *   - An interrupt of real mode that the program takes, DOS's question
 *     about a device, Ctrl-Break, Ctrl-C, is taken through the
 *     extender, which comes over from real mode and enters a routine of
 *     protected mode, on a stack of its own. And the extender is
 *     entered on the program's stack at every call, and keeps there
 *     what it needs of the call that is under way.
 *   - The memory that the program asks for is neither DOS's nor the
 *     extender's, but 640 KB of the program's own, handed out as DOS
 *     hands out its memory: see MemTake.
 *
 * The memory of the machine that the program reads and writes, the
 * BIOS's data and the cells of the display, is at the addresses that it
 * has in the machine, through plain pointers: the program's segments
 * begin at 0.
 *
 * Each interrupt is written once, as inline assembly, in a #pragma aux
 * of its own, as in dos.c. A register goes into the instruction with all
 * of its 32 bits, since the extender reads all of them where it
 * translates; of what comes back the program reads the low words, which
 * is what DOS and the BIOS answer in.
 *
 * The program begins here too, at Start, where the extender enters it.
 *
 * The file has two parts: the functions of sys.h, fourteen of them made
 * in real mode, and the start of the program.
 */
#include "vc.h"

/* The registers of a call of DOS. A pointer to what DOS takes, a name or
   a buffer, is the register itself; of a call that is made in real
   mode, `low`, it is a place in the block below the megabyte, and DS
   and ES are on that block. */
struct DosRegs {
    u32 eax, ebx, ecx, edx, esi, edi;
    u16 low;
};

/* The registers of a call in real mode, as the extender's service for
   one takes them and hands them back: DPMI's function 0300h. With SS
   and SP 0 the call is made on a stack of the extender's. */
struct RealRegs {
    u32 edi, esi, ebp, reserved, ebx, edx, ecx, eax;
    u16 flags, es, ds, fs, gs, ip, cs, sp, ss;
};

/* The registers of a call of the BIOS for the screen, as it leaves
   them. */
struct BiosRegs {
    u16 ax, bx, cx, dx;
};

/* The compiler's own functions for the interrupt flag: an instruction
   each, put where they are called. */
void _disable(void);
void _enable(void);
#pragma intrinsic(_disable, _enable)

/* The interrupts. Each of these is put in place where it is called, as
   the instructions between the quotes. The compiler takes ES to be on
   the data and the direction flag to be clear, and of an interrupt that
   went through the extender and real mode neither is promised here: the
   toy of harness/trial32/sys.c looks at both behind every call. */

/* INT 21h with the registers of `regs`. EAX, EBX, ECX and EDX go back
   into `regs` as DOS leaves them, and what is returned is not 0 if the
   carry flag is set then. The pointer to `regs` waits on the stack
   meanwhile, where EBP is kept too. POP, MOV and XCHG leave the flags
   alone. The carry flag is not set first for the calls of DOS that
   leave it as they find it, as dos.c sets it: the extender does not
   hand it on. See Ask. */
static u32 Dos(struct DosRegs *regs);
#pragma aux Dos = \
    "push ebp" \
    "push ebx" \
    "mov eax,[ebx]" \
    "mov ecx,[ebx+8]" \
    "mov edx,[ebx+12]" \
    "mov esi,[ebx+16]" \
    "mov edi,[ebx+20]" \
    "mov ebx,[ebx+4]" \
    "int 21h" \
    "xchg ebx,[esp]" \
    "mov [ebx],eax" \
    "mov [ebx+8],ecx" \
    "mov [ebx+12],edx" \
    "pop eax" \
    "mov [ebx+4],eax" \
    "pop ebp" \
    "sbb eax,eax" \
    parm [ebx] value [eax] modify exact [eax ebx ecx edx esi edi]

/* INT 31h, the extender's own services, which are DPMI's, with the
   registers of `regs`, as Dos has DOS's: EAX, EBX, ECX and EDX go back
   into `regs`, and what is returned is not 0 if the extender leaves the
   carry flag set, which is how it says that it refused. EDI is the
   address of what a service takes at ES:EDI. */
static u32 Dpmi(struct DosRegs *regs);
#pragma aux Dpmi = \
    "push ebp" \
    "push ebx" \
    "mov eax,[ebx]" \
    "mov ecx,[ebx+8]" \
    "mov edx,[ebx+12]" \
    "mov esi,[ebx+16]" \
    "mov edi,[ebx+20]" \
    "mov ebx,[ebx+4]" \
    "int 31h" \
    "xchg ebx,[esp]" \
    "mov [ebx],eax" \
    "mov [ebx+8],ecx" \
    "mov [ebx+12],edx" \
    "pop eax" \
    "mov [ebx+4],eax" \
    "pop ebp" \
    "sbb eax,eax" \
    parm [ebx] value [eax] modify exact [eax ebx ecx edx esi edi]

/* INT 10h with EAX, EBX, ECX and EDX, whose low words go back into the
   four words at `regs` as the BIOS leaves them: more than a function can
   return. EBP is kept around it, as dos.c keeps BP. */
static void Video(u32 eax, u32 ebx, u32 ecx, u32 edx, struct BiosRegs *regs);
#pragma aux Video = \
    "push ebp" \
    "int 10h" \
    "pop ebp" \
    "mov [esi],ax" \
    "mov [esi+2],bx" \
    "mov [esi+4],cx" \
    "mov [esi+6],dx" \
    parm [eax] [ebx] [ecx] [edx] [esi] modify exact [eax ebx ecx edx]

/* INT 16h, with AX as it leaves it and, above that, the low word of the
   flags. */
static u32 Keyboard(u32 eax);
#pragma aux Keyboard = \
    "int 16h" \
    "pushfd" \
    "pop edx" \
    "shl edx,16" \
    "mov dx,ax" \
    parm [eax] value [edx] modify exact [eax edx]

/* INT 2Fh, with AX as it leaves it and, above that, BX. */
static u32 Multiplex(u32 eax, u32 ebx, u32 ecx);
#pragma aux Multiplex = \
    "int 2Fh" \
    "shl ebx,16" \
    "mov bx,ax" \
    parm [eax] [ebx] [ecx] value [ebx] modify exact [eax ebx]

/* INT 28h, which leaves every register as it finds it. */
static void DosIdle(void);
#pragma aux DosIdle = \
    "int 28h" \
    modify exact []

/* INT 33h, with AX as it leaves it. */
static u32 MouseDriver(u32 eax, u32 ebx, u32 ecx, u32 edx);
#pragma aux MouseDriver = \
    "int 33h" \
    parm [eax] [ebx] [ecx] [edx] value [eax] modify exact [eax ebx ecx edx]

/* INT 33h, with BX, CX and DX as it leaves them, in the three words at
   `state`: more than a function can return. */
static void MouseState(u32 eax, struct MouseAt *state);
#pragma aux MouseState = \
    "int 33h" \
    "mov [edi],bx" \
    "mov [edi+2],cx" \
    "mov [edi+4],dx" \
    parm [eax] [edi] modify exact [eax ebx ecx edx]

/*
 * Four of the interrupts as functions, by which everything below calls
 * them: DOS, the BIOS for the screen and for the keyboard, and the mouse
 * driver are each put in place here and nowhere else.
 */

/* The block below the megabyte, by its segment in real mode: Start asks
   for it. A call that is made in real mode has there what DOS reads and
   writes at an address, each thing at a place of its own:

     LOW_NAME     a name or a path, 128 bytes
     LOW_RECORD   the record of a search, 43 bytes
     LOW_SECOND   128 bytes that DOS writes a path to
                  or the command tail of a program that is run, or a
                  command for the shell
     LOW_FCB      a file control block, 44 bytes with what an extended
                  one has in front
     LOW_COUNTRY  what DOS tells of the country, 34 bytes
     LOW_RUN      what DOS is told of a program that it is to run, 14
                  bytes, and the two file control blocks that this
                  names, 16 bytes each
     LOW_SHELL    instructions of real mode around the shell's
                  interrupt, and the four bytes in which they keep the
                  stack
     LOW_ENDED    instructions of real mode for where DOS goes on when
                  it has ended the program by another's hand, and the
                  seven bytes that they read: see ProgramEndedTake */
static u16 LowSeg = 0;

#define LOW_NAME    0
#define LOW_RECORD  128
#define LOW_SECOND  192
#define LOW_FCB     320
#define LOW_COUNTRY 368
#define LOW_RUN     416
#define LOW_FCB1    432
#define LOW_FCB2    448
#define LOW_SHELL   464
#define LOW_KEPT    496
#define LOW_ENDED   512
#define LOW_WAS     544
#define LOW_PSP     548
#define LOW_BREAK   550
#define LOW_SIZE    560
#define LOW(place) ((u8 *)(((u32)LowSeg << 4) + (place)))

#define DPMI_SEGMENT_BASE 0x0006        /* where the segment of the
                                           selector in BX begins: CX above
                                           DX */
#define DPMI_LOW_BLOCK 0x0100           /* a block of DOS's memory: BX
                                           paragraphs; AX its segment */
#define DPMI_REAL_VECTOR 0x0200         /* where the interrupt in BL goes
                                           in real mode: CX:DX */
#define DPMI_SET_REAL_VECTOR 0x0201     /* and that set */
#define DPMI_VECTOR 0x0204              /* where the interrupt in BL goes
                                           in protected mode: CX:EDX */
#define DPMI_SET_VECTOR 0x0205          /* and that set */
#define DPMI_REAL_INT  0x0300           /* an interrupt in real mode: BL
                                           which, CX words of the stack to
                                           take along, ES:EDI the
                                           registers */
#define DPMI_REAL_CALL 0x0301           /* a routine of real mode that
                                           returns far, at the CS:IP of
                                           the registers: the same */
#define FLAG_CARRY 1

/* The stack pointer. */
static u32 StackNow(void);
#pragma aux StackNow = \
    "mov eax,esp" \
    value [eax] modify exact [eax]

/* Where the program's stack is free while DOS is at work for it, in
   real mode: for the routine that DOS's question about a device arrives
   at, which goes on there (see DeviceEntry). It is the stack pointer
   where DOS, or the extender for a call of real mode, was last called,
   less what the extender has of the program's stack meanwhile: it is
   entered on that stack and keeps on it what it needs of the call that
   is under way. The toys saw it write 190 bytes below the stack pointer
   at a call of DOS that it translates, 130 at a call made in real mode
   and 74 at any other, with the 8 that the instructions here push; the
   512 are that and as much again. */
#define STACK_KEPT 512

static u32 Below = 0;

/* The extender, asked for the service `ax` with these registers, and
   with `at` for what the service takes at ES:EDI. EAX, EBX, ECX and EDX
   are in `regs` afterwards as it leaves them. Not 0 if it refused. */
static u16 Extender(struct DosRegs *regs, u16 ax, u32 ebx, u32 ecx, u32 edx,
                    void *at)
{
    regs->eax = ax;
    regs->ebx = ebx;
    regs->ecx = ecx;
    regs->edx = edx;
    regs->esi = 0;
    regs->edi = (u32)at;
    regs->low = 0;
    Below = StackNow() - STACK_KEPT;
    return (u16)Dpmi(regs);
}

/* An interrupt made in real mode, with the registers of `regs` and their
   answer there, and with DS and ES on the block below the megabyte; or,
   if `routine` is not 0, a routine of real mode called far at that
   address, its segment above its offset. On a stack of the extender's.
   Returns the flags as it leaves them. */
static u16 Real(struct DosRegs *regs, u16 number, u32 routine)
{
    struct RealRegs real;
    struct DosRegs asked;

    real.eax = regs->eax;
    real.ebx = regs->ebx;
    real.ecx = regs->ecx;
    real.edx = regs->edx;
    real.esi = regs->esi;
    real.edi = regs->edi;
    real.ebp = 0;
    real.flags = 0;
    real.ds = real.es = LowSeg;
    real.fs = real.gs = 0;
    real.sp = real.ss = 0;
    real.ip = (u16)routine;
    real.cs = (u16)(routine >> 16);
    Extender(&asked, routine ? DPMI_REAL_CALL : DPMI_REAL_INT, number, 0, 0,
             &real);
    regs->eax = real.eax;
    regs->ebx = real.ebx;
    regs->ecx = real.ecx;
    regs->edx = real.edx;
    return real.flags;
}

/* DOS, with the registers of `regs` and their answer there. Not 0 if
   DOS leaves the carry flag set: FFFFh, as in dos.c, which is what a
   function that asks bare answers with for `failed`. */
static u16 Call(struct DosRegs *regs)
{
    if (regs->low)
        return Real(regs, 0x21, 0) & FLAG_CARRY ? 0xFFFF : 0;
    Below = StackNow() - STACK_KEPT;
    return (u16)Dos(regs);
}

/* The BIOS for the screen, with these registers, which come back in
   `regs` as it leaves them. */
static void Display(u16 ax, u16 bx, u16 cx, u16 dx, struct BiosRegs *regs)
{
    Video(ax, bx, cx, dx, regs);
}

/* The BIOS for the keyboard: AX as it leaves it, and above that the
   flags. */
static u32 Keys(u16 ax)
{
    return Keyboard(ax);
}

/* The mouse driver: AX as it leaves it. */
static u16 Mouse(u16 ax, u16 bx, u16 cx, u16 dx)
{
    return (u16)MouseDriver(ax, bx, cx, dx);
}

/*
 * The functions of sys.h. What each is for is said there; here is how
 * DOS, the BIOS and the mouse driver are asked for it, which dos.c says
 * of each call and this does not say again.
 */

/* A pointer as the register that DOS takes it in. */
#define ADDRESS(pointer) ((u32)(pointer))

#define ERR_NOT_READY 0x53              /* the error of a call that the
                                           user gave up, when a device was
                                           not ready */

/* The registers of a call: these four, and 0 in the two others. dos.c
   leaves a register that a call does not read as it happens to be, and
   0 is what this hands DOS there. */
static void Set(struct DosRegs *regs, u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    regs->eax = eax;
    regs->ebx = ebx;
    regs->ecx = ecx;
    regs->edx = edx;
    regs->esi = 0;
    regs->edi = 0;
    regs->low = 0;
}

/* The same for a call that is made in real mode: `edx` is then a place
   in the block below the megabyte, or a number. */
static void SetLow(struct DosRegs *regs, u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    Set(regs, eax, ebx, ecx, edx);
    regs->low = 1;
}

/* Bytes from one place to another. */
static void Bytes(u8 *to, const u8 *from, u16 count)
{
    for (; count != 0; count--)
        *to++ = *from++;
}

/* A name or a path, as far as its zero byte, to the block below the
   megabyte: 127 characters of it at the most, which is more than DOS
   takes. */
static void NameLow(const u8 *name)
{
    u8 *to = LOW(LOW_NAME);
    u16 room;

    for (room = 127; room != 0 && *name != 0; room--)
        *to++ = *name++;
    *to = 0;
}

/* What DOS answers: AX, and whether the carry flag is set. */
static struct Answer Answered(const struct DosRegs *regs, u16 failed)
{
    struct Answer answer;

    answer.value = (u16)regs->eax;
    answer.failed = failed;
    return answer;
}

/* Ask DOS bare. */
static struct Answer Tell(struct DosRegs *regs)
{
    return Answered(regs, Call(regs));
}

/* Ask DOS with the program's errors: Intr21 of the original. When DOS
   asks what to do about a device, the program's routine for that,
   FtErr1 of main.c, shows a box and notes in ErrDat if the user gave up
   there; the call then counts as failed with the error 53h, and until
   the next call that comes here DOS is answered without a box. An error
   that DOS reports itself is asked about once more, and its extended
   code is what comes back: for that call AH is 59h and BX 0, and the
   other registers are as the first call left them, which they are
   afterwards too.

   `flagged` is not 0 where the original has the carry flag set when it
   calls, which it has at two calls that DOS leaves the flag at, 36h and
   13h: the flag is then set afterwards whatever DOS did, and the
   original takes that for an error. dos.c sets the flag and lets DOS
   leave it. Here that does not do: a flag that the program has set does
   not come back set through the extender, as the toy showed of 36h. So
   the flag counts as set afterwards if it was to be set before, which
   is the same for a DOS that leaves it. */
static struct Answer Ask(struct DosRegs *regs, u16 flagged)
{
    u32 ebx, ecx, edx;

    md.ErrDat = 0;
    flagged |= Call(regs);
    if (md.ErrDat != 0) {
        regs->eax = ERR_NOT_READY;
        return Answered(regs, 1);
    }
    if (flagged == 0)
        return Answered(regs, 0);
    ebx = regs->ebx;
    ecx = regs->ecx;
    edx = regs->edx;
    regs->eax = regs->eax & 0xFF | 0x5900;
    regs->ebx = 0;
    Call(regs);
    regs->ebx = ebx;
    regs->ecx = ecx;
    regs->edx = edx;
    return Answered(regs, 1);
}

/* A call that takes a name at EDX and a number in CX, and one that takes
   a handle in BX, a buffer at EDX and a count in ECX: with the program's
   errors. */
static struct Answer Named(u16 ax, const u8 *name, u16 cx)
{
    struct DosRegs regs;

    Set(&regs, ax, 0, cx, ADDRESS(name));
    return Ask(&regs, 0);
}

static struct Answer Handled(u16 ax, u16 handle, u32 edx, u16 count)
{
    struct DosRegs regs;

    Set(&regs, ax, handle, count, edx);
    return Ask(&regs, 0);
}

/* Files. */

struct Answer FileCreate(const u8 FAR *name, u16 attributes)
{
    return Named(0x3C00, name, attributes);
}

struct Answer FileOpen(const u8 FAR *name, u16 mode)
{
    return Named(0x3D00 | mode & 0xFF, name, 0);
}

struct Answer FileOpenBare(const u8 FAR *name, u16 mode)
{
    struct DosRegs regs;

    Set(&regs, 0x3D00 | mode & 0xFF, 0, 0, ADDRESS(name));
    return Tell(&regs);
}

struct Answer FileClose(u16 handle)
{
    return Handled(0x3E00, handle, 0, 0);
}

struct Answer FileCloseBare(u16 handle)
{
    struct DosRegs regs;

    Set(&regs, 0x3E00, handle, 0, 0);
    return Tell(&regs);
}

struct Answer FileRead(u16 handle, u8 FAR *to, u16 count)
{
    return Handled(0x3F00, handle, ADDRESS(to), count);
}

struct Answer FileReadBare(u16 handle, u8 FAR *to, u16 count)
{
    struct DosRegs regs;

    Set(&regs, 0x3F00, handle, count, ADDRESS(to));
    return Tell(&regs);
}

struct Answer FileWrite(u16 handle, const u8 FAR *from, u16 count)
{
    return Handled(0x4000, handle, ADDRESS(from), count);
}

struct FilePlace FileSeek(u16 handle, u16 from, u32 offset)
{
    struct DosRegs regs;
    struct FilePlace sought;

    Set(&regs, 0x4200 | from & 0xFF, handle, offset >> 16, (u16)offset);
    sought.failed = Ask(&regs, 0).failed;
    sought.place = (u32)(u16)regs.edx << 16 | (u16)regs.eax;
    return sought;
}

struct Answer FileDelete(const u8 FAR *name)
{
    return Named(0x4100, name, 0);
}

/* The new name is at EDI, in the segment that ES is on, which is the
   program's as every segment is. */
struct Answer FileRename(const u8 FAR *from, const u8 FAR *to)
{
    struct DosRegs regs;

    Set(&regs, 0x5600, 0, 0, ADDRESS(from));
    regs.edi = ADDRESS(to);
    return Ask(&regs, 0);
}

struct FileAttr FileAttributes(const u8 FAR *name)
{
    struct DosRegs regs;
    struct Answer asked;
    struct FileAttr attr;

    Set(&regs, 0x4300, 0, 0, ADDRESS(name));
    asked = Ask(&regs, 0);
    attr.value = asked.value;
    attr.failed = asked.failed;
    attr.attributes = (u16)regs.ecx;
    return attr;
}

/* In real mode, for what DOS leaves in AX. */
struct Answer FileSetAttributes(const u8 FAR *name, u16 attributes)
{
    struct DosRegs regs;

    NameLow(name);
    SetLow(&regs, 0x4301, 0, attributes, LOW_NAME);
    return Ask(&regs, 0);
}

struct FileStamp FileTime(u16 handle)
{
    struct DosRegs regs;
    struct Answer asked;
    struct FileStamp stamp;

    Set(&regs, 0x5700, handle, 0, 0);
    asked = Ask(&regs, 0);
    stamp.value = asked.value;
    stamp.failed = asked.failed;
    stamp.time = (u16)regs.ecx;
    stamp.date = (u16)regs.edx;
    return stamp;
}

struct Answer FileSetTime(u16 handle, u16 time, u16 date)
{
    return Handled(0x5701, handle, date, time);
}

struct Device FileDevice(u16 handle)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Device device;

    Set(&regs, 0x4400, handle, 0, 0);
    asked = Ask(&regs, 0);
    device.value = asked.value;
    device.failed = asked.failed;
    device.kind = (u16)regs.edx;
    return device;
}

/* In real mode: the extender does not know the call. The name is at
   DS:SI and the path goes to ES:DI, 128 bytes that DOS has in the block
   below the megabyte: the program's go there first and come back
   afterwards, so that what DOS leaves of them is what the program had
   there, failed or not. */
#define PATH_SIZE 128

struct Answer FileTrueName(const u8 FAR *name, u8 FAR *to)
{
    struct DosRegs regs;
    struct Answer asked;

    NameLow(name);
    Bytes(LOW(LOW_SECOND), to, PATH_SIZE);
    SetLow(&regs, 0x6000, 0, 0, 0);
    regs.esi = LOW_NAME;
    regs.edi = LOW_SECOND;
    asked = Ask(&regs, 0);
    Bytes(to, LOW(LOW_SECOND), PATH_SIZE);
    return asked;
}

/* Files by a file control block, in real mode: the extender knows
   neither call. AH 29h, with the name at DS:SI and the block at ES:DI,
   of which DOS writes what an unopened one has, 16 bytes, and reads
   what `how` has it keep: so the program's go down first and come back
   afterwards. And AH 13h, with the block at DS:DX, 37 bytes and 7 in
   front of them where it is an extended one, which is what the program
   hands over. */
#define FCB_UNOPENED 16
#define FCB_EXTENDED 44

u16 FcbParse(const u8 FAR *name, u8 FAR *fcb, u16 how)
{
    struct DosRegs regs;
    u16 value;

    NameLow(name);
    Bytes(LOW(LOW_FCB), fcb, FCB_UNOPENED);
    SetLow(&regs, 0x2900 | how & 0xFF, 0, 0, 0);
    regs.esi = LOW_NAME;
    regs.edi = LOW_FCB;
    value = Tell(&regs).value;
    Bytes(fcb, LOW(LOW_FCB), FCB_UNOPENED);
    return value;
}

struct Answer FcbDelete(const u8 FAR *fcb, u16 flagged)
{
    struct DosRegs regs;

    Bytes(LOW(LOW_FCB), fcb, FCB_EXTENDED);
    SetLow(&regs, 0x1300, 0, 0, LOW_FCB);
    return Ask(&regs, flagged);
}

/* Directories. The search and the current directory are asked in real
   mode, for what DOS leaves in AX.

   The record of a search is the program's, wherever it is, and DOS has
   one in the block below the megabyte. DOS keeps in the record how far
   a search has got, and the program may put the record of one search
   back while another has been made, or have it somewhere else: so the
   program's record goes to DOS's before a search goes on, and DOS's to
   the program's behind every call of a search, failed or not, the 43
   bytes that DOS has there. */

#define RECORD_SIZE 43

static struct DTAs *Record = 0;

void FindRecord(struct DTAs FAR *record)
{
    struct DosRegs regs;

    Record = record;
    SetLow(&regs, 0x1A00, 0, 0, LOW_RECORD);
    Tell(&regs);
}

/* A call of a search, with the record brought up behind it. */
static struct Answer Searched(struct DosRegs *regs)
{
    struct Answer found = Ask(regs, 0);

    if (Record != 0)
        Bytes((u8 *)Record, LOW(LOW_RECORD), RECORD_SIZE);
    return found;
}

struct Answer FindFirst(const u8 FAR *pattern, u16 attributes)
{
    struct DosRegs regs;

    NameLow(pattern);
    SetLow(&regs, 0x4E00, 0, attributes, LOW_NAME);
    return Searched(&regs);
}

struct Answer FindNext(void)
{
    struct DosRegs regs;

    if (Record != 0)
        Bytes(LOW(LOW_RECORD), (const u8 *)Record, RECORD_SIZE);
    SetLow(&regs, 0x4F00, 0, 0, 0);
    return Searched(&regs);
}

struct Answer DirMake(const u8 FAR *name)
{
    return Named(0x3900, name, 0);
}

struct Answer DirRemove(const u8 FAR *name)
{
    return Named(0x3A00, name, 0);
}

struct Answer DirChange(const u8 FAR *name)
{
    return Named(0x3B00, name, 0);
}

/* The drive is in DL and the room for the path at DS:SI: DOS writes the
   path below the megabyte, and it goes to `to` as far as its zero byte
   if DOS did not fail. */
static struct Answer Current(u16 drive, u8 *to, u16 bare)
{
    struct DosRegs regs;
    struct Answer asked;
    const u8 *from = LOW(LOW_NAME);
    u16 room;

    SetLow(&regs, 0x4700, 0, 0, drive & 0xFF);
    regs.esi = LOW_NAME;
    asked = bare ? Tell(&regs) : Ask(&regs, 0);
    if (!asked.failed)
        for (room = 64; room != 0 && (*to++ = *from++) != 0; room--)
            ;
    return asked;
}

struct Answer DirCurrent(u16 drive, u8 FAR *to)
{
    return Current(drive, to, 0);
}

struct Answer DirCurrentBare(u16 drive, u8 FAR *to)
{
    return Current(drive, to, 1);
}

/* Drives. */

u16 DriveCurrent(void)
{
    struct DosRegs regs;

    Set(&regs, 0x1900, 0, 0, 0);
    return Tell(&regs).value & 0xFF;
}

void DriveSelect(u16 drive)
{
    struct DosRegs regs;

    Set(&regs, 0x0E00, 0, 0, drive & 0xFF);
    Tell(&regs);
}

struct Space DriveSpace(u16 drive, u16 flagged)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Space space;

    Set(&regs, 0x3600, 0, 0, drive & 0xFF);
    asked = Ask(&regs, flagged);
    space.value = asked.value;
    space.failed = asked.failed;
    space.free = (u16)regs.ebx;
    space.sector = (u16)regs.ecx;
    space.total = (u16)regs.edx;
    return space;
}

/* One of the calls of AH 44h that take a drive, 1 for A:, in BL. */
static struct Answer Unit(u16 ax, u16 drive, struct DosRegs *regs)
{
    Set(regs, ax, drive & 0xFF, 0, 0);
    return Tell(regs);
}

struct Answer DriveLetter(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x440E, drive, &regs);
}

struct Answer DriveSetLetter(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x440F, drive, &regs);
}

struct Device DriveRemote(u16 drive)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Device device;

    asked = Unit(0x4409, drive, &regs);
    device.value = asked.value;
    device.failed = asked.failed;
    device.kind = (u16)regs.edx;
    return device;
}

struct Answer DriveFixed(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x4408, drive, &regs);
}

u16 DriveCd(u16 drive)
{
    u32 pair = Multiplex(0x150B, 0, drive & 0xFF);

    return (u16)(pair >> 16) == 0xADAD && (u16)pair != 0;
}

/* Memory. In 16 bits it is DOS's, in blocks that DOS hands out by their
   segments, and the program asks for the largest that there is and cuts
   it up by segments itself, for its panels and the text of the viewer.
   It does the same here, as long as a segment is a number in it (vc.h,
   "The words of the addresses"): so the memory that it gets has to be
   where a segment can name it, in the megabyte that begins 64K below
   the main data, and that is no memory of DOS's nor a block that the
   extender gives wherever it likes. It is of the program's own: 640 KB
   among what has no value, which the linker is to have behind the main
   data, with a paragraph in front for what is kept of the first block.

   It is handed out as DOS hands out its own, since the program goes by
   that: a block is known by its paragraph in the megabyte, in front of
   each is a paragraph that says how long it is and whether it is taken,
   the lowest block that is long enough is given, a refusal says how
   long the longest is, and free blocks that lie one behind the other
   become one when the memory is looked through and not when a block is
   given back. What it answers with, DOS's AX and its numbers for an
   error, is what the DOS of the test bench answers, FreeDOS. */

#define OWN_PARAGRAPHS 0xA000           /* 640 KB */

static u8 Own[((u32)OWN_PARAGRAPHS + 1) * 16 + 15];

/* The first paragraph of it in the megabyte, where what is kept of the
   first block is, and how many paragraphs there are from there on: none
   if it is not where a segment can name all of it. */
static u16 OwnFirst = 0, OwnCount = 0;

/* What is kept in front of a block: whether one follows it, whether it
   is taken, and its paragraphs. */
struct Held {
    u8 kind;
    u8 taken;
    u16 paragraphs;
};

#define HELD_MORE 'M'
#define HELD_LAST 'Z'
#define HELD(block) ((struct Held *)AT((block) - 1, 0))

#define ERR_NO_FUNCTION 1               /* DOS's error for a call that it
                                           does not know */
#define ERR_BAD_CHAIN   7               /* for what it keeps of its blocks
                                           being spoilt */
#define ERR_NO_MEMORY   8               /* for want of memory */
#define ERR_NO_BLOCK    9               /* and for a block that is none */

/* The start of the program has all of it be one block that is free. */
static void OwnSet(void)
{
    u32 first = (u32)(Own + 15 - ARENA) >> 4;
    struct Held *held;

    if (first > 0xFFFF - OWN_PARAGRAPHS)
        return;
    OwnFirst = (u16)first;
    OwnCount = OWN_PARAGRAPHS + 1;
    held = HELD(OwnFirst + 1);
    held->kind = HELD_LAST;
    held->taken = 0;
    held->paragraphs = OWN_PARAGRAPHS;
}

/* Is `block` a block, by what is in front of it? */
static u16 Known(u16 block)
{
    struct Held *held;

    if (block <= OwnFirst || (u32)block - OwnFirst > OwnCount)
        return 0;
    held = HELD(block);
    return held->kind == HELD_MORE || held->kind == HELD_LAST;
}

/* The free blocks that follow `block` become part of it. */
static void Join(u16 block)
{
    struct Held *held = HELD(block);
    struct Held *next;

    while (held->kind == HELD_MORE) {
        next = HELD(block + held->paragraphs + 1);
        if (next->taken)
            break;
        held->kind = next->kind;
        held->paragraphs += next->paragraphs + 1;
    }
}

/* `block` made so many paragraphs, which is no more than it has: what
   is left behind it becomes a block that is free. */
static void Split(u16 block, u16 paragraphs)
{
    struct Held *held = HELD(block);
    struct Held *rest;

    if (held->paragraphs == paragraphs)
        return;
    rest = HELD(block + paragraphs + 1);
    rest->kind = held->kind;
    rest->taken = 0;
    rest->paragraphs = held->paragraphs - paragraphs - 1;
    held->kind = HELD_MORE;
    held->paragraphs = paragraphs;
}

static struct Block Refused(u16 error, u16 largest)
{
    struct Block block;

    block.value = error;
    block.failed = 0xFFFF;
    block.largest = largest;
    return block;
}

struct Block MemTake(u16 paragraphs)
{
    struct Block block;
    struct Held *held;
    u16 at = OwnFirst + 1;
    u16 largest = 0;

    if (OwnCount == 0)
        return Refused(ERR_NO_MEMORY, 0);
    for (;;) {
        held = HELD(at);
        if (!Known(at))
            return Refused(ERR_BAD_CHAIN, paragraphs);
        if (!held->taken) {
            Join(at);
            if (held->paragraphs >= paragraphs)
                break;
            if (held->paragraphs > largest)
                largest = held->paragraphs;
        }
        if (held->kind == HELD_LAST)
            return Refused(ERR_NO_MEMORY, largest);
        at += held->paragraphs + 1;
    }
    Split(at, paragraphs);
    held->taken = 1;
    block.value = at;
    block.failed = 0;
    block.largest = paragraphs;
    return block;
}

/* A block that is given back stays as long as it is, and is given back
   a second time without a word, as DOS has it. */
struct Answer MemGive(u16 block)
{
    struct Answer answer;

    if (!Known(block)) {
        answer.value = ERR_NO_BLOCK;
        answer.failed = 0xFFFF;
        return answer;
    }
    HELD(block)->taken = 0;
    answer.value = 0x4900;
    answer.failed = 0;
    return answer;
}

/* A block that is to be longer is first made as long as it can be, by
   what is free behind it, and stays so where that is not long enough:
   the refusal says how long it is then. */
struct Block MemResize(u16 block, u16 paragraphs)
{
    struct Block made;
    struct Held *held = HELD(block);

    if (!Known(block))
        return Refused(ERR_BAD_CHAIN, paragraphs);
    if (paragraphs > held->paragraphs) {
        Join(block);
        if (paragraphs > held->paragraphs)
            return Refused(ERR_NO_MEMORY, held->paragraphs);
    }
    if (paragraphs < held->paragraphs) {
        Split(block, paragraphs);
        Join(block + paragraphs + 1);
    }
    held->taken = 1;
    made.value = block;
    made.failed = 0;
    made.largest = paragraphs;
    return made;
}

/* The memory that the program is in is the extender's to hand out, and
   it has given the program what its file says and no more; what
   MemTake hands out is of the program's own. So there is nothing to
   make shorter, and nothing has failed: `paragraphs` is as it came,
   none. */
struct Block MemOwn(void)
{
    struct Block block;

    block.value = 0;
    block.failed = 0;
    block.largest = 0;
    return block;
}

/* How DOS looks through its own memory, which is what a program that
   this one runs is given of: DOS is told as it is in 16 bits, though
   the program's own blocks are not DOS's here. */
void MemSetSearch(u16 how)
{
    struct DosRegs regs;

    Set(&regs, 0x5801, how, 0, 0);
    Tell(&regs);
}

void MemSetUpper(u16 on)
{
    struct DosRegs regs;

    Set(&regs, 0x5803, on, 0, 0);
    Tell(&regs);
}

/* The time, the country, the system itself. */

struct Time TimeOfDay(void)
{
    struct DosRegs regs;
    struct Time time;

    Set(&regs, 0x2C00, 0, 0, 0);
    Tell(&regs);
    time.clock = (u16)regs.ecx;
    time.seconds = (u16)regs.edx;
    return time;
}

/* The country, AH 38h, in real mode: the extender passes the call on
   and does not know that it takes an address. DOS writes 34 bytes at
   DS:DX, or fewer: the program's go down first and come back
   afterwards, so that it has what DOS wrote and, of the rest, what it
   had. Among them is the address of DOS's routine for capitals, which
   is one of real mode. */
#define COUNTRY_SIZE 34
#define COUNTRY_CAPITALS 18

static u32 Capitals = 0;

void CountryInfo(struct CNTRY FAR *to)
{
    struct DosRegs regs;
    u8 *low = LOW(LOW_COUNTRY);

    Bytes(low, (const u8 *)to, COUNTRY_SIZE);
    SetLow(&regs, 0x3800, 0, 0, LOW_COUNTRY);
    Tell(&regs);
    Bytes((u8 *)to, low, COUNTRY_SIZE);
    Capitals = *(const u32 *)(low + COUNTRY_CAPITALS);
}

/* DOS's routine is called far in real mode, by the extender's service
   for that, with the letter in AL and back there. Until the country has
   been asked for there is no routine, and the letter comes back as it
   went. */
u16 CaseMap(u16 letter)
{
    struct DosRegs regs;

    if (Capitals == 0)
        return letter;
    SetLow(&regs, letter, 0, 0, 0);
    Real(&regs, 0, Capitals);
    return (u16)regs.eax;
}

struct SysVersion SystemVersion(void)
{
    struct DosRegs regs;
    struct SysVersion version;

    Set(&regs, 0x3000, 0, 0, 0);
    Tell(&regs);
    version.version = (u16)regs.eax;
    version.other = (u16)regs.ebx;
    return version;
}

struct SysVersion SystemTrueVersion(void)
{
    struct DosRegs regs;
    struct SysVersion version;

    Set(&regs, 0x3306, 0, 0, 0);
    Tell(&regs);
    version.version = (u16)regs.ebx;
    version.other = (u16)regs.eax;
    return version;
}

u16 BreakCheck(void)
{
    struct DosRegs regs;

    Set(&regs, 0x3300, 0, 0, 0);
    Tell(&regs);
    return (u16)regs.edx & 0xFF;
}

void SetBreakCheck(u16 on)
{
    struct DosRegs regs;

    Set(&regs, 0x3301, 0, 0, on & 0xFF);
    Tell(&regs);
}

/* The extender takes it to real mode, where the call comes from the
   extender's own code, in the block of the memory that DOS has for the
   program, and on a stack of the extender's. */
void Idle(void)
{
    DosIdle();
}

/* What DOS and the BIOS enter of the program: three interrupts of real
   mode. The program takes them through the extender, which is told a
   routine of protected mode for each (DPMI 0205h) and has the interrupt
   of real mode go to a routine of its own that comes over to protected
   mode and enters that. Who had an interrupt is asked of the extender
   too (0204h), and is an address of protected mode, a selector and an
   offset of 32 bits: so it is kept here, and not in ErrVct and BrkVct of
   the main data, which have four bytes each. DOS is not asked, where
   dos.c asks it AH 35h and 25h. */

#define INT_CTRL_BREAK 0x1B             /* the BIOS's, from the keyboard */
#define INT_CTRL_C     0x23             /* DOS's, where it finds the key */
#define INT_ERROR      0x24             /* DOS's critical error */

/* Where an interrupt goes in protected mode. */
struct Vector {
    u32 offset;
    u16 selector;
};

/* Who had each of the three; and which of them the program has now, a
   bit each, so that what it has not given back when it ends can be
   (see ProgramEnd). */
static struct Vector ErrorWas = { 0, 0 };
static struct Vector BreakWas = { 0, 0 };
static struct Vector CtrlCWas = { 0, 0 };

#define TAKEN_ERROR  1
#define TAKEN_BREAK  2
#define TAKEN_CTRL_C 4

static u16 Taken = 0;

/* The selectors of the program's code and of its data: Start notes
   them. A routine that the extender enters finds the data by the
   second. */
static u16 CodeSel = 0, DataSel = 0;

static void Note(u16 number, struct Vector *was)
{
    struct DosRegs regs;

    Extender(&regs, DPMI_VECTOR, number, 0, 0, 0);
    was->offset = regs.edx;
    was->selector = (u16)regs.ecx;
}

static void Go(u16 number, u16 selector, u32 offset)
{
    struct DosRegs regs;

    Extender(&regs, DPMI_SET_VECTOR, number, selector, offset, 0);
}

/* An interrupt given back to whoever was noted, if someone was. */
static void Give(u16 number, const struct Vector *was, u16 taken)
{
    if (was->selector != 0)
        Go(number, was->selector, was->offset);
    Taken &= ~taken;
}

/* The function that the program has handed DeviceErrorTake. */
static DeviceErrorFn *DeviceAsk = 0;

/* The header of a device, which DOS names when it asks about one: the
   high byte of its attributes, where 80h says that it is no drive, and
   the name that such a device has there, 8 characters. */
#define HEADER_KIND 5
#define HEADER_NAME 10

/* DOS asks what to do about a device: INT 24h, as DeviceEntry below
   hands it on. `how` and `error` are AX and DI as DOS has them, and the
   header of the device is at BP:SI of real mode, `base` and `offset`.
   The program is asked, by the function that it handed over, with what
   sys.h says that such a function takes, as FatErr of dos.c asks it;
   what it answers goes back to DOS. Where the stack is free is kept
   around that, since the function may ask what sets it anew. */
static u32 Arrived(u32 how, u32 error, u32 base, u32 offset)
{
    const u8 *device = (const u8 *)(((base & 0xFFFF) << 4)
                                    + (offset & 0xFFFF));
    u32 below = Below;
    u16 answer;

    if (device[HEADER_KIND] & 0x80)
        device += HEADER_NAME;
    else
        device = 0;
    answer = DeviceAsk((u16)how, (u16)error, device);
    Below = below;
    return answer;
}
#pragma aux Arrived parm [eax] [edi] [edx] [esi] value [eax]

/* Where the extender enters the program for INT 24h. DOS is in the
   middle of something that the program asked of it, or that a program
   asked which this one has run; it has called the interrupt in real
   mode, and the extender has come over. The registers are DOS's, their
   low words: what it says of the error in AX and DI, the header of the
   device at BP:SI. Everything else is the extender's: DS and ES are
   anywhere, and the stack is one of its own, 512 bytes by what its
   settings say, which is not enough for what the program does when it
   is asked, a box drawn and a wait for a key, with the BIOS, the mouse
   and the idle call each asked through the extender again.

   So this is assembly, since no compiled instruction can run before DS
   is on the data and the stack is the program's: the registers are
   kept on the extender's stack; DS and ES are put on the data, by the
   selector that Start noted, read through CS, which is on the same
   memory; the stack goes over to the program's own, at Below, with
   where the extender's was kept on top of it; Arrived is called; and
   then all of it back, with what Arrived answered in AX for DOS, which
   takes AL. An instruction that loads SS keeps the machine from
   interrupting until the next one has run, so the two that change the
   stack are never apart. */
__declspec(naked) static void DeviceEntry(void)
{
    _asm {
        push ds
        push es
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        mov bx,word ptr cs:DataSel
        mov ds,bx
        mov es,bx
        cld
        mov ecx,esp
        mov edx,ss
        mov ss,bx
        mov esp,Below
        push edx
        push ecx
        mov edx,ebp
        call Arrived
        pop ecx
        pop edx
        mov ss,dx
        mov esp,ecx
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop es
        pop ds
        iretd
    }
}

/* INT 1Bh, Ctrl-Break of the BIOS, and INT 23h, Ctrl-C of DOS, while the
   program has them: nothing. */
__declspec(naked) static void NothingEntry(void)
{
    _asm {
        iretd
    }
}

void DeviceErrorNote(void)
{
    Note(INT_ERROR, &ErrorWas);
}

void DeviceErrorTake(DeviceErrorFn *ask)
{
    DeviceAsk = ask;
    Go(INT_ERROR, CodeSel, (u32)DeviceEntry);
    Taken |= TAKEN_ERROR;
}

void DeviceErrorGive(void)
{
    Give(INT_ERROR, &ErrorWas, TAKEN_ERROR);
}

void CtrlBreakNote(void)
{
    Note(INT_CTRL_BREAK, &BreakWas);
}

void CtrlBreakTake(void)
{
    Go(INT_CTRL_BREAK, CodeSel, (u32)NothingEntry);
    Taken |= TAKEN_BREAK;
}

void CtrlBreakGive(void)
{
    Give(INT_CTRL_BREAK, &BreakWas, TAKEN_BREAK);
}

/* DOS gives Ctrl-C back by itself when a program of 16 bits ends, from
   the PSP. What the program has taken through the extender only this
   can give back, so who had it is noted here, the first time that it is
   taken. */
void CtrlCTake(void)
{
    if (!(Taken & TAKEN_CTRL_C))
        Note(INT_CTRL_C, &CtrlCWas);
    Go(INT_CTRL_C, CodeSel, (u32)NothingEntry);
    Taken |= TAKEN_CTRL_C;
}

/* What the program has taken of the three and not given back, given
   back: for where it ends. */
static void GiveAll(void)
{
    if (Taken & TAKEN_ERROR)
        Give(INT_ERROR, &ErrorWas, TAKEN_ERROR);
    if (Taken & TAKEN_BREAK)
        Give(INT_CTRL_BREAK, &BreakWas, TAKEN_BREAK);
    if (Taken & TAKEN_CTRL_C)
        Give(INT_CTRL_C, &CtrlCWas, TAKEN_CTRL_C);
}

/* What the program was started with is in its PSP, which is DOS's and
   below the megabyte: the tail from 81h on, and at 2Ch what names the
   environment. Start has found both. The environment is DOS's own
   block, so what lies behind its end, the path of the program's file,
   is there to be read as in 16 bits. */

#define PSP_ENVIRON 0x2C
#define PSP_TAIL    0x81

static const u8 *Psp = 0;
static const u8 *Environment = 0;

const u8 FAR *ProgramTail(void)
{
    return Psp + PSP_TAIL;
}

const u8 FAR *ProgramEnvironment(void)
{
    return Environment;
}

/* Other programs: AX 4B00h in real mode, since what DOS is told of the
   program is addresses of real mode, with the name at DS:DX and at ES:BX
   the block that says what the program is run with; the shell's own
   interrupt, INT 2Eh; and AH 4Ch.

   All that DOS reads is in the block below the megabyte. The name. The
   tail, of which DOS copies 128 bytes into the PSP of the program
   whatever its count, so 128 go down. The two file control blocks,
   which DOS copies there too, 16 bytes each: the bytes of the main data
   at 5Ch and at 6Ch, as in dos.c, where Run says why. And the 14 bytes
   that name them, with the environment.

   dos.c names no environment, and DOS then gives the program a copy of
   this one's, which it finds through this program's PSP. Here the PSP
   has a selector of the extender's where DOS looks for the segment
   (see Start), so the environment is named, by its segment: the program
   gets a copy of it all the same. */

#define INT_GOES_ON 0x22                /* where DOS goes on when a
                                           program has ended */
#define TAIL_SIZE 128

/* Where INT 22h goes in real mode, and that set. DOS has it name where
   it was called from when it runs a program, and leaves it so; in 16
   bits it puts it back from the PSP when this program ends. The
   extender ends first, and says of every interrupt of real mode that is
   not as it found it that it "has been modified", onto the last screen:
   so it is put back behind every program that is run. Only the extender
   is asked. */
static u32 GoesOn(void)
{
    struct DosRegs regs;

    Extender(&regs, DPMI_REAL_VECTOR, INT_GOES_ON, 0, 0, 0);
    return (u32)(u16)regs.ecx << 16 | (u16)regs.edx;
}

static void SetGoesOn(u32 to)
{
    struct DosRegs regs;

    Extender(&regs, DPMI_SET_REAL_VECTOR, INT_GOES_ON, to >> 16, (u16)to, 0);
}

static void Run(struct DosRegs *regs, const u8 *name, const u8 *tail)
{
    u16 *block = (u16 *)LOW(LOW_RUN);

    NameLow(name);
    Bytes(LOW(LOW_SECOND), tail, TAIL_SIZE);
    Bytes(LOW(LOW_FCB1), (const u8 *)&md + 0x5C, FCB_UNOPENED);
    Bytes(LOW(LOW_FCB2), (const u8 *)&md + 0x6C, FCB_UNOPENED);
    block[0] = (u16)((u32)Environment >> 4);
    block[1] = LOW_SECOND;
    block[2] = LowSeg;
    block[3] = LOW_FCB1;
    block[4] = LowSeg;
    block[5] = LOW_FCB2;
    block[6] = LowSeg;
    SetLow(regs, 0x4B00, LOW_RUN, 0, LOW_NAME);
}

struct Answer ProgramRun(const u8 FAR *name, const u8 FAR *tail)
{
    struct DosRegs regs;
    struct Answer ran;
    u32 goes = GoesOn();

    Run(&regs, name, tail);
    ran = Ask(&regs, 0);
    SetGoesOn(goes);
    return ran;
}

struct Answer ProgramRunBare(const u8 FAR *name, const u8 FAR *tail)
{
    struct DosRegs regs;
    struct Answer ran;
    u32 goes = GoesOn();

    Run(&regs, name, tail);
    ran = Tell(&regs);
    SetGoesOn(goes);
    return ran;
}

/* The shell's own interrupt for a command, INT 2Eh of COMMAND.COM, with
   the command at DS:SI. It is of real mode, and comes back there with
   nothing as it was but where it comes back to: no stack and no
   register. So it cannot be called as an interrupt of real mode is, by
   the extender's service, which goes on with the stack: it is called
   from a stretch of instructions of real mode in the block below the
   megabyte, which keep where the stack is in four bytes there, behind
   their own segment, and put it back behind the interrupt, with the
   machine kept from interrupting meanwhile, and return far; and that
   stretch is called by the extender's service for a routine. Int2E of
   dos.c is the same in the program's own code. The command goes into
   the block too: its count, its characters and the carriage return.

   INT 22h is put back behind it as behind a program that DOS runs,
   since the shell runs programs. */
#define KEPT_LOW  (LOW_KEPT & 0xFF)
#define KEPT_HIGH (LOW_KEPT >> 8)

static const u8 Shell_T[] = {
    0x2E, 0x89, 0x26, KEPT_LOW, KEPT_HIGH,      /* mov cs:[LOW_KEPT],sp */
    0x2E, 0x8C, 0x16, KEPT_LOW + 2, KEPT_HIGH,  /* mov cs:[LOW_KEPT+2],ss */
    0xCD, 0x2E,                                 /* int 2Eh */
    0xFA,                                       /* cli */
    0x2E, 0x8E, 0x16, KEPT_LOW + 2, KEPT_HIGH,  /* mov ss,cs:[LOW_KEPT+2] */
    0x2E, 0x8B, 0x26, KEPT_LOW, KEPT_HIGH,      /* mov sp,cs:[LOW_KEPT] */
    0xFB,                                       /* sti */
    0xFC,                                       /* cld */
    0xCB                                        /* retf */
};

void ShellRun(const u8 FAR *command)
{
    struct DosRegs regs;
    u32 goes = GoesOn();
    u16 count = command[0] + 2;

    if (count > TAIL_SIZE)
        count = TAIL_SIZE;
    Bytes(LOW(LOW_SECOND), command, count);
    Bytes(LOW(LOW_SHELL), Shell_T, sizeof Shell_T);
    SetLow(&regs, 0, 0, 0, 0);
    regs.esi = LOW_SECOND;
    Real(&regs, 0, (u32)LowSeg << 16 | LOW_SHELL);
    SetGoesOn(goes);
}

/* The end of the program. The extender takes the call for its own end
   too, and leaves the machine as it found it.

   The program gives Ctrl-Break back before it ends, and not the
   critical error or Ctrl-C: DOS puts those two back from the PSP when a
   program of 16 bits ends. Here they are taken through the extender,
   which ends before DOS sees anything and writes a warning for every
   interrupt of real mode that is not as it found it: so what is still
   taken is given back first, of all three. It is asked of the extender
   alone. */
void ProgramEnd(u16 code)
{
    struct DosRegs regs;

    GiveAll();
    Set(&regs, 0x4C00 | code & 0xFF, 0, 0, 0);
    Tell(&regs);
}

/* The extender does not start on a DOS before 3, so there is none to
   leave another way: nothing, and the program goes on to say what it
   says of a DOS that is too old for it. */
void ProgramEndOld(void)
{
}

/* The console. */

void ConsoleChar(u16 character)
{
    struct DosRegs regs;

    Set(&regs, 0x0600, 0, 0, character & 0xFF);
    Tell(&regs);
}

void ConsoleText(const u8 FAR *text)
{
    struct DosRegs regs;

    Set(&regs, 0x0900, 0, 0, ADDRESS(text));
    Tell(&regs);
}

/* The keyboard. The byte of the BIOS's data that has the keys which are
   held down is at 418h. */

#define FLAG_ZERO 0x40

struct Keys KeyWaiting(void)
{
    u32 pair = Keys(0x100);
    struct Keys keys;

    keys.key = (u16)pair;
    keys.none = ((u16)(pair >> 16) & FLAG_ZERO) != 0;
    return keys;
}

u16 KeyRead(void)
{
    return (u16)Keys(0);
}

u16 KeysHeld(void)
{
    return (u16)Keys(0x200) & 0xFF;
}

void KeyForgetIns(void)
{
    *(u8 *)0x418 &= 0x7F;
}

/* The screen. The word of the BIOS's data that says where the page
   begins is at 44Eh, and the memory of the display at B0000h for the
   mode of one colour and at B8000h for the others. Where the cells are
   is kept where the original keeps it and as dos.c keeps it, in Screen
   of the main data: a segment of real mode above an offset. */

struct Screen ScreenState(void)
{
    struct BiosRegs regs;
    struct Screen screen;

    Display(0x0F00, 0, 0, 0, &regs);
    screen.mode = regs.ax & 0xFF;
    screen.columns = regs.ax >> 8;
    screen.page = regs.bx >> 8;
    return screen;
}

/* One of the calls that answer with nothing. */
static void Show(u16 ax, u16 bx, u16 cx, u16 dx)
{
    struct BiosRegs regs;

    Display(ax, bx, cx, dx, &regs);
}

void ScreenSetMode(u16 mode)
{
    Show(mode & 0xFF, 0, 0, 0);
}

struct CursorAt CursorWhere(u16 page)
{
    struct BiosRegs regs;
    struct CursorAt cursor;

    Display(0x0300, page << 8, 0, 0, &regs);
    cursor.place = regs.dx;
    cursor.shape = regs.cx;
    return cursor;
}

void CursorTo(u16 page, u16 place)
{
    Show(0x0200, page << 8, 0, place);
}

void CursorShape(u16 shape)
{
    Show(0x0100, 0, shape, 0);
}

void ScreenScroll(u16 down, u16 lines, u16 color, u16 upper, u16 lower)
{
    Show((down != 0 ? 0x0700 : 0x0600) | lines & 0xFF, color << 8, upper,
         lower);
}

u16 ScreenCell(u16 page)
{
    struct BiosRegs regs;

    Display(0x0800, page << 8, 0, 0, &regs);
    return regs.ax;
}

void ScreenFill(u16 page, u16 character, u16 color, u16 count)
{
    Show(0x0900 | character & 0xFF, page << 8 | color & 0xFF, count, 0);
}

void ScreenType(u16 character, u16 page, u16 color)
{
    Show(0x0E00 | character & 0xFF, page << 8 | color & 0xFF, 0, 0);
}

u16 ScreenEga(void)
{
    struct BiosRegs regs;

    Display(0x1200, 0xFF10, 0, 0, &regs);
    return regs.bx != 0xFF10;
}

/* The BIOS answers with an address in ES:BP besides, which the program
   does not read and the extender does not bring back. */
struct Font ScreenFont(void)
{
    struct BiosRegs regs;
    struct Font font;

    Display(0x1130, 0, 16, 0, &regs);
    font.height = regs.cx;
    font.last = regs.dx & 0xFF;
    return font;
}

void ScreenSmallFont(void)
{
    Show(0x1112, 0, 0, 0);
}

void ScreenPrintAll(void)
{
    Show(0x1200, 0x20, 0, 0);
}

u16 ScreenPageStart(void)
{
    return *(const u16 *)0x44E;
}

void ScreenMemory(u16 mode, u16 start)
{
    md.Screen = (u32)(mode == 7 ? 0xB000 : 0xB800) << 16 | start;
}

/* Where the cells are: the address that Screen of the main data is in
   real mode. */
#define CELLS ((u16 *)((md.Screen >> 16 << 4) + (u16)md.Screen))

void ScreenPut(u16 cell, const u16 FAR *from, u16 count)
{
    u16 *to = CELLS + cell;

    for (; count != 0; count--)
        *to++ = *from++;
}

void ScreenGet(u16 cell, u16 FAR *to, u16 count)
{
    const u16 *from = CELLS + cell;

    for (; count != 0; count--)
        *to++ = *from++;
}

/* The mouse. */

u16 MouseFind(void)
{
    return Mouse(0x21, 0, 0, 0);
}

u16 MouseReset(void)
{
    return Mouse(0, 0, 0, 0);
}

void MouseShow(void)
{
    Mouse(1, 0, 0, 0);
}

void MouseHide(void)
{
    Mouse(2, 0, 0, 0);
}

struct MouseAt MouseWhere(void)
{
    struct MouseAt at;

    MouseState(3, &at);
    return at;
}

void MouseTo(u16 x, u16 y)
{
    Mouse(4, 0, x, y);
}

void MouseRows(u16 first, u16 last)
{
    Mouse(8, 0, first, last);
}

void MousePointer(u16 keep, u16 turn)
{
    Mouse(0x0A, 0, keep, turn);
}

/* The timer: the BIOS's count of ticks since midnight, in its data at
   46Ch. A number of 32 bits is read at once here, and no tick can come
   between its halves; the machine is kept from interrupting all the
   same, and let again, as dos.c has it, since that leaves it able to
   interrupt wherever the program asks for the ticks. */
u32 Ticks(void)
{
    u32 ticks;

    _disable();
    ticks = *(const volatile u32 *)0x46C;
    _enable();
    return ticks;
}

/* The interrupt flag set, by the compiler's own function for that,
   as in dos.c. It is made though the routine that the critical error
   is to arrive at is not: the program calls it behind everything that
   it puts on the screen, and the extender lets a program of protected
   mode set the flag. */
void IntOn(void)
{
    _enable();
}

/* The program ended by another's hand. DOS then goes on at the address
   that the program's PSP has at 0Ah, in the program that started this
   one, and while the program has taken its end, the PSP names a routine
   there instead, as in dos.c, where that is Exit01: read it there.

   Here the routine cannot be the program's code. The program that DOS
   ends is the extender's: the extender has INT 21h of real mode while
   the program runs, sees the call that ends it, and takes itself out
   before DOS does anything, the interrupts with it that the program has
   taken through it, and the memory above the megabyte that the program
   is in (tried with the toy harness/trial32/ended.asm, which has DOS
   end the program at an idle call as the scenario `ended` does). Where
   DOS goes on, the machine is in real mode and nothing of the program
   can run.

   So the routine is eight instructions of real mode in the block below
   the megabyte, written here as their bytes: the compiler writes for a
   segment of 32 bits, and what it writes means something else in one
   of 16. DOS has freed the block by then, with the rest of the
   program's memory, and its bytes are still there, as those of the
   program are where Exit01 runs. It does what Exit01 and Exit05 of
   dos.c do, on the stack of the program that started this one: DOS is
   told to look for Ctrl-C as the program has said, AX 3301h, with AX
   kept around it; ES is put on the segment of this program's PSP; the
   flags are made those of the segment that the program goes on in
   compared with 0, by the last instruction that sets any; and it goes
   on where the PSP said before. DL is what DOS was told of Ctrl-C, and
   every other register is as it came. What it reads is behind it in
   the block, through CS: where the PSP had DOS go on, the PSP's
   segment, and what the program has said of Ctrl-C.

   What is not as in 16 bits afterwards is the extender's doing and
   cannot be: Ctrl-Break, INT 1Bh, goes where it went before the program
   was started, where the program of 16 bits leaves it going into
   memory that is free. And INT 22h, which DOS sets from the PSP, names
   the block where it names the program in 16 bits.

   A second copy of the program has a PSP and a block of its own, and
   neither copy reads or writes the other's. */
#define PSP_GOES_ON 0x0A                /* where DOS goes on when the
                                           program has ended */

/* A place of the block as the two bytes of an instruction's address. */
#define LO(place) ((place) & 0xFF)
#define HI(place) ((place) >> 8)

static const u8 Ended_T[] = {
    0x50,                                       /* push ax */
    0xB8, 0x01, 0x33,                           /* mov ax,3301h */
    0x2E, 0x8A, 0x16, LO(LOW_BREAK), HI(LOW_BREAK),
                                                /* mov dl,cs:[LOW_BREAK] */
    0xCD, 0x21,                                 /* int 21h */
    0x58,                                       /* pop ax */
    0x2E, 0x8E, 0x06, LO(LOW_PSP), HI(LOW_PSP), /* mov es,cs:[LOW_PSP] */
    0x2E, 0x83, 0x3E, LO(LOW_WAS + 2), HI(LOW_WAS + 2), 0,
                                                /* cmp word ptr
                                                   cs:[LOW_WAS+2],0 */
    0x2E, 0xFF, 0x2E, LO(LOW_WAS), HI(LOW_WAS)  /* jmp dword ptr
                                                   cs:[LOW_WAS] */
};

/* Where the PSP has DOS go on. */
#define GOES_ON (*(u32 *)(Psp + PSP_GOES_ON))

/* Where the PSP had DOS go on before the program took its end, and 0
   while the program has not taken it; and what the program has said
   that DOS is to be told of Ctrl-C, which the routine has in the block
   from the moment that it is said. */
static u32 EndedWas = 0;
static u8 EndedBreak = 0;

void ProgramEndedBreakCheck(u16 on)
{
    EndedBreak = (u8)on;
    *LOW(LOW_BREAK) = EndedBreak;
}

void ProgramEndedTake(void)
{
    if (EndedWas == 0) {
        EndedWas = GOES_ON;
        Bytes(LOW(LOW_ENDED), Ended_T, sizeof Ended_T);
        *(u32 *)LOW(LOW_WAS) = EndedWas;
        *(u16 *)LOW(LOW_PSP) = (u16)((u32)Psp >> 4);
        *LOW(LOW_BREAK) = EndedBreak;
        GOES_ON = (u32)LowSeg << 16 | LOW_ENDED;
    }
}

void ProgramEndedGive(void)
{
    if (EndedWas != 0) {
        GOES_ON = EndedWas;
        EndedWas = 0;
    }
}

/*
 * The start of the program.
 */

/* The selectors of the code and, below that, of the data. */
static u32 Selectors(void);
#pragma aux Selectors = \
    "mov ax,cs" \
    "shl eax,16" \
    "mov ax,ds" \
    value [eax] modify exact [eax]

/* ES put on the data, where compiled code takes it to be, and the
   direction flag cleared, which it takes to be clear: with what ES was
   until then, the selector of the PSP. */
static u32 Entered(void);
#pragma aux Entered = \
    "xor eax,eax" \
    "mov ax,es" \
    "push ds" \
    "pop es" \
    "cld" \
    value [eax] modify exact [eax]

/* Where the segment of a selector begins in the memory, or `none` if
   the extender does not know the selector. */
static u32 Base(u16 selector, u32 none)
{
    struct DosRegs regs;

    if (Extender(&regs, DPMI_SEGMENT_BASE, selector, 0, 0, 0))
        return none;
    return (u32)(u16)regs.ecx << 16 | (u16)regs.edx;
}

/* What the program says where it cannot start for want of memory below
   the megabyte: what the original says where it has too little memory
   to run, Aloc_S of resident.c, which is the nearest case that it
   knows. It is a copy of this file's own, since this file reads nothing
   of the program's, ended as DOS takes a text. */
static const u8 NoStart_S[] = "Program too big to fit in memory\r\n$";

/* The program begins here: the extender has loaded the file, set the
   words of it that hold an address, and enters this on the stack that
   the header of the file names, with interrupts on, CS, DS and SS on
   all of the memory, and ES on the PSP. The linker is told the name
   (`option start=Start_`). Settling ES and the direction flag is all
   that a C library's start-up would have had to do, and the program has
   none.

   What the functions of this file need before the program asks the
   first of them is made ready here, and only the extender is asked for
   it, by services that DOS does not see but for the block:

     - Where the PSP is, which is DOS's, below the megabyte: where the
       segment of ES begins. The program of 16 bits asks DOS for that,
       as the first thing that it asks.
     - Where the environment is. The extender has put a selector for it
       into the PSP at 2Ch, where DOS had its segment; should it not
       know what is there for a selector, that is taken for the segment.
     - The block below the megabyte for the calls that are made in real
       mode: DOS has it for the program, and takes it back when the
       program ends. A command that the program runs finds that much
       less of DOS's memory.
     - The program's own memory, for MemTake.

   A program that does not get the block says that it is too big to
   fit in the memory, in the original's words, and ends at once, with
   DOS's error for want of memory as its number. Main of main.c is the
   program, and does not return. */
void Start(void)
{
    struct DosRegs regs;
    u16 selector = (u16)Entered();
    u32 selectors = Selectors();
    u16 environment;

    DataSel = (u16)selectors;
    CodeSel = (u16)(selectors >> 16);
    Psp = (const u8 *)Base(selector, 0);
    environment = *(const u16 *)(Psp + PSP_ENVIRON);
    Environment = (const u8 *)Base(environment, (u32)environment << 4);
    if (Extender(&regs, DPMI_LOW_BLOCK, (LOW_SIZE + 15) >> 4, 0, 0, 0)) {
        ConsoleText(NoStart_S);
        ProgramEnd(ERR_NO_MEMORY);
    }
    LowSeg = (u16)regs.eax;
    OwnSet();
    Main();
}
