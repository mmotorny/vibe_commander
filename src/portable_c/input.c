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

/* C routines of other files that vc.h does not have. */
u16 HexDec(i32 number);                                 /* format.c */
void Help00(void);                                      /* help.c */

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
    struct Mouse seen;
    u16 key, time, flags, was, mouse;
    i32 hour, minute, i, shift, column;
    u16 hour_digits, minute_digits, color;
    u16 separator, suffix = 0;
    u16 held, buttons, look, shown;
    bool tracking, outside;
    u16 *screen;

    bar &= 0xFF;
    if (!KeyWaiting().none) {
        key = KeyRead();
        goto Inpt41;
    }
Inpt02:
    key = KEY_MOUSE | GetMous(0).buttons;
    if ((key & 0xFF) != 0)
        goto Inpt41;
    AltTime = Timer();
    for (i = 0; i < CLOCK_CELLS; i++)
        ClkBuf[i] = CLOCK_AT[i];
    KbdFlgs = KeysHeld();
    AltLock = false;
Inpt03:
    ClkFlag = 0;
    BlMouse = false;
Inpt04:
    /* The time to blank the screen at: now and the minutes of the setup.
       Without any, the assembly keeps here what it has in DX, which it
       does not read. */
    BlHigh = 0xFFFF;
    if (md.BlDelay != 0) {
        moment = TimeOfDay();
        BlLow = moment.seconds;
        time = moment.clock;
        minute = (time + md.BlDelay) & 0xFF;
        hour = time >> 8;
        if (minute >= 60) {
            minute -= 60;
            hour = hour + 1 & 0xFF;
        }
        BlHigh = (u16)(hour << 8 | minute);
    }
Inpt10:
    PutKey((u16)bar);                   /* a byte, as the top of the
                                           function makes it */
    LeaveTm();
    if (md.BlDelay != 0) {
        moment = TimeOfDay();
        time = moment.clock;
        /* Past midnight an hour of 24 or more is the hour 0. */
        if (time >> 8 == 0 && BlHigh >> 8 >= 24)
            BlHigh &= 0xFF;
        if (time > BlHigh || (time == BlHigh && moment.seconds >= BlLow)) {
Inpt12:
            Blank();
            Window(0, (u16)(md.Lines << 8 | 80));
            while (GetMous(0).buttons != 0)
                ;
            goto Inpt03;
        }
    }
    if (md.Clock != 0) {                /* Inpt20 */
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
    key = KEY_TIMER;                    /* Inpt30 */
    if ((u16)md.AutoTim != 0xFFFF) {
        /* AutoTim has the hours and the minutes in its low word. */
        moment = TimeOfDay();
        time = moment.clock;
        if (time > (u16)md.AutoTim
            || (time == (u16)md.AutoTim
               && moment.seconds >= (u16)(md.AutoTim >> 16)))
            goto Inpt40;
    }
    Rnd();                              /* Inpt31 */
    if (!KeyWaiting().none) {
        key = KeyRead();
        goto Inpt40;
    }
    key = KEY_MOUSE | GetMous(0).buttons;  /* Inpt33 */
    if ((key & 0xFF) != 0)
        goto Inpt40;
    flags = KeysHeld();
    was = KbdFlgs;
    KbdFlgs = flags;
    if (md.AltBar != 0) {               /* and not under DESQview, in
                                           the original */
        /* A tap on Alt: Alt is all that has changed since the round
           before, and that was less than three ticks ago. The assembly
           tests the time in 16 bits, twice the high word of the
           difference and 1 for a low word of 3 or more. */
        now = Timer();
        pair = now - AltTime;
        AltTime = now;
        if ((u16)(HIGH(pair) * 2 + (LOW(pair) >= 3 ? 1 : 0)) == 0
            && (was ^ flags) == 8) {
            if (flags & 8) {
                AltLock = true;
                goto Inpt04;
            }
            if (AltLock) {
                key = KEY_ALT;
                goto Inpt40;
            }
        }
    }
    flags ^= was & 0xF0;                /* Inpt34 */
    if (flags != 0) {
        /* A key among Shift, Ctrl and Alt is held, or a lock has changed:
           anything but Alt alone is no tap on Alt any more. */
        if ((flags & 0xF7) != 0)
            AltLock = false;
        goto Inpt04;
    }
    /* The mouse in the top right corner blanks the screen at once, when
       it has been elsewhere since; in the bottom right corner it keeps
       the screen from being blanked. */
    mouse = GetMous(0).place;
    if (InWind(mouse, 78, 0x102))
        BlMouse = true;
    else if (BlMouse)
        goto Inpt12;
    if (!InWind(mouse, (u16)((md.Lines - 1) << 8 | 78), 0x102))
        goto Inpt04;
    goto Inpt10;

Inpt40:
    if (md.Clock != 0) {
        for (i = 0; i < CLOCK_CELLS; i++)
            CLOCK_AT[i] = ClkBuf[i];
        Window(74, 0x106);
    }
Inpt41:
    *(u16 *)&md.AutoTim = 0xFFFF;
    if ((key & 0xFF00) != KEY_MOUSE || md.KeyBar == 0)
        return Pressed(key, false);
    /* A button of the mouse is down: on the key bar it is held for a
       key, the left one as the key is, the middle one with Shift and the
       right one with Alt. The bar shows what the keys are then, the
       pointer has a check mark while it is on the key, and the key is
       pressed when the button is let go there. */
    seen = GetMous(0);
    buttons = seen.buttons;
    mouse = seen.place;
    key = KEY_MOUSE | buttons;
    if ((u8)((mouse >> 8) + 1) != md.Lines)
        return Pressed(key, false);
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
                goto Inpt02;
            TypMous(0);
            if (held & 2)
                key = 0x6800;           /* Alt-F1 */
            else if (held & 4)
                key = 0x5400;           /* Shift-F1 */
            else if (held & 1)
                key = KEY_F1;
            else
                goto Inpt02;
            return Pressed((u16)(key + (column >> 3 << 8)), true);
        }
        PutKey0((u16)bar, (u16)shift);  /* Inpt54; the bar is a byte, as
                                           the top of the function makes
                                           it */
        held = buttons;
        seen = GetMous(0);
        buttons = seen.buttons;
        mouse = seen.place;
        if (!tracking) {
            /* A key without a name is none: the click is the caller's. */
            if ((*BasAdr((u16)((mouse & 0xFF00)
                               | column + 1 + (column >= 72 ? 1 : 0)))
                 & 0xFF) == ' ')
                return Pressed(KEY_MOUSE | buttons, false);
            tracking = true;
        }
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

