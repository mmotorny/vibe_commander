/*
 * The program and DOS around it: where the program begins and where it
 * ends, the commands of DOS that it runs, and the program's own call of
 * DOS.
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
 * The third is the program's own call of DOS, by which another program
 * asks it for its data, as the original's setup program does. For that
 * the program takes INT 21h while it runs, and while a command runs from
 * it: every call of DOS comes by New21, which passes on all but that
 * one. The original has more there, its watch on DOS: it notes which
 * program is being run, and when one stays in the memory, by DOS's call
 * for that or by INT 27h, it writes down who it is and which interrupts
 * it has taken; with those records the map of the memory, Alt+F5, lists
 * the programs and releases them. The portable C has no such map and
 * keeps no such watch (see DEVIATIONS.md). New21 and what it goes on in
 * are entered from other programs, on their stacks: nothing there takes
 * the stack to be the program's own, and a pointer to a local variable
 * is declared STACK, as everywhere.
 */
#include "vc.h"

#define LenCmd  124                     /* Length of command line */
#define VcApiVr 42

/*----------------------- The data -----------------------
   First the constants: the name that COMMAND.COM has when the
   environment does not say, and what the commands' code says for
   itself. */
static const u8 Command[] = "COMMAND.COM";
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

/* Where INT 22h went before the program took it: where the program that
   started this one goes on when this one has ended. */
static u32 Vect22;

/* The prompt that is written in front of a command which follows
   another, with the path if the setup says so: Exec02 makes it here.
   The original makes it where its watch notes the interrupts when a
   program is run, IntTab. A drive, a path as long as DOS has one, 64
   bytes with its zero, and five characters around them. */
static u8 PromptLine[2 + 3 + 64 + 1 + 1];

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

/* Where a routine is, as an address for DOS and for a return. */
#define ROUTINE(function) ((const u8 __far *)(void __far *)(function))

/* The interrupts that the program takes here: DOS's own, for the
   program's call, and Ctrl-C of DOS. */
#define INT_DOS    0x21
#define INT_CTRL_C 0x23

#define FLAG_CARRY     0x0001
#define FLAG_PARITY    0x0004
#define FLAG_AUX       0x0010
#define FLAG_ZERO      0x0040
#define FLAG_SIGN      0x0080
#define FLAG_TRAP      0x0100
#define FLAG_INTERRUPT 0x0200
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

/* The return address that an interrupt leaves on the stack, behind the
   registers that an interrupt function of the compiler saves there:
   struct Saved of vc.h. */
struct Frame {
    u16 ip, cs, flags;
};

/*----------------------- The flags -----------------------
   The assembly goes on in the program that started this one, when DOS
   has ended the program, with the flags as its last instruction of
   arithmetic left them. These say what that is, so that the C can leave
   the same. */

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

/* CMP: `b` taken from `a`. */
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

/*----------------------- The interrupts -----------------------*/

/* Ctrl-C of DOS and Ctrl-Break of the BIOS, while the program has them:
   nothing. */
void __interrupt __far CtrlC(void)
{
}

void __interrupt __far Int21(struct Saved r, struct Frame back);

/* INT 21h arrives here, every call of DOS that any program makes while
   this one is in the memory. Of all but one of them the program wants
   nothing, and for those it takes nothing either: no register changed
   and not a byte of the caller's stack, which may be short. They go on
   in the handler that had the interrupt before, with the flags of the
   comparison that sent them there, as in the assembly. Only the
   program's own call goes on to Int21: AH 2Bh with 'VC' in CX and 'OM'
   in DX.

   The original deals with three calls more, for its watch on the
   programs that stay in the memory, which the portable C does not keep
   (see DEVIATIONS.md): a program run, AH 4Bh, one that stays, 31h, and
   a block of memory given back, 49h. Here they are passed on like any
   other.

   Why this is assembly, all eight instructions: what the compiler makes
   saves registers before it compares, on the caller's stack, and cannot
   jump to another handler with the stack as it found it; a function
   without that, naked, is given no instruction for a statement of C at
   all; an ordinary one with the call in AX, nothing to change and the
   jump as a call that does not return makes itself room on the stack,
   changes AL to compare, reads the old address through the caller's DS
   or through an ES that it loads, and jumps with BP still pushed
   (tried, with this compiler). And the flags that the handler before is
   entered with are those of these comparisons, which C cannot promise
   of anything. The old address is read through CS, which is why it is a
   variable of the code's segment. */
