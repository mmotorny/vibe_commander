/*
 * The program and DOS around it: what it does first, the commands of
 * DOS that it runs, and what it does when it leaves.
 *
 * This file was resident.c, and is that in the C translation. In the
 * original, and in the C translation after it, it is the part of the
 * program that stays in the memory while a command runs, which that
 * name said: one segment with the PSP, a stack and copies of the setup
 * and the panels, which gives the rest of the program up for the
 * command and loads it again afterwards. The portable C stays in the
 * memory whole (see DEVIATIONS.md), so nothing of it is resident, and
 * the file is named for the largest thing that it has, Exec, the
 * running of the commands. It is a file of the program like any other:
 * md is the main data, its variables are variables, and what it wants
 * of DOS it asks through sys.h. Nothing that DOS enters is here: where
 * the program begins and where DOS goes on when it has ended the
 * program are the system's, in dos32.c.
 *
 * Two things are here. The first is what the program does before
 * anything else, Alloc3: it looks at the version of DOS and keeps no more
 * of the memory than it is. Main of main.c calls it.
 *
 * The second is the commands: Exec runs those that the command line
 * holds, DosBuf, by COMMAND.COM; and what the program does when it
 * leaves. Where DOS ends it by another's hand, nothing of it is
 * entered: the system puts back what the program has said beforehand,
 * SetBrk.
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
extern const u8 Vers_S[];

/* Where things are in the original, as it was shipped: what its
   registers hold when it calls the BIOS with them unset, and the BIOS
   takes them for a page of the screen and a colour. What the original
   keeps of the main data while a command runs, ComnIni, is at 25E5h
   there and laid out as the main data is, so a place of the main data
   is as far behind it; the part of it that stays in the memory is 31Ah
   paragraphs. */
#define COMNINI_WAS 0x25E5
#define WAS(place)  ((u16)((place) + COMNINI_WAS))
#define EPB_WAS     WAS(MD_PLACE(&md.EPB))
#define RESPARS_WAS 0x031A

/*------------------- Execute commands ------------------*/

/* Write the text at `text` on the screen as a teletype would, in grey,
   on the page in the high byte of `bx`. A text of no characters is
   written as its zero byte. The assembly leaves SI where the text ends,
   and nothing goes on with that. */
static void OutStr(const u8 *text, u16 bx)
{
    do
        ScreenType(*text++, bx >> 8, 7);
    while (*text != 0);
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
    OutStr(text, bx);
}

/* Say that the file `name` is missing, after `text`, and wait for Enter
   or Esc: true for Esc, to give up. The assembly has the address of
   ComnIni.EPB in BX where it calls, which DOS was to run the file
   with. */
static bool NotFind(const u8 *text, const u8 *name)
{
    OutStr0(text, EPB_WAS);
    OutStr(name, EPB_WAS);
    OutStr0(Retr_S, EPB_WAS);
    for (;;) {
        u8 key = (u8)KeyRead();

        if (key == 0x0D)
            return false;
        if (key == 0x1B)
            return true;
    }
}

/* Note whether DOS looks for Ctrl-C at every call, have it not do so,
   and take Ctrl-C: when the program starts, and after every command.
   What was noted is what DOS is to be told again if it ends the program
   by another's hand, and the system is told that at once, since the
   program is not asked then: see TakeEnd. */
void SetBrk(void)
{
    md.BreakFl = BreakCheck();
    ProgramEndedBreakCheck(md.BreakFl);
    SetBreakCheck(false);
    CtrlCTake();
}

/* Leave the program with `code`: the system is told that the program
   ends itself, so that the program that started this one goes on where
   it would have, the PSP's address for that being as it was; DOS looks
   for Ctrl-C as it did, which is what the program puts back when it
   leaves, Exit02 of the original; and DOS ends this one.

   The original first releases the programs that have stayed in the
   memory since it was started, by the list that its watch on DOS keeps,
   and gives INT 21h and INT 27h back, if its setup has it keep that
   watch; the portable C keeps no such list and takes neither interrupt
   (see DEVIATIONS.md). What a command has left in the memory stays there
   when the program leaves. */
void Exit00(u16 code)
{
    ProgramEndedGive();
    SetBreakCheck(md.BreakFl);          /* Exit02 */
    ProgramEnd(code);                   /* Exit04 */
}

