#!/usr/bin/env python3
"""Hold the harness's reader of a flat program, le.py, to one that is
built for it.

    python3 harness/lecheck.py

It builds harness/lecheck.c as a flat program of 32 bits with DOS/32A
for its stub, as the portable C will be built, into build/lecheck/, and
holds what le.py reads of the file and of the linker's map against what
that source says must be there: the test `harness-le` of the build. It
runs no machine and takes under a second. It needs the project
configured once, for the compiler and the stub that the build fetches;
the build gives it their places.

What is held, each against something that is not the reader:

  - where the stub ends, against the header of the stub and against the
    bytes of dos32a.exe;
  - the objects, against the segments that the map has in each, and
    what the file has of them against what has a value in the source;
  - the entry and the stack, against the linker's own summary;
  - a symbol's place, by the bytes that the source has under that name,
    in the file and in the image: a text, and a static variable that
    two files have under one name;
  - the relocations, against the 4,096 addresses of the toy's table,
    which the source gives by rule: what the file has of each, what the
    memory has with the program at one place and at another, and that
    the three of them that lie across the end of a page are each one;
    against code that names data and data that names code; against an
    address in front of the object that it is counted from, and one
    behind it; and a selector;
  - that the image with the addresses put in differs from the file's in
    the bytes that the reader lists as relocated and in no other;
  - that a file cut short or with no linear executable is refused.

What it cannot hold is where the extender puts the objects in the
memory, which is the bench's to say. The toy says it there, of itself:

    python3 harness/lecheck.py --screen
    mkdir -p build/tmp
    TMPDIR=$PWD/build/tmp python3 harness/trial32/run.py \\
        build/lecheck/LECHECK.EXE

The first prints five lines as the reader has them for a program at
le.BENCH, and the second the screen of the bench, which has the same
five from its row 10 on, behind the extender's banner: where the code,
a text, the table and what the file does not hold are; that every
entry of the table names what it should, with their sum; the two
addresses that are outside the object they are counted from; and what
the two words of data that name code have. They were the same on 13
October 2026. And with QEMU's log of the code that ran there,

    TMPDIR=$PWD/build/tmp python3 harness/trial32/run.py \\
        --trace $PWD/build/lecheck/trace build/lecheck/LECHECK.EXE
    python3 harness/lecheck.py --trace build/lecheck/trace

holds every block of it that began in the toy's code, as the reader
places that, against the image with the addresses put in: 48 blocks
that day, 30 of them with an address in them, and none that differed.
"""
import argparse
import re
import struct
import subprocess
import sys
from pathlib import Path

import coverage
import le

ROOT = Path(__file__).resolve().parent.parent
SOURCE = Path(__file__).resolve().with_suffix(".c")
# What the source says: see lecheck.c.
TEXT = b"the reader of linear executables\0"
ENTRIES, STEP, NONE = 4096, 17, 70000
FRONT, BEHIND = 0x10000, 0x345678
KEPT = {"first": 0xF125F125, "second": 0x5EC0D5EC}
STACK = 4096
# Another place than the bench's, to load the program at.
ELSEWHERE = 0x12345670
# As the portable C is compiled for 32 bits: DIARY.md, "Write down the
# plan for the 32 bits, with the scripts of its trials", the entry from
# "The build".
COMPILE = ("-q", "-3r", "-mf", "-os", "-s", "-zl", "-zls", "-wx", "-we")


def build(watcom, tools, out):
    """Compile the toy twice and link it; return the paths of the file
    and of the map."""
    out.mkdir(parents=True, exist_ok=True)
    environment = {"WATCOM": str(watcom), "PATH": "/usr/bin:/bin"}
    for name, more in (("first", ()), ("second", ("-dSECOND",))):
        subprocess.run([tools / "wcc386", *COMPILE, *more,
                        f"-fo={out / name}.obj", SOURCE],
                       env=environment, check=True)
    program, linked = out / "LECHECK.EXE", out / "LECHECK.MAP"
    subprocess.run(
        [tools / "wlink", "option", "quiet", "format", "os2", "le",
         "name", program, "file", out / "first.obj",
         "file", out / "second.obj", "option", "start=Start_",
         "option", f"stack={STACK}",
         "option", f"stub={watcom / 'binw' / 'dos32a.exe'}",
         "option", f"map={linked},static",
         "order", "clname", "CODE", "clname", "DATA", "clname", "STACK",
         "clname", "BSS"],
        env=environment, check=True)
    return program, linked


