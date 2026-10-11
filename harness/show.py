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
"""
import argparse

import scenarios
import setups


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
    args = parser.parse_args()

    program = setups.Program.load(args.program) if args.program \
        else setups.original_vc()
    scenario = scenarios.Scenario("", tuple(args.step), args.country)
    with setups.Benches() as benches:
        run = scenarios.run(scenario, program, benches[args.country],
                            args.signals)
    steps = ("the start",) + scenario.steps
    for number, (step, screen) in enumerate(zip(steps, run.screens)):
        print(f"== {number}: {step}    cursor at column {screen.cursor[0]}, "
              f"row {screen.cursor[1]}")
        for line in screen.lines(setups.codepage(args.country)):
            print(line.rstrip())


if __name__ == "__main__":
    main()
