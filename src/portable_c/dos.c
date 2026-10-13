/*
 * The system that the program runs on, for DOS on a PC: the functions of
 * sys.h, by which the program asks for files, directories, drives,
 * memory, the time, the keyboard, the screen, the mouse and the timer,
 * as DOS, the BIOS and the mouse driver are asked for them. This is the
 * only file that knows a register, the number of a service or an
 * interrupt; the same functions for another system are another file.
 *
 * DOS, the BIOS and the mouse driver are called by software interrupts,
 * and an interrupt is an instruction: each is written once, as inline
 * assembly, in a #pragma aux of its own, with as little beside it as will
 * do. The registers go into the instruction and come out of it as
 * arguments and as what it returns; the compiler loads them. What other
 * inline assembly there is, is where DOS goes on when it has ended the
 * program and where COMMAND.COM's interrupt is called, in resident.c.
 * The memory of the machine that the program reads and writes is here
 * too, the BIOS's data and the cells of the display, through far
 * pointers in plain C; and the interrupt flag of the machine, for which
 * the compiler has intrinsic functions.
 *
 * The interrupts that the program takes arrive here as well, DOS's
 * critical error, Ctrl-Break and Ctrl-C, at interrupt functions of the
 * compiler. What the program does about the first is the program's: a
 * function of its own that it hands over, and that is called from here
 * as any function is.
 *
 * The file has two parts: the functions of sys.h, and behind them what
 * is of DOS alone and no part of sys.h, where the PSP is. A structure of
 * registers, as the C translation hands one to an interrupt from every
 * file, is known here and nowhere else.
 */
#include "vc.h"

/* The registers of a call of DOS. DS goes with DX: a pointer to what DOS
   takes there, a name or a buffer, sets both. ES is what DOS is called
   with, and what it leaves there, for the calls that answer with an
   address in ES:BX. */
struct DosRegs {
    u16 ax, bx, cx;
    const u8 __far *name;               /* DX, and DS behind it */
    u16 si, di;
    u16 es;
};

/* The registers of a call of the BIOS for the screen. */
struct BiosRegs {
    u16 ax, bx, cx, dx;
};

/* The compiler's own functions for the interrupt flag: an instruction
   each, put where they are called. */
void _disable(void);
void _enable(void);
#pragma intrinsic(_disable, _enable)

/* The interrupts. Each of these is put in place where it is called, as
   the instructions between the quotes. */

/* INT 21h with the registers of `regs`, which is on the stack, ES among
   them; the carry flag is set first if `carry` is not 0, since some
   calls of DOS leave it as they find it. AX, BX, CX, DX and ES go back
   into `regs` as DOS leaves them, and what is returned is not 0 if the
   carry flag is set then. PUSH, POP, MOV and LDS leave the flags
   alone. */
static u16 Dos(struct DosRegs STACK *regs, u16 carry);
#pragma aux Dos = \
    "neg ax" \
    "push bp" \
    "push ds" \
    "mov bp,bx" \
    "mov es,[bp+14]" \
    "mov ax,[bp]" \
    "mov bx,[bp+2]" \
    "mov cx,[bp+4]" \
    "mov si,[bp+10]" \
    "mov di,[bp+12]" \
    "lds dx,[bp+6]" \
    "int 21h" \
    "mov [bp],ax" \
    "mov [bp+2],bx" \
    "mov [bp+4],cx" \
    "mov [bp+6],dx" \
    "mov [bp+14],es" \
    "sbb ax,ax" \
    "pop ds" \
    "pop bp" \
    parm [bx] [ax] value [ax] modify exact [ax bx cx dx si di es]

/* INT 10h with AX, BX, CX and DX, which go back into the four words at
   `regs`, on the stack, as the BIOS leaves them: more than a function
   can return. Some of the BIOSes that the program was written for lose
   BP in it, and the program saved BP around the calls that did; one call
   answers in ES and BP besides, which the program does not read. */
static void Video(u16 ax, u16 bx, u16 cx, u16 dx,
                  struct BiosRegs STACK *regs);
#pragma aux Video = \
    "push bp" \
    "int 10h" \
    "pop bp" \
    "mov ss:[si],ax" \
    "mov ss:[si+2],bx" \
    "mov ss:[si+4],cx" \
    "mov ss:[si+6],dx" \
    parm [ax] [bx] [cx] [dx] [si] modify exact [ax bx cx dx es]

