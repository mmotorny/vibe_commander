"""The driver of the tests, from outside the machine.

Inside the machine a small resident program, DRIVER.ASM, takes Volkov
Commander through the steps of a scenario: it hands the keyboard
controller the scan codes of each key when the program is idle and has
taken the keys before, and writes the screen out when a step has nothing
left to do. What it goes by is a script on a disk of the machine's, and
what it writes QEMU keeps in a file. This module writes the one and reads
the other.

Nothing here or there depends on how fast the host is: the driver acts
when the program asks, and the host only starts the machine and reads
what it left.

Steps are written as key names separated by spaces: a key ("F3", "Enter",
"a"), keys held down together ("Alt+F5", "Ctrl+o"), or a text to type in
double quotes, which may not contain spaces ("readme.txt"). Shift, Ctrl
and Alt go down first, one after the other in the order given, and come
up last, as under a hand, and the program goes round once with each
change before the next. Keys of one name that are not Shift, Ctrl or Alt
go down one behind the other with nothing in between ("Enter+Home"): when
the program takes the first, the others are waiting, which is how a key
gets to a program that looks for keys while it works.

A text with Russian letters is typed on the keys that the Russian
keyboard has them on. It comes out as written only where that keyboard is
loaded and its Cyrillic letters have been selected, with "Alt+RightShift",
and it may hold digits, a full stop, a comma and an underscore besides,
but no Latin letters.

The BIOS's keyboard buffer holds 15 keys, and keys of one name all go
into it at once.

The mouse is written with the cell of the screen it goes to, column and
row from 0: "Click@1,24" moves the pointer there, presses the left
button and lets it go, "RightClick@40,12" the right one,
"MiddleClick@40,12" the middle one, and "Move@40,12" only moves. A button
stays down until the program has asked the mouse driver 20 times where
the mouse is, or as many times as the step says: "Click@1,24*4". Under a
hand that would be thousands of times; what the program does with a
button that is held, it does for as long as it is.

A button that is held while the pointer moves has the places it goes to
one behind the other: "Click@3,5>3,2" presses the left button on column
3 of row 5, moves the pointer to row 2 with the button down, and lets it
go there. The button stays at each place for as long as a click would,
or as that place says: "Click@3,5*4>3,2*100>3,9". The program is not
asked on the way between two places: it finds the pointer at the next.

A row may be any of the 50 that a screen can have, "Click@1,49" for the
key bar of a screen of 50 lines; what is recorded of such a screen is its
first 25 rows all the same. On a screen of 25 lines the mouse driver
stops the pointer at the last row.

The mouse's packets go to the keyboard controller's auxiliary port as
the mouse would send them, and the BIOS and the mouse driver of DOS
make of them where the pointer is and which buttons are down. A mouse
says how far it has moved, not where it is, and the driver moves the
pointer further for a fast mouse than for a slow one. So the pointer is
first taken to the top left corner, by more than the screen is wide and
high, and from there to the middle of its cell in steps of 4, which the
driver takes as they are: 4 dots to the right for 4, 2 dots down for 4,
where a cell is 8 dots each way. From one place to the next with a button
down the pointer goes the same way, by steps of 4, without the corner.
"""
import re
import struct
from dataclasses import dataclass, replace
from pathlib import Path

from dos import COLUMNS, ROWS, MachineError, Screen

# The driver, assembled by the build.
PROGRAM = Path(__file__).resolve().parent.parent / "build" / "DRIVER.COM"
# The file it is on the floppy that DOS boots from.
FILENAME = "DRIVER.COM"
# Where its file has the number of paragraphs it keeps beyond its own.
PAD = 3

# A script (see DRIVER.ASM).
MAGIC = b"\xA5\x5A"
KEYBOARD, MOUSE, STEP, HOLD, ASKS, END = 1, 2, 3, 4, 5, 0
NONE = 0xFFFF
# The disk a script is on: its size is that of the disk the machine had
# when its state was saved.
SCRIPT_DISK = 1 << 20

# What the driver writes.
SCREEN = COLUMNS * ROWS * 2 + 5

NAME = 8                          # bytes of a program's name, as DOS has it
COM_ORIGIN = 0x100                # where a .COM file is loaded
EXE_HEADER = 0x20                 # of the port's VC.EXE, see tools/mkexe.c

