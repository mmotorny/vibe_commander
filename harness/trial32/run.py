#!/usr/bin/env python3
"""Run toys of the trial on the test bench, as the harness runs a program
there, and print every screen that the driver of the tests writes.

    run.py [OPTION...] FILE [STEP...] [-- FILE [STEP...]]...

Each FILE is put on D:\\VC under the name that --name says, VC.EXE unless
it does, started by the bench's own commands, and taken through its STEPs
by the driver, which are written as in harness/scenarios.py: "a", "F5",
"Alt+F1", '"text"', "Click@3,5", "Wait*18"; and "Ctrl+Break", which the
harness has no name for. A toy that ends leaves DOS's prompt, which idles
as a program does, so the screens of the steps left are written all the
same. Of a toy that hangs, or that the script leaves behind, the screen
it is stuck on is printed when nothing has been written for --limit
seconds.

The harness is used as it is: its bench, its driver, its scripts. The
options of the last group below change the machine or the driver, and
what is seen with them is not seen on the bench.

Needs the project built, and the toy: see build.sh. Run it as the tests
are run, with a short TMPDIR in the checkout's build/:

    mkdir -p build/tmp
    TMPDIR=$PWD/build/tmp python3 harness/trial32/run.py \\
        build/trial32/keys-d32.exe a F5 Alt+F1 q
"""
import argparse
import io
import shutil
import struct
import sys
import tempfile
import time
import zipfile
from dataclasses import replace
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import deps                                            # noqa: E402
import dos                                             # noqa: E402
import driver                                          # noqa: E402
import setups                                          # noqa: E402
from dos import FatImage, MachineError                 # noqa: E402

# Where DRIVERX.COM has what the host writes into it, behind the
# driver's own Pad: LAuto, LClock and Debug (see DRIVERX.ASM).
DRIVERX_SETTINGS = 5
# The record that it writes with Debug set: a letter and eight words.
DEBUG_RECORD = "<8H"
DEBUG_NAMES = ("CS", "IP", "flags", "DS", "ES", "SS", "SP", "MSW")


def without_debug(written):
    """What DRIVERX.COM wrote, without its records D, and those: each as
    its eight words."""
    at, plain, found = 0, bytearray(), []
    while at < len(written):
        kind = written[at:at + 1]
        if kind in (b"S", b"B"):
            size = 1 + driver.SCREEN
        elif kind == b"K":
            size = 5 + int.from_bytes(written[at + 1:at + 3], "little")
        elif kind == b"D":
            found.append(struct.unpack_from(DEBUG_RECORD, written, at + 1))
            at += 1 + struct.calcsize(DEBUG_RECORD)
            continue
        else:
            size = 1
        plain += written[at:at + size]
        at += size
    return bytes(plain), found


def use_driver(path, linear_auto, linear_clock, debug):
    """Have the bench load the driver at `path`, DRIVERX.COM, with its
    settings written into a copy next to it."""
    code = bytearray(Path(path).read_bytes())
    struct.pack_into("<IIB", code, DRIVERX_SETTINGS, linear_auto,
                     linear_clock, debug)
    copy = Path(path).resolve().with_suffix(".SET")
    copy.write_bytes(code)
    driver.PROGRAM = copy
    # The bench reads what the driver wrote when it sets itself up.
    plain = driver.read
    driver.read = lambda written: plain(without_debug(written)[0])


def other_machine(cpu, memory, more):
    """Have every machine be another than the bench's: its processor,
    its memory, or with more options for QEMU."""
    run = dos.subprocess.Popen

    def popen(command, *args, **keywords):
        if command and command[0] == "qemu-system-i386":
            command = list(command)
            if cpu:
                command[command.index("-cpu") + 1] = cpu
            if memory:
                command[command.index("-m") + 1] = memory
            command += [word for option in more for word in option.split()]
        return run(command, *args, **keywords)

    dos.subprocess.Popen = popen


def with_xms():
    """Have DOS load an XMS driver, FreeDOS's HimemX, which is a package
    of the image that the harness has fetched for the keyboard and the
    mouse."""
    make = setups.boot_floppy

    def boot_floppy(path, workdir, freedos_zip, epoch, commands,
                    country=None):
        floppy = make(path, workdir, freedos_zip, epoch, commands, country)
        image = Path(workdir) / "himem.img"
        with zipfile.ZipFile(deps.fetch(deps.FREEDOS_LITE)) as archive:
            image.write_bytes(archive.read("FD14LITE.img"))
        lite = FatImage(image, workdir, epoch, partitioned=True)
        packed = io.BytesIO(lite.read("packages/base/himemx.zip"))
        image.unlink()
        with zipfile.ZipFile(packed) as archive:
            floppy.write("FREEDOS/BIN/HIMEMX.EXE",
                         archive.read("BIN/HimemX.exe"), epoch)
        floppy.write("FDCONFIG.SYS",
                     b"DEVICE=\\FREEDOS\\BIN\\HIMEMX.EXE\r\n"
                     + floppy.read("FDCONFIG.SYS"), epoch)
        return floppy

    setups.boot_floppy = boot_floppy


def show(screen, rows):
    for line in screen.lines()[:rows]:
        if line.strip():
            print("  |" + line.rstrip())


