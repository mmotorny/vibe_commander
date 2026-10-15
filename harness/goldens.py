#!/usr/bin/env python3
"""Goldens: what the original Volkov Commander does in each scenario.

The original is the measure of every build, but it need not be run each
time one is tested. It is taken through a scenario once, and what it shows
and leaves behind is written to harness/goldens/: after the start and after
every step the text screen, cell by cell, with the cursor and the video
mode, and at the end the files on the disks. Next to that goes a list of
the original's code that ran, which ported.py holds against the routines
that have been translated.

The original is two programs, and a scenario is of one of them (see "The
scenarios of the setup program" at the top of scenarios.py). The golden of
a scenario of the setup program is what the original's setup program
showed, started from the main program, and the list next to it is of the
code of VCSETUP.COM that ran, with nothing of VC.COM's: ran() keeps the
two kinds apart, and nothing else reads the lists of all scenarios.

A golden is derived, never edited. Where a build is meant to differ from
the original, deviations.py says how, and the golden is derived with that
difference in it: it holds what a build is expected to show. What is not
compared at all is blanked on both screens when a build is held against a
golden, so a golden need not be derived again when more is left out.

    goldens.py derive [SCENARIO...]     derive goldens from the original;
                                        all of them if none is named

Scenarios are derived side by side, each on a machine of its own, as a
build is taken through them (see compare.py): the machine's time is
counted in what it executes, and a golden is the same however busy the
host is. All of them take a minute and a half on four processors. After
a change to the harness, `git status` of harness/goldens/ is clean when
they have been derived again, if the change has left the judging alone.

The files are text. A screen is 25 lines, each the row between bars and
then its colours, as runs of an attribute byte: 1Fx80. Bytes that are not
printable ASCII are written as the characters they show as in the code
page of the scenario's country.


What a tree is expected to show otherwise: the patch

A C tree that is declared to behave otherwise than the original, the
portable C (src/portable_c/DEVIATIONS.md), is held against the original's
goldens all the same. What it is expected to show in a scenario is the
original's golden with the declared difference made to it, each time the
golden is read for the tree (read), and written nowhere:

  1. the rules of the tree, for what is otherwise on many screens
     (`rules` of deviations.TREES, and deviations.Part for the kind of
     rule that the program's name is said by);
  2. the scenario's patch, if it has one;
  3. the screens of the steps that the tree is not taken through, left
     out (`left_out` and `left_out_of` of deviations.TREES, and what
     `in_place_in` there has None for).

A patch is a file of harness/goldens/TREE/, NAME.patch for the scenario
NAME.

A patch says which rows of the original's golden the tree is expected to
show otherwise, and what it is expected to show there. It is written by
whoever declared the deviation, as their statement of what is right, and
the tests hold the tree to it; it is no record of what the tree did. The
whole of one:

    The portable C in the scenario tree-choose: what it is expected to
    show otherwise than the original. A patch of
    harness/goldens/tree-choose.txt, see harness/goldens.py: a line with -
    is the golden's, a line with + is what is expected in its place.
    Deviation: 7 of src/portable_c/DEVIATIONS.md
    Why: the key bar with Alt held has no name for F5, where the original
    has Memory.

    == 13: Alt+F10
    row 23
    -|1Left   2Right  3View.. 4Edit.. 5Memory 6DirSiz ...| 07x1 30x6 ...
    +|1Left   2Right  3View.. 4Edit.. 5       6DirSiz ...| 07x1 30x6 ...

  - The header is everything before the first line that begins with
    "== ". It has a line "Deviation: N of src/TREE/DEVIATIONS.md", with
    the number of the entry that the difference is declared in, or
    several, "1, 7"; and "Why: ", a sentence on what is otherwise here,
    which may go on in the lines below it. The entry has to be there and
    to name the scenario: ported.py holds that.
  - "== 13: Alt+F10" is a screen, by the line that it has in the
    original's golden, word for word: the number of the step as the
    original is taken through it and the step. Where steps are left out
    for the tree, that is not the number that the tree's run has; a test
    that fails says both.
  - "row 23", or "rows 3-12" for rows that follow one another: then the
    rows as the golden has them, each behind a -, and as many rows as
    the tree is expected to show, each behind a +, in the golden's own
    form, characters and colours. So what the original shows and what is
    expected stand one above the other as two pictures.
  - "-cursor 0 24 lines 6 7 mode 3" and below it "+cursor 8 23 lines 6 7
    mode 3": the line that a screen has for its cursor and its mode, as
    the golden has it and as it is expected.
  - "-blinks 75 0 to |:| 30": a cell that the original was seen to
    blink and the tree is not expected to; with + for one that only the
    tree is.
  - "== disks", for what is left on the disks: "-C:\\WORK\\X.TXT  A ..."
    is a line that the golden has there and "+C:\\WORK\\X.TXT  A ..."
    one that is expected: both for a file that is left otherwise, one of
    them for a file that only one leaves.

A patch is exact. It is made to the golden only if every line with - is
what the golden has in that place, character for character (as the rules
of the tree leave it, which is as the file has it wherever no rule has a
word), and the screen has that step. So on a row that a rule changes, the
line with - is not the original's row but what the rule made of it: the
tree's row, where the patch is of other cells, as the banner of the
portable C beside a box that is narrower there; or, where the patch is
there because the rule is wrong of a cell, the tree's row with the
original's character in that cell, which neither program shows, and the
line with + puts right. Where it is not, nothing is run and
the test fails with the patch, the screen and the row, and both lines:
the original's golden was derived again and came out otherwise, or the
scenario has other steps now. That is a failure to look at. The patch
said what the tree shows otherwise than the original, and the original is
not what it was taken to be: a patch is not made again to make the
failure go away.

Why not a unified diff of the two files, which every tool reads: it
knows a place by the number of a line and the lines around it, so the
screens of the steps that are left out for a tree would be in it, by the
hundred lines, where deviations.TREES says them once; a row that comes
out otherwise elsewhere in the golden, or a step added behind, would
move or break a hunk that says nothing of them; and `patch` fits a hunk
where it nearly fits, which is what is not wanted. This form knows a row
by its screen and its number, needs no lines around it, and still shows
the two rows as a diff does.

    goldens.py patch --tree portable_c SCENARIO...
                                        write the patch of each scenario
                                        from a run of the tree's build,
                                        for its author to read and commit
    goldens.py changes --tree portable_c [SCENARIO...]
                                        print what each patch of the tree
                                        says: of each screen the rows
                                        that are expected otherwise, with
                                        the cells marked, and the steps
                                        that are left out; of every
                                        scenario that has either if none
                                        is named

`patch` is how the rows of a patch are got, not what makes them right. It
takes the build of the tree through the scenario, holds what it shows
against the original's golden as the rules leave it, and writes every
row that differs, with a header to be filled in, or with the header that
the patch had. Its author then reads it row by row against what
DEVIATIONS.md declares: a row that the declaration does not explain is a
change that was not meant, and is mended in the program, not committed.
What is committed is the author's statement, and from then on the tests
hold the program to it, not to another run. A patch with a header that
is not filled in fails them.


What a tree is expected to show where the original cannot go: its own

Two scenarios are held against no golden of the original: `editor-huge`,
where the original cannot go, a text larger than its editor has room
for, and `menu-none`, from which the original does not come back, the
menu bar opening the menu that is none. The user allowed on 14 October
2026 that the portable C be held there to an expectation of its own
(CLAUDE.md, "The portable C").
deviations.OWN names it, `own` of deviations.TREES says which tree it is
of and why, and an entry of the tree's DEVIATIONS.md names it as a patch
is named. Neither the original nor another tree is taken through it
(scenarios.not_run), so it has no golden of the original, no list of what
ran of the original, and no patch.

Its expectation is a file of the tree's directory, harness/goldens/TREE/
NAME.txt, written as a golden is, with a header of its own:

    The portable C in the scenario editor-huge: its own expectation, ...
    Deviation: 13 of src/portable_c/DEVIATIONS.md
    Why: ...
    Read: every row of every screen and the disks, against ...
    code page 437

and then the screens and the disks as in a golden of the original.

    goldens.py record --tree portable_c SCENARIO
                                        take the tree's build through the
                                        scenario and write what it showed
                                        and left as the expectation

`record` writes what the build did, as `patch` does, and that is not yet
the expectation. It keeps "Deviation" and "Why" of the file that was
there, and always writes "Read: TO BE WRITTEN", and the tests fail for
that until whoever recorded it has read it row by row, every screen
against the step that it follows and against the entry, and the disks,
and has written in that line what was read and against what. Recorded
again, it prints what is otherwise than in the file that was there, which
is then the reading list, as `git diff` of the file is. It is recorded
again only for a change that is meant to change what the scenario shows,
never to make the test pass.

The test says what it holds the build to: "as the portable C's own
expectation", and in a row that differs "its own" where it says
"original" for every other scenario. goldens.read refuses an expectation
of a tree's own for any scenario but those of deviations.OWN, and the
scenarios' test (ported.py) fails for a tree that declares another, or
that has a file of its directory that is neither a patch nor the
expectation of a scenario of its own.
"""
import argparse
import itertools
import os
import re
import sys
import textwrap
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field, replace
from pathlib import Path

