#!/usr/bin/env python3
"""Hold the cap of the memory to what it is for: a file larger than the
room that is left under it gets the original's refusal.

The piece I1 of step 8 (TODO.md, "Step 8, the app for iOS"). The program
of the machine of `*_posix.c` asks for all the memory that there is for
its panels and its texts, as it asks DOS, and the machine gives it the
host's, or at most a cap where the implementation has one: the web page
256 MB, iOS what the system lets a program have, Linux none; and
VC_MEMORY, which a test sets, in place of that (machine_posix.c). With a
cap the editor has room for less, and a file larger than that room is
refused as the original refuses a file that its room is short of, in the
box of the scenario `editor-big`.

For each program that it is given, the program for Linux, the app's on a
host (the tree "ios-host") or the program for the web under Node, on the
bench of posixrun.py, the editor is asked
for H:\\STAMPS\\noon.dat, 1 MB long:

  - with VC_MEMORY at 512K: the box is held cell for cell, characters and
    colours, to the box of the original's golden of `editor-big` where
    its editor refuses the same file;
  - with no VC_MEMORY: the editor opens it, no box. That is Linux as it
    was, and the caps of the web page and of iOS, with room for it;
  - with a VC_MEMORY that is no number of bytes: the program does not
    start, and says so.

It starts no machine of DOS but where posixrun.py makes the bench's
setup file the first time, which it keeps.
"""

import argparse
import pathlib
import subprocess
import sys

import goldens
import posixrun
import scenarios
import setups

# The steps from the start in H:\WORK: up to H:\, into STAMPS, down onto
# noon.dat, F4, and Cancel. They are those of the end of editor-big.
STEPS = ("Ctrl+PgUp", "Up Enter", " ".join(("Down",) * 8), "F4",
         "Right Enter")
ASKED = 4                     # the screen behind F4, after the start's
GOLDEN_STEP = 39              # editor-big's screen with the same box
# The box, rows and columns, its frame included.
ROWS = range(10, 15)
COLUMNS = range(27, 53)
TOO_LARGE = "is too large for Edit."


def box(screen):
    """The cells of the box, character and colour, row for row."""
    return [bytes(screen.cells[(row * 80 + column) * 2 + i]
                  for column in COLUMNS for i in (0, 1))
            for row in ROWS]


def check(path, golden):
    """What is wrong with the program at `path`, as lines."""
    program = setups.Program.load(path)
    scenario = scenarios.Scenario("the cap", steps=STEPS)
    faults = []

    run = posixrun.run(scenario, program, environment={"VC_MEMORY": "512K"})
    asked = run.screens[ASKED]
    for row, (got, want) in zip(ROWS, zip(box(asked), box(golden))):
        if got != want:
            faults.append(f"with VC_MEMORY=512K, row {row} of the box is "
                          f"{asked.line(row)[COLUMNS.start:COLUMNS.stop]!r}"
                          f", the original's "
                          f"{golden.line(row)[COLUMNS.start:COLUMNS.stop]!r}"
                          ", or their colours differ")
    run = posixrun.run(scenario, program)
    if any(TOO_LARGE in line for line in run.screens[ASKED].lines()):
        faults.append("with no VC_MEMORY, the editor refuses a file of 1 MB")
    try:
        posixrun.run(scenario, program, environment={"VC_MEMORY": "512Q"})
        faults.append("with VC_MEMORY=512Q, the program starts")
    except posixrun.MachineError as error:
        if "VC_MEMORY" not in str(error):
            faults.append(f"with VC_MEMORY=512Q, it ends otherwise: {error}")
    return faults


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("programs", nargs="+", type=pathlib.Path,
                        help="vc of a build for Linux or of ios-host, "
                             "vc.js of one for the web under Node")
    arguments = parser.parse_args()
    golden = goldens.read("editor-big").screens[GOLDEN_STEP]
    if not any(TOO_LARGE in line for line in golden.lines()):
        sys.exit(f"editor-big has no refusal at its step {GOLDEN_STEP}")
    failed = False
    for path in arguments.programs:
        if not path.exists():
            print(f"{path}: not built", file=sys.stderr)
            failed = True
            continue
        try:
            faults = check(path, golden)
        except (posixrun.MachineError, subprocess.SubprocessError) as error:
            faults = [str(error)]
        for fault in faults:
            print(f"{path}: {fault}")
        if not faults:
            print(f"{path}: the cap holds")
        failed = failed or bool(faults)
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
