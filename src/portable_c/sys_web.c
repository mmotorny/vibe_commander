/*
 * The functions of sys.h for the web page: step 7 of the portable C's
 * road (CLAUDE.md, "The web page"), the piece W2 (TODO.md, "Step 7, the
 * web page"). The program is built to WebAssembly by Emscripten, and
 * goes on asking what it asks DOS and the BIOS: the machine of the
 * program for Linux answers, machine_posix.c with the emulation of DOS
 * behind it, over the file system that Emscripten gives, and this file
 * has what the page has of its own, as sys_posix.c has Linux's:
 *
 *   - There are at most two drives, C: and D:, each the directory of
 *     the host that a variable of the environment names, VC_DRIVE_C and
 *     VC_DRIVE_D, and labelled as VC_LABEL_C and VC_LABEL_D say, VC and
 *     DEMO where they say nothing (DosBeginGiven of files_posix.c). A
 *     drive whose variable is not set is not there. The page names C:
 *     alone, a directory of Emscripten's file system in the page's
 *     memory, loaded from what is served with the page, with the
 *     program's own files in C:\VC, and starts the program there
 *     (web/page.js, web/drives.js); what the program changes there lasts
 *     until the page is closed or loaded again. Under Node a
 *     test gives both, with their labels, as the bench of DOS has them
 *     (harness/posixrun.py). The program's own files are in the
 *     directory vc of the drive that VC_PROGRAM_DRIVE names, C: where it
 *     names none: C:\VC on the page, and D:\VC in the tests. The
 *     program starts in the host's current directory where a drive has
 *     it, else at the root of the first drive.
 *   - The screen's grid is handed to the page, web_draw of the page's
 *     library (web/library.js), whenever the program yields; the keys
 *     come from the page as the bytes that a terminal with the kitty
 *     protocol sends (web/keys.js), and the mouse as its reports, made
 *     the BIOS's words and the driver's motions by keyboard_posix.c,
 *     web_input. Or both come from the test channel, as on Linux, and
 *     the grid is written there. Or the test channel has its keys and
 *     its mouse come through the page, as a user's do, where
 *     VC_TEST_PAGE is set besides (harness/webpagerun.py): it says
 *     which event is due, web_test_event, and is told each screen that
 *     it records, web_test_screen, which the page has just drawn.
 *   - The program yields to the page by Emscripten's Asyncify, at the
 *     places that the plan names: KeyRead waits by emscripten_sleep and
 *     Idle yields; KeyWaiting yields once 50 ms have passed since the
 *     last yield, so that a copy or a search that looks for Esc lets
 *     the page go on, and MouseWhere so too, and takes what the page has
 *     queued, so that a loop that waits for a button to be let go gets
 *     the release; Ticks yields at its second read with no yield
 *     between, so that the zooming boxes and the blanker are drawn.
 *     The page calls nothing of the program while it waits: what comes
 *     meanwhile is queued in the page and taken here at each yield.
 *   - A command cannot be run: DOS's screen says so, and the command
 *     answers that it ran with the code 1. Enter on a file opens it in a
 *     new tab of the browser, web_open_file; nothing is a program.
 */
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "emulate_posix.h"

/* What the page's library has, web/library.js: the bytes that wait from
   the keyboard and the mouse, as many as fit at `to`, the count
   answered; the grid drawn; and a file of the host opened in a new tab,
   `name` being DOS's name of it. And of the page's file system: the
   directory of the host that is DOS's current one, where the page puts
   what the user brings in. */
extern i32 web_input(char *to, i32 room);
extern void web_draw(const u16 *cells, u16 place, u16 shape);
extern void web_open_file(const char *host, const u8 *name);
extern void web_where(const char *host);
/* And of a test of the page: the event of the test channel's keys of
   that number had the page give, waited for; and a screen that the
   channel has recorded, of its kind and number, told, and waited for
   while the test reads it from the page. */
extern void web_test_event(i32 event);
extern void web_test_screen(const char *kind, u32 number);

/* The drives, by the variables of the environment that name each: its
   directory, its label and, of one of them, that the program's own files
   are there. */
#define DRIVE_VARIABLE "VC_DRIVE_"      /* and the letter */
#define LABEL_VARIABLE "VC_LABEL_"      /* and the letter */
#define PROGRAM_DRIVE_VARIABLE "VC_PROGRAM_DRIVE"
#define PAGED_VARIABLE "VC_TEST_PAGE"
#define LABEL_LONGEST 11                /* DOS's */
/* The cap of the memory that the program's blocks hold together: the
   memory of wasm, once grown, is never given back while the page is
   open, and the program asks for all that there is (machine_posix.c).
   The page's ceiling, MAXIMUM_MEMORY of CMakeLists.txt, is this and what
   the runtime needs besides (TODO.md, "The cap of the memory: done"). */