# Names for the keys of a US keyboard, as written in scenarios, and what
# QEMU calls them. Letters, digits and F1-F12 name themselves.
KEYS = {
    "Enter": "ret", "Esc": "esc", "Tab": "tab", "Space": "spc",
    "Backspace": "backspace", "Ins": "insert", "Del": "delete",
    "Home": "home", "End": "end", "PgUp": "pgup", "PgDn": "pgdn",
    "Up": "up", "Down": "down", "Left": "left", "Right": "right",
    "GrayPlus": "kp_add", "GrayMinus": "kp_subtract",
    "GrayStar": "kp_multiply",
    "Ctrl": "ctrl", "Alt": "alt", "Shift": "shift", "RightShift": "shift_r",
}
PUNCTUATION = {
    " ": "spc", "-": "minus", "=": "equal", "[": "bracket_left",
    "]": "bracket_right", ";": "semicolon", "'": "apostrophe",
    "`": "grave_accent", "\\": "backslash", ",": "comma", ".": "dot",
    "/": "slash",
}
SHIFTED = dict(zip('!@#$%^&*()_+{}:"~|<>?', "1234567890-=[];'`\\,./"))
MODIFIERS = {"ctrl", "alt", "shift", "shift_r"}

# The Cyrillic letters of the Russian keyboard, by the keys of a US one
# that they are on. With them selected the full stop and the comma are on
# one key, and the digits and the underscore where they always are.
RUSSIAN = dict(zip(
    "йцукенгшщз"
    "хъфывапрол"
    "джэячсмить"
    "бюё",
    "qwertyuiop[]asdfghjkl;'zxcvbnm,.`"))
RUSSIAN_KEYS = {".": ["slash"], ",": ["shift", "slash"],
                "_": ["shift", "minus"]}

# What the keyboard sends for each key when it goes down, as the
# controller hands it on: scan code set 1. Coming up, a key sends the same
# with the top bit set. The keys of EXTENDED send E0h before either.
SCAN = {"esc": 0x01, "minus": 0x0C, "equal": 0x0D, "backspace": 0x0E,
        "tab": 0x0F, "bracket_left": 0x1A, "bracket_right": 0x1B,
        "ret": 0x1C, "ctrl": 0x1D, "semicolon": 0x27, "apostrophe": 0x28,
        "grave_accent": 0x29, "shift": 0x2A, "backslash": 0x2B,
        "comma": 0x33, "dot": 0x34, "slash": 0x35, "shift_r": 0x36,
        "kp_multiply": 0x37, "alt": 0x38, "spc": 0x39,
        "kp_subtract": 0x4A, "kp_add": 0x4E, "f11": 0x57, "f12": 0x58}
SCAN.update({key: 0x02 + at for at, key in enumerate("1234567890")})
SCAN.update({key: 0x10 + at for at, key in enumerate("qwertyuiop")})
SCAN.update({key: 0x1E + at for at, key in enumerate("asdfghjkl")})
SCAN.update({key: 0x2C + at for at, key in enumerate("zxcvbnm")})
SCAN.update({f"f{number}": 0x3A + number for number in range(1, 11)})
EXTENDED = {"insert": 0x52, "delete": 0x53, "home": 0x47, "end": 0x4F,
            "pgup": 0x49, "pgdn": 0x51, "up": 0x48, "down": 0x50,
            "left": 0x4B, "right": 0x4D}


@dataclass(frozen=True)
class Facts:
    """What the driver has to know of a program to watch it."""
    name: bytes                   # its name, as DOS has it
    auto_time: int                # where AutoTim is in the main data
    clock: int                    # where Clock is there
    stack: int = 0                # where the stack begins in its segment
    stack_size: int = 0           # how long it is; 0 for not looking
    # What the stack holds when the program has been loaded: the bytes of
    # the program's file that are loaded there, which the program moves
    # elsewhere as it starts and leaves behind.
    stack_loaded: bytes = b""


def facts(program, stack=False):
    """The Facts of `program`, a setups.Program, from the name of its
    file and from the listing of its assembly. With `stack`, where its
    stack is too."""
    code = program.contents
    name = Path(program.filename).stem.upper().encode("ascii")
    if not program.listing or not Path(program.listing).exists():
        raise RuntimeError(
            f"the driver needs the listing of {program.filename}, which "
            "the build writes next to it")
    text = Path(program.listing).read_text(encoding="cp866")

    def symbol(pattern):
        return int(re.search(pattern, text, re.M)[1], 16)

    found = Facts(name[:NAME].ljust(NAME, b"\0"),
                  symbol(r"^ *AutoTim [ .]*([0-9A-F]+) +DWord"),
                  symbol(r"^ *Clock [ .]*([0-9A-F]+) +Byte"))
    if stack:
        bottom = symbol(r"^DataSav [ .]*Byte\[\d+\] +([0-9A-F]+)h")
        top = symbol(r"^VcStack [ .]*L Near +([0-9A-F]+)h")
        at = bottom - COM_ORIGIN + (EXE_HEADER if code[:2] == b"MZ" else 0)
        found = replace(found, stack=bottom, stack_size=top - bottom,
                        stack_loaded=code[at:at + top - bottom])
    return found


