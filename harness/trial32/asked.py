#!/usr/bin/env python3
"""Ask the system's functions of the portable C on the test bench: build
the toy of sys.h, run it there, and print what it writes.

    asked.py [--country ru] [--keep]

The toy is sys.c, which asks every function of the portable C's sys.h
and writes what it is answered, a page for each step of STEPS below.
sys-32.exe is the toy with sys_dos.c, the system's functions of the
program, and DOS/32A. It is run by run.py, as VC.EXE, and what run.py
prints of its screens is printed here, a screen for each step.

Nothing judges what it writes. While the portable C was built for 16
bits as well, this was both.py: it built the toy a second time, with the
dos.c of that program, ran both and said where the two did not write
the same, line for line; they wrote the same on every one of the 28
screens when it was last run, with both files as they were when that
build went. That is how sys_dos.c was written, a function at a time
(TODO.md, "The portable C in 32 bits: the plan", the pieces D1 and
D2). What is left is the toy and its steps: for seeing
what a function answers on the bench, and what a change to sys_dos.c does
to that, by holding what this prints against what it printed before.

It ends with 1 if the toy did not write a screen for every step.
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
OUT = ROOT / "build" / "trial32"

# The steps of the script, one behind each page of the toy: keys with
# and without Shift, Ctrl and Alt, a key of the cursor, a button of the
# mouse. The last is typed at DOS's prompt, when the toy has ended with
# its end left taken: the shell says what DOS has been told of Ctrl-C by
# the routine that DOS goes on in then.
STEPS = ["a", "F5", "Shift+F6", "Alt+F1", "Ctrl+o", "Up",
         "Click@3,5", "Ins", "m", "n", "p", "2", "3", "o",
         "r", "f", "x", "f", "f", "d", "f", "f", "Ctrl+Break", "Ctrl+c",
         "l", "q", '"break" Enter']


def screens(country, keep):
    """The screens of the toy: a list, of each step its lines."""
    subprocess.run([HERE / "build.sh", "sys", "32"], check=True,
                   stdout=subprocess.DEVNULL)
    tmp = ROOT / "build" / "tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, HERE / "run.py", "--country", country]
    if keep:
        command += ["--keep", OUT / "sys-32.out"]
    command += [OUT / "sys-32.exe", *STEPS]
    printed = subprocess.run(command, check=True, text=True,
                             stdout=subprocess.PIPE,
                             env={**os.environ, "TMPDIR": str(tmp)}).stdout
    found = []
    for line in printed.splitlines():
        if line.startswith("== "):
            found.append([])
        elif line.startswith("  |") and found:
            found[-1].append(line[3:])
        elif line.startswith("!!") or not found and "did not end" in line:
            sys.exit(f"asked.py: the toy: {line}")
    if len(found) != len(STEPS) + 1:
        sys.exit(f"asked.py: the toy wrote {len(found)} screens for "
                 f"{len(STEPS) + 1}:\n{printed}")
    return found


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--country", default="us", choices=["us", "ru"])
    parser.add_argument("--keep", action="store_true",
                        help="keep what the driver wrote and the disks, "
                             "as build/trial32/sys-32.out with the images "
                             "next to it")
    args = parser.parse_args()

    for number, lines in enumerate(screens(args.country, args.keep)):
        print(f"== {number}: {(['the start'] + STEPS)[number]}")
        for line in lines:
            print("  |" + line)
    print(f"{len(STEPS) + 1} screens")


if __name__ == "__main__":
    main()
