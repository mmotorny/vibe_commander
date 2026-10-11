#!/usr/bin/env python3
"""Check that the comparison in compare.py can be trusted.

A comparison that reports "identical" is only worth something if it would
have reported a difference. This runs the original program twice and
expects no differences, then runs a copy with one letter of the function
key bar changed and expects exactly that one cell to be reported.
"""
import sys

import setups
from compare import differences, start_screen
from dos import COLUMNS, ROWS

LABEL, ALTERED = b"RenMov", b"RenMow"       # the F6 label on the bottom row


def main():
    original = setups.original_vc()
    if original.count(LABEL) != 1:
        sys.exit(f"expected {LABEL!r} exactly once in the original VC.COM")

    first = start_screen(original)
    second = start_screen(original)
    report = differences(first, second)
    if report:
        print("\n".join(report))
        sys.exit("two runs of the same program were reported as different")

    changed = start_screen(original.replace(LABEL, ALTERED))
    differing = [i // 2 for i in range(len(first.cells))
                 if first.cells[i] != changed.cells[i]]
    bottom_row = first.lines()[ROWS - 1]
    expected = (ROWS - 1) * COLUMNS + bottom_row.index(LABEL.decode()) + 5
    if differing != [expected] or not differences(first, changed):
        print("\n".join(differences(first, changed)))
        sys.exit(f"expected exactly cell {expected} to differ, "
                 f"found {differing}")

    print("comparison self-test passed: identical runs match, and a "
          "one-letter change is reported as one cell")


if __name__ == "__main__":
    main()