/* INT 16h, with AX as it leaves it and, above that, the flags. */
static u32 Keyboard(u16 ax);
#pragma aux Keyboard = \
    "int 16h" \
    "pushf" \
    "pop dx" \
    parm [ax] value [dx ax] modify exact [ax dx]

/* INT 2Fh, with AX and BX as it leaves them. */
static u32 Multiplex(u16 ax, u16 bx, u16 cx);
#pragma aux Multiplex = \
    "int 2Fh" \
    parm [ax] [bx] [cx] value [bx ax] modify exact [ax bx]

/* INT 28h, which leaves every register as it finds it. */
static void DosIdle(void);
#pragma aux DosIdle = \
    "int 28h" \
    modify exact []

/* INT 33h, with AX as it leaves it. */
static u16 MouseDriver(u16 ax, u16 bx, u16 cx, u16 dx);
#pragma aux MouseDriver = \
    "int 33h" \
    parm [ax] [bx] [cx] [dx] value [ax] modify exact [ax bx cx dx]

/* INT 33h, with BX, CX and DX as it leaves them, in the three words at
   `state`, which is on the stack: more than a function can return. */
static void MouseState(u16 ax, struct MouseAt STACK *state);
#pragma aux MouseState = \
    "int 33h" \
    "mov ss:[di],bx" \
    "mov ss:[di+2],cx" \
    "mov ss:[di+4],dx" \
    parm [ax] [di] modify exact [ax bx cx dx]

/* DOS's routine that makes a capital of a letter beyond ASCII, which it
   names in what it tells of the country: called far, the letter in AL
   and back there, and every other register left alone. */
typedef u16 __far CaseMapFn(u16 letter);
#pragma aux CaseMapFn parm [ax] value [ax] modify exact [ax]

/*
 * Four of the interrupts as functions, by which everything below calls
 * them: DOS, the BIOS for the screen and for the keyboard, and the mouse
 * driver are each put in place here and nowhere else.
 */

/* DOS, with the registers of `regs` and their answer there, ES among
   them, and the carry flag set first if `carry` is not 0. Not 0 if DOS
   leaves the flag set. */
static u16 Call(struct DosRegs STACK *regs, u16 carry)
{
    return Dos(regs, carry);
}

/* The BIOS for the screen, with the registers of `regs`, which come back
   there as it leaves them. */
static void Display(struct BiosRegs STACK *regs)
{
    Video(regs->ax, regs->bx, regs->cx, regs->dx, regs);
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
    return MouseDriver(ax, bx, cx, dx);
}

/*
 * The functions of sys.h. What each is for is said there; here is how
 * DOS, the BIOS and the mouse driver are asked for it.
 */

#define REGS(regs) ((struct DosRegs STACK *)&(regs))
#define BIOS(regs) ((struct BiosRegs STACK *)&(regs))

/* The two words of a far pointer, and a far pointer made of two. */
#define SEGMENT(pointer)   ((u16)((u32)(pointer) >> 16))
#define OFFSET_OF(pointer) ((u16)(u32)(pointer))
#define POINTER(segment, offset) \
    ((const u8 __far *)((u32)(segment) << 16 | (u16)(offset)))

/* DX of a call of DOS where it is no pointer, a drive or a date: DS is
   on the main data for the call, as the original has it. And DX as DOS
   left it. */
#define NUMBER(value) POINTER(MAIN_SEG, value)
#define DX_OF(regs)   OFFSET_OF((regs).name)

/* The segment of the PSP, the 256 bytes that DOS keeps of a program in
   front of it: the command tail is there, the segment of the
   environment, where the memory ends, and where DOS goes on when the
   program has ended. The original is a .COM file and has all its
   segment registers there. Alloc0 of resident.c sets this when the
   program begins, by ProgramPrefix, and reads the PSP itself for where
   DOS goes on; vc.h declares both for it. */
u16 PspSeg;

#define PSP_ENVIRON 0x2C                /* the segment of the environment */
#define PSP_TAIL    0x81                /* the command tail, behind its
                                           count */
#define PSP_WORD(offset) (*(const u16 __far *)POINTER(PspSeg, offset))

#define ERR_NOT_READY 0x53              /* the error of a call that the
                                           user gave up, when a device was
                                           not ready */

/* What DOS answers: AX, and whether the carry flag is set. */
static struct Answer Answered(struct DosRegs STACK *regs, u16 failed)
{
    struct Answer answer;

