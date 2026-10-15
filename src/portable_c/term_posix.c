/*
 * The terminal of the program for Linux: the piece L4 of step 6 of the
 * portable C's road (CLAUDE.md, "The portable C", the road, step 6;
 * TODO.md, "L4, the terminal"). The program draws its screen of 80 by 25
 * cells in the terminal that it is started in, and takes its keys there,
 * by the terminal's own escape sequences and termios, no library.
 *
 *   - The terminal is taken when the program starts: raw mode by
 *     termios, the alternate screen (?1049h), no wrapping at the end of a
 *     row (?7l), and the cursor drawn by the program. It is given back as
 *     it was when the program ends, by exit, by SIGTERM, SIGHUP, SIGINT or
 *     SIGQUIT, and when it crashes, by SIGSEGV, SIGBUS, SIGFPE, SIGILL or
 *     SIGABRT, or by a sanitizer's end; and while the program is stopped,
 *     by SIGTSTP, to be taken again and drawn whole at SIGCONT.
 *   - The screen is the display's 80 by 25 cells, the character a byte of
 *     the code page 437 and its colour an attribute byte, as DOS's text
 *     mode 3 has them. A character is drawn as the character of Unicode
 *     that the VGA's font shows for it: the code page's mapping
 *     (CodePageToUnicode) above 7Fh and for the letters, and for the
 *     bytes below 20h and for 7Fh the glyphs that the font has there,
 *     where Unicode's mapping has controls (CodePageGlyph). The 16 colours are the VGA's
 *     of mode 3 as its BIOS sets them up: 0 to 7 with each of red, green
 *     and blue at AAh or 0, brown 6 with its green at 55h, and 8 to 15
 *     the same with 55h added to each, which are the DAC's 6-bit values
 *     2Ah and 15h made 8 bits. In 24 bits, where COLORTERM says that the
 *     terminal has them ("truecolor" or "24bit"); else the 16 colours of
 *     ANSI, 30 to 37 and 90 to 97 for the foreground, 40 to 47 for the
 *     background, which are whatever the terminal's theme makes them.
 *     The attribute's bit 7 is blink, SGR 5, with the background of the
 *     three bits below it: the BIOS starts mode 3 with blinking on, and
 *     the program never asks for the bright backgrounds (INT 10h 1003h
 *     is no function of sys.h), so DOS shows such a cell blinking.
 *   - What is drawn is what has changed since the frame before, each run
 *     of changed cells with the cursor placed at its start, so that a
 *     terminal that takes a character for wider than a cell loses at most
 *     that run; and every frame is one update of synchronized output,
 *     mode 2026, which a terminal that does not have it ignores, as it
 *     ignores every mode of DEC's that it does not know.
 *   - The cursor is the BIOS's: hidden where its shape has bit 5 of its
 *     first line set, or begins below where it ends, or where it is
 *     placed off the 80 by 25; a block where it is more than three lines
 *     tall, an underline otherwise; blinking, as a VGA's is (DECSCUSR 1
 *     or 3).
 *   - The screen is 80 columns by 25 rows and no other size, on the
 *     user's word of 14 October 2026 (CLAUDE.md, step 6). A terminal that
 *     is larger has it at its top left, and the rest of it blank in the
 *     terminal's own colours; one that is smaller shows a line that says
 *     how large it has to be, until it is (SIGWINCH), and the keys that
 *     are typed meanwhile are dropped: the program is not to be worked
 *     blind.
 *   - The keys are bytes of standard input, made the words that the BIOS
 *     of the test bench answers INT 16h with, AH 0: the scan code of a US
 *     keyboard above the character. Where the terminal has the kitty
 *     protocol of the keyboard, every key comes as an escape code, with
 *     what is held and when it is let go, so that Shift, Ctrl and Alt are
 *     known while held (KeysHeld) and Alt alone is a key; without it the
 *     keys are read as terminals have sent them since xterm: printable
 *     ASCII and what the code page has of Unicode, the control characters
 *     as Ctrl with a letter, the escape sequences of xterm, rxvt and the
 *     Linux console, and Alt with a key as ESC in front of it, Esc alone
 *     being an ESC that nothing follows within ESC_WAIT milliseconds.
 *     The bytes are read here and made keys by keyboard_posix.c, a file
 *     of its own since the piece W1 of step 7, whose part "The keys, and
 *     the mouse" has the rest.
 *   - The mouse is the terminal's reports in SGR's form, of every motion
 *     too, the cell and the buttons; machine_posix.c makes of them what a
 *     mouse driver of DOS answers, and draws its pointer in the cells.
 *   - A command (L6) has the terminal given back, its own screen and its
 *     modes, and handed to the command's process group (TermHandTo), so
 *     that Ctrl-C and Ctrl-Z are the command's; the program writes the
 *     prompt and the command there first (TermWrite). Ctrl-O shows that
 *     screen until a key (TermOwnScreen).
 *
 * What is async-signal-safe is said at each function that a handler of a
 * signal calls: those use write, tcsetattr, sigaction, sigprocmask and
 * raise alone, which POSIX names so.
 */
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#include <sanitizer/common_interface_defs.h>
#define SANITIZED 1
#endif
#endif

