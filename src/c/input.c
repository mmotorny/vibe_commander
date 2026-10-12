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

   Int16 calls the BIOS for the keyboard, with `ax` as INT 16h takes it: it
   returns AX as it comes back, and 1 in the high word if the zero flag was
   set, which says for function 1 that no key waits. Function 0 takes the
   key that waits. DosTime asks DOS for the time: the hours and the minutes
   in the two bytes of the low word, the seconds and their hundredths in
   those of the high one. Timer returns the BIOS's count of timer ticks.

   GetMous returns the buttons of the mouse that are down, and in the high
   word where the mouse is, the row in the high byte, or `place` as it was
   if there is no mouse. TypMous gives the mouse pointer the look of a
   character, or its own for 0. InWind is not 0 if a place is outside a
   window. Beep sounds, if the setup has errors do so.

   Blank blanks the screen until a key is pressed or the mouse moved, and
   LeaveTm tells DOS and what runs beside it that the program is idle.
 */

/* C routines of other files that vc.h does not have. */
u16 HexDec(u16 number);                                 /* format.c */
void Help00(void);                                      /* help.c */
#pragma aux HexDec  "*_c" __far parm [ax] value [ax] modify exact [ax]
#pragma aux Help00  "*_c" __far modify exact [ax]

/* The characters that words end at. */
static const u8 CODE CtEn_T[10] = {
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

/* Wait for a key with the key bar `bar` on the screen, of which only the
   low byte counts, and return it. The high word is not 0 if the key was
   clicked on the key bar with the mouse: the assembly has the carry flag
   for that. */
u32 Input0(u16 bar)
{
    u16 ClkBuf[CLOCK_CELLS];            /* what the clock covers */
    u16 KbdFlgs;                        /* Shift, Ctrl, Alt and the locks */
    u16 AltLock;                        /* Alt went down, and nothing else
                                           has since */
    u16 ClkFlag;                        /* what the clock has between hours
                                           and minutes: it blinks */
    u16 BlMouse;                        /* the mouse has been outside the
                                           corner that blanks the screen */
    u16 BlHigh, BlLow = 0;              /* when to blank the screen */
    u32 AltTime;                        /* the ticks one round before */
    u32 pair, now;
    u16 key, time, flags, was, hour, minute, mouse, i;
    u16 separator, suffix = 0;
    u16 column, held, buttons, look, shown, tracking, outside;
    u16 *screen;

    bar &= 0xFF;
    if (HIGH(Int16(0x100)) == 0) {
        key = (u16)Int16(0);
        goto Inpt41;
    }
Inpt02:
    key = KEY_MOUSE | LOW(GetMous(0));
    if ((key & 0xFF) != 0)
        goto Inpt41;
    AltTime = Timer();
    for (i = 0; i < CLOCK_CELLS; i++)
        ClkBuf[i] = CLOCK_AT[i];
    KbdFlgs = LOW(Int16(0x200)) & 0xFF;
    AltLock = 0;
Inpt03:
    ClkFlag = 0;
    BlMouse = 0;
Inpt04:
    /* The time to blank the screen at: now and the minutes of the setup.
       Without any, the assembly keeps here what it has in DX, which it
       does not read. */
    BlHigh = 0xFFFF;
    if (md.BlDelay != 0) {
        pair = DosTime();
        BlLow = HIGH(pair);
        time = LOW(pair);
        minute = (time + md.BlDelay) & 0xFF;
        hour = time >> 8;
        if (minute >= 60) {
            minute -= 60;
            hour = hour + 1 & 0xFF;
        }
        BlHigh = hour << 8 | minute;
    }
Inpt10:
    PutKey(bar);
    LeaveTm();
    if (md.BlDelay != 0) {
        pair = DosTime();
        time = LOW(pair);
        /* Past midnight an hour of 24 or more is the hour 0. */
        if (time >> 8 == 0 && BlHigh >> 8 >= 24)
            BlHigh &= 0xFF;
        if (time > BlHigh || time == BlHigh && HIGH(pair) >= BlLow) {
Inpt12:
            Blank();
            Window(0, md.Lines << 8 | 80);
            while (LOW(GetMous(0)) != 0)
                ;
            goto Inpt03;
        }
    }
    if (md.Clock != 0) {                /* Inpt20 */
        screen = CLOCK_AT + 1;
        pair = DosTime();
        time = LOW(pair);
        hour = time >> 8;
        if (md.Country.TimeFmt == 0) {
            screen--;
            suffix = hour / 12 == 0 ? 'a' : 'p';
            hour %= 12;
            if (hour == 0)
                hour = 12;
        }
        separator = ' ';
        if ((HIGH(pair) & 0xFF) >= 50)
            separator = md.Country.TimeSep[0];
        if (separator != ClkFlag) {
            ClkFlag = separator;
            hour = HexDec(hour);
            minute = HexDec(time);
            i = Color(Clock_C) << 8;
            *screen++ = i | ((hour & 0xFF) == '0' ? ' ' : hour & 0xFF);
            *screen++ = i | hour >> 8;
            *screen++ = i | separator;
            *screen++ = i | minute & 0xFF;
            *screen++ = i | minute >> 8;
            if (md.Country.TimeFmt == 0)
                *screen++ = i | suffix;
            Window(74, 0x106);
        }
    }
    key = KEY_TIMER;                    /* Inpt30 */
    if ((u16)md.AutoTim != 0xFFFF) {
        /* AutoTim has the hours and the minutes in its low word. */
        pair = DosTime();
        time = LOW(pair);
        if (time > (u16)md.AutoTim
            || time == (u16)md.AutoTim
               && HIGH(pair) >= (u16)(md.AutoTim >> 16))
            goto Inpt40;
    }
    Rnd();                              /* Inpt31 */
    if (HIGH(Int16(0x100)) == 0) {
        key = (u16)Int16(0);
        goto Inpt40;
    }
    key = KEY_MOUSE | LOW(GetMous(0));  /* Inpt33 */
    if ((key & 0xFF) != 0)
        goto Inpt40;
    flags = LOW(Int16(0x200)) & 0xFF;
    was = KbdFlgs;
    KbdFlgs = flags;
    if (md.DesqVie == 0 && md.AltBar != 0) {
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
                AltLock = 1;
                goto Inpt04;
            }
            if (AltLock != 0) {
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
            AltLock = 0;
        goto Inpt04;
    }
    /* The mouse in the top right corner blanks the screen at once, when
       it has been elsewhere since; in the bottom right corner it keeps
       the screen from being blanked. */
    mouse = HIGH(GetMous(0));
    if (InWind(mouse, 78, 0x102))
        BlMouse = 1;
    else if (BlMouse != 0)
        goto Inpt12;
    if (!InWind(mouse, (u16)(u8)(md.Lines - 1) << 8 | 78, 0x102))
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
        return key;
    /* A button of the mouse is down: on the key bar it is held for a
       key, the left one as the key is, the middle one with Shift and the
       right one with Alt. The bar shows what the keys are then, the
       pointer has a check mark while it is on the key, and the key is
       pressed when the button is let go there. */
    pair = GetMous(0);
    buttons = LOW(pair);
    mouse = HIGH(pair);
    key = KEY_MOUSE | buttons;
    if ((u8)((mouse >> 8) + 1) != md.Lines)
        return key;
    column = mouse & 0xF8;              /* where the key begins: BL */
    held = 0;                           /* the buttons one round before:
                                           DH */
    shown = 0;                          /* the look of the pointer: SI */
    tracking = 0;                       /* the key has a name: DI */
    for (;;) {                          /* Inpt50 */
        outside = InWind(mouse, (u16)(u8)(md.Lines - 1) << 8 | column,
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
            i = 3;
        } else if (buttons & 4) {
            i = 1;
        } else if (buttons & 1) {
            i = 0;
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
            return 0x10000UL | (u16)(key + (column >> 3 << 8));
        }
        PutKey0(bar, i);                /* Inpt54 */
        held = buttons;
        pair = GetMous(0);
        buttons = LOW(pair);
        mouse = HIGH(pair);
        if (!tracking) {
            /* A key without a name is none: the click is the caller's. */
            if ((*BasAdr((mouse & 0xFF00)
                         | column + 1 + (column >= 72 ? 1 : 0))
                 & 0xFF) == ' ')
                return KEY_MOUSE | buttons;
            tracking = 1;
        }
    }
}

/* Wait for a key with the key bar `bar` on the screen and return it. F1
   is the help, and is not returned. */
u16 Input(u16 bar)
{
    u16 key;

    for (;;) {
        key = (u16)Input0(bar);
        if (key != KEY_F1)
            return key;
        Help00();
    }
}

/* Whether a character is one that words end at: 1 or 0. Only the low byte
   of `symbol` counts. */
u16 CmpSym(u16 symbol)
{
    u16 i;

    for (i = 0; i < sizeof CtEn_T; i++)
        if (CtEn_T[i] == (u8)symbol)
            return 1;
    return 0;
}

/* Where the word begins that the cursor is in or behind: the column for
   the cursor at `cursor` in the line `line`. */
u16 EdSub1(const u8 *line, u16 cursor)
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
static u16 EdSub2(const u8 *line, u16 cursor)
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

/* Have a line that is typed into take a key: `text` is the line, in the
   main data, with room for `length` characters and a zero byte, and
   `cursor` the column of the cursor in it. Returns the column of the
   cursor then, and 1 in the high word if the line or the cursor has
   changed.

   Ctrl-Q has the next key typed as it is, whatever it does otherwise;
   Quote remembers that from one call to the next. */
u32 EdLin(u16 key, u8 *text, u16 cursor, u16 length)
{
#define CHANGED(column)   (0x10000UL | (u16)(column))
#define UNCHANGED(column) ((u32)(u16)(column))
    u16 low = key & 0xFF;
    u16 from, count;
    u8 *to;

    if (md.Quote != 0) {                /* E00_03 */
        md.Quote = 0;
        if (low == 0)
            return UNCHANGED(cursor);
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
                return UNCHANGED(cursor);
            }
        }
        if (low < ' ')
            return UNCHANGED(cursor);
    } else if (low == 0) {
        return UNCHANGED(cursor);
    }
