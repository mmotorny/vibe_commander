#!/usr/bin/env python3
"""List the routines of the port that are still assembly.

For choosing what to translate next. For every procedure in port/asm, in
the order of the source: its file, its length in the original in bytes,
how many places call it, the interrupts it uses, and the procedures it
calls or jumps to. A procedure that has been translated already is only a
stub there and is marked with a C, here and where others call it. A
length of ? belongs to a procedure that the original does not have, or
that the listing does not show as a near one.

    routines.py                 all of them
    routines.py --below 150     those no longer than 150 bytes
    routines.py NgLine LnStr    these

The lengths are read from the listing of the build that is laid out like
the original, build/shareware-layout/VC.LST, which has to be built first.
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "harness"))

import coverage                         # noqa: E402
import ported                           # noqa: E402

SOURCES = ROOT / "port" / "asm"
LISTING = ROOT / "build" / "shareware-layout" / "VC.LST"


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("routine", nargs="*", help="procedures to list")
    parser.add_argument("--below", type=int, metavar="BYTES",
                        help="list only procedures no longer than this")
    args = parser.parse_args()

    listing = coverage.Listing(LISTING)
    translated = {name for batch in ported.BATCHES for name in batch.routines}
    order, where, calls, interrupts = [], {}, {}, {}
    everything = ""
    for path in sorted(SOURCES.iterdir()):
        text = path.read_text(encoding="cp866")
        everything += text
        current = None
        for line in text.split("\n"):
            code = line.split(";")[0]
            if match := re.match(r"(\w+)\s+PROC\b", code):
                current = match[1]
                order.append(current)
                where[current] = path.name
                calls[current], interrupts[current] = set(), set()
            elif re.match(r"\w+\s+ENDP\b", code):
                current = None
            elif current:
                if match := re.search(r"\b(?:CALL|JMP)\s+(?:NEAR PTR )?(\w+)",
                                      code):
                    calls[current].add(match[1])
                if match := re.search(r"\bINT\s+(\w+)", code):
                    interrupts[current].add(match[1])

    def mark(name):
        return name + ("(C)" if name in translated else "")

    for name in args.routine:
        if name not in where:
            sys.exit(f"no procedure {name} in {SOURCES}")
    for name in order:
        if args.routine and name not in args.routine:
            continue
        length = listing.procedures.get(name, (0, None))[1]
        if args.below is not None and (length is None or length > args.below):
            continue
        callers = len(re.findall(rf"\bCALL\s+(?:NEAR PTR )?{name}\b",
                                 everything))
        callees = sorted(c for c in calls[name] if c in where and c != name)
        print(f"{where[name]:11} {mark(name):12} "
              f"{'?' if length is None else length:>5} bytes  "
              f"{callers:3} callers  "
              f"int {','.join(sorted(interrupts[name])) or '-':11} "
              f"calls {' '.join(mark(c) for c in callees) or '-'}")


if __name__ == "__main__":
    main()
