#!/usr/bin/env python3
"""Hold the table of the keys of iOS to the program's decoder of keys.

The piece I3 of step 8 (TODO.md, "Step 8, the app for iOS"): the shell
of the app hands each key event of a hardware keyboard, a UIKey with its
usage of the HID, its modifiers and its characters, to
src/portable_c/keys_ios.c, which makes of it the escape code of a
terminal with the kitty protocol of the keyboard, as keys.js does for a
web page; and the program decodes that with the terminal's decoder,
keyboard_posix.c. This test runs the table on Linux, built by itself as
build/portable_c/ios-keys/ioskeys (harness/ioskeys.c), with key events
of the shape that UIKit sends, of a US, a German and a Russian layout of
the Mac and of the keypad, with Shift, Control, Option and Caps Lock;
types the bytes into the program for Linux in a pseudo-terminal that has
the kitty protocol (terminal.Terminal), each case followed by Esc twice;
and holds what the program reads, by VC_TRACE, to the BIOS's words of the
bench, as harness/webrender.py holds keys.js and terminal.py's
DECODE_KITTY holds kitty's own codes. Keys with Command are held to be
left to UIKit, no bytes at all.

The test exits with 77, which the build takes for a test that is
skipped, where the program or the table has not been built (no clang).
"""

import argparse
import subprocess
import sys
from pathlib import Path

import dos
import terminal

SKIPPED = 77

# UIKeyModifierFlags.
CAPS, SHIFT, CTRL, OPTION, COMMAND, PAD = (0x10000, 0x20000, 0x40000,
                                           0x80000, 0x100000, 0x200000)
PRESS, REPEAT, RELEASE = 1, 2, 3

# Usages of the HID's page of the keyboard.
A, E, L, Q, Y = 0x04, 0x08, 0x0F, 0x14, 0x1C
DIGIT = {str(n): 0x1E + (n - 1) % 10 for n in range(1, 11)}
DIGIT["0"] = 0x27
ENTER, ESCAPE, BACKSPACE, TAB, SPACE = 0x28, 0x29, 0x2A, 0x2B, 0x2C
MINUS, EQUAL, BRACKET, SEMICOLON, SLASH = 0x2D, 0x2E, 0x2F, 0x33, 0x38
CAPS_LOCK = 0x39
F = {n: 0x3A + n - 1 for n in range(1, 13)}
PRINT_SCREEN, INSERT, HOME, PAGE_UP, DELETE, END, PAGE_DOWN = (
    0x46, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E)
UP = 0x52
CLEAR = 0x53
PAD_DIVIDE, PAD_MULTIPLY, PAD_MINUS, PAD_PLUS, PAD_ENTER = (
    0x54, 0x55, 0x56, 0x57, 0x58)
PAD_KEY = {str(n): 0x59 + n - 1 for n in range(1, 10)}
PAD_KEY["0"], PAD_KEY["."] = 0x62, 0x63
PAD_EQUAL, F13 = 0x67, 0x68
LEFT_CONTROL, LEFT_SHIFT, LEFT_OPTION, LEFT_COMMAND = 0xE0, 0xE1, 0xE2, 0xE3
RIGHT_OPTION = 0xE6

ARROW_UP = "UIKeyInputUpArrow"


def event(kind, usage, characters, ignoring=None, mods=0):
    """A key event as the shell hands it on."""
    return (kind, usage, mods, characters,
            characters if ignoring is None else ignoring)


def down_up(usage, characters, ignoring=None, mods=0):
    """A key pressed and let go, with what is held."""
    return [event(PRESS, usage, characters, ignoring, mods),
            event(RELEASE, usage, characters, ignoring, mods)]


def held(usage, flag, inner, under=0):
    """`inner` with a modifier pressed in front of it and let go behind
    it, as UIKit has them: the modifier's own press with its flag set,
    its release with it cleared; `under` is what is held around it."""
    return ([event(PRESS, usage, "", "", under | flag)] + inner
            + [event(RELEASE, usage, "", "", under)])


