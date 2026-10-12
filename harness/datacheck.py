#!/usr/bin/env python3
"""Hold the data of the C translation against the original's, byte for byte.

The scenarios compare screens and disks. A table that no screen shows can
be wrong and pass them: a text that is copied with what follows it, the
defaults of a setup, a colour of a box that never comes up. So every data
object of the translation that has the name of one of the original, a text,
a box, a dialog, a table, is compared with the original's bytes here, with
no machine involved.

The original is the build of the published source that is laid out like
the shipped binary, build/shareware-layout/VC.COM, which the test
asm-bytes holds against that binary. Its listing says where every label
is. The translation is build/c/VC.EXE, and the linker's map says where
each of its objects landed, static ones too; how long an object is, its
object file says. Nothing of the translation is assembled, and no
listing of it is read.

A word that holds the address of something else cannot be equal as a
number: a box that names its text, a dialog that names its box, a table of
commands. Which words those are is not described here. The original
source is assembled a second time, as an object file, whose relocations
name every such word and what it points at; here the word of the translation
is then held against where the translation has the thing of that name, with
the same distance from it: the offset in its group, which is what such a
word holds. So an object is written in C as it is in the assembly, with
the names of what it names, and the compiler and the linker see to the
addresses.

An object is compared as long as it is, and the program reads more of
some than their own bytes: a name that it copies as 13 bytes whatever
its length, a table of two colours of which a line may ask for the
fourth. Such an object is written as long as what is read, with the
original's bytes behind it, and then they are compared too. reads.py
finds what the code reads of each constant, and an object that is
shorter than that fails here; so does a text that a box or a dialog
names and that has no zero byte within itself.

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
import reads

COM_ORIGIN = 0x100          # where a .COM file is loaded, and the PSP ends
EXE_HEADER = 0x20           # of the translation's file: see tools/mkexe.c

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
    "Fnc_T": "the functions that FncAddr names by a number: the original "
             "has the address of a routine in that word, and no table",
    "Panel_T": "Brief_T and Full_T of the original, as two rows of four",
    "PowersOfTen": "what Number divides by: the original has them in its "
                   "instructions",
    "Scan_P": "the four colours of a box that the C draws without the "
              "table of one",
    "TrnBeg": "room for the header that DOS keeps in front of a block of "
              "memory, zeros: the original is loaded over that paragraph "
              "and has TrnBeg as a label of its code",
    "Stays": "what the part that stays in the memory keeps there, zeros "
             "in the file: the original lays it over the code that starts "
             "the program",
    **dict.fromkeys(
        ("ImgEnd", "RelTab", "RelCnt", "TrnPrW", "ResPrW", "DatPrW",
         "ResCnt"),
        "a number of how the file is laid out, which tools/mkexe.c writes "
        "there: the original's assembler works such numbers into its "
        "instructions"),
}

# The tables of the menus' commands. Where the original has the address
# of a command there, the translation has the number of the command: the
# CMD_ of src/c/vc.h that has the name of the original's label. So an item
# that names another command than the original's fails the test, though no
# screen shows a table of the menus until its item is chosen.
COMMAND_TABLES = {"Menu1_T", "Menu2_T", "Menu3_T", "Menu4_T", "MenuS_T",
                  "MenuC_T"}

# Objects of the translation that have the name of one of the original and
# are meant to be otherwise: name -> why. Each is still compared, and has to
# differ: an entry that no longer applies fails the test.
DIFFERENT = {
}

# Words of an object that hold an address in the original, where the
# translation has something else than the address of the thing of that name:
# (object, offset) -> (how, what, why). "at" is the address of the label
# `what` instead; "behind" is a number, how far what the original names
# there is behind the label `what` of the original; "is" is the number
# `what` itself, as where a routine is named by its place in a table of
# functions.
_MENUS = ("the C has the lines of the menus in one array and the pages of "
          "their help in another, and a menu by its place in them")
NAMED_OTHERWISE = {
    **{(dialog, 4): ("is", number,
                     "the routine that a dialog names for Enter and F10 is "
                     "a number, SUB_ of src/c/tables.h, by which Dialog "
                     "calls the function")
       for dialog, number in (("DelSe_B", 1), ("StAt1_B", 2), ("StAt2_B", 2),
                              ("Copy2_B", 3), ("Copy1_B", 4))},
    **{("MnuTab", row * 8 + 4): ("behind", "MenuLn1", _MENUS)
       for row in range(5)},
    **{("MnuTab", row * 8 + 6): ("behind", "MnuHlp1", _MENUS)
       for row in range(5)},
}


class Problem(Exception):
    pass


def commands():
    """The numbers of the commands: the names that src/c/vc.h has with CMD_
    in an enumeration, as the compiler counts them."""
    header = Path(__file__).resolve().parent.parent / "src" / "c" / "vc.h"
    text = re.sub(r"/\*.*?\*/", "", header.read_text(), flags=re.S)
    numbers = {}
    for body in re.findall(r"\benum\s*{(.*?)}", text, flags=re.S):
        if "CMD_" not in body:
            continue
        number = 0
        for entry in body.split(","):
            match = re.fullmatch(r"\s*(\w+)\s*(?:=\s*(\w+))?\s*", entry)
            if not match:
                raise Problem(f"an entry of the commands in {header.name} "
                              f"is not understood: {entry.strip()}")
            if match[2]:
                number = int(match[2], 0)
            numbers[match[1].removeprefix("CMD_")] = number
            number += 1
    if not numbers:
        raise Problem(f"{header.name} has no numbers of commands")
    return numbers


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


class Translation:
    """The translation: the bytes of its file, and where its objects are."""

    def __init__(self, directory):
        self.image = (directory / "VC.EXE").read_bytes()
        self.segments = {}          # (name, class) -> where it begins
        self.places = defaultdict(list)     # name -> [address]
        # name -> [(what a word has that names it, from where that
        # counts)]: the offset in its group, and where the group begins.
        self.named = defaultdict(list)
        self.objects = []           # (module, name, address, length)
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
                    self.named[match[3].lstrip("_")].append(
                        (int(match[2], 16), int(match[1], 16) * 16))
        if not modules:
            raise Problem("the linker's map lists no symbols")

        # What is no constant, though it has contents: the variables of
        # the C code that have a value in the file. The compiler has them
        # in a segment of their own, _DATA, and R_DATA for those of the
        # part that stays in the memory.
        self.variables = set()

        # The C code: an object of it is a symbol of a segment that is
        # no code, or one in the code that has an underscore in front, as
        # the compiler names data. It is as long as its object file says.
        # What has no value in the file is no object here: the stack of
        # the part that stays, in R_BSS.
        for module, symbols in modules.items():
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
                if compiled.segments[segment][0].endswith("_BSS"):
                    continue
                if compiled.segments[segment][0].endswith("_DATA"):
                    self.variables.add(name.lstrip("_"))
                end = min(there for there in offsets[segment]
                          if there > offset)
                self.objects.append((f"{module}.c", name.lstrip("_"),
                                     where[name], end - offset))
        self.objects.sort(key=lambda found: (found[0], found[2]))

    def bytes(self, address, length):
        at = address - COM_ORIGIN + EXE_HEADER
        return self.image[at:at + length]

    def end_of(self, address):
        """Where the object ends that has `address` in it, or None."""
        for _, _, start, length in self.objects:
            if start <= address < start + length:
                return start + length
        return None


def compare(original, translation, name, there, address, length):
    """Hold `length` bytes of the translation at `address` against the
    original at `there`, for the object `name`. Returns None if they are the
    same, or a line that says where they are not."""
    ours = translation.bytes(address, length)
    theirs = original.image[there - COM_ORIGIN:there - COM_ORIGIN + length]
    if len(theirs) < length:
        return f"{name} goes on beyond the end of the original"
    at = 0
    while at < length:
        named = original.named.get(there + at)
        if named is None or at + 2 > length:
            if ours[at] != theirs[at]:
                return (f"{name}+{at}: the translation has {ours[at]:02X}h, "
                        f"the original {theirs[at]:02X}h")
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
            wanted = [offset
                      for offset, _ in translation.named.get(label, [])]
            instead = f"where it has {label}"
        elif how == "is":
            wanted = [label]
            instead = "the number that it is to have"
        elif name in COMMAND_TABLES:
            numbers = commands()
            wanted = [numbers[candidate] for candidate in names
                      if candidate in numbers]
            instead = f"the number of the command {said} in src/c/vc.h"
        else:
            groups = {offset + behind & 0xFFFF: group
                      for candidate in names
                      for offset, group
                      in translation.named.get(candidate, [])}
            wanted = list(groups)
            instead = f"where it has {said}"
            if not wanted:
                return (f"{name}+{at}: the original names {said} there, "
                        f"which the translation has not by that name: see "
                        f"NAMED_OTHERWISE in {Path(__file__).name}")
        if word not in wanted:
            return (f"{name}+{at}: the translation has {word:04X}h, which "
                    f"is not {instead}, "
                    f"{' or '.join(f'{w:04X}h' for w in wanted) or 'nowhere'}"
                    f": the original names {said} there")
        # A text that is named so is read as far as a zero byte, and has
        # to have that within itself: behind it the translation has other
        # bytes than the original.
        if not how and original.segment_of[names[0]] == "CONST":
            text = groups[word] + word
            end = translation.end_of(text)
            if end is not None and \
                    0 not in translation.bytes(text, end - text):
                return (f"{name}+{at}: names {said}, a text without a "
                        f"zero byte of its own: what reads it goes on into "
                        f"what follows it, which is not what follows it in "
                        f"the original. Write it as far as the zero that "
                        f"the original has behind it")
        at += 2
    return None


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("layout", type=Path,
                        help="the directory of the original laid out like "
                             "the shipped binary: VC.COM, VC.LST, VC.OBJ")
    parser.add_argument("translation", type=Path,
                        help="the directory of the translation: VC.EXE, "
                             "VC.MAP and the object files")
    parser.add_argument("--quiet", action="store_true",
                        help="do not name every object that was compared")
    args = parser.parse_args()

    try:
        original = Original(args.layout)
        translation = Translation(args.translation)
    except Problem as problem:
        sys.exit(f"datacheck: {problem}")

    failed = []
    count = bytes_held = 0
    stale = set(DIFFERENT) | set(ELSEWHERE) | set(OWN)
    used = set()
    for module, name, address, length in translation.objects:
        stale.discard(name)
        if name in OWN:
            if not args.quiet:
                print(f"  {module:12} {name:11} {length:4}  not "
                      f"compared: {OWN[name]}")
            continue
        label, behind = ELSEWHERE.get(name, (name, 0))
        if original.kinds.get(label) != "data":
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
        difference = compare(original, translation, name, there, address,
                             length)
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
        count += 1
        bytes_held += length
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
    # An object is compared as long as it is. Where the code reads more
    # of it, the object has to be that long: see reads.py.
    failed += reads.check(translation.objects, translation.bytes,
                          translation.variables)
    for name, at in sorted(set(NAMED_OTHERWISE) - used):
        failed.append(f"{name}+{at} is listed in {Path(__file__).name}, "
                      "and the translation has no object of that name")

    print(f"{count} objects of the C code, {bytes_held} bytes, are as the "
          "original")
    for line in failed:
        print(f"DIFFERS  {line}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
