"""What the portable C has in its memory at the end of a scenario, kept
and held against what another build has there: the data's image.

    python3 harness/dataimage.py save PROGRAM DIRECTORY [SCENARIO ...]
                                 [--early]
    python3 harness/dataimage.py compare BEFORE AFTER [SCENARIO ...]
                                 [--changed NAME ...] [--all]
    python3 harness/dataimage.py show DIRECTORY SCENARIO
    python3 harness/dataimage.py check DIRECTORY

It is the net under a change that is to leave the program doing what it
did, where the bytes of two builds no longer say so: a change of how the
program addresses its memory moves code and data about, and what the
program has put where, by the end of a scenario, is to be the same all
the same. A write that went astray is in the image, whether or not a
screen shows it.

Two commands, for a piece of work: `save` with the build that it starts
from and a directory of `build/`, before anything is changed, and later
`compare` of that directory with `build/portable_c/images`, which the
scenarios' test writes of the build that it judges (harness/ported.py).
`save` takes the program through every scenario of its tree, or those
named, and holds it against no golden. The test's own images can be kept
in place of the first command, with `cp -r build/portable_c/images
build/before`, if the test has just run on the build that the work
starts from. The images are of a build and are large, 760 KB each before
they are packed and some 4 MB packed for all scenarios: they are kept
under `build/` and never committed.

When it is taken. The driver of the tests resets the machine when it has
written the screen of the last step, which it does when the program
waits for a key with nothing left to do: QEMU is told to pause a machine
that would end (dos.Machine.linger), and the memory is read there, with
QEMU's `pmemsave`. That is the same instruction in every run, since the
machine's clock is the count of what it has executed. A scenario that
did not get to its end has no image.

What an image is: stretches of the machine's memory, each with a name,
its address and its length, and the facts of the build next to them
(`build.json`: the linker's map of the program's data, and the
structures of the original's listing, by which a place is named). Today
it is one stretch, the program's data as the linker has it in one
object: the stack, the tables, the variables with a value, the main
data, and the variables without one, among them the 640 KB that the
program hands out to itself (`Own` of dos32.c). The tool makes of it
what it compares:

  - the main data, `md`, by the field of the original's `MainDat` that
    a byte is in, and of `ViewDat`, `EdData` and the other structures
    that the program lays over its buffers;
  - each block of the memory that the program was handed, by its number
    in the order of the memory, with its length and whether it is
    taken. What is compared of a block that is taken is what the
    program has there, by name: each panel, with the field of `WCBdef`
    or the entry of the list of files and its field for a byte, and
    what is behind the panels, the text of the viewer or the editor, a
    list, a buffer. ADDRESSES says where those begin, and each runs to
    the next or to the end of its block: so they are the same things
    whether the program cuts them out of one block or has a block for
    each. Where two builds have one of them of two lengths, it is held
    as far as both have it, and that is said;
  - every variable that the linker's map names, by the file that has it
    and its name;
  - the tables, CONST2, which a program does not write: they are held
    against the program's own file as the extender loads it, when the
    image is taken, so a write into them shows with no second build.

Where the blocks are. The tool learns that from the image itself and the
map: the main data at `_md`; the blocks by what dos32.c keeps of them.
Since the memory comes from the extender, a block lies wherever the
extender put it and is a stretch of its own in the image: taken by the
same `pmemsave` at the address and with the length that the table of
dos32.c has, `Blocks`, which the map names and `_handed` below reads,
with the extender's 16 bytes in front of it. A build from before that,
which handed out 640 KB of its own, `Own`, has its blocks inside the
program's data: by a table beside `Own`, `OwnBlocks`, and in a build
from before that table by a paragraph in front of each block. The
blocks are compared block by block whichever they are, and an address
is compared by the block that it is in and how far.

What differs by design, and is not compared as bytes:

  - A word that holds an address, ADDRESSES: a segment of the main data,
    or a pointer beside it. Two builds have another number there
    whenever the linker has put the program's memory elsewhere. It is
    compared by what it names: the block, the variable or the field,
    and how far into it.
  - A variable of four bytes, or of a multiple, whose words differ and
    name the same place in both builds by the maps: a pointer. That is
    said in the summary, and is no difference. A word that names
    another place in one build than in the other is one.
  - The stack: it holds return addresses and whatever the deepest call
    left, which is another thing in every build.
  - UNSTEADY, by name with the reason: what follows the time, which is
    counted in instructions, of which two builds take different numbers
    to the same step. Whether a thing follows the time, one build says
    by itself: `save --early` has the driver of the tests leave the
    stack unwritten, so that the same program gets to every step some
    instructions earlier, and what then differs from its images saved
    without that follows the time. Nothing else differs there.
  - What a piece of work declares, `--changed NAME`: a field of the main
    data, a structure that lies over one, as `EdData`, a variable as
    `file.Name`, a thing in a block as `the panel of WCB1seg`, or `the
    blocks` for how many there are and how long each is. It is
    compared, and what differs there is counted and not held against
    the piece.

The bytes round a block. The extender keeps 16 bytes of its own in front
of every block and ends a program that has written into them, at its
next call for memory; the program's own hand-out of memory, before the
memory was the extender's, kept 16 bytes of a pattern, GUARD below, in
front of each block that is taken and behind the last block. Either is
looked at when an image is taken, by guards(): a scenario whose image
has them otherwise than the extender or the pattern has them fails the
scenarios' test, with the block and the place, and `save` and `compare`
say the same. What the extender has there is EXTENDER below. In a
build from before dos32.c wrote the pattern, which is known by its
having no table of its blocks, nothing is looked at.

`compare` prints, for every scenario that differs, each run of bytes
with its name, and ends with what was held and how much. It fails if
anything differs that is not of the kinds above, or if a scenario has an
image on one side only. `check` holds a directory against itself and
against copies of its images with single bytes changed, each of which
has to be named: that the net catches something is tried, not taken for
granted.
"""
import argparse
import bisect
import hashlib
import json
import re
import struct
import sys
import zlib
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, replace
from pathlib import Path

# Where the scenarios' test keeps the images of the build that it
# judges, next to the program.
DIRECTORY = "images"
FACTS = "build.json"
SUFFIX = ".image"

# The stretch of an image that is the program's data.
PROGRAM = "the program's data"

# What the program is linked of that the tool knows by name.
MAIN = "_md"                      # the main data
OWN = ("dos32", "_Own")           # the memory that the program hands out
# What dos32.c keeps of the blocks of that memory, in a build that has
# a table for it: the table, and how many blocks it has. An entry is the
# paragraph of the megabyte that the block begins in, its paragraphs,
# and whether it is taken, a word each.
OWN_BLOCKS = ("dos32", "_OwnBlocks")
OWN_HELD = ("dos32", "_OwnHeld")
OWN_ENTRY = "<HHH"
OWN_MOST = 8                      # as many as the table has room for
# What dos32.c keeps of the blocks that the extender gave it, since the
# memory is the extender's: the table, Blocks, and how many it has,
# BlocksHeld. An entry is where the block begins, how many bytes it has
# and the extender's name for it, four bytes each. Only the blocks that
# are taken are in it.
BLOCKS = ("dos32", "_Blocks")
BLOCKS_HELD = ("dos32", "_BlocksHeld")
BLOCKS_ENTRY = "<IiI"
BLOCKS_MOST = 8
STACK = "STACK"                   # segments of the map
TABLES = ("CONST", "CONST2")
VARIABLES = ("_DATA", "_BSS")
# A megabyte that segments count in begins this far below the main data
# (ARENA of vc.h).
ARENA = 0x10000
PARAGRAPH = 16

