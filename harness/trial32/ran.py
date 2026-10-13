#!/usr/bin/env python3
"""Say which functions of a flat program ran on the bench, in the order
in which each first did: where a program that does not get to its first
screen stops, and by which way it got there.

    mkdir -p build/tmp
    TMPDIR=$PWD/build/tmp python3 harness/trial32/run.py \\
        --trace $PWD/build/tmp/trace build/portable_c32/VC.EXE
    python3 harness/trial32/ran.py build/tmp/trace \\
        build/portable_c32/VC.EXE build/portable_c32/VC.MAP

run.py has QEMU write a log of the code that it ran, and this reads the
log by the harness's reader of the file and of the linker's map
(harness/le.py), with the program where the extender puts it on the
bench, le.BENCH. It prints a line for each function of the map in which
a block of code ran, with the file that has it, in the order of the log.

What that order is, and is not:

  - QEMU logs a block of code when it translates it, which is when the
    block first runs, and once. So a function is listed where its first
    block ran, a function that ran a hundred times is listed once, and
    the last line is the function that was entered last for the first
    time, not the one that the program was in when it stopped.
  - A block is taken for the function whose name is the last in front
    of it in the map, a static one too. Where a block does not begin
    where that function does, the line says how far behind: that is
    then a piece of the function that follows, as a rule. The compiler
    puts pieces of a function in front of its label, behind the `ret`
    of the function before, and the map has no name for them: the lines
    by which Round of main.c goes to AlocEr stand behind Exit, and are
    listed as Exit, from 76h in. The listing of the file says which,
    build/portable_c32/<file>.lst.
  - The compiler gives every function that does nothing the address of
    the next instruction, so several names of the map are one place:
    of dos32.c's functions that have nothing to do there, several are
    one `ret`.
    Such a line names them all.
  - A block that is not what the reader has at its place is counted and
    not listed: with the program where le.BENCH has it there is none.
"""
import sys
from bisect import bisect_right
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import coverage                                        # noqa: E402
import le                                              # noqa: E402


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    trace, file, linked = sys.argv[1:]
    program = le.Linear(Path(file).read_bytes())
    code = program.objects[0]
    names = {}                    # offset in the code -> who is there
    for symbol in le.Map(linked).symbols:
        if symbol.first == code.number:
            names.setdefault(symbol.offset, []).append(
                (symbol.name, symbol.module))
    starts = sorted(names)
    image = program.image(le.BENCH)
    listed, other, blocks = set(), 0, 0
    for block in coverage.blocks(trace):
        address, there = block[0]
        at = address - le.BENCH
        if not 0 <= at < code.size:
            continue
        blocks += 1
        if image[at:at + len(there)] != there:
            other += 1
            continue
        start = starts[bisect_right(starts, at) - 1]
        if start in listed:
            continue
        listed.add(start)
        files = sorted({module for _, module in names[start]})
        print(f"{address:08X}  "
              + ", ".join(name for name, _ in names[start])
              + "  of " + ", ".join(f"{module}.c" for module in files)
              + ("" if at == start else f", from {at - start:X}h in"))
    print(f"{len(listed)} places of the map in {blocks} blocks of the "
          f"program's code; {other} blocks are not what the file has there")


if __name__ == "__main__":
    main()
