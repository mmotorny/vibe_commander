/*
 * The functions of sys.h for Linux: step 6 of the portable C's road
 * (CLAUDE.md, "The portable C", the road, steps 3 and 6), the piece L3a
 * (DIARY.md, "Run the program on Linux, with a test channel and no
 * terminal yet"). The program goes on asking what it asks DOS and the
 * BIOS, and this answers as they would, emulating them on the host at
 * the level of their services.
 *
 * Most of that is the machine, machine_posix.c, which the web page's
 * implementation shares (the piece W1 of step 7, TODO.md, "Step 7, the
 * web page"): DOS's files, directories and drives, the screen's grid,
 * the console, the mouse driver, the memory, the time, the environment
 * and the test channel. What is Linux's own is here:
 *
 *   - The grid is drawn in the terminal that the program is started in,
 *     term_posix.c, the piece L4, but where the test channel is told to
 *     drive it.
 *   - The keys come from the terminal, made the BIOS's words by
 *     keyboard_posix.c, by the kitty protocol of the keyboard where the
 *     terminal has that, with Shift, Ctrl and Alt held; or from the test
 *     channel. The mouse is the terminal's, or the test channel's.
 *   - The program waits for a key or idles in the terminal's poll.
 *   - A program run is a command of the host's shell, $SHELL -c, which
 *     COMSPEC names, on the terminal's own screen, which Ctrl-O shows:
 *     "A command" below, the piece L6. Enter on a file that is no
 *     program hands it to xdg-open.
 */
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "emulate_posix.h"

/* DOS's errors that are answered here. */
#define NO_MEMORY 8

/*
 * The terminal, where the program is not tested: term_posix.c, which
 * draws the page that is shown, the first 2,000 cells, whenever the
 * program asks for a key or is idle, and makes what is typed keys.
 */

#define IDLE_WAIT 50                    /* milliseconds, under a tick */

static bool OnTerminal = false;
static bool CommandShown = false;              /* to a command, which has run: the
                                           terminal's own screen shows */

/* The screen drawn, the terminal taken again first where a command had
   it. */
static void Show(void)
{
    struct ScreenShows shown;

    if (CommandShown) {
        CommandShown = false;
        TermTake();
    }
    TypedForget();
    shown = ScreenShown();
    TermDraw(shown.cells, shown.place, shown.shape);
}

/* Ctrl-O typed at the terminal, as many times as it waits: the
   terminal's own screen, which has what the commands wrote, shown until
   the next key, which is dropped, and the program's again. The program
   does not get the key (DIARY.md, "Run a command of the program for
   Linux in the host's shell"). */
#define CTRL_O 0x180F

static void OwnScreen(void)
{
    struct Keys keys = KeyboardWaiting();

    while (!keys.none && keys.key == CTRL_O) {
        KeyboardTake();
        TermOwnScreen();
        Show();
        TermInput(0);
        keys = KeyboardWaiting();
    }
}

/* The mouse as the terminal has reported it, taken as the driver takes
   a mouse's motion, where it has changed since it was last taken.
   `looked` where the program asks where the mouse is. */
static u32 PointerReports = 0;

static void MouseFromTerminal(bool looked)
{
    struct ReportedMouse pointer = KeyboardMouse(looked);

    if (pointer.count == PointerReports)
        return;
    PointerReports = pointer.count;
    MouseMoved(pointer.buttons, pointer.column, pointer.row);
}

void Idle(void)
{
    if (OnTerminal) {
        MouseFromTerminal(false);
        Show();
        TermInput(IDLE_WAIT);
        MouseFromTerminal(false);
        return;
    }
    TestIdle();
}

/*
 * The keyboard: what the test channel has made due, or the terminal.
 * KeyRead waits for a key, which is idle as Idle is.
 */

/* Asked with no key there, the program is not idle: the bench's driver
   takes only INT 16h 00h and 10h and INT 28h for the program's idle
   call, not 01h, and makes no key due for it. */
struct Keys KeyWaiting(void)
{
    if (OnTerminal) {
        Show();
        TermInput(0);
        MouseFromTerminal(false);
        OwnScreen();
        return KeyboardWaiting();
    }
    return TestKeyWaiting();
}

u16 KeyRead(void)
{
    u16 key;

    if (OnTerminal) {
        for (;;) {
            OwnScreen();
            if (!KeyboardWaiting().none)
                break;
            MouseFromTerminal(false);
            Show();
            TermInput(-1);
        }
        MouseFromTerminal(false);
        key = KeyboardTake();
    } else {
        key = TestKeyRead();
    }
    TraceKey(key);
    return key;
}

u16 KeysHeld(void)
{
    u16 held = OnTerminal ? KeyboardHeld() : TestKeysHeld();

    TraceHeld(held);
    return held;
}

