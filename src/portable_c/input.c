/*
 * Waiting for a key, and a line of text that is typed into.
 *
 * Translated from VCSUB2.INC.
 *
 * Everything that takes keys gets them from Input, or from Input0 where
 * F1 is not the help. While Input0 waits it keeps the key bar as Shift,
 * Ctrl and Alt are held, shows the clock, gives the time to other
 * programs, blanks the screen after a while, takes a tap on Alt for a key
 * and a click of the mouse on the key bar for the key that is clicked.
 *
 * A key is a word, the character in the low byte and the scan code in the
 * high one. Input0 has scan codes of its own: FCh says that a button of
 * the mouse is down, the low byte has which; FDh is a tap on Alt, and FEh
 * says that the time has come which the caller left in AutoTim.
 *
 * The screen blanker itself and what gives the time away, Blank and
 * LeaveTm, are in blank.c.
 */
#include "vc.h"

#define Clock_C 34                      /* the colour of the clock */

#define KEY_MOUSE 0xFC00
#define KEY_ALT   0xFD00
#define KEY_TIMER 0xFE00
#define KEY_F1    0x3B00

#define CHECK_MARK 0xFB                 /* the look of the mouse pointer on
                                           a key of the key bar */

/* Where the clock is in the screen buffer: six cells at the end of the
   first row, of which a clock of 24 hours leaves the first alone. */
#define CLOCK_CELLS 6
#define CLOCK_AT    (md.ScrBuf + 74)

#define LOW(pair)  ((u16)(pair))
#define HIGH(pair) ((u16)((pair) >> 16))

/* Routines of the rest of the program that this file calls, all of them
   C, of vc.h.

   The keyboard, KeyWaiting, KeyRead and KeysHeld, and the time of day,
   TimeOfDay, are the system's, of sys.h: the time has the hours and the
   minutes in the two bytes of one word, the seconds and their hundredths
   in those of another. Timer returns the timer's count of ticks.

   GetMous returns the buttons of the mouse that are down and where the
   mouse is, the row in the high byte, or `place` as it was if there is
   no mouse. TypMous gives the mouse pointer the look of a
   character, or its own for 0. InWind is not 0 if a place is outside a
   window. Beep sounds, if the setup has errors do so.

   Blank blanks the screen until a key is pressed or the mouse moved, and
   LeaveTm tells DOS that the program is idle.
 */

/* The characters that words end at. */
static const u8 CtEn_T[10] = {
    ' ', '+', ':', '.', ',', '>', '<', '|', '\\', '\t'
};

/* The next of the program's random numbers. It is asked for every time
   Input0 looks for a key and finds none, so the numbers that the screen
   blanker gets differ from one time to the next. */
u16 Rnd(void)
{
    u16 first = md.Rand1 * 3;
    u16 second = md.Rand2;

    md.Rand1 = second;
    second = second * 3 + first + 23457;
    md.Rand2 = second;
    return second;
}

/* What Input0 returns: a key, and whether it was clicked on the key
   bar. */
static struct Key Pressed(u16 key, bool clicked)
{
    struct Key pressed;

    pressed.key = key;
    pressed.clicked = clicked;
    return pressed;
}

/* Inpt04: when to blank the screen, now and the minutes of the setup,
   the hours in the high byte, with the seconds in `*low`. Without any,
   FFFFh, and `*low` as it is: the assembly keeps there what it has in DX,
   which it does not read. */
static u16 Inpt04(u16 *low)
{
    struct Time moment;
    u16 time;
    i32 hour, minute;

    if (md.BlDelay == 0)
        return 0xFFFF;
    moment = TimeOfDay();
    *low = moment.seconds;
    time = moment.clock;
    minute = (time + md.BlDelay) & 0xFF;
    hour = time >> 8;
    if (minute >= 60) {
        minute -= 60;
        hour = hour + 1 & 0xFF;
    }
    return (u16)(hour << 8 | minute);
}

/* Inpt12: the screen blanked until a key or the mouse, and drawn again.
   The round goes on at Inpt03 then. */
static void Inpt12(void)
{
    Blank();
    Window(0, (u16)(md.Lines << 8 | 80));
    while (GetMous(0).buttons != 0)
        ;
}

/* Inpt41: what Input0 does with the key `key` that it has, with the key
   bar `bar`, a byte. Returns true with what Input0 returns in `*pressed`,
   or false where a button of the mouse was let go on the key bar
   elsewhere than on a key: the round then begins anew, at Inpt02. */
