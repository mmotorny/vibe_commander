#!/usr/bin/env python3
"""Check built programs against the original binaries, byte for byte.

The original VC.COM and VCSETUP.COM are shareware: they hold registration
code, tables and strings that the published source lacks. The source can be
assembled with SHAREWARE_LAYOUT defined, which reserves exactly the room
that material takes and fills it: NOPs for code, zeros for data. Such a
build has every other byte at the address it has in the original.

For each file given, this checks that

  - it is as long as the original of the same name,
  - inside the regions listed below it holds nothing but the filler, and
  - everywhere else it is identical to the original.

Exits with status 0 if every file passes and 1 otherwise.
"""
import argparse
import sys
from pathlib import Path

import setups

NOP, ZERO = 0x90, 0x00

# The parts of the original files that have no counterpart in the source:
# (file offset, length, filler in our build, what the original has there).
# They add up to 109 of 65090 bytes in VC.COM and 691 of 24278 in VCSETUP.COM.
REGIONS = {
    "VC.COM": [
        (0x078B, 35, NOP, "check of the registration data at start-up"),
        (0x07BB, 4, NOP, "skipping the edition name when registered"),
        (0x07CB, 22, NOP, "printing the registration lines of the banner"),
        (0xE386, 9, ZERO, "the name of the edition in the banner"),
        (0xE3BD, 39, ZERO, "the registration lines of the banner"),
    ],
    "VCSETUP.COM": [
        (0x02F2, 275, NOP, "the registration switch, dialog and check"),
        (0x2B5B, 119, ZERO, "the registration dialog and its error box"),
        (0x35C9, 9, ZERO, "the name of the edition in the banner"),
        (0x35EC, 13, ZERO, "the copyright holder in the banner"),
        (0x36BA, 275, ZERO, "strings of the registration dialog"),
    ],
}


def check(name, built, original):
    """Return a list of problems; empty if `built` passes."""
    if len(built) != len(original):
        return [f"is {len(built)} bytes long, the original {len(original)}"]
    problems = []
    position = 0
    for offset, length, filler, what in REGIONS[name]:
        end = offset + length
        if built[offset:end] != bytes([filler]) * length:
            problems.append(f"bytes {offset:#x}-{end - 1:#x} ({what}) are "
                            f"not all filler {filler:#04x}")
        problems += mismatches(built, original, position, offset)
        position = end
    problems += mismatches(built, original, position, len(original))
    return problems


def mismatches(built, original, start, end):
    differing = [i for i in range(start, end) if built[i] != original[i]]
    if not differing:
        return []
    return [f"{len(differing)} bytes between {start:#x} and {end - 1:#x} "
            f"differ from the original, the first at {differing[0]:#x}"]


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("files", type=Path, nargs="+",
                        help="programs built with SHAREWARE_LAYOUT, named "
                             "as in the original distribution")
    args = parser.parse_args()

    originals = setups.distribution()
    failed = False
    for path in args.files:
        problems = check(path.name, path.read_bytes(),
                         originals[path.name][0])
        if problems:
            failed = True
            for problem in problems:
                print(f"{path.name}: {problem}")
            continue
        regions = REGIONS[path.name]
        skipped = sum(length for _, length, _, _ in regions)
        size = path.stat().st_size
        print(f"{path.name}: {size - skipped} of {size} bytes identical to "
              f"the original; {skipped} bytes in {len(regions)} regions are "
              "filler")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
