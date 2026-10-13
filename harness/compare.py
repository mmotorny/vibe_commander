#!/usr/bin/env python3
"""Compare a build of Volkov Commander with the original.

The build is taken through scenarios (see scenarios.py) under FreeDOS in
QEMU and must do what the original did, as the goldens record it (see
goldens.py): show the same text screen after the start and after every
step, cell by cell, character and colour attribute, with the same cursor
and video mode, and leave the same files on its disks.

A build of the portable C, build/portable_c/VC.EXE, is held against the
original in the same way but where that tree is declared to behave
otherwise (see deviations.TREES): a scenario that it is declared to
change is held against the tree's own golden of it, and one that only
shows what the tree has no more is not run.

The build is run as it is, and so is the original: a driver in the
machine presses the keys and writes each screen out when the program has
finished the step (see driver.py). The driver finds what it watches of a
build in the listing that the build writes next to it, and of the C
translation, whose main data is laid out as the original's, in the
original's.

Scenarios run side by side, each on a machine of its own, and are reported
in their order.

Exits with status 0 if the build is indistinguishable and 1 if it differs.
"""
import argparse
import os
import sys
import time
from concurrent.futures import ThreadPoolExecutor

import deviations
import goldens
import scenarios
import setups


# How many scenarios run side by side: each has a machine of its own,
# which keeps one processor busy while the program works.
TOGETHER = min(4, os.cpu_count() or 1)


def compare(names, program, trace=False, together=None, stack=False):
    """Take `program` through the scenarios of `names`, printing a line for
    each and how it departs from the golden.

    `together` scenarios run side by side, each on a machine of its own;
    TOGETHER if it is not given. `trace` and `stack` are as for
    scenarios.run(). Returns whether any departed, and the runs.
    """
    def one(name):
        scenario = scenarios.of(name, program.tree)
        began = time.monotonic()
        run = scenarios.run(scenario, program, benches[scenario.country],
                            trace, stack=stack)
        return run, time.monotonic() - began

    failed = False
    runs = []
    with setups.Benches() as benches, \
            ThreadPoolExecutor(together or TOGETHER) as pool:
        # In the order of `names`, as each is done.
        for name, (run, took) in zip(names, pool.map(one, names)):
            runs.append(run)
            golden = goldens.read(name, program.tree)
            report = goldens.differences(golden, run, {program.filename})
            screens = len(run.screens)
            # A golden of the tree's own is no measure of it: the line
            # says which of the two a scenario was held against.
            measure = f"its own golden, recorded from {golden.tree}" \
                if golden.tree else "the original"
            if report:
                failed = True
                print(f"{name}: differs from {measure}")
                print("\n".join(report))
            else:
                print(f"{name}: as {measure}, {screens} screen"
                      f"{'s' if screens > 1 else ''} and the disks "
                      f"({took:.1f} s)")
    return failed, runs


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("program", help="the VC.COM or VC.EXE to test")
    parser.add_argument("scenario", nargs="*",
                        help="scenarios to run (default: all): "
                             + ", ".join(scenarios.SCENARIOS))
    parser.add_argument("--together", type=int, metavar="N",
                        help="run this many scenarios side by side "
                             f"(default: {TOGETHER} here)")
    args = parser.parse_args()
    program = setups.Program.load(args.program)
    run = scenarios.names(program.tree)
    for name in args.scenario:
        if name not in run:
            parser.error(
                f"{name} is not run for {program.tree}: "
                + deviations.declared(program.tree).not_run[name])
    failed, _ = compare(args.scenario or run, program,
                        together=args.together)
    sys.exit(failed)


if __name__ == "__main__":
    main()
