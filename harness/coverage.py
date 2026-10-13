"""Which code of a program ran, from QEMU's log of the code it executed.

QEMU translates the guest's code a block at a time, just before running the
block for the first time, and with `-d in_asm` logs each block as it does:
the address and the bytes of every instruction. A block runs from where it
is entered to the next jump, so with rare exceptions (an interrupt taken in
the middle of one) what was logged is what was executed.

Volkov Commander moves itself about in memory, but its code can be told
from everything else by its bytes: a block whose bytes occur in the program
file exactly once shows where in memory that part of the file sat.

The C translation's file is an .EXE that DOS does not relocate: the program
adds the segment it is at to the words that its relocation table names, in
the part of it that stays in the memory once, and in the other part every
time it has loaded that (see tools/mkexe.c, and Reloc in
src/c/resident.c). Code that holds such a word is not in the memory as it
is in the file, once that is done. So a block is held against the file as
it is and against the file with those words as they are at the place in
question: a function that begins by loading a segment, as an interrupt
function of the compiler does, is known for what it is.

The portable C's file is an .EXE as the linker writes it, which DOS
relocates by the table in its header: the same holds of it, with that
table. Which of the two a file is, its header says: see origin().
"""
import bisect
import re
import struct
from collections import Counter
from dataclasses import dataclass

# Blocks shorter than this could match the program file by accident.
MIN_BLOCK = 8
# A place in memory counts as holding the program when this many blocks
# agree on it.
MIN_VOTES = 4

FAR_CALL, FAR_JUMP = 0x9A, 0xEA

# The translation's file, as tools/mkexe.c makes it: a header, and behind it
# the program from offset 100h of the segment it is loaded into, where a
# .COM file's begins. The program has the numbers it relocates itself by in
# words of src/c/resident.c, which the linker's map names: where its
# relocation table is, in paragraphs from the segment's start, how many
# entries that has for the part that goes while a command runs, and how
# many more for the part that stays. An entry is the offset and the
# segment of a word, counted from the segment's start too.
EXE_HEADER = 0x20
COM_ORIGIN = 0x100
EXE_TABLE, EXE_COUNT, EXE_STAYING = "RelTab", "RelCnt", "ResCnt"

# The header of an .EXE file: the number of its relocations, its length
# in paragraphs, the stack's segment and the code's as DOS is to set them,
# counted from where the file's contents are loaded, and where the
# relocations are, each the offset and the segment of a word.
EXE_RELOCATIONS, EXE_PARAGRAPHS, EXE_SS, EXE_CS, EXE_TABLE_AT = \
    0x06, 0x08, 0x0E, 0x16, 0x18
# What tools/mkexe.c writes for the code's segment: 10h paragraphs below
# the file's contents, the PSP.
EXE_AT_PSP = 0xFFF0


def made_by_mkexe(contents):
    """Whether the .EXE file `contents` is one that tools/mkexe.c made,
    the C translation's, and not one as the linker writes it: it starts
    in the segment of its PSP."""
    return struct.unpack_from("<H", contents, EXE_CS)[0] == EXE_AT_PSP


def origin(contents):
    """Where in the .EXE file `contents` the byte is that the linker's map
    has at 0000:0000, which is also where the words that hold a segment
    count from: add it to an address of the map for the place in the
    file. The map of a file that tools/mkexe.c made counts from the PSP,
    100h before the file's contents, so this is below 0 for it; that of
    one as the linker writes it counts from the contents, behind the
    header."""
    if made_by_mkexe(contents):
        return EXE_HEADER - COM_ORIGIN
    return struct.unpack_from("<H", contents, EXE_PARAGRAPHS)[0] * 16

_LINE = re.compile(r"0x([0-9a-f]{8}):  ((?:[0-9a-f]{2} )*[0-9a-f]{2})"
                   r"(?:  +(\S.*))?$")


