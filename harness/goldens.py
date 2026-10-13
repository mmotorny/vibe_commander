#!/usr/bin/env python3
"""Goldens: what the original Volkov Commander does in each scenario.

The original is the measure of every build, but it need not be run each
time one is tested. It is taken through a scenario once, and what it shows
and leaves behind is written to harness/goldens/: after the start and after
every step the text screen, cell by cell, with the cursor and the video
mode, and at the end the files on the disks. Next to that goes a list of
the original's code that ran, which ported.py holds against the routines
that have been translated.

A golden is derived, never edited. Where a build is meant to differ from
the original, deviations.py says how, and the golden is derived with that
difference in it: it holds what a build is expected to show. What is not
compared at all is blanked on both screens when a build is held against a
golden, so a golden need not be derived again when more is left out.

    goldens.py derive [SCENARIO...]     derive goldens from the original;
                                        all of them if none is named

A C tree that is declared to behave otherwise than the original, the
portable C, cannot be held against the original in the scenarios that
show it (see deviations.TREES, and src/portable_c/DEVIATIONS.md). Each of
those has a golden of the tree's own, in a directory of the tree's name
next to the original's: recorded from the tree as it is built, in the
same form, and saying in its first line where it is from. It is no
measure of the tree: it shows that nothing has changed since it was
recorded. Whoever records one reads it, against the original's:

    goldens.py record --tree portable_c [SCENARIO...]
                                        record the goldens of a tree from
                                        its build; all that are declared
                                        if none is named
    goldens.py changes --tree portable_c [SCENARIO...]
                                        print how each golden of a tree
                                        differs from the original's, the
                                        rows of each screen that do

The files are text. A screen is 25 lines, each the row between bars and
then its colours, as runs of an attribute byte: 1Fx80. Bytes that are not
printable ASCII are written as the characters they show as in the code
page of the scenario's country.
"""
import argparse
import re
from dataclasses import dataclass
from pathlib import Path

import deps
import deviations
import scenarios
import setups
from dos import COLUMNS, CP437_CONTROLS, ROWS, Screen

DIRECTORY = Path(__file__).resolve().parent / "goldens"

# What a tree is called in the first line of a golden of its own.
TREES = {"portable_c": "The portable C"}

ATTRIBUTES = "RHSVDA"             # read-only, hidden, system, label, ...

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
    tree: str = None              # the tree it was recorded from, or None
                                  # for the original's


def own(name, tree):
    """Whether the tree `tree` has a golden of its own for the scenario
    `name`: if it is declared to change it."""
    return name in deviations.declared(tree).changed


def path(name, tree=None):
    """Where the golden of the scenario `name` is that a program of the
    tree `tree` is held against: the tree's own, if it is declared to
    change the scenario, and the original's otherwise."""
    if own(name, tree):
        return DIRECTORY / tree / f"{name}.txt"
    return DIRECTORY / f"{name}.txt"


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
            if entry.date == date and time < entry.time < time + RUN_TIME:
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


def write(golden):
    chars = charset(golden.codepage)
    if golden.tree:
        out = [f"{TREES[golden.tree]} in the scenario {golden.scenario}: "
               "what it showed and left on its disks",
               "when this was recorded, not what the original does: see "
               f"src/{golden.tree}/DEVIATIONS.md. Not to be edited.",
               f"code page {golden.codepage}"]
    else:
        out = [f"Volkov Commander 4.05 in the scenario {golden.scenario}: "
               "what the original shows and",
               "leaves on its disks. Derived by harness/goldens.py; not to "
               "be edited.",
               f"code page {golden.codepage}"]
    steps = ("the start",) + scenarios.of(golden.scenario, golden.tree).steps
    for number, (step, screen) in enumerate(zip(steps, golden.screens)):
        out += ["", f"== {number}: {step}",
                f"cursor {screen.cursor[0]} {screen.cursor[1]} lines "
                f"{screen.cursor_lines[0]} {screen.cursor_lines[1]} mode "
                f"{screen.video_mode}"]
        for row in range(ROWS):
            cells = screen.cells[row * COLUMNS * 2:(row + 1) * COLUMNS * 2]
            text = "".join(chars[code] for code in cells[0::2])
            runs = []
            for attribute in cells[1::2]:
                if runs and runs[-1][0] == attribute:
                    runs[-1][1] += 1
                else:
                    runs.append([attribute, 1])
            out.append(f"|{text}| "
                       + " ".join(f"{a:02X}x{n}" for a, n in runs))
        for column, row, code, attribute in screen.blinking:
            out.append(f"blinks {column} {row} to |{chars[code]}| "
                       f"{attribute:02X}")
    out += ["", "== disks"]
    for drive in sorted(golden.disks):
        for name in sorted(golden.disks[drive]):
            out.append(f"{drive}:\\{name}  {golden.disks[drive][name]}")
    file = path(golden.scenario, golden.tree)
    file.parent.mkdir(parents=True, exist_ok=True)
    file.write_text("\n".join(out) + "\n", encoding="utf-8")


