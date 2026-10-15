#!/usr/bin/env python3
"""Hold the modules of the web page to the program's grid and its keys.

The piece W3a of step 7 (TODO.md, "Step 7, the web page"): the page
draws the program's 80 by 25 cells as text, by src/portable_c/web/
render.js, and makes the browser's key and pointer events the escape
codes of a terminal with the kitty protocol of the keyboard, by keys.js
and mouse.js, which the program's own decoder, the terminal's, takes.
The three are plain modules of ECMAScript, and this test runs them in
Node, harness/webrender.mjs, with no browser and nothing downloaded:

  1. The screens: the grids that the program for Linux writes through its
     test channel for the steps of harness/terminal.py's KEYS and KITTY,
     which have the panels, the viewer, the menus, the mouse's pointer,
     a dialog and the grid of the 256 colours, half of which blink; and
     one made here with every byte in every column and every attribute,
     the three characters that HTML takes for its own among them. Each
     is drawn on one Screen, one behind the other, as the page draws at
     its yields; the HTML of every row is read back, span by span, and
     held to the cells: the character as the VGA's font shows the byte
     (terminal.glyph, the table that the page is to make of the
     program's CodePageGlyph), escaped where HTML wants it; the class of
     the attribute, with the blink's for bit 7; a span for each run of
     one attribute. Only the rows that differ from the screen drawn
     before are to be made, all 25 at the first; the cursor is held to
     where the grid has it and to its shape, a block or a line, as the
     terminal's code makes it (ShapeOf); and apply, with objects for the
     page's elements. The style sheet is held to the VGA's 16 colours of
     mode 3, for the 256 attributes.
  2. The keys: KeyboardEvents of the shape that browsers send, keydown
     and keyup, of US, German and Russian layouts, with what is held and
     Caps Lock and Num Lock, made escape codes by keys.js and typed into
     the program for Linux in a pseudo-terminal that has the kitty
     protocol (terminal.Terminal), each case followed by Esc twice: what
     the program reads, by VC_TRACE, is held to the BIOS's words of the
     bench, as terminal.py's DECODE_KITTY holds kitty's own codes.
  3. The mouse: pointer events on a <pre> of 640 by 400 pixels made
     reports by mouse.js and typed into the same terminal: where the
     program has the mouse and its buttons, by VC_TRACE, is held to the
     cells; and the cell of a place outside the <pre>, kept to it.

The program is the sanitized build, posix-asan/vc. The test exits with
77, which the build takes for a test that is skipped, where the program
has not been built (no clang) or there is no Node.
"""

import argparse
import html
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

import dos
import terminal

SKIPPED = 77
HERE = Path(__file__).resolve().parent
WEB = HERE.parent / "src" / "portable_c" / "web"
ESC_PAIR = [{"type": "keydown", "code": "Escape", "key": "Escape"},
            {"type": "keyup", "code": "Escape", "key": "Escape"}]


def down_up(code, key, **held):
    """A key pressed and let go, with what is held."""
    return [dict(type="keydown", code=code, key=key, **held),
            dict(type="keyup", code=code, key=key, **held)]


def modifier(code, key, flag, inner):
    """`inner` with a modifier pressed in front of it and let go
    behind it, as the browser has them: the modifier's own keydown with
    its flag set, its keyup with it cleared."""
    return ([dict(type="keydown", code=code, key=key, **{flag: True})]
            + inner + [dict(type="keyup", code=code, key=key)])