import deps
import deviations
import scenarios
import setups
from dos import COLUMNS, CP437_CONTROLS, ROWS, Screen

DIRECTORY = Path(__file__).resolve().parent / "goldens"

# How many scenarios are derived side by side: each has a machine of its
# own, which keeps one processor busy while the program works.
TOGETHER = min(4, os.cpu_count() or 1)

# The trees that may have patches, and what each is called in the first
# line of one.
TREES = {"portable_c": "The portable C"}

# What a header has until its author has filled it in.
TO_WRITE = "TO BE WRITTEN"

ATTRIBUTES = "RHSVDA"             # read-only, hidden, system, label, ...

# What disk_lines() says of the time of a directory of the program for
# Linux that the run has written into, which is not compared.
WRITTEN_INTO = "written into by the run"

# A file stamped with a time in the minutes after the machine's clock was
# started has been written during the run. Here as DOS counts minutes in a
# time stamp: 10 of them.
RUN_TIME = 10 << 5


def charset(codepage):
    """The 256 characters that stand for the bytes of a screen cell."""
    chars = ["␀"] + list(CP437_CONTROLS[1:])
    chars += [chr(code) for code in range(32, 127)] + ["⌂"]
    chars += list(bytes(range(128, 255)).decode(f"cp{codepage}")) + ["⍽"]
    assert len(set(chars)) == 256
    return chars


@dataclass
class Golden:
    scenario: str
    codepage: int
    steps: list                   # what was done before each screen
    screens: list
    disks: dict                   # drive -> path -> line describing it
    # Of a golden as it was read for a tree (see read): the tree; of each
    # screen the number that it has in the original's golden and the step
    # that the original was taken through, which are other where steps
    # are left out for the tree or others taken in their place; and the
    # patch that was made to it, or None.
    tree: str = None
    places: list = None
    original_steps: list = None
    patch: "Patch" = None
    # Of the expectation of a tree's own (see the top): its file. Such a
    # golden has no places, no original steps and no patch.
    own: Path = None


@dataclass
class PatchedScreen:
    """What a patch says of one screen."""
    rows: dict = field(default_factory=dict)      # row -> (-, +)
    cursor: tuple = None                          # (-, +)
    blinks: list = field(default_factory=list)    # (sign, line)


@dataclass
class Patch:
    """A patch as its file has it: see the top of this file."""
    file: Path
    deviations: tuple             # the numbers of its entries
    why: str
    screens: dict                 # "13: Alt+F10" -> PatchedScreen
    disks: list                   # (sign, line)
    # Where it has been made to a golden: of each screen that it speaks
    # of, by its number in the original's golden, the PatchedScreen; and
    # the files of the disks that it speaks of, (drive, path).
    made: dict = field(default_factory=dict)
    files: set = field(default_factory=set)

    def count(self):
        """How many lines it says are expected: rows, cursors, cells
        that blink and files."""
        return (sum(len(screen.rows) + bool(screen.cursor)
                    + sum(sign == "+" for sign, _ in screen.blinks)
                    for screen in self.screens.values())
                + sum(sign == "+" for sign, _ in self.disks))


class Misfit(Exception):
    """A patch cannot be made to its golden, or is not written as a
    patch is: the message says which, where, and what to do."""


def path(name):
    """Where the golden of the scenario `name` is: the original's."""
    return DIRECTORY / f"{name}.txt"


def patch_path(name, tree):
    """Where the patch of the tree `tree` for the scenario `name` is, if
    it has one."""
    return DIRECTORY / tree / f"{name}.patch"


def own_path(name, tree):
    """Where the expectation of the tree `tree`'s own for the scenario
    `name` is, if the tree is held to one there (see the top)."""
    return DIRECTORY / tree / f"{name}.txt"


def patches(tree):
    """The files of patches that the tree `tree` has: those of its
    directory. Whatever else lies there is among them."""
    directory = DIRECTORY / tree
    return sorted(directory.glob("*")) if directory.is_dir() else []


def shown(file):
    """The path of `file` as a message has it: from the root."""
    return file.relative_to(deps.ROOT).as_posix()


def ran_path(name):
    return DIRECTORY / f"{name}.ran"


def disk_lines(disks, programs):
    """Describe the disks of a run: drive -> path -> text.

    `programs` are file names of programs under test, which stand for each
    other. A file written during the run is only said to be: runs do not
    keep the same time to the second.
    """
    described = {}
    for drive, tree in disks.items():
        lines = described[drive] = {}
        for entry in tree.values():
            directory, _, name = entry.path.rpartition("\\")
            if name in programs:
                lines[f"{directory}\\<program>"] = "the program under test"
                continue
            flags = "".join(letter for bit, letter in enumerate(ATTRIBUTES)
                            if entry.attributes >> bit & 1) or "-"
            date, time = setups.DOS_CLOCK
            if entry.is_directory and entry.date == entry.time == 0xFFFF:
                # Of the program for Linux, a directory whose time the
                # host has moved, since the run wrote into it: DOS keeps
                # a directory's time (posixrun.WRITTEN_INTO).
                when = WRITTEN_INTO
            elif entry.date == date and time < entry.time < time + RUN_TIME:
                when = "during the run"
            else:
                when = (f"{1980 + (entry.date >> 9)}-"
                        f"{entry.date >> 5 & 15:02}-{entry.date & 31:02} "
                        f"{entry.time >> 11:02}:{entry.time >> 5 & 63:02}:"
                        f"{(entry.time & 31) * 2:02}")
            what = "" if entry.is_directory else \
                f"  {entry.size} bytes  {entry.sha256[:16]}"
            lines[entry.path] = f"{flags:<3} {when}{what}"
    return described


# The lines of a golden, which a patch has too: of the cursor, of a row,
# of a cell that blinks and of a file on a disk, each as text and back.

def cursor_line(screen):
    return (f"cursor {screen.cursor[0]} {screen.cursor[1]} lines "
            f"{screen.cursor_lines[0]} {screen.cursor_lines[1]} mode "
            f"{screen.video_mode}")


def row_line(screen, row, chars):
    cells = screen.cells[row * COLUMNS * 2:(row + 1) * COLUMNS * 2]
    text = "".join(chars[code] for code in cells[0::2])
    runs = []
    for attribute in cells[1::2]:
        if runs and runs[-1][0] == attribute:
            runs[-1][1] += 1
        else:
            runs.append([attribute, 1])
    return f"|{text}| " + " ".join(f"{a:02X}x{n}" for a, n in runs)


def blink_line(blink, chars):
    column, row, code, attribute = blink
    return f"blinks {column} {row} to |{chars[code]}| {attribute:02X}"


def disk_line(drive, name, text):
    return f"{drive}:\\{name}  {text}"


def cursor_of(line):
    """The cursor, its lines and the mode of the line `line`."""
    _, x, y, _, first, last, _, mode = line.split()
    return (int(x), int(y)), (int(first), int(last)), int(mode)


def cells_of(line, codes):
    """The cells of the row that `line` is."""
    text, runs = line[1:1 + COLUMNS], line[COLUMNS + 3:].split()
    attributes = b"".join(bytes([int(run[:2], 16)]) * int(run[3:])
                          for run in runs)
    if len(attributes) != len(text):
        raise ValueError(f"{len(attributes)} colours for {len(text)} "
                         "characters")
    cells = bytearray(2 * len(text))
    cells[0::2] = bytes(map(codes.__getitem__, text))
    cells[1::2] = attributes
    return cells


