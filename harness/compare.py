#!/usr/bin/env python3
"""Compare a build of Volkov Commander with the original.

The build is taken through scenarios (see scenarios.py) under FreeDOS in
QEMU and must do what the original did, as the goldens record it (see
goldens.py): show the same text screen after the start and after every
step, cell by cell, character and colour attribute, with the same cursor
and video mode, and leave the same files on its disks.

A build that signals where it is (see "Signals to the tests" in
port/asm/VC.ASM) is waited for exactly. For any other build, such as the
original itself, the harness waits until the screen has stood still, which
takes most of a second for every step.

Scenarios run side by side, each on a machine of its own, and are reported
in their order.

Exits with status 0 if the build is indistinguishable and 1 if it differs.
"""
import argparse
import os
import sys
import time
from concurrent.futures import ThreadPoolExecutor

import goldens
import scenarios
import setups


# How many scenarios run side by side: each has a machine of its own,
# which keeps one processor busy while the program works.
TOGETHER = min(4, os.cpu_count() or 1)


def compare(names, program, signals, trace=False, together=None):
    """Take `program` through the scenarios of `names`, printing a line for
    each and how it departs from the golden.

    `together` scenarios run side by side, each on a machine of its own;
    TOGETHER if it is not given. Returns whether any departed, and the
    runs.
    """
    def one(name):
        scenario = scenarios.SCENARIOS[name]
        began = time.monotonic()
        run = scenarios.run(scenario, program, benches[scenario.country],
                            signals, trace)
        return run, time.monotonic() - began

    failed = False
    runs = []
    with setups.Benches() as benches, \
            ThreadPoolExecutor(together or TOGETHER) as pool:
        # In the order of `names`, as each is done.
        for name, (run, took) in zip(names, pool.map(one, names)):
            runs.append(run)
            report = goldens.differences(goldens.read(name), run,
                                         {program.filename})
            screens = len(run.screens)
            if report:
                failed = True
                print(f"{name}: differs from the original")
                print("\n".join(report))
            else:
                print(f"{name}: as the original, {screens} screen"
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
    parser.add_argument("--signals", action="store_true",
                        help="the program is a build that signals")
    parser.add_argument("--together", type=int, metavar="N",
                        help="run this many scenarios side by side "
                             f"(default: {TOGETHER} here)")
    args = parser.parse_args()
    failed, _ = compare(args.scenario or list(scenarios.SCENARIOS),
                        setups.Program.load(args.program), args.signals,
                        together=args.together)
    sys.exit(failed)


if __name__ == "__main__":
    main()
