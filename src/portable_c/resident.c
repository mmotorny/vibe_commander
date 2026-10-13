/*
 * The program and DOS around it: where the program begins and where it
 * ends, and the commands of DOS that it runs.
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
 * Two things are here. The first is what the program does before
 * anything else, Alloc3: it looks at the version of DOS and keeps no more
 * of the memory than it is. Where DOS enters the program is the
 * system's, Alloc0 of dos.c, which calls Main of main.c.
 *
 * The second is the commands: Exec runs those that the command line
 * holds, DosBuf, by COMMAND.COM or by the interrupt that COMMAND.COM has
 * for that; and what the program does when it leaves, or when DOS has
 * ended it. Where DOS goes on then is the system's, Exit01 of dos.c,
 * which calls Exit02 here on the stack of the program that started this
 * one: nothing there takes the stack to be the program's own.
 *
 * The original has a third here, which takes INT 21h, DOS's own
 * interrupt, while the program runs and while a command runs from it.
 * It is the program's own call of DOS, by which another program asks it
 * for its data, as the original's setup program does; and its watch on
 * DOS, which notes which program is being run, and when one stays in the
 * memory, by DOS's call for that or by INT 27h, writes down who it is
 * and which interrupts it has taken: with those records the map of the
 * memory, Alt+F5, lists the programs and releases them. The portable C
 * has no such map and keeps no such watch, has the setup program in
 * itself, which asks nobody, and takes neither interrupt (see
 * DEVIATIONS.md): a call of DOS goes to DOS.
 */
#include "vc.h"

#define LenCmd  124                     /* Length of command line */

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

/* The prompt that is written in front of a command which follows
   another, with the path if the setup says so: Exec02 makes it here.
   The original makes it where its watch notes the interrupts when a
   program is run, IntTab. A drive, a path as long as DOS has one, 64
   bytes with its zero, and five characters around them. */
static u8 PromptLine[2 + 3 + 64 + 1 + 1];

/* A text of main_t.c, for a DOS that is too old. */
extern const u8 __far Vers_S[];

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
    CtrlCTake();
}

/* What the program puts back when it leaves, and when DOS has ended
   it: DOS looks for Ctrl-C as it did. The system calls this where DOS
   has ended the program (see ProgramEndedTake of sys.h), on the stack
   of the program that started this one, and Exit00 where the program
   leaves by itself. What the program that started this one then goes
   on with, the registers that the scenario ended shows, is the
   system's: Exit05 of dos.c.

   The original first releases the programs that have stayed in the
   memory since it was started, by the list that its watch on DOS keeps,
   and gives INT 21h and INT 27h back, if its setup has it keep that
   watch; the portable C keeps no such list and takes neither interrupt
   (see DEVIATIONS.md). What a command has left in the memory stays there
   when the program leaves. */
static void Exit02(void)
{
    SetBreakCheck(md.BreakFl);          /* Exit01 */
}

/* Leave the program with `code`: the system is told that the program
   ends itself, so that the program that started this one goes on where
   it would have, the PSP's address for that being as it was; and DOS
   ends this one. */
void Exit00(u16 code)
{
    ProgramEndedGive();
    Exit02();
    ProgramEnd(code);                   /* Exit04 */
}

/* Have the program be told when DOS ends it: when the program has
   started. On DOS the system has the PSP name a routine of its own for
   that, Exit01 of dos.c, which calls Exit02. This was TakeDos while it
   took INT 21h as well.

   The original takes INT 21h here too, and INT 27h with it, if its
   setup has it know of the programs that stay in the memory, TSR manager
   of its setup program, as it has when it is distributed: for that
   watch, and for the program's own call, by which its setup program
   asks it for its data. The portable C keeps no such watch and has the
   setup in itself (see DEVIATIONS.md), and takes neither. */
void TakeEnd(void)
{
    ProgramEndedTake(Exit02);
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
        for (;;) {                      /* Exec05 */
            if (md.ComSpec == 0) {
                name = (const u8 __far *)Command;
            } else {
                name = ProgramEnvironment() + md.ComSpec;
                ran = ProgramRunBare(name, before);
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

/*---------------------- The start ----------------------*/

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

/* What the program does first, before anything else is asked of the
   system: Main of main.c calls this when the system has entered the
   program. It is the original's Alloc0 from its label Alloc3 on; what
   stands in front of that label there, and where DOS enters the
   program, is the system's: Alloc0 of dos.c.

   DOS has to be 3.20 at the least. DOS has given the program all of the
   memory: its block is made as long as the program is, MemOwn, and the
   rest is asked for again when the panels are made, Init11 of start.c.

   Ported as it is: how DOS looks for memory is set to what it is when
   DOS starts, the lowest block that fits and no upper memory, whatever
   it was.

   Not as the original, which is a .COM file (see DEVIATIONS.md): a DOS
   before 3 is left through the INT 20h that every PSP begins with,
   entered by a far call, where the original returns there:
   ProgramEndOld.

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
void Alloc3(void)
{
    struct SysVersion system;

    system = SystemVersion();           /* Check DOS version */
    if ((u8)system.version < 3)         /* Alloc4 */
        ProgramEndOld();
    if (((system.version & 0xFF) << 8 | system.version >> 8) < 0x314) {
        OutStr(Vers_S, system.other);   /* BX: as DOS answered */
        ProgramEnd(2);
    }
    if (MemOwn().failed)                /* Alloc5 */
        Alloc9();
    SetStat();                          /* Alloc7 */
}
