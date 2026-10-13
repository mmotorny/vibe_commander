#!/usr/bin/env python3
"""Hold the data of a C tree against the original's, byte for byte.

The scenarios compare screens and disks. A table that no screen shows can
be wrong and pass them: a text that is copied with what follows it, the
defaults of a setup, a colour of a box that never comes up. So every data
object of the translation that has the name of one of the original, a text,
a box, a dialog, a table, is compared with the original's bytes here, with
no machine involved.

The original is the build of the published source that is laid out like
the shipped binary, build/shareware-layout/VC.COM, which the test
asm-bytes holds against that binary. Its listing says where every label
is. The translation is the VC.EXE of a C tree, build/c/ for the C
translation and build/portable_c/ for the portable C, and the linker's map
says where each of its objects landed, static ones too; how long an object
is, its object file says. Nothing of the translation is assembled, and no
listing of it is read. The tree's source, where the numbers of the commands
are read and what the code reads of each constant, is the directory of
src/ that has the name of the build's, or the one that --source names.

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

The original is two programs, and a tree may have the second in itself:
the setup program, VCSETUP.COM, which goes into the portable C as a part
of the one program (TODO.md, "The setup program in the portable C: the
plan"). It is laid out, listed and assembled as an object file next to
the main program, build/shareware-layout/VCSETUP.COM, .LST and .OBJ, and
is a second original here. Many of its things have the names of the main
program's, with other contents, so a tree has every thing of the setup
program under the name of the assembly with St in front: StConf_B is
held against Conf_B of VCSETUP.COM, and a word of it that names Cnf01_S
there against where the tree has StCnf01_S. The setup program has texts
and tables that are the main program's byte for byte, too; a tree need
not have those twice. A word of the setup program's may name the main
program's thing of the same name, if the tree has none with St in front
and that thing is, as long as the tree has it, what the setup program
has there. For a tree that is declared to have no setup program, the C
translation, nothing is held against the second original.

An object is compared as long as it is, and the program reads more of
some than their own bytes: a name that it copies as 13 bytes whatever
its length, a table of two colours of which a line may ask for the
fourth. Such an object is written as long as what is read, with the
original's bytes behind it, and then they are compared too. reads.py
finds what the code reads of each constant, and an object that is
shorter than that fails here; so does a text that a box or a dialog
names and that has no zero byte within itself, unless COUNTED, or
SETUP_COUNTED for a thing of the setup program, says of the word that
names it how many bytes are read of the text there.

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
import coverage
import deviations
import reads

COM_ORIGIN = 0x100          # where a .COM file is loaded, and the PSP ends

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

# What a tree has in front of the name of a thing of the original's setup
# program, and the files of that program in the directory of the layout.
# An object of that name is held against the setup program's object of the
# name without it, if that program has data of that name: StConf_B against
# Conf_B of VCSETUP.COM. No data of the main program has such a name; if
# one ever does, the test fails for it and says so.
SETUP_PREFIX = "St"
SETUP_PROGRAM = "VCSETUP"
# The structure of the main program's listing that is its main data.
MAIN_DATA = "MainDat"

# As ELSEWHERE, for the constants of the setup program: the name that the
# tree has -> (the label of the setup program's listing, how far behind
# it). For one that the rule of the prefix does not find.
SETUP_ELSEWHERE = {
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

# As OWN, for the constants that one tree has and the other has not, since
# it is declared to show what the original does not show
# (src/<tree>/DEVIATIONS.md): tree -> name -> why. They are not compared.
OWN_OF = {
    "portable_c": {
        "NameVer_S": "the title of the Info panel, the name and the "
                     "version by which the portable C calls itself: the "
                     "original has one title for the Info, the help and "
                     "the banner, Title_S, which the portable C has too, "
                     "with what it is based on, and that is longer than "
                     "the panel is wide",
        "BasedOn_S": "the line below that title, what the portable C is "
                     "based on, where the original has the line across "
                     "the panel and below it two lines on the memory, "
                     "which the portable C has not",
    },
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

# Entries of ELSEWHERE and OWN that one tree has no constant for: tree ->
# name -> why. Each has to be gone from the tree.
_ONE_PROGRAM = ("the portable C is one program, which the linker lays out "
                "and DOS loads: ")
GONE = {
    "portable_c": {
        "TrnBeg": _ONE_PROGRAM + "it has no part of its own that it makes "
                  "a block of memory of",
        "Stays": _ONE_PROGRAM + "what the watch on DOS keeps is variables "
                 "of resident.c, and it keeps no copies of the setup and "
                 "the panels",
        **dict.fromkeys(
            ("ImgEnd", "RelTab", "RelCnt", "TrnPrW", "ResPrW", "DatPrW",
             "ResCnt"),
            _ONE_PROGRAM + "no tool writes numbers of its layout into "
            "it"),
    },
}

# Objects of one tree that are declared to be otherwise than the
# original's, in src/<tree>/DEVIATIONS.md: tree -> name -> (why, what is
# otherwise). The second is where the object departs from the original's,
# each place as where it is in the original's object, how many bytes of
# the original it is, and the bytes that the tree has there instead, none
# for bytes that are left out. The object is held against the original's
# with that done to it, and has to be exactly that.
_NO_OPTION = ("the portable C stays in the memory whole while a command "
              "runs, and its Options menu has no Memory allocation: ")
_NO_MAP = ("the portable C has no map of the memory, Alt+F5, Memory info "
           "of the Commands menu: ")
_SETUP = ("the portable C has the setup program in itself, the last item "
          "of its Options menu, More setup..., behind Save setup and a "
          "line across the menu: ")
_NAME = ("the portable C calls itself Vibe Commander 0.1, since it no "
         "longer is what the original is, and says what it is based on "
         "wherever the original named its version: ")
_NO_SETTINGS = ("the setup of the portable C has no setting for what the "
                "program has no more, Memory allocation and TSR manager "
                "of the Configuration and Screen output of Screen and "
                "Mouse, and what stood below them in the right half of "
                "the box stands further up: ")


def _field(number, links=None, place=None):
    """What a field of a dialog of the setup is declared to have
    otherwise, as an entry of DECLARED has it. `number` is the field's
    in the original's dialog, whose fields are 11 bytes each behind a
    header of 10. With `links`, the four fields that Up, Down, Left and
    Right go to from it, as the tree numbers them; with `place` too,
    where it is in its box, the row in the high byte. With neither, the
    field is left out."""
    at = 10 + 11 * number
    if links is None:
        return (at, 11, b"")
    if place is None:
        return (at + 1, 4, bytes(links))
    return (at + 1, 6, bytes(links) + struct.pack("<H", place))


def _lines(group, halves, blanks, spaces):
    """What the lines of a box of the setup are declared to have
    otherwise, as entries of DECLARED. A line of such a box is one text
    for the two groups that stand side by side. `halves` are the lines
    whose right half is that of a line further down: (the number of the
    line, where its right half begins, the number of the line that has
    the right half now, how many bytes of the original lie between the
    two halves), and those bytes are left out, so the right half is held
    against the original's too. `blanks` are the lines whose right half
    is spaces, with the two between the halves: (the number of the line,
    where its left half ends, how many bytes the original has from there
    to the zero), and the tree has the mark for `spaces` spaces there."""
    return {
        **{f"St{group}{line:02}_S": (
            _NO_SETTINGS + f"the right half of this line is that of the "
            f"original's {group}{from_:02}_S", ((where, count, b""),))
           for line, where, from_, count in halves},
        **{f"St{group}{line:02}_S": (
            _NO_SETTINGS + "the right half of this line is spaces, "
            f"{spaces} with those between the halves",
            ((where, count, bytes((0xFF, spaces))),))
           for line, where, count in blanks},
    }


DECLARED = {
    "portable_c": {
        "Title_S": (_NAME + "the title of the help's window, and without "
                    "its first two bytes the first line of the banner, is "
                    "the name with the version and what it is based on, "
                    "50 characters behind six spaces where the original "
                    "has 34 behind fourteen",
                    ((1, 35, b"\x06Vibe Commander 0.1, based on Volkov "
                             b"Commander 4.05"),)),
        "Name_S": (_NAME + "the name without a version, which the box of "
                   "F10 has in its title and in its question",
                   ((0, 6, b"Vibe"),)),
        "Copyr_S": (_NAME + "the second line of the banner has the "
                    "copyright of this program behind that of the "
                    "original's author",
                    ((44, 0, b", (C) 2026 Maksym Motorny"),)),
        "CrLf_S": (_NAME + "the name in the second of the two lines that "
                   "ask for Enter where the screen is not one that the "
                   "program can draw on, which stand behind the end of "
                   "the banner",
                   ((37, 6, b"Vibe"),)),
        "Menu3_T": (_NO_MAP + "the command of that item is left out, "
                    "F65_00", ((4, 2, b""),)),
        "Menu4_T": (_NO_OPTION + "the command of that item is left out, "
                    "M73_00; " + _SETUP + "nothing for the line, and the "
                    "command of that item, the number that CMD_SETUP of "
                    "vc.h is, 100, the last of the commands, are put in "
                    "at the end",
                    ((26, 2, b""), (32, 0, b"\x00\x00\x64\x00"))),
        "MenuLn": (_NO_MAP + "the line of that item is left out; "
                   + _NO_OPTION + "the line of that item is left out; "
                   + _SETUP + "the line across and the line of that item "
                   "are put in at the end",
                   ((369, 17, b""), (706, 19, b""),
                    (748, 0, b"\0 More setup...\0"))),
        "MnuHlp": (_NO_MAP + "the page of the help for that item is left "
                   "out, 30; " + _NO_OPTION + "the page of the help for "
                   "that item is left out, 52; " + _SETUP + "the line "
                   "across has no page of the help and neither has the "
                   "item, 0 and 0, which are put in at the end",
                   ((37, 1, b""), (62, 1, b""), (65, 0, b"\0\0"))),
        "MnuTab": (_NO_MAP + "the Commands menu is a row shorter, 0Fh "
                   "rows with its frame and its shadow, and the lines of "
                   "the Options menu and the pages of its help begin 17 "
                   "bytes and a byte further up, at 1FCh and 2Fh; "
                   + _NO_OPTION + "the Options menu is a row shorter "
                   "for that; " + _SETUP + "and two rows taller for "
                   "these, 14h rows in all",
                   ((19, 1, b"\x0F"), (27, 1, b"\x14"),
                    (28, 2, b"\xFC\x01"), (30, 2, b"\x2F\x00"))),
        "KeyTab": (_NO_MAP + "Alt+F5 chooses no item of the Commands "
                   "menu, and the seven keys of that menu below it choose "
                   "an item further up, 3 to 6 and 8 to 10; " + _NO_OPTION
                   + "Save setup, which Shift-F9 chooses in the menu, is "
                   "its item 15 and not 16",
                   ((80, 4, b""), (87, 1, b"\x03"), (91, 1, b"\x04"),
                    (95, 1, b"\x05"), (99, 1, b"\x06"), (103, 1, b"\x08"),
                    (107, 1, b"\x09"), (111, 1, b"\x0A"),
                    (119, 1, b"\x0F"))),
        "Key_S": (_NO_MAP + "the name of the key in the key bar, "
                  '"Memory", the 30th, is left out', ((180, 6, b""),)),
        "KeyC_T": (_NO_MAP + "Alt with F5 has no name in the two key "
                   "bars of the panels, and the three names behind "
                   '"Memory", Main, Label and Rescan, have a number less',
                   ((21, 1, b"\x1E"), (33, 1, b"\x1F"), (44, 1, b"\x00"),
                    (54, 1, b"\x00"), (181, 1, b"\x20"))),
        # The Configuration of the setup. Execute options has one line,
        # Quick execute commands, where Memory allocation was, and the
        # Keyboard options are two rows further up.
        **_lines("Cnf",
                 ((2, 23, 3, 53), (3, 24, 5, 106), (4, 20, 6, 83),
                  (5, 29, 7, 78), (6, 24, 8, 113), (7, 18, 9, 126),
                  (8, 32, 10, 118), (9, 24, 11, 120), (10, 28, 12, 103)),
                 ((11, 30, 34), (12, 6, 8)), 37),
        "StConf_B": (_NO_SETTINGS + "the fields of Memory allocation and "
                     "TSR manager, the 11th and the 13th, are left out, "
                     "and the fields go to each other as they stand now, "
                     "16 boxes and the buttons: a box of the left half "
                     "has to its right the box on its row or else the "
                     "one above that row, and a box of the right half "
                     "is on a row further up, with the box of its row "
                     "to its left",
                     (_field(0, (16, 1, 0, 10)), _field(1, (0, 2, 1, 10)),
                      _field(2, (1, 3, 2, 10)), _field(3, (2, 4, 3, 11)),
                      _field(8, (7, 9, 8, 15)), _field(9, (8, 10, 9, 15)),
                      _field(10),
                      _field(11, (9, 11, 0, 10), 0x329),
                      _field(12),
                      _field(13, (10, 12, 3, 11), 0x629),
                      _field(14, (11, 13, 4, 12), 0x729),
                      _field(15, (12, 14, 5, 13), 0x829),
                      _field(16, (13, 15, 6, 14), 0x929),
                      _field(17, (14, 16, 7, 15), 0xA29),
                      _field(18, (15, 0, 15, 16)))),
        # Screen and Mouse of the setup. The Other options and the Mouse
        # options are five rows further up, where Screen output was.
        **_lines("Scr",
                 ((1, 27, 6, 220), (2, 26, 7, 217), (3, 18, 8, 216),
                  (4, 19, 9, 215), (5, 8, 10, 214), (6, 30, 11, 225),
                  (7, 23, 12, 221)),
                 ((8, 21, 28), (9, 21, 8), (10, 21, 27), (11, 20, 30),
                  (12, 14, 8)), 34),
        "StScrn_B": (_NO_SETTINGS + "the field of Screen output, the "
                     "third, is left out, and the fields go to each "
                     "other as they stand now, five and the buttons: "
                     "Screen colors has Zooming boxes to its right and "
                     "Screen blank delay has Left-handed mouse, the last "
                     "of the right half; above Zooming boxes, which is "
                     "the first of the right half now, is the last line "
                     "of Screen blank delay, and to its left and to that "
                     "of the line below it is Screen colors, on its "
                     "first line",
                     (_field(0, (5, 0x81, 0, 2)), _field(1, (0xC0, 5, 1, 4)),
                      _field(2),
                      _field(3, (0xC1, 3, 0x80, 2), 0x323),
                      _field(4, (2, 4, 0x80, 3), 0x436),
                      _field(5, (3, 5, 0x81, 4), 0x723),
                      _field(6, (4, 0x80, 5, 5)))),
    },
}

# Words of an object that hold an address in the original, where the
# translation has something else than the address of the thing of that name:
# (object, offset) -> (how, what, why). "at" is the address of the label
# `what` instead; "behind" is a number, how far what the original names
# there is behind the label `what` of the original; "is" is the number
# `what` itself, as where a routine is named by its place in a table of
# functions; "field" is the place in the main data of its field `what`,
# and as far behind that as what the original names is behind its own
# label, for a variable of the setup program that a tree keeps in a field
# of another name: its BoxBuf in ErrBuf.
#
# A word of the setup program's that names one of that program's
# variables needs no entry, as a rule: see compare().
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

# As NAMED_OTHERWISE, for the words of the setup program's objects, by the
# names that a tree has them under. A tree that has no setup program has
# none of these objects, and no entry is held against it.
SETUP_NAMED_OTHERWISE = {
    ("StDfCol_P", 6): ("field", "ErrBuf",
                       "what the box of F8 in the list of colours covers "
                       "is kept in the setup program's BoxBuf, which is "
                       "as long as its WinBuf and lies behind it: the "
                       "main data has ErrBuf there"),
    # The help of the setup program.
    ("StHlpEr_P", 6): ("field", "ErrBuf",
                       "what the box of the setup's help covers is kept "
                       "in BoxBuf, which the main data has not: its "
                       "ErrBuf is as long, and is where the main "
                       "program's boxes of errors are kept"),
    ("StNoTim_P", 6): ("field", "ErrBuf",
                       "what the box \"Illegal format of\" covers, over "
                       "Panel options, which has WinBuf: in the setup "
                       "program's BoxBuf, the main data's ErrBuf"),
    ("StPanOp_B", 4): ("is", 5,
                       "the routine that Panel options names for Enter, "
                       "F04_10, is a number, SUB_StF04_10 of "
                       "src/portable_c/tables.h, by which Dialog calls "
                       "the function"),
}

# Words of an object that name a text which is not read as far as a zero
# byte, where a text that a word names is taken to be: (object, offset) ->
# (how many bytes of the text are read through that word, how that is
# known). The text has to be that long. As REVIEWED of reads.py, for what
# the code reads through a table and not by the constant's name. And the
# same for the setup program's objects.
COUNTED = {
}
SETUP_COUNTED = {
    **{("StPagTab", picture * 4): (
        25 * 58, "a picture of the colours is 25 rows of 58 characters, "
                 "which StCol00 of stcolor.c copies by those counts")
       for picture in range(4)},
}


class Problem(Exception):
    pass


def commands(source):
    """The numbers of the commands: the names that vc.h of the tree in the
    directory `source` has with CMD_ in an enumeration, as the compiler
    counts them."""
    header = source / "vc.h"
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
    """A program of the original, as its source is laid out like the
    shipped binary: the bytes, where every label is, and which words hold
    the address of what. `program` is the name of its files in
    `directory`, VC or VCSETUP, and `prefix` what a tree has in front of
    the names of that program's things: nothing for the main program's,
    and SETUP_PREFIX for the setup program's."""

    def __init__(self, directory, program="VC", prefix=""):
        self.directory = directory
        self.program = program
        self.prefix = prefix
        self.image = (directory / f"{program}.COM").read_bytes()
        # Where each segment begins, counted from the PSP: the listing's
        # map of the file. The listing counts the code from 100h already.
        begins = {}
        with open(directory / f"{program}.LST", encoding="cp866") as listing:
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
                directory / f"{program}.LST").items():
            if segment in self.begins:
                self.labels[name] = self.begins[segment] + value
                self.kinds[name] = kind
                self.segment_of[name] = segment
        self.places = sorted((address, name)
                             for name, address in self.labels.items())
        # The fields of the main data, as the main program's listing has
        # them, a structure: name -> where it is there. A tree has its
        # main data laid out so, in the segment that it reads through,
        # and so its word for a field holds this.
        self.fields = {}
        # The words that hold an address, and the address: from the same
        # source assembled as an object file.
        self.named = {}             # where the word is -> what it names
        module = ObjectFile(directory / f"{program}.OBJ")
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
                raise Problem(f"the object file of the original's "
                              f"{program} and its program differ at "
                              f"{where:04X}h")

    def word(self, address):
        return struct.unpack_from("<H", self.image, address - COM_ORIGIN)[0]

    def members(self, structure):
        """The members of a structure of this program's listing: name ->
        where it is in the structure."""
        found, inside = {}, False
        path = self.directory / f"{self.program}.LST"
        with open(path, encoding="cp866") as listing:
            for line in listing:
                if re.match(rf"{structure} [ .]*[0-9A-F]+\s*$", line):
                    inside = True
                elif inside:
                    match = re.match(r"  (\w+) [ .]*([0-9A-F]+)   \S", line)
                    if not match:
                        break
                    found[match[1]] = int(match[2], 16)
        if not found:
            raise Problem(f"{path.name} lists no structure {structure}")
        return found

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
                   for offset, size, _, _ in REGIONS[f"{self.program}.COM"])