    answer.value = regs->ax;
    answer.failed = failed;
    return answer;
}

/* Ask DOS bare, with ES as `regs` has it, which only a call needs that
   takes something there; and with ES on the main data, as the original
   has it at every other call. */
static struct Answer TellEs(struct DosRegs STACK *regs)
{
    return Answered(regs, Call(regs, 0));
}

static struct Answer Tell(struct DosRegs STACK *regs)
{
    regs->es = MAIN_SEG;
    return TellEs(regs);
}

/* Ask DOS with the program's errors: Intr21 of the original, with ES as
   `regs` has it, and with the carry flag set first if `flagged` is not
   0. When DOS asks what to do about a device, the program's routine for
   that, FtErr1 of main.c, which FatErr below calls, shows a box and
   notes in ErrDat if the user gave up there; the call then counts as
   failed with the error 53h, and
   until the next call that comes here DOS is answered without a box.
   An error that DOS reports itself is asked about once more, and its
   extended code is what comes back: for that call AH is 59h and BX 0,
   and the other registers are as the first call left them, which they
   are afterwards too. */
static struct Answer AskEs(struct DosRegs STACK *regs, u16 flagged)
{
    u16 bx, cx, es;
    const u8 __far *name;

    md.ErrDat = 0;
    flagged = Call(regs, flagged);
    if (md.ErrDat != 0) {
        regs->ax = ERR_NOT_READY;
        return Answered(regs, 1);
    }
    if (flagged == 0)
        return Answered(regs, 0);
    bx = regs->bx;
    cx = regs->cx;
    name = regs->name;
    es = regs->es;
    regs->ax = regs->ax & 0xFF | 0x5900;
    regs->bx = 0;
    regs->es = MAIN_SEG;
    Call(regs, 0);
    regs->bx = bx;
    regs->cx = cx;
    regs->name = name;
    regs->es = es;
    return Answered(regs, 1);
}

static struct Answer Ask(struct DosRegs STACK *regs)
{
    regs->es = MAIN_SEG;
    return AskEs(regs, 0);
}

/* A call that takes a name at DS:DX and a number in CX, and one that
   takes a handle in BX, a buffer at DS:DX and a count in CX: with the
   program's errors. */
static struct Answer Named(u16 ax, const u8 __far *name, u16 cx)
{
    struct DosRegs regs;

    regs.ax = ax;
    regs.cx = cx;
    regs.name = name;
    return Ask(REGS(regs));
}

static struct Answer Handled(u16 ax, u16 handle, const u8 __far *buffer,
                             u16 count)
{
    struct DosRegs regs;

    regs.ax = ax;
    regs.bx = handle;
    regs.cx = count;
    regs.name = buffer;
    return Ask(REGS(regs));
}

/* Files: AH 3Ch to 43h, 56h, 57h and 60h, and 4400h. */

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

    regs.ax = 0x3D00 | mode & 0xFF;
    regs.name = name;
    return Tell(REGS(regs));
}

struct Answer FileClose(u16 handle)
{
    return Handled(0x3E00, handle, NUMBER(0), 0);
}

struct Answer FileCloseBare(u16 handle)
{
    struct DosRegs regs;

    regs.ax = 0x3E00;
    regs.bx = handle;
    regs.name = NUMBER(0);
    return Tell(REGS(regs));
}

struct Answer FileRead(u16 handle, u8 FAR *to, u16 count)
{
    return Handled(0x3F00, handle, to, count);
}

struct Answer FileReadBare(u16 handle, u8 FAR *to, u16 count)
{
    struct DosRegs regs;

    regs.ax = 0x3F00;
    regs.bx = handle;
    regs.cx = count;
    regs.name = to;
    return Tell(REGS(regs));
}

struct Answer FileWrite(u16 handle, const u8 FAR *from, u16 count)
{
    return Handled(0x4000, handle, from, count);
}

/* The offset goes in CX and DX, and the place comes back in DX and
   AX. */
struct FilePlace FileSeek(u16 handle, u16 from, u32 offset)
{
    struct DosRegs regs;
    struct FilePlace sought;

    regs.ax = 0x4200 | from & 0xFF;
    regs.bx = handle;
    regs.cx = (u16)(offset >> 16);
    regs.name = NUMBER((u16)offset);
    sought.failed = Ask(REGS(regs)).failed;
    sought.place = (u32)DX_OF(regs) << 16 | regs.ax;
    return sought;
}

