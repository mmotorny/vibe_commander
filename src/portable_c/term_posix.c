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
 *     where Unicode's mapping has controls. The 16 colours are the VGA's
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
 *     answers INT 16h with, AH 0: the scan code of a US keyboard above
 *     the character. Printable ASCII and what the code page has of
 *     Unicode, Enter, Tab, Backspace, Esc and the control characters as
 *     Ctrl with a letter; the escape sequences of xterm, and the Linux
 *     console's and rxvt's for F1 to F5 and Home and End, for the
 *     arrows, F1 to F10, Home, End, PgUp, PgDn, Ins and Del, with Shift,
 *     Ctrl and Alt as the BIOS has them; and Alt with a letter, a digit,
 *     - or = as ESC in front of the key, Esc alone being an ESC that
 *     nothing follows within ESC_WAIT milliseconds. What the BIOS's AH 0
 *     does not answer, F11, F12, and the keys that only its AH 10h has,
 *     is dropped, as that call drops them. The full keyboard, by the
 *     kitty protocol where the terminal has it, with Shift, Ctrl and Alt
 *     held (KeysHeld), is the piece L5 (Csi below), and so is the mouse;
 *     Ctrl-O's screen, the terminal's own with what a command wrote, is
 *     L6 (TermGive and TermTake are what it calls around a command).
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
#include <time.h>
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
   shown in the terminal's own shape, rows wrapped again, and the
   terminal's own screen. */
static const char Leave[] = "\033[?2026l\033[0m\033[0 q\033[?25h\033[?7h"
                            "\033[?1049l";
/* What it is taken with: the alternate screen, no wrapping, no cursor
   until a frame places it. */
static const char Enter[] = "\033[?1049h\033[?7l\033[?25l";

static struct termios Saved;            /* as the terminal was */
static struct termios Raw;
static volatile sig_atomic_t Held = 0;  /* while the program has it */
static volatile sig_atomic_t Whole = 1; /* the next frame draws every cell */
static volatile sig_atomic_t Resized = 1;
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

/*
 * Drawing.
 */

#define COLUMNS 80
#define ROWS 25
#define CELLS (COLUMNS * ROWS)

/* What the VGA's font shows for the bytes below 20h, where Unicode's
   mapping of the code page has the controls of ASCII; 0 is a blank. */
static const u16 Glyphs[0x20] = {
    0x0020, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
    0x25D8, 0x25CB, 0x25D9, 0x2642, 0x2640, 0x266A, 0x266B, 0x263C,
    0x25BA, 0x25C4, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8,
    0x2191, 0x2193, 0x2192, 0x2190, 0x221F, 0x2194, 0x25B2, 0x25BC
};
#define HOUSE 0x2302                    /* what the font has at 7Fh */

u32 TermGlyph(u8 byte)
{
    if (byte < 0x20)
        return Glyphs[byte];
    if (byte == 0x7F)
        return HOUSE;
    return CodePageToUnicode(byte);
}

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
    i32 count = Utf8Encode(TermGlyph(byte), bytes);

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
 * The keys.
 */

#define ESC 0x1B
#define ESC_WAIT 25                     /* milliseconds */
#define KEYS_ROOM 16                    /* as the BIOS's buffer */
#define TYPED_ROOM 256

static u16 Buffer[KEYS_ROOM];
static i32 KeysFirst = 0, KeysCount = 0;
static char Typed[TYPED_ROOM];          /* bytes not made keys yet */
static i32 TypedCount = 0;
static int64_t TypedSince = 0;          /* when the first of them came */