# Each case: what it is, the events, and the BIOS's words that the program
# is to read of them (the bench's, as terminal.DECODE_KITTY has them).
SHIFT = dict(shift=True)
CTRL = dict(ctrl=True)
ALT = dict(alt=True)
NUM = dict(states=["NumLock"])
KEYS = [
    ("a", down_up("KeyA", "a"), ["1E61"]),
    ("Shift-A", modifier("ShiftLeft", "Shift", "shift",
                         down_up("KeyA", "A", **SHIFT)), ["1E41"]),
    ("A with Caps Lock on", down_up("KeyA", "A", states=["CapsLock"]),
     ["1E41"]),
    ("Shift-1, !", down_up("Digit1", "!", **SHIFT), ["0221"]),
    ("a, held: a repeat", [dict(type="keydown", code="KeyA", key="a"),
                           dict(type="keydown", code="KeyA", key="a",
                                repeat=True),
                           dict(type="keyup", code="KeyA", key="a")],
     ["1E61", "1E61"]),
    ("ф on a Russian layout, which 437 lacks", down_up("KeyA", "ф"), []),
    ("é on the key of e, which 437 has", down_up("KeyE", "é"), ["1282"]),
    ("@ by AltGraph with q on a German layout, as Windows has it: Ctrl "
     "and Alt held",
     [dict(type="keydown", code="AltRight", key="AltGraph", ctrl=True,
           alt=True, states=["AltGraph"]),
      dict(type="keydown", code="KeyQ", key="@", ctrl=True, alt=True,
           states=["AltGraph"]),
      dict(type="keyup", code="KeyQ", key="@", ctrl=True, alt=True,
           states=["AltGraph"]),
      dict(type="keyup", code="AltRight", key="AltGraph")], ["1040"]),
    ("Ctrl-A", down_up("KeyA", "a", **CTRL), ["1E01"]),
    ("Ctrl-1", down_up("Digit1", "1", **CTRL), []),
    ("Ctrl-2", down_up("Digit2", "2", **CTRL), ["0300"]),
    ("Ctrl-[", down_up("BracketLeft", "[", **CTRL), ["1A1B"]),
    ("Alt with ф: Alt-A", down_up("KeyA", "ф", **ALT), ["1E00"]),
    ("Alt-[", down_up("BracketLeft", "[", **ALT), ["1A00"]),
    ("Alt-1", down_up("Digit1", "1", **ALT), ["7800"]),
    ("Alt-=", down_up("Equal", "=", **ALT), ["8300"]),
    ("Alt-Space", down_up("Space", " ", **ALT), ["3920"]),
    ("Alt alone: the menu bar", down_up("AltLeft", "Alt", **ALT), []),
    ("Up", down_up("ArrowUp", "ArrowUp"), ["4800"]),
    ("Ctrl-Up", down_up("ArrowUp", "ArrowUp", **CTRL), ["8D00"]),
    ("Alt-Up", down_up("ArrowUp", "ArrowUp", **ALT), ["9800"]),
    ("Home", down_up("Home", "Home"), ["4700"]),
    ("End", down_up("End", "End"), ["4F00"]),
    ("PgDn", down_up("PageDown", "PageDown"), ["5100"]),
    ("Ctrl-PgUp", down_up("PageUp", "PageUp", **CTRL), ["8400"]),
    ("Ins", down_up("Insert", "Insert"), ["5200"]),
    ("Del", down_up("Delete", "Delete"), ["5300"]),
    ("Alt-Del", down_up("Delete", "Delete", **ALT), []),
    ("Ctrl-Del", down_up("Delete", "Delete", **CTRL), ["9300"]),
    ("F1", down_up("F1", "F1"), ["3B00"]),
    ("Shift-F1", down_up("F1", "F1", **SHIFT), ["5400"]),
    ("Alt-F1", down_up("F1", "F1", **ALT), ["6800"]),
    ("F3", down_up("F3", "F3"), ["3D00"]),
    ("Ctrl-F4", down_up("F4", "F4", **CTRL), ["6100"]),
    ("F10", down_up("F10", "F10"), ["4400"]),
    ("F11", down_up("F11", "F11"), ["8500"]),
    ("Shift-F12", down_up("F12", "F12", **SHIFT), ["8800"]),
    ("Alt-Esc", down_up("Escape", "Escape", **ALT), ["0100"]),
    ("Tab", down_up("Tab", "Tab"), ["0F09"]),
    ("Shift-Tab", down_up("Tab", "Tab", **SHIFT), ["0F00"]),
    ("Ctrl-Tab", down_up("Tab", "Tab", **CTRL), ["9400"]),
    ("Backspace", down_up("Backspace", "Backspace"), ["0E08"]),
    ("Alt-Backspace", down_up("Backspace", "Backspace", **ALT), ["0E00"]),
    ("Ctrl-Backspace", down_up("Backspace", "Backspace", **CTRL), ["0E7F"]),
    ("Ctrl-Enter", down_up("Enter", "Enter", **CTRL), ["1C0A"]),
    ("the keypad's 0, Num Lock on", down_up("Numpad0", "0", **NUM),
     ["5230"]),
    ("its 7, Num Lock off: Home", down_up("Numpad7", "Home"), ["4700"]),
    ("Shift with it", down_up("Numpad7", "Home", **SHIFT), ["4737"]),
    ("its 5, Num Lock off", down_up("Numpad5", "Clear"), ["4C00"]),
    ("Ctrl with it", down_up("Numpad5", "Clear", **CTRL), ["8F00"]),
    ("its /", down_up("NumpadDivide", "/"), ["352F"]),
    ("Ctrl with its *", down_up("NumpadMultiply", "*", **CTRL), ["9600"]),
    ("Alt with its -", down_up("NumpadSubtract", "-", **ALT), ["4A00"]),
    ("Alt with its Enter", down_up("NumpadEnter", "Enter", **ALT),
     ["A600"]),
    ("its +: the group to select", down_up("NumpadAdd", "+"), ["4E2B"]),
    ("F13", down_up("F13", "F13"), []),
    ("Print Screen", down_up("PrintScreen", "PrintScreen"), []),
    ("Shift alone", down_up("ShiftLeft", "Shift", **SHIFT), []),
]