def blocks(trace_path):
    """Yield each logged block as a list of (address, bytes) instructions."""
    block = []
    with open(trace_path, errors="replace") as log:
        for line in log:
            match = _LINE.match(line)
            if not match:
                if line.startswith("IN:") and block:
                    yield block
                    block = []
                continue
            address = int(match[1], 16)
            code = bytes.fromhex(match[2])
            # A long instruction continues on a line of its own, which has
            # no mnemonic.
            if block and match[3] is None:
                block[-1] = (block[-1][0], block[-1][1] + code)
            else:
                block.append((address, code))
    if block:
        yield block


def relocated(contents, linked=None):
    """The offsets in `contents`, a program file, of the words that the
    program adds its segment to when it is loaded: those of the
    translation's relocation table, with the linker's map of the translation
    at `linked`. None of them for a file that is no .EXE, the original."""
    if contents[:2] != b"MZ":
        return ()
    in_file = origin(contents)              # from the segment to the file
    if not made_by_mkexe(contents):
        # As the linker writes the file: the table of its header.
        count, table = (struct.unpack_from("<H", contents, at)[0]
                        for at in (EXE_RELOCATIONS, EXE_TABLE_AT))
        return tuple(segment * 16 + offset + in_file
                     for offset, segment in struct.iter_unpack(
                         "<HH", contents[table:table + count * 4]))
    symbols = link_map(linked)
    table, count, staying = (
        struct.unpack_from("<H", contents, symbols[name][0] * 16
                           + symbols[name][1] + in_file)[0]
        for name in (EXE_TABLE, EXE_COUNT, EXE_STAYING))
    count += staying
    return tuple(segment * 16 + offset + in_file
                 for offset, segment in struct.iter_unpack(
                     "<HH", contents[table * 16 + in_file:][:count * 4]))


def _loaded(contents, words, base):
    """`contents` as the program has it in the memory when the byte at
    offset 0 of the file is at address `base`: with its segment added to
    each of `words`. None if the program cannot be there."""
    segment, odd = divmod(base + origin(contents), 16)
    if odd or segment < 0:
        return None
    there = bytearray(contents)
    for at in words:
        value, = struct.unpack_from("<H", contents, at)
        struct.pack_into("<H", there, at, value + segment & 0xFFFF)
    return bytes(there)


def executed(trace_path, contents, linked=None):
    """Offset -> length in `contents` (a program file) of what was
    executed. `linked` is the linker's map of the translation, for where its
    relocation table is."""
    logged = list(blocks(trace_path))
    votes = Counter()
    # Instructions that follow one another in memory vote as one block
    # too: with a breakpoint near, QEMU translates one at a time.
    runs = []
    for address, code in sorted({instruction for block in logged
                                 for instruction in block}):
        if runs and runs[-1][-1][0] + len(runs[-1][-1][1]) == address:
            runs[-1].append((address, code))
        else:
            runs.append([(address, code)])
    for block in logged + runs:
        code = b"".join(code for _, code in block)
        if len(code) < MIN_BLOCK:
            continue
        offset = contents.find(code)
        if offset >= 0 and contents.find(code, offset + 1) < 0:
            # A long stretch counts for as many blocks as it could be.
            votes[block[0][0] - offset] += len(code) // MIN_BLOCK
    bases = [base for base, count in votes.most_common()
             if count >= MIN_VOTES]

    # A block is in the program if every instruction of it is, at one and
    # the same place: single instructions are too short to go by.
    words = relocated(contents, linked)
    there = {base: _loaded(contents, words, base) if words else contents
             for base in bases}
    bases = [base for base in bases if there[base] is not None]
    result = {}
    for block in logged:
        for base in bases:
            # As the program is at that place, or as it is in the file:
            # the part that stays runs before it relocates itself.
            if any(all(_matches(image, address - base, code)
                       for address, code in block)
                   for image in dict.fromkeys((there[base], contents))):
                for address, code in block:
                    result[address - base] = len(code)
                break
    return result


