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
import tempfile
import zipfile
from pathlib import Path

import deps
from dos import FatImage, Machine, boot_floppy, timestamp

# The PC's clock at power-on, and the date given to directories on its
# disks: noon on the day Volkov Commander 4.05 was released.
CLOCK = "2000-06-16T12:00:00"
EPOCH = timestamp(2000, 6, 16, 12)


def install_volkov_commander(disk, distribution):
    """Unpack the distribution into \\VC, keeping the archive's file dates."""
    disk.mkdir("VC")
    with zipfile.ZipFile(distribution) as archive:
        for member in archive.infolist():
            disk.write(f"VC/{member.filename}", archive.read(member),
                       timestamp(*member.date_time))


def capture(out, scale):
    out.mkdir(parents=True, exist_ok=True)
    distribution = deps.fetch(deps.VC_SHAREWARE)
    freedos = deps.fetch(deps.FREEDOS)
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        floppy = boot_floppy(workdir / "a.img", workdir, freedos, EPOCH,
                             ["C:", r"CD \VC", "VC"])
        # The serial number is the day work on the program began, 16.05.1991.
        disk = FatImage.hard_disk(workdir / "c.img", workdir, EPOCH,
                                  label="VOLKOV", serial="19910516")
        install_volkov_commander(disk, distribution)
        with Machine(floppy.path, [disk.path], workdir, CLOCK) as machine:
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