def program(pad=0):
    """The driver's file, for a driver that keeps `pad` paragraphs of
    memory beyond its own."""
    if not PROGRAM.exists():
        raise RuntimeError(f"{PROGRAM} is not there: build the project "
                           "first")
    code = bytearray(PROGRAM.read_bytes())
    struct.pack_into("<H", code, PAD, pad)
    return bytes(code)


def _key(name):
    if name in KEYS:
        return KEYS[name]
    if re.fullmatch(r"[a-z0-9]|F\d\d?", name):
        return name.lower()
    if name in PUNCTUATION:
        return PUNCTUATION[name]
    raise ValueError(f"unknown key {name!r}")


def _russian(char):
    """The keys to hold down to type `char` of a text with Russian
    letters."""
    if char in RUSSIAN_KEYS:
        return RUSSIAN_KEYS[char]
    if char.isdigit():
        return [char]
    if char.lower() not in RUSSIAN:
        raise ValueError(f"{char!r} cannot be typed with Russian letters")
    keys = _character(RUSSIAN[char.lower()])
    return keys if char.islower() else ["shift", *keys]


def _character(char):
    """The keys to hold down to type `char`."""
    if char in SHIFTED:
        return ["shift", _key(SHIFTED[char])]
    if char.isupper():
        return ["shift", char.lower()]
    return [_key(char)]


def _codes(key, down):
    """What the keyboard sends when `key` goes down or comes up."""
    released = 0 if down else 0x80
    if key in EXTENDED:
        return [0xE0, EXTENDED[key] | released]
    return [SCAN[key] | released]


def _event(kind, codes):
    return bytes([kind, len(codes), *codes])


# The buttons of the mouse, as its packets have them.
BUTTONS = {"Move": None, "Click": 1, "RightClick": 2, "MiddleClick": 4}
# For how many times that the program asks where the mouse is a button
# stays down in one place, unless the step says.
ASKED = 20
STRIDE = 4                        # how far the mouse moves in one packet
# The rows that a step may name: those of the screen with the most lines
# that the program switches to.
POINTER_ROWS = 50


def _packet(right, up, buttons=0):
    """What the mouse sends when it has moved so far with those buttons
    down."""
    return bytes([8 | buttons | (0x10 if right < 0 else 0)
                  | (0x20 if up < 0 else 0), right & 255, up & 255])


def _mouse(packets):
    data = b"".join(packets)
    return bytes([MOUSE]) + struct.pack("<H", len(data)) + data


def _way(columns, rows, buttons):
    """The packets of a mouse that moves the pointer so many cells to
    the right and down, with those buttons down."""
    return [_packet(STRIDE if columns > 0 else -STRIDE, 0, buttons)] \
        * (2 * abs(columns)) \
        + [_packet(0, -STRIDE if rows > 0 else STRIDE, buttons)] \
        * (4 * abs(rows))


def _pointer(name):
    """The events for a name of the mouse, "Click@column,row*asked", or
    with more places, "Click@column,row*asked>column,row*asked"."""
    what, _, places = name.partition("@")
    try:
        if what not in BUTTONS:
            raise ValueError
        stops = []
        for place in places.split(">"):
            place, _, asked = place.partition("*")
            column, row = (int(number) for number in place.split(","))
            asked = int(asked or ASKED)
            if not (0 <= column < COLUMNS and 0 <= row < POINTER_ROWS
                    and 0 < asked < 256):
                raise ValueError
            stops.append((column, row, asked))
        button = BUTTONS[what]
        if button is None and (len(stops) > 1 or "*" in places):
            raise ValueError
    except ValueError:
        raise ValueError(f"unknown step of the mouse {name!r}") from None
    # To the corner, then to the middle of the cell: 8 dots a cell, and
    # two units of the mouse to a dot downwards.
    column, row, asked = stops[0]
    way = [_packet(-127, 127)] * 4 \
        + [_packet(STRIDE, 0)] * (2 * column + 1) \
        + [_packet(0, -STRIDE)] * (4 * row + 2)
    if button is None:
        return _mouse(way)
    out = _mouse(way + [_packet(0, 0, button)]) + bytes([ASKS, asked])
    for to_column, to_row, asked in stops[1:]:
        out += _mouse(_way(to_column - column, to_row - row, button)) \
            + bytes([ASKS, asked])
        column, row = to_column, to_row
    return out + _mouse([_packet(0, 0)])