/* Take the program's end: when the program has started. From now on,
   if DOS ends the program by another's hand, the system has DOS look
   for Ctrl-C as it did before the program that started this one goes
   on, as SetBrk has last said: what Exit00 does where the program
   leaves by itself. The original's Exit02 is both, entered from the
   PSP at Exit01 where DOS has ended the program; here nothing of the
   program is entered then, and the registers that the program which
   started this one goes on with, which the scenario ended shows, are
   the system's to leave. On DOS the system has the PSP name a routine
   of its own for that: a few instructions of real mode that dos32.c
   keeps below the megabyte. This was TakeDos while it took INT 21h as
   well.

   The original takes INT 21h here too, and INT 27h with it, if its
   setup has it know of the programs that stay in the memory, TSR manager
   of its setup program, as it has when it is distributed: for that
   watch, and for the program's own call, by which its setup program
   asks it for its data. The portable C keeps no such watch and has the
   setup in itself (see DEVIATIONS.md), and takes neither. */
void TakeEnd(void)
{
    ProgramEndedTake();
}

/* No memory: say so and leave. `bx` is what the assembly has in BX
   where it jumps here, a number of DOS's or an address. */
void AlocEr(u16 bx)
{
    OutStr0(Aloc_S, bx);
    Exit00(1);
}

/* Run the commands that DosBuf holds from NextCommand on, one after the
   other: each ends with a zero byte, and behind the last there is one
   more. A command that begins with `@`, behind another, is run without
   being shown first, with a prompt, as the others are. Returns when
   none is left, or when COMMAND.COM cannot be run.

   A command is at most 123 characters; what a longer one has beyond
   that is cut off by the carriage return that is written there.

   COMMAND.COM is run for each command, by the name that COMSPEC has in
   the environment, with /C and the command for its tail, which is
   written over the four bytes in front of the command: those in front
   of the first are the last four of DosChg, which is there for them.

   The original has a second way, with ExecTyp, Quick execute commands
   of its setup: the commands are handed to the COMMAND.COM that the
   program was started from, by that shell's own interrupt, INT 2Eh,
   and before the first of them a command that goes to the current
   drive, if that is another than when the program last started. The
   portable C has neither the setting nor that way (see
   DEVIATIONS.md). */
static void Exec02(void)
{
    struct Answer ran;
    const u8 *name;
    u8 *command, *before, *end, *to;
    i32 count, drive;
    u16 bx;

    for (;;) {
        SetBreakCheck(md.BreakFl);
        command = NextCommand;
        before = command - 4;
        for (end = command; *end++ != 0; )
            ;
        NextCommand = end;
        for (end = command, count = LenCmd; count; count--)
            if (*end++ == 0)
                break;
        end--;
        *end = 0x0D;
        before[0] = (u8)(end - 1 - before);     /* Exec04 */
        before[1] = '/';
        before[2] = 'C';
        before[3] = ' ';
        for (;;) {                      /* Exec05 */
            if (ShellPlace == 0) {
                name = Command;
            } else {
                name = ProgramEnvironment() + ShellPlace;
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
        command = NextCommand;          /* Exec07 */
        if (*command == 0)
            return;
        if (*command == '@') {
            NextCommand++;
            continue;
        }
        /* Exec08: the prompt, and the command behind it. BX is the
           address of the command, for the texts and for the end of the
           line. */
        to = PromptLine;
        *to++ = 0x0D;
        *to++ = 0x0A;
        drive = DriveCurrent();
        *to++ = (u8)(drive + 'A');
        if (md.Prompt != 0) {
            *to++ = ':';
            *to++ = '\\';
            DirCurrentBare((u16)(drive + 1), to);
            while (*to != 0)
                to++;
        }
        *to++ = '>';                    /* Exec09 */
        *to = 0;
        bx = WAS(MD_PLACE(NextCommand));
        OutStr0(PromptLine, bx);
        OutStr0(NextCommand, bx);
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
   but the part that this is of, if the program is set up so. */
void Exec(void)
{
    NextCommand = md.DosBuf;            /* Exec01 */
    Exec02();
    Exec10();
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
    MemSetUpper(false);
}

/* No memory when the program begins: say so and end. */
static NO_RETURN void Alloc9(void)
{
    OutStr0(Aloc_S, RESPARS_WAS);       /* BX: the paragraphs of the part
                                           of the original that stays */
    SetStat();
    ProgramEnd(1);
}

/* What the program does first, before anything else is asked of the
   system: Main of main.c calls this when the system has entered the
   program. It is the original's Alloc0 from its label Alloc3 on; what
   stands in front of that label there, and where DOS enters the
   program, is the system's: Start of dos32.c.

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
