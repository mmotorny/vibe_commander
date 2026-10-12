#!/usr/bin/env python3
"""Hold the data of the port against the original's, byte for byte.

The scenarios compare screens and disks. A table that no screen shows can
be wrong and pass them: a text that is copied with what follows it, the
defaults of a setup, a colour of a box that never comes up. So every data
object of the port that has the name of one of the original, a text, a
box, a dialog, a table, is compared with the original's bytes here, with
no machine involved.

The original is the build of the published source that is laid out like
the shipped binary, build/shareware-layout/VC.COM, which the test
bytes-match-original holds against that binary. Its listing says where
every label is. The port is build/port/VC.EXE, and the linker's map says
where each of its objects landed, static ones too; how long an object of
the C code is, its object file says. Of the assembly that is left the
listing says where a label is in its segment, and an object there goes
as far as the next label.

A word that holds the address of something else cannot be equal as a
number: a box that names its text, a dialog that names its box, a table of
commands. Which words those are is not described here. The original
source is assembled a second time, as an object file, whose relocations
name every such word and what it points at; here the word of the port is
then held against where the port has the thing of that name, with the
same distance from it. So an object is written in C as it is in the
assembly, with the names of what it names, and the compiler and the
linker see to the addresses.

Exits with status 0 if every object that was compared is as the original,
and 1 otherwise, with the object and the offset of the first byte or word
that differs.
"""
import argparse
import bisect
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

from bytecheck import REGIONS

COM_ORIGIN = 0x100          # where a .COM file is loaded, and the PSP ends
EXE_HEADER = 0x20           # of the port's file: see tools/mkexe.c

# Records of an object file (OMF).
LNAMES, SEGDEF, GRPDEF, EXTDEF, LEXTDEF = 0x96, 0x98, 0x9A, 0x8C, 0xB4
PUBDEF, LPUBDEF, LEDATA, FIXUPP = 0x90, 0xB6, 0xA0, 0x9C
OFFSET16 = 1                # the kind of a relocation: a 16-bit offset
TO_SEGMENT, TO_GROUP, TO_EXTERNAL = 0, 1, 2

# Constants of the C code that the original has under another name, or
# inside something of another name: name -> (the label of the original,
# how far behind it). They are compared there.
ELSEWHERE = {
    "AllIn_S": ("Mask_S", 0),
    "DirMarker": ("UpDir", 0),
    "FindColors": ("Find_P", 0),
    "MainExt_F": ("Ext_F", 0),
    "MenuLn": ("MenuLn1", 0),
    "MnuHlp": ("MnuHlp1", 0),
    "SubDir": ("SubDir", 1),
}

# Constants of the C code that the original has not as data, or not in one
# piece: name -> why. They are not compared.
OWN = {
    "Panel_T": "Brief_T and Full_T of the original, as two rows of four",
    "PowersOfTen": "what Number divides by: the original has them in its "
                   "instructions",
    "Scan_P": "the four colours of a box that the C draws without the "
              "table of one",
}

# Objects of the port that have the name of one of the original and are
# meant to be otherwise: name -> why. Each is still compared, and has to
# differ: an entry that no longer applies fails the test.
DIFFERENT = {
}

# Words of an object that hold an address in the original, where the port
# has something else than the address of the thing of that name:
# (object, offset) -> (how, what, why). "at" is the address of the label
# `what` instead; "behind" is a number, how far what the original names
# there is behind the label `what` of the original; "is" is the number
# `what` itself, as where a routine is named by its place in a table of
# functions.
_MENUS = ("the C has the lines of the menus in one array and the pages of "
          "their help in another, and a menu by its place in them")
NAMED_OTHERWISE = {
    ("Menu1_T", 28): ("at", "M17_00",
                      "Drive... goes on in a stub of its own, which hands "
                      "the box of the drives the panel and the menu as its "
                      "function takes them"),
    **{("MnuTab", row * 8 + 4): ("behind", "MenuLn1", _MENUS)
       for row in range(5)},
    **{("MnuTab", row * 8 + 6): ("behind", "MnuHlp1", _MENUS)
       for row in range(5)},
}


class Problem(Exception):
    pass