# The keys that the page leaves to the browser: none of them is a key
# that the program takes.
UNMAPPED = [
    ("a key of no code, an input method's", dict(type="keydown",
                                                 code="", key="Process")),
    ("Unidentified", dict(type="keydown", code="", key="Unidentified")),
]

# The <pre>: 80 by 25 cells of 8 by 16 pixels, at 8, 4 in the client.
RECT = {"left": 8, "top": 4, "width": 640, "height": 400}


def at(column, row, buttons, kind="pointermove"):
    """A pointer event in the middle of a cell."""
    return {"type": kind, "clientX": RECT["left"] + column * 8 + 4,
            "clientY": RECT["top"] + row * 16 + 8, "buttons": buttons}


# Each case: what it is, the pointer's events, and where the program has
# the mouse then, by VC_TRACE, the buttons and the place in dots. The
# cases follow one another, on one Mouse.
MOUSE = [
    ("moved", [at(60, 10, 0)], ["mouse 0 1E0 50"]),
    ("a click on the third row of the left panel",
     [at(3, 4, 0), at(3, 4, 1, "pointerdown"), at(3, 4, 0, "pointerup")],
     ["mouse 0 18 20", "mouse 1 18 20", "mouse 0 18 20"]),
    ("the right button, which selects", [at(3, 3, 2, "pointerdown"),
                                         at(3, 3, 0, "pointerup")],
     ["mouse 2 18 18", "mouse 0 18 18"]),
    ("moved off the <pre>, to the bottom right: kept to the last cell",
     [{"type": "pointermove", "clientX": 2000, "clientY": 2000,
       "buttons": 0}], ["mouse 0 278 C0"]),
]
# Places of the client and the cell that each is to be, kept to the
# screen.
CELLS = [((8, 4), (0, 0)), ((647.9, 403.9), (79, 24)),
         ((0, 0), (0, 0)), ((5000, -50), (79, 0)), ((8 + 8 * 41, 4 + 160),
                                                    (41, 10))]


def own_screen():
    """A grid with every byte in every column and every attribute: the
    cell of row r and column c has the byte 80r + c + r and the attribute
    7r + 3c, each cut to its byte; the cursor a line, at 24, 79."""
    cells = []
    for row in range(25):
        for column in range(80):
            cells.append((80 * row + column + row) % 256
                         | ((7 * row + 3 * column) % 256) << 8)
    return cells, 24 << 8 | 79, 0x0E0F


def screens(program):
    """The screens: those of the test channel for KEYS and KITTY, and the
    one of own_screen; each as (cells, place, shape)."""
    out = []
    with dos.workdir() as top:
        work = Path(top)
        for name, steps in (("channel", terminal.KEYS),
                            ("channel-kitty", terminal.KITTY)):
            (work / name).mkdir()
            home, here = terminal.lay_out(work / name)
            records, _ = terminal.channel(program, home, here, work, steps,
                                          name)
            for record in records:
                cells = [record[8 + 2 * i] | record[9 + 2 * i] << 8
                         for i in range(2000)]
                rest = record[-8:]
                out.append((cells, rest[1] << 8 | rest[0],
                            rest[2] << 8 | rest[3]))
    out.append(own_screen())
    # The same cells again, so that no row is to be made, with the cursor
    # at the edges of its shapes: a line of four lines, a block of five,
    # hidden by bit 5, none by its first line below its last, a block of
    # all 32, and off the screen.
    cells = out[-1][0]
    for place, shape in ((0, 0x0C0F), (0x0100, 0x0B0F), (0x1801, 0x2607),
                         (0x0A28, 0x0F0E), (0x184F, 0x001F),
                         (0x1900, 0x0607), (0x0050, 0x0607)):
        out.append((cells, place, shape))
    return out


SPAN = re.compile(r'<span class="([^"]*)">([^<]*)</span>')


