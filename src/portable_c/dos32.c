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
 *     there. Those have to be made in real mode with a block below the
 *     megabyte, and are not here yet: see the end of this file.
 *   - Four calls that it translates it brings back with the AX that it
 *     was called with where they did not fail, and not with what DOS
 *     left there, which is what sys.h has an answer be: the search of a
 *     directory, AH 4Eh and 4Fh, the current directory, 47h, and the
 *     attributes set, AX 4301h. The toy of harness/trial32/sys.c showed
 *     it. So these are made in real mode here, by the extender's
 *     service for that, with the name and the record in a block below
 *     the megabyte that the program asks for when it starts, and what
 *     DOS wrote there copied to where the program wants it.
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
 * The file has three parts: the functions of sys.h that are made here,
 * 72 of the 93, six of them in real mode; the 21 that are not made
 * yet, which do nothing and say so; and the start of the program.
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

/* INT 31h, the extender's own services, which are DPMI's: AX as it
   leaves it, and above that FFFFh if it leaves the carry flag set, which
   is how it says that it failed. EDI is the address of what a service
   takes in ES:EDI. */
static u32 Extender(u32 eax, u32 ebx, u32 ecx, void *edi);
#pragma aux Extender = \
    "int 31h" \
    "sbb edx,edx" \
    "shl edx,16" \
    "mov dx,ax" \
    parm [eax] [ebx] [ecx] [edi] value [edx] modify exact [eax ebx edx]

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
   for it. It has room for a name or a path, and for the record of a
   search. */
static u16 LowSeg = 0;

#define LOW_NAME   0                    /* 128 bytes */
#define LOW_RECORD 128                  /* 64 bytes */
#define LOW_SIZE   192
#define LOW(place) ((u8 *)(((u32)LowSeg << 4) + (place)))

#define DPMI_LOW_BLOCK 0x0100           /* a block of DOS's memory: BX
                                           paragraphs; AX its segment */
#define DPMI_REAL_INT  0x0300           /* an interrupt in real mode: BL
                                           which, CX words of the stack to
                                           take along, ES:EDI the
                                           registers */
#define FLAG_CARRY 1

/* DOS, with the registers of `regs` and their answer there. Not 0 if
   DOS leaves the carry flag set: FFFFh, as in dos.c, which is what a
   function that asks bare answers with for `failed`. */