__declspec(naked) void __far New21(void)
{
    _asm {
        cmp ah,2Bh
        jne old
        cmp cx,5643h
        jne old
        cmp dx,4F4Dh
        jne old
        jmp Int21
    old:
        jmp dword ptr cs:Vect21
    }
}

/* INT 21h, the one call that New21 does not pass on. Another program
   asks for this one's data, the program's own call (AH 2Bh, CX 'VC', DX
   'OM'): it gets in ES:BX the first panel for AL 0, the second for 1 and
   the main data for anything else, and the number of this interface in
   AH. An interrupt function of the compiler's saves the registers and
   finds DS, and has them as `r`, to read and to change; `back` is what
   the interrupt left on the stack, the place it came from and the flags
   that it had, which the function returns to and with.

   The original hands out what it keeps of the three while a command
   runs, copies in the part of it that stays in the memory: of the main
   data what it has before DosLin, of a panel what it has before ExtBuf
   and behind that 13 bytes of every file. Here it is the panel and the
   main data themselves, which begin as those copies do; the files of a
   panel are where a panel has them, 24 bytes each (see DEVIATIONS.md).

   Ported as it is: the call returns with the flags as the original's
   handler has them, not as its caller had them. The carry flag is
   clear, which is the answer; interrupts are still off, as every
   interrupt begins; and the other flags are those of AL compared with
   1. */
void __interrupt __far Int21(struct Saved r, struct Frame back)
{
    u8 which = (u8)r.ax;                /* VCapi */
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
}

/*------------------- Execute commands ------------------*/

/* Write the text at `text` on the screen as a teletype would, in grey,
   on the page in the high byte of `bx`. Returns where the text ends. A
   text of no characters is written as its zero byte. */
static u16 OutStr(const u8 __far *text, u16 bx)
{
    do
        ScreenType(*text++, bx >> 8, 7);
    while (*text != 0);
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
        u8 key = (u8)KeyRead();

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
    md.BreakFl = (u8)BreakCheck();
    SetBreakCheck(0);
    SetVector(INT_CTRL_C, (u32)ROUTINE(CtrlC));
}

/* Leave: have DOS look for Ctrl-C as it did, and give INT 21h back.
   Then end the program with `code`, unless DOS has ended it already and
   this is where it went on: then return.

   The original first releases the programs that have stayed in the
   memory since it was started, by the list that its watch on DOS keeps,
   and gives INT 27h back as well; the portable C keeps no such list and
   does not take that interrupt (see DEVIATIONS.md). What a command has
   left in the memory stays there when the program leaves.

   `r` and `flags` are the registers and the flags that this was entered
   with, and they are left as the assembly leaves its own, without what
   is gone, for the program that this returns to when DOS has ended this
   one: ES on the segment of this program's PSP, and DS:DX the address
   that INT 21h went to before, which DOS was told last. The others are
   as they came. The flags are those of the segment that the program
   goes on in, compared with 0. */