E00_01:                                 /* a character */
    if (cursor >= length) {
        Beep();
        return UNCHANGED(cursor);
    }
    to = text + length;
    for (count = length - cursor; count != 0; count--) {
        *to = to[-1];
        to--;
    }
    text[length] = 0;
    text[cursor] = (u8)low;
    return CHANGED(cursor + 1);

E01_00:                                 /* Left, Ctrl-S */
    if (cursor == 0)
        return UNCHANGED(cursor);
    return CHANGED(cursor - 1);
E02_00:                                 /* Right, Ctrl-D */
    if (text[cursor] == 0)
        return UNCHANGED(cursor);
    return CHANGED(cursor + 1);
E03_00:                                 /* Ctrl-Left, Ctrl-A */
    return CHANGED(EdSub1(text, cursor));
E04_00:                                 /* Ctrl-Right, Ctrl-F */
    return CHANGED(EdSub2(text, cursor));
E05_00:                                 /* Home */
    return CHANGED(0);
E06_00:                                 /* End: where the text ends, looked
                                           for in the line and one more */
    cursor = 0;
    for (count = length + 1; count != 0; count--)
        if (text[cursor++] == 0)
            break;
    return CHANGED(cursor - 1);
E07_00:                                 /* Backspace */
    if (cursor == 0)
        return UNCHANGED(cursor);
    cursor--;
