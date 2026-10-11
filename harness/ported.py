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
  - every C function that replaces them ran in the port,
  - the port is no more than three times the size of the original, and
  - the port leaves some of its stack untouched in every scenario.

The stack is 256 bytes, a few less in the build that is tested, and
compiled code uses more of it than the assembly it replaces. The build
that is tested marks its stack when it has set it up, and how much of the
mark is left is read from the machine every time the program waits for a
key (see scenarios.py). A stack used beyond its bottom overwrites what the
program saves of its panels and its setup while a command runs, and no
screen would show that. What every scenario leaves is reported, not only
the least: a new scenario may go deeper than any before it by itself, and
what it leaves before a batch is translated and after says which it was,
the scenario or the batch.

A comparison that finds nothing is only worth something if it would have
found a difference. So one cell of a screen that the port has shown is
changed, and then one entry is taken off a disk it has left, and the
comparison must report each.

The build of the port that is tested is the one that signals where it is.
What ran in the original was recorded with the goldens; what runs in the
port is read from QEMU's log of the code it executed (see coverage.py).
The assembly is identified by the listing of the build that is laid out
like the original, and the C functions by the port's link map.

Exits with status 0 if all of this holds and 1 otherwise.
"""
import argparse
import sys
import textwrap
from dataclasses import dataclass, field, replace

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

# How many bytes of its stack the port must leave untouched in every
# scenario. When this was set the scenarios left 36 at the least, and had
# done since before the first routine was translated. What the limit
# keeps is room for what no scenario has: an interrupt that arrives when
# the calls are at their deepest.
STACK_LEFT = 24


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
    Batch(
        title="Lines of text with markup",
        source="line.c",
        routines=("LnStr", "NgLine", "NgLn30", "CntLine"),
        functions=("LnStr_c", "NgLine_c", "CntLine_c"),
        unreached={
            **dict.fromkeys(
                (0x99CF, 0x99D0, 0x99D1, 0x99D2),
                "LnStr, for ^A and ^C: it measures texts of the program "
                "only, and none has either"),
            **dict.fromkeys(
                (0x9A9D, 0x9A9F, 0x9AA1, 0x9AA3, 0x9AA5, 0x9AA7, 0x9AA8,
                 0x9AAB, 0x9AAD, 0x9AAE, 0x9AB0, 0x9AB3, 0x9AB5, 0x9AB6,
                 0x9AB8, 0x9ABA, 0x9ABC, 0x9ABD, 0x9AC0, 0x9AC2, 0x9AC5,
                 0x9AC7, 0x9AC9),
                "NgLine, for ^A, ^C and a letter that means nothing: no "
                "text of the program or of its help file has any"),
        },
    ),
    Batch(
        title="Texts, paths and file names",
        source="string.c",
        routines=("AddFile", "MvzLin0", "MvzLine", "FilZero", "StrLen",
                  "LowCase", "FndLet", "CutPath", "NormStr", "PathLin",
                  "LetLin", "ShrtLin", "MainFil"),
        functions=("AddFile_c", "MvzLine_c", "FilZero_c", "StrLen_c",
                   "LowCase_c", "FndLet_c", "CutPath_c", "NormStr_c",
                   "PathLin_c", "LetLin_c", "ShrtLin_c", "MainFil_c"),
    ),
    Batch(
        title="Colours and the screen buffer",
        source="screen.c",
        routines=("BasAdr", "Color", "ColAddr", "Shadow", "MvsBwc",
                  "MvsBwe", "MvsColr", "MovDz"),
        functions=("BasAdr_c", "Color_c", "ColAddr_c", "Shadow_c",
                   "MvsBw_c", "MvsColr_c", "MovDz0_c"),
        unreached={
            0xA28F: "ColAddr, for a palette that is none of the three: "
                    "the Configuration dialog sets no other",
            0xA29C: "ColAddr, for a monochrome display: the machine has "
                    "a colour one",
        },
    ),
    Batch(
        title="Hex digits and numbers that are typed",
        source="digits.c",
        routines=("HexByt", "HexCod", "TstHex", "TxtNum"),
        functions=("HexByt_c", "TstHex_c", "TxtNum_c"),
    ),
    Batch(
        title="File names in their fixed form, and masks",
        source="mask.c",
        routines=("Convert", "Conv10", "UnConv", "UnCnv1", "TstMas"),
        functions=("Convert_c", "UnConv_c", "TstMas_c"),
    ),
    Batch(
        title="The list of files of a panel: its filter and its order",
        source="files.c",
        routines=("SetMask", "TstExe", "Sort", "CmpPar", "CmpExt", "FnFile",
                  "NearFl", "TotSel"),
        functions=("SetMask_c", "TstExe_c", "Sort_c", "FnFile_c",
                   "NearFl_c", "TotSel_c"),
    ),
    Batch(
        title="Where the panels are, and the cursor in them",
        source="place.c",
        routines=("BegBX", "SizDX", "LnsAX", "InvWCB", "AdrNum", "FilNum",
                  "NumAdr", "ChgAdr", "CorPar"),
        functions=("BegBX_c", "SizDX_c", "LnsAX_c", "InvWCB_c", "AdrNum_c",
                   "NumAdr_c", "ChgAdr_c", "CorPar_c"),
    ),
    Batch(
        title="A panel of files on the screen: its frame, its path, its "
              "files and the cursor",
        source="panel.c",
        routines=("Panel", "Panel8", "PutPath", "PtPt10", "PutName",
                  "PutNm5", "PutFls", "SetCur", "FilDir"),
        functions=("Panel_c", "PutPath_c", "PtPt10_c", "PutFls_c",
                   "SetCur_c", "FilDir_c"),
        unreached={
            0xAF9B: "SetCur, for a panel that shows neither files nor the "
                    "tree, or a list with nothing in it: it is called when "
                    "the cursor moves or a file is selected, and such a "
                    "panel has neither",
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


def check_stack(names, runs):
    """Report on how much of its stack the port left untouched; return
    success."""
    left = {name: run.stack_left for name, run in zip(names, runs)}
    least = min(left, key=left.get)
    print(f"The stack: {left[least]} bytes of it are left untouched at the "
          f"least, in {least}; the limit is {STACK_LEFT}")
    print(textwrap.fill(
        "in each scenario: " + ", ".join(f"{name} {left[name]}"
                                         for name in names),
        width=78, initial_indent="  ", subsequent_indent="    ",
        break_on_hyphens=False))
    for name in names:
        if left[name] < STACK_LEFT:
            print(f"  too little of the stack left: {left[name]} bytes in "
                  f"{name}")
    return left[least] >= STACK_LEFT


def check_comparison(name, run, program):
    """Report on whether the comparison notices differences; return
    success.

    `run` is that of `program` in scenario `name`. The character in the
    top left corner of its first screen is changed, and then the first
    entry of its drive C: is left out.
    """
    screen = run.screens[0]
    cells = bytearray(screen.cells)
    cells[0] ^= 1
    entries = dict(run.disks["C"])
    del entries[min(entries)]
    changes = {
        "a changed cell on the first screen": replace(
            run, screens=[replace(screen, cells=bytes(cells))]
            + run.screens[1:]),
        "an entry missing from drive C:": replace(
            run, disks={**run.disks, "C": entries}),
    }
    golden = goldens.read(name)
    passed = True
    for what, changed in changes.items():
        if goldens.differences(golden, changed, {program.filename}):
            print(f"The comparison: {what} of {name} is reported")
        else:
            passed = False
            print(f"  not reported by the comparison: {what} of {name}")
    return passed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("port", help="the port's VC.EXE, the build that "
                                     "signals")
    parser.add_argument("link_map", help="its link map")
    parser.add_argument("listing", help="the listing of VC.COM laid out "
                                        "like the original")
    parser.add_argument("shipped", help="the port's VC.EXE as it is shipped, "
                                        "for its size")
    args = parser.parse_args()

    original, port = setups.original_vc(), setups.Program.load(args.port)
    size = len(setups.Program.load(args.shipped).contents)
    limit = SIZE_LIMIT * len(original.contents)
    print(f"VC.EXE is {size} bytes, {size / len(original.contents):.2f} "
          f"times the original's {len(original.contents)}; the limit is "
          f"{SIZE_LIMIT} times")

    names = list(scenarios.SCENARIOS)
    failed, runs = compare.compare(names, port, signals=True, trace=True)
    in_original, in_port = set(), {}
    for name, run in zip(names, runs):
        in_original |= goldens.read_ran(name)
        in_port.update(run.executed)
    passed = check_original(coverage.Listing(args.listing), in_original)
    passed = check_port(args.link_map, in_port) and passed
    passed = check_stack(names, runs) and passed
    passed = check_comparison(names[0], runs[0], port) and passed
    sys.exit(failed or not passed or size > limit)


if __name__ == "__main__":
    main()
