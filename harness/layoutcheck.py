#!/usr/bin/env python3
"""Hold the portable C, a program of 32 bits, to the layout that it rests
on.

    python3 harness/layoutcheck.py [--stub DOS32A.EXE] VC.EXE VC.MAP

The portable C is a flat program, build/portable_c/VC.EXE. What it
rests on of where the linker has put things is little, and is said in
the linker's command in CMakeLists.txt, which is what keeps it: nothing
in the program looks. This does, in the file that the linker wrote and
in its map of it, as le.py reads them: the tests `portable_c-layout`
and `portable_c-debug-layout` of the build, one for each of the two
programs. It runs no machine and takes no time. A link that breaks one
of these gives a program that the tests cannot watch, or one that the
machine of the tests runs many times slower, with no word said.

What is held of the layout:

  - The stack is where the driver of the tests looks for it: the segment
    of the map that ends where the header of the file has the top of the
    stack (coverage.flat_stack), of the class STACK and as long as the
    linker says the stack is; and the file has all of it, as zeros, since
    the tests tell what a scenario has never used of it by what is still
    as it was loaded.
  - What has no value is not in the file, and everything else of the
    data is, the main data among it: the file holds of the data what
    lies in front of the first segment of the class BSS, and no segment
    of another class lies behind that.
  - Nothing that a scenario writes lies in a page of 4K that holds code:
    the machine of the tests is many times slower at such a write. The
    pages are those of the memory on the bench, le.BENCH. The constants
    of the class DATA are not written, CONST and CONST2; of the stack
    the upper half counts, which is what the scenarios' test lets a
    scenario use (coverage.STACK_LEFT); everything else of the data
    does.

And of the file, against the map, which is what says that the reader
has read it right (lecheck.py holds the reader to a toy the same way):

  - It is two objects, the code and the data, each as long as the
    segments that the map has in it; the code is whole in the file.
  - The program is entered at Start_ of the map, which is also what the
    linker's summary says.
  - Every relocation is an address: no selector, which would be a
    segment of 16 bits left in the code. Each lies in what the file
    holds of its object, and the file has there the offset of what it
    names, as the linker leaves it.
  - With --stub: the stub is that file behind its header, DOS/32A's, but
    for the one bit that tools/stub.c clears, which is clear.

It prints where things are, and then what is wrong, and ends with 1 if
anything is.
"""
import argparse
import re
import struct
import sys
from pathlib import Path

import coverage
import le

PAGE = 0x1000                     # of the memory, for the machine of the
                                  # tests
CONSTANTS = "CONST", "CONST2"     # the segments that are never written
# The settings of DOS/32A in a stub, as tools/stub.c has them.
SETTINGS, FLAGS, BANNER = b"ID32", 0x15, 0x08


