/*
 * The keys and the mouse of the program for Linux, as a terminal sends
 * them: the decoder of the bytes that come, made the words of the BIOS,
 * with what is held and the mouse's reports. It was a part of
 * term_posix.c, the piece L4 of step 6 (DIARY.md, "Write down the
 * terminal of the program for Linux, and what L5 should know"), and is
 * a file of its own since the piece W1 of step 7 (TODO.md, "Step 7, the
 * web page"), so that a build that has no terminal, the web page's, can
 * hand it the keys in the same form, the kitty protocol's, and have
 * them made the same words.
 *
 * It reads and writes nothing of the host: the bytes are handed in
 * (KeyboardRoom, KeyboardAfter), and what it answers is asked of it
 * (KeyboardWaiting and the rest of emulate_posix.h). The one thing that
 * it has the terminal do, push the kitty protocol's flags when the
 * terminal has said that it has the protocol, is said by
 * KeyboardKittyAnswered, which term_posix.c asks.
 */
#include <string.h>
#include <time.h>

#include "emulate_posix.h"

/*
 * The keys, and the mouse.
 *
 * A key is the word that the bench's BIOS answers INT 16h 00h with, the
 * scan code of a US keyboard above the character. The bench's BIOS is
 * SeaBIOS, which hands the keys of the enhanced keyboard on through 00h
 * as well, F11, F12, Ctrl-Up, Alt with an arrow or a sign and the like,
 * where an IBM BIOS drops them: what it answers was read on the bench on
 * 15 October 2026 by harness/trial32/keys.c with 00h in place of 10h,
 * and is in the tables below. What it answers nothing for, Ctrl-1 or
 * Alt-Del, is dropped here too. It makes nothing of Alt with the digits
 * of the keypad, and never sets the bit of Ins at 40:17h.
 *
 * Where the terminal has the kitty protocol of the keyboard, it says so
 * when asked (Enter), the program pushes its flags (PushFlags), and every
 * key comes as an escape code with its modifiers, its press, repeat or
 * release, the key of a US layout behind it and the text that it types:
 * Shift, Ctrl and Alt are then known while they are held, KeysHeld, and
 * a tap on Alt alone is the menu bar, as on DOS. Without it the keys are
 * read from what terminals have sent since xterm, and nothing is known
 * to be held.
 *
 * A change of what is held, or of the mouse's buttons, waits until the
 * program has looked at the one before (KeysHeld, MouseWhere), or for
 * CHANGE_WAIT at the most: a press and a release that come together, a
 * tap or a click over a slow line, are seen by the program as two, as a
 * hand's are on DOS. And a change of the buttons stays for BUTTON_HOLD
 * at the least, looked at or not: the program looks at the mouse once to
 * know that a button is down and again to know where, and since
 * MouseWhere reads what has come, a press would be let go between the
 * two, where a hand holds it longer.
 */

#define ESC 0x1B
#define ESC_WAIT 25                     /* milliseconds */
#define CHANGE_WAIT 100                 /* milliseconds */
#define BUTTON_HOLD 50                  /* milliseconds, under CHANGE_WAIT */
#define KEYS_ROOM 16                    /* as the BIOS's buffer */
#define TYPED_ROOM 256

static u16 Buffer[KEYS_ROOM];
static i32 KeysFirst = 0, KeysCount = 0;
static char Typed[TYPED_ROOM];          /* bytes not made keys yet */
static i32 TypedCount = 0;
static int64_t TypedSince = 0;          /* when the first of them came */

/* What is held, as the BIOS keeps it at 40:17h: the right Shift 01h,
   the left one 02h, Ctrl 04h, Alt 08h, Num Lock 20h and Caps Lock 40h.
   Known only by the kitty protocol. */
static u16 HeldNow = 0;
static bool Kitty = false;
static bool KittyAnswered = false;      /* since KeyboardKittyAnswered */
static bool Refused = false;            /* keys dropped, KeyboardRefuse */

/* The mouse as the terminal last reported it: the cell, from 0, the
   buttons as a mouse driver has them, and how many reports have come. */