# The structures of the original's listing: the main data, those that
# lie over its buffers at offsets of their own, a panel and an entry of
# its list of files.
MAIN_DATA = "MainDat"
LAID_OVER = ("ViewDat", "EdData", "FndData", "HlpData")
PANEL, ENTRY, LIST = "WCBdef", "FILENT", "DskBuf"
# A field that the portable C has under another name than the original,
# since it is another thing there: structure -> the original's name ->
# the name in vc.h, which is the one that is printed.
RENAMED = {MAIN_DATA: {"FreMem1": "MemSeg"}}


@dataclass(frozen=True)
class Address:
    """A word that holds an address. `field` is the field of the main
    data that has it, as `kind` says: a SEGMENT, a paragraph of the
    megabyte; PARAGRAPHS from the main data on; or a PLACE of the main
    data, bytes from where it begins, of which 0 is no place: the word
    was never set, or says that there is none, as a pointer that is 0
    does. `variable` is the
    symbol of the map that has it as a pointer, in a build that has
    one: a build is asked for the variable first, so that a build in
    which the word has left the main data and the build before it are
    held against each other. `cut` is what the program has from there
    on in a block, for naming a byte: PANEL, TEXT, or None."""
    name: str
    field: str = None
    kind: str = None
    variable: tuple = None        # (file, symbol)
    cut: str = None


SEGMENT, PARAGRAPHS, PLACE = "segment", "paragraphs", "place"
TEXT = "behind the panels"

# The piece that takes a word out of the main data names its variable
# here, next to the field.
ADDRESSES = (
    # The panels: pointers beside the main data since a panel is one,
    # LeftPanel, RightPanel, CurrentPanel and OtherPanel of main.c. They
    # keep the names of the original's words here, by which a thing in
    # a block is named.
    Address("WCB1seg", "WCB1seg", SEGMENT, ("main", "_LeftPanel"),
            cut=PANEL),
    Address("WCB2seg", "WCB2seg", SEGMENT, ("main", "_RightPanel"),
            cut=PANEL),
    Address("WCBcrn", "WCBcrn", SEGMENT, ("main", "_CurrentPanel")),
    Address("WCBsec", "WCBsec", SEGMENT, ("main", "_OtherPanel")),
    # Where the room begins and where it ends: beside the main data
    # since the program has three blocks where it had one to cut. The
    # word MemSeg, the block that all of it was in, names nothing since
    # then and is no address: its field is not set, and is held as the
    # bytes that it is.
    Address("ViewSeg", "ViewSeg", SEGMENT, ("main", "_Room"), cut=TEXT),
    Address("FreMem2", "FreMem2", PARAGRAPHS, ("main", "_RoomEnd")),
    # The places of the main data that it kept in words of itself:
    # pointers beside it since a place is one. The tree's cursor, its
    # first line shown and the current directory in it, which is none
    # where the tree was written without it; where the history is; and
    # the command to run next.
    Address("TreAdr", "TreAdr", PLACE, ("main", "_TreeCursor")),
    Address("TreFrst", "TreFrst", PLACE, ("main", "_TreeFirst")),
    Address("TreCrnt", "TreCrnt", PLACE, ("main", "_TreeCurrent")),
    Address("HstAdr", "HstAdr", PLACE, ("main", "_HistoryPlace")),
    Address("BatAdr", "BatAdr", PLACE, ("main", "_NextCommand")),
)

# The variables that have such a word, as Build.variables names them:
# each is held by what it names, as the field was, and not as bytes.
POINTERS = {"{}.{}".format(one.variable[0], one.variable[1].lstrip("_"))
            for one in ADDRESSES if one.variable}

# What two builds may have otherwise with no fault in either, each with
# its reason. See "What differs by design" at the top; TODO.md, "F0,
# the net and the record", has how each was found, and how another is.
# Of the main data: the field, how far into it, how many bytes, None
# for all of it.
TIME = "follows the time, which two builds reach a step at differently"
UNSTEADY = (
    ("Rand1", 0, None, "a random number, stirred by the timer: " + TIME),
    ("Rand2", 0, None, "the other random number"),
    ("Stars", 0, None, "the blanker's stars, put where the random "
                       "numbers say"),
    ("MousTim", 0, None, "the timer's count when the mouse was last "
                         "looked at: " + TIME),
    ("DTA", 22, 4, "the time and the date of the file that DOS last "
                   "found, which a scenario may have made: " + TIME),
    # Found by F1, against a build that took thousands of instructions
    # more to a step, which `save --early` does not: read in Input of
    # input.c.
    ("ScrBuf", 152, 1, "the separator of the clock, at the end of the "
                       "first row of the screen as the program keeps "
                       "it: a colon in the second half of a second and "
                       "a space in the first. " + TIME),
    ("ScrBuf", 154, 1, "the same, of a clock of 24 hours"),
)
# Of the variables, by name.
UNSTEADY_VARIABLES = {
    "dos32.OwnFirst": "the paragraph of the megabyte that the program's "
                      "own memory begins in: by where the linker has it",
    # Found by F2, whose four pointers are 16 bytes in front of the
    # memory that the program hands out.
    "dos32.OwnBlocks": "the paragraphs of the megabyte that the blocks "
                       "begin in: by where the linker has the memory. "
                       "How many blocks there are, how long each is "
                       "and which are taken is held as `the blocks`",
    # Found by F9, against a build whose objects are a paragraph longer.
    "dos32.Blocks": "where the extender gave each block, and its name "
                    "for it: by where it has the program's objects. How "
                    "many blocks there are and how long each is is held "
                    "as `the blocks`",
    "dos32.Below": "the stack pointer at the last call of DOS or of "
                   "the extender: by the frames of the build's "
                   "functions, as the stack is, which is left out",
}
# Of a panel's list: the field of an entry.
UNSTEADY_ENTRY = {
    "TimeF": "the time of a file, which a scenario may have made: " + TIME,
}
COPIED = "the same bytes as the time of a file in a panel's list, in a " \
         "copy of that list"

# Found by F2 at DTA+41 and by F5 at DTA+42. The record of a search
# comes up from DOS's memory, 43 bytes of it, and behind the zero of a
# name that is shorter than twelve characters is what that memory held
# before. What was seen: such a byte is one value in one build and
# another in the next, in every scenario that has it, and the files of
# the two builds have F2's pair at one offset and nowhere else, 6E24h,
# in the table that says where the relocations of each page begin. What
# is inferred from that, and was not looked at in DOS's memory: that it
# is a byte of the program's own file, left there by the extender's
# loading of it, so that another build has another. The Russian bench
# has another byte there behind DTA+42, which F5 did not find in the
# files. The program copies the name as 13 bytes, into the entries of
# a panel's list and into its buffers. So a byte may differ where it is
# behind the zero of a name of 13 bytes, the name and its zero being
# the same in both: in the DTA's name, in NameF of an entry of a
# panel's list, or in 13 bytes that are, in each build, the same as
# one of those fields.
NAME_TAIL = "a byte behind the zero of a name that DOS found, of what " \
            "DOS's memory held before: by what was seen, and inferred " \
            "from it, a byte of the program's file, left by the " \
            "extender's loading of it, which follows the build"