static bool Inpt41(u16 key, i32 bar, struct Key *pressed)
{
    struct Mouse seen;
    u16 held, buttons, look, shown, mouse;
    i32 shift, column;
    bool tracking, outside;

    *(u16 *)&md.AutoTim = 0xFFFF;
    if ((key & 0xFF00) != KEY_MOUSE || md.KeyBar == 0) {
        *pressed = Pressed(key, false);
        return true;
    }
    /* A button of the mouse is down: on the key bar it is held for a
       key, the left one as the key is, the middle one with Shift and the
       right one with Alt. The bar shows what the keys are then, the
       pointer has a check mark while it is on the key, and the key is
       pressed when the button is let go there. */
    seen = GetMous(0);
    buttons = seen.buttons;
    mouse = seen.place;
    key = KEY_MOUSE | buttons;
    if ((u8)((mouse >> 8) + 1) != md.Lines) {
        *pressed = Pressed(key, false);
        return true;
    }
    column = mouse & 0xF8;              /* where the key begins: BL */
    held = 0;                           /* the buttons one round before:
                                           DH */
    shown = 0;                          /* the look of the pointer: SI */
    tracking = false;                   /* the key has a name: DI */
    for (;;) {                          /* Inpt50 */
        outside = InWind(mouse, (u16)((md.Lines - 1) << 8 | column),
                         0x108);
        look = 0;
        /* The first time round, on the key, the pointer is left as it
           is. */
        if (outside || tracking) {
            if (!outside)
                look = CHECK_MARK;
            if (look != shown) {
                shown = look;
                TypMous(look);
            }
        }
        if (buttons & 2) {              /* Inpt52 */
            shift = 3;
        } else if (buttons & 4) {
            shift = 1;
        } else if (buttons & 1) {
            shift = 0;
        } else {
            /* Inpt56: let go. */
            if (look == 0)
                return false;
            TypMous(0);
            if (held & 2)
                key = 0x6800;           /* Alt-F1 */
            else if (held & 4)
                key = 0x5400;           /* Shift-F1 */
            else if (held & 1)
                key = KEY_F1;
            else
                return false;
            *pressed = Pressed((u16)(key + (column >> 3 << 8)), true);
            return true;
        }
        PutKey0((u16)bar, (u16)shift);  /* Inpt54; the bar is a byte, as
                                           Input0 makes it */
        held = buttons;
        seen = GetMous(0);
        buttons = seen.buttons;
        mouse = seen.place;
        if (!tracking) {
            /* A key without a name is none: the click is the caller's. */
            if ((*BasAdr((u16)((mouse & 0xFF00)
                               | column + 1 + (column >= 72 ? 1 : 0)))
                 & 0xFF) == ' ') {
                *pressed = Pressed(KEY_MOUSE | buttons, false);
                return true;
            }
            tracking = true;
        }
    }
}

/* Wait for a key with the key bar `bar` on the screen, of which only the
   low byte counts, and return it, and whether the key was clicked on the
   key bar with the mouse: the assembly has the carry flag for that. */
struct Key Input0(i32 bar)
{
    u16 ClkBuf[CLOCK_CELLS];            /* what the clock covers */
    u16 KbdFlgs;                        /* Shift, Ctrl, Alt and the locks */
    bool AltLock;                       /* Alt went down, and nothing else
                                           has since */
    u16 ClkFlag;                        /* what the clock has between hours
                                           and minutes: it blinks */
    bool BlMouse;                       /* the mouse has been outside the
                                           corner that blanks the screen */
    u16 BlHigh, BlLow = 0;              /* when to blank the screen */
    u32 AltTime;                        /* the ticks one round before */
    u32 pair, now;
    struct Time moment;
    struct Key pressed;
    u16 key, time, flags, was, mouse;
    i32 hour, i;
    u16 hour_digits, minute_digits, color;
    u16 separator, suffix = 0;
    u16 *screen;

