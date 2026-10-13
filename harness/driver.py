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

"Wait*18" lets time pass with nothing pressed: 18 ticks of the machine's
timer, of which a second has 18.2, from when the program has taken what
the step has before it. The program goes round meanwhile and does what it
does by itself when time passes. What the step has behind the wait, or
its end, comes the first time that the program goes round after the last
of those ticks: later than one tick less than was asked for, and no later
than the ticks asked for and one round of the program. The machine's time
is counted in the instructions it executes, so that is the same
instruction every time a program is taken through the scenario. The
original and the C translation do not get to the wait at the same time, and
each counts its ticks from where it is: their waits are of one length to
within a tick, and both go on just behind a tick. A wait may have 65535
ticks, an hour; a minute of them takes four seconds to run with the
original and eight with the translation.

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

A flat program of 32 bits, as the portable C is to be, has its data above
the megabyte, and is in the machine's memory where its extender puts it.
What the driver reads of it, it reads at addresses of the machine's
memory, which are worked out here: from the file and the linker's map,
which le.py reads, and from where the extender has the first object of a
program on the bench, le.BENCH. See flat_facts.
"""
import re
import struct
from dataclasses import dataclass, replace
from pathlib import Path

import coverage
import le
from dos import COLUMNS, ROWS, MachineError, Screen

# The driver, assembled by the build.
PROGRAM = Path(__file__).resolve().parent.parent / "build" / "DRIVER.COM"
# The file it is on the floppy that DOS boots from.
FILENAME = "DRIVER.COM"
# Where its file has the number of paragraphs it keeps beyond its own.
PAD = 3

# A script (see DRIVER.ASM).
MAGIC = b"\xA5\x5A"
KEYBOARD, MOUSE, STEP, HOLD, ASKS, ABOVE, END = 1, 2, 3, 4, 5, 6, 0
NONE = 0xFFFF
# The disk a script is on: its size is that of the disk the machine had
# when its state was saved, and the longest that a script may be. The
# driver would read one of 32 MB.
SCRIPT_DISK = 1 << 20

# What the driver writes.
SCREEN = COLUMNS * ROWS * 2 + 5

NAME = 8                          # bytes of a program's name, as DOS has it
COM_ORIGIN = 0x100                # where a .COM file is loaded

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
    # the program's file that are loaded there. The original has code
    # there, which it moves elsewhere as it starts and leaves behind; the
    # translation has zeros.
    stack_loaded: bytes = b""
    # A flat program of 32 bits: the three places are addresses of the
    # machine's memory, 0 for none, and the driver reads them with the
    # BIOS's block move. The stack has no segment then.
    flat: bool = False


def facts(program, stack=False):
    """The Facts of `program`, a setups.Program: its name, from that of
    its file, and where AutoTim and Clock are in its main data, from the
    listing of its assembly. The translation has the listing of the original
    for that: its main data is laid out as the original's. With `stack`,
    where its stack is too: by the same listing, or for the translation by
    the linker's map, which the build writes next to it.

    Of a flat program of 32 bits the main data is a variable, `md`, laid
    out as the original's all the same: the Facts are addresses of the
    machine's memory then, by the file and the map (see flat_facts)."""
    code = program.contents
    name = Path(program.filename).stem.upper().encode("ascii")
    name = name[:NAME].ljust(NAME, b"\0")
    if not program.listing or not Path(program.listing).exists():
        raise RuntimeError(
            f"the driver needs a listing for {program.filename}: its own, "
            "which the build writes next to it, or for the translation "
            "that of the original, in build/shareware-layout")
    text = Path(program.listing).read_text(encoding="cp866")

    def symbol(pattern):
        return int(re.search(pattern, text, re.M)[1], 16)

    auto_time = symbol(r"^ *AutoTim [ .]*([0-9A-F]+) +DWord")
    clock = symbol(r"^ *Clock [ .]*([0-9A-F]+) +Byte")
    flat = linear(code)
    if flat:
        return flat_facts(name, flat, le.Map(program.linked),
                          ("_md", auto_time), ("_md", clock), stack)
    found = Facts(name, auto_time, clock)
    if program.linked and not coverage.made_by_mkexe(code):
        # The driver reads the two at those places of the segment that DS
        # is on when the program idles: the main data has to begin there.
        if coverage.link_map(program.linked).get("_md") != (0, 0):
            raise RuntimeError(
                f"the main data of {program.filename} is not at offset 0 "
                "of the program's data, where the driver of the tests "
                "takes it to be: see md in src/portable_c/main.c")
    if stack:
        if program.linked:
            bottom, top = coverage.stack(program.linked)
            at = bottom + coverage.origin(code)
            if not coverage.made_by_mkexe(code):
                # The driver finds the stack by its place in the segment
                # that SS is on, which the header of the file names.
                bottom -= 16 * struct.unpack_from("<H", code,
                                                  coverage.EXE_SS)[0]
                top -= 16 * struct.unpack_from("<H", code,
                                               coverage.EXE_SS)[0]
        else:
            bottom = symbol(r"^DataSav [ .]*Byte\[\d+\] +([0-9A-F]+)h")
            top = symbol(r"^VcStack [ .]*L Near +([0-9A-F]+)h")
            at = bottom - COM_ORIGIN
        found = replace(found, stack=bottom, stack_size=top - bottom,
                        stack_loaded=code[at:at + top - bottom])
        if len(found.stack_loaded) != top - bottom:
            raise RuntimeError(
                f"the stack of {program.filename} is not in its file, and "
                "what it holds when the program has been loaded is not "
                "known")
    return found


