/*
 * The functions of sys.h for the app for iOS: step 8 of the portable
 * C's road (CLAUDE.md, "The app for iOS"), the piece I2 (TODO.md, "Step
 * 8, the app for iOS"). The program goes on asking what it asks DOS and
 * the BIOS: the machine of the program for Linux answers,
 * machine_posix.c with the emulation of DOS behind it, over the files of
 * the app, and this file has what the app has of its own, as sys_web.c
 * has the web page's. It is plain C with POSIX's threads; what it asks
 * of the shell, and the shell of it, is ios_shell.h.
 *
 *   - The program runs on a thread of its own, which ios_start starts.
 *     It waits for a key on a condition variable, which the shell's
 *     thread signals when it hands bytes on, ios_input: a key's, as a
 *     terminal with the kitty protocol sends them, or the mouse's
 *     report, made the BIOS's words and the driver's motions by
 *     keyboard_posix.c as on Linux and the web.
 *   - The screen's grid is copied for the shell whenever the program
 *     waits, as the web page has it drawn at a yield: KeyRead and Idle
 *     before they wait; KeyWaiting where 50 ms have passed since the
 *     last copy, so that a copy or a search that looks for Esc is seen
 *     going on; and Ticks at its second read with no copy between, so
 *     that the zooming boxes and the blanker are seen. The shell is
 *     told, ios_shell_draw, and takes the copy on its own thread,
 *     ios_screen.
 *   - There are two drives, C:, labelled VIBE, the app's Documents, with
 *     the program's own files in C:\VC, and D:, labelled DEMO, the demo,
 *     as the shell names them. A test may have the program's files on
 *     another drive, VC_PROGRAM_DRIVE, D: as on the bench of DOS.
 *   - A command cannot be run: DOS's screen says so, and the command
 *     answers that it ran with the code 1. Enter on a file hands it to
 *     the shell, which shows it by QuickLook; nothing is a program.
 *   - When the program ends, F10, the shell keeps its last screen and
 *     may start it anew, ios_start again, on a thread of its own again.
 *   - Or the keys come from the test channel, VC_TEST, as on Linux and
 *     the web, the grid being written there, and the end of the program
 *     is the process's.
 *   - The room of the editor and the viewer has a cap: half of what the
 *     system lets the app have when the program starts, and no more
 *     than 256 MB (CLAUDE.md, "The app for iOS").
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "emulate_posix.h"
#include "ios_shell.h"

#define VIBE_LABEL "VIBE"
#define DEMO_LABEL "DEMO"
#define PROGRAM_DRIVE_VARIABLE "VC_PROGRAM_DRIVE"

#define IDLE_WAIT 50                    /* milliseconds, under a tick */
#define BUSY_DRAW 50                    /* milliseconds that KeyWaiting
                                           goes without a copy */
#define TICKS_DRAW 15                   /* milliseconds that Ticks goes
                                           without a copy: under a tick,
                                           so that each step of a box
                                           that zooms is seen */
#define SPIN_WAIT 1                     /* milliseconds that Ticks lets
                                           go by where the program waits
                                           for the next tick itself */
#define QUEUE_ROOM 4096                 /* bytes that the shell may have
                                           handed on and the program not
                                           taken yet */
#define STACK_BYTES (8 * 1024 * 1024)   /* the program's thread's: as
                                           Linux gives the main thread,
                                           where the device gives another
                                           thread 512 KB */
#define CAP_MOST ((int64_t)256 * 1024 * 1024)

static bool Testing = false;            /* driven by the test channel */
static bool Begun = false;              /* the test channel looked for */
static int64_t Cap = 0;                 /* the room's, in bytes */