    bar &= 0xFF;
    if (!KeyWaiting().none) {
        key = KeyRead();
        if (Inpt41(key, bar, &pressed))
            return pressed;
    }
    for (;;) {                          /* Inpt02 */
        key = KEY_MOUSE | GetMous(0).buttons;
        if ((key & 0xFF) == 0) {
            AltTime = Timer();
            for (i = 0; i < CLOCK_CELLS; i++)
                ClkBuf[i] = CLOCK_AT[i];
            KbdFlgs = KeysHeld();
            AltLock = false;
            ClkFlag = 0;                /* Inpt03 */
            BlMouse = false;
            BlHigh = Inpt04(&BlLow);
            for (;;) {                  /* Inpt10 */
                PutKey((u16)bar);       /* a byte, as the top of the
                                           function makes it */
                LeaveTm();
                if (md.BlDelay != 0) {
                    moment = TimeOfDay();
                    time = moment.clock;
                    /* Past midnight an hour of 24 or more is the hour
                       0. */
                    if (time >> 8 == 0 && BlHigh >> 8 >= 24)
                        BlHigh &= 0xFF;
                    if (time > BlHigh
                        || (time == BlHigh && moment.seconds >= BlLow)) {
                        Inpt12();
                        ClkFlag = 0;    /* Inpt03 */
                        BlMouse = false;
                        BlHigh = Inpt04(&BlLow);
                        continue;
                    }
                }
                if (md.Clock != 0) {            /* Inpt20 */
                    screen = CLOCK_AT + 1;
                    moment = TimeOfDay();
                    time = moment.clock;
                    hour = time >> 8;
                    if (md.Country.TimeFmt == 0) {
                        screen--;
                        suffix = hour / 12 == 0 ? 'a' : 'p';
                        hour %= 12;
                        if (hour == 0)
                            hour = 12;
                    }
                    separator = ' ';
                    if ((moment.seconds & 0xFF) >= 50)
                        separator = md.Country.TimeSep[0];
                    if (separator != ClkFlag) {
                        ClkFlag = separator;
                        hour_digits = HexDec(hour);
                        minute_digits = HexDec(time);
                        color = (u16)(Color(Clock_C) << 8);
                        *screen++ = color | ((hour_digits & 0xFF) == '0'
                                             ? ' ' : hour_digits & 0xFF);
                        *screen++ = color | hour_digits >> 8;
                        *screen++ = color | separator;
                        *screen++ = color | (minute_digits & 0xFF);
                        *screen++ = color | minute_digits >> 8;
                        if (md.Country.TimeFmt == 0)
                            *screen++ = color | suffix;
                        Window(74, 0x106);
                    }
                }
                key = KEY_TIMER;        /* Inpt30 */
                if ((u16)md.AutoTim != 0xFFFF) {
                    /* AutoTim has the hours and the minutes in its low
                       word. */
                    moment = TimeOfDay();
                    time = moment.clock;
                    if (time > (u16)md.AutoTim
                        || (time == (u16)md.AutoTim
                           && moment.seconds >= (u16)(md.AutoTim >> 16)))
                        break;
                }
                Rnd();                  /* Inpt31 */
                if (!KeyWaiting().none) {
                    key = KeyRead();
                    break;
                }
                key = KEY_MOUSE | GetMous(0).buttons;  /* Inpt33 */
                if ((key & 0xFF) != 0)
                    break;
                flags = KeysHeld();
                was = KbdFlgs;
                KbdFlgs = flags;
                if (md.AltBar != 0) {   /* and not under DESQview, in
                                           the original */
                    /* A tap on Alt: Alt is all that has changed since the
                       round before, and that was less than three ticks
                       ago. The assembly tests the time in 16 bits, twice
                       the high word of the difference and 1 for a low
                       word of 3 or more. */
                    now = Timer();
                    pair = now - AltTime;
                    AltTime = now;
                    if ((u16)(HIGH(pair) * 2 + (LOW(pair) >= 3 ? 1 : 0)) == 0
                        && (was ^ flags) == 8) {
                        if (flags & 8) {
                            AltLock = true;
                            BlHigh = Inpt04(&BlLow);
                            continue;
                        }
                        if (AltLock) {
                            key = KEY_ALT;
                            break;
                        }
                    }
                }
                flags ^= was & 0xF0;    /* Inpt34 */
                if (flags != 0) {
                    /* A key among Shift, Ctrl and Alt is held, or a lock
                       has changed: anything but Alt alone is no tap on
                       Alt any more. */
                    if ((flags & 0xF7) != 0)
                        AltLock = false;
                    BlHigh = Inpt04(&BlLow);
                    continue;
                }
                /* The mouse in the top right corner blanks the screen at
                   once, when it has been elsewhere since; in the bottom
                   right corner it keeps the screen from being blanked. */
                mouse = GetMous(0).place;
                if (InWind(mouse, 78, 0x102))
                    BlMouse = true;
                else if (BlMouse) {
                    Inpt12();
                    ClkFlag = 0;        /* Inpt03 */
                    BlMouse = false;
                    BlHigh = Inpt04(&BlLow);
                    continue;
                }
                if (!InWind(mouse, (u16)((md.Lines - 1) << 8 | 78), 0x102))
                    BlHigh = Inpt04(&BlLow);    /* and Inpt10 */
            }
            /* Inpt40 */
            if (md.Clock != 0) {
                for (i = 0; i < CLOCK_CELLS; i++)
                    CLOCK_AT[i] = ClkBuf[i];
                Window(74, 0x106);
            }
        }
        if (Inpt41(key, bar, &pressed))  /* Inpt41 */
            return pressed;
    }
}