static u16 Call(struct DosRegs *regs)
{
    struct RealRegs real;

    if (!regs->low)
        return (u16)Dos(regs);
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
    Extender(DPMI_REAL_INT, 0x21, 0, &real);
    regs->eax = real.eax;
    regs->ebx = real.ebx;
    regs->ecx = real.ecx;
    regs->edx = real.edx;
    return real.flags & FLAG_CARRY ? 0xFFFF : 0;
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

/* The time, the system itself. */

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

/* The end of the program. The extender takes the call for its own end
   too, and leaves the machine as it found it. */
void ProgramEnd(u16 code)
{
    struct DosRegs regs;

    Set(&regs, 0x4C00 | code & 0xFF, 0, 0, 0);
    Tell(&regs);
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

/*
 * The 21 functions of sys.h that are not made here yet. Each does
 * nothing, and answers as a system does that failed or does not know
 * the question; none of them calls DOS, since a call that the extender
 * does not translate has DOS write wherever the registers happen to
 * point in real mode. What each needs is in TODO.md, "The portable C in
 * 32 bits: the plan", with the piece D2:
 *
 *   - through the block below the megabyte and a call in real mode, as
 *     the search of a directory is made above, with more room in the
 *     block: FileTrueName, FcbParse, FcbDelete, CountryInfo, CaseMap,
 *     ProgramRun and ProgramRunBare;
 *   - other in 32 bits: MemTake, MemGive and MemResize, which hand out
 *     memory of the program's own; MemSetSearch and MemSetUpper;
 *   - out of the PSP, which is below the megabyte, once the start of
 *     the program knows where that is: ProgramTail and
 *     ProgramEnvironment;
 *   - what DOS and the BIOS enter of the program, the critical error,
 *     Ctrl-Break and Ctrl-C, which are taken through the extender here
 *     and arrive at routines of protected mode: the seven that note,
 *     take and give them, at the end of this part.
 *
 * ProgramRun and ProgramRunBare take the name and the tail, and what
 * else DOS is told of a program that it runs is theirs to make, in the
 * block below the megabyte: see Run of dos.c for what that is, the two
 * file control blocks above all, which are bytes of the main data.
 */

#define ERR_NO_FUNCTION 1               /* DOS's error for a call that it
                                           does not know */
#define ERR_NO_MEMORY   8               /* and for want of memory */

static struct Answer NotMade(u16 error)
{
    struct Answer answer;

    answer.value = error;
    answer.failed = 1;
    return answer;
}

struct Answer FileTrueName(const u8 FAR *name, u8 FAR *to)
{
    (void)name;
    (void)to;
    return NotMade(ERR_NO_FUNCTION);
}

u16 FcbParse(const u8 FAR *name, u8 FAR *fcb, u16 how)
{
    (void)name;
    (void)fcb;
    (void)how;
    return 0xFF;
}

struct Answer FcbDelete(const u8 FAR *fcb, u16 flagged)
{
    (void)fcb;
    (void)flagged;
    return NotMade(ERR_NO_FUNCTION);
}

struct Block MemTake(u16 paragraphs)
{
    struct Block block;

    (void)paragraphs;
    block.value = ERR_NO_MEMORY;
    block.failed = 1;
    block.largest = 0;
    return block;
}

struct Answer MemGive(u16 block)
{
    (void)block;
    return NotMade(ERR_NO_FUNCTION);
}

struct Block MemResize(u16 block, u16 paragraphs)
{
    (void)block;
    return MemTake(paragraphs);
}

void MemSetSearch(u16 how)
{
    (void)how;
}

void MemSetUpper(u16 on)
{
    (void)on;
}

void CountryInfo(struct CNTRY FAR *to)
{
    (void)to;
}

u16 CaseMap(u16 letter)
{
    return letter;
}

struct Answer ProgramRun(const u8 FAR *name, const u8 FAR *tail)
{
    (void)name;
    (void)tail;
    return NotMade(ERR_NO_FUNCTION);
}

struct Answer ProgramRunBare(const u8 FAR *name, const u8 FAR *tail)
{
    return ProgramRun(name, tail);
}

/* A tail of no characters, and an environment of no texts with no
   path of the program's file behind it: as many zero bytes as the
   program reads there, the path being read as LenPath bytes. */
static const u8 NoTail[] = "\r";
static const u8 NoEnvironment[1 + 2 + LenPath] = { 0 };

const u8 FAR *ProgramTail(void)
{
    return NoTail;
}

const u8 FAR *ProgramEnvironment(void)
{
    return NoEnvironment;
}

/* The critical error, Ctrl-Break and Ctrl-C: none is noted, taken or
   given back, so each goes where the extender has it go, and the
   program is not asked about a device. dos.c has the three as
   interrupt functions that DOS's services for the vectors name, and
   the first calls the function that the program hands over, with what
   sys.h says: the same call is to be made here from a routine of
   protected mode that the extender's services name, with the header of
   the device read at its address of real mode, and on the program's own
   stack, since such a routine is entered on one of the extender's.
   What the function answers goes back to DOS in AL. */

void DeviceErrorNote(void)
{
}

void DeviceErrorTake(DeviceErrorFn *ask)
{
    (void)ask;
}

void DeviceErrorGive(void)
{
}

void CtrlBreakNote(void)
{
}

void CtrlBreakTake(void)
{
}

void CtrlBreakGive(void)
{
}

void CtrlCTake(void)
{
}

/*
 * The start of the program.
 */

/* ES put on the data, where compiled code takes it to be, and the
   direction flag cleared, which it takes to be clear. */
static void Settle(void);
#pragma aux Settle = \
    "push ds" \
    "pop es" \
    "cld" \
    modify exact []

/* The program begins here: the extender has loaded the file, set the
   words of it that hold an address, and enters this on the stack that
   the header of the file names, with interrupts on, CS, DS and SS on
   all of the memory, and ES on the PSP. The linker is told the name
   (`option start=Start_`). Settling the two things is all that a C
   library's start-up would have had to do, and the program has none.

   The block below the megabyte is asked for here, once, for the calls
   that are made in real mode: DOS has it for the program, and takes it
   back when the program ends. A program that does not get it ends at
   once, with DOS's error for want of memory as its number. Main of
   main.c is the program, and does not return. */
void Start(void)
{
    u32 block;

    Settle();
    block = Extender(DPMI_LOW_BLOCK, (LOW_SIZE + 15) >> 4, 0, 0);
    if (block >> 16)
        ProgramEnd(ERR_NO_MEMORY);
    LowSeg = (u16)block;
    Main();
}
