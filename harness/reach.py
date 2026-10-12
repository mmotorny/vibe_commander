#!/usr/bin/env python3
"""Say which instructions of some routines no scenario reaches.

Before routines are translated, every instruction of theirs has to run in
the original in some scenario, or be listed as out of reach (see
ported.py). This says of each routine how many of its instructions have
run and which are left, from what was recorded with the goldens, so it
takes no time:

    reach.py F35_40 CurNl Quit

    F35_40: reached 4, left 4: 412C-4137
    CurNl:  reached 7, left 3: 0DA4-0DA8
    Quit:   reached 12, left 0

A routine is a procedure, or code that stands between two procedures,
which goes by its first label: PutInf, between PutTree and PtIn20.

An instruction is named by its address in the listing, and instructions
that follow one another by the first and the last of them. With --list
those that are left are printed as well, one to a line, as the listing
has them:

    reach.py CurNl --list

With steps it also takes the original through them, as show.py does, and
says which of those instructions they reach, for finding the steps of a
new scenario. That takes most of a second for every step.

    reach.py F35_40 --steps "Down Down Down" F6

    F35_40: reached 4, these steps 3 more: 412C-4135, left 1: 4137

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
import watch
import show

LISTING = deps.ROOT / "build" / "shareware-layout" / "VC.LST"
COM_ORIGIN = 0x100                # where a .COM file is loaded


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("routine", nargs="+",
                        help="procedures of the original, by name, or "
                             "code between two by its first label")
    parser.add_argument("--steps", nargs="+", metavar="STEP",
                        help="keys to press, written as in scenarios.py")
    parser.add_argument("--country", default="us",
                        choices=sorted(setups.COUNTRIES),
                        help="the country DOS is configured for")
    parser.add_argument("--screens", action="store_true",
                        help="print the screen after every step")
    show.add_arguments(parser)
    parser.add_argument("--list", action="store_true",
                        help="print every instruction that is left, with "
                             "its text")
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
        original = setups.original_vc()
        with setups.Benches(not watch.available(original)) as benches:
            run = scenarios.run(scenario, original,
                                benches[args.country], signals=False,
                                trace=True)
        tried = set(run.executed)
        if args.screens or args.changes or args.last:
            show.print_screens(scenario.steps, run.screens,
                               setups.codepage(args.country),
                               args.changes, args.last)

    width = max(len(name) for name in args.routine) + 1
    total = missed = still = 0
    for name in args.routine:
        lines = listing.procedure(name)
        # For each instruction, whether no scenario has run it and, if
        # none has, whether the steps reach it.
        missing = [line.address - COM_ORIGIN not in ran for line in lines]
        new = [is_missing and tried is not None
               and line.address - COM_ORIGIN in tried
               for line, is_missing in zip(lines, missing)]
        rest = [is_missing and not is_new
                for is_missing, is_new in zip(missing, new)]
        total += len(lines)
        missed += sum(missing)
        still += sum(rest)
        report = f"{name + ':':{width}} reached {len(lines) - sum(missing)}"
        if tried is not None:
            report += f", these steps {sum(new)} more{runs(lines, new)}"
        print(f"{report}, left {sum(rest)}{runs(lines, rest)}")
        if args.list:
            for line, is_missing, is_new in zip(lines, missing, new):
                if is_missing:
                    note = ""
                    if tried is not None:
                        note = "these steps reach it  " if is_new \
                            else " " * 22
                    print(f"  {note}{line.address:04X} "
                          f"{line.text.expandtabs(8)}")
    print(f"{total - missed} of {total} instructions ran in some scenario, "
          f"{missed} did not")
    if tried is not None:
        print(f"these steps reach {missed - still} of them and leave "
              f"{still}")


def runs(lines, chosen):
    """The addresses of those of `lines`, the instructions of a routine,
    that `chosen` has true: ": C566 C570-C579", instructions that follow
    one another as the first and the last. Nothing if there is none."""
    found = []
    for line, is_chosen, before in zip(lines, chosen, [False] + chosen):
        if is_chosen and before:
            found[-1][1] = line.address
        elif is_chosen:
            found.append([line.address, line.address])
    return "".join(
        (": " if number == 0 else " ")
        + (f"{first:04X}" if first == last else f"{first:04X}-{last:04X}")
        for number, (first, last) in enumerate(found))


if __name__ == "__main__":
    main()