/* Wait for a key with the key bar `bar` on the screen and return it. F1
   is the help, and is not returned: the help of the program, or, while
   the setup is open (StOpen of vc.h), that of the setup, which is
   another. The setup program has this routine word for word, with its
   own Help behind the name. */
u16 Input(i32 bar)
{
    u16 key;

    for (;;) {
        key = Input0(bar).key;
        if (key != KEY_F1)
            return key;
        if (StOpen != 0)
            StHelp();
        else
            Help00();
    }
}

/* Whether a character is one that words end at. Only the low byte of
   `symbol` counts. */
bool CmpSym(u16 symbol)
{
    i32 i;

    for (i = 0; i < (i32)sizeof CtEn_T; i++)
        if (CtEn_T[i] == (u8)symbol)
            return true;
    return false;
}

/* Where the word begins that the cursor is in or behind: the column for
   the cursor at `cursor` in the line `line`. */
i32 EdSub1(const u8 *line, i32 cursor)
{
    do {
        if (cursor == 0)
            return 0;
        cursor--;
    } while (CmpSym(line[cursor]));
    do {
        if (cursor == 0)
            return 0;
        cursor--;
    } while (!CmpSym(line[cursor]));
    return cursor + 1;
}

/* Where the word ends that the cursor is in or before, or the line. */
static i32 EdSub2(const u8 *line, i32 cursor)
{
    do {
        if (line[cursor] == 0)
            return cursor;
        cursor++;
    } while (CmpSym(line[cursor]));
    do {
        if (line[cursor] == 0)
            return cursor;
        cursor++;
    } while (!CmpSym(line[cursor]));
    return cursor;
}

/* What EdLin returns: the cursor at a column, with the line or the
   cursor changed, or neither. */
static struct Edited Changed(i32 column)
{
    struct Edited edited;

    edited.cursor = column;
    edited.changed = true;
    return edited;
}

static struct Edited Unchanged(i32 column)
{
    struct Edited edited;

    edited.cursor = column;
    edited.changed = false;
    return edited;
}

/* E01_00, Left and Ctrl-S: the cursor a column back. */
static struct Edited E01_00(i32 cursor)
{
    if (cursor == 0)
        return Unchanged(cursor);
    return Changed(cursor - 1);
}

/* E02_00, Right and Ctrl-D: the cursor a column on, within the text. */
static struct Edited E02_00(const u8 *text, i32 cursor)
{
    if (text[cursor] == 0)
        return Unchanged(cursor);
    return Changed(cursor + 1);
}

/* E00_05: what is between the columns `cursor` and `from` of `text`
   is taken out. */
static struct Edited E00_05(u8 *text, i32 cursor, i32 from)
{
    u8 *to = text + cursor;

    do
        *to++ = text[from];
    while (text[from++] != 0);
    return Changed(cursor);
}

/* E08_00, Del and Ctrl-G, and Backspace behind its step back: the
   character at the cursor is taken out. */
static struct Edited E08_00(u8 *text, i32 cursor)
{
    if (text[cursor] == 0)
        return Unchanged(cursor);
    return E00_05(text, cursor, cursor + 1);
}

/* Have a line that is typed into take a key: `text` is the line, in the
   main data, with room for `length` characters and a zero byte, and
   `cursor` the column of the cursor in it. Returns the column of the
   cursor then, and whether the line or the cursor has changed.

   Ctrl-Q has the next key typed as it is, whatever it does otherwise;
   Quote remembers that from one call to the next. */
struct Edited EdLin(u16 key, u8 *text, i32 cursor, i32 length)
{
    u16 low = key & 0xFF;
    i32 count;
    u8 *to;