E08_00:                                 /* Del, Ctrl-G */
    if (text[cursor] == 0)
        return UNCHANGED(cursor);
    from = cursor + 1;
    goto E00_05;
E09_00:                                 /* Ctrl-Y: all of the line */
    cursor = 0;
E10_00:                                 /* Ctrl-K: the rest of it */
    text[cursor] = 0;
    return CHANGED(cursor);
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
    return CHANGED(cursor);
#undef CHANGED
#undef UNCHANGED
}

/* The speed search: look for the next name that begins with what has
   been typed so far and the key `key`. The names are in the segment
   `segment`, one every `step` bytes from `begin` to `end`; `at` is the
   one that the cursor is on. Ctrl-Enter, a line feed, types nothing and
   looks from the name behind that one. Returns the name found, or `at`
   if there is none, and then what has been typed stays as it was.

   What has been typed is at MenuBuf+228: the column of the cursor in a
   word and a line of twelve characters, which takes the key as any line
   does. It is tried at MenuBuf+208 and kept if a name fits it, and the
   capitals of it are at MenuBuf. */
u16 Search(u16 segment, u16 at, u16 begin, u16 end, u16 step, u16 key)
{
    u8 *tried = md.MenuBuf + 208;
    u8 *typed = md.MenuBuf + 228;
    const u8 __far *name;
    u16 from = at;
    u16 length, i;

    if ((key & 0xFF) == 0x0A) {
        at += step;
        if (at >= end)
            at = begin;
    }
    for (i = 0; i < 15; i++)
        tried[i] = typed[i];
    if ((key & 0xFF) != 0x0A)
        *(u16 *)tried = (u16)EdLin(key, tried + 2, *(u16 *)tried, 12);
    length = (u16)(u32)MvzLine(md.MenuBuf, tried + 2, 13)
             - (u16)(u8 *)md.MenuBuf;
    PathLin(md.MenuBuf);
    if (length != 0) {
        if (end <= begin)
            return at;
        for (;;) {
            name = (const u8 __far *)((u32)segment << 16 | at);
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