class ObjectFile:
    """An object file: its segments, its symbols, what its segments hold
    and which words of that the linker has yet to fill in."""

    def __init__(self, path):
        self.names = [None]         # numbered from 1, like all that follows
        self.segments = [None]      # (name, class, length)
        self.groups = [None]
        self.externals = [None]
        self.symbols = []           # (name, segment number, offset)
        self.data = []              # (segment number, offset, contents)
        self.fixups = []            # (segment number, offset, kind of
                                    #  target, its number, displacement)
        contents = Path(path).read_bytes()
        at = 0
        while at + 3 <= len(contents):
            kind = contents[at]
            end = at + 3 + struct.unpack_from("<H", contents, at + 1)[0] - 1
            self._record(kind, contents[at + 3:end])
            at = end + 1

    @staticmethod
    def _index(record, at):
        if record[at] & 0x80:
            return (record[at] & 0x7F) << 8 | record[at + 1], at + 2
        return record[at], at + 1

    def _record(self, kind, record):
        at = 0
        if kind == LNAMES:
            while at < len(record):
                self.names.append(
                    record[at + 1:at + 1 + record[at]].decode("latin-1"))
                at += 1 + record[at]
        elif kind == SEGDEF:
            at = 4 if record[0] >> 5 == 0 else 1    # absolute: an address
            length = struct.unpack_from("<H", record, at)[0]
            name, at = self._index(record, at + 2)
            cls, at = self._index(record, at)
            self.segments.append((self.names[name], self.names[cls], length))
        elif kind == GRPDEF:
            name, at = self._index(record, 0)
            self.groups.append(self.names[name])
        elif kind in (EXTDEF, LEXTDEF):
            while at < len(record):
                name = record[at + 1:at + 1 + record[at]].decode("latin-1")
                _, at = self._index(record, at + 1 + record[at])
                self.externals.append(name)
        elif kind in (PUBDEF, LPUBDEF):
            _, at = self._index(record, 0)
            segment, at = self._index(record, at)
            if segment == 0:
                at += 2                             # an absolute frame
            while at < len(record):
                name = record[at + 1:at + 1 + record[at]].decode("latin-1")
                at += 1 + record[at]
                offset = struct.unpack_from("<H", record, at)[0]
                _, at = self._index(record, at + 2)
                self.symbols.append((name, segment, offset))
        elif kind == LEDATA:
            segment, at = self._index(record, 0)
            offset = struct.unpack_from("<H", record, at)[0]
            self.data.append((segment, offset, record[at + 2:]))
        elif kind == FIXUPP:
            segment, base, _ = self.data[-1]
            while at < len(record):
                if not record[at] & 0x80:
                    raise Problem("an object file has a relocation of a "
                                  "kind that is not known here (a thread)")
                where = (record[at] & 3) << 8 | record[at + 1]
                relative = record[at] & 0x40
                location = record[at] >> 2 & 0xF
                fix = record[at + 2]
                at += 3
                if not fix & 0x80 and fix >> 4 & 7 < 3:
                    _, at = self._index(record, at)     # the frame
                if fix & 8:
                    raise Problem("an object file has a relocation of a "
                                  "kind that is not known here (a thread)")
                target, at = self._index(record, at)
                displacement = 0
                if not fix & 4:
                    displacement = struct.unpack_from("<H", record, at)[0]
                    at += 2
                if relative and location == OFFSET16:
                    self.fixups.append((segment, base + where, fix & 3,
                                        target, displacement))

    def contents(self, segment):
        """What segment number `segment` holds, as far as it is given."""
        result = bytearray(self.segments[segment][2])
        for number, offset, contents in self.data:
            if number == segment:
                result[offset:offset + len(contents)] = contents
        return result