#include "emulate_posix.h"

#define TERM_IN 0
#define TERM_OUT 1

/*
 * Taking the terminal and giving it back.
 */

/* What the terminal is given back with: the attributes reset, the cursor
   shown in the terminal's own shape, rows wrapped again, the keypad in
   its normal mode, the mouse's
   reports and those of the focus off, and the terminal's own screen. The
   kitty protocol's flags are popped first where they were pushed (Pop):
   the alternate screen has a stack of its own in that protocol. */
static const char Leave[] = "\033[?2026l\033[0m\033[0 q\033[?25h\033[?7h"
                            "\033>\033[?1004l\033[?1006l\033[?1003l\033[?1002l"
                            "\033[?1000l\033[?1049l";
static const char Pop[] = "\033[<u";
/* What it is taken with: the alternate screen, no wrapping, no cursor
   until a frame places it; the keypad in xterm's application mode
   (DECKPAM), so that a terminal without the kitty protocol sends its
   signs and Enter by SS3 and not as the main keyboard's (Ss3); the mouse reported in SGR's form, with every
   motion, and the focus reported; and the question whether the terminal
   has the kitty protocol of the keyboard, whose answer pushes its flags
   (Answered, below). */
static const char Enter[] = "\033[?1049h\033[?7l\033[?25l\033=\033[?1000h"
                            "\033[?1002h\033[?1003h\033[?1006h\033[?1004h"
                            "\033[?u";
/* The kitty protocol's flags that the program asks for: 1 keys told
   apart from what they once shared, Esc from Alt, Ctrl-I from Tab; 2
   presses, repeats and releases, so that Shift, Ctrl and Alt are known
   while held; 4 the key of a US layout behind the key, which Alt with a
   letter goes by as the BIOS's scan code; 8 every key an escape code, the
   modifiers alone too; 16 the text that a key types. */
static const char PushFlags[] = "\033[>31u";

static struct termios Saved;            /* as the terminal was */
static struct termios Raw;
static volatile sig_atomic_t Held = 0;  /* while the program has it */
static volatile sig_atomic_t Whole = 1; /* the next frame draws every cell */
static volatile sig_atomic_t Resized = 1;
static volatile sig_atomic_t Pushed = 0; /* the kitty protocol's flags */
static bool Colours24 = false;          /* where COLORTERM says so */

/* Write all of it, as far as the terminal takes it. Async-signal-safe. */
static void Put(const char *from, size_t count)
{
    ssize_t written;

    while (count > 0) {
        written = write(TERM_OUT, from, count);
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return;
        from += written;
        count -= (size_t)written;
    }
}

/* Give the terminal back as it was. Async-signal-safe. */
static void Give(void)
{
    int error = errno;

    if (Held) {
        Held = 0;
        if (Pushed) {
            Pushed = 0;
            Put(Pop, sizeof Pop - 1);
        }
        Put(Leave, sizeof Leave - 1);
        tcsetattr(TERM_IN, TCSADRAIN, &Saved);
    }
    errno = error;
}

/* Take it again, as when the program started. Async-signal-safe. */
static void Take(void)
{
    int error = errno;

    if (!Held) {
        tcsetattr(TERM_IN, TCSADRAIN, &Raw);
        Put(Enter, sizeof Enter - 1);
        Held = 1;
        Whole = 1;
        Resized = 1;
    }
    errno = error;
}

static void Ended(void)
{
    Give();
}

/* The signals that end the program, each with what it did before: the
   terminal is given back, and the signal goes on as it would have. */
static const int Ending[] = { SIGTERM, SIGHUP, SIGINT, SIGQUIT, SIGSEGV,
                              SIGBUS, SIGFPE, SIGILL, SIGABRT };
