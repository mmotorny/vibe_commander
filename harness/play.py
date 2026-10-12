#!/usr/bin/env python3
"""Run the port, or the original, on the test bench, to use it by hand.

The machine is the one of the tests: FreeDOS booted from its floppy, the
bench's files on C:, the program with what it reads on D:, and the program
set up as the tests have it; only the driver of the tests, which is on the
floppy, is not loaded. Its screen is shown in the terminal, which
has to be 80 columns by 25 lines at least, and its keyboard is the
terminal's. Nothing is kept: the disks are made anew every time.

    play.py                     the port, build/c/VC.EXE
    play.py --original          the original, VC.COM
    play.py --country ru        on the Russian bench
    play.py PROGRAM             another build

F10 leaves the program, and the machine then switches itself off. Should
it not, Alt+2 (or Esc, then 2) brings up QEMU's own console, where `quit`
ends it; Alt+1 goes back to the screen.

Needs `qemu-system-i386` and `mtools` on the PATH, and the project built.
"""
import argparse
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import setups

ROOT = Path(__file__).resolve().parent.parent

# What DOS runs once it has booted: the program, where the tests start it,
# and then what switches the machine off.
COMMANDS = [setups.LOAD_MOUSE, "C:", r"CD \WORK", r"D:\VC\VC",
            r"A:\FREEDOS\BIN\FDAPM POWEROFF"]


def machine(workdir, program, country):
    """The floppy and the hard disks of a bench for `country`, in `workdir`,
    with `program` on drive D:."""
    bench = setups.Bench(workdir / "bench", country)
    floppy = setups._floppy(workdir, COMMANDS, setups.COUNTRIES[country],
                            setups.KEYBOARDS.get(country), bench=country).path
    disks = [workdir / "c.img", workdir / "d.img"]
    shutil.copyfile(bench._work, disks[0])
    shutil.copyfile(bench._program_disk(program), disks[1])
    return floppy, disks


def command(floppy, disks, display):
    """QEMU's command line for the machine, shown on `display`."""
    line = ["qemu-system-i386", "-machine", "pc", "-cpu", "486", "-m", "16",
            "-nodefaults", "-vga", "std", "-display", display, "-no-reboot",
            "-rtc", "base=localtime", "-boot", "a", "-monitor", "vc",
            # The floppy tells its serial port when DOS has booted, for
            # the tests: there has to be one, and nobody listens.
            "-serial", "null",
            "-drive", f"if=floppy,index=0,format=raw,readonly=on,file={floppy}"]
    for index, disk in enumerate(disks):
        line += ["-drive", f"if=ide,index={index},format=raw,file={disk}"]
    return line


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("program", nargs="?", type=Path,
                        default=ROOT / "build" / "c" / "VC.EXE")
    parser.add_argument("--original", action="store_true",
                        help="run the original in place of a build")
    parser.add_argument("--country", choices=sorted(setups.COUNTRIES),
                        default="us")
    parser.add_argument("--display", default="curses",
                        help="QEMU's display: curses, or gtk for a window")
    args = parser.parse_args()
    if args.original:
        program = setups.original_vc()
    elif args.program.exists():
        program = setups.Program.load(args.program)
    else:
        sys.exit(f"{args.program} is not there: build the project first")
    with tempfile.TemporaryDirectory() as directory:
        print("Making the bench...", flush=True)
        floppy, disks = machine(Path(directory), program, args.country)
        subprocess.run(command(floppy, disks, args.display), check=False)


if __name__ == "__main__":
    main()