struct Answer FileDelete(const u8 FAR *name)
{
    return Named(0x4100, name, 0);
}

/* The new name is at ES:DI. */
struct Answer FileRename(const u8 FAR *from, const u8 FAR *to)
{
    struct DosRegs regs;

    regs.ax = 0x5600;
    regs.name = from;
    regs.di = OFFSET_OF(to);
    regs.es = SEGMENT(to);
    return AskEs(REGS(regs), 0);
}

struct FileAttr FileAttributes(const u8 FAR *name)
{
    struct DosRegs regs;
    struct Answer asked;
    struct FileAttr attr;

    regs.ax = 0x4300;
    regs.cx = 0;
    regs.name = name;
    asked = Ask(REGS(regs));
    attr.value = asked.value;
    attr.failed = asked.failed;
    attr.attributes = regs.cx;
    return attr;
}

struct Answer FileSetAttributes(const u8 FAR *name, u16 attributes)
{
    return Named(0x4301, name, attributes);
}

/* The time is in CX and the date in DX. */
struct FileStamp FileTime(u16 handle)
{
    struct DosRegs regs;
    struct Answer asked;
    struct FileStamp stamp;

    regs.ax = 0x5700;
    regs.bx = handle;
    regs.cx = 0;
    regs.name = NUMBER(0);
    asked = Ask(REGS(regs));
    stamp.value = asked.value;
    stamp.failed = asked.failed;
    stamp.time = regs.cx;
    stamp.date = DX_OF(regs);
    return stamp;
}

struct Answer FileSetTime(u16 handle, u16 time, u16 date)
{
    return Handled(0x5701, handle, NUMBER(date), time);
}

/* What the handle is comes back in DX. */
struct Device FileDevice(u16 handle)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Device device;

    regs.ax = 0x4400;
    regs.bx = handle;
    regs.name = NUMBER(0);
    asked = Ask(REGS(regs));
    device.value = asked.value;
    device.failed = asked.failed;
    device.kind = DX_OF(regs);
    return device;
}

/* The name is at DS:SI and the path goes to ES:DI. */
struct Answer FileTrueName(const u8 FAR *name, u8 FAR *to)
{
    struct DosRegs regs;

    regs.ax = 0x6000;
    regs.name = POINTER(SEGMENT(name), 0);
    regs.si = OFFSET_OF(name);
    regs.di = OFFSET_OF(to);
    regs.es = SEGMENT(to);
    return AskEs(REGS(regs), 0);
}

/* Files by a file control block: AH 29h, with the name at DS:SI, the
   block at ES:DI and `how` in AL, and AH 13h, with the block at
   DS:DX. */

u16 FcbParse(const u8 FAR *name, u8 FAR *fcb, u16 how)
{
    struct DosRegs regs;

    regs.ax = 0x2900 | how & 0xFF;
    regs.name = POINTER(SEGMENT(name), 0);
    regs.si = OFFSET_OF(name);
    regs.di = OFFSET_OF(fcb);
    regs.es = SEGMENT(fcb);
    return TellEs(REGS(regs)).value;
}

struct Answer FcbDelete(const u8 FAR *fcb, u16 flagged)
{
    struct DosRegs regs;

    regs.ax = 0x1300;
    regs.name = fcb;
    regs.es = MAIN_SEG;
    return AskEs(REGS(regs), flagged);
}

/* Directories: AH 1Ah, 4Eh and 4Fh for what is in one, 39h to 3Bh, and
   47h, with the drive in DL and the room for the path at DS:SI. */

void FindRecord(struct DTAs FAR *record)
{
    struct DosRegs regs;

    regs.ax = 0x1A00;
    regs.name = (const u8 __far *)record;
    Tell(REGS(regs));
}

struct Answer FindFirst(const u8 FAR *pattern, u16 attributes)
{
    return Named(0x4E00, pattern, attributes);
}

struct Answer FindNext(void)
{
    return Named(0x4F00, NUMBER(0), 0);
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

struct Answer DirCurrent(u16 drive, u8 FAR *to)
{
    struct DosRegs regs;

    regs.ax = 0x4700;
    regs.name = POINTER(SEGMENT(to), drive & 0xFF);
    regs.si = OFFSET_OF(to);
    return Ask(REGS(regs));
}

struct Answer DirCurrentBare(u16 drive, u8 FAR *to)
{
    struct DosRegs regs;