def read_row(text, failures, where):
    """The cells of a row's HTML: (character, attribute) for each."""
    cells = []
    previous = None
    end = 0
    for match in SPAN.finditer(text):
        if match.start() != end:
            failures.append(f"{where}: {text[end:match.start()]!r} is no "
                            "span")
        end = match.end()
        classes = match.group(1).split()
        name = re.fullmatch(r"vc-a([0-9a-f]{2})", classes[0])
        if name is None:
            failures.append(f"{where}: a span of the class "
                            f"{match.group(1)!r}")
            continue
        attribute = int(name.group(1), 16)
        if classes[1:] != (["vc-b"] if attribute & 0x80 else []):
            failures.append(f"{where}: the attribute {attribute:02X} has "
                            f"the classes {match.group(1)!r}")
        if attribute == previous:
            failures.append(f"{where}: two spans of {attribute:02X} one "
                            "behind the other, where a run is one")
        previous = attribute
        body = match.group(2)
        if re.search(r"&(?!amp;|lt;|gt;)|>", body):
            failures.append(f"{where}: {body!r} is not escaped as HTML is")
        cells += [(character, attribute)
                  for character in html.unescape(body)]
    if end != len(text):
        failures.append(f"{where}: {text[end:]!r} is no span")
    return cells


def shape_of(place, shape):
    """The cursor as the terminal's code makes it, ShapeOf: None, or
    (row, column, block)."""
    first, last = shape >> 8 & 0x1F, shape & 0x1F
    row, column = place >> 8, place & 0xFF
    if shape & 0x2000 or first > last or row >= 25 or column >= 80:
        return None
    return {"row": row, "column": column, "block": last - first > 3}


def held_screens(given, answered, failures):
    shown = None
    for number, ((cells, place, shape), drawn, applied) in enumerate(
            zip(given, answered["draws"], answered["applied"])):
        where = f"screen {number + 1}"
        want = [row for row in range(25)
                if shown is None
                or cells[row * 80:row * 80 + 80] != shown[row * 80:
                                                         row * 80 + 80]]
        have = [row for row, _ in drawn["rows"]]
        if have != want:
            failures.append(f"{where}: the rows {have} are made, where "
                            f"those that changed are {want}")
        shown = cells
        for row, text in drawn["rows"]:
            got = read_row(text, failures, f"{where}, row {row}")
            expect = [(terminal.glyph(cell & 0xFF), cell >> 8)
                      for cell in cells[row * 80:row * 80 + 80]]
            if got != expect:
                wrong = [column for column in range(80)
                         if column >= len(got)
                         or got[column] != expect[column]]
                failures.append(f"{where}, row {row}: the columns {wrong} "
                                "are not the grid's")
        for row in range(25):
            if read_row(applied["rows"][row], [], "") != [
                    (terminal.glyph(cell & 0xFF), cell >> 8)
                    for cell in cells[row * 80:row * 80 + 80]]:
                failures.append(f"{where}: apply leaves row {row} "
                                "otherwise than the grid")
        cursor = shape_of(place, shape)
        if drawn["cursor"] != cursor:
            failures.append(f"{where}: the cursor is {drawn['cursor']}, "
                            f"where it is {cursor}")
        style = applied["cursor"]
        if cursor is None:
            want = {"display": "none"}
        else:
            height = 16 if cursor["block"] else 2
            want = {"display": "block",
                    "left": f"{cursor['column'] * 8}px",
                    "top": f"{(cursor['row'] + 1) * 16 - height}px",
                    "width": "8px", "height": f"{height}px"}
        if {key: style.get(key) for key in want} != want:
            failures.append(f"{where}: apply places the cursor {style}, "
                            f"where {want}")


def held_css(text, blink, failures):
    rules = dict(re.findall(r"\.(vc-a[0-9a-f]{2})\{([^}]*)\}", text))
    for attribute in range(256):
        name = f"vc-a{attribute:02x}"
        want = (f"color:#{terminal.VGA[attribute & 15]};background:"
                f"#{terminal.VGA[attribute >> 4 & 7]}")
        if rules.get(name) != want:
            failures.append(f"the style of {attribute:02X} is "
                            f"{rules.get(name)!r}, where {want!r}")
    if len(rules) != 256:
        failures.append(f"the style sheet has {len(rules)} attributes")
    if f".{blink[2]} .vc-b{{color:transparent}}" not in text:
        failures.append("the style sheet does not hide a blinking "
                        f"character under .{blink[2]}")
    # The VGA's character blinks at a 32nd of its 70.086 frames a second,
    # half of it shown; its cursor twice as fast.
    if abs(blink[0] - 228.3) > 0.1 or abs(blink[1] - 114.1) > 0.1:
        failures.append(f"the blink's halves are {blink[:2]} ms")


