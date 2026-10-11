#!/usr/bin/env python3
"""Show what Volkov Commander shows after each of some steps.

For finding out what the program does with keys before a scenario is
written, and for looking at a build where it departs from a golden. The
program is started on the test bench as for a scenario, the steps are
pressed, and the screen after the start and after each step is printed.

    show.py F9 Enter "Down Enter"
    show.py --country ru Ctrl+PgUp

Steps are written as in scenarios.py; one that presses several keys needs
quotes around it. The program is the original unless another is named.
The original takes most of a second for every step; the port's build that
signals, build/port/signalling/VC.EXE with --signals, takes none.

A screen is 25 rows, and most of what a key does is in a few of them:

    show.py --changes F9 o Enter        of each screen only the rows that
                                        differ from the screen before, with
                                        ~ where only the colours differ
    show.py --last F9 o Enter           only the screen after the last step
"""
import argparse

import scenarios
import setups


def add_arguments(parser):
    """Give `parser` the options that say how much of the screens to
    print."""
    parser.add_argument("--changes", action="store_true",
                        help="print of each screen only the rows that "
                             "differ from the screen before, numbered,\n"
                             "with ~ where only the colours differ")
    parser.add_argument("--last", action="store_true",
                        help="print only the screen after the last step")


def print_screens(steps, screens, codepage, changes=False, last=False):
    """Print the screens that follow the start and each of `steps`, as
    text in `codepage`: all of each, or what add_arguments() offers."""
    before = None
    names = ("the start",) + tuple(steps)
    for number, (step, screen) in enumerate(zip(names, screens)):
        rows = [line.rstrip() for line in screen.lines(codepage)]
        width = len(screen.cells) // len(rows)
        cells = [screen.cells[row * width:(row + 1) * width]
                 for row in range(len(rows))]
        if not last or number == len(names) - 1:
            print(f"== {number}: {step}    cursor at column "
                  f"{screen.cursor[0]}, row {screen.cursor[1]}")
            if changes and before is not None:
                # A row whose colours alone have changed is marked ~.
                changed = [row for row in range(len(rows))
                           if cells[row] != before[1][row]]
                for row in changed:
                    mark = "~" if rows[row] == before[0][row] else " "
                    print(f"{row:2}{mark}{rows[row]}")
                if not changed:
                    print("   no row has changed")
            else:
                print("\n".join(rows))
        before = rows, cells


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("step", nargs="*", help="keys to press")
    parser.add_argument("--country", default="us",
                        choices=sorted(setups.COUNTRIES),
                        help="the country DOS is configured for")
    parser.add_argument("--program", help="the VC.COM or VC.EXE to run "
                                          "(default: the original)")
    parser.add_argument("--signals", action="store_true",
                        help="the program is a build that signals")
    add_arguments(parser)
    args = parser.parse_args()

    program = setups.Program.load(args.program) if args.program \
        else setups.original_vc()
    scenario = scenarios.Scenario("", tuple(args.step), args.country)
    with setups.Benches() as benches:
        run = scenarios.run(scenario, program, benches[args.country],
                            args.signals)
    print_screens(scenario.steps, run.screens, setups.codepage(args.country),
                  args.changes, args.last)


if __name__ == "__main__":
    main()