def source_of(directory):
    """Where the tree is that is built in `directory`: build/c is of
    src/c."""
    root = Path(__file__).resolve().parent.parent
    return root / "src" / directory.resolve().name


class Translation:
    """The translation: the bytes of its file, and where its objects are.
    `source` is the directory of its C files; without one, it is the
    directory of src/ that has the name of `directory`."""

    def __init__(self, directory, source=None):
        self.source = source or source_of(directory)
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
                # Nor are the main data and the stack of the portable C,
                # each a segment of a class of its own.
                if compiled.segments[segment][1] in ("MAIN", "STACK"):
                    continue
                if compiled.segments[segment][0].endswith("_DATA"):
                    self.variables.add(name.lstrip("_"))
                end = min(there for there in offsets[segment]
                          if there > offset)
                self.objects.append((f"{module}.c", name.lstrip("_"),
                                     where[name], end - offset))
        self.objects.sort(key=lambda found: (found[0], found[2]))

    def bytes(self, address, length):
        at = address + coverage.origin(self.image)
        return self.image[at:at + length]

    def end_of(self, address):
        """Where the object ends that has `address` in it, or None."""
        for _, _, start, length in self.objects:
            if start <= address < start + length:
                return start + length
        return None


def whose(name, original, setup):
    """Which program of the original the object `name` of a tree is held
    against, and where: the program, the label there, and how far behind
    it. `setup` is the original's setup program, or None for a tree that
    has none. Returns a line that says what is wrong if neither has it."""
    label, behind = ELSEWHERE.get(name, (name, 0))
    of_main = original.kinds.get(label) == "data"
    there = None
    if setup and name in SETUP_ELSEWHERE:
        there = SETUP_ELSEWHERE[name]
    elif setup and name.startswith(SETUP_PREFIX):
        there = (name.removeprefix(SETUP_PREFIX), 0)
    # What has no value in the setup program's file is a variable.
    of_setup = there is not None and setup.kinds.get(there[0]) == "data" \
        and setup.segment_of[there[0]] != "_BSS"
    if of_main and of_setup:
        return (f"{name} is data of the original's main program, {label}, "
                f"and by the name that follows {SETUP_PREFIX} data of its "
                f"setup program too, {there[0]}: say in OWN of "
                f"{Path(__file__).name} which it is, or give it another "
                "name")
    if of_main:
        return original, label, behind
    if of_setup:
        return setup, *there
    return (f"the original has no data called {label}"
            + (f", and its setup program none called {there[0]}"
               if there else "")
            + f": say in ELSEWHERE or in OWN of {Path(__file__).name} what "
            f"{name} is")


