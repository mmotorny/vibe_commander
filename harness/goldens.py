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
difference in it: it holds what a build is expected to show.

    goldens.py derive [SCENARIO...]     derive goldens from the original;
                                        all of them if none is named

The files are text. A screen is 25 lines, each the row between bars and
then its colours, as runs of an attribute byte: 1Fx80. Bytes that are not
printable ASCII are written as the characters they show as in the code
page of the scenario's country.
"""
import argparse
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

import deviations
import scenarios
import setups
from dos import COLUMNS, CP437_CONTROLS, ROWS, Screen

DIRECTORY = Path(__file__).resolve().parent / "goldens"

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
    qemu: str                     # the version that the original ran on
    steps: list                   # what was done before each screen
    screens: list
    disks: dict                   # drive -> path -> line describing it


def path(name):
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
    out = [f"Volkov Commander 4.05 in the scenario {golden.scenario}: what "
           "the original shows and",
           "leaves on its disks. Derived by harness/goldens.py; not to be "
           "edited.",
           f"code page {golden.codepage}",
           f"recorded on {golden.qemu}"]
    steps = ("the start",) + scenarios.SCENARIOS[golden.scenario].steps
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
    DIRECTORY.mkdir(exist_ok=True)
    path(golden.scenario).write_text("\n".join(out) + "\n", encoding="utf-8")


def read(name):
    lines = path(name).read_text(encoding="utf-8").split("\n")
    codepage = int(lines[2].split()[-1])
    codes = {char: code for code, char in enumerate(charset(codepage))}
    golden = Golden(name, codepage, lines[3].partition(" on ")[2], [], [],
                    {})
    at = 4
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
    steps = ("the start",) + scenarios.SCENARIOS[golden.scenario].steps
    if list(steps) != golden.steps:
        return ["the golden was derived for other steps than the scenario "
                "has now: derive it again"]
    for number, step in enumerate(steps):
        found = screen_differences(golden.screens[number],
                                   run.screens[number])
        if found:
            report += [f"--- after step {number}: {step}"] + found
    want, got = golden.disks, disk_lines(run.disks, programs)
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
                        signals=False, trace=True)
    # Runs do not keep the same time of day: one waits longer for a step
    # to finish than another. Within one minute nothing on the screen
    # tells them apart.
    if run.began // 60 != run.ended // 60:
        sys.exit(f"{name}: recording took the guest's clock into the next "
                 f"minute, after {run.ended - run.began:.0f} s; make the "
                 "scenario shorter")
    version = subprocess.run(["qemu-system-i386", "--version"],
                             capture_output=True, text=True).stdout
    codepage = (setups.COUNTRIES[scenario.country] or (1, 437))[1]
    write(Golden(name, codepage, version.split("\n")[0], [],
                 [deviations.expected(screen) for screen in run.screens],
                 disk_lines(run.disks, {original.filename})))
    write_ran(name, run.executed)
    print(f"{name}: derived, {len(run.screens)} screen"
          f"{'s' if len(run.screens) > 1 else ''} in "
          f"{run.ended - run.began:.0f} s of the guest's time")


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("command", choices=["derive"])
    parser.add_argument("scenario", nargs="*",
                        help=", ".join(scenarios.SCENARIOS))
    args = parser.parse_args()
    with setups.Benches() as benches:
        for name in args.scenario or scenarios.SCENARIOS:
            derive(name, benches)


if __name__ == "__main__":
    main()
