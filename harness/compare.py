#!/usr/bin/env python3
"""Compare a build of Volkov Commander with the original, under FreeDOS.

Both programs are taken through the same scenarios (see scenarios.py) on
identical machines. After the start and after every step their text screens
are compared cell by cell, character and colour attribute, along with the
cursor and the video mode; at the end, so is everything on their disks.

Where a build is meant to differ from the original, deviations.py says how;
anything else is a failure.

Exits with status 0 if they are indistinguishable and 1 if they differ.
"""
import argparse
import sys
from concurrent.futures import ThreadPoolExecutor

import deviations
import scenarios
import setups
from dos import COLUMNS, ROWS


def differences(expected, actual):
    """Describe how screen `actual` departs from `expected`, as text lines.

    An empty list means the two are indistinguishable.
    """
    report = []
    cells = 0
    for row in range(ROWS):
        start, end = row * COLUMNS * 2, (row + 1) * COLUMNS * 2
        want, got = expected.cells[start:end], actual.cells[start:end]
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
        cells += COLUMNS - marks.count(" ")
        report += [f"row {row:2}  original  |{expected.lines()[row]}|",
                   f"        candidate |{actual.lines()[row]}|",
                   f"                   {marks}"]
    if cells:
        report.append(f"{cells} of {COLUMNS * ROWS} cells differ "
                      "(^ character, ~ colour only)")
    for name in ("cursor", "cursor_lines", "video_mode", "blinking"):
        want, got = getattr(expected, name), getattr(actual, name)
        if want != got:
            report.append(f"{name.replace('_', ' ')}: original {want}, "
                          f"candidate {got}")
    return report


def disk_differences(expected, actual, programs):
    """Describe how the disks of two runs differ, as text lines.

    `programs` are the file names of the two programs under test, which
    stand for each other. A time stamp may differ only where both files
    were written during the run: the two runs do not keep the same time.
    """
    report = []
    for drive in sorted(expected):
        want = {_neutral(path, programs): entry
                for path, entry in expected[drive].items()}
        got = {_neutral(path, programs): entry
               for path, entry in actual[drive].items()}
        for path in sorted(want.keys() | got.keys()):
            name = f"{drive}:\\{path}"
            if path not in got:
                report.append(f"{name}: only the original has it")
            elif path not in want:
                report.append(f"{name}: only the candidate has it")
            elif path.endswith("<program>"):
                continue
            else:
                a, b = want[path], got[path]
                if (a.attributes, a.size, a.sha256) != \
                        (b.attributes, b.size, b.sha256):
                    report.append(
                        f"{name}: original has attributes "
                        f"{a.attributes:02x}h and {a.size} bytes, candidate "
                        f"{b.attributes:02x}h and {b.size} bytes"
                        + ("" if a.sha256 == b.sha256
                           else "; the contents differ"))
                elif (a.date, a.time) != (b.date, b.time) and \
                        min((a.date, a.time), (b.date, b.time)) < \
                        setups.DOS_CLOCK:
                    report.append(f"{name}: the time stamps differ")
    return report


def _neutral(path, programs):
    directory, _, name = path.rpartition("\\")
    if name in programs:
        name = "<program>"
    return f"{directory}\\{name}" if directory else name


def compare(name, original, candidate, trace=False):
    """Run scenario `name` on both programs.

    Returns the differences as text lines, and the two runs.
    """
    scenario = scenarios.SCENARIOS[name]
    with ThreadPoolExecutor() as pool:
        expected, actual = pool.map(
            lambda program: scenarios.run(scenario, program, trace),
            (original, candidate))
    report = []
    steps = ("start",) + scenario.steps
    for number, step in enumerate(steps):
        found = differences(deviations.declared(expected.screens[number]),
                            deviations.declared(actual.screens[number]))
        if found:
            report += [f"--- after step {number}: {step}"] + found
    report += disk_differences(expected.disks, actual.disks,
                               {original.filename, candidate.filename})
    return report, expected, actual


def compare_all(names, original, candidate, trace=False):
    """Compare the programs on each scenario of `names`, printing a line
    for each and the differences found.

    Returns whether any differed, and the runs of each program.
    """
    failed = False
    runs = ([], [])
    for name in names:
        report, expected, actual = compare(name, original, candidate, trace)
        runs[0].append(expected)
        runs[1].append(actual)
        if report:
            failed = True
            print(f"{name}: differs from the original")
            print("\n".join(report))
        else:
            steps = len(scenarios.SCENARIOS[name].steps)
            print(f"{name}: identical to the original "
                  f"({steps + 1} screen{'s' if steps else ''} and the disks)")
    return failed, runs


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("candidate", help="the VC.COM or VC.EXE to test")
    parser.add_argument("scenario", nargs="*", default=["start"],
                        help="scenarios to run (default: start; 'all' for\n"
                             "every one): "
                             + ", ".join(scenarios.SCENARIOS))
    args = parser.parse_args()
    names = args.scenario
    if names == ["all"]:
        names = list(scenarios.SCENARIOS)

    failed, _ = compare_all(names, setups.original_vc(),
                            setups.Program.load(args.candidate))
    sys.exit(failed)


if __name__ == "__main__":
    main()