def blink_of(line, codes):
    """The cell that blinks of `line`, or None if it is no such line."""
    match = re.fullmatch(r"blinks (\d+) (\d+) to \|(.)\| (..)", line)
    return match and (int(match[1]), int(match[2]), codes[match[3]],
                      int(match[4], 16))


def write(golden, header=None):
    """Write the golden `golden`: the original's, or with `header`, the
    lines in front of its code page, the expectation of a tree's own, to
    the file that its `own` is."""
    chars = charset(golden.codepage)
    out = (header or
           [f"Volkov Commander 4.05 in the scenario {golden.scenario}: "
            "what the original shows and",
            "leaves on its disks. Derived by harness/goldens.py; not to "
            "be edited."]) + [f"code page {golden.codepage}"]
    steps = ("the start",) + scenarios.SCENARIOS[golden.scenario].steps
    for number, (step, screen) in enumerate(zip(steps, golden.screens)):
        out += ["", f"== {number}: {step}", cursor_line(screen)]
        out += [row_line(screen, row, chars) for row in range(ROWS)]
        out += [blink_line(blink, chars) for blink in screen.blinking]
    out += ["", "== disks"]
    for drive in sorted(golden.disks):
        for name in sorted(golden.disks[drive]):
            out.append(disk_line(drive, name, golden.disks[drive][name]))
    (golden.own or path(golden.scenario)).write_text(
        "\n".join(out) + "\n", encoding="utf-8")


def read(name, tree=None, patched=True):
    """The golden of the scenario `name`: what the original showed and
    left. With `tree`, what a program of that tree is expected to show
    and leave, which is the same with what is declared for the tree made
    to it, in this order: its rules, to every screen; the patch that it
    has for the scenario, if it has one, or no patch if `patched` is
    false, or the Patch that `patched` is; and the screens of the steps
    that it is not taken through left out. A patch that cannot be made
    to the golden is a Misfit. Of a scenario that a tree is held to an
    expectation of its own in, that expectation (read_own)."""
    if deviations.owner(name) is not None or \
            tree is not None and own_path(name, tree).exists():
        return read_own(name, tree)
    golden = read_original(name)
    if tree is None:
        return golden
    declared = deviations.declared(tree)
    golden.tree = tree
    if declared.rules:
        golden.screens = [deviations.expected(screen, declared.rules)
                          for screen in golden.screens]
    kept = [0] + [at + 1 for at in declared.kept(golden.steps[1:], name)]
    file = patch_path(name, tree)
    if not file.exists() and tree in deviations.BASE:
        # A tree of another's source has that one's patch where it has
        # none of its own.
        file = patch_path(name, deviations.BASE[tree])
    if isinstance(patched, Patch):
        golden.patch = patched
    elif patched and file.exists():
        golden.patch = read_patch(file, file.parent.name)
    if golden.patch:
        fit(golden.patch, golden, kept)
    golden.places = kept
    golden.original_steps = [golden.steps[at] for at in kept]
    golden.steps = ["the start"] + list(declared.steps(golden.steps[1:],
                                                       name))
    golden.screens = [golden.screens[at] for at in kept]
    return golden


def ruled(name, tree):
    """What the rules of `tree` make of the scenario `name`: of each
    rule, in their order, how many screens of the original's golden it
    expects otherwise, and how many screens any of them does. Only the
    screens of the steps that the tree is taken through count."""
    declared = deviations.declared(tree)
    golden = read_original(name)
    screens = [golden.screens[0]] + [
        golden.screens[at + 1]
        for at in declared.kept(golden.steps[1:], name)]
    # Most screens have nothing of what any rule is of, which is told
    # by making all the rules at once.
    found = [[deviations.ruled(screen, rule) for rule in declared.rules]
             for screen in screens
             if deviations.expected(screen, declared.rules).cells
             != screen.cells]
    return ([sum(column) for column in zip(*found)] or
            [0] * len(declared.rules)), sum(map(any, found))


def read_original(name):
    """The golden of the scenario `name` as its file has it."""
    file = path(name)
    if not file.exists():
        raise FileNotFoundError(
            f"{file} is not there: derive it, goldens.py derive {name}")
    return parsed(name, file.read_text(encoding="utf-8").split("\n"), 2)


# What the line of a tree's own expectation says when it has been read,
# which `record` leaves to be written: see the top.
READ = "Read: "


def own_header(name, tree, numbers=None, why=None):
    """The lines in front of the code page of the expectation of the
    tree `tree`'s own for the scenario `name`, with what `record` leaves
    to be written: the entry's number and why if they are not given, and
    always what was read."""
    return (textwrap.wrap(
        f"{TREES[tree]} in the scenario {name}: its own expectation, what "
        "it is expected to show and to leave on its disks where the "
        "original cannot go. Recorded from a run of its build by "
        "harness/goldens.py record, and read row by row by whoever "
        "recorded it: see the top of harness/goldens.py. There is no "
        "golden of the original for this scenario.", width=78,
        break_on_hyphens=False) + [
            "",
            f"Deviation: {', '.join(map(str, numbers)) if numbers else 'N'}"
            f" of src/{tree}/DEVIATIONS.md"]
            + textwrap.wrap(f"Why: {why or TO_WRITE}", width=78)
            + ["", f"{READ}{TO_WRITE}", ""])


def read_own(name, tree):
    """The expectation of the tree `tree`'s own for the scenario `name`
    (see the top), with its header filled in: a Misfit for a scenario
    that is not of deviations.OWN or not of that tree, and for a header
    that is not filled in."""
    owner = deviations.owner(name)
    file = own_path(name, tree) if tree else path(name)
    if name not in deviations.OWN:
        raise Misfit(
            f"{shown(file)} would hold {tree} to an expectation of its own "
            f"in {name}: only {', '.join(deviations.OWN)} may be held so, "
            "on the user's word of 14 October 2026, and every other "
            "scenario is held against the original's golden (the top of "
            "harness/goldens.py)")
    if owner != tree:
        raise Misfit(
            f"{name} is of {owner} alone, which is held to an expectation "
            f"of its own there, and is not run for "
            f"{tree or 'the original'}")
    if not file.exists():
        raise FileNotFoundError(
            f"{file} is not there: record it, goldens.py record --tree "
            f"{tree} {name}, and read it")
    lines = file.read_text(encoding="utf-8").split("\n")
    at = next((at for at, line in enumerate(lines)
               if line.startswith("code page ")), len(lines))
    header = lines[:at]
    numbers, why = read_header(header, tree)
    # What was read, from its line to the empty line behind it.
    read_ = next((" ".join([line[len(READ):]] + list(
        itertools.takewhile(bool, header[number + 1:])))
        for number, line in enumerate(header) if line.startswith(READ)),
        "")
    if numbers is None or not read_.strip():
        raise Misfit(
            f"{shown(file)}: its header is not filled in. It wants a line "
            f"\"Deviation: N of src/{tree}/DEVIATIONS.md\", the entry that "
            "names the scenario; \"Why: \" with a sentence on why the "
            f"original cannot go there; and \"{READ}\" with what was read "
            "of what the build showed and left, and against what. That "
            "is the statement of whoever recorded it, who has read every "
            "row: the top of harness/goldens.py says how")
    golden = parsed(name, lines, at)
    golden.tree = tree
    golden.own = file
    return golden


def parsed(name, lines, at):
    """The golden of the scenario `name` of the lines of its file, whose
    code page is the line `at`."""
    codepage = int(lines[at].split()[-1])
    codes = {char: code for code, char in enumerate(charset(codepage))}
    golden = Golden(name, codepage, [], [], {})
    at += 1
    while at < len(lines):
        line = lines[at]
        at += 1
        if line == "== disks":
            for line in lines[at:]:
                if line:
                    name, _, rest = line.partition("  ")
                    golden.disks.setdefault(name[0], {})[name[3:]] = rest
            break
        if not line.startswith("== "):
            continue
        golden.steps.append(line.partition(": ")[2])
        cursor, cursor_lines, mode = cursor_of(lines[at])
        cells = bytearray()
        for row in lines[at + 1:at + 1 + ROWS]:
            cells += cells_of(row, codes)
        at += 1 + ROWS
        blinking = []
        while blink := blink_of(lines[at], codes):
            blinking.append(blink)
            at += 1
        golden.screens.append(Screen(bytes(cells), cursor, cursor_lines,
                                     mode, tuple(blinking)))
    return golden


