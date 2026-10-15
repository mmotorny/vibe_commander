"""What Open Watcom's linker writes for a flat program of 32 bits, read:
the file, a linear executable (LE) behind the extender's stub, and the
linker's map of it.

    python3 harness/le.py FILE [MAP]

prints what the file has, and with the map where each segment and how
many symbols are; harness/lecheck.py holds this reader to a file that
it builds.

The file. The stub is a program of real mode, the extender, DOS/32A,
and the number at 3Ch of the file says where it ends and the linear
executable begins. That has a header, a table of its objects, a map of
their pages, and the relocations page by page. An object is what a
segment is to a program of 16 bits, a stretch of the memory that is
loaded as one: the linker makes one of the code and one of everything
else, and its map counts an address as an object's number and an offset
in it, `0002:0000f48c`. An object is as long as its header says, and the
file has as much of it as it has pages for, 4K each but the last of the
file: what it has none for is zeros when the program is entered, the
variables without a value and whatever the linker's `order` has behind
them.

Where the objects are in the memory. The extender asks its own memory
for each in turn, and a block of that has 16 bytes of the extender's in
front: so an object begins 16 bytes behind the end of the one before,
that end rounded up to 16. On the bench of the tests, where DOS/32A has
the memory from the megabyte on for itself, the first begins at BENCH.
Both were seen there, the same in every run (TODO.md, "The portable C in
32 bits: what a trial on the bench showed"), and neither is promised by
anything: with an XMS driver loaded the first object is elsewhere. So
everything here is counted from where the first object begins, a
*place*, and the address of that is given by whoever knows it, as
coverage.py finds where a program is from a trace of what ran.

A program that this one runs, with this one still in the memory. If it
has DOS/32A for its extender too, that one takes no memory of its own:
it asks the first for it, and its first object is the next block of the
same memory that is free, behind the last object of this program as an
object is behind the one before, and behind what blocks this program
has taken there and keeps while it runs another. The portable C keeps
its two panels, KEPT: it takes them first, right behind its objects,
and gives the room behind them back before it runs anything (Init11 and
GiveMem of start.c). So a copy of the program that is started from a
command of the program is at behind(), a copy that that one starts is
behind it by as much again, and copies() has them. Seen on the bench as
the rest was, with the program in every scenario that starts it twice
and with a toy three deep (harness/drivercheck.py), and promised by as
little; the blocks kept in front of a copy, with toys (TODO.md, "X, the
extender's memory on the bench", row 27).

The relocations. A flat program is linked for no address: every place
where code or a table names something by its address has a relocation
in the file, which says where the four bytes are, in which object the
thing is and at which offset. The extender puts the address there when
it has loaded the objects. The linker leaves the offset in those four
bytes of the file, so the memory differs from the file in each of them,
by where the object named begins: code with an address in it is not in
the memory as it is in the file. A relocation whose four bytes lie
across the end of a page is in the file twice, once for each page; it
is one here. Code that takes the segment of something, which the
portable C has none of, has a selector there, with a relocation of two
bytes:
what the extender puts there is its own to say, and those bytes are as
the file has them here, and listed.

The map is the same text as that of a program of 16 bits, with offsets
of eight digits where that has four, and with an object's number in
front of the colon where that has a paragraph: Map reads both, and
coverage.link_map and coverage.segments read theirs through it.
"""
import re
import struct
import sys
from dataclasses import dataclass

# Where DOS/32A puts the first object of a program on the bench of the
# tests, and what it keeps for itself in front of each object.
BENCH = 0x00100010
IN_FRONT = 16
# What the portable C keeps of the extender's memory while a program
# that it has run is in the memory, in bytes: the blocks of its two
# panels, which it takes behind its objects, each as long as PANEL_LEAST
# of vc.h, the original's 64K and the 32 bytes in front of the panel.
# A panel's block grows while it reads a directory of more entries than
# that holds, 2,703, and shrinks to it again with one of fewer: a
# scenario that runs a program with such a directory in a panel would
# have it elsewhere, and there is none.
PANEL_LEAST = 32 + 0x10000
KEPT = (PANEL_LEAST, PANEL_LEAST)