def listed_symbols(path):
    """The labels of a JWasm listing: name -> (segment, value, kind), the
    kind "code" for a label of instructions or a procedure and "data" for
    anything else that has a place."""
    symbols = {}
    table = False
    with open(path, encoding="cp866") as listing:
        for line in listing:
            if line.startswith(("Procedures, parameters and locals:",
                                "Symbols:")):
                table = True
            if not table:
                continue
            match = re.match(r"(\S+) [ .]*P Near +([0-9A-F]+) +(\w+) ", line)
            if match:
                symbols[match[1]] = (match[3], int(match[2], 16), "code")
                continue
            match = re.match(r"(\S+) [ .]*?  +(\S+(?: Near)?) +"
                             r"([0-9A-F]+)h (\w+)", line)
            if match and match[2] not in ("Number", "Text"):
                kind = "code" if match[2].endswith("Near") else "data"
                symbols.setdefault(match[1],
                                   (match[4], int(match[3], 16), kind))
    return symbols


class Original:
    """The original program, as its source is laid out like the shipped
    binary: the bytes, where every label is, and which words hold the
    address of what."""

    def __init__(self, directory):
        self.image = (directory / "VC.COM").read_bytes()
        # Where each segment begins, counted from the PSP: the listing's
        # map of the file. The listing counts the code from 100h already.
        begins = {}
        with open(directory / "VC.LST", encoding="cp866") as listing:
            for line in listing:
                match = re.match(r"(\w+) +([0-9A-F]+) +([0-9A-F]+) +"
                                 r"([0-9A-F]+) +[0-9A-F]+\s*$", line)
                if match:
                    begins[match[1]] = (int(match[3], 16),
                                        int(match[3], 16) + int(match[4], 16))
        self.begins = {name: 0 if name == "_TEXT" else begin
                       for name, (begin, _) in begins.items()}
        self.ends = {name: end for name, (_, end) in begins.items()}
        self.labels = {}            # name -> address
        self.kinds = {}             # name -> "code" or "data"
        self.segment_of = {}
        for name, (segment, value, kind) in listed_symbols(
                directory / "VC.LST").items():
            if segment in self.begins:
                self.labels[name] = self.begins[segment] + value
                self.kinds[name] = kind
                self.segment_of[name] = segment
        self.places = sorted((address, name)
                             for name, address in self.labels.items())
        # The words that hold an address, and the address: from the same
        # source assembled as an object file.
        self.named = {}             # where the word is -> what it names
        module = ObjectFile(directory / "VC.OBJ")
        base = [None] + [self.begins[name]
                         for name, _, _ in module.segments[1:]]
        held = {number: module.contents(number)
                for number in range(1, len(module.segments))}
        for segment, offset, kind, target, displacement in module.fixups:
            if kind != TO_SEGMENT:
                raise Problem("the original names something that is not in "
                              "one of its segments")
            there = struct.unpack_from("<H", held[segment], offset)[0]
            address = base[target] + displacement + there & 0xFFFF
            self.named[base[segment] + offset] = address
        for where, address in self.named.items():
            if self.word(where) != address:
                raise Problem(f"the object file of the original and its "
                              f"program differ at {where:04X}h")

    def word(self, address):
        return struct.unpack_from("<H", self.image, address - COM_ORIGIN)[0]

    def extent(self, name):
        """How far the object at a label goes: to the next label."""
        address = self.labels[name]
        behind = bisect.bisect_right(self.places, (address, "\x7f"))
        end = self.ends[self.segment_of[name]]
        if behind < len(self.places):
            end = min(end, self.places[behind][0])
        return end - address

    def name_of(self, address):
        """The label that an address belongs to and how far behind it the
        address is: a label right there if there is one, of data before
        one of code, else the nearest before it."""
        at = bisect.bisect_right(self.places, (address, "\x7f"))
        if not at:
            raise Problem(f"nothing of the original is at {address:04X}h")
        nearest = self.places[at - 1][0]
        here = [name for where, name in self.places[:at][-8:]
                if where == nearest]
        here.sort(key=lambda name: self.kinds[name] != "data")
        return here, address - nearest

    def reserved(self, address, length):
        """Whether these bytes reach into room that the shipped binary has
        something in and the published source has not."""
        return any(address - COM_ORIGIN < offset + size
                   and offset < address - COM_ORIGIN + length
                   for offset, size, _, _ in REGIONS["VC.COM"])