    regs.ax = 0x4700;
    regs.name = POINTER(SEGMENT(to), drive & 0xFF);
    regs.si = OFFSET_OF(to);
    return Tell(REGS(regs));
}

/* Drives: AH 19h and 0Eh, 36h, and of 44h the calls 0Eh, 0Fh, 9 and 8,
   which take the drive in BL. */

u16 DriveCurrent(void)
{
    struct DosRegs regs;

    regs.ax = 0x1900;
    regs.name = NUMBER(0);
    return Tell(REGS(regs)).value & 0xFF;
}

void DriveSelect(u16 drive)
{
    struct DosRegs regs;

    regs.ax = 0x0E00;
    regs.name = NUMBER(drive & 0xFF);
    Tell(REGS(regs));
}

/* The drive is in DL; the clusters that are free come back in BX, the
   bytes of a sector in CX and the clusters of the drive in DX. */
struct Space DriveSpace(u16 drive, u16 flagged)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Space space;

    regs.ax = 0x3600;
    regs.bx = 0;
    regs.cx = 0;
    regs.name = NUMBER(drive & 0xFF);
    regs.es = MAIN_SEG;
    asked = AskEs(REGS(regs), flagged);
    space.value = asked.value;
    space.failed = asked.failed;
    space.free = regs.bx;
    space.sector = regs.cx;
    space.total = DX_OF(regs);
    return space;
}

/* One of the calls of AH 44h that take a drive, 1 for A:, in BL. */
static struct Answer Unit(u16 ax, u16 drive, struct DosRegs STACK *regs)
{
    regs->ax = ax;
    regs->bx = drive & 0xFF;
    regs->name = NUMBER(0);
    return Tell(regs);
}

struct Answer DriveLetter(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x440E, drive, REGS(regs));
}

struct Answer DriveSetLetter(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x440F, drive, REGS(regs));
}

struct Device DriveRemote(u16 drive)
{
    struct DosRegs regs;
    struct Answer asked;
    struct Device device;

    asked = Unit(0x4409, drive, REGS(regs));
    device.value = asked.value;
    device.failed = asked.failed;
    device.kind = DX_OF(regs);
    return device;
}

struct Answer DriveFixed(u16 drive)
{
    struct DosRegs regs;

    return Unit(0x4408, drive, REGS(regs));
}

/* The CD-ROM extensions are asked by the multiplex interrupt whether
   the drive is theirs (AX 150Bh), with the drive in CX: they answer
   with ADADh in BX, and with something in AX if it is. */
u16 DriveCd(u16 drive)
{
    u32 pair = Multiplex(0x150B, 0, drive & 0xFF);

    return (u16)(pair >> 16) == 0xADAD && (u16)pair != 0;
}

/* Memory: AH 48h, with the paragraphs in BX, and 49h and 4Ah, with the
   block in ES. DOS says in BX how much there is when it has not
   enough. */

static struct Block Paragraphs(struct DosRegs STACK *regs)
{
    struct Answer asked;
    struct Block block;

    regs->name = NUMBER(0);
    asked = TellEs(regs);
    block.value = asked.value;
    block.failed = asked.failed;
    block.largest = regs->bx;
    return block;
}

struct Block MemTake(u16 paragraphs)
{
    struct DosRegs regs;

    regs.ax = 0x4800;
    regs.bx = paragraphs;
    regs.es = MAIN_SEG;
    return Paragraphs(REGS(regs));
}

struct Answer MemGive(u16 block)
{
    struct DosRegs regs;

    regs.ax = 0x4900;
    regs.name = NUMBER(0);
    regs.es = block;
    return TellEs(REGS(regs));
}

struct Block MemResize(u16 block, u16 paragraphs)
{
    struct DosRegs regs;

    regs.ax = 0x4A00;
    regs.bx = paragraphs;
    regs.es = block;
    return Paragraphs(REGS(regs));
}

/* How DOS looks for memory, AX 5801h, and whether the upper memory is in
   its chain of blocks, AX 5803h, which DOS has since its version 5: both
   take what is set in BX. */
void MemSetSearch(u16 how)
{
    struct DosRegs regs;

    regs.ax = 0x5801;
    regs.bx = how;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
}

void MemSetUpper(u16 on)
{
    struct DosRegs regs;

    regs.ax = 0x5803;
    regs.bx = on;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
}