class Check:
    """The toy as the reader has it, and what is wrong with that."""

    def __init__(self, contents, linked, stub):
        self.program = le.Linear(contents)
        self.map = le.Map(linked)
        self.text = Path(linked).read_text()
        self.stub = stub
        self.problems = []

    def hold(self, what, found, wanted):
        if found != wanted:
            self.problems.append(f"{what}: the reader has {found!r}, and "
                                 f"it is {wanted!r}")

    def place(self, name, module=None):
        """Where the one symbol of that name is, in the image."""
        symbol, = self.map.named(name, module)
        return self.program.place(symbol.first, symbol.offset)

    def stub_ends(self):
        """The stub is a program of DOS by itself, as long as its header
        says, and behind its header it is dos32a.exe behind its own."""
        contents, stub = self.program.contents, self.stub

        def header(file):
            last, pages, _, paragraphs = struct.unpack_from("<4H", file, 2)
            return (pages - 1) * 512 + (last or 512), paragraphs * 16

        length, begins = header(contents)
        self.hold("where the stub ends", self.program.stub, length)
        self.hold("the stub's bytes",
                  contents[begins:self.program.stub] == stub[header(stub)[1]:],
                  True)

    def objects(self):
        program = self.program
        self.hold("the objects that are code",
                  [found.code for found in program.objects], [True, False])
        for found in program.objects:
            ends = [segment.offset + segment.size
                    for segment in self.map.segments
                    if segment.first == found.number]
            self.hold(f"how long object {found.number} is", found.size,
                      max(ends))
        code, data = program.objects
        self.hold("how much of the code the file has", code.held, code.size)
        # Of the data it has what has a value, and the 16 bytes of the
        # stack that have one; none of what has none.
        stack = self.map.segment("STACK")
        self.hold("how much of the data the file has", data.held,
                  stack.offset + 16)
        self.hold("the place in the file of what it does not hold",
                  program.in_file(*self.map.public["_None"]), None)
        self.hold("the entry", program.entry, self.map.public["Start_"])
        said = re.search(r"^Entry point address: (\w+):(\w+)$", self.text,
                         re.M)
        self.hold("the entry, by the linker's summary", program.entry,
                  (int(said[1], 16), int(said[2], 16)))
        self.hold("the stack's top", program.stack,
                  (stack.first, stack.offset + stack.size))
        self.hold("the stack's length, by the map", stack.size, STACK)
        self.hold("that the map is of a flat program", self.map.flat, True)
        self.hold("where the code's segment is",
                  (self.map.segment("_TEXT").first,
                   self.map.segment("_TEXT").offset), (1, 0))

    def names(self):
        """A symbol is where the bytes are that the source has under its
        name: in the file, and in the image."""
        program, contents = self.program, self.program.contents
        image = program.image()
        at = program.in_file(*self.map.public["_Text_S"])
        self.hold("the text, in the file", contents[at:at + len(TEXT)], TEXT)
        at = self.place("_Text_S")
        self.hold("the text, in the image", image[at:at + len(TEXT)], TEXT)
        self.hold("the files that have a static _Kept",
                  sorted(symbol.module for symbol in self.map.named("_Kept")
                         if symbol.mark == "s"), sorted(KEPT))
        self.hold("a static symbol among the public ones",
                  "_Kept" in self.map.public, False)
        for module, value in KEPT.items():
            at = self.place("_Kept", module)
            self.hold(f"the static _Kept of {module}",
                      struct.unpack_from("<I", image, at)[0], value)
        self.hold("the mark of a static function",
                  [symbol.mark for symbol in self.map.named("Hex_")], ["s"])

    def relocations(self):
        program = self.program
        table, none = self.place("_Table"), self.place("_None")
        _, none_offset = self.map.public["_None"]
        entries = [(table + 5 * number, number) for number in range(ENTRIES)]
        relocated = program.relocated()
        # The file has the offset of what is named, in its object; the
        # memory its address.
        for base, first in ((None, none_offset), (le.BENCH, le.BENCH + none),
                            (ELSEWHERE, ELSEWHERE + none)):
            image = program.image(base)
            wrong = [number for at, number in entries
                     if struct.unpack_from("<IB", image, at)
                     != (first + STEP * number, number & 0xFF)]
            where = "the file" if base is None else f"the memory at {base:X}h"
            self.hold(f"the entries of the table that are wrong in {where}",
                      wrong[:5], [])
        self.hold("the entries of the table that have no relocation",
                  [number for at, number in entries
                   if at not in relocated or not relocated[at].address][:5],
                  [])
        # Three lie across the end of a page of the file, cut at each of
        # the three places.
        _, offset = self.map.public["_Table"]
        across = sorted(-(offset + 5 * number) % program.page
                        for number in range(ENTRIES)
                        if (offset + 5 * number) % program.page
                        > program.page - 4)
        if across != [1, 2, 3]:
            self.problems.append(
                "the toy no longer has an address across the end of a page "
                f"at each of its bytes: {across}. See Table of lecheck.c")
        self.hold("how many addresses of the data name data",
                  sum(one.object == 2 and one.target == 2
                      for one in program.relocations), ENTRIES + 2)
        if not none_offset < 0x10000 <= none_offset + STEP * (ENTRIES - 1):
            self.problems.append("the toy no longer names something on "
                                 "both sides of 64K of the data")

        # What is named in front of its object, and behind it.
        outside = self.place("_Outside")
        self.hold("the relocations that name something outside an object",
                  [relocated[at].target_offset
                   for at in (outside, outside + 4)],
                  [offset - FRONT, none_offset + NONE + BEHIND])
        for base in (le.BENCH, ELSEWHERE):
            self.hold(f"the words that name something outside at {base:X}h",
                      struct.unpack_from("<2I", program.image(base), outside),
                      (base + table - FRONT, base + none + NONE + BEHIND))

        # Code that names data, and data that names code.
        for base in (le.BENCH, ELSEWHERE):
            image = program.image(base)
            gives, text = self.place("Gives_"), self.place("_Text_S")
            self.hold(f"the code of Gives at {base:X}h",
                      image[gives:gives + 6],
                      b"\xB8" + struct.pack("<I", base + text) + b"\xC3")
            self.hold(f"the words that name code at {base:X}h",
                      struct.unpack_from("<2I", image, self.place("_Calls")),
                      (base + gives, base + self.place("Start_")))

        # The selector, which is not known and stays as the file has it.
        selectors = [one for one in program.relocations if not one.address]
        loads = self.map.public["Loads_"]
        self.hold("the selectors",
                  [(one.object, one.offset, one.length, one.target)
                   for one in selectors], [(loads[0], loads[1] + 2, 2, 2)])

        # The memory differs from the file where an address is, and
        # nowhere else; and every relocation is inside its object and
        # names something inside one.
        plain, loaded = program.image(), program.image(le.BENCH)
        differ = {at for at in range(len(plain)) if plain[at] != loaded[at]}
        listed = {at + byte for at, one in relocated.items()
                  for byte in range(one.length)}
        self.hold("bytes that differ in the memory and are not listed as "
                  "relocated", sorted(differ - listed)[:5], [])
        self.hold("how many bytes are listed as relocated", len(listed),
                  4 * (len(program.relocations) - 1) + 2)
        self.hold("relocations that are not inside their object",
                  [one for one in program.relocations
                   if one.offset + one.length
                   > program.objects[one.object - 1].size], [])
        for one in program.relocations:
            if one.address and base_of(plain, program, one) \
                    != one.target_offset & 0xFFFFFFFF:
                self.problems.append(
                    f"the file has not the offset of what is named at "
                    f"{one.offset:X}h of object {one.object}")
                break

    def refusals(self):
        contents = self.program.contents
        for what, damaged in (
                ("a file cut short", contents[:-1]),
                ("a file with no linear executable",
                 contents[:self.program.stub] + b"NE"
                 + contents[self.program.stub + 2:]),
                ("the stub by itself", self.stub)):
            try:
                le.Linear(damaged)
            except le.Unknown:
                continue
            self.problems.append(f"{what} is read as if it were whole")

    def screen(self):
        """What the toy writes on the bench, by the reader."""
        image = self.program.image(le.BENCH)
        table = self.place("_Table")
        total = sum(struct.unpack_from("<I", image, table + 5 * number)[0]
                    for number in range(ENTRIES)) & 0xFFFFFFFF
        first, second = struct.unpack_from("<2I", image, self.place("_Calls"))
        outside = struct.unpack_from("<2I", image, self.place("_Outside"))
        kept, = struct.unpack_from("<I", image, self.place("_Kept", "first"))
        return [
            " ".join(f"{name} {le.BENCH + self.place(symbol):08X}"
                     for name, symbol in (("code", "Gives_"),
                                          ("text", "_Text_S"),
                                          ("table", "_Table"),
                                          ("none", "_None"))),
            "entries that do not name what they should: 00000000",
            f"their sum: {total:08X}",
            f"outside {outside[0]:08X} {outside[1]:08X}",
            f"calls {first:08X} {second:08X} kept {kept:08X}"]