NAME_SIZE = 13
DTA_NAME = 30                   # where the name is in the DTA


# The bytes that the program keeps round a block of the memory that it
# was handed, where the extender will keep 16 of its own: in front of
# each block that is taken, and behind the last block of the memory. A
# write just in front of a block or just behind its end is in them. The
# image's tool holds them when an image is taken, and a scenario whose
# image has them otherwise fails the scenarios' test, with the block
# and the place: guards(). The pattern is here as bytes, as Guard() of
# dos32.c writes it, and _guarded() says where it is, by the blocks of
# _blocks().
GUARD = bytes(0xF0 - at for at in range(16))   # Guard() of dos32.c

# What DOS/32A keeps in front of a block that it has given, as was seen
# on the bench (TODO.md, "X, the extender's memory on the bench", row
# 5): four words of four bytes, 12345678h, the block's length rounded up
# to 16 with the highest bit set while it is taken, 0, and 12345678h
# again. The image has them as the first 16 bytes of the stretch that
# it takes of a block.
IN_FRONT = 16
EXTENDER_MARK = 0x12345678
TAKEN_BIT = 0x80000000


def _in_front(size):
    """The 16 bytes that the extender has in front of a block of `size`
    bytes that is taken."""
    return struct.pack("<IIII", EXTENDER_MARK,
                       (-(-size // PARAGRAPH) * PARAGRAPH) | TAKEN_BIT, 0,
                       EXTENDER_MARK)


class Spoilt(Exception):
    """An image, or the facts of its build, is not what this reads."""


# The original's listing.

def structures(listing):
    """name -> (its length, [(field, offset)] in the order of the
    offsets) for every structure of the listing of the original."""
    text = Path(listing).read_text(encoding="cp866")
    begins = text.index("Structures and Unions:")
    found, fields = {}, None
    for line in text[begins:].splitlines()[1:]:
        if line.startswith("Segments and Groups"):
            break
        one = re.match(r"(\w+) [ .]+ +([0-9A-F]+)$", line)
        if one:
            fields = []
            found[one[1]] = (int(one[2], 16), fields)
            continue
        one = re.match(r"  (\w+) [ .]+ +([0-9A-F]+) +\S", line)
        if one and fields is not None:
            fields.append((one[1], int(one[2], 16)))
    for name, (size, fields) in found.items():
        fields[:] = sorted(((RENAMED.get(name, {}).get(field, field), at)
                            for field, at in fields),
                           key=lambda field: field[1])
    return found


# Taking an image.

def facts(program):
    """What is kept of a build next to its images: where the program's
    data is on the bench and what the linker's map has there, and the
    structures of the original's listing."""
    import le
    import setups
    flat = le.linear(program.contents)
    if not flat:
        raise Spoilt(f"{program.filename} is no flat program: an image "
                     "is of the portable C")
    linked = le.Map(program.linked)
    number = linked.public[MAIN][0]
    return {
        "program": hashlib.sha256(program.contents).hexdigest(),
        "object": number,
        "address": le.BENCH + flat.place(number),
        "size": flat.objects[number - 1].size,
        "objects": [[le.BENCH + flat.place(index + 1), found.size]
                    for index, found in enumerate(flat.objects)],
        "segments": [[segment.name, segment.first, segment.offset,
                      segment.size] for segment in linked.segments],
        "symbols": [[symbol.module, symbol.name, symbol.first,
                     symbol.offset] for symbol in linked.symbols],
        "structures": structures(setups.LISTING),
    }


class Taker:
    """Takes the image of `program` from a machine that lingers at the
    end of a scenario: what scenarios.run() is handed as `image`."""

    def __init__(self, program):
        import le
        self.facts = facts(program)
        flat = le.linear(program.contents)
        at = flat.place(self.facts["object"])
        #: The program's data as the extender has loaded it.
        self._loaded = flat.image(le.BENCH)[at:at + self.facts["size"]]
        self._build = Build(known=self.facts)

    def __call__(self, machine):
        known = self.facts
        data = machine.memory(known["address"], known["size"])
        stretches = [(PROGRAM, known["address"], data)]
        for name, address, size in _handed(self._build, data):
            stretches.append((name, address, machine.memory(address, size)))
        image = Image(stretches,
                      _tables_written(known, data, self._loaded))
        try:
            image.guards = guards(Read(self._build, image))
        except Spoilt as spoilt:
            image.guards = [str(spoilt)]
        return image


def _handed(build, data):
    """The blocks that the program was handed and that are no part of
    its own data: (name, address, length) of each, for the image to
    take, with the extender's 16 bytes in front of it. dos32.c keeps a
    table of them, Blocks, which the map names, and this reads it in
    `data`. There are none in a build that hands out memory of its own,
    the blocks being inside `Own`."""
    table, held = build._symbol(*BLOCKS), build._symbol(*BLOCKS_HELD)
    if table is None or held is None:
        return []
    count = int.from_bytes(data[held:held + 4], "little", signed=True)
    if not 0 <= count <= BLOCKS_MOST:
        raise Spoilt(f"dos32.c's table of its blocks says that it has "
                     f"{count} of them")
    found = []
    for number in range(count):
        address, size, _ = struct.unpack_from(
            BLOCKS_ENTRY, data, table + number * struct.calcsize(BLOCKS_ENTRY))
        if size <= 0 or address < IN_FRONT:
            raise Spoilt(f"dos32.c's table has block {number} where it "
                         "cannot be")
        found.append((f"block {number}", address - IN_FRONT,
                      size + IN_FRONT))
    return found


def _tables_written(known, data, loaded):
    """Where the tables are not as the program's file has them, loaded:
    [(offset in the program's data, length)]."""
    found = []
    for name, first, offset, size in known["segments"]:
        if name in TABLES and first == known["object"]:
            found += [(offset + at, length) for at, length
                      in runs(data[offset:offset + size],
                              loaded[offset:offset + size])]
    return found


@dataclass
class Image:
    """What was taken of a machine: [(name, address, bytes)], and where
    the tables had been written into."""
    stretches: list
    written: list
    # Where the pattern round a block was not the pattern, as text: see
    # GUARD.
    guards: tuple = ()

    def write(self, path):
        head = {"stretches": [[name, address, len(data)]
                              for name, address, data in self.stretches],
                "written": self.written, "guards": list(self.guards)}
        packed = zlib.compress(b"".join(data for _, _, data
                                        in self.stretches), 1)
        Path(path).write_bytes(json.dumps(head).encode() + b"\n" + packed)

    @classmethod
    def read(cls, path):
        head, _, packed = Path(path).read_bytes().partition(b"\n")
        head = json.loads(head)
        data, at, stretches = zlib.decompress(packed), 0, []
        for name, address, size in head["stretches"]:
            stretches.append((name, address, data[at:at + size]))
            at += size
        if at != len(data):
            raise Spoilt(f"{path} holds {len(data)} bytes and says {at}")
        return cls(stretches, [tuple(one) for one in head["written"]],
                   tuple(head.get("guards", ())))


def write_facts(directory, known):
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    for stale in directory.glob(f"*{SUFFIX}"):
        stale.unlink()
    (directory / FACTS).write_text(json.dumps(known))


def runs(one, other):
    """Where two stretches of bytes of one length differ: [(offset,
    length)] of each run of bytes that do."""
    if one == other:
        return []
    found, at, size = [], 0, len(one)
    step = 4096
    while at < size:
        if one[at:at + step] == other[at:at + step]:
            at += step
            continue
        end = min(at + step, size)
        while at < end:
            if one[at] == other[at]:
                at += 1
                continue
            begins = at
            while at < size and one[at] != other[at]:
                at += 1
            if found and found[-1][0] + found[-1][1] == begins:
                found[-1] = (found[-1][0], at - found[-1][0])
            else:
                found.append((begins, at - begins))
    return found


# Reading an image as what the program has.

@dataclass
class Part:
    """A part of an image that is compared as bytes with the part of the
    same name of another: its bytes, where they were in the machine,
    and what names a byte of it."""
    name: str
    address: int
    data: bytes
    kind: str                     # "the main data", "a block", "a variable"
    namer: object = None          # offset -> text, or None


class Build:
    """The facts of a build, as its images are read by them."""

    def __init__(self, directory=None, known=None):
        """Of a directory of images, or of the facts in hand, `known`."""
        if known is None:
            self.directory = Path(directory)
            path = self.directory / FACTS
            if not path.exists():
                raise Spoilt(f"{directory} has no {FACTS}: it is no "
                             "directory of images")
            known = path.read_text()
        else:
            # As they are when they have been written and read.
            known = json.dumps(known)
        known = self.known = json.loads(known)
        self.address, self.size = known["address"], known["size"]
        number = known["object"]
        self.segments = {name: (offset, size) for name, first, offset, size
                         in known["segments"] if first == number}
        # Every symbol by its address in the machine, for naming one.
        self.symbols = sorted(
            (known["objects"][first - 1][0] + offset, module or "", name)
            for module, name, first, offset in known["symbols"])
        self.main = self._symbol(None, MAIN)
        self.main_size = self.segments["MAINDATA"][1]
        self.structures = known["structures"]
        if self.structures[MAIN_DATA][0] != self.main_size:
            raise Spoilt("the main data is not as long as the original's: "
                         "the tool names its fields by the original's "
                         "listing")
        self.fields = dict(self.structures[MAIN_DATA][1])
        self.variables = self._variables()

    def _symbol(self, module, name):
        """Where a symbol is in the program's data, or None."""
        for address, has, called in self.symbols:
            if called == name and module in (None, has):
                return address - self.address
        return None

    def _variables(self):
        """name -> (offset in the program's data, length) of everything
        that the map names in the segments of the variables: a variable
        is as long as it is to the next symbol or the segment's end.
        What stands in front of a segment's first symbol has the
        segment's name."""
        found, taken = {}, Counter()
        marks = sorted({address - self.address
                        for address, _, _ in self.symbols})
        named = {}
        for address, module, name in self.symbols:
            named.setdefault(address - self.address, (module, name))
        for segment in VARIABLES:
            begins, size = self.segments[segment]
            inside = [at for at in marks if begins <= at < begins + size]
            edges = inside + [begins + size]
            if not inside or inside[0] != begins:
                found[f"({segment})"] = (begins, edges[0] - begins)
            for at, end in zip(inside, edges[1:]):
                if at == self.main or named[at] == OWN:
                    continue
                name = "{}.{}".format(*named[at]).replace("._", ".", 1)
                taken[name] += 1
                if taken[name] > 1:
                    name += f"#{taken[name]}"
                found[name] = (at, end - at)
        return found

    def images(self):
        return sorted(path.name[:-len(SUFFIX)]
                      for path in self.directory.glob(f"*{SUFFIX}"))

    def read(self, name):
        return Read(self, Image.read(self.directory / (name + SUFFIX)))

    def field_name(self, offset):
        """A byte of the main data by its field, and by the fields of
        the structures that lie over it there."""
        said = _field(self.structures[MAIN_DATA][1], offset)
        over = []
        for name in LAID_OVER:
            size, fields = self.structures[name]
            if fields and fields[0][1] <= offset < size:
                over.append(f"{name}.{_field(fields, offset)}")
        return said + (f" ({', or '.join(over)})" if over else "")


def _field(fields, offset):
    name, at = max((field for field in fields if field[1] <= offset),
                   key=lambda field: field[1], default=("", 0))
    return name + (f"+{offset - at}" if offset != at or not name else "")


class Read:
    """An image, read by the facts of its build."""

    def __init__(self, build, image):
        self.build, self.image = build, image
        name, address, self.data = image.stretches[0]
        if name != PROGRAM or address != build.address \
                or len(self.data) != build.size:
            raise Spoilt("the image is not of the build whose facts are "
                         "next to it")
        # The blocks that the extender gave, each a stretch of its own:
        # offset in the program's data -> (the extender's 16 bytes in
        # front of it, the block's bytes).
        self.handed = {
            stretch_address + IN_FRONT - build.address:
                (stretch[:IN_FRONT], stretch[IN_FRONT:])
            for _, stretch_address, stretch in image.stretches[1:]}
        self.blocks = self._blocks()
        self.addresses = {one.name: self._address(one) for one in ADDRESSES}
        self.things = self._things()

    def word(self, offset, size=2):
        return int.from_bytes(self.data[offset:offset + size], "little")

    def span(self, begins, size):
        """The bytes of the image from `begins`, an offset in the
        program's data, as far as `size`: in the program's data, or in a
        block that the extender gave, which may lie anywhere."""
        for at, (_, data) in self.handed.items():
            if at <= begins < at + len(data):
                return data[begins - at:begins - at + size]
        return self.data[max(begins, 0):begins + size] if begins >= 0 \
            else b""

    def _blocks(self):
        """The blocks of the memory that the program was handed, in
        their order: (offset in the program's data, length, taken).
        dos32.c keeps them in a table, which the map names: of the
        blocks that the extender gave, each a stretch of the image, or
        of the memory that the program handed out to itself, which a
        build from before it had the table keeps in a paragraph in front
        of each block: whether a block follows, whether this one is
        taken, and its paragraphs."""
        build = self.build
        if build._symbol(*BLOCKS) is not None:
            found = []
            for begins, (_, data) in self.handed.items():
                found.append((begins, len(data), True))
            return found
        own = build._symbol(*OWN)
        if own is None:
            raise Spoilt("the map has no {1} of {0}.c: where the blocks "
                         "are that the program was handed is read there"
                         .format(*OWN))
        arena = build.main - ARENA
        table, held = build._symbol(*OWN_BLOCKS), build._symbol(*OWN_HELD)
        if table is not None and held is not None:
            count = self.word(held, 4)
            if count > OWN_MOST:
                raise Spoilt(f"dos32.c's table of its blocks says that it "
                             f"has {count} of them")
            found = []
            for number in range(count):
                block, paragraphs, taken = struct.unpack_from(
                    OWN_ENTRY, self.data,
                    table + number * struct.calcsize(OWN_ENTRY))
                at = arena + block * PARAGRAPH
                # The first a paragraph behind where the memory begins,
                # rounded up to one; each a paragraph behind the last.
                if at != sum(found[-1][:2]) + PARAGRAPH if found \
                        else not own < at <= own + 2 * PARAGRAPH:
                    raise Spoilt(f"dos32.c's table has block {number} "
                                 "where it cannot be")
                found.append((at, paragraphs * PARAGRAPH, bool(taken)))
            return found
        at = arena + (own + PARAGRAPH - 1 - arena) // PARAGRAPH * PARAGRAPH
        found = []
        while True:
            kind, taken, paragraphs = struct.unpack_from("<cBH", self.data,
                                                         at)
            if kind not in (b"M", b"Z"):
                if not found and kind == b"\0":
                    return found  # the program never set its memory up
                raise Spoilt(f"what dos32.c keeps in front of block "
                             f"{len(found)} is spoilt")
            found.append((at + PARAGRAPH, paragraphs * PARAGRAPH,
                          bool(taken)))
            at += PARAGRAPH + paragraphs * PARAGRAPH
            if kind == b"Z":
                return found

    def _things(self):
        """What the program has in the blocks that are taken, by
        ADDRESSES: (offset in the program's data, length, name, the
        Address that says where it begins, or None). A panel and what
        is behind the panels each run to the next of them or to the end
        of their block, so they are the same things whether the program
        has cut them out of one block or has a block for each; what a
        block has in front of the first of them, or a block with none,
        has the block's number for its name."""
        build = self.build
        marks = sorted(((self.addresses[one.name][0] - build.address, one)
                        for one in ADDRESSES if one.cut),
                       key=lambda mark: mark[0])
        found = []
        for number, (begins, size, taken) in enumerate(self.blocks):
            if not taken:
                continue
            inside = [mark for mark in marks
                      if begins <= mark[0] < begins + size]
            edges = [at for at, _ in inside] + [begins + size]
            if edges[0] != begins:
                found.append((begins, edges[0] - begins, f"block {number}",
                              None))
            for (at, one), end in zip(inside, edges[1:]):
                found.append((at, end - at, TEXT if one.cut == TEXT
                              else f"the panel of {one.name}", one))
        return found

    def _address(self, one):
        """What the build has for an Address: its number, in the
        machine's memory, and where the word is, the field's offset in
        the main data or the variable's name."""
        build = self.build
        if one.variable:
            at = build._symbol(*one.variable)
            if at is not None:
                return self.word(at, 4), "{}.{}".format(*one.variable)
        at = build.main + build.fields[one.field]
        value = self.word(at)
        if one.kind == PLACE:
            if value == 0:              # no place: as a pointer that is 0
                return 0, build.fields[one.field]
            address = build.main + value
        elif one.kind == SEGMENT:
            address = build.main - ARENA + PARAGRAPH * value
        else:
            address = build.main + PARAGRAPH * value
        return build.address + address, build.fields[one.field]

    def place(self, address):
        """An address of the machine's memory as what it names, which is
        the same in two builds that have the same thing there: (what,
        how far into it), or None for an address of nothing known."""
        build = self.build
        if address == 0:
            return "nothing", 0         # a pointer to nothing
        at = address - build.address
        if 0 <= at - build.main < build.main_size:
            return "the main data", at - build.main
        for begins, size, name, _ in self.things:
            if begins <= at < begins + size:
                return name, at - begins
        # The end of a thing is a place of it too: where it ends.
        for begins, size, name, _ in self.things:
            if at == begins + size:
                return name, size
        for number, (begins, size, taken) in enumerate(self.blocks):
            if begins <= at <= begins + size:
                return f"block {number}, which is free", at - begins
        for name, (begins, size) in build.variables.items():
            if begins <= at < begins + size:
                return name, at - begins
        # A table, or code: by the symbol of the map that it is behind.
        if any(begins <= address < begins + size
               for begins, size in build.known["objects"]):
            index = bisect.bisect_right(build.symbols, (address, "~", ""))
            if index:
                found = build.symbols[index - 1]
                return f"{found[1]}.{found[2]}", address - found[0]
        return None

    def parts(self):
        """The Parts of the image, by name."""
        build = self.build
        found = {"the main data": Part(
            "the main data", build.address + build.main,
            self.data[build.main:build.main + build.main_size],
            "the main data", build.field_name)}
        for begins, size, name, one in self.things:
            found[name] = Part(name, build.address + begins,
                               self.span(begins, size), "a block",
                               self._namer(one))
        for name, (begins, size) in build.variables.items():
            if name in POINTERS:        # held by what it names
                continue
            found[name] = Part(name, build.address + begins,
                               self.data[begins:begins + size],
                               "a variable")
        return found

    def _namer(self, one):
        """What names a byte of the thing that the Address `one` says
        the place of: of a panel, the field of its structure, or the
        entry of its list of files and the field of that."""
        if one is None or one.cut != PANEL:
            return None
        _, fields = self.build.structures[PANEL]
        entry, entry_fields = self.build.structures[ENTRY]
        # The list of files begins inside the structure, and runs on
        # behind it for as far as the panel has files.
        size = dict(fields)[LIST]

        def name(offset):
            if offset < size:
                return _field(fields, offset)
            number, rest = divmod(offset - size, entry)
            return (f"entry {number} of its list, "
                    f"{_field(entry_fields, rest)}, +{offset}")
        return name


# The pattern round a block.

def _guarded(read):
    """Where the pattern is to be in the Read's image: (offset in the
    program's data, what it is said to be) for the bytes in front of
    each block that is taken and for those behind the last block of
    the memory. Of a build whose blocks the extender gave, none: see
    _extender()."""
    if read.handed:
        return []
    found = [(begins - len(GUARD or b""), f"in front of block {number}")
             for number, (begins, _, taken) in enumerate(read.blocks)
             if taken]
    if read.blocks:
        begins, size, _ = read.blocks[-1]
        found.append((begins + size,
                      f"behind block {len(read.blocks) - 1}, the last"))
    return found


def guards(read, pattern=None, places=None):
    """Where the bytes round a block are not the pattern, as lines of
    text, each with the block and the first byte that is another. None
    is looked for in a build that has no table of its blocks, which is
    one from before dos32.c wrote the pattern: see GUARD."""
    if not pattern and read.build._symbol(*BLOCKS) is not None:
        return _extender(read)
    if not pattern and read.build._symbol(*OWN_BLOCKS) is None:
        return []                 # a build from before there was one
    pattern = pattern or GUARD
    if not pattern:
        return []
    found = []
    for at, said in places or _guarded(read):
        has = read.data[max(at, 0):at + len(pattern)] if at >= 0 else b""
        if has == pattern:
            continue
        if len(has) != len(pattern):
            found.append(f"the {len(pattern)} bytes {said} are not in "
                         "the image")
            continue
        first = next(index for index in range(len(pattern))
                     if has[index] != pattern[index])
        found.append(
            f"the {len(pattern)} bytes {said} are not the pattern, from "
            f"the byte at +{first} of them on: {_hex(has)}, where the "
            f"pattern is {_hex(pattern)}. Something has written "
            + ("in front of the block, or behind the end of the one "
               "before it" if "front" in said else
               "behind the end of the block")
            + ", or the block was not taken as dos32.c takes one")
    return found


def _extender(read):
    """Where what the extender keeps in front of a block is not as it
    has it, EXTENDER: as lines of text, each with the block and the
    first byte that is another."""
    found = []
    for number, (begins, (front, data)) in enumerate(read.handed.items()):
        expected = _in_front(len(data))
        if front == expected:
            continue
        first = next(index for index in range(len(expected))
                     if index >= len(front) or front[index] != expected[index])
        found.append(
            f"the extender's {IN_FRONT} bytes in front of block {number} "
            f"are not as it has them, from the byte at +{first} of them "
            f"on: {_hex(front)}, where it has {_hex(expected)}. Something "
            "has written in front of the block, or behind the end of the "
            "one before it, and the extender ends the program for that "
            "at its next call for memory")
    return found


# Holding one image against another.

@dataclass
class Difference:
    part: str
    said: str                     # where, and what each side has
    declared: bool = False        # in what the piece says it changes
    unsteady: str = None          # or the reason why it may
    bytes: tuple = None           # the bytes of each side, of a run


def _hex(data, limit=16):
    return data[:limit].hex(" ") + (" ..." if len(data) > limit else "")


def _masked(build):
    """The bytes of the main data that are compared as addresses, not
    as bytes: [(offset, length)]."""
    return [(build.fields[one.field], 2) for one in ADDRESSES if one.field]


def _unsteady(build):
    """The bytes of the main data that may differ: [(offset, length,
    reason)]."""
    fields = build.structures[MAIN_DATA][1]
    found = []
    for name, into, length, reason in UNSTEADY:
        at = build.fields[name]
        if length is None:
            length = min((other for _, other in fields if other > at),
                         default=build.main_size) - at
        found.append((at + into, length, reason))
    return found


def _names(read):
    """The fields of the Read that hold a name that DOS found, as 13
    bytes each: the DTA's, and NameF of every entry that the list of a
    panel has room for. A set of bytes, kept with the Read."""
    if getattr(read, "_name_fields", None) is None:
        build = read.build
        main = build.main + build.fields["DTA"] + DTA_NAME
        found = {bytes(read.data[main:main + NAME_SIZE])}
        _, fields = build.structures[PANEL]
        entry, entry_fields = build.structures[ENTRY]
        name = dict(entry_fields)["NameF"]
        for part in read.parts().values():
            if part.kind != "a block" or part.namer is None:
                continue
            for at in range(dict(fields)[LIST] + name,
                            len(part.data) - NAME_SIZE + 1, entry):
                found.add(bytes(part.data[at:at + NAME_SIZE]))
        read._name_fields = found
    return read._name_fields


def _name_tail(before, after, part, other, begins, ends):
    """Whether every byte from `begins` to `ends` of a part is behind
    the zero of a name of 13 bytes that is the same, with its zero, in
    both builds: of a field that holds such a name, or of 13 bytes that
    are in each build what one of those fields is. See NAME_TAIL."""
    build = before.build
    dta = build.fields["DTA"] + DTA_NAME

    def field(at):
        if part.kind == "the main data":
            return at == dta
        return part.namer is not None \
            and "list, NameF, " in part.namer(at)

    for byte in range(begins, ends):
        for at in range(max(0, byte - NAME_SIZE + 1), byte + 1):
            was = bytes(part.data[at:at + NAME_SIZE])
            has = bytes(other.data[at:at + NAME_SIZE])
            zero = was.find(0)
            if len(was) < NAME_SIZE or len(has) < NAME_SIZE \
                    or zero < 0 or at + zero >= byte \
                    or was[:zero + 1] != has[:zero + 1]:
                continue
            if field(at) or (was in _names(before)
                             and has in _names(after)):
                break
        else:
            return False
    return True


def _same_segment(before, after, part, other, begins, ends):
    """Whether the bytes that differ are of a word that holds a segment
    of the same thing in both builds: a paragraph of the megabyte that
    is in the same block, as far into it. The program keeps such words
    in its buffers, and two builds that the linker has laid out
    otherwise have other numbers there."""
    if ends - begins > 2:
        return False
    for at in {begins, ends - 2}:
        if at < 0 or at + 2 > len(part.data):
            continue
        places = [read.place(read.build.address + read.build.main - ARENA
                             + PARAGRAPH * int.from_bytes(data[at:at + 2],
                                                          "little"))
                  for read, data in ((before, part.data),
                                     (after, other.data))]
        if places[0] and places[0] == places[1] \
                and places[0][0] in {thing[2] for thing in before.things}:
            return True
    return False


def compare_one(before, after, changed=()):
    """How the Read `after` differs from the Read `before`: Differences,
    and what was held, a Counter of bytes by kind."""
    found, held = [], Counter()

    def differs(part, said, name=None, unsteady=None, bytes=None,
                over=()):
        name = name or part
        found.append(Difference(
            part, said,
            declared=bool({name, part, *over} & set(changed)),
            unsteady=unsteady or UNSTEADY_VARIABLES.get(name),
            bytes=bytes))

    # The addresses, by what they name.
    for one in ADDRESSES:
        was, where_was = before.addresses[one.name]
        has, where_has = after.addresses[one.name]
        named = before.place(was), after.place(has)
        # An address of nothing known is the same only where it is the
        # same number: a pointer into a block that has been given back,
        # which the image of the same build has at the same place.
        if named[0] != named[1] or None in named and (
                was != has or before.build.known != after.build.known):
            said = [f"{place[0]} +{place[1]}" if place else
                    f"nothing known, {address:08X}h"
                    for place, address in zip(named, (was, has))]
            differs("the main data",
                    f"{one.name}: names {said[0]} before and {said[1]} "
                    "after", one.name)
    if [block[1:] for block in before.blocks] \
            != [block[1:] for block in after.blocks]:
        said = [", ".join(f"{size}{'' if taken else ' free'}"
                          for _, size, taken in read.blocks)
                for read in (before, after)]
        differs("the blocks", f"the program was handed {said[0]} bytes "
                              f"before and {said[1]} after")
    for read, side in ((before, "before"), (after, "after")):
        for said in read.image.guards:
            differs("the blocks", f"{side}, {said}")
    # The tables, each against its own file.
    for read, side in ((before, "before"), (after, "after")):
        for offset, length in read.image.written:
            place = read.place(read.build.address + offset)
            differs("the tables",
                    f"{side}, {length} bytes of the tables are not as "
                    "the program's file has them, at "
                    + (f"{place[0]}+{place[1]}" if place else
                       f"+{offset:X}h of the program's data"))
    held["the tables"] += sum(after.build.segments[name][1]
                              for name in TABLES)
    # The parts, as bytes.
    parts, others = before.parts(), after.parts()
    for name, other in others.items():
        if name not in parts and other.kind == "a block":
            differs("the blocks", f"{name}, {len(other.data)} bytes, is "
                                  "not there before")
    for name, part in parts.items():
        other = others.get(name)
        if other is None or len(other.data) != len(part.data):
            if part.kind == "a variable":
                continue          # of the builds: said once, by compare()
            if other is None:
                differs("the blocks", f"{name}, {len(part.data)} bytes, "
                                      "is not there after")
                continue
            # As far as both have it.
            size = min(len(part.data), len(other.data))
            differs("the blocks",
                    f"{name} is {len(part.data)} bytes before and "
                    f"{len(other.data)} after: held as far as both have "
                    "it")
            part = replace(part, data=part.data[:size])
            other = replace(other, data=other.data[:size])
        held[part.kind] += len(part.data)
        masked = _masked(before.build) \
            if part.kind == "the main data" else []
        for offset, length in runs(part.data, other.data):
            _run(before, after, part, other, offset, length, masked,
                 differs, held)
    # The time of a file is in every copy of a panel's list.
    times = {one.bytes for one in found
             if one.unsteady in UNSTEADY_ENTRY.values()}
    for one in found:
        if one.part in parts and parts[one.part].kind == "a block" \
                and not one.unsteady and one.bytes in times:
            one.unsteady = COPIED
    return found, held


def _run(before, after, part, other, offset, length, masked, differs,
         held):
    """A run of bytes that differ in a part: say it, or do not, where
    it is addresses that name the same thing."""
    end = offset + length
    # The words of the main data that are compared as addresses.
    pieces = [(offset, end)]
    for at, size in masked:
        pieces = [piece for begins, ends in pieces
                  for piece in ((begins, min(ends, at)),
                                (max(begins, at + size), ends))
                  if piece[0] < piece[1]]
    if part.kind == "a variable" and len(part.data) % 4 == 0:
        # A pointer, or pointers: the words that differ, if each names
        # the same thing in both builds.
        words = range(offset // 4 * 4, end, 4)
        places = [(before.place(int.from_bytes(part.data[at:at + 4],
                                               "little")),
                   after.place(int.from_bytes(other.data[at:at + 4],
                                              "little"))) for at in words]
        if all(was is not None and was == has for was, has in places):
            held["pointers that name the same in both"] += len(words)
            return
    unsteady = _unsteady(before.build) \
        if part.kind == "the main data" else []
    for at, size, _ in unsteady:
        pieces = [piece for begins, ends in pieces
                  for piece in ((begins, min(ends, at)),
                                (max(begins, at), min(ends, at + size)),
                                (max(begins, at + size), ends))
                  if piece[0] < piece[1]]
    for begins, ends in sorted(set(pieces)):
        where = part.namer(begins) if part.namer else f"+{begins}"
        name, reason, over = part.name, None, ()
        if part.kind == "the main data":
            name = where.split("+")[0].split(" ")[0]
            # The structures that lie over the byte, which a piece may
            # name as changed in place of the field.
            over = [laid for laid in LAID_OVER
                    if f"{laid}." in where]
            reason = next((reason for at, size, reason in unsteady
                           if at <= begins < at + size), None)
            if not reason and _name_tail(before, after, part, other,
                                         begins, ends):
                reason = NAME_TAIL
            if not reason and _same_segment(before, after, part, other,
                                            begins, ends):
                held["segments that name the same in both"] += 1
                continue
        elif part.kind == "a block":
            reason = next((reason for field, reason
                           in UNSTEADY_ENTRY.items()
                           if f"list, {field}" in where), None)
            if not reason and _name_tail(before, after, part, other,
                                         begins, ends):
                reason = NAME_TAIL
        differs(part.name,
                f"{part.name}, {where}"
                + (f", {ends - begins} bytes" if ends - begins > 1 else "")
                + f": {_hex(part.data[begins:ends])} before, "
                  f"{_hex(other.data[begins:ends])} after",
                name, reason,
                (part.data[begins:ends], other.data[begins:ends]), over)


def compare(before, after, names=None, changed=(), every=False,
            out=print):
    """Hold the images of the directory `after` against those of
    `before`, and say what differs. Returns whether anything does that
    is not declared in `changed` and not UNSTEADY."""
    before, after = Build(before), Build(after)
    have, others = before.images(), after.images()
    failed = False
    for name in names or ():
        if name not in have and name not in others:
            out(f"{name}: neither directory has an image of it")
            failed = True
    names = [name for name in sorted(set(have) | set(others))
             if not names or name in names]
    same = before.known["program"] == after.known["program"]
    out(f"{before.directory} and {after.directory}: "
        + ("images of one program" if same else "of two builds"))
    # What the builds themselves have otherwise, said once.
    for name in sorted(set(before.variables) | set(after.variables)):
        was, has = before.variables.get(name), after.variables.get(name)
        if was is None or has is None or was[1] != has[1]:
            out(f"  {name}: "
                + (f"{was[1]} bytes before" if was else "not there before")
                + ", " + (f"{has[1]} after" if has else "not there after")
                + ": not compared")
    equal, total, counts = 0, Counter(), Counter()
    for name in names:
        if name not in have or name not in others:
            only = before.directory if name in have else after.directory
            out(f"{name}: only {only} has an image of it")
            failed = True
            continue
        try:
            found, held = compare_one(before.read(name), after.read(name),
                                      changed)
        except Spoilt as spoilt:
            out(f"{name}: {spoilt}")
            failed = True
            continue
        total.update(held)
        wrong = [one for one in found
                 if not one.declared and not one.unsteady]
        for one in found:
            counts["declared" if one.declared else "unsteady"
                   if one.unsteady else "differ"] += 1
        if not found:
            equal += 1
            continue
        if wrong:
            failed = True
        out(f"{name}: " + (f"{len(wrong)} differences" if wrong else
                           "as before, but for what may differ"))
        shown = found if every else wrong[:LINES]
        for one in shown:
            out(f"    {one.said}"
                + ("  [declared]" if one.declared else
                   f"  [{one.unsteady}]" if one.unsteady else ""))
        if len(wrong) > len(shown):
            parts = Counter(one.part for one in wrong[len(shown):])
            out("    and {} more, in {}".format(
                len(wrong) - len(shown), ", ".join(
                    f"{part} ({count})" for part, count in parts.items())))
    count = len(names)
    out(f"{count} scenarios, {equal} with nothing that differs; "
        f"{counts['differ']} differences, {counts['declared']} more in "
        f"what was declared as changed, {counts['unsteady']} in what "
        "may differ")
    if count:
        loose = sum(size for _, size, _ in _unsteady(before))
        names = {"the main data": "the main data",
                 "a block": "the blocks that are taken",
                 "a variable": "the variables", "the tables": "the tables"}
        out("held in a scenario, in bytes: " + ", ".join(
            f"{names[kind]} {total[kind] // count}" for kind in names)
            + f"; of the main data {loose} bytes may differ and "
              f"{2 * len(_masked(before))} are addresses, held by what "
              "they name")
        same = [f"{total[kind]} {kind}" for kind in total
                if kind not in names]
        if same:
            out("in all scenarios: " + ", ".join(same))
    return failed


LINES = 12                        # of one scenario's differences, shown


# The commands.

def save(program, directory, names=None, together=None, early=False,
         out=print):
    """Take `program` through its scenarios, or those of `names`, and
    write the image of each into `directory`. With `early`, the driver
    of the tests does not write the program's stack out, and the same
    program gets to every step some instructions earlier."""
    import compare as scenario_compare
    import scenarios
    import setups
    from dos import MachineError
    program = setups.Program.load(program)
    taker = Taker(program)
    write_facts(directory, taker.facts)
    names = names or scenarios.names(program.tree)
    failed = False

    def one(name):
        scenario = scenarios.of(name, program.tree)
        try:
            # With the stack looked at, as the scenarios' test has
            # it: the driver of the tests then writes the stack out at
            # every step, the program gets to a step some instructions
            # later, and what follows the time follows that.
            return scenarios.run(scenario, program,
                                 benches[scenario.bench],
                                 stack=not early, image=taker)
        except MachineError as error:
            return str(error)

    with setups.Benches() as benches, ThreadPoolExecutor(
            together or scenario_compare.TOGETHER) as pool:
        for name, run in zip(names, pool.map(one, names)):
            if isinstance(run, str) or run.image is None:
                out(f"{name}: no image: "
                    + (run if isinstance(run, str) else
                       "the scenario did not end where the driver of the "
                       "tests ends it"))
                failed = True
                continue
            run.image.write(Path(directory) / (name + SUFFIX))
            for said in run.image.guards:
                out(f"{name}: {said}")
                failed = True
    out(f"{directory}: the images of {len(names)} scenarios"
        + (", but for those named above" if failed else ""))
    return failed


def show(directory, name, out=print):
    build = Build(directory)
    read = build.read(name)
    out(f"{name}, of the program {build.known['program'][:16]}...")
    for called, address, data in read.image.stretches:
        out(f"  {called}: {len(data)} bytes at {address:08X}h")
    out(f"  the main data: {build.main_size} bytes at "
        f"{build.address + build.main:08X}h")
    for number, (begins, size, taken) in enumerate(read.blocks):
        out(f"  block {number}: {size} bytes at "
            f"{build.address + begins:08X}h, "
            + ("taken" if taken else "free, and not compared"))
    for one in ADDRESSES:
        address, where = read.addresses[one.name]
        place = read.place(address)
        out(f"  {one.name}: {address:08X}h, "
            + (f"{place[0]} +{place[1]}" if place else "nothing known"))
    out(f"  {len(build.variables)} variables, "
        f"{sum(size for _, size in build.variables.values())} bytes")
    out(f"  the tables: {len(read.image.written)} places that are not as "
        "the program's file has them")


def _changed(read, offset):
    """The stretches of the Read's image with the byte at `offset` of
    the program's data changed, in whichever stretch it is."""
    address = read.build.address + offset
    found = []
    for name, at, data in read.image.stretches:
        if at <= address < at + len(data):
            data = bytearray(data)
            data[address - at] ^= 0x40
            data = bytes(data)
        found.append((name, at, data))
    return found


def check(directory, out=print):
    """Hold `directory` against itself, and against itself with bytes
    changed by hand, each of which has to be named. Returns whether the
    net failed to catch one."""
    import shutil
    import tempfile
    build = Build(directory)
    names = build.images()
    if not names:
        out(f"{directory} has no images")
        return True
    lines = []
    if compare(directory, directory, out=lines.append):
        out("\n".join(lines))
        out("the directory differs from itself")
        return True
    out(lines[-2])
    name = names[0]
    read = build.read(name)
    block = next(number for number, (_, _, taken)
                 in enumerate(read.blocks) if taken)
    begins = read.blocks[block][0]
    panel = read.addresses["WCB1seg"][0] - build.address
    text = read.addresses["ViewSeg"][0] - build.address
    # The block that the room is in, which the program had as one block
    # with its panels once and has as a block of its own since.
    room = next(one for one in read.blocks
                if one[0] <= text < sum(one[:2]))
    # A variable that two builds are held to have alike, and a word
    # that holds an address, where this build has it: beside the main
    # data, or in its field.
    variable, (at, _) = next(iter(sorted(
        (name, where) for name, where in build.variables.items()
        if where[1] and not name.startswith("(")
        and name not in UNSTEADY_VARIABLES and name not in POINTERS)))
    word = next(one for one in ADDRESSES if one.name == "WCBsec")
    held = build._symbol(*word.variable) if word.variable else None
    if held is None:
        held = build.main + build.fields[word.field]
    # Where a byte is changed, and what has to be said of it.
    trials = [
        (build.main + build.fields["TmpBuf"] + 3, "the main data, TmpBuf+3"),
        (build.main + build.main_size - 1, "the main data, "),
        (panel + 5, "the panel of WCB1seg, "),
        (panel + dict(build.structures[PANEL][1])[LIST] + 24 * 2 + 13,
         "entry 2 of its list, AttrF"),
        (text + 1000, f"{TEXT}, +1000"),
        (sum(room[:2]) - 1, f"{TEXT}, +"),
        (at, variable),
        (held, "WCBsec: names "),
    ]
    failed = False
    with tempfile.TemporaryDirectory(dir=Path(directory).parent) as copy:
        shutil.copyfile(Path(directory) / FACTS, Path(copy) / FACTS)
        for offset, expected in trials:
            stretches = _changed(read, offset)
            Image(stretches, read.image.written).write(
                Path(copy) / (name + SUFFIX))
            lines = []
            caught = compare(directory, copy, [name], out=lines.append)
            said = [line for line in lines if expected in line]
            if caught and said and "1 differences" in "\n".join(lines):
                out(f"  a byte changed at +{offset:X}h: {said[0].strip()}")
            else:
                failed = True
                out(f"  a byte changed at +{offset:X}h of {name} was to "
                    f"be named with \"{expected}\", and this was said:")
                out("\n".join("      " + line for line in lines))
    # The bytes round a block: what the extender has in front of a
    # block, or the pattern that the program's own hand-out wrote, or,
    # in a build from before dos32.c wrote one, what the image has in
    # front of its first block that is taken, held there alone.
    front = begins - 16
    old = build._symbol(*OWN_BLOCKS) is None \
        and build._symbol(*BLOCKS) is None
    pattern = read.data[front:begins] if old else None
    places = [(front, f"in front of block {block}")] if old else None
    spoilt = Read(build, Image(_changed(read, begins - 1), []))
    said = guards(spoilt, pattern, places)
    if guards(read, pattern, places) or len(said) != 1 \
            or f"in front of block {block}" not in said[0]:
        failed = True
        out("  the pattern round a block: a byte changed in front of "
            f"block {block} was to be named, and this was said: {said}; "
            f"of the image as it is: {guards(read, pattern, places)}")
    else:
        out(f"  a byte changed at +{begins - 1:X}h: {said[0]}")
        if old:
            out("  (the build writes no pattern: what the image has "
                "there was taken for one, for this trial)")
    out("the net " + ("did not catch every one" if failed else
                      f"caught each of {len(trials) + 1} bytes changed in "
                      f"{name}, by its name"))
    return failed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    one = commands.add_parser("save")
    one.add_argument("program")
    one.add_argument("directory")
    one.add_argument("scenario", nargs="*")
    one.add_argument("--together", type=int, metavar="N")
    one.add_argument("--early", action="store_true",
                     help="have the same program get to every step some "
                          "instructions earlier: what then differs from "
                          "its images saved without this follows the "
                          "time, and nothing else does")
    one = commands.add_parser("compare")
    one.add_argument("before")
    one.add_argument("after")
    one.add_argument("scenario", nargs="*")
    one.add_argument("--changed", action="append", default=[],
                     metavar="NAME",
                     help="a field of the main data, a structure that "
                          "lies over one (EdData, ViewDat, FndData, "
                          "HlpData), a variable as file.Name, a thing "
                          "in a block as \"the panel of WCB1seg\", or "
                          "\"the blocks\" for their number and lengths, "
                          "that the work is meant to change")
    one.add_argument("--all", action="store_true",
                     help="every difference of a scenario, also those "
                          f"beyond the first {LINES} and those that may "
                          "differ")
    one = commands.add_parser("show")
    one.add_argument("directory")
    one.add_argument("scenario")
    one = commands.add_parser("check")
    one.add_argument("directory")
    args = parser.parse_args()
    try:
        if args.command == "save":
            sys.exit(save(args.program, args.directory, args.scenario,
                          args.together, args.early))
        if args.command == "compare":
            sys.exit(compare(args.before, args.after, args.scenario,
                             args.changed, args.all))
        if args.command == "show":
            show(args.directory, args.scenario)
        if args.command == "check":
            sys.exit(check(args.directory))
    except Spoilt as spoilt:
        sys.exit(str(spoilt))


if __name__ == "__main__":
    main()
