#!/usr/bin/env python3
"""Say which instructions of some routines no scenario reaches.

Before routines are translated, every instruction of theirs has to run in
the original in some scenario, or be listed as out of reach (see
ported.py). This lists the instructions that have not run, from what was
recorded with the goldens, so it takes no time:

    reach.py NgLine LnStr CntLine

With steps it also takes the original through them, as show.py does, and
says which of those instructions they reach, for finding the steps of a
new scenario. That takes most of a second for every step.

    reach.py NgLine --steps F1 "Down Enter" PgDn
    reach.py NgLine --steps F1 --screens        and print the screens
    reach.py NgLine --steps F1 --changes        or of each only the rows
                                                that changed (see show.py)

The routines are found in the listing of the build that is laid out like
the original, build/shareware-layout/VC.LST, which has to be built first.
"""
import argparse
import sys

import coverage
import deps
import goldens
import scenarios
import setups
import show

LISTING = deps.ROOT / "build" / "shareware-layout" / "VC.LST"
COM_ORIGIN = 0x100                # where a .COM file is loaded


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("routine", nargs="+",
                        help="procedures of the original, by name")
    parser.add_argument("--steps", nargs="+", metavar="STEP",
                        help="keys to press, written as in scenarios.py")
    parser.add_argument("--country", default="us",
                        choices=sorted(setups.COUNTRIES),
                        help="the country DOS is configured for")
    parser.add_argument("--screens", action="store_true",
                        help="print the screen after every step")
    show.add_arguments(parser)
    args = parser.parse_args()

    listing = coverage.Listing(LISTING)
    for name in args.routine:
        if name not in listing.procedures:
            sys.exit(f"no procedure {name} in {LISTING}")
    ran = set()
    for name in scenarios.SCENARIOS:
        if goldens.derived(name):
            ran |= goldens.read_ran(name)
        else:
            print(f"{name} has no golden yet and is left out",
                  file=sys.stderr)

    tried = None
    if args.steps:
        scenario = scenarios.Scenario("", tuple(args.steps), args.country)
        with setups.Benches() as benches:
            run = scenarios.run(scenario, setups.original_vc(),
                                benches[args.country], signals=False,
                                trace=True)
        tried = set(run.executed)
        if args.screens or args.changes or args.last:
            show.print_screens(scenario.steps, run.screens,
                               setups.codepage(args.country),
                               args.changes, args.last)

    lines = [line for name in args.routine
             for line in listing.procedure(name)]
    missed = [line for line in lines if line.address - COM_ORIGIN not in ran]
    for line in missed:
        note = ""
        if tried is not None:
            reached = line.address - COM_ORIGIN in tried
            note = "these steps reach it  " if reached else " " * 22
        print(f"{note}{line.address:04X} {line.text.expandtabs(8)}")
    print(f"{len(lines) - len(missed)} of {len(lines)} instructions ran in "
          f"some scenario, {len(missed)} did not")
    if tried is not None:
        still = sum(line.address - COM_ORIGIN not in tried for line in missed)
        print(f"these steps reach {len(missed) - still} of them and leave "
              f"{still}")


if __name__ == "__main__":
    main()