def events(step):
    """The events of the script for one step, without the one that ends
    it."""
    chords = []
    for name in step.split():
        if "@" in name and not name.startswith('"'):
            chords.append(_pointer(name))
        elif name.startswith('"'):
            text = name.strip('"')
            keys_of = (_russian if any(char.lower() in RUSSIAN
                                       for char in text) else _character)
            chords += [keys_of(char) for char in text]
        else:
            chords.append([_key(part) for part in name.split("+")])
    out = b""
    for chord in chords:
        if isinstance(chord, bytes):
            out += chord
            continue
        held = [key for key in chord if key in MODIFIERS]
        keys = [key for key in chord if key not in MODIFIERS]
        for key in held:
            out += _event(KEYBOARD, _codes(key, True))
        if keys:
            out += _event(KEYBOARD,
                          [code for key in keys for code in _codes(key, True)]
                          + [code for key in keys[::-1]
                             for code in _codes(key, False)])
        for key in held[::-1]:
            out += _event(KEYBOARD, _codes(key, False))
    return out


def script(steps, known, blinking=False):
    """The script that takes a program through `steps` from where the
    bench's machines begin, waiting at the PAUSE before the program is
    started: a screen is written out when the program has started, and
    after each step. `known` is the program's Facts. With `blinking`, a
    screen that shows the program's clock is written out in both its
    states."""
    out = bytearray(MAGIC)
    out += struct.pack("<HHHH", known.auto_time,
                       known.clock if blinking else NONE,
                       known.stack, known.stack_size)
    out += known.name
    out += events("Enter")                      # ends the PAUSE
    for step in ("",) + tuple(steps):
        out += events(step)
        out.append(STEP)
    out.append(END)
    if len(out) > SCRIPT_DISK:
        raise ValueError("the script is longer than its disk")
    return bytes(out)


@dataclass
class Step:
    """What the driver wrote at the end of a step."""
    screen: Screen
    stack: bytes = None           # what the program's stack held, if the
                                  # program waited where that is looked at
    stack_segment: int = None     # and the segment it is in


def _screen(data):
    cells, rest = data[:-5], data[-5:]
    return Screen(cells=cells, cursor=(rest[0], rest[1]),
                  cursor_lines=(rest[3], rest[2]), video_mode=rest[4])


def read(data):
    """What the driver wrote, `data`: the Steps that ended, and whether
    the script did."""
    steps, states = [], []
    at, ended = 0, False

    def close():
        """Make of the states of the screen that the last step showed
        the one screen that is kept: of two, the one with the greater
        bytes, with the cells that differ in the other as blinking."""
        if len(states) > 1:
            distinct = sorted({state.cells for state in states})
            if len(distinct) != 2:
                raise MachineError(
                    f"the screen after step {len(steps) - 1} was seen in "
                    f"{len(distinct)} states within a second: more than "
                    "blinks")
            other, shown = distinct
            size = COLUMNS * ROWS * 2
            blinking = tuple((i // 2 % COLUMNS, i // 2 // COLUMNS,
                              other[i], other[i + 1])
                             for i in range(0, size, 2)
                             if shown[i:i + 2] != other[i:i + 2])
            kept = next(state for state in states if state.cells == shown)
            steps[-1].screen = replace(kept, blinking=blinking)
        states.clear()

    while at < len(data) and not ended:
        kind = data[at:at + 1]
        if kind in (b"S", b"B") and at + 1 + SCREEN <= len(data):
            screen = _screen(data[at + 1:at + 1 + SCREEN])
            if kind == b"S":
                close()
                steps.append(Step(screen))
            states.append(screen)
            at += 1 + SCREEN
        elif kind == b"K" and at + 5 <= len(data) and at + 5 + \
                int.from_bytes(data[at + 1:at + 3], "little") <= len(data):
            size, segment = struct.unpack_from("<HH", data, at + 1)
            steps[-1].stack = data[at + 5:at + 5 + size]
            steps[-1].stack_segment = segment
            at += 5 + size
        elif kind == b"R":
            at += 1
        elif kind == b"E":
            ended = True
        else:
            break                 # cut short: the machine was stopped
    close()
    return steps, ended


def stack_left(stack, known):
    """How many bytes of `stack`, as the driver wrote it out, the program
    has never used: as many as still are what they were when the program
    was loaded, from the bottom up. That is a little too many if the
    deepest use of the stack wrote what was there."""
    left = 0
    while left < len(stack) and stack[left] == known.stack_loaded[left]:
        left += 1
    return left