static struct ReportedMouse Pointer = { 0, 0, 0, 0 };

/* The order of what comes, as the program is to see it: a change of
   what is held, or of the buttons, is seen by the program before
   anything behind it is taken (HeldSeen, ButtonSeen), and is not made
   while a key is in the buffer, so that the key is read with what was
   held when it was pressed. Each waits CHANGE_WAIT at the most, for a
   program that does not look. */
static bool HeldSeen = true, ButtonSeen = true;
static int64_t ChangedAt = 0, PushedAt = 0, ButtonAt = -BUTTON_HOLD;

static int64_t Now(void)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

/* Whether what comes behind the last change may be taken. */
static bool Settled(void)
{
    int64_t now = Now();

    return (HeldSeen && ButtonSeen && now - ButtonAt >= BUTTON_HOLD)
           || now - ChangedAt >= CHANGE_WAIT;
}

/* Whether a change may be made now. */
static bool MayChange(void)
{
    return KeysCount == 0 || Now() - PushedAt >= CHANGE_WAIT;
}

/* A key into the buffer; dropped where it is full, as the BIOS beeps and
   drops it, or while the screen cannot be shown (KeyboardRefuse), or
   where the bench's BIOS has no word for it (0). */
static void Push(u16 key)
{
    if (key == 0 || KeysCount == KEYS_ROOM || Refused)
        return;
    Buffer[(KeysFirst + KeysCount) % KEYS_ROOM] = key;
    KeysCount++;
    PushedAt = Now();
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

/* The scan code of a character of ASCII, 0 for any other. */
static u16 ScanOf(u32 character)
{
    if (character < 0x20 || character > 0x7E)
        return 0;
    return Scan[character - 0x20];
}

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

/* Alt with the key of a character of a US layout: a letter, a digit, -
   and =, which have words of their own, and the signs and the space,
   which have their scan codes; 0 for the others. */
static u16 KeyOfAlt(u32 key)
{
    if (key >= 'A' && key <= 'Z')
        key += (u32)('a' - 'A');
    if (key >= '1' && key <= '9')
        return (u16)((0x78 + key - '1') << 8);
    switch (key) {
    case '0': return 0x8100;
    case '-': return 0x8200;
    case '=': return 0x8300;
    case ' ': return 0x3920;
    case '[': case ']': case ';': case '\'': case '`': case '\\':
    case ',': case '.': case '/':
        return (u16)(ScanOf(key) << 8);
    }
    if (key >= 'a' && key <= 'z')
        return (u16)(ScanOf(key) << 8);
    return 0;
}

/* Ctrl with the key of a character of a US layout. */
static u16 KeyOfCtrl(u32 key)
{
    if (key >= 'A' && key <= 'Z')
        key += (u32)('a' - 'A');
    if (key >= 'a' && key <= 'z')
        return (u16)((u32)ScanOf(key) << 8 | (key - 0x60));
    switch (key) {
    case '2': return 0x0300;
    case '6': return 0x071E;
    case '-': return 0x0C1F;
    case '[': return 0x1A1B;
    case ']': return 0x1B1D;
    case '\\': return 0x2B1C;
    case ' ': return 0x3920;
    }
    return 0;
}

/* The keys that escape sequences name, in the order of Named. */
enum Sequence { SEQ_NONE, SEQ_UP, SEQ_DOWN, SEQ_LEFT, SEQ_RIGHT,
             SEQ_HOME, SEQ_END, SEQ_PGUP, SEQ_PGDN, SEQ_INS,
             SEQ_DEL, SEQ_BACKTAB, SEQ_BEGIN, SEQ_ESC, SEQ_ENTER,
             SEQ_TAB, SEQ_BACKSPACE, SEQ_F1, SEQ_COUNT = SEQ_F1 + 12 };
/* SEQ_F1 + 9 is F10, and F11 and F12 are SEQ_F1 + 10 and 11. */

/* The column of Named for what is held, as the BIOS goes by it: Alt
   before Ctrl, and Ctrl before Shift. `held` is xterm's and kitty's
   modifier less 1: Shift 1, Alt 2, Ctrl 4. */
static i32 Column(i32 held)
{
    return (held & 2) != 0 ? 3 : (held & 4) != 0 ? 2 : (held & 1) != 0;
}

/* The bench's BIOS's words for the named keys: alone, with Shift, with
   Ctrl and with Alt. The cursor's keys are the gray block's, whose
   character E0h 00h makes 0; SEQ_BEGIN is the keypad's 5 with Num Lock
   off. */
static const u16 Named[SEQ_COUNT][4] = {
    { 0, 0, 0, 0 },
    { 0x4800, 0x4800, 0x8D00, 0x9800 },         /* Up */
    { 0x5000, 0x5000, 0x9100, 0xA000 },         /* Down */
    { 0x4B00, 0x4B00, 0x7300, 0x9B00 },         /* Left */
    { 0x4D00, 0x4D00, 0x7400, 0x9D00 },         /* Right */
    { 0x4700, 0x4700, 0x7700, 0x9700 },         /* Home */
    { 0x4F00, 0x4F00, 0x7500, 0x9F00 },         /* End */
    { 0x4900, 0x4900, 0x8400, 0x9900 },         /* PgUp */
    { 0x5100, 0x5100, 0x7600, 0xA100 },         /* PgDn */
    { 0x5200, 0x5200, 0x9200, 0xA200 },         /* Ins */
    { 0x5300, 0x5300, 0x9300, 0 },              /* Del */
    { 0x0F00, 0x0F00, 0x9400, 0xA500 },         /* Shift-Tab */
    { 0x4C00, 0x4C35, 0x8F00, 0 },              /* the keypad's 5 */
    { 0x011B, 0x011B, 0x011B, 0x0100 },         /* Esc */
    { 0x1C0D, 0x1C0D, 0x1C0A, 0x1C00 },         /* Enter */
    { 0x0F09, 0x0F00, 0x9400, 0xA500 },         /* Tab */
    { 0x0E08, 0x0E08, 0x0E7F, 0x0E00 },         /* Backspace */
    { 0x3B00, 0x5400, 0x5E00, 0x6800 },         /* F1 */
    { 0x3C00, 0x5500, 0x5F00, 0x6900 },
    { 0x3D00, 0x5600, 0x6000, 0x6A00 },
    { 0x3E00, 0x5700, 0x6100, 0x6B00 },
    { 0x3F00, 0x5800, 0x6200, 0x6C00 },
    { 0x4000, 0x5900, 0x6300, 0x6D00 },
    { 0x4100, 0x5A00, 0x6400, 0x6E00 },
    { 0x4200, 0x5B00, 0x6500, 0x6F00 },
    { 0x4300, 0x5C00, 0x6600, 0x7000 },
    { 0x4400, 0x5D00, 0x6700, 0x7100 },         /* F10 */
    { 0x8500, 0x8700, 0x8900, 0x8B00 },         /* F11 */
    { 0x8600, 0x8800, 0x8A00, 0x8C00 }          /* F12 */
};

static u16 KeyOfNamed(i32 named, i32 held)
{
    if (named <= SEQ_NONE || named >= SEQ_COUNT)
        return 0;
    return Named[named][Column(held)];
}

/* The keypad by kitty's codes, from KP_0 at 57399 to KP_BEGIN at 57427:
   KP_0 to KP_9 and KP_DECIMAL are what it reports with Num Lock on, and
   KP_LEFT to KP_BEGIN with it off. The scan code of each key, and the
   character that it types with Num Lock on. */
#define KP_FIRST 57399
#define KP_LAST 57427
static const u8 PadScan[KP_LAST - KP_FIRST + 1] = {
    0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49, /* 0-9 */
    0x53, 0x35, 0x37, 0x4A, 0x4E, 0x1C, 0, 0,   /* . / * - + Enter = , */
    0x4B, 0x4D, 0x48, 0x50, 0x49, 0x51, 0x47, 0x4F, 0x52, 0x53, 0x4C
};
static const char PadText[] = "0123456789.";

/* Ctrl with a key of the keypad's figures, by its scan code. */
static u16 KeyOfPadCtrl(u16 scan)
{
    switch (scan) {
    case 0x47: return 0x7700;
    case 0x48: return 0x8D00;
    case 0x49: return 0x8400;
    case 0x4B: return 0x7300;
    case 0x4C: return 0x8F00;
    case 0x4D: return 0x7400;
    case 0x4F: return 0x7500;
    case 0x50: return 0x9100;
    case 0x51: return 0x7600;
    case 0x52: return 0x9200;
    case 0x53: return 0x9300;
    }
    return 0;
}

/* A key of the keypad, as the bench's BIOS answers it: Shift turns Num
   Lock round, as there; Alt with a figure is nothing. */
static u16 KeyOfPad(i32 code, i32 held)
{
    static const u16 Signs[5][4] = {
        { 0x352F, 0x352F, 0x9500, 0xA400 },     /* / */
        { 0x372A, 0x372A, 0x9600, 0x3700 },     /* * */
        { 0x4A2D, 0x4A2D, 0x8E00, 0x4A00 },     /* - */
        { 0x4E2B, 0x4E2B, 0x9000, 0x4E00 },     /* + */
        { 0x1C0D, 0x1C0D, 0x1C0A, 0xA600 }      /* Enter */
    };
    i32 at = code - KP_FIRST, column = Column(held);
    u16 scan = PadScan[at];
    bool figure;

    if (at >= 11 && at <= 15)
        return Signs[at - 11][column];
    if (scan == 0 || column == 3)
        return 0;
    if (column == 2)
        return KeyOfPadCtrl(scan);
    figure = at <= 10;
    if (column == 1)
        figure = !figure;
    if (!figure)
        return (u16)(scan << 8);
    for (at = 0; at <= 10; at++)
        if (PadScan[at] == scan)
            return (u16)(scan << 8 | (u8)PadText[at]);
    return 0;
}

/* The key of `~` with a number in front, as xterm, rxvt, kitty and the
   Linux console have them. */
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
    case 'E': return SEQ_BEGIN;
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

/* kitty's codes of the keys that are no character. */
#define KITTY_PRIVATE 57344             /* the first of them */
#define CAPS_LOCK 57358
#define NUM_LOCK 57360
#define LEFT_SHIFT 57441
#define LEFT_CONTROL 57442
#define LEFT_ALT 57443
#define RIGHT_SHIFT 57447
#define RIGHT_CONTROL 57448
#define RIGHT_ALT 57449
#define LAST_MODIFIER 57454             /* ISO_LEVEL5_SHIFT */
#define RELEASED 3                      /* kitty's event type */

/* What is held by a key event of kitty's: `held` its modifiers less 1,
   which are the state after the event; Shift, Ctrl and Alt by their own
   presses and releases as well, the left Shift told from the right one
   by its key. False where the change has to wait for the program to have
   seen the one before. */
static bool Hold(i32 code, i32 held, i32 event)
{
    u16 now = (u16)(HeldNow & 0x03);
    bool down = event != RELEASED;

    if ((held & 1) == 0)
        now = 0;
    else if (now == 0)
        now = 0x02;
    if ((held & 2) != 0)
        now |= 0x08;
    if ((held & 4) != 0)
        now |= 0x04;
    if ((held & 64) != 0)
        now |= 0x40;
    if ((held & 128) != 0)
        now |= 0x20;
    switch (code) {
    case LEFT_SHIFT:
        now = (u16)(down ? now | 0x02 : now & ~0x02);
        break;
    case RIGHT_SHIFT:
        now = (u16)(down ? now | 0x01 : now & ~0x01);
        break;
    case LEFT_CONTROL: case RIGHT_CONTROL:
        now = (u16)(down ? now | 0x04 : now & ~0x04);
        break;
    case LEFT_ALT: case RIGHT_ALT:
        now = (u16)(down ? now | 0x08 : now & ~0x08);
        break;
    }
    if (now == HeldNow)
        return true;
    if (!MayChange())
        return false;
    HeldNow = now;
    HeldSeen = false;
    ChangedAt = Now();
    return true;
}

/* A key event of the kitty protocol, CSI code:shifted:base ; held+1:event
   ; text u. False where it has to wait. */
static bool KittyKey(const i32 *key, const bool *given, i32 held, i32 event,
                     i32 text)
{
    i32 code = key[0], character;
    u32 us = given[2] ? (u32)key[2] : (u32)code;
    i32 column = Column(held);

    if (!Hold(code, held, event))
        return false;
    if (event == RELEASED || (code >= CAPS_LOCK && code <= NUM_LOCK)
        || (code >= LEFT_SHIFT && code <= LAST_MODIFIER))
        return true;
    switch (code) {
    case 27: Push(KeyOfNamed(SEQ_ESC, held)); return true;
    case 13: Push(KeyOfNamed(SEQ_ENTER, held)); return true;
    case 9: Push(KeyOfNamed(SEQ_TAB, held)); return true;
    case 8: case 127: Push(KeyOfNamed(SEQ_BACKSPACE, held)); return true;
    }
    if (code >= KP_FIRST && code <= KP_LAST) {
        Push(KeyOfPad(code, held));
        return true;
    }
    if (code < 0x20 || code >= KITTY_PRIVATE)
        return true;                    /* what the BIOS has no word for */
    if (column == 3) {
        Push(KeyOfAlt(us));
        return true;
    }
    if (column == 2) {
        Push(KeyOfCtrl(us));
        return true;
    }
    character = text > 0 ? text : column == 1 && given[1] ? key[1] : code;
    if (character >= 0x80)
        character = CodePageFromUnicode((u32)character);
    if (character >= 0x20 && us >= 'A' && us <= 'Z')
        us += 'a' - 'A';
    if (character >= 0x20)
        Push((u16)(ScanOf(us) << 8 | character));
    return true;
}

/* A report of the mouse in SGR's form, CSI < b ; x ; y M, or m for a
   button let go: the cell, from 1, and in b the button, 0 left, 1
   middle, 2 right, with 32 for a motion and 64 for the wheel. The wheel
   and the buttons beyond the third are not reported to the program, a
   mouse driver of DOS having none. False where it has to wait. */
static bool Mouse(i32 b, i32 x, i32 y, bool up)
{
    static const u16 Button[4] = { 1, 4, 2, 0 };
    u16 buttons = Pointer.buttons;

    if ((b & (64 | 128)) != 0)
        return true;
    if (up)
        buttons &= (u16)~Button[b & 3];
    else if ((b & 32) == 0)
        buttons |= Button[b & 3];
    if (buttons != Pointer.buttons) {
        if (!MayChange())
            return false;
        ButtonSeen = false;
        ChangedAt = Now();
        ButtonAt = ChangedAt;
    }
    Pointer.column = x < 1 ? 0 : x > 256 ? 255 : x - 1;
    Pointer.row = y < 1 ? 0 : y > 256 ? 255 : y - 1;
    Pointer.buttons = buttons;
    Pointer.count++;
    return true;
}

/* The focus gone elsewhere: nothing is held any more that the terminal
   could tell of, since what is let go elsewhere is not reported. */
static void Unfocused(void)
{
    HeldNow &= 0x60;
    HeldSeen = true;
    if (Pointer.buttons != 0) {
        Pointer.buttons = 0;
        Pointer.count++;
        ButtonSeen = true;
    }
}

#define FIELDS 3
#define PARTS 4

/* A sequence of CSI, read: the private byte in front of it, if any; up
   to FIELDS numbers between `;`, each of up to PARTS between `:`; and
   the final byte. */
struct Csi {
    char private_byte, final;
    i32 number[FIELDS][PARTS];
    bool given[FIELDS][PARTS];
};

/* A sequence of CSI read, made what it names: false where it has to
   wait. A sequence that names nothing is dropped, an answer of the
   terminal among them, but the one to the question for the kitty
   protocol, which KeyboardKittyAnswered tells. */
static bool Sequence(const struct Csi *csi)
{
    i32 held = csi->given[1][0] && csi->number[1][0] > 1
               ? csi->number[1][0] - 1 : 0;
    i32 event = csi->given[1][1] ? csi->number[1][1] : 1;
    i32 named;

    if (csi->private_byte == '<' && (csi->final == 'M' || csi->final == 'm'))
        return Mouse(csi->number[0][0], csi->number[1][0], csi->number[2][0],
                     csi->final == 'm');
    if (csi->private_byte == '?' && csi->final == 'u') {
        Kitty = true;
        KittyAnswered = true;
        return true;
    }
    if (csi->private_byte != 0)
        return true;
    if (csi->final == 'u')
        return KittyKey(csi->number[0], csi->given[0], held, event,
                        csi->given[2][0] && !csi->given[2][1]
                        ? csi->number[2][0] : 0);
    if (csi->final == 'O' && !csi->given[0][0]) {
        Unfocused();
        return true;
    }
    if (csi->final == 'I' && !csi->given[0][0])
        return true;
    named = csi->final == '~' ? Tilde(csi->number[0][0]) : Final(csi->final);
    if (Kitty && !Hold(0, held, 1))
        return false;
    if (event != RELEASED)
        Push(KeyOfNamed(named, held));
    return true;
}

/* A sequence of CSI at `from` (behind ESC [), of `count` bytes: how many
   bytes it is, made what it names; 0 where it is not whole yet, and -1
   where what it names has to wait. */
static i32 Csi(const char *from, i32 count)
{
    struct Csi csi;
    i32 field = 0, part = 0, i;

    if (count >= 1 && from[0] == '[') {         /* the Linux console */
        if (count < 2)
            return 0;
        if (from[1] >= 'A' && from[1] <= 'E')
            Push(KeyOfNamed(SEQ_F1 + from[1] - 'A', 0));
        return 2;
    }
    memset(&csi, 0, sizeof csi);
    for (i = 0; i < count; i++) {
        char c = from[i];

        if (i == 0 && c >= '<' && c <= '?') {
            csi.private_byte = c;
        } else if (c >= '0' && c <= '9') {
            if (field < FIELDS && part < PARTS) {
                csi.given[field][part] = true;
                if (csi.number[field][part] < 10000000)
                    csi.number[field][part] =
                        csi.number[field][part] * 10 + (c - '0');
            }
        } else if (c == ';') {
            field++;
            part = 0;
        } else if (c == ':') {
            part++;
        } else if (c >= 0x40 && c <= 0x7E) {
            csi.final = c;
            return Sequence(&csi) ? i + 1 : -1;
        } else if (c < 0x20 || c > 0x3F) {
            return i;                   /* broken: dropped as far */
        }
    }
    return 0;
}

/* The keys of the keypad that xterm sends by SS3 in its application
   mode, as kitty's codes: * + - . / Enter, and p to y for its figures. */
static i32 PadOfSs3(char final)
{
    switch (final) {
    case 'j': return KP_FIRST + 12;     /* * */
    case 'k': return KP_FIRST + 14;     /* + */
    case 'm': return KP_FIRST + 13;     /* - */
    case 'n': return KP_FIRST + 10;     /* . */
    case 'o': return KP_FIRST + 11;     /* / */
    case 'M': return KP_FIRST + 15;     /* Enter */
    }
    if (final >= 'p' && final <= 'y')
        return KP_FIRST + final - 'p';
    return 0;
}

/* A sequence of SS3 at `from` (behind ESC O): F1 to F4, the arrows, Home
   and End, as xterm sends them where its keypad is set so, and the
   keypad's keys in its application mode, with a modifier's number in
   front in some. */
static i32 Ss3(const char *from, i32 count)
{
    i32 i, held = 0;

    for (i = 0; i < count; i++) {
        if (from[i] >= '0' && from[i] <= '9') {
            held = held * 10 + (from[i] - '0');
            continue;
        }
        if (PadOfSs3(from[i]) != 0)
            Push(KeyOfPad(PadOfSs3(from[i]), held > 1 ? held - 1 : 0));
        else
            Push(KeyOfNamed(Final(from[i]), held > 1 ? held - 1 : 0));
        return i + 1;
    }
    return 0;
}

/* ESC in front of a byte that begins no sequence: Alt with the key. */
static u16 KeyOfEscaped(u8 byte)
{
    switch (byte) {
    case 0x09: return KeyOfNamed(SEQ_TAB, 2);
    case 0x0D: return KeyOfNamed(SEQ_ENTER, 2);
    case 0x08: case 0x7F: return KeyOfNamed(SEQ_BACKSPACE, 2);
    }
    if (byte >= 0x01 && byte <= 0x1A)   /* Ctrl with a letter */
        return KeyOfAlt((u32)byte + 0x60);
    return KeyOfAlt(byte);
}

/* The bytes that have come, made keys as far as they are whole; `late`
   where what is left has waited ESC_WAIT, so that an ESC alone is Esc.
   A sequence whose change has to wait stops it there. */
static void Decode(bool late)
{
    i32 at = 0, used, character;
    u32 unicode;
    u8 byte;

    while (at < TypedCount && Settled()) {
        byte = (u8)Typed[at];
        if (byte == ESC) {
            if (at + 1 == TypedCount) {
                if (!late)
                    break;
                Push(0x011B);
                used = 1;
            } else if (Typed[at + 1] == '[') {
                used = Csi(Typed + at + 2, TypedCount - at - 2);
                if (used < 0)
                    break;              /* a change that waits */
                used = used == 0 ? (late ? 2 : 0) : used + 2;
            } else if (Typed[at + 1] == 'O') {
                used = Ss3(Typed + at + 2, TypedCount - at - 2);
                used = used == 0 ? (late ? 2 : 0) : used + 2;
            } else if (Typed[at + 1] == ESC) {
                Push(0x011B);
                used = 1;
            } else {
                Push(KeyOfEscaped((u8)Typed[at + 1]));
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

/* How long until what is left of the bytes may be made more of: ESC_WAIT
   from when they came, or what waits for a change or a key to be seen;
   1 at the least. */
static i32 Soonest(void)
{
    int64_t now = Now(), left = TypedSince + ESC_WAIT - now, more;

    more = !Settled() ? (HeldSeen && ButtonSeen ? ButtonAt + BUTTON_HOLD
                                                : ChangedAt + CHANGE_WAIT) - now
           : KeysCount > 0 ? PushedAt + CHANGE_WAIT - now : 0;
    if (more > 0 && (left <= 0 || more < left))
        left = more;
    return left < 1 ? 1 : (i32)left;
}

void KeyboardRefuse(bool refused)
{
    Refused = refused;
}

bool KeyboardKittyAnswered(void)
{
    bool answered = KittyAnswered;

    KittyAnswered = false;
    return answered;
}

bool KeyboardBefore(i32 *wait)
{
    i32 left;

    if (TypedCount > 0) {
        Decode(Now() - TypedSince >= ESC_WAIT);
        if (TypedCount > 0) {
            left = Soonest();
            if (*wait < 0 || *wait > left)
                *wait = left;
        }
    }
    if (KeysCount > 0)
        *wait = 0;
    return TypedCount < TYPED_ROOM;
}

i32 KeyboardRoom(char **to)
{
    if (TypedCount == 0)
        TypedSince = Now();
    *to = Typed + TypedCount;
    return TYPED_ROOM - TypedCount;
}

void KeyboardAfter(i32 count)
{
    TypedCount += count;
    Decode(TypedCount == TYPED_ROOM);
    if (TypedCount == 0)
        TypedSince = 0;
}

struct Keys KeyboardWaiting(void)
{
    struct Keys keys;

    keys.none = KeysCount == 0;
    keys.key = keys.none ? 0 : Buffer[KeysFirst];
    return keys;
}

u16 KeyboardTake(void)
{
    u16 key = Buffer[KeysFirst];

    KeysFirst = (KeysFirst + 1) % KEYS_ROOM;
    KeysCount--;
    return key;
}

u16 KeyboardHeld(void)
{
    HeldSeen = true;
    return HeldNow;
}

struct ReportedMouse KeyboardMouse(bool looked)
{
    if (looked)
        ButtonSeen = true;
    return Pointer;
}