struct MouseAt MouseWhere(void)
{
    if (OnTerminal)
        MouseFromTerminal(true);
    else
        TestMouseAsked();
    return MouseAnswer();
}

/* The timer: the machine's (MachineTicks). */
u32 Ticks(void)
{
    return MachineTicks();
}

/*
 * Other programs, and the end of this one.
 */

/* A command, the piece L6 (DIARY.md, "Run a command of the program for
   Linux in the host's shell"). What the program runs is COMMAND.COM, by
   the name that COMSPEC has in its environment, with /C and the command
   for its tail: COMSPEC names the host's shell, $SHELL or /bin/sh where
   that is not set (ShellOf), and the shell is run with -c and the
   command made one of the host's (DosCommandToHost). Any other program
   is run by the shell too, by its path of the host and its tail.

   On the terminal, the command has it as the user has it, with its
   own screen and its modes as they were before the program took it, in
   the foreground, so that Ctrl-C and Ctrl-Z are its signals and not the
   program's; in front of what it writes, what DOS would have shown
   there: the line of the prompt and the command, which the program has
   on its screen, and what it has typed by the BIOS since (Typed). The
   terminal stays so until the program next draws its screen, so that
   commands run one behind the other are on one screen, as on DOS. A
   command that is stopped, Ctrl-Z, stays so when the program goes on,
   as behind a shell without job control. Under the test channel the
   command has nothing of standard input and writes nowhere.

   DOS's answer is that the program ran, whatever it ended with: the
   shell's 127 for a command that it did not find is a run, as COMMAND.COM
   that says "Bad command or file name" is. `value` is the code that the
   command ended with, which DOS would answer to 4Dh. The shell that
   cannot be run is DOS's 2, file not found, as a COMSPEC that names no
   file is. */
#define FILE_NOT_FOUND 2
#define SHELL_DEFAULT "/bin/sh"
#define COMMAND_ROOM 0x10000
#define TAIL_ROOM 256                   /* a count of a byte, and what it
                                           counts */

static char Shell[HOST_PATH_ROOM];      /* of the host */
static u8 ShellDos[DOS_PATH_ROOM] = "COMMAND.COM";  /* what COMSPEC names */

/* The shell: $SHELL where it is a path from the root, else /bin/sh; and
   the path of DOS's that COMSPEC names it by, or COMMAND.COM where DOS
   cannot name it. */
static void ShellOf(void)
{
    const char *shell = getenv("SHELL");
    char real[PATH_MAX];

    if (shell == 0 || shell[0] != '/' || strlen(shell) >= sizeof Shell)
        shell = SHELL_DEFAULT;
    memcpy(Shell, shell, strlen(shell) + 1);
    if (realpath(Shell, real) != 0)
        DosPathOfHost(&MachineDos.directories, &MachineDos.drives, real,
                      ShellDos);
}

/* Whether two names of DOS's are one, by their capitals. */
static bool SameName(const u8 *one, const u8 *other)
{
    for (; *one != 0 && *other != 0; one++, other++)
        if (CodePageUpper(*one) != CodePageUpper(*other))
            return false;
    return *one == *other;
}

/* What the program has typed, written on the terminal's own screen, and
   forgotten: in front of the first command, the row that the typing
   began in, as far as it began, or all of it where the typing began
   with a new line, which is the prompt and the command that the program
   drew there. */
static void Typed(bool first)
{
    char text[TYPED_TEXT_ROOM];

    TermWrite(text, TypedText(first, text));
}

/* The command run, in UTF-8, in the directory of the host that DOS's
   current one is: how it ended, the code in `value`. */
static struct Answer Run(const char *command)
{
    char directory[HOST_PATH_ROOM];
    struct Answer answer;
    sigset_t none;
    pid_t child;
    int status = 0, null;

    answer.failed = false;
    answer.value = 0;
    if (DosCurrentHost(&MachineDos, directory, (i32)sizeof directory).failed)
        memcpy(directory, "/", 2);
    if (OnTerminal) {
        if (!CommandShown) {
            CommandShown = true;
            TermGive();
            Typed(true);
        } else {
            Typed(false);
        }
    }
    TraceFlush();
    child = fork();
    if (child == 0) {
        if (OnTerminal) {
            /* Its group, which the terminal is handed to; and what it
               says of errors on the terminal too, whatever the program's
               own standard error is. */
            setpgid(0, 0);
            TermHandTo(getpid());
            dup2(1, 2);
        } else {
            null = open("/dev/null", O_RDWR);
            if (null >= 0) {
                dup2(null, 0);
                dup2(null, 1);
                dup2(null, 2);
            }
        }
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, 0);
        if (chdir(directory) != 0 && chdir("/") != 0)
            _exit(127);
        execl(Shell, Shell, "-c", command, (char *)0);
        _exit(127);
    }
    if (child < 0) {
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    if (OnTerminal) {
        setpgid(child, child);
        TermHandTo(child);
    }
    while (waitpid(child, &status, WUNTRACED) < 0 && errno == EINTR)
        ;
    if (OnTerminal)
        TermHandBack();
    if (WIFEXITED(status))
        answer.value = (u16)WEXITSTATUS(status);
    return answer;
}

