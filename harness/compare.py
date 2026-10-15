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
show otherwise is held against the original's golden with that
difference made to it, by the scenario's patch (see goldens.py), and one
that only shows what the tree has no more is not run. A patch that does
not fit the golden it is of fails the comparison before anything is run.

The build is run as it is, and so is the original: a driver in the
machine presses the keys and writes each screen out when the program has
finished the step (see driver.py). The driver finds what it watches of a
build in the listing that the build writes next to it, and of the C
translation, whose main data is laid out as the original's, in the
original's.

Scenarios run side by side, each on a machine of its own, and are reported
in their order.

A scenario that the machine did not get through is reported as stopped,
with the step that was under way and what the screen showed: the program
did not come to wait for a key within the time that a step has, or the
machine was reset. The others are run all the same.

Exits with status 0 if the build is indistinguishable and 1 if it differs.
"""
import argparse
import json
import os
import re
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path

import dataimage
import goldens
import scenarios
import setups
from dos import MachineError


# How many scenarios run side by side: each has a machine of its own,
# which keeps one processor busy while the program works.
TOGETHER = min(4, os.cpu_count() or 1)


@dataclass
class Outcome:
    """What became of a program in a scenario."""
    name: str
    # What it showed and left, a scenarios.Run; None if the machine did
    # not get through the scenario.
    run: object = None
    # Why it did not, then: what MachineError says.
    stopped: str = None
    # How the run departs from what it is held against, as
    # goldens.departures() has it: empty if it does not.
    departs: tuple = ()

    @property
    def passed(self):
        return self.run is not None and not self.departs

    @property
    def first(self):
        """Where the scenario first went otherwise, and how, as text
        lines: the step, with the first row of its screen that differs,
        or what else does; or the step at which it stopped, with why
        and the screen as it stood. The place is None where there is
        none to name, for the disks. None for a scenario that passed."""
        if self.stopped:
            # As scenarios.run() words it. A step may have a colon.
            found = re.match(r"(after .*?): (the (?:program did not come|"
                             r"machine stopped)\b.*)", self.stopped, re.S)
            where, rest = found.groups() if found else (None, self.stopped)
            lines = [line.rstrip() for line in rest.split("\n")]
            while len(lines) > 1 and not lines[-1]:
                del lines[-1]
            return where, lines
        if not self.departs:
            return None
        where, lines = self.departs[0]
        return where, lines[:3 if lines[0].startswith("row ") else 1]


def compare(names, program, trace=False, together=None, stack=False,
            limits=None, first=False, images=None):
    """Take `program` through the scenarios of `names`, printing a line for
    each and how it departs from the golden.

    `together` scenarios run side by side, each on a machine of its own;
    TOGETHER if it is not given. `trace` and `stack` are as for
    scenarios.run(). `limits` has the seconds that a step may take in a
    scenario, name -> seconds, where that is not scenarios.STEP_TIME.
    With `first`, only where a scenario first departs is wanted: it is
    ended at the first screen that does, and that screen's first row is
    printed, not every row of every screen. That is for a program of
    which many scenarios are expected to fail: what the keys of a
    scenario do once the program is somewhere else says nothing of it.
    And a scenario is not run then, and reported as stopped, once the
    program has not come to its first screen in another of the same
    country: every scenario starts the program in the same way, on the
    bench of its country, and would wait as long for the same thing.
    With `images`, a directory, the image of the program's data at the
    end of each scenario is written there, for a flat program of 32
    bits: see dataimage.py. A scenario that did not get to its end, or
    that was ended at its first screen that differs, leaves none. And
    a scenario fails if the pattern that the program keeps round a
    block of its memory has been written into by its end: see
    dataimage.GUARD.
    Returns whether any failed, and of each what became of it, an
    Outcome, in the order of `names`.

    What each scenario is held against is read before any is run: a
    patch that cannot be made to its golden wants a person, and nothing
    that a run would say is worth waiting for until one has looked.
    """
    expected, misfits = {}, []
    for name in names:
        try:
            expected[name] = goldens.read(name, program.tree)
        except goldens.Misfit as misfit:
            misfits.append(str(misfit))
    if misfits:
        sys.exit("\n\n".join(misfits))

    taker = None
    if images:
        taker = dataimage.Taker(program)
        dataimage.write_facts(images, taker.facts)

    # With `first`: country -> a scenario of it in which the program
    # did not come to its first screen.
    never = {}

    def one(name):
        scenario = scenarios.of(name, program.tree)
        golden = expected[name]
        if scenario.country in never:
            return Outcome(name, stopped=(
                "after the start: the program did not come to its first "
                f"screen in {never[scenario.country]}, which starts as "
                "this does: not run")), None

        def departs(number, screen):
            return number >= len(golden.screens) \
                or bool(goldens.screen_departs(golden, number, screen))

        began = time.monotonic()
        try:
            run = scenarios.run(
                scenario, program,
                # The program for Linux has no bench of DOS's.
                None if program.tree == "posix"
                else benches[scenario.bench], trace,
                stack=stack,
                limit=(limits or {}).get(name, scenarios.STEP_TIME),
                until=departs if first else None, image=taker)
        except MachineError as error:
            # As scenarios.run() words it.
            if first and str(error).startswith(
                    "after the start: the program did not come "):
                never.setdefault(scenario.country, name)
            return Outcome(name, stopped=str(error)), None
        return Outcome(name, run), time.monotonic() - began

    failed = False
    outcomes = []
    # The scenarios that took longest the last time are started first,
    # so that none of them is left to run by itself at the end; what is
    # printed is in the order of `names` all the same. What each took is
    # of the host, and only orders the work.
    times = _times(program.tree)
    took_now = {}
    with setups.Benches() as benches, \
            ThreadPoolExecutor(together or TOGETHER) as pool:
        started = {name: pool.submit(one, name)
                   for name in sorted(names,
                                      key=lambda name: -times.get(name, 0))}
        # In the order of `names`, as each is done.
        for name in names:
            outcome, took = started[name].result()
            if took is not None:
                took_now[name] = took
            outcomes.append(outcome)
            golden = expected[name]
            # The line says what a scenario was held against: the
            # original's golden, or that without the steps that are
            # left out for the tree and with the scenario's patch.
            measure = goldens.measure(golden)
            if outcome.stopped:
                failed = True
                where, lines = outcome.first
                print(f"{name}: stopped" + (f" {where}" if where else "")
                      + ": " + "\n    ".join(lines))
                continue
            run = outcome.run
            if run.image:
                run.image.write(Path(images)
                                / (name + dataimage.SUFFIX))
                # What the program keeps round each block of its memory
                # was written into: that fails the scenario, whatever
                # its screens show.
                for said in run.image.guards:
                    failed = True
                    print(f"{name}: {said}")
                # It is on the disk, and a test has many.
                run.image = None
            outcome.departs = tuple(goldens.departures(
                golden, run, {program.filename}))
            screens = len(run.screens)
            if outcome.departs and first:
                failed = True
                where, lines = outcome.first
                print(f"{name}: differs from {measure}, first "
                      + (where or "in what it leaves on the disks"))
                print("\n".join("    " + line for line in lines))
            elif outcome.departs:
                failed = True
                print(f"{name}: differs from {measure}")
                print("\n".join(goldens.differences(
                    golden, run, {program.filename})))
            else:
                print(f"{name}: as {measure}, {screens} screen"
                      f"{'s' if screens > 1 else ''} and the disks "
                      f"({took:.1f} s)")
    _times(program.tree, {**times, **took_now})
    return failed, outcomes


# Where the seconds that each scenario took the last time are kept, for
# a tree: in the build's directory, since they are of this host.
TIMES = Path(__file__).resolve().parent.parent / "build" / "times"


def _times(tree, new=None):
    """The seconds that each scenario took when `tree` was last taken
    through it, name -> seconds, as far as they are known; with `new`,
    write those instead."""
    path = TIMES / f"{tree}.json"
    if new is not None:
        try:
            TIMES.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps(new, indent=0, sort_keys=True))
        except OSError:
            pass
        return new
    try:
        return json.loads(path.read_text())
    except (OSError, ValueError):
        return {}


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
    parser.add_argument("--limit", type=float, metavar="SECONDS",
                        default=scenarios.STEP_TIME,
                        help="give a step up after so many seconds "
                             f"(default: {scenarios.STEP_TIME})")
    parser.add_argument("--first", action="store_true",
                        help="end a scenario at the first screen that "
                             "differs, and print the first row of it "
                             "that does")
    args = parser.parse_args()
    program = setups.Program.load(args.program)
    run = scenarios.names(program.tree)
    for name in args.scenario:
        if name not in run:
            parser.error(
                f"{name} is not run for {program.tree}: "
                + scenarios.not_run(program.tree)[name])
    names = args.scenario or run
    failed, _ = compare(names, program, together=args.together,
                        limits=dict.fromkeys(names, args.limit),
                        first=args.first)
    sys.exit(failed)


if __name__ == "__main__":
    main()