# Each case: what it is, the events, and the BIOS's words that the program
# is to read of them (the bench's, as terminal.DECODE_KITTY has them).
KEYS = [
    # A US layout.
    ("a", down_up(A, "a"), ["1E61"]),
    ("Shift-A", held(LEFT_SHIFT, SHIFT, down_up(A, "A", "a", SHIFT)),
     ["1E41"]),
    ("A with Caps Lock on", down_up(A, "A", "a", CAPS), ["1E41"]),
    ("Shift-1, !", down_up(DIGIT["1"], "!", "1", SHIFT), ["0221"]),
    ("Shift-1, ! where charactersIgnoringModifiers keeps Shift",
     down_up(DIGIT["1"], "!", "!", SHIFT), ["0221"]),
    ("a, held: a repeat that the shell makes",
     [event(PRESS, A, "a"), event(REPEAT, A, "a"), event(RELEASE, A, "a")],
     ["1E61", "1E61"]),
    ("é on the key of e, which 437 has", down_up(E, "é"), ["1282"]),
    ("Ctrl-A", held(LEFT_CONTROL, CTRL, down_up(A, "\x01", "a", CTRL)),
     ["1E01"]),
    ("Ctrl-1", down_up(DIGIT["1"], "1", "1", CTRL), []),
    ("Ctrl-2", down_up(DIGIT["2"], "\x00", "2", CTRL), ["0300"]),
    ("Ctrl-[", down_up(BRACKET, "\x1b", "[", CTRL), ["1A1B"]),
    ("the left Option, Alt, with a: å on the Mac, Alt-A",
     held(LEFT_OPTION, OPTION, down_up(A, "å", "a", OPTION)), ["1E00"]),
    ("Alt-[", down_up(BRACKET, "“", "[", OPTION), ["1A00"]),
    ("Alt-1", down_up(DIGIT["1"], "¡", "1", OPTION), ["7800"]),
    ("Alt-=", down_up(EQUAL, "≠", "=", OPTION), ["8300"]),
    ("Alt-Space, a space that does not break", down_up(SPACE, "\xa0", " ",
                                                       OPTION), ["3920"]),
    ("Alt alone: the menu bar", held(LEFT_OPTION, OPTION, []), []),
    ("Up", down_up(UP, ARROW_UP), ["4800"]),
    ("Ctrl-Up", down_up(UP, ARROW_UP, mods=CTRL), ["8D00"]),
    ("Alt-Up", held(LEFT_OPTION, OPTION, down_up(UP, ARROW_UP,
                                                 mods=OPTION)), ["9800"]),
    ("Home", down_up(HOME, "UIKeyInputHome"), ["4700"]),
    ("End", down_up(END, "UIKeyInputEnd"), ["4F00"]),
    ("PgDn", down_up(PAGE_DOWN, "UIKeyInputPageDown"), ["5100"]),
    ("Ctrl-PgUp", down_up(PAGE_UP, "UIKeyInputPageUp", mods=CTRL),
     ["8400"]),
    ("Ins", down_up(INSERT, ""), ["5200"]),
    ("Del, the Mac's forward delete", down_up(DELETE, "UIKeyInputDelete"),
     ["5300"]),
    ("Alt-Del", down_up(DELETE, "UIKeyInputDelete", mods=OPTION), []),
    ("Ctrl-Del", down_up(DELETE, "UIKeyInputDelete", mods=CTRL), ["9300"]),
    ("F1", down_up(F[1], "UIKeyInputF1"), ["3B00"]),
    ("Shift-F1", down_up(F[1], "UIKeyInputF1", mods=SHIFT), ["5400"]),
    ("Alt-F1", held(LEFT_OPTION, OPTION, down_up(F[1], "UIKeyInputF1",
                                                 mods=OPTION)), ["6800"]),
    ("F3", down_up(F[3], "UIKeyInputF3"), ["3D00"]),
    ("Ctrl-F4", down_up(F[4], "UIKeyInputF4", mods=CTRL), ["6100"]),
    ("F10", down_up(F[10], "UIKeyInputF10"), ["4400"]),
    ("F11", down_up(F[11], "UIKeyInputF11"), ["8500"]),
    ("Shift-F12", down_up(F[12], "UIKeyInputF12", mods=SHIFT), ["8800"]),
    ("Alt-Esc", down_up(ESCAPE, "UIKeyInputEscape", mods=OPTION), ["0100"]),
    ("Enter", down_up(ENTER, "\r"), ["1C0D"]),
    ("Ctrl-Enter", down_up(ENTER, "\r", mods=CTRL), ["1C0A"]),
    ("Tab", down_up(TAB, "\t"), ["0F09"]),
    ("Shift-Tab", down_up(TAB, "\t", mods=SHIFT), ["0F00"]),
    ("Ctrl-Tab", down_up(TAB, "\t", mods=CTRL), ["9400"]),
    ("Backspace, the Mac's delete", down_up(BACKSPACE, "\b"), ["0E08"]),
    ("Alt-Backspace", down_up(BACKSPACE, "\b", mods=OPTION), ["0E00"]),
    ("Ctrl-Backspace", down_up(BACKSPACE, "\b", mods=CTRL), ["0E7F"]),
    ("F13", down_up(F13, ""), []),
    ("Print Screen", down_up(PRINT_SCREEN, ""), []),
    ("Shift alone", held(LEFT_SHIFT, SHIFT, []), []),
    ("Caps Lock, the key", [event(PRESS, CAPS_LOCK, "", "", CAPS),
                            event(RELEASE, CAPS_LOCK, "", "", CAPS)], []),
    ("the left Option held, the focus lost, then a: no Alt held",
     [event(PRESS, LEFT_OPTION, "", "", OPTION), "lost",
      event(PRESS, A, "a"), event(RELEASE, A, "a")], ["1E61"]),

    # A German layout of the Mac: Y and Z changed round, and @ [ ] { } |
    # \ ~ on Option, which the right Option types as AltGraph would.
    ("z on the key of y", down_up(Y, "z"), ["157A"]),
    ("ö on the key of ;", down_up(SEMICOLON, "ö"), ["2794"]),
    ("ß on the key of -", down_up(MINUS, "ß"), ["0CE1"]),
    ("Shift-ß, ? on the key of -", down_up(MINUS, "?", "ß", SHIFT),
     ["0C3F"]),
    ("@ by the right Option with l",
     held(RIGHT_OPTION, OPTION, down_up(L, "@", "l", OPTION)), ["2640"]),
    ("the right Option held for two: [ and ] on 5 and 6",
     held(RIGHT_OPTION, OPTION, down_up(DIGIT["5"], "[", "5", OPTION)
          + down_up(DIGIT["6"], "]", "6", OPTION)), ["065B", "075D"]),
    ("\\ by Shift and the right Option with 7",
     held(LEFT_SHIFT, SHIFT, held(RIGHT_OPTION, OPTION, down_up(
         DIGIT["7"], "\\", "7", SHIFT | OPTION), SHIFT)), ["085C"]),
    ("the right Option alone: no Alt, no menu bar",
     held(RIGHT_OPTION, OPTION, []), []),
    ("F1 with the right Option: AltGraph holds no Alt",
     held(RIGHT_OPTION, OPTION, down_up(F[1], "UIKeyInputF1",
                                        mods=OPTION)), ["3B00"]),
    ("the left Option with l: Alt-L, though Option types @ there",
     held(LEFT_OPTION, OPTION, down_up(L, "@", "l", OPTION)), ["2600"]),
    ("both Options held, with l: Alt-L",
     held(LEFT_OPTION, OPTION, held(RIGHT_OPTION, OPTION, down_up(
         L, "@", "l", OPTION), OPTION)), ["2600"]),
    ("Ctrl-Z on the key of y: the key of the US layout",
     down_up(Y, "\x1a", "z", CTRL), ["1519"]),

    # A Russian layout of the Mac.
    ("ф on the key of a, which 437 lacks", down_up(A, "ф"), []),
    ("Shift-Ф", down_up(A, "Ф", "ф", SHIFT), []),
    ("Ф with Caps Lock on", down_up(A, "Ф", "ф", CAPS), []),
    ("Alt with ф: Alt-A", down_up(A, "ƒ", "ф", OPTION), ["1E00"]),
    ("Ctrl with ф: Ctrl-A", down_up(A, "\x01", "ф", CTRL), ["1E01"]),
    (". on the key of /", down_up(SLASH, "."), ["352E"]),
    (", by Shift on the key of /", down_up(SLASH, ",", ".", SHIFT),
     ["352C"]),
    ("Alt with ю, the key of .: Alt-.", down_up(0x37, "≥", "ю", OPTION),
     ["3400"]),

    # The keypad, which types its figures always: Num Lock on.
    ("the keypad's 0", down_up(PAD_KEY["0"], "0", mods=PAD), ["5230"]),
    ("its 7", down_up(PAD_KEY["7"], "7", mods=PAD), ["4737"]),
    ("Shift with it: Home", down_up(PAD_KEY["7"], "7", mods=PAD | SHIFT),
     ["4700"]),
    ("its 5", down_up(PAD_KEY["5"], "5", mods=PAD), ["4C35"]),
    ("Ctrl with it", down_up(PAD_KEY["5"], "5", mods=PAD | CTRL), ["8F00"]),
    ("its .", down_up(PAD_KEY["."], ".", mods=PAD), ["532E"]),
    ("its 8 where UIKit gives no figure: Up",
     down_up(PAD_KEY["8"], "", mods=PAD), ["4800"]),
    ("its /", down_up(PAD_DIVIDE, "/", mods=PAD), ["352F"]),
    ("Ctrl with its *", down_up(PAD_MULTIPLY, "*", mods=PAD | CTRL),
     ["9600"]),
    ("Alt with its -", down_up(PAD_MINUS, "-", mods=PAD | OPTION),
     ["4A00"]),
    ("its +: the group to select", down_up(PAD_PLUS, "+", mods=PAD),
     ["4E2B"]),
    ("its Enter", down_up(PAD_ENTER, "\r", mods=PAD), ["1C0D"]),
    ("Alt with its Enter", down_up(PAD_ENTER, "\r", mods=PAD | OPTION),
     ["A600"]),
    ("its =, which the BIOS has no word for",
     down_up(PAD_EQUAL, "=", mods=PAD), []),
    ("Clear, where a PC has Num Lock", down_up(CLEAR, "", mods=PAD), []),
]

