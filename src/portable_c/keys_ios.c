/*
 * The keys of a hardware keyboard on iOS made kitty's escape codes: see
 * keys_ios.h. What follows is src/portable_c/web/keys.js, for a UIKey
 * in place of a KeyboardEvent: an event of kitty's is the key, as kitty
 * numbers it (the character that the key types unshifted on the layout
 * in use, or a code of kitty's own for a key that types none), the
 * character that it types shifted, the key of a US layout in the same
 * place, the modifiers, the kind of event, the text, and the final byte
 * of its sequence, which is `u` but for the keys that keep the sequences
 * of xterm. It is written as kitty would send it with the flags that the
 * program pushes, 31.
 *
 * Where a UIKey says less than a KeyboardEvent:
 *
 * - The place of a key is the usage of the HID's page of the keyboard,
 *   where the browser has `code`; the two name the same places, and the
 *   US layout's character at each is the same.
 * - UIKit does not say which Option is held while another key is
 *   pressed, so the presses of the two are kept (struct IosKeys). The
 *   left one is Alt, the right one AltGraph: a shift of the layout, as
 *   kitty's ISO_LEVEL3_SHIFT is, under which a key types what the layout
 *   has there and no Alt is held. That is how a layout of the Mac, a
 *   German one, types @ [ ] { } | \ ~, which are on Option there. Both
 *   could not be both: a key that types a character under Option would
 *   then be text by one rule and Alt by the other, and Alt alone, which
 *   shows the key bar of Alt and opens the menu bar when it is tapped,
 *   could not be told from the shift of a character that is to come.
 *   Terminals of the Mac, iTerm2 and Terminal, let either Option be Meta
 *   in the same way.
 * - There is no Num Lock: the keypad of a Mac types its figures always,
 *   which is Num Lock on. Shift turns it round, as the bench's BIOS has
 *   it and the decoder makes it.
 * - Command is the system's and the shell's (Command-Tab, Command-H, the
 *   shell's own commands): a key with it is left to UIKit, where keys.js
 *   hands the browser's Meta on as kitty's Super.
 * - UIKit repeats no key; the shell does, as IOS_REPEAT.
 */
#include <string.h>

#include "emulate_posix.h"
#include "keys_ios.h"

/* kitty's modifiers, as bits of the number less 1. */
#define K_SHIFT 1
#define K_ALT 2
#define K_CTRL 4
#define K_CAPS_LOCK 64

/* kitty's codes of the keys that are no character. */
#define K_CAPS_LOCK_KEY 57358
#define K_SCROLL_LOCK 57359
#define K_NUM_LOCK 57360
#define K_PRINT_SCREEN 57361
#define K_PAUSE 57362
#define K_MENU 57363
#define K_F13 57376
#define K_LEFT_SHIFT 57441
#define K_LEFT_CONTROL 57442
#define K_LEFT_ALT 57443
#define K_LEFT_SUPER 57444
#define K_RIGHT_SHIFT 57447
#define K_RIGHT_CONTROL 57448
#define K_ALT_GRAPH 57453               /* ISO_LEVEL3_SHIFT, no Alt */

/* The usages of the HID's page of the keyboard (0x07) that are named
   below. */
#define U_A 0x04
#define U_Z 0x1D
#define U_1 0x1E
#define U_0 0x27
#define U_ENTER 0x28
#define U_ESCAPE 0x29
#define U_BACKSPACE 0x2A
#define U_TAB 0x2B
#define U_SPACE 0x2C
#define U_CAPS_LOCK 0x39
#define U_F1 0x3A
#define U_F12 0x45
#define U_PRINT_SCREEN 0x46
#define U_SCROLL_LOCK 0x47
#define U_PAUSE 0x48
#define U_NUM_LOCK 0x53                 /* Clear, on a Mac */
#define U_PAD_DIVIDE 0x54
#define U_PAD_1 0x59
#define U_PAD_0 0x62
#define U_PAD_DECIMAL 0x63
#define U_MENU 0x65
#define U_PAD_EQUAL 0x67
#define U_F13 0x68
#define U_F24 0x73
#define U_PAD_COMMA 0x85
#define U_LEFT_CONTROL 0xE0
#define U_LEFT_SHIFT 0xE1
#define U_LEFT_ALT 0xE2
#define U_LEFT_GUI 0xE3
#define U_RIGHT_CONTROL 0xE4
#define U_RIGHT_SHIFT 0xE5
#define U_RIGHT_ALT 0xE6
#define U_RIGHT_GUI 0xE7

