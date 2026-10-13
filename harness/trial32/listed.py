#!/usr/bin/env python3
"""Print the calls of DOS that calls.asm wrote down, from the disk that
`run.py --keep FILE` kept: FILE.D.img, whose root has CALLS.BIN.

    listed.py build/trial32/calls.D.img

A line for each call, AX, BX, DS and ES as DOS got them, in order. A run
of the same call with the same registers is one line with its count. The
second word of the file, how many calls with 62h in AH went by, which
are the driver's, is said first.
"""
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import fat  # noqa: E402


class Kept:
    """In the place of hashlib in fat.py, which hashes every file that
    it reads and keeps none: the contents, by their length and start."""
    def __init__(self):
        self.files = []

    def sha256(self, contents):
        self.files.append(contents)
        return self

    def hexdigest(self):
        return str(len(self.files) - 1)


def main():
    kept = Kept()
    fat.hashlib = kept
    tree = fat.read_tree(sys.argv[1])
    raw = kept.files[int(tree["CALLS.BIN"].sha256)]
    count, drivers = struct.unpack_from("<HH", raw, 0)
    print(f"{count} calls, and {drivers} of the driver's with 62h")
    calls = [struct.unpack_from("<4H", raw, 4 + 8 * i) for i in range(count)]
    at = 0
    while at < count:
        run = 1
        while at + run < count and calls[at + run] == calls[at]:
            run += 1
        ax, bx, ds, es = calls[at]
        times = f"  x{run}" if run > 1 else ""
        print(f"{at:5}  AX {ax:04X} BX {bx:04X} DS {ds:04X} ES {es:04X}{times}")
        at += run


if __name__ == "__main__":
    main()