def ran(program, trace):
    """Hold the image of `program` at le.BENCH against the code that
    ran there, by QEMU's log at `trace`; return success."""
    image = program.image(le.BENCH)
    relocated = {at + byte for at, one in program.relocated().items()
                 for byte in range(one.length)}
    code = program.objects[0]
    blocks = {tuple(block) for block in coverage.blocks(trace)
              if 0 <= block[0][0] - le.BENCH < code.size}
    other = with_address = 0
    for block in sorted(blocks):
        for address, there in block:
            at = address - le.BENCH
            if image[at:at + len(there)] != there:
                other += 1
                print(f"  at {address:08X}h {there.hex()} ran, and the "
                      f"reader has {image[at:at + len(there)].hex()}")
                break
        with_address += any(address - le.BENCH + byte in relocated
                            for address, there in block
                            for byte in range(len(there)))
    print(f"{len(blocks)} blocks ran in the code as the reader places it, "
          f"{with_address} of them with an address that the extender put "
          f"in; {other} are not what the reader has there")
    return bool(blocks) and not other


def base_of(image, program, one):
    """What the image has at a relocation's four bytes."""
    return struct.unpack_from("<I", image,
                              program.place(one.object, one.offset))[0]


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    watcom = ROOT / "build" / "_deps" / "open_watcom-src"
    parser.add_argument("--watcom", type=Path, default=watcom,
                        help="where the build has Open Watcom")
    parser.add_argument("--tools", type=Path,
                        help="its programs for this computer (binl64 in it)")
    parser.add_argument("--out", type=Path,
                        default=ROOT / "build" / "lecheck",
                        help="where to build the toy (build/lecheck)")
    parser.add_argument("--screen", action="store_true",
                        help="print what the toy writes on the bench, by "
                             "the reader, and check nothing")
    parser.add_argument("--trace", metavar="FILE",
                        help="hold the reader's image against QEMU's log "
                             "of the toy on the bench, and check nothing "
                             "else")
    args = parser.parse_args()
    tools = args.tools or args.watcom / "binl64"
    if not (tools / "wcc386").exists():
        sys.exit(f"lecheck: no wcc386 in {tools}: the project has to be "
                 "configured once, for the compiler that the build fetches")
    program, linked = build(args.watcom, tools, args.out)
    check = Check(program.read_bytes(), linked,
                  (args.watcom / "binw" / "dos32a.exe").read_bytes())
    if args.screen:
        print("\n".join(check.screen()))
        return
    if args.trace:
        sys.exit(0 if ran(check.program, args.trace) else 1)
    check.stub_ends()
    check.objects()
    check.names()
    check.relocations()
    check.refusals()
    for problem in check.problems:
        print(problem)
    found = check.program
    addresses = sum(one.address for one in found.relocations)
    print(f"{program.name}: {len(found.contents):,} bytes, the stub "
          f"{found.stub:,} of them; "
          + ", ".join(f"object {one.number} {one.size:,} bytes with "
                      f"{one.held:,} in the file" for one in found.objects)
          + f"; {addresses:,} addresses and "
          f"{len(found.relocations) - addresses} selector relocated; "
          f"{len(check.map.symbols)} symbols. "
          + (f"{len(check.problems)} things are not as the source says"
             if check.problems else "All as the source says"))
    sys.exit(1 if check.problems else 0)


if __name__ == "__main__":
    main()