def read_header(header, tree):
    """Of the lines `header`, which a patch of the tree `tree` has in
    front of its screens: the numbers of the entries that it names and
    why, or None and None if it is not filled in."""
    numbers = why = None
    for number, line in enumerate(header):
        match = re.fullmatch(
            rf"Deviation: (\d+(?:, \d+)*) of "
            rf"src/{tree}/DEVIATIONS\.md", line)
        if match:
            numbers = tuple(int(word) for word in match[1].split(", "))
        if line.startswith("Why: "):
            rest = header[number + 1:]
            end = next((n for n, more in enumerate(rest) if not more),
                       len(rest))
            why = " ".join([line[5:]] + rest[:end])
    if numbers is None or not why or TO_WRITE in " ".join(header):
        return None, None
    return numbers, why


def read_patch(file, tree):
    """The patch of the file `file`, of the tree `tree`, as it is
    written: a Misfit if it is not written as a patch is."""
    def wrong(what, at=None):
        return Misfit(f"{shown(file)}"
                      + (f", line {at + 1}" if at is not None else "")
                      + f": {what}. The top of harness/goldens.py says how "
                      "a patch is written")

    lines = file.read_text(encoding="utf-8").split("\n")
    at = next((at for at, line in enumerate(lines)
               if line.startswith("== ")), len(lines))
    numbers, why = read_header(lines[:at], tree)
    if numbers is None:
        raise wrong(
            "its header is not filled in. It wants a line \"Deviation: N "
            f"of src/{tree}/DEVIATIONS.md\", N the "
            "number of the entry "
            "that declares what the patch says, or several, \"1, 7\"; and "
            "\"Why: \" with a sentence on what is otherwise here. A patch "
            "is the statement of whoever declared the deviation, who has "
            "read every row of it, and says so there")
    patch = Patch(file, numbers, why, {}, [])
    screen = None
    while at < len(lines):
        line = lines[at]
        at += 1
        if not line:
            continue
        if line.startswith("== "):
            key = line[3:]
            if key == "disks":
                screen = patch.disks
            elif key in patch.screens:
                raise wrong(f"it has \"{line}\" twice", at - 1)
            else:
                screen = patch.screens[key] = PatchedScreen()
            continue
        if screen is patch.disks:
            if line[0] not in "-+" or "  " not in line:
                raise wrong("behind \"== disks\" a line is that of a "
                            "file, with - or + in front", at - 1)
            screen.append((line[0], line[1:]))
        elif match := re.fullmatch(r"rows? (\d+)(?:-(\d+))?", line):
            first = int(match[1])
            last = int(match[2] or first)
            count = last - first + 1
            if not first <= last < ROWS:
                raise wrong(f"a screen has no {line}", at - 1)
            rows = lines[at:at + 2 * count]
            if [row[:2] for row in rows] != ["-|"] * count + ["+|"] * count:
                raise wrong(
                    f"behind \"{line}\" there are to be {count} line"
                    f"{'s' if count > 1 else ''} with - and as many with "
                    "+, each a row as a golden has it", at - 1)
            for row in range(first, last + 1):
                if row in screen.rows:
                    raise wrong(f"it has row {row} of a screen twice",
                                at - 1)
                screen.rows[row] = (rows[row - first][1:],
                                    rows[row - first + count][1:])
            at += 2 * count
        elif line.startswith("-cursor "):
            if screen.cursor or not lines[at].startswith("+cursor "):
                raise wrong("a line \"-cursor\" is followed by one "
                            "\"+cursor\", once for a screen", at - 1)
            screen.cursor = (line[1:], lines[at][1:])
            at += 1
        elif line.startswith(("-blinks ", "+blinks ")):
            screen.blinks.append((line[0], line[1:]))
        else:
            raise wrong(f"\"{line[:40]}\" is no line of a patch", at - 1)
    if not patch.count() and not patch.disks and not any(
            screen.blinks for screen in patch.screens.values()):
        raise wrong("it says nothing: no row, no cursor, no file. A "
                    "scenario that the tree shows as the original has no "
                    "patch")
    return patch


def fit(patch, golden, kept):
    """Make `patch` to `golden`, the original's golden as the rules of
    the tree have left it and before any screen is left out of it:
    `kept` are the screens that the tree is taken through. Every line of
    the patch with - has to be what the golden has there; a Misfit says
    each that is not, and apart from those each line that is not written
    as a patch has it."""
    chars = charset(golden.codepage)
    codes = {char: code for code, char in enumerate(chars)}
    # What the golden has otherwise than the patch says, and what is
    # wrong with the patch whatever the golden has.
    problems, faults = [], []
    # Of the rows that the golden has otherwise: how many are otherwise
    # only in cells that the patch leaves as they were, and how many in
    # cells that it changes.
    apart = within = 0

    def parsed(what, parse, line, again):
        """`line` of the patch, which is to be `what`, as `parse` reads
        it, or None with the problem noted: also if it is not written
        as `again` writes what was read, as a golden would have it."""
        try:
            value = parse(line)
            if value is None or again(value) != line:
                raise ValueError
            return value
        except (ValueError, KeyError, IndexError):
            faults.append(
                f"{what}: the line with + is not written as a golden has "
                f"such a line, in code page {golden.codepage}:\n"
                f"      +{line}")

    def differs(what, have, minus, plus=None):
        """Note that the golden has `have` where the patch has `minus`.
        Of a row, with `plus` what the patch expects there: the cells in
        which the two differ are marked, and whether the patch changes
        any of them is said, which is whether what the patch says of the
        row could still hold on the row as it is now."""
        text = (f"{what}: the original's golden is not what the patch "
                f"was written for\n      golden  {have}\n"
                f"      patch  -{minus}")
        try:
            now, then = cells_of(have, codes), cells_of(minus, codes)
            new = cells_of(plus, codes)
        except (ValueError, KeyError, IndexError, TypeError):
            problems.append(text)
            return
        marks, moved, meant = "", 0, 0
        for column in range(COLUMNS):
            a, b = now[column * 2:column * 2 + 2], \
                then[column * 2:column * 2 + 2]
            marks += " " if a == b else "^" if a[0] != b[0] else "~"
            if a != b:
                moved += 1
                meant += b != new[column * 2:column * 2 + 2]
        problems.append(
            f"{text}\n               {marks.rstrip()}\n"
            f"      {moved} cell{'s are' if moved > 1 else ' is'} "
            "otherwise in the golden (^ character, ~ colour only), "
            + (f"{meant} of them where the patch expects something else "
               "than the original had" if meant else
               "none of them where the patch expects something else than "
               "the original had"))
        nonlocal apart, within
        apart += not meant
        within += bool(meant)

    for key, stated in patch.screens.items():
        number, _, step = key.partition(": ")
        at = int(number) if number.isdigit() else len(golden.steps)
        if at >= len(golden.steps) or golden.steps[at] != step:
            there = (f"it has \"== {at}: {golden.steps[at]}\" there"
                     if at < len(golden.steps) else
                     f"its last is \"== {len(golden.steps) - 1}: "
                     f"{golden.steps[-1]}\"")
            same = [f"\"== {n}: {step}\"" for n in range(len(golden.steps))
                    if golden.steps[n] == step]
            problems.append(
                f"== {key}: the golden has no such screen: {there}"
                + (f". It has that step as {', '.join(same)}"
                   if same else ""))
            continue
        if at not in kept:
            faults.append(
                f"== {key}: {golden.tree} is not taken through that step, "
                "which is left out for it (deviations.TREES), and so shows "
                "no such screen")
            continue
        screen = golden.screens[at]
        cells = bytearray(screen.cells)
        for row, (minus, plus) in sorted(stated.rows.items()):
            what = f"== {key}, row {row}"
            have = row_line(screen, row, chars)
            if have != minus:
                differs(what, have, minus, plus)
            elif plus == minus:
                faults.append(f"{what}: the patch says twice what the "
                              "golden has, and nothing otherwise")
            else:
                new = parsed(what, lambda line: cells_of(line, codes), plus,
                             lambda new: row_line(replace(screen, cells=new),
                                                  0, chars))
                if new is not None:
                    cells[row * COLUMNS * 2:(row + 1) * COLUMNS * 2] = new
        screen = replace(screen, cells=bytes(cells))
        if stated.cursor:
            what = f"== {key}, the cursor"
            minus, plus = stated.cursor
            if cursor_line(screen) != minus:
                differs(what, cursor_line(screen), minus)
            elif plus == minus:
                faults.append(f"{what}: the patch says twice what the "
                              "golden has, and nothing otherwise")
            else:
                new = parsed(
                    what, cursor_of, plus,
                    lambda new: cursor_line(replace(
                        screen, cursor=new[0], cursor_lines=new[1],
                        video_mode=new[2])))
                if new is not None:
                    screen = replace(screen, cursor=new[0],
                                     cursor_lines=new[1], video_mode=new[2])
        blinking = [blink_line(blink, chars) for blink in screen.blinking]
        for sign, line in stated.blinks:
            what = f"== {key}, a cell that blinks"
            if sign == "-" and line in blinking:
                blinking.remove(line)
            elif sign == "-":
                differs(what, "\n      ".join(blinking) or "none", line)
            elif line in blinking:
                faults.append(f"{what}: the golden has it too:\n"
                              f"      {line}")
            elif parsed(what, lambda line: blink_of(line, codes), line,
                        lambda new: blink_line(new, chars)) is not None:
                blinking.append(line)
        golden.screens[at] = replace(screen, blinking=tuple(
            blink_of(line, codes) for line in blinking))
        patch.made[at] = stated
    for sign in "-+":
        # What goes before what comes: a file that is left otherwise is
        # a line of each.
        for line in (line for mark, line in patch.disks if mark == sign):
            name, _, rest = line.partition("  ")
            drive, name = name[:1], name[3:]
            what = f"== disks, {drive}:\\{name}"
            have = golden.disks.get(drive, {}).get(name)
            if drive not in golden.disks:
                faults.append(f"{what}: the golden has no such disk")
            elif sign == "-" and have != rest:
                differs(what, disk_line(drive, name, have) if have
                        else "no such file", line)
            elif sign == "-":
                del golden.disks[drive][name]
            elif have is not None:
                problems.append(
                    f"{what}: the golden has\n"
                    f"      {disk_line(drive, name, have)}\n"
                    "    there, and the patch has no line with - for it")
            else:
                golden.disks[drive][name] = rest
            patch.files.add((drive, name))
    said = []
    if problems:
        said.append(
            f"{shown(patch.file)} does not fit "
            f"{shown(path(golden.scenario))}, the original's golden that "
            "it is a patch of.\nNothing was run: it is the golden that is "
            "otherwise than the patch has it, and this says nothing of the "
            "program.\n  " + "\n  ".join(problems) + "\n"
            + to_do(patch, golden, apart, within))
    if faults:
        said.append(
            f"{shown(patch.file)} is not written as a patch is. Nothing "
            "was run.\n  " + "\n  ".join(faults) + "\n"
            "The top of harness/goldens.py says how a patch is written.")
    if said:
        raise Misfit("\n\n".join(said))