/* The time, AH 2Ch: the hour and the minute come back in CX, the second
   and its hundredth in DX. */
struct Time TimeOfDay(void)
{
    struct DosRegs regs;
    struct Time time;

    regs.ax = 0x2C00;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    time.clock = regs.cx;
    time.seconds = DX_OF(regs);
    return time;
}

/* DOS's routine that makes a capital of a letter beyond ASCII, which it
   names in what it tells of the country. */
static u32 Capitals;

/* The country, AH 38h: the current one, to DS:DX. */
void CountryInfo(struct CNTRY FAR *to)
{
    struct DosRegs regs;

    regs.ax = 0x3800;
    regs.name = (const u8 __far *)to;
    Tell(REGS(regs));
    Capitals = to->CaseMap;
}

u16 CaseMap(u16 letter)
{
    return ((CaseMapFn *)Capitals)(letter);
}

/* The version, AH 30h, and AX 3306h, which DOS has since its version 5
   and which leaves BX as it is otherwise. */
struct SysVersion SystemVersion(void)
{
    struct DosRegs regs;
    struct SysVersion version;

    regs.ax = 0x3000;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    version.version = regs.ax;
    version.other = regs.bx;
    return version;
}

struct SysVersion SystemTrueVersion(void)
{
    struct DosRegs regs;
    struct SysVersion version;

    regs.ax = 0x3306;
    regs.bx = 0;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    version.version = regs.bx;
    version.other = regs.ax;
    return version;
}

/* Ctrl-C, AX 3300h and 3301h: DL is 1 if DOS looks for it at every
   call. */
u16 BreakCheck(void)
{
    struct DosRegs regs;

    regs.ax = 0x3300;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    return DX_OF(regs) & 0xFF;
}

void SetBreakCheck(u16 on)
{
    struct DosRegs regs;

    regs.ax = 0x3301;
    regs.name = NUMBER(on & 0xFF);
    Tell(REGS(regs));
}

/* INT 28h: the interrupt that DOS itself calls while it waits for a key,
   and that programs which stay in the memory take to do their work
   in. */
void Idle(void)
{
    DosIdle();
}

/* Other programs: AX 4B00h, with the name at DS:DX and at ES:BX the
   block that says what the program is run with; and AH 4Ch.

   The block is where the original has it and as the original has it,
   EPB of the main data: no environment named, so that the program gets a
   copy of this one's; the tail; and for the two file control blocks that
   DOS copies into the new program's PSP the bytes at 5Ch and 6Ch of the
   main data, where a program hands on those of its own PSP: the
   original has ES on the main data when it fills the block in, once,
   when it starts. Here the block is filled in at every run. Nothing
   else reads it. */
static void Run(struct DosRegs STACK *regs, const u8 FAR *name,
                const u8 FAR *tail)
{
    md.EPB.EnvSeg = 0;
    md.EPB.CmdLine = (u32)tail;
    md.EPB.FCB1 = (u32)MAIN_SEG << 16 | 0x5C;
    md.EPB.FCB2 = (u32)MAIN_SEG << 16 | 0x6C;
    regs->ax = 0x4B00;
    regs->bx = (u16)&md.EPB;
    regs->name = name;
    regs->es = MAIN_SEG;
}

struct Answer ProgramRun(const u8 FAR *name, const u8 FAR *tail)
{
    struct DosRegs regs;

    Run(REGS(regs), name, tail);
    return AskEs(REGS(regs), 0);
}

struct Answer ProgramRunBare(const u8 FAR *name, const u8 FAR *tail)
{
    struct DosRegs regs;

    Run(REGS(regs), name, tail);
    return TellEs(REGS(regs));
}

void ProgramEnd(u16 code)
{
    struct DosRegs regs;

    regs.ax = 0x4C00 | code & 0xFF;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
}

/* What the program was started with is in its PSP: the tail from 81h
   on, behind the byte that counts it, and the segment of the
   environment. DOS is not asked. */

const u8 FAR *ProgramTail(void)
{
    return POINTER(PspSeg, PSP_TAIL);
}

const u8 FAR *ProgramEnvironment(void)
{
    return POINTER(PSP_WORD(PSP_ENVIRON), 0);
}

/* What DOS and the BIOS enter of the program: three interrupts that it
   takes. DOS says where an interrupt goes (AH 35h, which answers in
   ES:BX) and has one go somewhere (AH 25h, with the address at DS:DX),
   and is asked both bare. Who had the critical error and Ctrl-Break is
   kept where the original keeps it, in ErrVct and BrkVct of the main
   data, which nothing else reads. */