/* An event of kitty's, as above. */
struct Kitty {
    u32 code;
    u32 shifted;
    u32 base;
    i32 mods;
    i32 event;
    u32 text;
    char final;
};

/* The keys that keep the sequences of xterm: the number and the final
   byte. */
static bool Legacy(i32 usage, struct Kitty *tuple)
{
    static const struct { u8 usage; u8 number; char final; } Table[] = {
        { 0x52, 1, 'A' }, { 0x51, 1, 'B' }, { 0x4F, 1, 'C' },
        { 0x50, 1, 'D' }, { 0x4A, 1, 'H' }, { 0x4D, 1, 'F' },
        { 0x49, 2, '~' }, { 0x4C, 3, '~' }, { 0x4B, 5, '~' },
        { 0x4E, 6, '~' },
        { 0x3A, 1, 'P' }, { 0x3B, 1, 'Q' }, { 0x3C, 13, '~' },
        { 0x3D, 1, 'S' }, { 0x3E, 15, '~' }, { 0x3F, 17, '~' },
        { 0x40, 18, '~' }, { 0x41, 19, '~' }, { 0x42, 20, '~' },
        { 0x43, 21, '~' }, { 0x44, 23, '~' }, { 0x45, 24, '~' }
    };
    size_t i;

    for (i = 0; i < sizeof Table / sizeof Table[0]; i++)
        if (Table[i].usage == usage) {
            tuple->code = Table[i].number;
            tuple->final = Table[i].final;
            return true;
        }
    return false;
}

/* The keys that type no character, as kitty numbers them; 0 for the
   others. */
static u32 Functional(i32 usage)
{
    switch (usage) {
    case U_ESCAPE: return 27;
    case U_ENTER: return 13;
    case U_TAB: return 9;
    case U_BACKSPACE: return 127;
    case U_CAPS_LOCK: return K_CAPS_LOCK_KEY;
    case U_SCROLL_LOCK: return K_SCROLL_LOCK;
    case U_NUM_LOCK: return K_NUM_LOCK;
    case U_PRINT_SCREEN: return K_PRINT_SCREEN;
    case U_PAUSE: return K_PAUSE;
    case U_MENU: return K_MENU;
    case U_LEFT_SHIFT: return K_LEFT_SHIFT;
    case U_LEFT_CONTROL: return K_LEFT_CONTROL;
    case U_LEFT_ALT: return K_LEFT_ALT;
    case U_RIGHT_SHIFT: return K_RIGHT_SHIFT;
    case U_RIGHT_CONTROL: return K_RIGHT_CONTROL;
    case U_RIGHT_ALT: return K_ALT_GRAPH;
    }
    if (usage >= U_F13 && usage <= U_F24)
        return (u32)(K_F13 + usage - U_F13);
    return 0;
}

/* The keypad: kitty's code with Num Lock on, and with it off, from KP_1
   to KP_0 and KP_DECIMAL by the order of the usages. */
static const u16 PadOn[] = { 57400, 57401, 57402, 57403, 57404, 57405,
                             57406, 57407, 57408, 57399, 57409 };
static const u16 PadOff[] = { 57424 /* End */, 57420 /* Down */,
                              57422 /* PgDn */, 57417 /* Left */,
                              57427 /* Begin */, 57418 /* Right */,
                              57423 /* Home */, 57419 /* Up */,
                              57421 /* PgUp */, 57425 /* Ins */,
                              57426 /* Del */ };

/* The signs of the keypad: / * - + Enter, by their usages from 54h on,
   and = and the comma. */
static u32 PadSign(i32 usage)
{
    if (usage >= U_PAD_DIVIDE && usage < U_PAD_1)
        return (u32)(57410 + usage - U_PAD_DIVIDE);
    if (usage == U_PAD_EQUAL)
        return 57415;
    if (usage == U_PAD_COMMA)
        return 57416;
    return 0;
}