/* Have a line that is typed into take a key: `text` is the line, in the
   main data, with room for `length` characters and a zero byte, and
   `cursor` the column of the cursor in it. Returns the column of the
   cursor then, and whether the line or the cursor has changed.

   Ctrl-Q has the next key typed as it is, whatever it does otherwise;
   Quote remembers that from one call to the next. */
struct Edited EdLin(u16 key, u8 *text, i32 cursor, i32 length)
{
    u16 low = key & 0xFF;
    i32 from, count;
    u8 *to;

    if (md.Quote != 0) {                /* E00_03 */
        md.Quote = 0;
        if (low == 0)
            return Unchanged(cursor);
        goto E00_01;
    }
    /* A key without a scan code is a character, and none of these. */
    if (key >> 8 != 0) {
        if (low == 0) {
            switch (key >> 8) {
            case 0x4B: goto E01_00;     /* Left */
            case 0x4D: goto E02_00;     /* Right */
            case 0x73: goto E03_00;     /* Ctrl-Left */
            case 0x74: goto E04_00;     /* Ctrl-Right */
            case 0x47:                  /* Home */
            case 0x77: goto E05_00;     /* Ctrl-Home */
            case 0x4F:                  /* End */
            case 0x75: goto E06_00;     /* Ctrl-End */
            case 0x53: goto E08_00;     /* Del */
            }
        } else {
            switch (low) {
            case 0x13: goto E01_00;     /* Ctrl-S */
            case 0x04: goto E02_00;     /* Ctrl-D */
            case 0x01: goto E03_00;     /* Ctrl-A */
            case 0x06: goto E04_00;     /* Ctrl-F */
            case 0x08: goto E07_00;     /* Backspace */
            case 0x07: goto E08_00;     /* Ctrl-G */
            case 0x19: goto E09_00;     /* Ctrl-Y */
            case 0x0B: goto E10_00;     /* Ctrl-K */
            case 0x17:                  /* Ctrl-W */
            case 0x7F: goto E11_00;     /* Ctrl-Backspace */
            case 0x14: goto E12_00;     /* Ctrl-T */
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
E00_01:                                 /* a character */
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

E01_00:                                 /* Left, Ctrl-S */
    if (cursor == 0)
        return Unchanged(cursor);
    return Changed(cursor - 1);
E02_00:                                 /* Right, Ctrl-D */
    if (text[cursor] == 0)
        return Unchanged(cursor);
    return Changed(cursor + 1);
E03_00:                                 /* Ctrl-Left, Ctrl-A */
    return Changed(EdSub1(text, cursor));
E04_00:                                 /* Ctrl-Right, Ctrl-F */
    return Changed(EdSub2(text, cursor));
E05_00:                                 /* Home */
    return Changed(0);
E06_00:                                 /* End: where the text ends, looked
                                           for in the line and one more */
    cursor = 0;
    for (count = length + 1; count != 0; count--)
        if (text[cursor++] == 0)
            break;
    return Changed(cursor - 1);
E07_00:                                 /* Backspace */
    if (cursor == 0)
        return Unchanged(cursor);
    cursor--;
E08_00:                                 /* Del, Ctrl-G */
    if (text[cursor] == 0)
        return Unchanged(cursor);
    from = cursor + 1;
    goto E00_05;
E09_00:                                 /* Ctrl-Y: all of the line */
    cursor = 0;
E10_00:                                 /* Ctrl-K: the rest of it */
    text[cursor] = 0;
    return Changed(cursor);
E11_00:                                 /* Ctrl-Backspace, Ctrl-W: back to
                                           where the word begins */
    from = cursor;
    cursor = EdSub1(text, cursor);
    goto E00_05;
E12_00:                                 /* Ctrl-T: up to where it ends */
    from = EdSub2(text, cursor);
E00_05:                                 /* take out what is between */
    to = text + cursor;
    do
        *to++ = text[from];
    while (text[from++] != 0);
    return Changed(cursor);
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
u16 Search(const u8 *list, u16 at, u16 begin, u16 end, u16 step, u16 key)
{
    u8 *tried = md.MenuBuf + 208;
    u8 *typed = md.MenuBuf + 228;
    const u8 *name;
    u16 from = at;
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