#define INT_CTRL_BREAK 0x1B             /* the BIOS's, from the keyboard */
#define INT_CTRL_C     0x23             /* DOS's, where it finds the key */
#define INT_ERROR      0x24             /* DOS's critical error */

/* Where an interrupt goes, as an address with the offset in the low
   word, and one made to go somewhere. The low byte of `number` is the
   interrupt. */
static u32 Vector(u16 number)
{
    struct DosRegs regs;

    regs.ax = 0x3500 | number & 0xFF;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    return (u32)regs.es << 16 | regs.bx;
}

static void SetVector(u16 number, u32 to)
{
    struct DosRegs regs;

    regs.ax = 0x2500 | number & 0xFF;
    regs.name = (const u8 __far *)to;
    Tell(REGS(regs));
}

/* Where a function is, as such an address. */
#define HANDLER(function) ((u32)(void __far *)(function))

/* The function that the program has handed DeviceErrorTake. */
static DeviceErrorFn *DeviceAsk;

/* The header of a device, which DOS names when it asks about one: the
   high byte of its attributes, where 80h says that it is no drive, and
   the name that such a device has there, 8 characters. */
#define HEADER_KIND 5
#define HEADER_NAME 10

/* INT 24h: what DOS calls for a critical error, with what it says of it
   in AX and DI and the header of the device at BP:SI, with the machine
   kept from interrupting, and on the stack of whoever called DOS: the
   program, or a program that this one has run without giving the
   critical error back first (Aexec0 of view.c). It answers in AL, and
   every other register comes back as it was. An interrupt function is
   entered with DS anywhere, and the compiler has it load DS with the
   program's data.

   The program is asked what to do, by the function that it handed
   over, with what sys.h says that such a function takes: DOS's two
   words as they are, DI whole, as the original reads it, and of the
   header whether it is a drive and, if not, where the name is. AX
   whole is what the function answers, which is DOS's number for it.

   The header is read before the program is asked. The original reads
   it only where it shows a box, behind the two bytes by which it
   answers without one, ErrDat and BlocEr: those are the program's, and
   its function looks at them first, as before.

   Not static, though nothing else names it: the scenarios' test holds
   that the function of this name has run, and looks for it in the
   linker's map among those that are not (harness/ported.py). */
void __interrupt __far FatErr(struct Saved r)
{
    const u8 __far *device = POINTER(r.bp, r.si);

    if (device[HEADER_KIND] & 0x80)
        device += HEADER_NAME;
    else
        device = 0;
    r.ax = DeviceAsk(r.ax, r.di, device);
}

/* INT 1Bh, Ctrl-Break of the BIOS, and INT 23h, Ctrl-C of DOS, while the
   program has them: nothing. */
static void __interrupt __far CtrlC(void)
{
}

void DeviceErrorNote(void)
{
    md.ErrVct = Vector(INT_ERROR);
}

void DeviceErrorTake(DeviceErrorFn *ask)
{
    DeviceAsk = ask;
    SetVector(INT_ERROR, HANDLER(FatErr));
}

void DeviceErrorGive(void)
{
    SetVector(INT_ERROR, md.ErrVct);
}

/* The interrupt flag set, by the compiler's own function for that,
   which is STI and CLD: the direction flag is clear afterwards, as the
   compiler wants it, and the entry of the program calls this for that,
   Alloc0 of resident.c. */
void IntOn(void)
{
    _enable();
}

void CtrlBreakNote(void)
{
    md.BrkVct = Vector(INT_CTRL_BREAK);
}

void CtrlBreakTake(void)
{
    SetVector(INT_CTRL_BREAK, HANDLER(CtrlC));
}

void CtrlBreakGive(void)
{
    SetVector(INT_CTRL_BREAK, md.BrkVct);
}

void CtrlCTake(void)
{
    SetVector(INT_CTRL_C, HANDLER(CtrlC));
}

/* The console: AH 6, with the character in DL, and AH 9. */

void ConsoleChar(u16 character)
{
    struct DosRegs regs;

    regs.ax = 0x0600;
    regs.name = NUMBER(character & 0xFF);
    Tell(REGS(regs));
}

void ConsoleText(const u8 FAR *text)
{
    struct DosRegs regs;

    regs.ax = 0x0900;
    regs.name = text;
    Tell(REGS(regs));
}