def to_do(patch, golden, apart, within):
    """What a Misfit says below the rows that do not fit: that it is the
    golden which has changed and not the program, and what follows.
    `apart` and `within` are how many rows the golden has otherwise only
    in cells that the patch leaves as they were, and in cells that it
    changes."""
    name, tree = golden.scenario, golden.tree
    paragraphs = []
    if apart or within:
        paragraphs.append((
            f"Of the rows above: {apart} otherwise only in cells that the "
            f"patch leaves as the original had them, {within} in cells "
            "where the patch expects something else. So "
            + ("what is declared for those cells has to be looked at "
               "again." if within else
               "what the patch says can hold as it is, on the rows as the "
               "original has them now."), ""))
    paragraphs.append((
        f"The patch says, row by row of the original's golden, what {tree} "
        "is expected to show otherwise, and the golden is not what it was "
        "when that was written: it has been derived again and come out "
        "otherwise, or the scenario has other steps now. `git log -- "
        f"{shown(path(name))}` says which, and why.", ""))
    paragraphs += [
        ("  - If the golden was meant to change, the patch is made again "
         "from a run and read again: python3 harness/goldens.py patch "
         f"--tree {patch.file.parent.name} {name}. That keeps its header "
         "and says of every "
         "row whether it states what the patch stated before, on a row "
         "of the original that is otherwise now, or states something "
         "else. The rows of the second kind are read against "
         "DEVIATIONS.md like those of a new patch, and a row that the "
         "declaration does not explain is the program's to mend.", "    "),
        ("  - If the golden was not meant to change, that is what to "
         "look at first, and the patch stays as it is.", "    ")]
    return "\n".join(
        textwrap.fill(paragraph, width=78, subsequent_indent=indent,
                      break_on_hyphens=False)
        for paragraph, indent in paragraphs)


def write_patch(patch, golden):
    """Write `patch` to its file: `golden` is that of its scenario, for
    the tree."""
    name = TREES[golden.tree]
    out = textwrap.wrap(
        f"{name} in the scenario {golden.scenario}: what it is expected "
        "to show otherwise than the original. A patch of "
        f"{shown(path(golden.scenario))}, see harness/goldens.py: a line "
        "with - is the golden's, a line with + is what is expected in its "
        "place.", width=78, break_on_hyphens=False)
    out.append("Deviation: "
               + (", ".join(str(number) for number in patch.deviations)
                  or TO_WRITE)
               + f" of src/{golden.tree}/DEVIATIONS.md")
    out += textwrap.wrap(f"Why: {patch.why}", width=78,
                         break_on_hyphens=False)
    for key, screen in patch.screens.items():
        out += ["", f"== {key}"]
        rows = sorted(screen.rows)
        while rows:
            count = 1
            while count < len(rows) and rows[count] == rows[0] + count:
                count += 1
            out.append(f"row {rows[0]}" if count == 1 else
                       f"rows {rows[0]}-{rows[count - 1]}")
            out += ["-" + screen.rows[row][0] for row in rows[:count]]
            out += ["+" + screen.rows[row][1] for row in rows[:count]]
            del rows[:count]
        if screen.cursor:
            out += ["-" + screen.cursor[0], "+" + screen.cursor[1]]
        out += [sign + line for sign, line in screen.blinks]
    if patch.disks:
        out += ["", "== disks"]
        out += [sign + line for sign, line in patch.disks]
    patch.file.parent.mkdir(parents=True, exist_ok=True)
    patch.file.write_text("\n".join(out) + "\n", encoding="utf-8")


def write_ran(name, executed):
    """Write which parts of the original's file ran: offsets, in hex."""
    ranges = []
    for offset in sorted(executed):
        if ranges and ranges[-1][1] == offset:
            ranges[-1][1] = offset + executed[offset]
        else:
            ranges.append([offset, offset + executed[offset]])
    words = [f"{a:04X}-{b:04X}" for a, b in ranges]
    text = "\n".join(" ".join(words[at:at + 8])
                     for at in range(0, len(words), 8))
    ran_path(name).write_text(text + "\n")


def derived(name):
    """Whether the scenario `name` has a golden."""
    return path(name).exists() and ran_path(name).exists()


def read_ran(name):
    """The offsets in the original's file of the bytes that ran in the
    scenario `name`: in VC.COM, or in VCSETUP.COM if the scenario is of
    the setup program."""
    ran = set()
    for word in ran_path(name).read_text().split():
        first, last = (int(part, 16) for part in word.split("-"))
        ran.update(range(first, last))
    return ran


def ran(setup=False, missing=None):
    """The offsets in a file of the original of the bytes that ran in
    any scenario: in VC.COM, by the scenarios of the main program, or
    with `setup` in VCSETUP.COM, by those of the setup program. The two
    files have routines of the same bytes at other offsets, and what one
    kind of scenario recorded says nothing of the other file. A scenario
    without a golden fails this, unless `missing` is a list, which then
    gets its name."""
    found = set()
    for name in scenarios.names(setup=setup):
        if missing is not None and not derived(name):
            missing.append(name)
        else:
            found |= read_ran(name)
    return found


