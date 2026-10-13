#!/usr/bin/env python3
"""Hold the program of 32 bits to the layout that it rests on.

    python3 harness/layoutcheck.py [--stub DOS32A.EXE] VC.EXE VC.MAP

The portable C as a flat program, build/portable_c32/VC.EXE, keeps the
memory of the program of 16 bits inside its own: a segment and an offset
are still numbers of 16 bits in it, counted from 64K below the main
data, and a table is still known by its offset among the tables ("The
words of the addresses" in src/portable_c/vc.h). The words that make
pointers of those numbers take things for granted about where the linker
has put what, and say so at each; nothing in the program looks. This
does, in the file that the linker wrote and in its map of it, as le.py
reads them: the tests `portable_c32-layout` and
`portable_c32-debug-layout` of the build, one for each of the two
programs. It runs no machine and takes no time. A link that breaks one
of these gives a program that reads and writes somewhere else, with no
word said: the order of the linker's command in CMakeLists.txt is what
keeps them, and it says there why it is as it is.

What is held of the layout, each with the word that goes by it:

  - Every object of a file of tables, the files whose names end in _t,
    lies in the 64K that begin where the segment of the tables does: the
    paragraph of the megabyte that 32K below Title_S is in. OFFSET makes
    a number of 16 bits of how far a table is behind that, and what lies
    outside gets a number that has wrapped round. The tables that the
    program writes to are among them, which the compiler puts with what
    has a value and not with the constants.
  - The segment of the tables is one of the megabyte: 32K below Title_S
    is not in front of where the megabyte begins, 64K below the main
    data, nor a megabyte behind that. TABLES_BY, which is AsmSeg.
  - IniSave of setup.c lies behind the main data and ends within 64K of
    where that begins: the main loop keeps how far it is behind md as a
    number of 16 bits (MD_PLACE, in Loop of main.c).
  - The program ends within the megabyte: what it has behind the main
    data, the memory that dos32.c hands out above all, is named by
    segments, which are paragraphs counted from 64K below the main data
    in 16 bits. AT, PANEL.
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
    scenario use; everything else of the data does.

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
ARENA = 0x10000                   # how far in front of md the megabyte
                                  # begins: ARENA of vc.h
MEGABYTE = 0x100000
TABLES = 0x8000                   # how far in front of Title_S the tables
                                  # are counted from: TABLES_BY of vc.h
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
        #: How far the program is from two of its limits, in bytes: the
        #: tables from where the offsets of 16 bits end, in front of
        #: Title_S and behind it, and IniSave from the 64K behind md.
        self.room = {}
        # Where each symbol ends: where the next one of its object
        # begins, or the object ends. It is no shorter than the symbol.
        starts = {}
        for symbol in self.map.symbols:
            starts.setdefault(symbol.first, set()).add(symbol.offset)
        self._ends = {}
        for number, offsets in starts.items():
            offsets = sorted(offsets | {self.program.objects[number - 1].size})
            self._ends.update(((number, at), end)
                              for at, end in zip(offsets, offsets[1:]))

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
            self.tables()
            self.behind()
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

    def tables(self):
        """The reach of OFFSET and TABLES_BY."""
        linked = self.map
        md, title = self.symbol("_md"), self.symbol("_Title_S")
        if not md or not title:
            return
        if md[0] != title[0]:
            self.wrong("Title_S and the main data are in two objects")
            return
        # As the words of vc.h count, with the main data at 0: the
        # megabyte begins at -ARENA, and the segment of the tables is
        # the paragraph of it that TABLES below Title_S is in.
        below = title[1] - md[1] - TABLES + ARENA
        if not 0 <= below < MEGABYTE:
            self.wrong(
                f"Title_S is {md[1] - title[1]:,} bytes in front of the "
                f"main data: 32K below it is "
                + ("in front of where the megabyte begins, 64K below md"
                   if below < 0 else "behind the megabyte")
                + ", and the tables have no segment (TABLES_BY of vc.h)")
            return
        begins = md[1] - ARENA + (below >> 4 << 4)
        tables = [(symbol, self._ends[symbol.first, symbol.offset])
                  for symbol in linked.symbols
                  if (symbol.module or "").endswith("_t")]
        if not tables:
            self.wrong("the map has no object of a file of tables")
            return
        outside = [symbol for symbol, end in tables
                   if symbol.first != md[0] or symbol.offset < begins
                   or end > begins + 0x10000]
        if outside:
            first = outside[0]
            self.wrong(
                f"{len(outside)} of the {len(tables)} objects of the files "
                "of tables lie outside the 64K that begin 32K below "
                f"Title_S, the first {first.name} of {first.module}.c, "
                f"{first.offset - title[1]:+,} bytes from it: OFFSET of "
                "vc.h makes of such a one a number that has wrapped round")
        first = min(symbol.offset for symbol, _ in tables)
        last = max(end for _, end in tables)
        self.room["in front of Title_S"] = first - begins
        self.room["behind Title_S"] = begins + 0x10000 - last
        self.room["between Title_S and the main data"] = below
        self.say(f"the tables: {len(tables)} objects, from "
                 f"{first - title[1]:+,} to {last - title[1]:+,} bytes of "
                 f"Title_S, which is {md[1] - title[1]:,} bytes in front "
                 "of the main data")

    def behind(self):
        """What lies behind the main data: IniSave, and the end of the
        program."""
        program = self.program
        md, saved = self.symbol("_md"), self.symbol("_IniSave")
        if not md:
            return
        if saved:
            ends = self._ends[saved] - md[1]
            if saved[0] != md[0] or saved[1] < md[1] or ends > 0xFFFF:
                self.wrong(
                    f"IniSave of setup.c ends {ends:,} bytes behind where "
                    "the main data begins, and the main loop keeps that in "
                    "16 bits (MD_PLACE of vc.h, in Loop of main.c)")
            self.room["behind IniSave"] = 0xFFFF - ends
            self.say(f"IniSave ends {ends:,} bytes behind where the main "
                     "data begins")
        # The megabyte: the end of the program, by its last object.
        ends = program.objects[md[0] - 1].size - md[1]
        if ends > MEGABYTE - ARENA or md[0] != len(program.objects):
            self.wrong(
                f"the program ends {ends:,} bytes behind where the main "
                f"data begins, and a segment names no more than "
                f"{MEGABYTE - ARENA:,} there (AT of vc.h)")
        self.room["behind the program"] = MEGABYTE - ARENA - ends
        self.say(f"the program ends {ends:,} bytes behind where the main "
                 f"data begins, of {MEGABYTE - ARENA:,} that a segment names")

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
                begins += segment.size // 2
                what = "the upper half of the stack"
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