def _matches(contents, offset, code):
    if offset < 0:
        return False
    there = contents[offset:offset + len(code)]
    # A far call or jump has had its segment relocated.
    return there == code or (code[0] in (FAR_CALL, FAR_JUMP)
                             and there[:3] == code[:3])


@dataclass(frozen=True)
class Line:
    """A line of an assembler listing that produced code."""
    address: int
    text: str


class _Procedures(dict):
    """Procedures by name, and code from one label to another by the two
    names with two full stops between them."""

    def __init__(self, labels):
        super().__init__()
        self.labels = labels

    def __missing__(self, name):
        first, stops, last = name.partition("..")
        if not stops or first not in self.labels or last not in self.labels:
            raise KeyError(name)
        start = self.labels[first]
        return start, self.labels[last] - start

    def __contains__(self, name):
        try:
            self[name]
        except KeyError:
            return False
        return True


class Listing:
    """A JWasm listing of a tiny-model program: its code, line by line.

    Addresses are offsets in the program's one segment, so for a .COM file
    100h more than offsets in the file.

    The program has code in its data too: what follows a .DATA directive
    with no .CODE behind it, DirFunc and the code after it. The listing
    counts the addresses of that from where the data begins, and says
    where that is in its map of the file, at its end.

    Code that is no procedure also goes by two labels of the code
    segment, "Alloc0..Init00": what stands from the first to the second.
    That is for a stretch of which only the beginning is meant, where a
    single label names all that stands from there to the next procedure.
    """

    def __init__(self, path):
        self.lines = []
        # name -> (address, length)
        self.procedures = _Procedures(labels := {})
        # As the listing has them, counted from where their segment
        # begins: segment -> ...
        lines = {"_TEXT": [], "_DATA": []}   # [(address, text)]
        data = {"_TEXT": [], "_DATA": []}    # addresses of what is no code
        procedures = {"_TEXT": {}, "_DATA": {}}  # name -> (address, length)
        labels = {"_TEXT": labels, "_DATA": {}}  # name -> address
        begins = {}                          # segment -> where it begins
        segment = "_TEXT"
        table = False
        with open(path, encoding="cp866") as listing:
            for line in listing:
                line = line.rstrip("\r\n")
                match = re.match(r"(_TEXT|_DATA) +[0-9A-F]+ +([0-9A-F]+) ",
                                 line)
                if match:
                    begins[match[1]] = int(match[2], 16)
                if line.startswith("Procedures, parameters and locals:"):
                    table = True
                elif table:
                    match = re.match(r"(\w+) [ .]*P Near +([0-9A-F]{4}) +"
                                     r"(_TEXT|_DATA) +([0-9A-F]{4})", line)
                    if match:
                        procedures[match[3]][match[1]] = (int(match[2], 16),
                                                          int(match[4], 16))
                    match = re.match(r"(\w+) [ .]*L Near +([0-9A-F]+)h +"
                                     r"(_TEXT|_DATA)", line)
                    if match:
                        labels[match[3]][match[1]] = int(match[2], 16)
                    continue
                text = line[32:]
                directive = re.match(r"\s*\.(CODE|CONST|DATA)\b", text)
                if directive:
                    segment = {"CODE": "_TEXT", "DATA": "_DATA",
                               "CONST": None}[directive[1]]
                match = re.match(r"([0-9A-F]{4}) [0-9A-F]{2}", line)
                if match and segment and not _is_data(text):
                    lines[segment].append((int(match[1], 16), text))
                elif match and segment:
                    data[segment].append(int(match[1], 16))

        for segment in lines:
            # The listing counts the code from 100h already.
            base = begins[segment] if segment == "_DATA" else 0
            # Code that stands between two procedures belongs to neither.
            # Such a stretch counts as a procedure of its own: it has the
            # name of its first label, and ends where the next procedure
            # begins. A stretch may hold the code of two files, the end
            # of VCMENU.INC and the commands that VCF1.INC begins with,
            # so a later label of it names what stands from there to the
            # same end: F37_00 in the stretch M60_00. In the data no
            # procedure need follow the code, and a label may stand in a
            # table: there a stretch begins at a label that has an
            # instruction, and ends where the next data begin as well:
            # FncSel, behind DirFunc.
            inside = sorted(procedures[segment].values())
            starts = [start for start, _ in inside]
            code = {address for address, _ in lines[segment]}
            for name, address in sorted(labels[segment].items(),
                                        key=lambda item: item[1]):
                before = bisect.bisect_right(starts, address)
                if before and address < sum(inside[before - 1]):
                    continue
                end = starts[before] if before < len(starts) else None
                if segment == "_DATA":
                    if address not in code:
                        continue
                    behind = [there for there in data[segment]
                              if there > address]
                    if behind and (end is None or min(behind) < end):
                        end = min(behind)
                if end is not None:
                    procedures[segment][name] = (address, end - address)
            # What is not in a procedure of the data is data, whatever
            # directive made it.
            for address, text in lines[segment]:
                if segment == "_TEXT" or any(
                        start <= address < start + length
                        for start, length in procedures[segment].values()):
                    self.lines.append(Line(base + address, text))
            for name, (address, length) in procedures[segment].items():
                self.procedures[name] = (base + address, length)

    def procedure(self, name):
        """The code lines of procedure `name`, or of the code between two
        procedures whose first label is `name`."""
        start, length = self.procedures[name]
        return [line for line in self.lines
                if start <= line.address < start + length]