struct Answer ProgramRun(const u8 *name, const u8 *tail)
{
    u8 dos[DOS_PATH_ROOM + 1 + TAIL_ROOM + 1], full[DOS_PATH_ROOM];
    char host[HOST_PATH_ROOM];
    char *command;
    struct Answer answer;
    i32 length = 0, start = 0, count = tail[0], made;

    TestProgramRun();
    answer.failed = true;
    answer.value = FILE_NOT_FOUND;
    if (access(Shell, X_OK) != 0)
        return answer;
    if (SameName(name, ShellDos)) {
        while (length < count && tail[1 + length] != 0)
            length++;
        if (length < 2 || tail[1] != '/' || CodePageUpper(tail[2]) != 'C')
            return answer;              /* the shell is run for /C alone */
        start = 3;
        if (start < length && tail[1 + start] == ' ')
            start++;
        memcpy(dos, tail + 1 + start, (size_t)(length - start));
        length -= start;
    } else {
        /* Another program, by its full path, which leads to a file of the
           host that the user may run. */
        if (DosPathFull(name, &MachineDos.drives, full).failed)
            return answer;
        answer = HostPathOfDos(&MachineDos.directories, &MachineDos.drives,
                               full, host, (i32)sizeof host);
        if (answer.failed || answer.value != 1 || access(host, X_OK) != 0) {
            answer.failed = true;
            answer.value = FILE_NOT_FOUND;
            return answer;
        }
        length = (i32)strlen((const char *)full);
        memcpy(dos, full, (size_t)length);
        while (start < count && tail[1 + start] != 0)
            dos[length++] = tail[1 + start++];
    }
    command = malloc(COMMAND_ROOM);
    if (command == 0) {
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    made = DosCommandToHost(&MachineDos, dos, length, command, COMMAND_ROOM);
    if (made < 0) {
        free(command);
        answer.failed = true;
        answer.value = NO_MEMORY;
        return answer;
    }
    answer = Run(command);
    free(command);
    return answer;
}

/* Enter on a file, the piece L6b (DIARY.md, "Run a file that the user
   may execute by Enter, hand others to xdg-open, and make names of DOS's
   in a command the host's (L6b)"): a program is a regular file of the
   host that the user may execute, whatever its name, the extensions of
   DOS's meaning nothing here; it is run as ./name, typed .\NAME, which
   the command made one of the host's has as './name'
   (DosCommandToHost). Any other regular file is handed to xdg-open, the
   desktop's opener, which is not waited for and has none of the
   program's terminal: the panels stay. */
static const u8 ProgramBefore[] = ".\\";

const u8 *ProgramOfFile(const u8 *name)
{
    char host[HOST_PATH_ROOM];
    struct stat status;

    if (HostFileOf(name, host, (i32)sizeof host, &status)
        && S_ISREG(status.st_mode) && access(host, X_OK) == 0)
        return ProgramBefore;
    return 0;
}

struct Answer ProgramOpenFile(const u8 *name)
{
    char host[HOST_PATH_ROOM];
    struct stat status;
    struct Answer answer;
    sigset_t none;
    pid_t child, opener;
    int null, ended = 0;

    answer.failed = true;
    answer.value = FILE_NOT_FOUND;
    if (!HostFileOf(name, host, (i32)sizeof host, &status)
        || !S_ISREG(status.st_mode))
        return answer;
    /* A child that starts the opener and ends at once, so that the
       opener is nobody's to wait for. */
    child = fork();
    if (child == 0) {
        setsid();
        null = open("/dev/null", O_RDWR);
        if (null >= 0) {
            dup2(null, 0);
            dup2(null, 1);
            dup2(null, 2);
        }
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, 0);
        opener = fork();
        if (opener == 0) {
            execlp("xdg-open", "xdg-open", host, (char *)0);
            _exit(127);
        }
        _exit(opener < 0 ? 1 : 0);
    }
    if (child < 0) {
        answer.value = NO_MEMORY;
        return answer;
    }
    while (waitpid(child, &ended, 0) < 0 && errno == EINTR)
        ;
    answer.failed = false;
    answer.value = 0;
    return answer;
}

void ProgramEnd(u16 code)
{
    MachineEnd(code);
}

int main(int count, char **arguments)
{
    if (!TestBegin()) {
        if (!TermBegin()) {
            fprintf(stderr, "vc: standard input and output are to be a "
                            "terminal\n");
            return 1;
        }
        OnTerminal = true;
    }
    MachineBegin();
    ShellOf();
    MachineEnvironment(ShellDos, count, arguments);
    Main();
    return 0;
}