def read(name, tree=None):
    """The golden that a program of the tree `tree` is held against in
    the scenario `name`: the original's, or the tree's own where it is
    declared to change the scenario."""
    file = path(name, tree)
    if not file.exists():
        raise FileNotFoundError(
            f"{file} is not there: " + (
                f"record it, goldens.py record --tree {tree} {name}"
                if own(name, tree) else f"derive it, goldens.py derive "
                                        f"{name}"))
    lines = file.read_text(encoding="utf-8").split("\n")
    codepage = int(lines[2].split()[-1])
    codes = {char: code for code, char in enumerate(charset(codepage))}
    golden = Golden(name, codepage, [], [], {},
                    tree if own(name, tree) else None)
    at = 3
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
        _, x, y, _, first, last, _, mode = lines[at].split()
        cells = bytearray()
        for row in lines[at + 1:at + 1 + ROWS]:
            text, runs = row[1:1 + COLUMNS], row[COLUMNS + 3:].split()
            attributes = [int(run[:2], 16) for run in runs
                          for _ in range(int(run[3:]))]
            for char, attribute in zip(text, attributes, strict=True):
                cells += bytes([codes[char], attribute])
        at += 1 + ROWS
        blinking = []
        while match := re.fullmatch(r"blinks (\d+) (\d+) to \|(.)\| (..)",
                                    lines[at]):
            blinking.append((int(match[1]), int(match[2]), codes[match[3]],
                             int(match[4], 16)))
            at += 1
        golden.screens.append(Screen(bytes(cells), (int(x), int(y)),
                                     (int(first), int(last)), int(mode),
                                     tuple(blinking)))
    return golden


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
    """The offsets in the original's file of the bytes that ran."""
    ran = set()
    for word in ran_path(name).read_text().split():
        first, last = (int(part, 16) for part in word.split("-"))
        ran.update(range(first, last))
    return ran


def differences(golden, run, programs):
    """Describe how a run departs from a golden, as text lines.

    An empty list means it does not. `programs` as for disk_lines().
    """
    report = []
    steps = ("the start",) + scenarios.of(golden.scenario, golden.tree).steps
    if list(steps) != golden.steps:
        return ["the golden was derived for other steps than the scenario "
                "has now: derive it again"]
    for number, step in enumerate(steps):
        # Both are blanked, the golden's screen as well: one derived before
        # a figure was left out of the comparison still holds it.
        found = screen_differences(
            deviations.compared(golden.screens[number]),
            deviations.compared(run.screens[number]))
        if found:
            report += [f"--- after step {number}: {step}"] + found
    return report + disk_differences(golden.disks,
                                     disk_lines(run.disks, programs))


def disk_differences(want, got):
    """Describe how the disks `got` depart from `want`, both as
    disk_lines() has them, as text lines."""
    report = []
    for drive in sorted(want):
        for name in sorted(want[drive].keys() | got[drive].keys()):
            a, b = want[drive].get(name), got[drive].get(name)
            if a != b:
                report.append(f"{drive}:\\{name}: the original leaves "
                              f"{a or 'nothing'}, this build "
                              f"{b or 'nothing'}")
    return report