/* The keyboard: the BIOS's calls 1, which leaves the zero flag set if no
   key waits, 0 and 2; and the byte of the BIOS's data that has the keys
   which are held down, whose highest bit is Ins. */

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
    *(u8 __far *)0x00000418UL &= 0x7F;
}

/* The screen: the BIOS's calls 0Fh, 0, 3, 2, 1, 6 and 7, 8, 9 and 0Eh,
   which take the page in BH; for an EGA or a VGA 12h with 10h in BL,
   which leaves BX as it is where there is neither, 1130h, 1112h, which
   takes in BL the block of the font, 0, and 12h with 20h in BL; the
   word of the BIOS's data that says where the page begins; and the
   memory of the display, which is at the segment B000h for the mode of
   one colour and at B800h for the others. Its address is kept where the
   original keeps it, in Screen of the main data. */

struct Screen ScreenState(void)
{
    struct BiosRegs regs;
    struct Screen screen;

    regs.ax = 0x0F00;
    regs.bx = 0;
    regs.cx = 0;
    regs.dx = 0;
    Display(BIOS(regs));
    screen.mode = regs.ax & 0xFF;
    screen.columns = regs.ax >> 8;
    screen.page = regs.bx >> 8;
    return screen;
}

/* One of the calls that answer with nothing. */
static void Show(u16 ax, u16 bx, u16 cx, u16 dx)
{
    struct BiosRegs regs;

    regs.ax = ax;
    regs.bx = bx;
    regs.cx = cx;
    regs.dx = dx;
    Display(BIOS(regs));
}

void ScreenSetMode(u16 mode)
{
    Show(mode & 0xFF, 0, 0, 0);
}

struct CursorAt CursorWhere(u16 page)
{
    struct BiosRegs regs;
    struct CursorAt cursor;

    regs.ax = 0x0300;
    regs.bx = page << 8;
    regs.cx = 0;
    regs.dx = 0;
    Display(BIOS(regs));
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

    regs.ax = 0x0800;
    regs.bx = page << 8;
    regs.cx = 0;
    regs.dx = 0;
    Display(BIOS(regs));
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

    regs.ax = 0x1200;
    regs.bx = 0xFF10;
    regs.cx = 0;
    regs.dx = 0;
    Display(BIOS(regs));
    return regs.bx != 0xFF10;
}

/* The height comes back in CX and the last row in DL: both as they go,
   from a BIOS that does not know the call. */
struct Font ScreenFont(void)
{
    struct BiosRegs regs;
    struct Font font;

    regs.ax = 0x1130;
    regs.bx = 0;
    regs.cx = 16;
    regs.dx = 0;
    Display(BIOS(regs));
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
    return *(const u16 __far *)0x0040004EUL;
}

void ScreenMemory(u16 mode, u16 start)
{
    md.Screen = (u32)(mode == 7 ? 0xB000 : 0xB800) << 16 | start;
}

void ScreenPut(u16 cell, const u16 FAR *from, u16 count)
{
    u16 __far *to = (u16 __far *)md.Screen + cell;

    for (; count != 0; count--)
        *to++ = *from++;
}

void ScreenGet(u16 cell, u16 FAR *to, u16 count)
{
    const u16 __far *from = (const u16 __far *)md.Screen + cell;

    for (; count != 0; count--)
        *to++ = *from++;
}

/* The mouse: the driver's calls 21h and 0, 1 and 2, 3, 4, 8 and 0Ah,
   which gets 0 in BX for a pointer that the driver makes of the cell. */

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

    MouseState(3, (struct MouseAt STACK *)&at);
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

/* The timer: the BIOS's count of ticks since midnight, in its data, read
   with the timer kept from counting between its two halves. */
u32 Ticks(void)
{
    u32 ticks;

    _disable();
    ticks = *(const u32 __far *)0x0040006CUL;
    _enable();
    return ticks;
}

/*
 * What is of DOS on a PC alone, and no part of sys.h: where the PSP is.
 */

/* Where the PSP is, its segment, as DOS says (AH 51h, which answers in
   BX: as 62h, and of DOS 2 too). The program's block of the memory is
   known by the same segment. */
u16 ProgramPrefix(void)
{
    struct DosRegs regs;

    regs.ax = 0x5100;
    regs.name = NUMBER(0);
    Tell(REGS(regs));
    return regs.bx;
}