# The header of a linear executable, as offsets in it. The tables are
# counted from the header, the pages from the beginning of the file.
PAGES, ENTRY, STACK, PAGE_SIZE, LAST_PAGE = 0x14, 0x18, 0x20, 0x28, 0x2C
OBJECTS, OBJECT_COUNT, PAGE_MAP = 0x40, 0x44, 0x48
RELOCATION_PAGES, RELOCATION_RECORDS, DATA_PAGES = 0x68, 0x6C, 0x80
# An object in the table: how long it is in the memory, the address that
# the linker gave it, which no one uses, its flags, the first of its
# pages in the map, counted from 1, and how many it has; a word unused.
OBJECT = "<6I"
EXECUTABLE = 0x0004
# A relocation: what kind of thing is put there and how the target is
# written. The kinds that Open Watcom's linker makes for a flat program
# with no library: an address of 32 bits, and a selector.
OFFSET32, SELECTOR = 7, 2
LENGTH = {OFFSET32: 4, SELECTOR: 2}
KIND, WIDE_OBJECT, WIDE_OFFSET = 0x0F, 0x40, 0x10


class Unknown(Exception):
    """The file has something that this reader was not written for."""


@dataclass(frozen=True)
class Object:
    """An object of the file."""
    number: int                   # as the map counts them, from 1
    size: int                     # how long it is in the memory
    flags: int
    at: int                       # where its bytes begin in the file
    held: int                     # how many of them the file has
    place: int                    # where it begins in the memory, from
                                  # where the first does

    @property
    def code(self):
        return bool(self.flags & EXECUTABLE)


@dataclass(frozen=True)
class Relocation:
    """A place that the extender writes an address to, or a selector,
    and what that names: each an object's number and an offset in it.
    A selector names an object and no offset. What an address names may
    lie in front of its object or behind it, as `Table - 0x10000` does:
    the offset is below 0 then, or beyond the object's length."""
    object: int
    offset: int
    length: int                   # 4 for an address, 2 for a selector
    target: int
    target_offset: int = 0

    @property
    def address(self):
        return self.length == LENGTH[OFFSET32]


