"""Which code of a program ran, from QEMU's log of the code it executed.

QEMU translates the guest's code a block at a time, just before running the
block for the first time, and with `-d in_asm` logs each block as it does:
the address and the bytes of every instruction. A block runs from where it
is entered to the next jump, so with rare exceptions (an interrupt taken in
the middle of one) what was logged is what was executed.

Volkov Commander moves itself about in memory, but its code can be told
from everything else by its bytes: a block whose bytes occur in the program
file exactly once shows where in memory that part of the file sat.
"""
import bisect
import re
from collections import Counter
from dataclasses import dataclass

# Blocks shorter than this could match the program file by accident.
MIN_BLOCK = 8
# A place in memory counts as holding the program when this many blocks
# agree on it.
MIN_VOTES = 4

FAR_CALL, FAR_JUMP = 0x9A, 0xEA

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


def executed(trace_path, contents):
    """Offset -> length in `contents` (a program file) of what was executed."""
    logged = list(blocks(trace_path))
    votes = Counter()
    for block in logged:
        code = b"".join(code for _, code in block)
        if len(code) < MIN_BLOCK:
            continue
        offset = contents.find(code)
        if offset >= 0 and contents.find(code, offset + 1) < 0:
            votes[block[0][0] - offset] += 1
    bases = [base for base, count in votes.most_common()
             if count >= MIN_VOTES]

    # A block is in the program if every instruction of it is, at one and
    # the same place: single instructions are too short to go by.
    result = {}
    for block in logged:
        for base in bases:
            if all(_matches(contents, address - base, code)
                   for address, code in block):
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


class Listing:
    """A JWasm listing of a tiny-model program: its code, line by line.

    Addresses are offsets in the program's one segment, so for a .COM file
    100h more than offsets in the file.

    The program has code in its data too: what follows a .DATA directive
    with no .CODE behind it, DirFunc and the code after it. The listing
    counts the addresses of that from where the data begins, and says
    where that is in its map of the file, at its end.
    """

    def __init__(self, path):
        self.lines = []
        self.procedures = {}                 # name -> (address, length)
        # As the listing has them, counted from where their segment
        # begins: segment -> ...
        lines = {"_TEXT": [], "_DATA": []}   # [(address, text)]
        procedures = {"_TEXT": {}, "_DATA": {}}  # name -> (address, length)
        labels = {"_TEXT": {}, "_DATA": {}}  # name -> address
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

        for segment in lines:
            # The listing counts the code from 100h already.
            base = begins[segment] if segment == "_DATA" else 0
            # Code that stands between two procedures belongs to neither.
            # Such a stretch counts as a procedure of its own: it has the
            # name of its first label, and ends where the next procedure
            # begins. Not in the data, where a label may stand in a table
            # and no procedure follows the code: FncSel, behind DirFunc,
            # cannot be named yet.
            inside = sorted(procedures[segment].values())
            starts = [start for start, _ in inside]
            stretches = {}                   # where it ends -> name
            for name, address in sorted(labels[segment].items(),
                                        key=lambda item: item[1]):
                if segment == "_DATA":
                    break
                before = bisect.bisect_right(starts, address)
                if before and address < sum(inside[before - 1]):
                    continue
                end = starts[before] if before < len(starts) else None
                if end is not None and end not in stretches:
                    stretches[end] = name
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