def _is_data(text):
    return re.match(r"\s*(\w+\s+)?(DB|DW|DD)\s", text) is not None


def link_map(path):
    """Read a wlink map: public symbol -> (segment, offset), both counted
    from where the program is loaded."""
    symbols = {}
    with open(path) as text:
        for line in text:
            match = re.match(r"([0-9a-f]{4}):([0-9a-f]{4})[*+]? +(\S+)$", line)
            if match:
                symbols[match[3]] = (int(match[1], 16), int(match[2], 16))
    return symbols


# The stack of a C tree, as its link map names it: an array, and the
# segment that the compiler has it in, of which it is all. The C
# translation's is of src/c/resident.c; see tools/mkexe.c, which stops the
# build if it is not all of that segment. The portable C's is of
# src/portable_c/stack.c.
STACKS = (("DataSav", "R_BSS"), ("_Stack", "SSTACK"))


def segments(path):
    """Read a wlink map: its segments, each as name, class, where it
    begins, as the paragraph and the offset that the map has, and how
    long it is."""
    found = []
    with open(path) as text:
        for line in text:
            match = re.match(r"(\w+) +(\w+) +(\w+) +([0-9a-f]{4}):"
                             r"([0-9a-f]{4}) +([0-9a-f]{8})$", line)
            if match:
                found.append((match[1], match[2], int(match[4], 16),
                              int(match[5], 16), int(match[6], 16)))
    return found


def stack(path):
    """Where the stack of a C tree begins and ends, by the wlink map at
    `path`: two addresses as the map counts them (see origin())."""
    symbols = link_map(path)
    array, segment = next(names for names in STACKS if names[0] in symbols)
    at, offset = symbols[array]
    bottom = at * 16 + offset
    with open(path) as text:
        for line in text:
            match = re.match(rf"{segment} +\w+ +\w+ +([0-9a-f]{{4}}):"
                             r"([0-9a-f]{4}) +([0-9a-f]{8})$", line)
            if match:
                begin = int(match[1], 16) * 16 + int(match[2], 16)
                if begin != bottom:
                    raise ValueError(f"{path}: {array} is not where "
                                     f"{segment} begins")
                return bottom, begin + int(match[3], 16)
    raise ValueError(f"{path} has no segment {segment}")