/* The character of a US layout at a key that types one, by its usage;
   0 for the others. The usage 32h, the key of # and ~ of an ISO
   keyboard, is the browser's Backslash too. */
static u32 Us(i32 usage)
{
    static const char Signs[] = "-=[]\\\\;'`,./";   /* from 2Dh */

    if (usage >= U_A && usage <= U_Z)
        return (u32)('a' + usage - U_A);
    if (usage >= U_1 && usage < U_0)
        return (u32)('1' + usage - U_1);
    if (usage == U_0)
        return '0';
    if (usage == U_SPACE)
        return ' ';
    if (usage >= 0x2D && usage <= 0x38)
        return (u8)Signs[usage - 0x2D];
    return 0;
}

/* The one character of `text`, or 0 where it is empty, more than one,
   or no UTF-8. */
static u32 Single(const char *text)
{
    size_t length = text == NULL ? 0 : strlen(text);
    u32 character = 0;

    if (length == 0 || length > 4)
        return 0;
    if (Utf8Decode(text, (i32)length, &character) != (i32)length)
        return 0;
    return character;
}

/* The small letter of a character, for the letters of ASCII, Latin-1,
   Greek and Cyrillic; any other character is itself. */
static u32 Small(u32 character)
{
    if (character >= 0x410 && character <= 0x42F)
        return character + 0x20;
    if (character >= 0x400 && character <= 0x40F)
        return character + 0x50;
    return UnicodeSmall(character);
}

/* Write `number` in decimal at `to`; answers how many bytes. */
static i32 Decimal(u32 number, char *to)
{
    char digits[10];
    i32 count = 0, i;

    do {
        digits[count++] = (char)('0' + number % 10);
        number /= 10;
    } while (number != 0);
    for (i = 0; i < count; i++)
        to[i] = digits[count - 1 - i];
    return count;
}

/* The bytes that kitty sends for an event, at `to`, which has room for
   IOS_KEY_ROOM; answers how many. As `sequence` of keys.js. */
static i32 Sequence(const struct Kitty *tuple, char *to)
{
    i32 at = 0;
    bool mods = tuple->mods != 0 || tuple->event != IOS_PRESS;

    to[at++] = '\x1B';
    to[at++] = '[';
    if (tuple->final != 'u') {
        /* xterm's form: CSI A, CSI 5~, or with what is held CSI 1;5A. */
        if (!mods) {
            if (tuple->code != 1)
                at += Decimal(tuple->code, to + at);
            to[at++] = tuple->final;
            return at;
        }
        at += Decimal(tuple->code, to + at);
        to[at++] = ';';
        at += Decimal((u32)tuple->mods + 1, to + at);
        if (tuple->event != IOS_PRESS) {
            to[at++] = ':';
            at += Decimal((u32)tuple->event, to + at);
        }
        to[at++] = tuple->final;
        return at;
    }
    at += Decimal(tuple->code, to + at);
    if (tuple->shifted != 0 || tuple->base != 0) {
        to[at++] = ':';
        if (tuple->shifted != 0)
            at += Decimal(tuple->shifted, to + at);
    }
    if (tuple->base != 0) {
        to[at++] = ':';
        at += Decimal(tuple->base, to + at);
    }
    if (mods || tuple->text != 0)
        to[at++] = ';';
    if (mods) {
        at += Decimal((u32)tuple->mods + 1, to + at);
        if (tuple->event != IOS_PRESS) {
            to[at++] = ':';
            at += Decimal((u32)tuple->event, to + at);
        }
    }
    if (tuple->text != 0) {
        to[at++] = ';';
        at += Decimal(tuple->text, to + at);
    }
    to[at++] = 'u';
    return at;
}

/* The event of kitty's for a key; false where the program has nothing
   to do with it. As `kittyOf` of keys.js. */
static bool KittyOf(struct IosKeys *keys, const struct IosKey *key,
                    struct Kitty *tuple)
{
    u32 flags = key->modifiers, typed, us, unshifted;
    bool graph, down = key->event != IOS_RELEASE;
    i32 at;