static int64_t Now(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/* A key into the buffer; dropped where it is full, as the BIOS beeps and
   drops it, or while the terminal is too small to show the program, or
   where the BIOS's AH 0 has no word for it (0). */
static void Push(u16 key)
{
    if (key == 0 || KeysCount == KEYS_ROOM || TooSmall)
        return;
    Buffer[(KeysFirst + KeysCount) % KEYS_ROOM] = key;
    KeysCount++;
}

/* The scan codes of a US keyboard for printable ASCII, from 20h. */
static const u8 Scan[0x5F] = {
    0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28, /*  !"#$%&' */
    0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35, /* ()*+,-./ */
    0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* 01234567 */
    0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35, /* 89:;<=>? */
    0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, /* @ABCDEFG */
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, /* HIJKLMNO */
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, /* PQRSTUVW */
    0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x07, 0x0C, /* XYZ[\]^_ */
    0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, /* `abcdefg */
    0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18, /* hijklmno */
    0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, /* pqrstuvw */
    0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x29        /* xyz{|}~ */
};

/* A byte that no ESC is in front of. */
static u16 KeyOfByte(u8 byte)
{
    switch (byte) {
    case 0x00: return 0x0300;           /* Ctrl-@, as the BIOS's Ctrl-2 */
    case 0x08: return 0x0E08;           /* Backspace, as some send it */
    case 0x09: return 0x0F09;           /* Tab */
    case 0x0A: return 0x1C0A;           /* Ctrl-Enter */
    case 0x0D: return 0x1C0D;           /* Enter */
    case 0x1C: return 0x2B1C;           /* Ctrl-\ */
    case 0x1D: return 0x1B1D;           /* Ctrl-] */
    case 0x1E: return 0x071E;           /* Ctrl-^ */
    case 0x1F: return 0x0C1F;           /* Ctrl-_ */
    case 0x7F: return 0x0E08;           /* Backspace */
    }
    if (byte < 0x20)                    /* Ctrl with a letter */
        return (u16)(Scan[byte + 0x40 - 0x20] << 8 | byte);
    return (u16)(Scan[byte - 0x20] << 8 | byte);
}

/* Alt with a key, as ESC in front of it: a letter, a digit, - and =,
   which the BIOS's AH 0 answers; 0 for the others. */
static u16 KeyOfAlt(u8 byte)
{
    if (byte >= '1' && byte <= '9')
        return (u16)((0x78 + byte - '1') << 8);
    switch (byte) {
    case '0': return 0x8100;
    case '-': return 0x8200;
    case '=': return 0x8300;
    }
    if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z'))
        return (u16)(Scan[byte - 0x20] << 8);
    return 0;
}

/* The keys that escape sequences name. */
enum Sequence { SEQ_NONE, SEQ_UP, SEQ_DOWN, SEQ_LEFT, SEQ_RIGHT,
             SEQ_HOME, SEQ_END, SEQ_PGUP, SEQ_PGDN, SEQ_INS,
             SEQ_DEL, SEQ_BACKTAB, SEQ_F1 };
/* SEQ_F1 + 9 is F10, and F11 and F12 are SEQ_F1 + 10 and 11. */

/* The BIOS's word of a named key with what is held, xterm's modifier
   less 1: Shift 1, Alt 2, Ctrl 4. */
static u16 KeyOfNamed(i32 named, i32 held)
{
    /* plain, Ctrl: the scan codes; 0 for what AH 0 does not answer */
    static const u8 Moving[SEQ_DEL + 1][2] = {
        { 0, 0 }, { 0x48, 0 }, { 0x50, 0 }, { 0x4B, 0x73 }, { 0x4D, 0x74 },
        { 0x47, 0x77 }, { 0x4F, 0x75 }, { 0x49, 0x84 }, { 0x51, 0x76 },
        { 0x52, 0 }, { 0x53, 0 }
    };
    bool alt = (held & 2) != 0, ctrl = (held & 4) != 0;
    bool shift = (held & 1) != 0;
    i32 f;

    if (named == SEQ_BACKTAB)
        return 0x0F00;
    if (named >= SEQ_F1) {
        f = named - SEQ_F1;
        if (f >= 10)
            return 0;                   /* F11 and F12 */
        if (alt)
            return (u16)((0x68 + f) << 8);
        if (ctrl)
            return (u16)((0x5E + f) << 8);
        if (shift)
            return (u16)((0x54 + f) << 8);
        return (u16)((0x3B + f) << 8);
    }
    if (named == SEQ_NONE || alt)
        return 0;
    return (u16)(Moving[named][ctrl ? 1 : 0] << 8);
}

/* The key of `~` with a number in front, as xterm, rxvt and the Linux
   console have them. */
static i32 Tilde(i32 number)
{
    static const i32 Function[] = { 11, 12, 13, 14, 15, 17, 18, 19, 20, 21,
                                    23, 24 };
    i32 i;

    switch (number) {
    case 1: case 7: return SEQ_HOME;
    case 2: return SEQ_INS;
    case 3: return SEQ_DEL;
    case 4: case 8: return SEQ_END;
    case 5: return SEQ_PGUP;
    case 6: return SEQ_PGDN;
    }
    for (i = 0; i < 12; i++)
        if (Function[i] == number)
            return SEQ_F1 + i;
    return SEQ_NONE;
}

/* The key of a final byte of CSI or SS3. */
static i32 Final(char final)
{
    switch (final) {
    case 'A': return SEQ_UP;
    case 'B': return SEQ_DOWN;
    case 'C': return SEQ_RIGHT;
    case 'D': return SEQ_LEFT;
    case 'H': return SEQ_HOME;
    case 'F': return SEQ_END;
    case 'Z': return SEQ_BACKTAB;
    case 'P': return SEQ_F1;
    case 'Q': return SEQ_F1 + 1;
    case 'R': return SEQ_F1 + 2;
    case 'S': return SEQ_F1 + 3;
    }
    return SEQ_NONE;
}

/* A sequence of CSI at `from` (behind ESC [), of `count` bytes: how many
   bytes it is, its key pushed; 0 where it is not whole yet. A sequence
   that names no key is taken and dropped: the answers of a terminal,
   the mouse, and the kitty protocol's `u`, which L5 is to read here. */
static i32 Csi(const char *from, i32 count)
{
    i32 numbers[2] = { 0, 0 }, have = 0, i;
    i32 named;

    if (count >= 1 && from[0] == '[') {         /* the Linux console */
        if (count < 2)
            return 0;
        if (from[1] >= 'A' && from[1] <= 'E')
            Push(KeyOfNamed(SEQ_F1 + from[1] - 'A', 0));
        return 2;
    }
    for (i = 0; i < count; i++) {
        char c = from[i];

        if (c >= '0' && c <= '9') {
            if (have == 0)
                have = 1;
            if (have <= 2 && numbers[have - 1] < 10000)
                numbers[have - 1] = numbers[have - 1] * 10 + (c - '0');
        } else if (c == ';') {
            have = have == 0 ? 2 : have + 1;
        } else if (c >= 0x40 && c <= 0x7E) {
            named = c == '~' ? Tilde(numbers[0]) : Final(c);
            Push(KeyOfNamed(named, numbers[1] > 1 ? numbers[1] - 1 : 0));
            return i + 1;
        } else if (c < 0x20 || c > 0x3F) {
            return i;                   /* broken: dropped as far */
        }
    }
    return 0;
}

/* A sequence of SS3 at `from` (behind ESC O): F1 to F4, the arrows, Home
   and End, as xterm sends them where its keypad is set so, with a
   modifier's number in front in some. */
static i32 Ss3(const char *from, i32 count)
{
    i32 i, held = 0;

    for (i = 0; i < count; i++) {
        if (from[i] >= '0' && from[i] <= '9') {
            held = held * 10 + (from[i] - '0');
            continue;
        }
        Push(KeyOfNamed(Final(from[i]), held > 1 ? held - 1 : 0));
        return i + 1;
    }
    return 0;
}

/* The bytes that have come, made keys as far as they are whole; `late`
   where what is left has waited ESC_WAIT, so that an ESC alone is Esc. */
static void Decode(bool late)
{
    i32 at = 0, used, character;
    u32 unicode;
    u8 byte;

    while (at < TypedCount) {
        byte = (u8)Typed[at];
        if (byte == ESC) {
            if (at + 1 == TypedCount) {
                if (!late)
                    break;
                Push(0x011B);
                used = 1;
            } else if (Typed[at + 1] == '[') {
                used = Csi(Typed + at + 2, TypedCount - at - 2);
                used = used == 0 ? (late ? 2 : 0) : used + 2;
            } else if (Typed[at + 1] == 'O') {
                used = Ss3(Typed + at + 2, TypedCount - at - 2);
                used = used == 0 ? (late ? 2 : 0) : used + 2;
            } else if (Typed[at + 1] == ESC) {
                Push(0x011B);
                used = 1;
            } else {
                Push(KeyOfAlt((u8)Typed[at + 1]));
                used = 2;
            }
            if (used == 0)
                break;
        } else if (byte < 0x80) {
            Push(KeyOfByte(byte));
            used = 1;
        } else {
            used = Utf8Decode(Typed + at, TypedCount - at, &unicode);
            if (used == 0 && !late && TypedCount - at < 4)
                break;                  /* the rest of it is to come */
            if (used == 0)
                used = 1;
            else {
                character = CodePageFromUnicode(unicode);
                if (character >= 0)
                    Push((u16)character);
            }
        }
        at += used;
    }
    memmove(Typed, Typed + at, (size_t)(TypedCount - at));
    TypedCount -= at;
}

void TermInput(i32 wait)
{
    struct pollfd terminal;
    ssize_t got;
    int64_t left;

    if (!Held)
        return;
    if (TypedCount > 0) {
        left = TypedSince + ESC_WAIT - Now();
        if (left <= 0) {
            Decode(true);
            wait = 0;
        } else if (wait < 0 || wait > left)
            wait = (i32)left;
    }
    if (KeysCount > 0)
        wait = 0;
    terminal.fd = TERM_IN;
    terminal.events = POLLIN;
    terminal.revents = 0;
    if (poll(&terminal, 1, wait) <= 0)
        return;
    if (TypedCount == 0)
        TypedSince = Now();
    got = read(TERM_IN, Typed + TypedCount, (size_t)(TYPED_ROOM - TypedCount));
    if (got == 0) {                     /* the terminal has gone */
        Give();
        exit(1);
    }
    if (got < 0)
        return;
    TypedCount += (i32)got;
    Decode(TypedCount == TYPED_ROOM);
    if (TypedCount == 0)
        TypedSince = 0;
}

struct Keys TermKeyWaiting(void)
{
    struct Keys keys;

    keys.none = KeysCount == 0;
    keys.key = keys.none ? 0 : Buffer[KeysFirst];
    return keys;
}

u16 TermKeyTake(void)
{
    u16 key = Buffer[KeysFirst];

    KeysFirst = (KeysFirst + 1) % KEYS_ROOM;
    KeysCount--;
    return key;
}