# What the table leaves to UIKit: no bytes for any of the events.
LEFT = [
    ("Command alone", held(LEFT_COMMAND, COMMAND, [])),
    ("Command-A", down_up(A, "a", mods=COMMAND)),
    ("Command-Tab", down_up(TAB, "\t", mods=COMMAND)),
    ("a key that is none, the usage 0", down_up(0, "")),
    ("the ISO key of < on a US layout, with nothing typed",
     down_up(0x64, "")),
]


def line(step):
    if step == "lost":
        return "lost"
    kind, usage, mods, characters, ignoring = step
    return " ".join([str(kind), f"{usage:x}", f"{mods:x}",
                     characters.encode().hex() or "-",
                     ignoring.encode().hex() or "-"])


def made(table, events):
    """The table's bytes for each event, None for none, with one state
    for all of them, as the shell keeps one."""
    answered = subprocess.run(
        [str(table)], input="".join(line(step) + "\n" for step in events),
        capture_output=True, text=True, check=True).stdout.splitlines()
    return [None if bytes_ == "-" else bytes.fromhex(bytes_)
            for bytes_ in answered]


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("table", type=Path,
                        help="the table built for Linux, ios-keys/ioskeys")
    parser.add_argument("program", type=Path,
                        help="the program for Linux, posix-asan/vc")
    args = parser.parse_args()
    if not args.table.exists() or not args.program.exists():
        print(f"{args.table} or {args.program} is not there: the test is "
              "skipped")
        sys.exit(SKIPPED)
    failures = []
    events = [step for _, steps, _ in KEYS for step in steps]
    answers = iter(made(args.table, events))
    data = []
    for what, steps, _ in KEYS:
        case = b""
        for step in steps:
            answer = next(answers)
            if answer is None and step != "lost":
                failures.append(f"{what}: {step} is left to UIKit")
            case += answer or b""
        data.append(case)
    for what, steps in LEFT:
        for step, answer in zip(steps, made(args.table, steps)):
            if answer is not None:
                failures.append(f"{what}: {step} is made {answer!r}, where "
                                "it is to be left to UIKit")
    pyte = terminal.load_pyte()
    with dos.workdir() as top:
        work = Path(top)
        home, here = terminal.lay_out(work / "keys")
        term = terminal.Terminal(pyte, args.program.resolve(), home, here,
                                 work, 80, 25, "ios-keys", kitty=True)
        term.read()
        esc = terminal.CSI + b"27u" + terminal.CSI + b"27;1:3u"
        for case in data:
            term.type(case, 0.15)
            term.type(esc, 0.15)
            term.type(esc, 0.15)
        have = [text for text in term.traced() if text.startswith("key ")]
        want = []
        for _, _, words in KEYS:
            want += [f"key {word}" for word in words] + ["key 011B"] * 2
        if have != want:
            at = next((i for i, (a, b) in enumerate(zip(have, want))
                       if a != b), min(len(have), len(want)))
            failures.append(f"the keys: the program reads "
                            f"{have[at:at + 6]} from the {at + 1}th word "
                            f"on, where it was to read {want[at:at + 6]}")
            for (what, _, words), case in zip(KEYS, data):
                print(f"  {what}: {case!r} for {words}")
        terminal.ended("the keys", term, failures)
    for failure in failures:
        print(failure)
    if failures:
        sys.exit(1)
    print(f"{len(KEYS)} cases of UIKit's key events made kitty's escape "
          "codes and read by the program as the bench's BIOS answers them; "
          f"{len(LEFT)} left to UIKit")


if __name__ == "__main__":
    main()
