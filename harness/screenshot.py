#!/usr/bin/env python3
"""Run Volkov Commander under FreeDOS and capture its screen.

By itself, the original Volkov Commander 4.05: the shareware distribution
is downloaded from its author's site, unpacked onto a fresh hard disk as
C:\\VC with its original file dates, and started from there in QEMU, and
the screen is taken once it stands still.

With --program, a build, such as the portable C's build/portable_c/VC.EXE,
is started on the test bench instead (see setups.Bench), in D:\\VC with
the files that it reads there and nothing else, and taken through the
steps that follow, as a scenario is (see driver.py for how a step is
written): the screen is taken when the program waits for a key behind
the last of them. The bench has a mouse, and the program shows its
pointer, in the middle of the screen until a step such as Move@4,7 moves
it. Of the bench's setup file,
which has the clock off and files in the right panel, "F9 o l" switches
the clock on and Ctrl+l makes the right panel the Info panel.

Written to the output directory:

  screen.png    the display as the video card renders it
  screen.bin    the 80x25 text screen from video memory, 4000 bytes
                (character, attribute for each cell)
  screen.txt    the characters of screen.bin as UTF-8 text
  screen.json   video mode, cursor position and shape, blinking cells
"""
import argparse
import json
import sys
from pathlib import Path

import deps
import dos
import scenarios
import setups


def capture(out, scale):
    """The original, as it starts up from its distribution."""
    with dos.workdir() as workdir:
        with setups.fresh_install(Path(workdir)) as machine:
            machine.wait_for_boot()
            screen = machine.wait_for_screen()
            picture = machine.screenshot()
    return write(out, scale, screen, picture)


def capture_on_bench(out, scale, program, steps):
    """`program`, a setups.Program, on the test bench, after `steps`."""
    scenario = scenarios.Scenario("", tuple(steps))
    with setups.Benches() as benches:
        try:
            made = scenarios.run(
                scenario, program, benches[scenario.bench],
                image=lambda machine: (machine.screen(),
                                       machine.screenshot()))
        except dos.MachineError as error:
            sys.exit(str(error))
    if made.image is None:
        sys.exit("the program did not get through the steps")
    return write(out, scale, *made.image)


def write(out, scale, screen, picture):
    out.mkdir(parents=True, exist_ok=True)
    (out / "screen.png").write_bytes(picture.scaled(*scale).to_png())
    (out / "screen.bin").write_bytes(screen.cells)
    (out / "screen.txt").write_text("\n".join(screen.lines()) + "\n")
    (out / "screen.json").write_text(
        json.dumps(screen.describe(), indent=2) + "\n")
    return screen


def parse_scale(text):
    x, _, y = text.partition("x")
    return int(x), int(y or x)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("step", nargs="*",
                        help="with --program, the steps to take it through")
    parser.add_argument("--out", type=Path, default=deps.ROOT / "out",
                        help="output directory (default: out/)")
    parser.add_argument("--scale", type=parse_scale, default=(1, 1),
                        metavar="XxY",
                        help="enlarge screen.png by whole factors, e.g. 3x4,\n"
                             "which brings the 720x400 display close to the\n"
                             "4:3 shape it had on a monitor")
    parser.add_argument("--program", type=Path, metavar="FILE",
                        help="run this build on the test bench, e.g.\n"
                             "build/portable_c/VC.EXE, in place of the\n"
                             "original from its distribution")
    args = parser.parse_args()
    if args.step and not args.program:
        parser.error("steps are for a program on the bench: --program")
    if args.program:
        screen = capture_on_bench(args.out, args.scale,
                                  setups.Program.load(args.program),
                                  args.step)
    else:
        screen = capture(args.out, args.scale)
    print("\n".join(screen.lines()))


if __name__ == "__main__":
    main()