def run(bench, args, path, steps):
    program = setups.Program(args.name.upper(), Path(path).read_bytes())
    disk = bench._program_disk(program)
    for extra in args.extra:
        name, _, source = extra.partition("=")
        FatImage(disk, bench._workdir, setups.EPOCH, partitioned=True).write(
            name, Path(source).read_bytes(), setups.EPOCH)
    name = Path(args.name).stem.upper().encode("ascii")
    known = driver.Facts(name[:driver.NAME].ljust(driver.NAME, b"\0"),
                         args.auto, args.clock)
    if args.stack:
        at, size = (int(number, 0) for number in args.stack.split(","))
        known = replace(known, stack=at, stack_size=size)
    script = driver.script(tuple(steps), known,
                           blinking=args.clock != driver.NONE)
    with tempfile.TemporaryDirectory() as workdir:
        began = time.monotonic()
        with bench.start(Path(workdir), program, script,
                         args.trace) as machine:
            machine.resume()
            stuck = None
            try:
                machine.wait_for_end(args.limit)
            except MachineError as error:
                stuck = machine.screen()
                print(f"!! {path}: {error}")
            written, debug = without_debug(machine.written())
            done, ended = driver.read(written)
        took = time.monotonic() - began
        if args.keep:
            Path(args.keep).write_bytes(written)
            for letter, image in machine.disks.items():
                shutil.copyfile(image, f"{args.keep}.{letter}.img")
    print(f"#### {path}: {len(done)} screens for {len(steps) + 1} steps, "
          f"the script {'ended' if ended else 'did not end'}, "
          f"{took:.1f} s")
    for record in dict.fromkeys(debug):
        print(f"   INT 28h of the program, {debug.count(record)} times: "
              + " ".join(f"{name} {word:04X}"
                         for name, word in zip(DEBUG_NAMES, record)))
    for number, step in enumerate(done):
        line = f"== {number}: {(['the start'] + steps)[number]}"
        if step.stack is not None:
            zeros = len(step.stack) - len(step.stack.lstrip(b"\0"))
            line += (f"   of the {len(step.stack)} bytes at "
                     f"{step.stack_segment:04X}:{known.stack:04X}, the "
                     f"first {zeros} are 0")
        print(line)
        show(step.screen, args.rows)
    if stuck:
        print("== stuck on")
        show(stuck, args.rows)


def number(text):
    return int(text, 0)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--name", default="VC.EXE",
                        help="the file's name on the bench: VC.COM for a "
                             "program of real mode")
    parser.add_argument("--country", default="us",
                        choices=sorted(setups.COUNTRIES))
    parser.add_argument("--limit", type=float, default=30,
                        help="seconds with nothing written before a toy "
                             "counts as stuck (default: 30)")
    parser.add_argument("--rows", type=int, default=25,
                        help="print so many rows of a screen")
    parser.add_argument("--extra", action="append", default=[],
                        metavar="VC/NAME=FILE",
                        help="another file for drive D:")
    parser.add_argument("--auto", type=number, default=driver.NONE,
                        help="where the driver looks for AutoTim in the "
                             "segment that DS is on (default: nowhere)")
    parser.add_argument("--clock", type=number, default=driver.NONE,
                        help="the same for Clock; given at all, the driver "
                             "watches what blinks")
    parser.add_argument("--stack", metavar="AT,SIZE",
                        help="have the driver write out so many bytes from "
                             "there in the segment that SS is on at the "
                             "program's idle call")
    parser.add_argument("--trace", help="QEMU's log of the code it ran, "
                                        "as coverage.py reads it")
    parser.add_argument("--keep", metavar="FILE",
                        help="keep what the driver wrote there, and the "
                             "disks next to it as FILE.C.img and FILE.D.img")
    other = parser.add_argument_group("not the bench as it is")
    other.add_argument("--driver", help="DRIVERX.COM, in place of the "
                                        "driver of the tests")
    other.add_argument("--lauto", type=number, default=0,
                       help="with it: AutoTim's linear address")
    other.add_argument("--lclock", type=number, default=0,
                       help="with it: Clock's linear address")
    other.add_argument("--debug", action="store_true",
                       help="with it: say where the program's idle calls "
                            "come from")
    other.add_argument("--cpu", help="another processor than the 486")
    other.add_argument("--mem", help="other memory than 16 MB")
    other.add_argument("--qemu", action="append", default=[],
                       metavar="OPTIONS", help="more options for QEMU, "
                       'as "-d int -D FILE" for its log of interrupts')
    other.add_argument("--himem", action="store_true",
                       help="with an XMS driver loaded")
    parser.add_argument("rest", nargs=argparse.REMAINDER,
                        metavar="FILE [STEP...] [-- FILE [STEP...]]...")
    args = parser.parse_args()
    runs = [[]]
    for word in args.rest:
        if word == "--":
            runs.append([])
        else:
            runs[-1].append(word)
    if not all(runs):
        parser.error("no file to run")

    if args.driver:
        use_driver(args.driver, args.lauto, args.lclock, args.debug)
    if args.cpu or args.mem or args.qemu:
        other_machine(args.cpu, args.mem, args.qemu)
    if args.himem:
        with_xms()
    # Ctrl+Break: the key Pause, which sends E0 46 with Ctrl down.
    driver.KEYS["Break"] = "break"
    driver.EXTENDED["break"] = 0x46

    with setups.Benches() as benches:
        bench = benches[args.country]
        for path, *steps in runs:
            run(bench, args, path, steps)


if __name__ == "__main__":
    main()