def typed(answered, failures):
    """The bytes of each case, as one string of bytes per case."""
    out = []
    for (what, events, _), results in zip(KEYS, answered["keys"]):
        data = b""
        for event, result in zip(events, results):
            if result["bytes"] is None or not result["prevent"]:
                failures.append(f"{what}: {event} is left to the browser")
                continue
            data += result["bytes"].encode("latin-1")
        out.append(data)
    return out


def held_keys(pyte, program, work, data, failures):
    home, here = terminal.lay_out(work / "keys")
    term = terminal.Terminal(pyte, program, home, here, work, 80, 25,
                             "web-keys", kitty=True)
    term.read()
    esc = terminal.CSI + b"27u" + terminal.CSI + b"27;1:3u"
    for case in data:
        term.type(case, 0.15)
        term.type(esc, 0.15)
        term.type(esc, 0.15)
    have = [line for line in term.traced() if line.startswith("key ")]
    want = []
    for _, _, words in KEYS:
        want += [f"key {word}" for word in words] + ["key 011B"] * 2
    if have != want:
        at = next((i for i, (a, b) in enumerate(zip(have, want)) if a != b),
                  min(len(have), len(want)))
        failures.append(f"the keys: the program reads {have[at:at + 6]} "
                        f"from the {at + 1}th word on, where it was to "
                        f"read {want[at:at + 6]}")
        for (what, _, words), case in zip(KEYS, data):
            print(f"  {what}: {case!r} for {words}")
    terminal.ended("the keys", term, failures)


def held_mouse(pyte, program, work, answered, failures):
    home, here = terminal.lay_out(work / "mouse")
    term = terminal.Terminal(pyte, program, home, here, work, 80, 25,
                             "web-mouse", kitty=True)
    term.read()
    want = []
    for (what, _, lines), reports in zip(MOUSE, answered["mouse"]):
        for report in reports:
            term.type(report.encode(), 0.15)
        want += lines
    have = [line for line in term.traced() if line.startswith("mouse ")]
    # The first report is of the mouse before anything has moved it.
    if have[1:] != want:
        failures.append(f"the mouse: the program has {have[1:]}, where it "
                        f"was to have {want}; the reports were "
                        f"{answered['mouse']}")
    terminal.ended("the mouse", term, failures)
    for (place, cell), got in zip(CELLS, answered["cells"]):
        if (got["column"], got["row"]) != cell:
            failures.append(f"the cell of {place} is {got}, where {cell}")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("program", type=Path,
                        help="the program for Linux, posix-asan/vc")
    args = parser.parse_args()
    node = shutil.which("node")
    if not args.program.exists() or node is None:
        print(f"{args.program} is not there, or Node is not: the test is "
              "skipped")
        sys.exit(SKIPPED)
    program = args.program.resolve()
    failures = []
    given = screens(program)
    glyphs = [ord(terminal.glyph(byte)) for byte in range(256)]
    with dos.workdir() as top:
        work = Path(top)
        (work / "in.json").write_text(json.dumps({
            "glyphs": glyphs,
            "screens": [{"cells": c, "place": p, "shape": s}
                        for c, p, s in given],
            "keys": [events for _, events, _ in KEYS]
                    + [[event] for _, event in UNMAPPED],
            "mouse": [{"rect": RECT,
                       "events": [e for _, events, _ in MOUSE
                                  for e in events]}],
            "cells": [[x, y, RECT] for (x, y), _ in CELLS]}))
        subprocess.run([node, str(HERE / "webrender.mjs"), str(WEB),
                        str(work / "in.json"), str(work / "out.json")],
                       check=True)
        answered = json.loads((work / "out.json").read_text())
        for (what, _), results in zip(UNMAPPED,
                                      answered["keys"][len(KEYS):]):
            if results[0]["bytes"] is not None or results[0]["prevent"]:
                failures.append(f"{what}: the page takes it, {results[0]}")
        # The mouse's reports, as one list per case.
        reports = iter(answered["mouse"][0])
        answered["mouse"] = [[next(reports) for _ in events]
                             for _, events, _ in MOUSE]
        held_screens(given, answered, failures)
        held_css(answered["css"], answered["blink"], failures)
        pyte = terminal.load_pyte()
        held_keys(pyte, program, work, typed(answered, failures), failures)
        held_mouse(pyte, program, work, answered, failures)
    for failure in failures:
        print(failure)
    if failures:
        sys.exit(1)
    print(f"{len(given)} screens drawn and read back, the style of 256 "
          f"attributes; {len(KEYS)} keys and {len(MOUSE)} cases of the "
          "mouse read by the program as the bench's BIOS and driver answer")


if __name__ == "__main__":
    main()
