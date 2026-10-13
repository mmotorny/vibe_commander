#!/usr/bin/env python3
"""Run the original Volkov Commander 4.05 under FreeDOS and capture its screen.

The shareware distribution is downloaded from its author's site, unpacked
onto a fresh hard disk as C:\\VC with its original file dates, and started
from there in QEMU. Written to the output directory:

  screen.png    the display as the video card renders it
  screen.bin    the 80x25 text screen from video memory, 4000 bytes
                (character, attribute for each cell)
  screen.txt    the characters of screen.bin as UTF-8 text
  screen.json   video mode, cursor position and shape, blinking cells
"""
import argparse
import json
from pathlib import Path

import deps
import dos
import setups


def capture(out, scale):
    out.mkdir(parents=True, exist_ok=True)
    with dos.workdir() as workdir:
        with setups.fresh_install(Path(workdir)) as machine:
            machine.wait_for_boot()
            screen = machine.wait_for_screen()
            picture = machine.screenshot()

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
    parser.add_argument("--out", type=Path, default=deps.ROOT / "out",
                        help="output directory (default: out/)")
    parser.add_argument("--scale", type=parse_scale, default=(1, 1),
                        metavar="XxY",
                        help="enlarge screen.png by whole factors, e.g. 3x4,\n"
                             "which brings the 720x400 display close to the\n"
                             "4:3 shape it had on a monitor")
    args = parser.parse_args()
    screen = capture(args.out, args.scale)
    print("\n".join(screen.lines()))


if __name__ == "__main__":
    main()