def departures(golden, run, programs):
    """How a run departs from a golden, piece by piece and in the order
    of the scenario: of every screen that is otherwise, where it is,
    "after step 2: F3", with the text lines that say how; and behind
    them the disks, or that the golden is of other steps, under None.
    Empty if the run does not depart. `programs` as for disk_lines().
    Where the golden was read for a tree and has a patch, a row that the
    patch says is called the patch's, not the original's. Of a run that
    was cut (scenarios.Run.cut), the screens that it has are held, and
    no disks.
    """
    found = []
    steps = ("the start",) + scenarios.of(golden.scenario, golden.tree).steps
    if list(steps) != golden.steps:
        return [(None, ["the golden was derived for other steps than the "
                        "scenario has now: derive it again"])]
    for number, step in enumerate(steps[:len(run.screens)]):
        lines = screen_departs(golden, number, run.screens[number])
        if lines:
            found.append((f"after step {number}: {step}"
                          + _in_golden(golden, number), lines))
    tree = deviations.TREES.get(golden.tree)
    if run.cut or (tree is not None and golden.scenario in tree.cut):
        # A scenario that the tree is taken through only as far as a step
        # (deviations.Declared.cut) leaves its disks as they are then.
        return found
    want, got = golden.disks, disk_lines(run.disks, programs)
    if golden.tree == "posix":
        want = on_the_host(want)
        got = on_the_host(got, want)
    lines = disk_differences(
        want, got,
        golden.patch.files if golden.patch else (),
        mine="its own expectation has" if golden.own else None)
    if lines:
        found.append((None, lines))
    return found


def on_the_host(disks, golden=None):
    """The disks of disk_lines() as the program for Linux is held to
    them: without the attributes hidden, system and archive, which the
    host has nowhere for (CLAUDE.md, step 6: taken when set and not
    kept); and, of the run's, `golden` given, with a directory that the
    run has written into said so of in the golden's line too, where the
    golden has the directory, since the host moves a directory's time
    at every entry made or removed and DOS does not (a mask, on the
    user's word of 15 October 2026)."""
    out = {}
    for drive, lines in disks.items():
        out[drive] = {}
        for name, line in lines.items():
            flags, rest = line[:3].strip(), line[4:]
            if flags != "the":          # "the program under test"
                flags = "".join(c for c in flags if c not in "HSA") or "-"
                line = f"{flags:<3} {rest}"
            out[drive][name] = line
    if golden is not None:
        for drive, lines in out.items():
            for name, line in lines.items():
                was = golden.get(drive, {}).get(name)
                if WRITTEN_INTO in line and was and was.startswith("D "):
                    golden[drive][name] = line
    return out


def screen_departs(golden, number, screen):
    """How `screen`, which a build showed after step `number` of the
    scenario that `golden` is of, the start being 0, departs from the
    golden's, as text lines: none if it does not."""
    # Both are blanked, the golden's screen as well: one derived before
    # a figure was left out of the comparison still holds it. The
    # golden's is without what any tree leaves out, as it was derived,
    # and the build's without what its own tree does.
    made = golden.patch.made if golden.patch else {}
    place = golden.places[number] if golden.places else number
    expected, actual = deviations.masked(
        deviations.compared(golden.screens[number]),
        deviations.compared(screen, golden.tree),
        deviations.declared(golden.tree).rules)
    return screen_differences(
        expected, actual, made.get(place),
        mine="its own" if golden.own else "original")


def differences(golden, run, programs):
    """Describe how a run departs from a golden, as text lines: what
    departures() finds, one piece behind the other.

    An empty list means it does not.
    """
    report = []
    for where, lines in departures(golden, run, programs):
        report += ([f"--- {where}"] if where else []) + lines
    if any("the patch" in line.split("|")[0] for line in report):
        report += textwrap.wrap(
            f"What is called the patch's is what {shown(golden.patch.file)} "
            f"says {golden.tree} is to show there, in place of what the "
            "original shows. The patch fits the original's golden, so it "
            "is the program that does otherwise than was declared: mend "
            "the program, or, if it is meant to show this now, declare "
            "that first and then change the patch. A patch is not made "
            "again to make a test pass.", width=78, break_on_hyphens=False)
    if report and golden.own:
        report += textwrap.wrap(
            f"What is called its own is what {shown(golden.own)} says "
            f"{golden.tree} is to show and leave, its own expectation: the "
            "original cannot go where this scenario goes, and there is no "
            "golden of it here. The expectation was recorded from a build "
            "and read row by row, and states what is right: mend the "
            "program, or, if it is meant to show this now, record the "
            "expectation again and read every row that it changes. It is "
            "not recorded again to make a test pass.",
            width=78, break_on_hyphens=False)
    return report


def measure(golden):
    """What a golden as it was read is called in a line of a report: the
    original, with what was made to it for the tree it was read for; or
    the expectation of the tree's own."""
    if golden.own:
        return (f"{TREES[golden.tree].replace('The', 'the', 1)}'s own "
                f"expectation, {shown(golden.own)}, where there is no "
                "golden of the original")
    gone = len(scenarios.SCENARIOS[golden.scenario].steps) + 1 \
        - len(golden.screens) if golden.places else 0
    return ("the original"
            + (f" without the {gone} steps that are left out" if gone > 1
               else " without the step that is left out" if gone else "")
            + (f"{',' if gone else ''} with {golden.patch.file.name}"
               if golden.patch else ""))


def _in_golden(golden, number):
    """Where a report names the screen `number` of `golden` as a tree
    has it: what the original's golden calls it, if that is otherwise."""
    if not golden.places:
        return ""
    place, step = golden.places[number], golden.original_steps[number]
    if (place, step) == (number, golden.steps[number]):
        return ""
    return f"   (== {place}: {step} in the original's golden)"


def disk_differences(want, got, stated=(), build="this build", mine=None):
    """Describe how the disks `got` depart from `want`, both as
    disk_lines() has them, as text lines. `stated` are the files that a
    patch speaks of, (drive, path), and `build` is what `got` is called;
    `mine`, if it is given, what `want` is called for every file."""
    report = []
    for drive in sorted(want):
        for name in sorted(want[drive].keys() | got[drive].keys()):
            a, b = want[drive].get(name), got[drive].get(name)
            if a != b:
                measure = mine or (
                    "the patch has" if (drive, name) in stated
                    else "the original leaves")
                report.append(f"{drive}:\\{name}: {measure} "
                              f"{a or 'nothing'}, {build} "
                              f"{b or 'nothing'}")
    return report


def settled(expected, actual):
    """The cells of the screen `actual` with every cell that the screen
    `expected` says may blink, and that is in its other state, as
    `expected` has it."""
    cells = bytearray(actual.cells)
    for column, row, code, attribute in expected.blinking:
        at = (row * COLUMNS + column) * 2
        if cells[at:at + 2] == bytes([code, attribute]):
            cells[at:at + 2] = expected.cells[at:at + 2]
    return bytes(cells)


def screen_differences(expected, actual, stated=None, build="this build",
                       mine="original"):
    """Describe how screen `actual` departs from `expected`, as text lines.

    A cell that the original was seen to blink may be in either state.
    `stated` is what a patch says of the screen, a PatchedScreen, or
    None: a row or a cursor of `expected` that is the patch's is called
    so. `build` is what `actual` is called, and `mine` what `expected`
    is called where it is not the patch's.
    """
    cells = settled(expected, actual)
    report = []
    count = 0
    for row in range(ROWS):
        start, end = row * COLUMNS * 2, (row + 1) * COLUMNS * 2
        want, got = expected.cells[start:end], cells[start:end]
        if want == got:
            continue
        marks = ""
        for column in range(COLUMNS):
            if want[column * 2] != got[column * 2]:
                marks += "^"
            elif want[column * 2 + 1] != got[column * 2 + 1]:
                marks += "~"
            else:
                marks += " "
        count += COLUMNS - marks.count(" ")
        measure = "the patch" if stated and row in stated.rows \
            else mine
        report += [f"row {row:2}  {measure:<11}|{expected.lines()[row]}|",
                   f"        {build:<11}|{actual.lines()[row]}|",
                   f"                    {marks}"]
    if count:
        report.append(f"{count} of {COLUMNS * ROWS} cells differ "
                      "(^ character, ~ colour only)")
    measure = "the patch" if stated and stated.cursor else mine
    for name in ("cursor", "cursor_lines", "video_mode"):
        want, got = getattr(expected, name), getattr(actual, name)
        if want != got:
            report.append(f"{name.replace('_', ' ')}: {measure} {want}, "
                          f"{build} {got}")
    return report