def screen_differences(expected, actual):
    """Describe how screen `actual` departs from `expected`, as text lines.

    A cell that the original was seen to blink may be in either state.
    """
    cells = bytearray(actual.cells)
    for column, row, code, attribute in expected.blinking:
        at = (row * COLUMNS + column) * 2
        if cells[at:at + 2] == bytes([code, attribute]):
            cells[at:at + 2] = expected.cells[at:at + 2]
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
        report += [f"row {row:2}  original   |{expected.lines()[row]}|",
                   f"        this build |{actual.lines()[row]}|",
                   f"                    {marks}"]
    if count:
        report.append(f"{count} of {COLUMNS * ROWS} cells differ "
                      "(^ character, ~ colour only)")
    for name in ("cursor", "cursor_lines", "video_mode"):
        want, got = getattr(expected, name), getattr(actual, name)
        if want != got:
            report.append(f"{name.replace('_', ' ')}: original {want}, "
                          f"this build {got}")
    return report


def derive(name, benches):
    """Take the original through scenario `name` and write its golden."""
    scenario = scenarios.SCENARIOS[name]
    original = setups.original_vc()
    run = scenarios.run(scenario, original, benches[scenario.country],
                        trace=True, blinking=True)
    write(Golden(name, setups.codepage(scenario.country), [],
                 [deviations.compared(deviations.expected(screen))
                  for screen in run.screens],
                 disk_lines(run.disks, {original.filename})))
    write_ran(name, run.executed)
    print(f"{name}: derived, {len(run.screens)} screen"
          f"{'s' if len(run.screens) > 1 else ''}")


def record(name, program, benches):
    """Take `program`, the build of a tree, through the scenario `name`,
    which the tree is declared to change, and write the tree's own golden
    of it: what the build shows, with what is not compared blanked."""
    scenario = scenarios.of(name, program.tree)
    run = scenarios.run(scenario, program, benches[scenario.country],
                        blinking=True)
    write(Golden(name, setups.codepage(scenario.country), [],
                 [deviations.compared(screen) for screen in run.screens],
                 disk_lines(run.disks, {program.filename}), program.tree))
    print(f"{name}: recorded from {program.tree}, {len(run.screens)} "
          f"screen{'s' if len(run.screens) > 1 else ''}")


def changes(name, tree):
    """Describe how the golden of the tree `tree` for the scenario `name`
    differs from the original's, as text lines: of each screen the rows
    that differ, and the disks. Where the tree is taken through fewer
    steps than the original, a screen is held against the original's
    after the same step, which then has more behind it."""
    ours = read(name, tree)
    theirs = read(name)
    report = []
    at = 0
    for number, step in enumerate(ours.steps):
        while theirs.steps[at] != step:
            report.append(f"--- step {at} of the original is left out: "
                          f"{theirs.steps[at]}")
            at += 1
        found = screen_differences(deviations.compared(theirs.screens[at]),
                                   ours.screens[number])
        if found:
            report += [f"--- after step {number}: {step}"] + found
        at += 1
    report += disk_differences(theirs.disks, ours.disks)
    return report


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("command", choices=["derive", "record", "changes"])
    parser.add_argument("scenario", nargs="*",
                        help=", ".join(scenarios.SCENARIOS))
    parser.add_argument("--tree", choices=sorted(TREES),
                        help="for record and changes: the tree whose "
                             "goldens are meant")
    args = parser.parse_args()
    if args.command == "derive":
        if args.tree:
            parser.error("derive is of the original: record is of a tree")
        names = args.scenario or list(scenarios.SCENARIOS)
        with setups.Benches() as benches:
            for name in names:
                derive(name, benches)
        return
    if not args.tree:
        parser.error(f"{args.command} wants --tree")
    declared = deviations.declared(args.tree).changed
    names = args.scenario or list(declared)
    for name in names:
        if name not in declared:
            parser.error(
                f"{args.tree} is not declared to change the scenario "
                f"{name}: it is held against the original's golden there. "
                f"A deviation is declared first, in src/{args.tree}/"
                "DEVIATIONS.md and in harness/deviations.py")
    if args.command == "changes":
        for name in names:
            print(f"{name}: {declared[name]}")
            print("\n".join(changes(name, args.tree)
                            or ["as the original's golden"]))
        return
    program = setups.Program.load(
        deps.ROOT / "build" / args.tree / "VC.EXE")
    with setups.Benches() as benches:
        for name in names:
            record(name, program, benches)


if __name__ == "__main__":
    main()
