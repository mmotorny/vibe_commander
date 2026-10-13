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
screen it is of. One kind of difference does not fail: a line that
EXPECTED names, where DOS/32A answers otherwise than DOS and dos32.c
does not hide it. Each entry says what differs and why, and is a
difference in the program too, for the pieces of the plan that follow
(TODO.md, "The portable C in 32 bits"). They are counted and, with
--show, printed.

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

# The steps of the script, one behind each page of the toy: keys with
# and without Shift, Ctrl and Alt, a key of the cursor, a button of the
# mouse. The last is typed at DOS's prompt, when the toy has ended with
# its end left taken: the shell says what DOS has been told of Ctrl-C by
# the routine that DOS goes on in then.
STEPS = ["a", "F5", "Shift+F6", "Alt+F1", "Ctrl+o", "Up", "Tab",
         "Click@3,5", "Ins", "m", "n", "p", "2", "3", "o",
         "r", "f", "x", "f", "f", "d", "f", "f", "Ctrl+Break", "Ctrl+c",
         "l", "q", '"break" Enter']

# Where the two are known to differ, and why: the text that the line of
# the 16-bit toy begins with or holds, and the reason.
EXPECTED = {
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
                             "also what differs as expected")
    parser.add_argument("--keep", action="store_true",
                        help="keep what the driver wrote and the disks, "
                             "as build/trial32/sys-16.out and sys-32.out "
                             "with the images next to them")
    args = parser.parse_args()

    old = screens("16", args.country, args.keep)
    new = screens("32", args.country, args.keep)
    wrong = 0
    met = set()
    for number, (was, now) in enumerate(zip(old, new)):
        step = (["the start"] + STEPS)[number]
        if args.show:
            print(f"== {number}: {step}")
            for line in was:
                print("  |" + line)
        rows = max(len(was), len(now))
        for row in range(rows):
            first = was[row] if row < len(was) else ""
            second = now[row] if row < len(now) else ""
            if first == second:
                continue
            known = [text for text in EXPECTED if text in first]
            if known:
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
          f"not, and {len(met)} of {len(EXPECTED)} differ as expected")
    sys.exit(1 if wrong else 0)


if __name__ == "__main__":
    main()