#define ENDING_COUNT ((i32)(sizeof Ending / sizeof Ending[0]))

static struct sigaction Before[ENDING_COUNT];

static void OnEnd(int number)
{
    i32 i;

    Give();
    for (i = 0; i < ENDING_COUNT; i++)
        if (Ending[i] == number)
            sigaction(number, &Before[i], 0);
    raise(number);
}

/* Stopped: the terminal given back while the program is, and taken again
   when it goes on. */
static void OnStop(int number)
{
    struct sigaction stop, own;
    sigset_t unblocked;

    (void)number;
    Give();
    memset(&stop, 0, sizeof stop);
    stop.sa_handler = SIG_DFL;
    sigemptyset(&stop.sa_mask);
    sigaction(SIGTSTP, &stop, &own);
    sigemptyset(&unblocked);
    sigaddset(&unblocked, SIGTSTP);
    sigprocmask(SIG_UNBLOCK, &unblocked, 0);
    raise(SIGTSTP);
    sigaction(SIGTSTP, &own, 0);
    Take();
}

/* Gone on, after a stop of any kind: the shell may have set the terminal
   otherwise, so it is taken anew and drawn whole. */
static void OnGo(int number)
{
    (void)number;
    if (Held)
        tcsetattr(TERM_IN, TCSADRAIN, &Raw);
    Whole = 1;
    Resized = 1;
}

static void OnSize(int number)
{
    (void)number;
    Resized = 1;
}

static void Handle(int number, void (*handler)(int))
{
    struct sigaction action;

    memset(&action, 0, sizeof action);
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    sigaction(number, &action, 0);
}

