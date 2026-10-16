/*
 * The keys of a hardware keyboard on iOS, made what the program for
 * Linux takes from a terminal with the kitty protocol of the keyboard:
 * the piece I3 of step 8 (CLAUDE.md, "The app for iOS"; TODO.md, "Step
 * 8, the app for iOS"). The shell in SwiftUI hands each UIPress of
 * pressesBegan and pressesEnded that has a UIKey to IosKeyBytes, and the
 * bytes that it answers to the program, which decodes them with the
 * decoder of the terminal, keyboard_posix.c, into the words of the
 * bench's BIOS and what is held. So there is one decoder, as for the web
 * page, whose src/portable_c/web/keys.js does for a KeyboardEvent what
 * keys_ios.c does for a UIKey; the test portable_c-ios-keys,
 * harness/ioskeys.py, holds the two files to the same words.
 *
 * Plain C, with no type of UIKit: the shell takes the numbers and the
 * texts out of the UIKey and fills a struct IosKey.
 */
#ifndef KEYS_IOS_H
#define KEYS_IOS_H

#include "vc.h"

/* The bits of UIKeyModifierFlags, as UIKit has them. */
#define IOS_ALPHA_SHIFT 0x10000u        /* Caps Lock */
#define IOS_SHIFT 0x20000u
#define IOS_CONTROL 0x40000u
#define IOS_ALTERNATE 0x80000u          /* Option */
#define IOS_COMMAND 0x100000u
#define IOS_NUMERIC_PAD 0x200000u

/* What became of the key: pressesBegan, a repeat of it that the shell
   makes while the key is held (UIKit sends none), or pressesEnded and
   pressesCancelled. The numbers are kitty's. */
#define IOS_PRESS 1
#define IOS_REPEAT 2
#define IOS_RELEASE 3

/* A key event: of the UIKey, keyCode, which is the usage of the HID's
   page of the keyboard (UIKeyboardHIDUsage), modifierFlags.rawValue, and
   characters and charactersIgnoringModifiers in UTF-8, each ended by a
   zero byte, "" where UIKit has an empty string. */
struct IosKey {
    i32 usage;
    u32 modifiers;                      /* bits: the IOS_ above */
    const char *characters;
    const char *ignoring;
    i32 event;                          /* IOS_PRESS and the others */
};

/* What is kept from one event to the next, which UIKit does not say
   with a key: which Option is held. The left one is Alt; the right one
   is a shift of the layout, as AltGraph is on a PC, and a key with it
   types what the layout has there, the text of `characters`, with no
   Alt held. The shell keeps one, zeros at the start. */
struct IosKeys {
    bool leftOption;
    bool rightOption;
};

/* Room enough for the bytes of any key. */
#define IOS_KEY_ROOM 64

/* The bytes of kitty's for `key`, at `to`, which has room for `room`
   bytes, IOS_KEY_ROOM at the least, with no zero byte behind them:
   answers how many, or 0 where the program has nothing to do with the
   key, which the shell then hands on to UIKit (super). A key with
   Command held is left so, Command itself too: those are the system's
   and the shell's. */
i32 IosKeyBytes(struct IosKeys *keys, const struct IosKey *key, char *to,
                i32 room);

/* The bytes that a terminal sends when it has lost the focus, which make
   the program let go what it takes to be held, since the releases go
   elsewhere: for the shell to hand on when it is no longer the first
   responder, or the app leaves the foreground. Clears what `keys` holds.
   Answers how many, as IosKeyBytes. */
i32 IosKeysLost(struct IosKeys *keys, char *to, i32 room);

#endif
