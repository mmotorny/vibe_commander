#!/usr/bin/env python3
"""quiet.py EXE: switch the banner of DOS/32A off in a program that has
dos32a.exe as its stub.

The stub has its settings behind the letters ID32, and bit 3 of the byte
15h behind them is the banner: found by clearing each bit of the settings
in turn on the bench, since the pinned Open Watcom has no description of
them. With the bit clear DOS/32A writes nothing and leaves the cursor
where it is. The linker moves the stub's header, so the letters are
looked for.
"""
import sys

BANNER = 0x15, 0x08               # where behind ID32, and which bit


def main():
    path = sys.argv[1]
    with open(path, "rb") as file:
        program = bytearray(file.read())
    at = program.find(b"ID32")
    if not 0 < at < 0x200:
        sys.exit(f"{path}: no settings of DOS/32A in its stub")
    program[at + BANNER[0]] &= ~BANNER[1]
    with open(path, "wb") as file:
        file.write(program)


if __name__ == "__main__":
    main()
