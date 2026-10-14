#!/usr/bin/env python3
"""Show what Volkov Commander shows after each of some steps.

For finding out what the program does with keys before a scenario is
written, and for looking at a build where it departs from a golden. The
program is started on the test bench as for a scenario, the steps are
pressed, and the screen after the start and after each step is printed.

    show.py F9 Enter "Down Enter"
    show.py --country ru Ctrl+PgUp

Steps are written as in scenarios.py; one that presses several keys needs
quotes around it. The program is the original unless another is named:
--program build/c/VC.EXE is the C translation, and
--program build/portable_c/VC.EXE the portable C.

With --setup the bench has the original's setup program next to the
original, for a step to start, as a scenario of the setup program has it
(see the top of scenarios.py):

    show.py --setup '"d:\\vc\\vcsetup>nul" Enter' F2

A screen is 25 rows, and most of what a key does is in a few of them:

    show.py --changes F9 o Enter        of each screen only the rows that
                                        differ from the screen before, with
                                        ~ where only the colours differ
                                        and, when a dialog closes, in a
                                        line the rows that are as they
                                        were after an earlier step
    show.py --last F9 o Enter           only the screen after the last step

A step that the program does not get through, because it waits for a key
that the step has not, or hangs, ends the run when nothing has come of it
for five minutes, as in a test, and the screen it stopped on is printed.
For trying steps out that is long:

    show.py --limit 10 F5 Enter         give a step up after 10 seconds
"""
import argparse
import sys

import scenarios
import setups
from dos import MachineError

# With --changes, how many rows have to be as they were after an earlier
# step for them to be named in a line instead of printed.
AGAIN = 3


def add_arguments(parser):
    """Give `parser` the options that say how much of the screens to
    print, and how long a step may take."""
    parser.add_argument("--changes", action="store_true",
                        help="print of each screen only the rows that "
                             "differ from the screen before, numbered,\n"
                             "with ~ where only the colours differ, and "
                             "in a line the rows\nthat are again as they "
                             "were after an earlier step")
    parser.add_argument("--last", action="store_true",
                        help="print only the screen after the last step")
    parser.add_argument("--limit", type=float, metavar="SECONDS",
                        default=scenarios.STEP_TIME,
                        help="give a step up when nothing has come of it "
                             "for so long, and print the\nscreen it "
                             f"stopped on (default: {scenarios.STEP_TIME}, "
                             "as in a test)")


def run(scenario, program, bench, limit, **how):
    """scenarios.run(), ending the tool with what it says of a step that
    the program did not get through."""
    try:
        return scenarios.run(scenario, program, bench, limit=limit, **how)
    except MachineError as error:
        sys.exit(str(error))


def print_screens(steps, screens, codepage, changes=False, last=False):
    """Print the screens that follow the start and each of `steps`, as
    text in `codepage`: all of each, or what add_arguments() offers."""
    earlier = []                  # of every screen so far: rows, cells
    names = ("the start",) + tuple(steps)
    for number, (step, screen) in enumerate(zip(names, screens)):
        rows = [line.rstrip() for line in screen.lines(codepage)]
        width = len(screen.cells) // len(rows)
        cells = [screen.cells[row * width:(row + 1) * width]
                 for row in range(len(rows))]
        if not last or number == len(names) - 1:
            print(f"== {number}: {step}    cursor at column "
                  f"{screen.cursor[0]}, row {screen.cursor[1]}")
            if changes and earlier:
                before = earlier[-1]
                changed = [row for row in range(len(rows))
                           if cells[row] != before[1][row]]
                # When a dialog closes, most of what changes is again as
                # it was after an earlier step. Such rows are only named:
                # those of the step that has the most of them, the last
                # such step, if they are more than half and at least
                # AGAIN.
                then, again = 0, []
                for step_before in range(number - 1):
                    same = [row for row in changed
                            if earlier[step_before][1][row] == cells[row]]
                    if len(same) >= len(again):
                        then, again = step_before, same
                if len(again) < max(AGAIN, len(changed) // 2 + 1):
                    again = []
                for row in changed:
                    if row not in again:
                        # A row whose colours alone have changed is
                        # marked ~.
                        mark = "~" if rows[row] == before[0][row] else " "
                        print(f"{row:2}{mark}{rows[row]}")
                if again:
                    print(f"   as after step {then} again: rows "
                          f"{_ranges(again)}")
                if not changed:
                    print("   no row has changed")
            else:
                print("\n".join(rows))
        earlier.append((rows, cells))


def _ranges(numbers):
    """Rising `numbers` as text, "5-12 24"."""
    found = []
    for number in numbers:
        if found and found[-1][1] == number - 1:
            found[-1][1] = number
        else:
            found.append([number, number])
    return " ".join(str(first) if first == last else f"{first}-{last}"
                    for first, last in found)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("step", nargs="*", help="keys to press")
    parser.add_argument("--country", default="us",
                        choices=sorted(setups.COUNTRIES),
                        help="the country DOS is configured for")
    parser.add_argument("--program", help="the VC.COM or VC.EXE to run "
                                          "(default: the original)")
    parser.add_argument("--memory", type=int,
                        help="the machine's memory in megabytes (default: "
                             "the bench's)")
    parser.add_argument("--setup", action="store_true",
                        help="have the original's setup program on the "
                             "bench, next to the original")
    add_arguments(parser)
    args = parser.parse_args()

    program = setups.Program.load(args.program) if args.program \
        else setups.original_vc()
    scenario = scenarios.Scenario("", tuple(args.step), args.country,
                                  setup=args.setup, memory=args.memory)
    with setups.Benches() as benches:
        made = run(scenario, program, benches[scenario.bench], args.limit)
    print_screens(scenario.steps, made.screens,
                  setups.codepage(args.country),
                  args.changes, args.last)


if __name__ == "__main__":
    main()