#define MEMORY_CAP ((int64_t)256 << 20)
static const struct {
    u8 letter;
    const char *label;                  /* where its variable says none */
} Drives[DOS_DRIVES] = {{'C', "VC"}, {'D', "DEMO"}};

#define IDLE_WAIT 50                    /* milliseconds, under a tick */
#define BUSY_YIELD 50.0                 /* milliseconds that KeyWaiting
                                           goes without a yield */

static bool Testing = false;            /* driven by the test channel */
static bool Paged = false;              /* its keys through the page */
static double YieldedAt = 0;            /* emscripten_get_now() */
static i32 TicksRead = 0;               /* since the last yield */

/* The grid handed to the page, with where DOS is. */
static void Draw(void)
{
    struct ScreenShows shown;
    char where[HOST_PATH_ROOM];

    if (Testing && !Paged)
        return;
    TypedForget();
    shown = ScreenShown();
    web_draw(shown.cells, shown.place, shown.shape);
    if (DosCurrentHost(&MachineDos, where, (i32)sizeof where).failed)
        where[0] = 0;
    web_where(where);
}

/* The page let go on for `wait` milliseconds, the grid drawn first. */
static void Yield(i32 wait)
{
    Draw();
    emscripten_sleep((unsigned int)wait);
    YieldedAt = emscripten_get_now();
    TicksRead = 0;
}

/* What the page has queued of the keyboard and the mouse, taken, after a
   yield of `wait` milliseconds where nothing is queued and `wait` is
   not 0, -1 for IDLE_WAIT: as TermInput of term_posix.c takes the
   terminal's bytes. */
static void PageInput(i32 wait)
{
    char *room;
    i32 space, got;

    if (!KeyboardBefore(&wait))
        return;                         /* what waits fills the room */
    space = KeyboardRoom(&room);
    got = web_input(room, space);
    if (got > 0) {
        KeyboardAfter(got);
        return;
    }
    if (wait == 0)
        return;
    Yield(wait < 0 || wait > IDLE_WAIT ? IDLE_WAIT : wait);
    space = KeyboardRoom(&room);
    got = web_input(room, space);
    if (got > 0)
        KeyboardAfter(got);
}

/* The mouse as the page has reported it, taken as the driver takes a
   mouse's motion, where it has changed since it was last taken, as
   MouseFromTerminal of sys_posix.c has it. */
static u32 PointerReports = 0;

static void MouseFromPage(bool looked)
{
    struct ReportedMouse pointer = KeyboardMouse(looked);

    if (pointer.count == PointerReports)
        return;
    PointerReports = pointer.count;
    MouseMoved(pointer.buttons, pointer.column, pointer.row);
}

/* What the page has queued taken, and what the decoder has held back
   behind a change that the program has not yet seen made keys as far as
   it may be now: through the page, the test channel's program reads its
   keys, what is held and the mouse from the decoder, which it takes no
   bytes for otherwise. */
static void PageTake(void)
{
    char *room;
    i32 space, got, wait;

    if (!Paged)
        return;
    do {
        wait = 0;
        if (!KeyboardBefore(&wait))
            break;
        space = KeyboardRoom(&room);
        got = web_input(room, space);
        if (got > 0)
            KeyboardAfter(got);
    } while (got > 0);
}

/* An event of the test channel had the page give, and what the page has
   then queued taken, as a user's keys and mouse are; or, for -1, only
   what it has queued. */
static void PageDeliver(i32 event)
{
    if (event >= 0)
        web_test_event(event);
    PageTake();
    MouseFromPage(false);
}

/* A screen that the test channel has recorded: drawn, and the page told,
   which reads it from what it shows. */
static void PageRecorded(const char *kind, u32 number)
{
    Draw();
    web_test_screen(kind, number);
}

void Idle(void)
{
    if (Testing) {
        PageTake();
        TestIdle();
        Yield(0);
        return;
    }
    MouseFromPage(false);
    PageInput(IDLE_WAIT);
    MouseFromPage(false);
}

/*
 * The keyboard: what the test channel has made due, or the page.
 */

struct Keys KeyWaiting(void)
{
    if (emscripten_get_now() - YieldedAt >= BUSY_YIELD)
        Yield(0);
    if (Testing) {
        PageTake();
        return TestKeyWaiting();
    }
    PageInput(0);
    MouseFromPage(false);
    return KeyboardWaiting();
}

u16 KeyRead(void)
{
    u16 key;

    if (Testing) {
        PageTake();
        key = TestKeyRead();
        Yield(0);
    } else {
        while (KeyboardWaiting().none) {
            MouseFromPage(false);
            PageInput(-1);
        }
        MouseFromPage(false);
        key = KeyboardTake();
    }
    TraceKey(key);
    return key;
}