def derive(name, benches):
    """Take the original through scenario `name` and write its golden.
    In a scenario of the setup program the original is both its
    programs: either file stands for the program under test on the
    disks, where a C tree has one, and what is written as having run is
    the setup program's."""
    scenario = scenarios.SCENARIOS[name]
    original = setups.original_vc()
    run = scenarios.run(scenario, original, benches[scenario.bench],
                        trace=True, blinking=True)
    programs = {original.filename}
    if scenario.setup:
        programs.add(setups.SETUP_PROGRAM)
    write(Golden(name, setups.codepage(scenario.country), [],
                 [deviations.compared(deviations.expected(screen))
                  for screen in run.screens],
                 disk_lines(run.disks, programs)))
    write_ran(name, run.executed)
    print(f"{name}: derived, {len(run.screens)} screen"
          f"{'s' if len(run.screens) > 1 else ''}"
          + (f", {sum(run.executed.values())} bytes of "
             f"{setups.SETUP_PROGRAM} ran" if scenario.setup else ""))


def record(name, program, benches):
    """Take `program`, the build of a tree, through the scenario `name`,
    which the tree is held to an expectation of its own in, and write
    what it showed and left as that expectation, with "Deviation" and
    "Why" of the file that was there and "Read" to be written (see the
    top). Returns the lines to print: what was written, and what is
    otherwise than in the file that was there."""
    tree = program.tree
    if deviations.owner(name) != tree or name not in deviations.OWN:
        raise Misfit(
            f"{name} is no scenario that {tree} is held to an expectation "
            "of its own in: that is declared in `own` of deviations.TREES "
            f"and allowed by deviations.OWN, which has "
            f"{', '.join(deviations.OWN)}")
    file = own_path(name, tree)
    numbers = why = old = None
    if file.exists():
        lines = file.read_text(encoding="utf-8").split("\n")
        header = lines[:next((at for at, line in enumerate(lines)
                              if line.startswith("code page ")),
                             len(lines))]
        # What was read is not kept, whatever it said.
        numbers, why = read_header(
            [line for line in header if not line.startswith(READ)], tree)
        try:
            old = read_own(name, tree)
        except Misfit:
            # Not read yet: what was there is still what to hold the
            # new one against.
            old = parsed(name, lines, len(header))
    scenario = scenarios.SCENARIOS[name]
    run = scenarios.run(scenario, program, benches[scenario.bench],
                        blinking=True)
    golden = Golden(name, setups.codepage(scenario.country), [],
                    [deviations.compared(screen, tree)
                     for screen in run.screens],
                    disk_lines(run.disks, {program.filename}),
                    tree=tree, own=file)
    write(golden, own_header(name, tree, numbers, why))
    report = [f"{name}: {shown(file)} recorded from a run of {tree}, "
              f"{len(golden.screens)} screens and the disks"
              + (", under the header that it had, but for what was read"
                 if numbers else ", its header to be written")]
    if old is not None:
        steps = ("the start",) + scenario.steps
        for number, step in enumerate(steps):
            if number >= len(old.screens):
                report.append(f"--- after step {number}: {step}: new")
                continue
            found = screen_differences(
                deviations.compared(old.screens[number]),
                golden.screens[number], build="recorded", mine="was")
            if found:
                report += [f"--- after step {number}: {step}"] + found
        report += disk_differences(old.disks, golden.disks,
                                   build="recorded", mine="was")
        if len(report) == 1:
            report.append("as the file that was there, row for row")
    return report + ["", AFTER_RECORD]


def from_run(name, program, benches):
    """Take `program`, the build of a tree, through the scenario `name`
    and write the patch of that scenario for the tree: every row that
    the build shows otherwise than the original's golden as the rules of
    the tree leave it, and every file that it leaves otherwise. The
    header is that of the patch that was there, or one to be filled in.
    Returns the lines to print: what was written, for its author to
    read."""
    tree = program.tree
    scenario = scenarios.of(name, tree)
    golden = read(name, tree, patched=False)
    if golden.steps != ["the start"] + list(scenario.steps):
        raise Misfit(f"{shown(path(name))} was derived for other steps "
                     "than the scenario has now: derive it again first, "
                     f"goldens.py derive {name}")
    file = patch_path(name, tree)
    numbers = why = old = None
    if file.exists():
        lines = file.read_text(encoding="utf-8").split("\n")
        numbers, why = read_header(
            lines[:next((at for at, line in enumerate(lines)
                         if line.startswith("== ")), len(lines))], tree)
        try:
            old = read_patch(file, tree)
        except Misfit:
            pass                  # not filled in, or not a patch: no
                                  # statement to hold the new one against
    run = scenarios.run(scenario, program, benches[scenario.bench],
                        blinking=True)
    chars = charset(golden.codepage)
    patch = Patch(file, numbers or (), why or TO_WRITE, {}, [])
    for number, place in enumerate(golden.places):
        # The row with - is the golden's as it is, the row with + what
        # the build shows as it is compared: with what is not compared
        # blanked, and a cell that the original blinks as the golden
        # has it.
        want = golden.screens[number]
        got = deviations.compared(run.screens[number], tree)
        got = replace(got, cells=settled(deviations.compared(want), got))
        stated = PatchedScreen()
        for row in range(ROWS):
            start, end = row * COLUMNS * 2, (row + 1) * COLUMNS * 2
            if deviations.compared(want).cells[start:end] \
                    != got.cells[start:end]:
                stated.rows[row] = (row_line(want, row, chars),
                                    row_line(got, row, chars))
        if cursor_line(want) != cursor_line(got):
            stated.cursor = (cursor_line(want), cursor_line(got))
        ours = {blink_line(blink, chars) for blink in want.blinking}
        theirs = {blink_line(blink, chars) for blink in got.blinking}
        stated.blinks = [("-", line) for line in sorted(ours - theirs)] \
            + [("+", line) for line in sorted(theirs - ours)]
        if stated.rows or stated.cursor or stated.blinks:
            patch.screens[f"{place}: {golden.original_steps[number]}"] = \
                stated
    left = disk_lines(run.disks, {program.filename})
    for drive in sorted(golden.disks):
        for entry in sorted(golden.disks[drive].keys() | left[drive].keys()):
            a, b = golden.disks[drive].get(entry), left[drive].get(entry)
            if a != b:
                patch.disks += [(sign, disk_line(drive, entry, text))
                                for sign, text in (("-", a), ("+", b))
                                if text]
    if not patch.screens and not patch.disks:
        return [f"{name}: {tree} shows and leaves what the original's "
                "golden has"
                + (", with the steps that are left out for it left out"
                   if len(golden.places) < len(read_original(name).steps)
                   else "") + ": there is nothing to patch"
                + (f". {shown(file)} is there and says otherwise: the "
                   "test fails for it. Take it out if the deviation is "
                   "gone" if file.exists() else "")]
    write_patch(patch, golden)
    return [f"{name}: {shown(file)} written from a run of {tree}, "
            f"{patch.count()} line{'s' if patch.count() > 1 else ''} "
            f"expected otherwise on {len(patch.screens)} screen"
            f"{'s' if len(patch.screens) != 1 else ''}"
            + (" and the disks" if patch.disks else "")
            + (", under the header that it had" if numbers else
               f", its header {TO_WRITE.lower()}")] \
        + changes(name, tree, patch) \
        + (restated(old, patch, golden.codepage) if old else [])