def known(original, translation, candidate):
    """Where the translation has what `original`, a program of the
    original, calls `candidate`: what a word has that names it, and from
    where that counts, as Translation.named has it. That is under the
    name with the program's prefix. A thing of the setup program may be
    the main program's thing of the same name as well, where the tree
    has none with the prefix: but only if every object of that name is,
    as long as the tree has it, what the setup program has there."""
    found = translation.named.get(original.prefix + candidate, [])
    if found or not original.prefix or candidate not in original.labels:
        return found
    shared = [(address, length)
              for _, name, address, length in translation.objects
              if name == candidate]
    if shared and all(compare(original, translation, candidate,
                              original.labels[candidate], address,
                              length) is None
                      for address, length in shared):
        return translation.named.get(candidate, [])
    return []


def origins(length, otherwise=()):
    """For each of the `length` bytes of an object of the translation,
    where the original has it, counted from the start of its object, or
    the byte that is declared to be there instead: `otherwise` as
    DECLARED has it."""
    origin, behind = [], 0
    for where, count, instead in sorted(otherwise):
        origin += list(range(behind, where)) + [bytes([b]) for b in instead]
        behind = where + count
    return (origin + list(range(behind, behind + length)))[:length]


def held(length, otherwise=()):
    """How many bytes of the original's object an object of the
    translation of `length` bytes is held against, from its start to the
    last of them: its own length, unless it is declared to have more or
    fewer than the original, `otherwise` as DECLARED has it."""
    return max((o for o in origins(length, otherwise)
                if isinstance(o, int)), default=0) + 1