bool TermBegin(void)
{
    const char *colours;
    i32 i;

    if (!isatty(TERM_IN) || !isatty(TERM_OUT)
        || tcgetattr(TERM_IN, &Saved) != 0)
        return false;
    colours = getenv("COLORTERM");
    Colours24 = colours != 0 && (strcmp(colours, "truecolor") == 0
                                 || strcmp(colours, "24bit") == 0);
    Raw = Saved;
    Raw.c_iflag &= ~(tcflag_t)(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
                               | IGNCR | ICRNL | IXON);
    Raw.c_oflag &= ~(tcflag_t)OPOST;
    Raw.c_lflag &= ~(tcflag_t)(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    Raw.c_cflag &= ~(tcflag_t)(CSIZE | PARENB);
    Raw.c_cflag |= CS8;
    Raw.c_cc[VMIN] = 1;
    Raw.c_cc[VTIME] = 0;
    /* A signal that the program was started with ignored, as nohup has
       SIGHUP, stays so. */
    for (i = 0; i < ENDING_COUNT; i++)
        if (sigaction(Ending[i], 0, &Before[i]) == 0
            && Before[i].sa_handler != SIG_IGN)
            Handle(Ending[i], OnEnd);
    Handle(SIGTSTP, OnStop);
    Handle(SIGCONT, OnGo);
    Handle(SIGWINCH, OnSize);
    atexit(Ended);
#ifdef SANITIZED
    /* A sanitizer ends the program by _exit, past atexit: its report is
       written on the alternate screen, which goes with it. */
    __sanitizer_set_death_callback(Ended);
#endif
    Take();
    return true;
}

void TermGive(void)
{
    Give();
}

void TermTake(void)
{
    Take();
}

void TermWrite(const char *text, i32 length)
{
    if (length > 0)
        Put(text, (size_t)length);
}

/* tcsetpgrp from a group that is not the terminal's foreground one has
   the kernel send it SIGTTOU, which stops it: blocked around the call,
   as a shell does. */
static void Foreground(pid_t group)
{
    sigset_t ttou, before;

    sigemptyset(&ttou);
    sigaddset(&ttou, SIGTTOU);
    sigprocmask(SIG_BLOCK, &ttou, &before);
    while (tcsetpgrp(TERM_IN, group) != 0 && errno == EINTR)
        ;
    sigprocmask(SIG_SETMASK, &before, 0);
}

void TermHandTo(pid_t group)
{
    Foreground(group);
}

void TermHandBack(void)
{
    Foreground(getpgrp());
}

/* Ctrl-O: the terminal given back as for a command, but for its modes,
   which stay raw so that a key is a key and is not echoed; the first
   that comes, and what comes with it within OWN_WAIT milliseconds, an
   escape sequence, is dropped. */
#define OWN_WAIT 50

void TermOwnScreen(void)
{
    char bytes[64];
    struct pollfd input;
    ssize_t count;
    int ready;

    Give();
    tcsetattr(TERM_IN, TCSADRAIN, &Raw);
    input.fd = TERM_IN;
    input.events = POLLIN;
    do
        count = read(TERM_IN, bytes, sizeof bytes);
    while (count < 0 && errno == EINTR);
    do {
        input.revents = 0;
        ready = poll(&input, 1, OWN_WAIT);
        if (ready > 0 && read(TERM_IN, bytes, sizeof bytes) <= 0)
            break;
    } while (ready > 0 || (ready < 0 && errno == EINTR));
    Take();
}

/*
 * Drawing.
 */

#define COLUMNS 80
#define ROWS 25
#define CELLS (COLUMNS * ROWS)

/* The VGA's 16 colours of mode 3, red, green and blue. */
static const u8 Palette[16][3] = {
    { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xAA }, { 0x00, 0xAA, 0x00 },
    { 0x00, 0xAA, 0xAA }, { 0xAA, 0x00, 0x00 }, { 0xAA, 0x00, 0xAA },
    { 0xAA, 0x55, 0x00 }, { 0xAA, 0xAA, 0xAA }, { 0x55, 0x55, 0x55 },
    { 0x55, 0x55, 0xFF }, { 0x55, 0xFF, 0x55 }, { 0x55, 0xFF, 0xFF },
    { 0xFF, 0x55, 0x55 }, { 0xFF, 0x55, 0xFF }, { 0xFF, 0xFF, 0x55 },
    { 0xFF, 0xFF, 0xFF }
};

/* ANSI's colour of each of the PC's 0 to 7: the PC has blue in bit 0
   and red in bit 2, ANSI the other way round. */
static const u8 Ansi[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };

static u16 Shown[CELLS];
static i32 ShownCursor = -1;            /* the place, or -1 for hidden */
static i32 ShownShape = -1;             /* DECSCUSR's number */
static bool TooSmall = false;

/* A frame is made here and written at once. */
#define FRAME_ROOM 0x10000
static char Frame[FRAME_ROOM];
static size_t FrameLength = 0;

static void Flush(void)
{
    Put(Frame, FrameLength);
    FrameLength = 0;
}

static void Add(const char *text, size_t count)
{
    if (FrameLength + count > FRAME_ROOM)
        Flush();
    memcpy(Frame + FrameLength, text, count);
    FrameLength += count;
}

static void AddText(const char *text)
{
    Add(text, strlen(text));
}

static void AddNumber(i32 number)
{
    char digits[12];
    int length = snprintf(digits, sizeof digits, "%d", number);

    Add(digits, (size_t)length);
}

/* The cursor at a row and a column, from 0. */
static void AddPlace(i32 row, i32 column)
{
    AddText("\033[");
    AddNumber(row + 1);
    AddText(";");
    AddNumber(column + 1);
    AddText("H");
}

static void AddRgb(const char *kind, u8 colour)
{
    AddText(kind);
    AddNumber(Palette[colour][0]);
    AddText(";");
    AddNumber(Palette[colour][1]);
    AddText(";");
    AddNumber(Palette[colour][2]);
}

/* The SGR of an attribute byte, from nothing. */
static void AddColour(u8 attribute)
{
    u8 fore = attribute & 0x0F, back = attribute >> 4 & 0x07;

    AddText("\033[0");
    if (Colours24) {
        AddRgb(";38;2;", fore);
        AddRgb(";48;2;", back);
    } else {
        AddText(fore < 8 ? ";3" : ";9");
        AddNumber(Ansi[fore & 7]);
        AddText(";4");
        AddNumber(Ansi[back]);
    }
    if ((attribute & 0x80) != 0)
        AddText(";5");
    AddText("m");
}

static void AddCharacter(u8 byte)
{
    char bytes[4];
    i32 count = Utf8Encode(CodePageGlyph(byte), bytes);

    Add(bytes, (size_t)count);
}

/* Whether the terminal holds 80 by 25 now: asked when it has said that
   its size has changed, and at the start. */
static void Measure(void)
{
    struct winsize size;
    bool small = false;

    if (!Resized)
        return;
    Resized = 0;
    if (ioctl(TERM_OUT, TIOCGWINSZ, &size) == 0 && size.ws_col > 0
        && size.ws_row > 0)
        small = size.ws_col < COLUMNS || size.ws_row < ROWS;
    if (small) {
        FrameLength = 0;
        AddText("\033[?2026h\033[0m\033[2J\033[?25l\033[1;1H"
                "Vibe Commander needs a terminal\033[2;1H"
                "of 80 columns by 25 rows;\033[3;1H"
                "this one has ");
        AddNumber(size.ws_col);
        AddText(" by ");
        AddNumber(size.ws_row);
        AddText(".\033[?2026l");
        Flush();
    }
    if (small || TooSmall)
        Whole = 1;
    TooSmall = small;
    KeyboardRefuse(small);
}

/* The shape of the cursor as DECSCUSR has it, 1 a blinking block and 3 a
   blinking underline, or 0 for none. */
static i32 ShapeOf(u16 shape)
{
    i32 first = shape >> 8 & 0x1F, last = shape & 0x1F;

    if ((shape & 0x2000) != 0 || first > last)
        return 0;
    return last - first > 3 ? 1 : 3;
}

/* Begin a frame, once there is something to draw: synchronized output,
   and the cursor hidden while the cells are drawn, for a terminal that
   shows each step. */
static void Begin(bool *begun)
{
    if (*begun)
        return;
    *begun = true;
    FrameLength = 0;
    AddText("\033[?2026h");
    if (ShownCursor >= 0)
        AddText("\033[?25l");
}

void TermDraw(const u16 *cells, u16 place, u16 shape)
{
    i32 row, column, at, cursor, scusr;
    i32 colour = -1;
    bool whole, run, begun = false, drew = false;

    if (!Held)
        return;
    Measure();
    if (TooSmall)
        return;
    whole = Whole != 0;
    Whole = 0;
    if (whole) {
        Begin(&begun);
        AddText("\033[0m\033[2J");
        ShownCursor = -1;
        ShownShape = -1;
    }
    for (row = 0; row < ROWS; row++) {
        run = false;
        for (column = 0; column < COLUMNS; column++) {
            at = row * COLUMNS + column;
            if (!whole && cells[at] == Shown[at]) {
                run = false;
                continue;
            }
            Begin(&begun);
            drew = true;
            if (!run)
                AddPlace(row, column);
            run = true;
            if ((cells[at] >> 8) != colour) {
                colour = cells[at] >> 8;
                AddColour((u8)colour);
            }
            AddCharacter((u8)cells[at]);
            Shown[at] = cells[at];
        }
    }
    scusr = ShapeOf(shape);
    cursor = (place >> 8) < ROWS && (place & 0xFF) < COLUMNS && scusr != 0
             ? (place >> 8) * COLUMNS + (place & 0xFF) : -1;
    if (cursor >= 0 && (drew || cursor != ShownCursor || scusr != ShownShape)) {
        Begin(&begun);
        AddPlace(cursor / COLUMNS, cursor % COLUMNS);
        if (scusr != ShownShape) {
            AddText("\033[");
            AddNumber(scusr);
            AddText(" q");
            ShownShape = scusr;
        }
        AddText("\033[?25h");
    } else if (cursor < 0 && ShownCursor >= 0)
        Begin(&begun);                  /* which hides it */
    ShownCursor = cursor;
    if (!begun)
        return;
    AddText("\033[?2026l");
    Flush();
}

/*
 * The keys, and the mouse: keyboard_posix.c, which makes words of the
 * BIOS of the bytes that are read here.
 */

/* The terminal's answer to the question for the kitty protocol, where
   it has come, followed by the flags that the program asks for, pushed
   where they are not. */
static void Answered(void)
{
    if (KeyboardKittyAnswered() && !Pushed) {
        Put(PushFlags, sizeof PushFlags - 1);
        Pushed = 1;
    }
}


void TermInput(i32 wait)
{
    struct pollfd terminal;
    ssize_t got;
    char *room;
    i32 space;

    if (!Held)
        return;
    if (!KeyboardBefore(&wait)) {
        Answered();
        return;                         /* what waits fills the room */
    }
    Answered();
    terminal.fd = TERM_IN;
    terminal.events = POLLIN;
    terminal.revents = 0;
    if (poll(&terminal, 1, wait) <= 0)
        return;
    space = KeyboardRoom(&room);
    got = read(TERM_IN, room, (size_t)space);
    if (got == 0) {                     /* the terminal has gone */
        Give();
        exit(1);
    }
    if (got < 0)
        return;
    KeyboardAfter((i32)got);
    Answered();
}