class Check:
    def __init__(self, contents, linked):
        self.program = le.Linear(contents)
        self.map = le.Map(linked)
        self.text = Path(linked).read_text()
        self.problems = []
        #: What was found, a line each, for whoever reads the test.
        self.found = []

    def wrong(self, text):
        self.problems.append(text)

    def say(self, text):
        self.found.append(text)

    def symbol(self, name):
        """The address of a public symbol of the map, an object's number
        and an offset, or None with a problem noted."""
        if name not in self.map.public:
            self.wrong(f"the linker's map has no {name}")
            return None
        return self.map.public[name]

    def summary(self, pattern):
        found = re.search(pattern, self.text, re.M)
        return found and tuple(int(number, 16) for number in found.groups())

    def all(self, stub=None):
        """Hold everything; the problems are in `problems` then."""
        if self.file():
            self.relocations()
            if stub:
                self.stub(stub)
            self.held()
            self.pages(self.stack())
        return not self.problems

    def file(self):
        program, linked = self.program, self.map
        if [found.code for found in program.objects] != [True, False]:
            self.wrong("the file is not two objects, the code and the data: "
                       f"it has {len(program.objects)}")
            return False
        for found in program.objects:
            ends = max(segment.offset + segment.size
                       for segment in linked.segments
                       if segment.first == found.number)
            if found.size != ends:
                self.wrong(f"object {found.number} is {found.size:,} bytes "
                           f"by the file and {ends:,} by the map's segments")
        code = program.objects[0]
        if code.held != code.size:
            self.wrong(f"the file has {code.held:,} bytes of the code's "
                       f"{code.size:,}")
        entry = self.symbol("Start_")
        if entry and program.entry != entry:
            self.wrong(f"the program is entered at {program.entry[0]:04x}:"
                       f"{program.entry[1]:08x}, which is not Start_")
        said = self.summary(r"^Entry point address: (\w+):(\w+)$")
        if said != program.entry:
            self.wrong("the linker's summary has another entry than the "
                       "file's header")
        if not linked.flat:
            self.wrong("the map is not of a flat program")
        return True

    def relocations(self):
        program = self.program
        selectors = [one for one in program.relocations if not one.address]
        if selectors:
            self.wrong(f"{len(selectors)} relocations are selectors, the "
                       "first at "
                       f"{selectors[0].object:04x}:{selectors[0].offset:08x}"
                       ": a segment of 16 bits is left in the program")
        astray = 0
        for one in program.relocations:
            if not one.address:
                continue
            at = program.in_file(one.object, one.offset)
            last = program.in_file(one.object, one.offset + 3)
            if at is None or last is None or struct.unpack_from(
                    "<i", program.contents, at)[0] != one.target_offset:
                astray += 1
        if astray:
            self.wrong(f"{astray} relocations lie where the file holds "
                       "nothing, or the file has not the offset of what "
                       "they name there")

    def stub(self, path):
        """The stub is DOS/32A's file behind its header, with the bit of
        the banner cleared and nothing else changed."""
        contents, stub = self.program.contents, Path(path).read_bytes()

        def begins(file):
            return struct.unpack_from("<H", file, 8)[0] * 16

        ours = bytearray(contents[begins(contents):self.program.stub])
        theirs = bytearray(stub[begins(stub):])
        at = theirs.find(SETTINGS)
        if at < 0:
            self.wrong(f"{path} has no settings of DOS/32A")
            return
        banner = len(ours) > at + FLAGS and ours[at + FLAGS] & BANNER
        theirs[at + FLAGS] &= ~BANNER
        if banner:
            ours[at + FLAGS] &= ~BANNER
        if ours != theirs:
            self.wrong(f"the stub of the program is not {path}")
        elif banner:
            self.wrong("the stub's bit for the banner is not clear: the "
                       "extender would write its name onto the first screen")

    def stack(self):
        """The stack: where it is, as two places, or None."""
        program, linked = self.program, self.map
        try:
            bottom, top = coverage.flat_stack(program, linked)
        except ValueError as error:
            self.wrong(str(error))
            return None
        found, = (segment for segment in linked.segments if segment.size
                  and program.place(segment.first, segment.offset) == bottom)
        if found.kind != "STACK":
            self.wrong(f"the segment that ends at the stack's top is "
                       f"{found.name} of the class {found.kind}, not the "
                       "stack")
        said = self.summary(r"^Stack size: +(\w+) ")
        if said != (top - bottom,):
            self.wrong(f"the stack is {top - bottom:,} bytes by the map's "
                       "segment and another length by the linker's summary")
        held = program.in_file(found.first, found.offset) is not None \
            and program.in_file(found.first,
                                found.offset + found.size - 1) is not None
        if not held or any(program.image()[bottom:top]):
            self.wrong("the stack is not in the file as zeros: the tests "
                       "tell what a scenario has left of it by what is "
                       "still as it was loaded")
        self.say(f"the stack: {top - bottom:,} bytes at "
                 f"{le.BENCH + bottom:08X}h on the bench")
        return bottom, top

    def held(self):
        """What has no value is not in the file, and the rest is."""
        program, linked = self.program, self.map
        data = program.objects[-1]
        segments = [segment for segment in linked.segments
                    if segment.first == data.number and segment.size]
        valued = [segment for segment in segments if segment.kind != "BSS"]
        none = [segment for segment in segments if segment.kind == "BSS"]
        ends = max(segment.offset + segment.size for segment in valued)
        if none and min(segment.offset for segment in none) < ends:
            late = max(valued, key=lambda segment: segment.offset)
            self.wrong(
                f"{late.name} of the class {late.kind} lies behind what "
                "has no value, and all of that is in the file as zeros: "
                f"{sum(segment.size for segment in none):,} bytes")
        if data.held != ends:
            self.wrong(f"the file has {data.held:,} bytes of the data, and "
                       f"what has a value ends at {ends:,}")
        self.say(f"the file has {data.held:,} bytes of the data's "
                 f"{data.size:,}: all but what has no value")

    def pages(self, stack):
        """Nothing that a scenario writes is in a page that holds code."""
        program, linked = self.program, self.map
        code = program.objects[0]
        # The first address behind the last page that has code.
        clear = (le.BENCH + code.place + code.size - 1) // PAGE * PAGE + PAGE
        for segment in linked.segments:
            if segment.first == code.number or not segment.size \
                    or segment.name in CONSTANTS:
                continue
            begins = le.BENCH + program.place(segment.first, segment.offset)
            what = segment.name
            if stack and program.place(segment.first, segment.offset) \
                    == stack[0]:
                begins += segment.size // coverage.STACK_LEFT
                what = "what a scenario may use of the stack"
            if begins < clear:
                self.wrong(
                    f"{what} begins at {begins:08X}h on the bench, in a "
                    f"page that has code, which ends at "
                    f"{le.BENCH + code.place + code.size:08X}h: every "
                    "write there is many times slower on the machine of "
                    "the tests")
        self.say("the code ends at "
                 f"{le.BENCH + code.place + code.size:08X}h on the bench, "
                 f"and no page from {clear:08X}h on has any")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("program", type=Path)
    parser.add_argument("linked", type=Path, help="the linker's map of it")
    parser.add_argument("--stub", type=Path,
                        help="the file of DOS/32A that it was linked with")
    arguments = parser.parse_args()
    check = Check(arguments.program.read_bytes(), arguments.linked)
    program = check.program
    print(f"{arguments.program}: {len(program.contents):,} bytes, "
          f"{program.objects[0].size:,} of code, "
          f"{len(program.relocations):,} relocations")
    check.all(arguments.stub)
    for found in check.found:
        print(f"  {found}")
    for problem in check.problems:
        print(f"{arguments.program}: {problem}", file=sys.stderr)
    return 1 if check.problems else 0


if __name__ == "__main__":
    sys.exit(main())
