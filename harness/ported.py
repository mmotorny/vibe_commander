#!/usr/bin/env python3
"""Check the port: the program that is part assembly and part C.

The port is taken through every scenario and must show the same screens
and leave the same disks as the original did, which the goldens record
(see compare.py and goldens.py). That alone would say little about a
routine no scenario reaches, so for the routines translated so far, listed
below, this also checks that

  - every instruction of the assembly they replace ran in the original, in
    some scenario, unless it is listed here with the reason why none gets
    there,
  - every C function that replaces them ran in the port, and
  - the port is no more than three times the size of the original.

What ran in the original was recorded with the goldens; what runs in the
port is read from QEMU's log of the code it executed (see coverage.py).
The assembly is identified by the listing of the build that is laid out
like the original, and the C functions by the port's link map.

Exits with status 0 if all of this holds and 1 otherwise.
"""
import argparse
import sys
from dataclasses import dataclass, field

import compare
import coverage
import goldens
import scenarios
import setups

COM_ORIGIN = 0x100                # where a .COM file is loaded
EXE_HEADER = 0x20                 # of the port's VC.EXE, see tools/mkexe.c

# How large the port may grow, as a multiple of the original's size. The
# aim is to stay well below; this is where growth must have gone wrong.
SIZE_LIMIT = 3


@dataclass(frozen=True)
class Batch:
    """Routines translated together."""
    title: str
    source: str                   # the C file in port/
    routines: tuple               # the assembly procedures it replaces
    functions: tuple              # its functions that the assembly calls
    # Address in the original -> why no scenario executes the instruction.
    unreached: dict = field(default_factory=dict)


BATCHES = [
    Batch(
        title="Numbers, sizes, dates and times as text",
        source="format.c",
        routines=("Number", "Numb10", "Numb20", "SzDtTm", "PutSize",
                  "PutDat0", "PutDate", "BufDate", "PutTim0", "PutTime",
                  "BufTime", "HexDec"),
        functions=("HexDec_c", "Number_c", "PutDate_c", "BufDate_c",
                   "PutTime_c", "BufTime_c", "SzDtTm_c"),
        unreached={
            **dict.fromkeys(
                (0xA420, 0xA422, 0xA424, 0xA427, 0xA429, 0xA42B, 0xA42C,
                 0xA42D),
                "Number, for a country without a thousands separator: the "
                "harness knows of none to configure DOS for"),
            **dict.fromkeys(
                (0xAE02, 0xAE18),
                "PutDate and BufDate, for a country that writes the year "
                "first: the scenarios cover the United States and Russia"),
        },
    ),
]


def check_original(listing, ran):
    """Report on the assembly that has been replaced; return success.

    `ran` holds the offsets in the original's file of the bytes that ran.
    """
    passed = True
    for batch in BATCHES:
        lines = [line for name in batch.routines
                 for line in listing.procedure(name)]
        missed = [line for line in lines
                  if line.address - COM_ORIGIN not in ran]
        excused = [line for line in missed if line.address in batch.unreached]
        stale = sorted(set(batch.unreached)
                       - {line.address for line in missed})
        print(f"{batch.title}: {len(lines) - len(missed)} of {len(lines)} "
              f"instructions of the original ran, {len(excused)} more are "
              "listed as out of reach")
        for line in missed:
            if line.address not in batch.unreached:
                passed = False
                print(f"  not executed: {line.address:04X} {line.text}")
        for address in stale:
            passed = False
            print(f"  listed as out of reach, but executed or not an "
                  f"instruction of these routines: {address:04X}")
    return passed


def check_port(link_map, executed):
    """Report on the C functions that replace it; return success."""
    symbols = coverage.link_map(link_map)
    passed = True
    for batch in BATCHES:
        for name in batch.functions:
            segment, offset = symbols[name]
            start = segment * 16 + offset
            if start - COM_ORIGIN + EXE_HEADER not in executed:
                passed = False
                print(f"  not executed in the port: {name}")
        print(f"{batch.title}: {len(batch.functions)} C functions of "
              f"{batch.source} checked")
    return passed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("port", help="the port's VC.EXE")
    parser.add_argument("link_map", help="its link map")
    parser.add_argument("listing", help="the listing of VC.COM laid out "
                                        "like the original")
    args = parser.parse_args()

    original, port = setups.original_vc(), setups.Program.load(args.port)
    size = len(port.contents)
    limit = SIZE_LIMIT * len(original.contents)
    print(f"VC.EXE is {size} bytes, {size / len(original.contents):.2f} "
          f"times the original's {len(original.contents)}; the limit is "
          f"{SIZE_LIMIT} times")

    names = list(scenarios.SCENARIOS)
    failed, runs = compare.compare(names, port, trace=True)
    in_original, in_port = set(), {}
    for name, run in zip(names, runs):
        in_original |= goldens.read_ran(name)
        in_port.update(run.executed)
    passed = check_original(coverage.Listing(args.listing), in_original)
    passed = check_port(args.link_map, in_port) and passed
    sys.exit(failed or not passed or size > limit)


if __name__ == "__main__":
    main()