    /* Which Option is held. One that is no longer in the flags has been
       let go where the shell did not see it. */
    if (key->usage == U_LEFT_ALT)
        keys->leftOption = down;
    else if (key->usage == U_RIGHT_ALT)
        keys->rightOption = down;
    else if ((flags & IOS_ALTERNATE) == 0)
        keys->leftOption = keys->rightOption = false;
    if ((flags & IOS_COMMAND) != 0 || key->usage == U_LEFT_GUI
        || key->usage == U_RIGHT_GUI)
        return false;
    graph = (flags & IOS_ALTERNATE) != 0 && keys->rightOption
            && !keys->leftOption;

    memset(tuple, 0, sizeof *tuple);
    tuple->final = 'u';
    tuple->event = key->event;
    if ((flags & IOS_SHIFT) != 0)
        tuple->mods |= K_SHIFT;
    if ((flags & IOS_ALTERNATE) != 0 && !graph)
        tuple->mods |= K_ALT;
    if ((flags & IOS_CONTROL) != 0)
        tuple->mods |= K_CTRL;
    if ((flags & IOS_ALPHA_SHIFT) != 0)
        tuple->mods |= K_CAPS_LOCK;

    if (Legacy(key->usage, tuple))
        return true;
    tuple->code = Functional(key->usage);
    if (tuple->code != 0)
        return true;
    tuple->code = PadSign(key->usage);
    if (tuple->code == 0 && key->usage >= U_PAD_1
        && key->usage <= U_PAD_DECIMAL) {
        /* A key of the keypad that types no figure is one with Num Lock
           off. */
        at = key->usage - U_PAD_1;
        tuple->code = Single(key->characters) != 0 ? PadOn[at] : PadOff[at];
    }
    if (tuple->code != 0) {
        typed = Single(key->characters);
        if (down && typed >= 0x20 && (tuple->mods & (K_ALT | K_CTRL)) == 0)
            tuple->text = typed;
        return true;
    }

    /* A key that types a character. Under Alt or Ctrl the characters are
       what those make of it, a letter of the Mac's Option or a control
       code, and the key is taken by what it types without them. */
    us = Us(key->usage);
    if ((tuple->mods & (K_ALT | K_CTRL)) != 0 && Single(key->ignoring) != 0)
        typed = Single(key->ignoring);
    else
        typed = Single(key->characters);
    if (typed == 0 && us == 0)
        return false;
    if (typed < 0x20 || typed == 0x7F) {
        /* Nothing typed, a dead key, or a control code: the key of the
           US layout. */
        if (us == 0)
            return false;
        tuple->code = us;
    } else {
        /* What the key types unshifted on this layout: what UIKit says
           it types without the modifiers, or else the small letter of
           what it types. */
        unshifted = Single(key->ignoring);
        unshifted = Small(unshifted != 0 ? unshifted : typed);
        if ((tuple->mods & K_SHIFT) != 0 && unshifted == typed && us != 0)
            /* Shift with a key that is no letter, where what it types
               unshifted on this layout is not told: the US layout's. */
            tuple->code = us;
        else
            tuple->code = unshifted;
        if ((tuple->mods & K_SHIFT) != 0 && typed != tuple->code)
            tuple->shifted = typed;
        if (down && (tuple->mods & (K_ALT | K_CTRL)) == 0)
            tuple->text = typed;
    }
    if (us != 0 && us != tuple->code)
        tuple->base = us;
    return true;
}

i32 IosKeyBytes(struct IosKeys *keys, const struct IosKey *key, char *to,
                i32 room)
{
    struct Kitty tuple;

    if (room < IOS_KEY_ROOM || !KittyOf(keys, key, &tuple))
        return 0;
    return Sequence(&tuple, to);
}

i32 IosKeysLost(struct IosKeys *keys, char *to, i32 room)
{
    keys->leftOption = keys->rightOption = false;
    if (room < IOS_KEY_ROOM)
        return 0;
    to[0] = '\x1B';
    to[1] = '[';
    to[2] = 'O';
    return 3;
}
