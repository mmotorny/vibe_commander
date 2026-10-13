#!/usr/bin/env python3
"""The instructions of the setup program by class, counted in the listing
of the build that is laid out like the original,
build/shareware-layout/VCSETUP.LST: those of a routine that the main
program has word for word, of one that it has with another text, of one
that it has not, and of the code that stands between procedures. And how
many of each have run in the scenarios of the setup program, by what was
recorded with their goldens (goldens.ran), with the routines that are not
the main program's word for word one by one:

    classes.py
"""
import bisect
import sys
from pathlib import Path

import procs

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import coverage                 # noqa: E402
import goldens                  # noqa: E402
import setups                   # noqa: E402

COM_ORIGIN = 0x100


def main():
    listing = coverage.Listing(setups.SETUP_LISTING)
    main_program, setup = procs.procs(procs.MAIN), procs.procs(procs.SETUP)
    ran = goldens.ran(setup=True, missing=[])
    ranges = {name: listing.procedures[name] for name in setup
              if name in listing.procedures}
    addresses = sorted(line.address for line in listing.lines)

    def size(address):
        """About how long the instruction is: as far as to the next."""
        at = bisect.bisect_right(addresses, address)
        return min(addresses[at] - address, 8) if at < len(addresses) else 1

    def klass(address):
        for name, (start, length) in ranges.items():
            if start <= address < start + length:
                if name not in main_program:
                    return "the setup program's alone, a procedure", name
                if main_program[name][1] == setup[name][1]:
                    return "the main program's word for word", name
                return "the main program's with another text", name
        return "the setup program's alone, between procedures", None

    totals, routines = {}, {}
    for line in listing.lines:
        kind, name = klass(line.address)
        total = totals.setdefault(kind, [0, 0, 0])
        total[0] += 1
        total[1] += size(line.address)
        total[2] += line.address - COM_ORIGIN in ran
        if name:
            routine = routines.setdefault((kind, name), [0, 0])
            routine[0] += 1
            routine[1] += line.address - COM_ORIGIN in ran
    for kind, (count, length, hit) in totals.items():
        print(f"{kind:46} {count:5} instructions, about {length:5} bytes"
              + (f", {hit:5} ran" if ran else ""))
    print(f"{'all':46} {sum(total[0] for total in totals.values()):5} "
          "instructions"
          + (f", {sum(total[2] for total in totals.values())} ran"
             if ran else ""))
    for (kind, name), (count, hit) in sorted(routines.items()):
        if "word for word" not in kind:
            print(f"   {name:8} {count:4}" + (f", {hit:4} ran" if ran else "")
                  + f"   {kind}")


if __name__ == "__main__":
    main()