/* The milliseconds of a clock that goes on, from wherever it starts. */
static int64_t NowMs(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/*
 * The screen, copied for the shell.
 */

static pthread_mutex_t ScreenLock = PTHREAD_MUTEX_INITIALIZER;
static uint16_t ScreenCells[IOS_CELLS];
static u16 ScreenPlace = 0, ScreenShape = 0;
static uint32_t ScreensHanded = 0;
static int64_t DrawnAt = 0;             /* NowMs() of the last copy */
static i32 TicksRead = 0;               /* since the last copy */

/* The grid copied for the shell, with where the cursor is, and the
   shell told. Under the test channel, which has its own record of what
   was typed, nothing is forgotten of that. */
static void Draw(void)
{
    struct ScreenShows shown;

    if (!Testing)
        TypedForget();
    shown = ScreenShown();
    pthread_mutex_lock(&ScreenLock);
    memcpy(ScreenCells, shown.cells, sizeof ScreenCells);
    ScreenPlace = shown.place;
    ScreenShape = shown.shape;
    ScreensHanded++;
    pthread_mutex_unlock(&ScreenLock);
    DrawnAt = NowMs();
    TicksRead = 0;
    ios_shell_draw();
}

uint32_t ios_screen(uint16_t cells[IOS_CELLS], int *row, int *column,
                    uint16_t *shape)
{
    uint32_t handed;

    pthread_mutex_lock(&ScreenLock);
    memcpy(cells, ScreenCells, sizeof ScreenCells);
    *row = ScreenPlace >> 8;
    *column = ScreenPlace & 0xFF;
    *shape = ScreenShape;
    handed = ScreensHanded;
    pthread_mutex_unlock(&ScreenLock);
    return handed;
}

/*
 * The bytes that the shell hands on, queued until the program takes
 * them.
 */

static pthread_mutex_t InputLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t InputCame = PTHREAD_COND_INITIALIZER;
static char Queue[QUEUE_ROOM];
static i32 Queued = 0;

void ios_input(const char *bytes, int count)
{
    i32 fits;

    if (count <= 0)
        return;
    pthread_mutex_lock(&InputLock);
    fits = QUEUE_ROOM - Queued;
    if (count < fits)
        fits = count;
    memcpy(Queue + Queued, bytes, (size_t)fits);
    Queued += fits;
    pthread_cond_signal(&InputCame);
    pthread_mutex_unlock(&InputLock);
}

/* What is queued, as much as fits at `to`, which has room for `room`
   bytes, the count answered; where nothing is, after a wait of `wait`
   milliseconds at the most, -1 for as long as it takes, for the shell
   to hand something on. */
static i32 Taken(char *to, i32 room, i32 wait)
{
    struct timespec until;
    int64_t nanoseconds;
    i32 got;

    pthread_mutex_lock(&InputLock);
    if (Queued == 0 && wait != 0) {
        if (wait < 0) {
            while (Queued == 0)
                pthread_cond_wait(&InputCame, &InputLock);
        } else {
            /* The real time, which pthread_cond_timedwait goes by
               wherever the clock of a condition cannot be chosen, as on
               Apple's systems. */
            clock_gettime(CLOCK_REALTIME, &until);
            nanoseconds = until.tv_nsec + (int64_t)wait * 1000000;
            until.tv_sec += (time_t)(nanoseconds / 1000000000);
            until.tv_nsec = (long)(nanoseconds % 1000000000);
            while (Queued == 0
                   && pthread_cond_timedwait(&InputCame, &InputLock,
                                             &until) != ETIMEDOUT)
                ;
        }
    }
    got = Queued < room ? Queued : room;
    memcpy(to, Queue, (size_t)got);
    memmove(Queue, Queue + got, (size_t)(Queued - got));
    Queued -= got;
    pthread_mutex_unlock(&InputLock);
    return got;
}

/* What the shell has handed on of the keyboard and the mouse, taken,
   after a wait of `wait` milliseconds where nothing is queued and
   `wait` is not 0, -1 for as long as it takes, the grid copied first:
   as PageInput of sys_web.c takes the page's. */
static void ShellInput(i32 wait)
{
    char *room;
    i32 space, got;

    if (!KeyboardBefore(&wait))
        return;                         /* what waits fills the room */
    space = KeyboardRoom(&room);
    got = Taken(room, space, 0);
    if (got == 0 && wait != 0) {
        Draw();
        got = Taken(room, space, wait);
    }
    if (got > 0)
        KeyboardAfter(got);
}

/* The mouse as the shell has reported it, taken as the driver takes a
   mouse's motion, where it has changed since it was last taken, as
   MouseFromPage of sys_web.c has it. */
static u32 PointerReports = 0;

static void MouseFromShell(bool looked)
{
    struct ReportedMouse pointer = KeyboardMouse(looked);

    if (pointer.count == PointerReports)
        return;
    PointerReports = pointer.count;
    MouseMoved(pointer.buttons, pointer.column, pointer.row);
}

void Idle(void)
{
    if (Testing) {
        TestIdle();
        Draw();
        return;
    }
    MouseFromShell(false);
    ShellInput(IDLE_WAIT);
    MouseFromShell(false);
}

/*
 * The keyboard: what the test channel has made due, or the shell.
 */

struct Keys KeyWaiting(void)
{
    if (NowMs() - DrawnAt >= BUSY_DRAW)
        Draw();
    if (Testing)
        return TestKeyWaiting();
    ShellInput(0);
    MouseFromShell(false);
    return KeyboardWaiting();
}

u16 KeyRead(void)
{
    u16 key;

    if (Testing) {
        key = TestKeyRead();
        Draw();
    } else {
        while (KeyboardWaiting().none) {
            MouseFromShell(false);
            ShellInput(-1);
        }
        MouseFromShell(false);
        key = KeyboardTake();
    }
    TraceKey(key);
    return key;
}

u16 KeysHeld(void)
{
    u16 held = Testing ? TestKeysHeld() : KeyboardHeld();

    TraceHeld(held);
    return held;
}

struct MouseAt MouseWhere(void)
{
    if (Testing)
        TestMouseAsked();
    else
        MouseFromShell(true);
    return MouseAnswer();
}

/* The timer: the machine's (MachineTicks), with the grid copied at the
   second read that has no copy between, where the program waits for the
   next tick by itself, at most every TICKS_DRAW ms, and that wait made
   gentler on the device's battery than a loop that only reads. */
u32 Ticks(void)
{
    struct timespec spin;

    if (++TicksRead > 1) {
        if (Testing || NowMs() - DrawnAt >= TICKS_DRAW) {
            Draw();
        } else {
            spin.tv_sec = 0;
            spin.tv_nsec = SPIN_WAIT * 1000000L;
            nanosleep(&spin, 0);
        }
    }
    return MachineTicks();
}

/*
 * Other programs, and the end of this one.
 */

/* A command, on the user's word of 15 October 2026 (CLAUDE.md, "The app
   for iOS"): what DOS has shown has the prompt and the command, which
   the program has typed there by the BIOS, and behind it a line that
   says that it cannot be run; DOS's answer is that the program ran, and
   ended with the code 1, as on the web page. */
static const u8 NoCommand[] = "Commands cannot be run on iOS.\r\n$";

struct Answer ProgramRun(const u8 *name, const u8 *tail)
{
    struct Answer answer;

    (void)name;
    (void)tail;
    TestProgramRun();
    TypedForget();
    ConsoleText(NoCommand);
    answer.failed = false;
    answer.value = 1;
    return answer;
}

/* Nothing in the app is a program. */
const u8 *ProgramOfFile(const u8 *name)
{
    (void)name;
    return 0;
}

/* Enter on a file that is no program: the shell shows it by QuickLook. */
#define FILE_NOT_FOUND 2

struct Answer ProgramOpenFile(const u8 *name)
{
    char host[HOST_PATH_ROOM];
    struct stat status;
    struct Answer answer;

    answer.failed = true;
    answer.value = FILE_NOT_FOUND;
    if (!HostFileOf(name, host, (i32)sizeof host, &status)
        || !S_ISREG(status.st_mode))
        return answer;
    TraceOpen(name);
    ios_shell_open_file(host);
    answer.failed = false;
    answer.value = 0;
    return answer;
}

/* The end: the last screen handed, the machine ended, the shell told,
   and the program's thread ended, the shell going on, as it does under
   the test channel too, where it ends the process. */
void ProgramEnd(u16 code)
{
    Draw();
    MachineStop(code);
    ios_shell_ended(code & 0xFF);
    pthread_exit(0);
}

/* Built with the sanitizer of addresses, as sys_web.c says why. */
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
const char *__asan_default_options(void);
const char *__asan_default_options(void)
{
    return "allocator_may_return_null=1:quarantine_size_mb=0";
}
#endif
#endif

/*
 * The start.
 */

static struct DriveGiven Given[DOS_DRIVES];
static i32 GivenCount = 0;
static u8 Own = 'C';                    /* the drive of the program's
                                           own files */
static char ProgramName[] = "vc";
static char *Arguments[] = {ProgramName, 0};

static void *Program(void *unused)
{
    (void)unused;
    (void)Cap;                          /* I1: MachineBegin's cap */
    MachineBegin(Given, GivenCount, Own);
    MachineEnvironment((const u8 *)"COMMAND.COM", 1, Arguments);
    DrawnAt = NowMs();
    TicksRead = 0;
    PointerReports = 0;
    Main();
    return 0;
}

int ios_start(const char *drive_c, const char *drive_d)
{
    pthread_attr_t attributes;
    pthread_t thread;
    struct stat status;
    const char *own = getenv(PROGRAM_DRIVE_VARIABLE);
    uint64_t available;
    int failed;

    if (drive_c == 0 || stat(drive_c, &status) != 0
        || !S_ISDIR(status.st_mode))
        return -1;
    if (drive_d != 0
        && (stat(drive_d, &status) != 0 || !S_ISDIR(status.st_mode)))
        return -1;
    GivenCount = 0;
    Given[GivenCount].letter = 'C';
    Given[GivenCount].root = drive_c;
    Given[GivenCount++].label = VIBE_LABEL;
    if (drive_d != 0) {
        Given[GivenCount].letter = 'D';
        Given[GivenCount].root = drive_d;
        Given[GivenCount++].label = DEMO_LABEL;
    }
    Own = own != 0 && (own[0] & ~0x20) == 'D' && drive_d != 0 ? 'D' : 'C';
    if (!Begun) {
        Testing = TestBegin();
        Begun = true;
    }
    available = ios_shell_memory();
    Cap = available / 2 < (uint64_t)CAP_MOST ? (int64_t)(available / 2)
                                              : CAP_MOST;
    if (pthread_attr_init(&attributes) != 0)
        return -1;
    failed = pthread_attr_setstacksize(&attributes, STACK_BYTES)
             || pthread_attr_setdetachstate(&attributes,
                                            PTHREAD_CREATE_DETACHED)
             || pthread_create(&thread, &attributes, Program, 0);
    pthread_attr_destroy(&attributes);
    return failed ? -1 : 0;
}