    if (md.Quote != 0) {                /* E00_03 */
        md.Quote = 0;
        if (low == 0)
            return Unchanged(cursor);
        /* on to E00_01 */
    } else if (key >> 8 != 0) {
        /* A key without a scan code is a character, and none of these. */
        if (low == 0) {
            switch (key >> 8) {
            case 0x4B:                  /* Left */
                return E01_00(cursor);
            case 0x4D:                  /* Right */
                return E02_00(text, cursor);
            case 0x73:                  /* Ctrl-Left: E03_00 */
                return Changed(EdSub1(text, cursor));
            case 0x74:                  /* Ctrl-Right: E04_00 */
                return Changed(EdSub2(text, cursor));
            case 0x47:                  /* Home */
            case 0x77:                  /* Ctrl-Home: E05_00 */
                return Changed(0);
            case 0x4F:                  /* End */
            case 0x75:                  /* Ctrl-End: E06_00 */
                /* Where the text ends, looked for in the line and one
                   more. */
                cursor = 0;
                for (count = length + 1; count != 0; count--)
                    if (text[cursor++] == 0)
                        break;
                return Changed(cursor - 1);
            case 0x53:                  /* Del */
                return E08_00(text, cursor);
            }
        } else {
            switch (low) {
            case 0x13:                  /* Ctrl-S */
                return E01_00(cursor);
            case 0x04:                  /* Ctrl-D */
                return E02_00(text, cursor);
            case 0x01:                  /* Ctrl-A: E03_00 */
                return Changed(EdSub1(text, cursor));
            case 0x06:                  /* Ctrl-F: E04_00 */
                return Changed(EdSub2(text, cursor));
            case 0x08:                  /* Backspace: E07_00 */
                if (cursor == 0)
                    return Unchanged(cursor);
                return E08_00(text, cursor - 1);
            case 0x07:                  /* Ctrl-G */
                return E08_00(text, cursor);
            case 0x19:                  /* Ctrl-Y: E09_00, all of the line */
                text[0] = 0;
                return Changed(0);
            case 0x0B:                  /* Ctrl-K: E10_00, the rest of it */
                text[cursor] = 0;
                return Changed(cursor);
            case 0x17:                  /* Ctrl-W */
            case 0x7F:                  /* Ctrl-Backspace: E11_00, back to
                                           where the word begins */
                return E00_05(text, EdSub1(text, cursor), cursor);
            case 0x14:                  /* Ctrl-T: E12_00, up to where it
                                           ends */
                return E00_05(text, cursor, EdSub2(text, cursor));
            case 0x11:                  /* Ctrl-Q */
                md.Quote = 1;
                return Unchanged(cursor);
            }
        }
        if (low < ' ')
            return Unchanged(cursor);
    } else if (low == 0) {
        return Unchanged(cursor);
    }
    /* E00_01: a character */
    if (cursor >= length) {
        Beep();
        return Unchanged(cursor);
    }
    to = text + length;
    for (count = length - cursor; count != 0; count--) {
        *to = to[-1];
        to--;
    }
    text[length] = 0;
    text[cursor] = (u8)low;
    return Changed(cursor + 1);
}

/* The speed search: look for the next name that begins with what has
   been typed so far and the key `key`. The names are in `list`, a
   panel or the tree's list, one every `step` bytes from the offset
   `begin` to `end`; `at` is the one that the cursor is on. Ctrl-Enter, a line feed, types nothing and
   looks from the name behind that one. Returns the name found, or `at`
   if there is none, and then what has been typed stays as it was.

   What has been typed is at MenuBuf+228: the column of the cursor in a
   word and a line of twelve characters, which takes the key as any line
   does. It is tried at MenuBuf+208 and kept if a name fits it, and the
   capitals of it are at MenuBuf. */
i32 Search(const u8 *list, i32 at, i32 begin, i32 end, i32 step, u16 key)
{
    u8 *tried = md.MenuBuf + 208;
    u8 *typed = md.MenuBuf + 228;
    const u8 *name;
    i32 from = at;
    i32 length, i;

    if ((key & 0xFF) == 0x0A) {
        at += step;
        if (at >= end)
            at = begin;
    }
    for (i = 0; i < 15; i++)
        tried[i] = typed[i];
    if ((key & 0xFF) != 0x0A)
        *(u16 *)tried = (u16)EdLin(key, tried + 2, *(u16 *)tried, 12).cursor;
    length = (i32)(MvzLine(md.MenuBuf, tried + 2, 13) - md.MenuBuf);
    PathLin(md.MenuBuf);
    if (length != 0) {
        if (end <= begin)
            return at;
        for (;;) {
            name = list + at;
            for (i = 0; i < length; i++)
                if ((u8)UpCase(name[i]) != md.MenuBuf[i])
                    break;
            if (i == length)
                break;
            at += step;
            if (at >= end)
                at = begin;
            if (at == from)
                return at;
        }
    }
    for (i = 0; i < 15; i++)
        typed[i] = tried[i];
    return at;
}