def restated(old, new, codepage):
    """How the patch `new`, just written from a run, stands to the patch
    `old` that was there, as text lines: which of its lines say what the
    old one said, and which are to be read as new. A row says the same
    if it expects the same in the same cells, also where the row of the
    original around those cells has come out otherwise since."""
    codes = {char: code for code, char in enumerate(charset(codepage))}

    def expects(pair):
        """What a row of a patch expects otherwise than the original
        has: the cells, each with what is expected there."""
        try:
            minus, plus = (cells_of(line, codes) for line in pair)
        except (ValueError, KeyError, IndexError):
            return None
        return [(at, bytes(plus[at:at + 2])) for at in range(0, len(plus), 2)
                if minus[at:at + 2] != plus[at:at + 2]]

    same = moved = 0
    kinds = {"says something else than it did": [],
             "new, and not in the patch that was there": [],
             "gone: the patch that was there had it, and the run has "
             "not": []}
    other, more, gone = kinds.values()
    empty = PatchedScreen()
    for key in list(new.screens) + [key for key in old.screens
                                    if key not in new.screens]:
        was, now = old.screens.get(key, empty), new.screens.get(key, empty)
        for row in sorted(was.rows.keys() | now.rows.keys()):
            what = f"== {key}, row {row}"
            if row not in now.rows:
                gone.append(what)
            elif row not in was.rows:
                more.append(what)
            elif was.rows[row] == now.rows[row]:
                same += 1
            elif expects(was.rows[row]) == expects(now.rows[row]) \
                    is not None:
                moved += 1
            else:
                other.append(what)
        if was.cursor == now.cursor:
            same += bool(now.cursor)
        else:
            (other if was.cursor and now.cursor else
             more if now.cursor else gone).append(f"== {key}, the cursor")
        same += len(set(was.blinks) & set(now.blinks))
        more += [f"== {key}, {sign}{line}"
                 for sign, line in now.blinks if (sign, line) not in was.blinks]
        gone += [f"== {key}, {sign}{line}"
                 for sign, line in was.blinks if (sign, line) not in now.blinks]
    same += len(set(old.disks) & set(new.disks))
    more += [f"== disks, {sign}{line}" for sign, line in new.disks
             if (sign, line) not in old.disks]
    gone += [f"== disks, {sign}{line}" for sign, line in old.disks
             if (sign, line) not in new.disks]
    report = textwrap.wrap(
        f"Against the patch that was there: {same} lines are as they "
        f"were; {moved} rows expect the same in the same cells, on a row "
        "of the original that is otherwise now; "
        f"{len(other)} say something else than they did, {len(more)} are "
        f"new and {len(gone)} are gone.", width=78)
    for kind, some in kinds.items():
        report += [f"  {kind}:"] + [f"    {what}" for what in some] \
            if some else []
    return report + textwrap.wrap(
        "Those lines are what this run has that the patch did not say: "
        "they are the ones to read against DEVIATIONS.md."
        if other or more or gone else
        "The patch says what it said before: nothing but the original's "
        "rows under it has changed.", width=78)


# What `patch` says when it has written: a run is where the rows of a
# patch come from, not what makes them right.
AFTER_PATCH = """\
What was written is what the build showed, and not yet a patch. Read each
row above against the entry of DEVIATIONS.md that declares it: a row that
the declaration does not explain is a change that was not meant, and is
mended in the program, or declared first if it is meant. Then fill in the
header, the number of that entry and in a sentence why these rows are
otherwise, and commit the file as your statement of what the tree is to
show. From then on the tests hold the program to the patch, not to
another run: where they fail, it is the program or the golden that is
looked at, and the patch is not made again to make them pass."""


AFTER_RECORD = """\
What was written is what the build showed and left, and not yet the
expectation. Read every row of every screen against the step that it
follows and against the entry of DEVIATIONS.md, and the disks: what is
there that the scenario does not explain is the program's to mend. Then
fill in the header, the number of the entry, why the original cannot go
there, and what was read and against what, and commit the file as your
statement of what the tree is to show. Recorded again, the lines above
say what is otherwise than before, and are the reading list."""


def changes(name, tree, patch=None):
    """Describe what the patch of the tree `tree` for the scenario `name`
    says, as text lines: of each screen the rows that are expected
    otherwise, against the original's, and the disks; and which steps of
    the original are left out for the tree. With `patch`, of that patch
    as it was just written, whatever its header. What the rules of the
    tree make otherwise is on both sides, and is not shown."""
    theirs = read(name, tree, patched=False)
    ours = read(name, tree, patch or True)
    whole = read_original(name).steps
    report = []
    for at, step in enumerate(whole):
        if at not in theirs.places:
            report.append(f"--- step {at} of the original is left out: "
                          f"{step}")
            continue
        number = theirs.places.index(at)
        found = screen_differences(theirs.screens[number],
                                   ours.screens[number],
                                   build="the patch")
        if found:
            report += [f"--- after step {at}: {step}"
                       + (f"   (step {number} of {tree}"
                          + (f", {ours.steps[number]}"
                             if ours.steps[number] != step else "") + ")"
                          if (number, ours.steps[number]) != (at, step)
                          else "")] + found
    report += disk_differences(theirs.disks, ours.disks, build="the patch")
    return report


def otherwise(tree):
    """The scenarios that the tree `tree` is not held in against the
    original's golden as it is, in their order: those that it has a
    patch for, by the files that are there, and those that steps are
    left out of for it."""
    declared = deviations.declared(tree)
    return [name for name in scenarios.names(tree)
            if patch_path(name, tree).exists()
            or len(declared.kept(scenarios.SCENARIOS[name].steps, name))
            < len(scenarios.SCENARIOS[name].steps)]


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("command",
                        choices=["derive", "patch", "changes", "record"])
    parser.add_argument("scenario", nargs="*",
                        help=", ".join(scenarios.SCENARIOS))
    parser.add_argument("--tree", choices=sorted(TREES),
                        help="for patch, changes and record: the tree "
                             "whose patches, or expectation of its own, "
                             "are meant")
    parser.add_argument("--together", type=int, metavar="N",
                        default=TOGETHER,
                        help="for derive: so many scenarios side by side "
                             f"(default: {TOGETHER} here)")
    args = parser.parse_args()
    for name in args.scenario:
        if name not in scenarios.SCENARIOS:
            parser.error(f"no scenario {name}")
    if args.command == "derive":
        if args.tree:
            parser.error("derive is of the original: a tree has patches "
                         "of the original's goldens, and none of its own")
        for name in args.scenario:
            if deviations.owner(name) is not None:
                parser.error(f"{name} is not run for the original: "
                             + scenarios.not_run()[name])
        names = args.scenario or scenarios.names()
        with setups.Benches() as benches, \
                ThreadPoolExecutor(args.together) as pool:
            # Each says that it is done when it is; a failure ends the
            # tool when those before it are done.
            list(pool.map(lambda name: derive(name, benches), names))
        return
    if not args.tree:
        parser.error(f"{args.command} wants --tree")
    run = scenarios.names(args.tree)
    for name in args.scenario:
        if name not in run:
            parser.error(f"{name} is not run for {args.tree}: "
                         + scenarios.not_run(args.tree)[name])
        own = deviations.owner(name) == args.tree
        if own != (args.command == "record"):
            parser.error(
                f"{name} is held to an expectation of {args.tree}'s own, "
                "which is recorded: goldens.py record" if own else
                f"{name} is held against the original's golden, and "
                "record is for a scenario that a tree is held to an "
                "expectation of its own in")
    try:
        if args.command == "record":
            if not args.scenario:
                parser.error("record wants the scenario named")
            program = setups.Program.load(
                deps.ROOT / "build" / args.tree / "VC.EXE")
            with setups.Benches() as benches:
                for name in args.scenario:
                    print("\n".join(record(name, program, benches)))
            return
        if args.command == "changes":
            for name in args.scenario or otherwise(args.tree):
                if not patch_path(name, args.tree).exists():
                    print(f"{name}: no patch")
                else:
                    patch = read(name, args.tree).patch
                    print(f"{name}: {shown(patch.file)}, deviation "
                          f"{', '.join(map(str, patch.deviations))}: "
                          f"{patch.why}")
                print("\n".join(changes(name, args.tree)
                                or ["as the original's golden"]))
            return
        if not args.scenario:
            parser.error(
                "patch wants the scenarios named: a patch is written for "
                "a difference that has been declared and is then read, "
                "one at a time")
        program = setups.Program.load(
            deps.ROOT / "build" / args.tree / "VC.EXE")
        with setups.Benches() as benches:
            for name in args.scenario:
                print("\n".join(from_run(name, program, benches)))
        print("\n" + AFTER_PATCH)
    except Misfit as misfit:
        sys.exit(str(misfit))


if __name__ == "__main__":
    main()