u16 KeysHeld(void)
{
    u16 held;

    PageTake();
    held = Testing ? TestKeysHeld() : KeyboardHeld();

    TraceHeld(held);
    return held;
}

/* The program waits on the mouse by asking for it in a loop that asks
   nothing else, until a button is let go (Inpt12 of input.c, Select of
   dialog.c, a click on the key bar): so MouseWhere takes what the
   page has queued, as KeyWaiting does, and yields as it does, or the
   page would never go on and the release would never come. */
struct MouseAt MouseWhere(void)
{
    PageTake();
    if (emscripten_get_now() - YieldedAt >= BUSY_YIELD)
        Yield(0);
    if (Testing) {
        TestMouseAsked();
    } else {
        PageInput(0);
    }
    /* Through the page, the mouse is the page's as for a user: what it
       has reported, as the program looks, after the channel has had the
       page give the next event where it is due. */
    if (!Testing || Paged)
        MouseFromPage(true);
    return MouseAnswer();
}

/* The timer: the machine's (MachineTicks), with a yield at the second
   read that has none between, where the program waits for the next tick
   by itself. */
u32 Ticks(void)
{
    if (++TicksRead > 1)
        Yield(0);
    return MachineTicks();
}

/*
 * Other programs, and the end of this one.
 */

/* A command, on the user's word of 15 October 2026 (CLAUDE.md, "The web
   page"): what DOS has shown has the prompt and the command, which the
   program has typed there by the BIOS, and behind it a line that says
   that it cannot be run; DOS's answer is that the program ran, and ended
   with the code 1. The user menu, the extension file and an external
   viewer or editor come here alike. */
static const u8 NoCommand[] = "Commands cannot be run in a web page.\r\n$";

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

/* Nothing on the page is a program. */
const u8 *ProgramOfFile(const u8 *name)
{
    (void)name;
    return 0;
}

/* Enter on a file that is no program: the page opens it in a new tab of
   the browser, where the user may view it or save it. */
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
    web_open_file(host, name);
    answer.failed = false;
    answer.value = 0;
    return answer;
}

/* The end: the grid drawn, then the machine's end. */
void ProgramEnd(u16 code)
{
    Draw();
    MachineEnd(code);
}

/* Built with the sanitizer of addresses, a block refused for want of
   memory is answered as the C library answers it, NULL, and does not
   end the program: the program asks for all that there is, as it asks
   DOS, and takes what it is told is left (MemTake of machine_posix.c).
   And a block given back is given back, not kept from use for a while to
   catch a use of it after it is free, which the sanitizer would do: the
   machine finds how long a block the heap can give by taking it and
   giving it back (Obtainable of machine_posix.c), and a heap with a
   ceiling may have no room for a second such block beside the first. */
#if defined(__has_feature)
#if __has_feature(address_sanitizer)
const char *__asan_default_options(void);
const char *__asan_default_options(void)
{
    return "allocator_may_return_null=1:quarantine_size_mb=0";
}
#endif
#endif

/* The variable of the environment that is `name` and the letter. */
static const char *Variable(const char *name, u8 letter)
{
    char full[32];

    snprintf(full, sizeof full, "%s%c", name, letter);
    return getenv(full);
}

int main(int count, char **arguments)
{
    struct DriveGiven given[DOS_DRIVES];
    i32 drives = 0, i;
    const char *label, *own = getenv(PROGRAM_DRIVE_VARIABLE);

    for (i = 0; i < DOS_DRIVES; i++) {
        given[drives].root = Variable(DRIVE_VARIABLE, Drives[i].letter);
        if (given[drives].root == 0)
            continue;
        label = Variable(LABEL_VARIABLE, Drives[i].letter);
        given[drives].letter = Drives[i].letter;
        given[drives].label = label != 0 && label[0] != 0
            && strlen(label) <= LABEL_LONGEST ? label : Drives[i].label;
        drives++;
    }
    if (drives == 0) {
        fprintf(stderr, "vc: neither %sC nor %sD names a directory for a "
                        "drive\n", DRIVE_VARIABLE, DRIVE_VARIABLE);
        return 1;
    }
    Testing = TestBegin();
    if (Testing && getenv(PAGED_VARIABLE) != 0) {
        Paged = true;
        TestByPage(PageDeliver, PageRecorded);
        KeyboardUnpaced();
    }
    MachineBegin(given, drives,
                 own != 0 && own[0] != 0 ? (u8)(own[0] & ~0x20) : 'C',
                 MEMORY_CAP);
    MachineEnvironment((const u8 *)"COMMAND.COM", count, arguments);
    YieldedAt = emscripten_get_now();
    Main();
    return 0;
}