def compare(original, translation, name, there, address, length,
            otherwise=()):
    """Hold `length` bytes of the translation at `address` against the
    original at `there`, for the object `name`. Returns None if they are the
    same, or a line that says where they are not. `otherwise` is what the
    object is declared to have otherwise than the original's, as DECLARED
    has it: it is held against the original's with that done to it.
    `original` is the program of the original that has the object."""
    ours = translation.bytes(address, length)
    origin = origins(length, otherwise)
    last = held(length, otherwise) - 1
    theirs = original.image[there - COM_ORIGIN:there - COM_ORIGIN + last + 1]
    if len(theirs) < last + 1:
        return f"{name} goes on beyond the end of the original"
    at = 0
    while at < length:
        if isinstance(origin[at], bytes):
            if ours[at] != origin[at][0]:
                return (f"{name}+{at}: the translation has {ours[at]:02X}h, "
                        f"and is declared to have {origin[at][0]:02X}h "
                        "there")
            at += 1
            continue
        here = origin[at]
        named = original.named.get(there + here)
        if named is None or at + 2 > length or origin[at + 1] != here + 1:
            if ours[at] != theirs[here]:
                return (f"{name}+{at}: the translation has {ours[at]:02X}h, "
                        f"the original {theirs[here]:02X}h")
            at += 1
            continue
        word = struct.unpack_from("<H", ours, at)[0]
        names, behind = original.name_of(named)
        said = names[0] + (f"+{behind}" if behind else "")
        how, label, _ = NAMED_OTHERWISE.get(
            (name, at),
            SETUP_NAMED_OTHERWISE.get((name, at), ("", None, None)))
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
        elif how == "field":
            wanted = [original.fields[label] + behind & 0xFFFF]
            instead = (f"where the main data has {label}"
                       + (f"+{behind}" if behind else ""))
        elif original.prefix and original.segment_of[names[0]] == "_BSS":
            # A variable of the setup program, which is a program of
            # one segment. A tree has it as a variable of its own, by
            # the name with the prefix, or else as the field of that
            # name of the main data, which is what the setup program's
            # variables are while the setup is open.
            wanted = [offset + behind & 0xFFFF
                      for candidate in names for offset, _
                      in translation.named.get(original.prefix + candidate,
                                               [])]
            wanted = wanted or [original.fields[candidate] + behind & 0xFFFF
                                for candidate in names
                                if candidate in original.fields]
            instead = (f"where it has {original.prefix}{said}, or where "
                       f"the main data has {said}")
            how = "variable"
        elif name in COMMAND_TABLES:
            numbers = commands(translation.source)
            wanted = [numbers[candidate] for candidate in names
                      if candidate in numbers]
            instead = (f"the number of the command {said} in "
                       f"{translation.source.name}/vc.h")
        else:
            groups = {offset + behind & 0xFFFF: group
                      for candidate in names
                      for offset, group
                      in known(original, translation, candidate)}
            wanted = list(groups)
            instead = f"where it has {said}"
            if not wanted:
                return (f"{name}+{at}: the original names {said} there, "
                        f"which the translation has not by that name"
                        + (f", {original.prefix}{names[0]}, nor as the "
                           f"main program's {names[0]} with the setup "
                           f"program's bytes" if original.prefix else "")
                        + f": see NAMED_OTHERWISE in {Path(__file__).name}")
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
            counted = COUNTED.get((name, at)) \
                or SETUP_COUNTED.get((name, at))
            if counted:
                read = counted[0]
                if end is not None and end - text < read:
                    return (f"{name}+{at}: names {said}, of which {read} "
                            f"bytes are read there by COUNTED, and it is "
                            f"{end - text} bytes. Write it that long, "
                            f"with what the original has behind it")
            elif end is not None and \
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
                             "the shipped binary: VC.COM, VC.LST, VC.OBJ, "
                             "and the same of VCSETUP")
    parser.add_argument("translation", type=Path,
                        help="the directory of the translation: VC.EXE, "
                             "VC.MAP and the object files")
    parser.add_argument("--source", type=Path, metavar="DIRECTORY",
                        help="the directory of its C files: by default "
                             "the one of src/ that has the name of the "
                             "translation's directory")
    parser.add_argument("--quiet", action="store_true",
                        help="do not name every object that was compared")
    args = parser.parse_args()

    try:
        original = Original(args.layout)
        translation = Translation(args.translation, args.source)
        # The setup program, for a tree that has it in itself.
        original.fields = original.members(MAIN_DATA)
        setup = None
        if not deviations.declared(translation.source.name).no_setup:
            setup = Original(args.layout, SETUP_PROGRAM, SETUP_PREFIX)
            setup.fields = original.fields
    except Problem as problem:
        sys.exit(f"datacheck: {problem}")

    declared = DECLARED.get(translation.source.name, {})
    gone = GONE.get(translation.source.name, {})
    variables = reads.VARIABLES.get(translation.source.name, {})
    failed = []
    count = bytes_held = 0
    of_setup = bytes_of_setup = 0
    own = OWN | OWN_OF.get(translation.source.name, {})
    stale = (set(DIFFERENT) | set(ELSEWHERE) | set(own) | set(declared)
             | set(variables) | set(SETUP_ELSEWHERE if setup else ())) \
        - set(gone)
    used = set()
    for module, name, address, length in translation.objects:
        stale.discard(name)
        if name in gone:
            failed.append(f"{module}: {name} is listed in "
                          f"{Path(__file__).name} as gone from "
                          f"{translation.source.name}")
        if name in own or name in variables:
            if not args.quiet:
                print(f"  {module:12} {name:11} {length:4}  not "
                      f"compared: {own.get(name) or variables[name]}")
            continue
        found = whose(name, original, setup)
        if isinstance(found, str):
            failed.append(f"{module}: {found}")
            continue
        program, label, behind = found
        there = program.labels[label] + behind
        why, otherwise = declared.get(name, (None, ()))
        if program.reserved(there, held(length, otherwise)):
            if not args.quiet:
                print(f"  {module:12} {name:11} {length:4}  not compared: "
                      "in the room reserved for what only the shipped "
                      "binary has")
            continue
        difference = compare(program, translation, name, there, address,
                             length, otherwise)
        used |= {key for key in (*NAMED_OTHERWISE, *COUNTED,
                                 *SETUP_NAMED_OTHERWISE, *SETUP_COUNTED)
                 if key[0] == name}
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
        if program is setup:
            of_setup += 1
            bytes_of_setup += length
        if not args.quiet:
            whole = program.extent(label) - behind
            note = "" if length == whole else (
                f"  {length} of {whole} bytes" if length < whole else
                "  with what follows it")
            if program is setup:
                note += f"  {label} of {SETUP_PROGRAM}" \
                    + (f"+{behind}" if behind else "")
            elif label != name or behind:
                note += f"  {label}" + (f"+{behind}" if behind else "")
            # A second object of the name, a file's own copy of a text,
            # may end before the first place that is declared.
            if why and origins(length, otherwise) != list(range(length)):
                note += f"  but for what is declared: {why}"
            print(f"  {module:12} {name:11} {length:4}  {there:04X}h{note}")
    for name in sorted(stale):
        failed.append(f"{name} is listed in {Path(__file__).name}, and the "
                      "C code has no constant of that name")
    # An object is compared as long as it is. Where the code reads more
    # of it, the object has to be that long: see reads.py.
    failed += reads.check(translation.source, translation.objects,
                          translation.bytes,
                          translation.variables | set(variables))
    listed = set(NAMED_OTHERWISE) | set(COUNTED)
    if setup:
        listed |= set(SETUP_NAMED_OTHERWISE) | set(SETUP_COUNTED)
    for name, at in sorted(listed - used):
        failed.append(f"{name}+{at} is listed in {Path(__file__).name}, "
                      "and the translation has no object of that name")

    print(f"{count} objects of the C code, {bytes_held} bytes, are as the "
          "original" + (f", {len(declared)} of them but for what "
                        f"src/{translation.source.name}/DEVIATIONS.md "
                        "declares" if declared else ""))
    if setup:
        print(f"{of_setup} of them, {bytes_of_setup} bytes, are the "
              "original's setup program's")
    for line in failed:
        print(f"DIFFERS  {line}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
