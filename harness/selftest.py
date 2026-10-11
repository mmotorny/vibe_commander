#!/usr/bin/env python3
"""Check that the comparison in compare.py can be trusted.

A comparison that reports "as the original" is only worth something if it
would have reported a difference. This runs the original itself and expects
it to show what the golden of its start screen holds, which also says that
the golden can still be derived on this computer. It then runs a copy with
one letter of the function key bar changed and expects exactly that one
cell to be reported.
"""
import sys
from dataclasses import replace

import goldens
import scenarios
import setups
from dos import COLUMNS, ROWS

LABEL, ALTERED = b"RenMov", b"RenMow"       # the F6 label on the bottom row


def main():
    original = setups.original_vc()
    if original.contents.count(LABEL) != 1:
        sys.exit(f"expected {LABEL!r} exactly once in the original VC.COM")
    changed = replace(original,
                      contents=original.contents.replace(LABEL, ALTERED))
    golden = goldens.read("start")
    scenario = scenarios.SCENARIOS["start"]

    with setups.Benches() as benches:
        bench = benches[scenario.country]
        report = goldens.differences(
            golden, scenarios.run(scenario, original, bench),
            {original.filename})
        if report:
            print("\n".join(report))
            sys.exit("the original was reported to differ from its golden")
        screen = scenarios.run(scenario, changed, bench).screens[0]

    expected = golden.screens[0]
    differing = [i // 2 for i in range(len(expected.cells))
                 if expected.cells[i] != screen.cells[i]]
    bottom_row = expected.lines()[ROWS - 1]
    cell = (ROWS - 1) * COLUMNS + bottom_row.index(LABEL.decode()) + 5
    found = goldens.screen_differences(expected, screen)
    if differing != [cell] or not found:
        print("\n".join(found))
        sys.exit(f"expected exactly cell {cell} to differ, found {differing}")

    print("comparison self-test passed: the original matches its golden, "
          "and a one-letter change is reported as one cell")


if __name__ == "__main__":
    main()