static void Exit02(u16 code, struct Saved STACK *r, u16 STACK *flags)
{
    r->es = PspSeg;                     /* Exit01 */
    SetBreakCheck(md.BreakFl);
    SetVector(INT_DOS, Vect21);         /* Exit03 */
    r->ds = (u16)(Vect21 >> 16);
    r->dx = (u16)Vect21;
    if ((u16)(Vect22 >> 16) == 0)       /* Exit04 */
        ProgramEnd(code);
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
   flags as Exit02 leaves them. That is done by four instructions of
   assembly, room on the stack for one more return address with the
   flags as they are on top of it, and a jump to an interrupt function
   of the compiler's: that saves the registers and finds DS, has them as
   `r`, to read and to change, and the room as `on`, and ends as every
   interrupt function does, by taking `on` off the stack and going
   there, with the registers as they are in `r`, the flags of `on`, and
   the stack as it was found. The assembly took of that program's stack
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

/* Have the program go on in Exit01 when DOS ends it, and take INT 21h,
   for the program's own call: when the program has started.

   The original takes INT 21h only if its setup has it know of the
   programs that stay in the memory, TSR manager of its setup program,
   as it has when it is distributed, and INT 27h with it. The portable C
   keeps no such watch (see DEVIATIONS.md), and takes INT 21h whatever
   that byte of the setup is. */
void TakeDos(void)
{
    Vect22 = PSP_EXIT;
    PSP_EXIT = (u32)ROUTINE(Exit01);
    Vect21 = Vector(INT_DOS);
    PutVector(INT_DOS, (u32)ROUTINE(New21));
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
    struct Answer ran;
    const u8 __far *name;
    u8 *command, *before, *end, *to;
    u16 count, bx;
    u8 drive;

    for (;;) {
        if (behind)
            goto Exec07;
        SetBreakCheck(md.BreakFl);
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
                ran = ProgramRunBare(name, &md.EPB);
                SetBrk();
                if (!ran.failed)
                    break;
                if (ran.value > 3) {
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
        /* Exec08: the prompt, and the command behind it. BX is the
           address of the command, for the texts and for the end of the
           line. */
        to = PromptLine;
        *to++ = 0x0D;
        *to++ = 0x0A;
        drive = (u8)DriveCurrent();
        *to++ = drive + 'A';
        if (md.Prompt != 0) {
            *to++ = ':';
            *to++ = '\\';
            DirCurrentBare(drive + 1, to);
            while (*to != 0)
                to++;
        }
        *to++ = '>';                    /* Exec09 */
        *to = 0;
        bx = WAS(md.BatAdr);
        OutStr0(PromptLine, bx);
        OutStr0((const u8 *)md.BatAdr, bx);
        ScreenType(0x0D, bx >> 8, bx);
        ScreenType(0x0A, bx >> 8, bx);
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

/* Have DOS look for memory as it does when it starts: the lowest block
   that fits, and the upper memory not in its chain of blocks. This is
   SetStat of the original with 0, which is how the entry calls it; the
   original also notes how DOS looks, GetStat, and puts that back, around
   the release of a program that has stayed in the memory, which the
   portable C does not do (see DEVIATIONS.md). */
static void SetStat(void)
{
    MemSetSearch(0);
    MemSetUpper(0);
}

/* No memory when the program begins: say so and end. */
static void Alloc9(void)
{
    OutStr0(Aloc_S, RESPARS_WAS);       /* BX: the paragraphs of the part
                                           of the original that stays */
    SetStat();
    ProgramEnd(1);
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
    struct SysVersion system;
    u32 end = (u32)(void __far *)Stack;
    u16 length;

    IntOn();                            /* for the direction flag, which
                                           the compiler wants clear */
    PspSeg = ProgramPrefix();
    system = SystemVersion();           /* Check DOS version */
    if ((u8)system.version < 3)         /* Alloc4 */
        ((void (__far *)(void))AT(PspSeg, 0))();
    if (((system.version & 0xFF) << 8 | system.version >> 8) < 0x314) {
        OutStr(Vers_S, system.other);   /* BX: as DOS answered */
        ProgramEnd(2);
    }
    length = (u16)(end >> 16) + (((u16)end + LenStk + 15) >> 4)
             - PspSeg;                  /* Alloc5 */
    if (MemResize(PspSeg, length).failed)
        Alloc9();
    SetStat();                          /* Alloc7 */
    Main();
}
#pragma aux Alloc0 aborts