def linear(contents):
    """The flat program whose file `contents` is, as le.py reads it, or
    None for a program of 16 bits."""
    try:
        return le.Linear(contents)
    except le.Unknown:
        return None


def flat_facts(name, program, linked, auto_time, clock, stack=False,
               base=le.BENCH):
    """The Facts of a flat program, an le.Linear with its le.Map, that
    DOS has under `name`: where the driver finds in the machine's memory
    what it watches. `auto_time` and `clock` are each a symbol of the map
    and an offset from it, or None for a program that has none. With
    `stack`, where the stack is too, and what it holds when the program
    has been loaded: what the file has there, and zeros for the rest.

    `base` is where the extender puts the first object. On the bench that
    is le.BENCH, the same in every run that was looked at, and promised
    by nothing: a run with QEMU's log of the code says where the program
    was, as harness/drivercheck.py --trace asks it."""
    def address(where):
        if where is None:
            return 0
        symbol, offset = where
        if symbol not in linked.public:
            raise RuntimeError(
                f"the linker's map has no {symbol}: the driver of the "
                "tests finds by it what it reads of a flat program")
        first, at = linked.public[symbol]
        return base + program.place(first, at + offset)

    found = Facts(name, address(auto_time), address(clock), flat=True)
    if stack:
        bottom, top = coverage.flat_stack(program, linked)
        found = replace(found, stack=base + bottom, stack_size=top - bottom,
                        stack_loaded=program.image()[bottom:top])
        if found.stack_size >= 1 << 16:
            raise RuntimeError(
                f"the stack is {found.stack_size} bytes: the driver of the "
                "tests writes out one of less than 64K")
    return found


def setup_facts(program):
    """The Facts of `program`, the original's setup program (see
    setups.original_setup): its name, and where Clock is in its one
    segment, by its listing. A script that is written with these has the
    driver take the setup program for the program that it watches, and
    not the main program, which started it: that is for a scenario that
    shows the setup program's clock, whose blinking the driver then
    records (see the top of scenarios.py). The setup program has no time
    at which it does something by itself, and its stack is not looked
    at.

    The listing has a variable by its place in the segment of what has
    no value, _BSS, and says in its map of the file where that segment
    begins in the program."""
    if not program.listing or not Path(program.listing).exists():
        raise RuntimeError(
            f"the driver needs the listing of {program.filename}, which "
            "the build writes into build/shareware-layout")
    text = Path(program.listing).read_text(encoding="cp866")
    begins = re.search(r"^_BSS +[0-9A-F]+ +([0-9A-F]+) ", text, re.M)
    clock = re.search(r"^Clock [ .]*Byte +([0-9A-F]+)h +_BSS", text, re.M)
    if not begins or not clock:
        raise RuntimeError(f"{program.listing} does not say where the "
                           "setup program keeps Clock")
    name = Path(program.filename).stem.upper().encode("ascii")
    return Facts(name[:NAME].ljust(NAME, b"\0"), NONE,
                 int(begins[1], 16) + int(clock[1], 16))


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


def _wait(name):
    """The event for "Wait*ticks"."""
    ticks = name.partition("*")[2]
    if not ticks.isdigit() or not 0 < int(ticks) < 1 << 16:
        raise ValueError(f"unknown wait {name!r}")
    return bytes([HOLD]) + struct.pack("<H", int(ticks))


def events(step):
    """The events of the script for one step, without the one that ends
    it."""
    chords = []
    for name in step.split():
        if "@" in name and not name.startswith('"'):
            chords.append(_pointer(name))
        elif name.startswith("Wait*"):
            chords.append(_wait(name))
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
    if known.flat:
        # Nothing by the header but how long the stack is: the rest by
        # addresses, in an event of their own.
        out += struct.pack("<HHHH", NONE, NONE, 0, known.stack_size)
        out += known.name
        out += bytes([ABOVE]) + struct.pack(
            "<III", known.auto_time, known.clock if blinking else 0,
            known.stack if known.stack_size else 0)
    else:
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
        raise ValueError(f"the script is {len(out)} bytes, and its disk "
                         f"holds {SCRIPT_DISK}: SCRIPT_DISK in driver.py, "
                         "a power of two")
    return bytes(out)


@dataclass
class Step:
    """What the driver wrote at the end of a step."""
    screen: Screen
    stack: bytes = None           # what the program's stack held, if the
                                  # program waited where that is looked at
    stack_segment: int = None     # and the segment it is in: 0 for
                                  # the stack of a flat program


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
        elif kind == b"F":
            raise MachineError(
                "the BIOS refused the driver of the tests a read of the "
                "program's memory above the megabyte (INT 15h, function "
                "87h): what it did then is worth nothing")
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
