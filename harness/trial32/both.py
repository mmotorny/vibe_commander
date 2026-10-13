#!/usr/bin/env python3
"""Hold dos32.c against dos.c: build the toy of sys.h both ways, run both
on the test bench, and say where they do not write the same.

    both.py [--country ru] [--show] [--keep]

The toy is sys.c, which asks every function of the portable C's sys.h
and writes what it is answered, a page for each step of STEPS below.
sys-16.exe is the toy with dos.c as the project's build compiled it for
the program of 16 bits, sys-32.exe the toy with dos32.c and DOS/32A.
Each is run by run.py, on a bench of its own, as VC.EXE; what run.py
prints of their screens is held one against the other, line for line.

A line that differs is printed as both have it, under the step whose
screen it is of. Two kinds of difference do not fail:

  - On the page that begins with NOT_MADE, the functions that dos32.c
    does not make yet.
  - A line that EXPECTED names: where DOS/32A answers otherwise than
    DOS and dos32.c does not hide it. Each entry says what differs and
    why, and is a difference in the program too, for the pieces of the
    plan that follow (TODO.md, "The portable C in 32 bits").

Both kinds are counted and, with --show, printed.

Anything else is a mistake of dos32.c, or a finding. It ends with 1 if
there is any, and also when an entry of EXPECTED no longer differs, so
that the table is what is so.

It needs the project built, for the compilers and for dos.obj, and
qemu-system-i386 and mtools as the tests do. It takes ten seconds.
"""
import argparse
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
OUT = ROOT / "build" / "trial32"

# The steps of the script, one behind each page of the toy but the last:
# keys with and without Shift, Ctrl and Alt, a key of the cursor, a
# button of the mouse.
STEPS = ["a", "F5", "Shift+F6", "Alt+F1", "Ctrl+o", "Up", "Tab",
         "Click@3,5", "Ins", "q"]

# The first line, behind the one that says what turned the page, of the
# page of the functions that dos32.c does not make yet.
NOT_MADE = "Not made in dos32.c yet:"

# Where the two are known to differ, and why: the text that the line of
# the 16-bit toy begins with or holds, and the reason.
KEPT = ("DOS/32A brings back the AX that it was called with where the "
        "call did not fail, and not what DOS left there: ")
EXPECTED = {
    "FileSetAttributes 1: 0001/0": KEPT + "4301h, where DOS leaves CX",
    "FileSetAttributes 0: 0000/0": KEPT + "4301h, where DOS leaves CX",
    "DirCurrent 0: 0100/0": KEPT + "47h, where DOS leaves 0100h",
    "DirCurrentBare 0303h: 0100/0": KEPT + "47h, where DOS leaves 0100h",
    "DirCurrent 0100/0": KEPT + "47h, where DOS leaves 0100h",
    "FindFirst 0000/0": KEPT + "4Eh and 4Fh, where DOS leaves 0",
    "FindFirst of another: 0000/0": KEPT + "4Eh, where DOS leaves 0",
    "t back: 0000/0": KEPT + "4Fh, where DOS leaves 0",
    "FindNext with the record moved: 0000/0":
        KEPT + "4Fh, where DOS leaves 0",
}


def screens(kind, country, keep):
    """The screens of the toy built as `kind`, 16 or 32: a list, of each
    step its lines."""
    subprocess.run([HERE / "build.sh", "sys", kind], check=True,
                   stdout=subprocess.DEVNULL)
    tmp = ROOT / "build" / "tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, HERE / "run.py", "--country", country]
    if keep:
        command += ["--keep", OUT / f"sys-{kind}.out"]
    command += [OUT / f"sys-{kind}.exe", *STEPS]
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
            sys.exit(f"both.py: the toy of {kind} bits: {line}")
    if len(found) != len(STEPS) + 1:
        sys.exit(f"both.py: the toy of {kind} bits wrote {len(found)} "
                 f"screens for {len(STEPS) + 1}:\n{printed}")
    return found


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--country", default="us", choices=["us", "ru"])
    parser.add_argument("--show", action="store_true",
                        help="print every screen of the 16-bit toy, and "
                             "also what differs as expected and on the "
                             "page of what is not made yet")
    parser.add_argument("--keep", action="store_true",
                        help="keep what the driver wrote and the disks, "
                             "as build/trial32/sys-16.out and sys-32.out "
                             "with the images next to them")
    args = parser.parse_args()

    old = screens("16", args.country, args.keep)
    new = screens("32", args.country, args.keep)
    wrong = later = 0
    met = set()
    for number, (was, now) in enumerate(zip(old, new)):
        step = (["the start"] + STEPS)[number]
        if args.show:
            print(f"== {number}: {step}")
            for line in was:
                print("  |" + line)
        not_made = len(was) > 1 and was[1] == NOT_MADE
        rows = max(len(was), len(now))
        for row in range(rows):
            first = was[row] if row < len(was) else ""
            second = now[row] if row < len(now) else ""
            if first == second:
                continue
            known = [text for text in EXPECTED if text in first]
            if not_made and row > 0:
                later += 1
                if not args.show:
                    continue
                kind = "not made yet"
            elif known:
                met.update(known)
                if not args.show:
                    continue
                kind = "expected: " + EXPECTED[known[0]]
            else:
                wrong += 1
                kind = "DIFFERENT"
            print(f"{kind}, in the screen of step {number}, {step}:")
            print(f"    16: {first}")
            print(f"    32: {second}")
    for text in EXPECTED:
        if text not in met:
            wrong += 1
            print(f"expected to differ, and does not: {text!r}")
    print(f"{len(STEPS) + 1} screens; {wrong} lines differ that should "
          f"not, {len(met)} of {len(EXPECTED)} differ as expected, and "
          f"{later} are of what dos32.c does not make yet")
    sys.exit(1 if wrong else 0)


if __name__ == "__main__":
    main()