class Port:
    """The port: the bytes of its file, and where its objects are."""

    def __init__(self, directory):
        self.image = (directory / "VC.EXE").read_bytes()
        self.segments = {}          # (name, class) -> where it begins
        self.places = defaultdict(list)     # name -> [address]
        self.objects = []           # (module, name, address, length)
        self.assembly = {}          # label -> (address, kind, segment)
        modules = defaultdict(list)     # module -> [(name, address)]
        module = None
        with open(directory / "VC.MAP") as text:
            for line in text:
                match = re.match(r"(\w+) +(\w+) +(\w+) +([0-9a-f]{4}):"
                                 r"([0-9a-f]{4}) +([0-9a-f]{8})$", line)
                if match:
                    self.segments[match[1], match[2]] = (
                        int(match[4], 16) * 16 + int(match[5], 16),
                        int(match[6], 16))
                match = re.match(r"Module: .*\((.*)\)$", line)
                if match:
                    module = Path(match[1]).stem
                match = re.match(r"([0-9a-f]{4}):([0-9a-f]{4})[*+s]? +(\S+)$",
                                 line)
                if match:
                    address = int(match[1], 16) * 16 + int(match[2], 16)
                    modules[module].append((match[3], address))
        if not modules:
            raise Problem("the linker's map lists no symbols")

        # The assembly, as long as there is any. It is linked first, so
        # that what it has of a segment begins where the segment does.
        ends = defaultdict(list)
        listed = {}
        if (directory / "VC.LST").exists():
            listed = listed_symbols(directory / "VC.LST")
            own = ObjectFile(directory / "VC.OBJ")
            for name, cls, length in own.segments[1:]:
                if (name, cls) in self.segments:
                    ends[name].append(self.segments[name, cls][0] + length)
        for name, (segment, value, kind) in listed.items():
            for (known, _), (begin, size) in self.segments.items():
                if known == segment:
                    self.assembly[name] = (begin + value, kind, segment)
                    ends[segment].append(begin + value)
                    break
        for name, (address, kind, segment) in self.assembly.items():
            self.places[name].append(address)
            if kind == "data" and segment not in ("RDATA", "RGAP"):
                end = min(there for there in ends[segment] if there > address)
                self.objects.append(("assembly", name, address, end - address))

        # The C code: an object of it is a symbol of a segment that is
        # no code, or one in the code that has an underscore in front, as
        # the compiler names data. It is as long as its object file says.
        for module, symbols in modules.items():
            if module == "VC":
                continue
            for name, address in symbols:
                self.places[name.lstrip("_")].append(address)
            compiled = ObjectFile(directory / f"{module}.obj")
            offsets = defaultdict(list)
            for name, segment, offset in compiled.symbols:
                offsets[segment].append(offset)
            for segment in offsets:
                offsets[segment].append(compiled.segments[segment][2])
            where = dict(symbols)
            for name, segment, offset in compiled.symbols:
                code = compiled.segments[segment][1] in ("CCODE", "RCODE")
                if code and not re.match(r"_[A-Za-z0-9]", name):
                    continue
                end = min(there for there in offsets[segment]
                          if there > offset)
                self.objects.append((f"{module}.c", name.lstrip("_"),
                                     where[name], end - offset))
        self.objects.sort(key=lambda found: (found[0] == "assembly",
                                             found[0], found[2]))

    def bytes(self, address, length):
        at = address - COM_ORIGIN + EXE_HEADER
        return self.image[at:at + length]


