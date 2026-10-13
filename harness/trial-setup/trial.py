#!/usr/bin/env python3
"""The original setup program, VCSETUP.COM, taken through steps on the
bench of the tests by the harness as it is: the trial that the plan of
its translation rests on (TODO.md, "The setup program in the portable C:
the plan"). What the harness has to do otherwise for it is overridden
here and nowhere else, and is what piece 0a of that plan puts into the
harness itself:

  - Bench, below: the distribution's VCSETUP.COM next to the program
    under test, in D:\\VC.
  - executed(), below: what ran of a program that runs next to another
    with routines of the same bytes.
  - run(..., watch_setup=True): the driver told of the setup program, by
    its name and where it keeps Clock, for a step that shows its clock.

The driver, DRIVER.ASM, is not changed: the setup program says that it
is idle as the main program does, INT 28h, and is fed and written out
like it.

    trial.py show STEP...           the screens after each step
    trial.py record NAME STEP...    a golden and what ran of the setup
                                    program, into
                                    build/trial-setup/goldens/NAME.*

The steps are those of harness/scenarios.py, and the program that they
are pressed on is the original VC.COM, started in C:\\WORK:

    trial.py show '"d:\\vc\\vcsetup>nul" Enter' F2 Esc F10

starts the setup program from its command line, with the banner that it
prints sent nowhere, and

    trial.py show F10 Enter '"d:\\vc\\vcsetup" Enter'

from DOS, with no copy of the main program in the memory.
"""
import argparse
import re
import sys
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(ROOT / "harness"))

import coverage                 # noqa: E402
import deviations               # noqa: E402
import dos                      # noqa: E402
import driver                   # noqa: E402
import fat                      # noqa: E402
import goldens                  # noqa: E402
import scenarios                # noqa: E402
import setups                   # noqa: E402
import show                     # noqa: E402
from dos import MachineError    # noqa: E402

SETUP_NAME = "VCSETUP.COM"
SETUP_LISTING = ROOT / "build" / "shareware-layout" / "VCSETUP.LST"
GOLDENS = ROOT / "build" / "trial-setup" / "goldens"


class Bench(setups.Bench):
    """The bench with the original's setup program next to the program
    under test, in D:\\VC: the one change to what a machine holds."""

    def _new_disk(self, filename, program, setup):
        path = super()._new_disk(filename, program, setup)
        if program:
            data, date = setups.distribution()[SETUP_NAME]
            disk = setups.FatImage(path, self._workdir, setups.EPOCH,
                                   partitioned=True)
            disk.write(f"VC/{SETUP_NAME}", data, date)
        return path


def executed(trace_path, contents, others=()):
    """coverage.executed() for a program that runs next to another which
    has routines of the same bytes, as the setup program and the main
    program have: a place in the memory counts as holding `contents`
    only by blocks that none of `others`, the files of the other
    programs, has too. As coverage.executed() stands, code of the main
    program that ran is taken for the setup program's, at a place where
    that never was: phantom.py shows it. The one change is the line
    marked."""
    logged = list(coverage.blocks(trace_path))
    votes = Counter()
    runs = []
    for address, code in sorted({instruction for block in logged
                                 for instruction in block}):
        if runs and runs[-1][-1][0] + len(runs[-1][-1][1]) == address:
            runs[-1].append((address, code))
        else:
            runs.append([(address, code)])
    for block in logged + runs:
        code = b"".join(code for _, code in block)
        if len(code) < coverage.MIN_BLOCK:
            continue
        if any(code in other for other in others):      # the change
            continue
        offset = contents.find(code)
        if offset >= 0 and contents.find(code, offset + 1) < 0:
            votes[block[0][0] - offset] += len(code) // coverage.MIN_BLOCK
    bases = [base for base, count in votes.most_common()
             if count >= coverage.MIN_VOTES]
    result = {}
    for block in logged:
        for base in bases:
            if all(coverage._matches(contents, address - base, code)
                   for address, code in block):
                for address, code in block:
                    result[address - base] = len(code)
                break
    return result