class Linear:
    """A flat program, from the bytes of its file."""

    def __init__(self, contents):
        self.contents = contents
        if contents[:2] != b"MZ" or len(contents) < 0x40:
            raise Unknown("the file is no program of DOS")
        #: Where the stub ends in the file, and the linear executable
        #: begins.
        self.stub, = struct.unpack_from("<I", contents, 0x3C)
        if contents[self.stub:self.stub + 4] != b"LE\0\0":
            raise Unknown("the file has no linear executable (LE) behind "
                          "its stub")
        self.page = self._at(PAGE_SIZE)
        #: The entry of the program and the top of its stack, each an
        #: object's number and an offset.
        self.entry = self._at(ENTRY), self._at(ENTRY + 4)
        self.stack = self._at(STACK), self._at(STACK + 4)
        # Where the pages end in the file: the last of them is as long
        # as the header says. A file may have more behind them.
        ends = self._at(DATA_PAGES) + (self._at(PAGES) - 1) * self.page \
            + self._at(LAST_PAGE)
        if self._at(PAGES) and ends > len(contents):
            raise Unknown("the file ends before its last page does")
        self.objects = ()
        self._pages = []          # for each page of the map: its object
        place = 0                 # and where it begins in that
        for number in range(1, self._at(OBJECT_COUNT) + 1):
            size, _, flags, first, count, _ = struct.unpack_from(
                OBJECT, contents,
                self.stub + self._at(OBJECTS) + 24 * (number - 1))
            if first != len(self._pages) + 1:
                raise Unknown(f"the pages of object {number} do not follow "
                              "those of the one before")
            at = self._page(first) if count else 0
            for page in range(count):
                if self._page(first + page) != at + page * self.page:
                    raise Unknown(f"the pages of object {number} are not "
                                  "one behind the other in the file")
                self._pages.append((number, page * self.page))
            held = min(size, count * self.page, ends - at)
            self.objects += Object(number, size, flags, at, held, place),
            place = -(-(place + size) // IN_FRONT) * IN_FRONT + IN_FRONT
        #: The relocations, in the order of the places they are at.
        self.relocations = tuple(self._relocations())

    def _at(self, offset):
        return struct.unpack_from("<I", self.contents, self.stub + offset)[0]

    def _page(self, number):
        """Where the page `number` of the map is in the file. An entry of
        the map is the page's number among those of the file, its high
        byte first, and a byte of flags."""
        entry = self.stub + self._at(PAGE_MAP) + 4 * (number - 1)
        if self.contents[entry + 3]:
            raise Unknown(f"page {number} is not one that the file has "
                          "as it is")
        return self._at(DATA_PAGES) + self.page * (
            int.from_bytes(self.contents[entry:entry + 3], "big") - 1)

    def _relocations(self):
        contents = self.contents
        table = self.stub + self._at(RELOCATION_PAGES)
        records = self.stub + self._at(RELOCATION_RECORDS)
        found = {}
        for page, (number, begins) in enumerate(self._pages):
            at, end = (records + value for value in struct.unpack_from(
                "<2I", contents, table + 4 * page))
            while at < end:
                source, flags, offset = struct.unpack_from("<BBh", contents,
                                                           at)
                kind = source & KIND
                if kind not in LENGTH or source & ~KIND \
                        or flags & ~(WIDE_OBJECT | WIDE_OFFSET):
                    raise Unknown(f"a relocation of the kind {source:02X}h "
                                  f"{flags:02X}h, in object {number}")
                at += 4
                wide = "H" if flags & WIDE_OBJECT else "B"
                if kind == OFFSET32:
                    wide += "I" if flags & WIDE_OFFSET else "H"
                # An offset of 32 bits has a sign: the linker writes
                # what is in front of an object as counted round.
                wide = "<" + wide.replace("I", "i")
                target = struct.unpack_from(wide, contents, at)
                at += struct.calcsize(wide)
                # One that lies across the end of a page is listed for
                # the page behind it as well, from before that begins.
                one = Relocation(number, begins + offset, LENGTH[kind],
                                 *target)
                if found.setdefault((number, begins + offset), one) != one:
                    raise Unknown(f"two relocations at {begins + offset:X}h "
                                  f"of object {number}")
            if at != end:
                raise Unknown("the relocations of a page end in the middle "
                              "of one")
        return [found[where] for where in sorted(found)]

    def place(self, number, offset=0):
        """Where an offset of an object is in the memory, counted from
        where the first object begins: what an address of the map is in
        the image."""
        found = self.objects[number - 1]
        if not 0 <= offset <= found.size:
            raise ValueError(f"object {number} has no offset {offset:X}h")
        return found.place + offset

    def behind(self, base=BENCH, kept=()):
        """Where the extender puts the first object of a program that is
        started while this one is in the memory with its first object at
        `base`, and with blocks of the lengths `kept` taken behind its
        objects: see the top."""
        ends = base + self.place(len(self.objects), self.objects[-1].size)
        for size in kept:
            ends = -(-ends // IN_FRONT) * IN_FRONT + IN_FRONT + size
        return -(-ends // IN_FRONT) * IN_FRONT + IN_FRONT

    def copies(self, count, base=BENCH, kept=()):
        """Where the first object of each of `count` copies of this
        program is, each started while those before it are in the
        memory, and each keeping blocks of the lengths `kept` behind its
        objects meanwhile: `base` for the first."""
        found = [base]
        while len(found) < count:
            found.append(self.behind(found[-1], kept))
        return tuple(found)

    def in_file(self, number, offset=0):
        """Where an offset of an object is in the file, or None for what
        the file does not hold."""
        found = self.objects[number - 1]
        return found.at + offset if 0 <= offset < found.held else None

    def image(self, base=None):
        """The objects as they lie in the memory, from where the first
        begins: as the file has them, and with a `base`, the address of
        the first, as the extender leaves them, with each address that a
        relocation names put in. The bytes between two objects are the
        extender's and are zeros here."""
        image = bytearray(self.place(len(self.objects),
                                     self.objects[-1].size))
        for found in self.objects:
            image[found.place:found.place + found.held] = \
                self.contents[found.at:found.at + found.held]
        if base is not None:
            for one in self.relocations:
                if one.address:
                    struct.pack_into(
                        "<I", image, self.place(one.object, one.offset),
                        base + self.objects[one.target - 1].place
                        + one.target_offset & 0xFFFFFFFF)
        return bytes(image)

    def relocated(self):
        """The bytes of the image that are not in the memory as they are
        in the file: place -> the relocation there, which says how many
        they are and what they name."""
        return {self.place(one.object, one.offset): one
                for one in self.relocations}


def linear(contents):
    """The flat program whose file `contents` is, a Linear, or None for a
    file that is none: a program of 16 bits, the original. What reads
    both kinds of program asks this which it has."""
    try:
        return Linear(contents)
    except Unknown:
        return None


@dataclass(frozen=True)
class Segment:
    """A segment of the linker's map. `first` and `offset` are the two
    numbers of its address: a paragraph and an offset in a program of 16
    bits, an object and an offset in a flat one."""
    name: str
    kind: str                     # its class
    group: str
    first: int
    offset: int
    size: int


@dataclass(frozen=True)
class Symbol:
    """A symbol of the linker's map, with the file that has it and the
    two numbers of its address, as a Segment's. `mark` is what the map
    has behind the address: s for a static one, which another file may
    have under the same name, * for one that nothing names, + for one
    that only its own file does."""
    module: str
    name: str
    first: int
    offset: int
    mark: str


class Map:
    """A map of Open Watcom's linker, of a flat program or of one of 16
    bits: its segments and its symbols, in the order of the map. Static
    symbols are in it where the linker was told `option map=FILE,static`
    or `option static`."""

    def __init__(self, path):
        self.segments, self.symbols = [], []
        #: Whether it is of a flat program: the offsets have eight digits.
        self.flat = False
        module = None
        with open(path) as text:
            for line in text:
                found = re.match(
                    r"(\w+) +(\w+) +(\w+) +([0-9a-f]{4}):"
                    r"([0-9a-f]{4}|[0-9a-f]{8}) +([0-9a-f]{8})$", line)
                if found:
                    self.segments.append(Segment(
                        found[1], found[2], found[3], int(found[4], 16),
                        int(found[5], 16), int(found[6], 16)))
                    self.flat = len(found[5]) == 8
                found = re.match(r"Module: .*\((.*)\)$", line)
                if found:
                    module = re.split(r"[/\\]", found[1])[-1]
                    module = module.rsplit(".", 1)[0]
                found = re.match(r"([0-9a-f]{4}):([0-9a-f]{4}|[0-9a-f]{8})"
                                 r"([*+s]?) +(\S+)$", line)
                if found:
                    self.symbols.append(Symbol(
                        module, found[4], int(found[1], 16),
                        int(found[2], 16), found[3]))
        #: name -> the two numbers of its address, for every symbol that
        #: is not static.
        self.public = {symbol.name: (symbol.first, symbol.offset)
                       for symbol in self.symbols if symbol.mark != "s"}

    def segment(self, name):
        """The segment of that name."""
        for found in self.segments:
            if found.name == name:
                return found
        raise KeyError(f"the linker's map has no segment {name}")

    def named(self, name, module=None):
        """The symbols of that name, static ones too, as the compiler
        names them: `_Name` for data and `Name_` for a function of a flat
        program. With `module`, those of that file: `bars` for bars.c."""
        return [symbol for symbol in self.symbols if symbol.name == name
                and module in (None, symbol.module)]


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__)
    with open(sys.argv[1], "rb") as file:
        program = Linear(file.read())
    print(f"{sys.argv[1]}: {len(program.contents):,} bytes, of which the "
          f"stub is {program.stub:,}; entered at {program.entry[0]:04x}:"
          f"{program.entry[1]:08x}, the stack's top at "
          f"{program.stack[0]:04x}:{program.stack[1]:08x}")
    for found in program.objects:
        print(f"  object {found.number}, {'code' if found.code else 'data'}:"
              f" {found.size:,} bytes, {found.held:,} of them in the file "
              f"at {found.at:X}h; at {BENCH + found.place:08X}h on the "
              "bench")
    lengths = [one.length for one in program.relocations]
    print(f"  {lengths.count(4):,} addresses and {lengths.count(2):,} "
          "selectors that the extender puts in")
    if len(sys.argv) == 3:
        linked = Map(sys.argv[2])
        for found in linked.segments:
            print(f"  {found.name} of {found.kind}: {found.size:,} bytes "
                  f"at {BENCH + program.place(found.first, found.offset):08X}h")
        static = sum(symbol.mark == "s" for symbol in linked.symbols)
        print(f"  {len(linked.symbols):,} symbols, {static:,} of them "
              "static")


if __name__ == "__main__":
    main()