def compare(original, port, name, there, address, length):
    """Hold `length` bytes of the port at `address` against the original
    at `there`, for the object `name`. Returns None if they are the same,
    or a line that says where they are not."""
    ours = port.bytes(address, length)
    theirs = original.image[there - COM_ORIGIN:there - COM_ORIGIN + length]
    if len(theirs) < length:
        return f"{name} goes on beyond the end of the original"
    at = 0
    while at < length:
        named = original.named.get(there + at)
        if named is None or at + 2 > length:
            if ours[at] != theirs[at]:
                return (f"{name}+{at}: the port has {ours[at]:02X}h, the "
                        f"original {theirs[at]:02X}h")
            at += 1
            continue
        word = struct.unpack_from("<H", ours, at)[0]
        names, behind = original.name_of(named)
        said = names[0] + (f"+{behind}" if behind else "")
        how, label, _ = NAMED_OTHERWISE.get((name, at), ("", None, None))
        if how == "behind":
            wanted = [named - original.labels[label]]
            instead = f"how far {said} is behind {label}"
        elif how == "at":
            wanted = port.places.get(label, [])
            instead = f"where it has {label}"
        elif how == "is":
            wanted = [label]
            instead = "the number that it is to have"
        else:
            wanted = [place + behind & 0xFFFF for candidate in names
                      for place in port.places.get(candidate, [])]
            instead = f"where it has {said}"
            if not wanted:
                return (f"{name}+{at}: the original names {said} there, "
                        f"which the port has not by that name: see "
                        f"NAMED_OTHERWISE in {Path(__file__).name}")
        if word not in wanted:
            return (f"{name}+{at}: the port has {word:04X}h, which is not "
                    f"{instead}, "
                    f"{' or '.join(f'{w:04X}h' for w in wanted) or 'nowhere'}"
                    f": the original names {said} there")
        at += 2
    return None


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("layout", type=Path,
                        help="the directory of the original laid out like "
                             "the shipped binary: VC.COM, VC.LST, VC.OBJ")
    parser.add_argument("port", type=Path,
                        help="the directory of the port: VC.EXE, VC.MAP, "
                             "VC.LST and the object files")
    parser.add_argument("--quiet", action="store_true",
                        help="do not name every object that was compared")
    args = parser.parse_args()

    try:
        original = Original(args.layout)
        port = Port(args.port)
    except Problem as problem:
        sys.exit(f"datacheck: {problem}")

    failed = []
    count = defaultdict(lambda: [0, 0])
    stale = set(DIFFERENT) | set(ELSEWHERE) | set(OWN)
    used = set()
    for module, name, address, length in port.objects:
        kind = "assembly" if module == "assembly" else "C"
        label, behind = name, 0
        if kind == "C":
            stale.discard(name)
            if name in OWN:
                if not args.quiet:
                    print(f"  {module:12} {name:11} {length:4}  not "
                          f"compared: {OWN[name]}")
                continue
            label, behind = ELSEWHERE.get(name, (name, 0))
        if original.kinds.get(label) != "data":
            if kind == "C":
                failed.append(f"{module}: the original has no data called "
                              f"{label}: say in ELSEWHERE or in OWN of "
                              f"{Path(__file__).name} what {name} is")
            continue
        there = original.labels[label] + behind
        if original.reserved(there, length):
            if not args.quiet:
                print(f"  {module:12} {name:11} {length:4}  not compared: "
                      "in the room reserved for what only the shipped "
                      "binary has")
            continue
        difference = compare(original, port, name, there, address, length)
        used |= {key for key in NAMED_OTHERWISE if key[0] == name}
        if name in DIFFERENT:
            if difference is None:
                failed.append(f"{module}: {name} is as the original, and "
                              "listed as different")
            elif not args.quiet:
                print(f"  {module:12} {name:11} {length:4}  different: "
                      f"{DIFFERENT[name]}")
            continue
        if difference is not None:
            failed.append(f"{module}: {difference}")
            continue
        count[kind][0] += 1
        count[kind][1] += length
        if not args.quiet:
            whole = original.extent(label) - behind
            note = "" if length == whole else (
                f"  {length} of {whole} bytes" if length < whole else
                "  with what follows it")
            if label != name or behind:
                note += f"  {label}" + (f"+{behind}" if behind else "")
            print(f"  {module:12} {name:11} {length:4}  {there:04X}h{note}")
    for name in sorted(stale):
        failed.append(f"{name} is listed in {Path(__file__).name}, and the "
                      "C code has no constant of that name")
    for name, at in sorted(set(NAMED_OTHERWISE) - used):
        failed.append(f"{name}+{at} is listed in {Path(__file__).name}, "
                      "and the port has no object of that name")

    for kind in ("C", "assembly"):
        print(f"{count[kind][0]} objects of the {kind} code, "
              f"{count[kind][1]} bytes, are as the original")
    for line in failed:
        print(f"DIFFERS  {line}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
