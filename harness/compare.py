#!/usr/bin/env python3
"""Compare a build of VC.COM with the original, running each under FreeDOS.

Both programs are started on identical machines (the test bench described
in setups.py). Their text screens are then compared cell by cell, character
and colour attribute, along with the cursor and the video mode.

Exits with status 0 if they are indistinguishable and 1 if they differ.
"""
import argparse
import sys
import tempfile
from pathlib import Path

import setups
from dos import COLUMNS, ROWS


def start_screen(vc_com):
    """The screen `vc_com` shows once it has started on the test bench."""
    with tempfile.TemporaryDirectory() as workdir:
        with setups.test_bench(Path(workdir), vc_com) as machine:
            machine.wait_for_boot()
            return machine.wait_for_screen()


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


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("candidate", type=Path, help="the VC.COM to test")
    args = parser.parse_args()

    original = start_screen(setups.original_vc())
    candidate = start_screen(args.candidate.read_bytes())
    report = differences(original, candidate)
    if report:
        print("\n".join(report))
        sys.exit(1)
    print("\n".join(candidate.lines()))
    print("start screen: identical to the original")


if __name__ == "__main__":
    main()