def setup_facts():
    """What the driver has to know of the setup program to watch its
    clock: its name, and where Clock is in its one segment. The listing
    has a variable by its place in the segment of what has no value,
    _BSS, and says where that begins in the program. The setup program
    has an AutoTim and never sets it."""
    text = SETUP_LISTING.read_text(encoding="cp866")
    bss = int(re.search(r"^_BSS +[0-9A-F]+ +([0-9A-F]+) ", text,
                        re.M)[1], 16)
    clock = int(re.search(r"^Clock [ .]*Byte +([0-9A-F]+)h +_BSS", text,
                          re.M)[1], 16)
    return driver.Facts(b"VCSETUP\0", driver.NONE, bss + clock)


def run(steps, country="us", trace=False, limit=30, watch_setup=False):
    """Take the original VC.COM through `steps` on the bench that has the
    setup program too. Returns the scenarios.Run; what ran of the setup
    program, if `trace`; and how many bytes of it coverage.executed()
    says ran, as it stands."""
    program = setups.original_vc()
    known = setup_facts() if watch_setup else driver.facts(program)
    script = driver.script(steps, known, blinking=watch_setup)
    with dos.workdir() as top:
        bench = Bench(Path(top) / "bench", country)
        workdir = Path(top) / "run"
        workdir.mkdir()
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, program, script, log) as machine:
            machine.resume()
            try:
                machine.wait_for_end(limit)
            except MachineError:
                done = len(driver.read(machine.written())[0])
                shown = "\n".join(machine.screen().lines())
                sys.exit(f"stuck after {done} screens; it shows\n{shown}")
            done, ended = driver.read(machine.written())
            disks = machine.disks
        if not ended or len(done) != len(steps) + 1:
            sys.exit(f"the driver wrote {len(done)} screens for "
                     f"{len(steps) + 1}, and the script "
                     f"{'ended' if ended else 'did not end'}")
        result = scenarios.Run(
            [step.screen for step in done],
            {drive: fat.read_tree(path, setups.codepage(country))
             for drive, path in disks.items()})
        ran, as_it_stands = {}, 0
        if trace:
            setup = setups.distribution()[SETUP_NAME][0]
            ran = executed(log, setup, (program.contents,))
            as_it_stands = sum(coverage.executed(log, setup).values())
            result.executed = coverage.executed(log, program.contents)
    return result, ran, as_it_stands


def write_golden(name, steps, result, ran, country="us"):
    """As goldens.write and goldens.write_ran, into GOLDENS."""
    GOLDENS.mkdir(parents=True, exist_ok=True)
    kept = goldens.DIRECTORY
    goldens.DIRECTORY = GOLDENS
    scenarios.SCENARIOS[name] = scenarios.Scenario("a trial", tuple(steps),
                                                   country)
    try:
        goldens.write(goldens.Golden(
            name, setups.codepage(country), [],
            [deviations.compared(deviations.expected(screen))
             for screen in result.screens],
            goldens.disk_lines(result.disks, {"VC.COM"})))
        goldens.write_ran(name, ran)
    finally:
        goldens.DIRECTORY = kept
        del scenarios.SCENARIOS[name]


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("command", choices=["show", "record"])
    parser.add_argument("rest", nargs="*", metavar="STEP",
                        help="for record, the name first")
    parser.add_argument("--changes", action="store_true",
                        help="as show.py has it")
    parser.add_argument("--last", action="store_true",
                        help="as show.py has it")
    parser.add_argument("--watch-setup", action="store_true",
                        help="tell the driver of the setup program, not "
                             "of the main one")
    parser.add_argument("--limit", type=float, default=30,
                        help="seconds that a step may take")
    args = parser.parse_args()
    if args.command == "show":
        result, _, _ = run(tuple(args.rest), limit=args.limit,
                           watch_setup=args.watch_setup)
        show.print_screens(args.rest, result.screens, 437, args.changes,
                           args.last)
    else:
        name, *steps = args.rest
        result, ran, _ = run(tuple(steps), trace=True, limit=args.limit,
                             watch_setup=args.watch_setup)
        write_golden(name, steps, result, ran)
        print(f"{name}: {len(result.screens)} screens, "
              f"{sum(ran.values())} bytes of {SETUP_NAME} ran")


if __name__ == "__main__":
    main()
