#!/usr/bin/env python3
"""Check a C tree: the C translation, the program as it has been translated
to C, or the portable C, which began as a copy of that. Either is "the
translation" below, and the one that is checked is the one whose VC.EXE
and link map are named.

    ported.py build/c/VC.EXE
    ported.py build/portable_c/VC.EXE

The translation is taken through every scenario and must show the same
screens and leave the same disks as the original did, which the goldens
record (see compare.py and goldens.py). That alone would say little about a
routine no scenario reaches, so for the routines translated so far, listed
below, this also checks that

  - every instruction of the assembly they replace ran in the original, in
    some scenario, unless it is listed here with the reason why none gets
    there,
  - every C function that replaces them ran in the translation,
  - the C translation is no more than three times the size of the
    original, which the portable C is not held to, and
  - the translation leaves half of its stack untouched in every scenario.

Compiled code uses more of the stack than the assembly it replaces, so
the translation has a stack many times as long as the original's 256 bytes:
LenStk in src/c/resident.c. When the program has been loaded its stack
holds what its file has there, which is nothing, zeros; how much of that
is still there, from the bottom up, is looked at every time a step ends
with the program waiting for a key (see driver.py). A
stack used beyond its bottom overwrites what the program
saves of its panels and its setup while a command runs, and no screen
would show that. How long the stack is, is taken from the link map, so
the limit follows it. What every scenario leaves is reported, not only
the least.

A comparison that finds nothing is only worth something if it would have
found a difference. So one cell of a screen that the translation has shown
is changed, and then one entry is taken off a disk it has left, and the
comparison must report each.

The portable C is declared to behave otherwise than the original in some
things (src/portable_c/DEVIATIONS.md, and deviations.TREES for the
scenarios). A scenario that it is declared to show otherwise is held
against the original's golden with that difference made to it: the
steps that are left out for the tree, and the scenario's patch, a file
of the rows that are expected otherwise (see goldens.py). This says how
many scenarios were held against the original's golden as it is and
names the others, and fails for a patch that does not fit its golden,
before anything is run, and for one whose header names no entry of
DEVIATIONS.md that names the scenario. A scenario that only shows what
the tree has no more is not run, and a function that the tree has no
more is listed in GONE, with what took it away.

The original is two programs, and its setup program, VCSETUP.COM, goes
into the portable C as a part of the one program (TODO.md, "The setup
program in the portable C: the plan"). Its routines are listed below
too, apart, in SETUP_BATCHES: they are held against the listing of the
setup program, and against what ran of that program in the scenarios
that are of it (scenarios.Scenario.setup), where the others are held
against what ran of VC.COM in the scenarios of the main program. They
are checked for a tree that has the setup program, which the C
translation is declared not to have.

The build of the translation that is tested is the one that is shipped. What
ran in the original was recorded with the goldens; what runs in the
translation is read from QEMU's log of the code it executed (see
coverage.py). The assembly is identified by the listing of the build that is
laid out like the original, and the C functions by the translation's link
map.

The portable C is a flat program of 32 bits for DOS under an extender,
where the C translation is a program of 16 bits (TODO.md, "The portable C
in 32 bits: the plan"). Its file and the linker's map of it are read by
le.py, and what ran of it is known by its objects as the extender loads
them (coverage.py). Of such a program this holds besides:

  - The layout that the words of its addresses and the machine of the
    tests rest on, which is harness/layoutcheck.py's to say: this asks it
    before anything is run.
  - From QEMU's log, that the program was in the memory where the driver
    of the tests reads it (check_place).

One scenario by itself, with every row of every screen that differs:
compare.py build/portable_c/VC.EXE NAME, and with --first only where it
first departs; show.py --program for steps that are tried out.

Exits with status 0 if all of this holds and 1 otherwise.
"""
import argparse
import re
import sys
import textwrap
from dataclasses import dataclass, field, replace
from pathlib import Path

import compare
import coverage
import dataimage
import deps
import deviations
from driver import COM_ORIGIN, COPIES
import goldens
import layoutcheck
import le
import scenarios
import setups


# How large a tree's file may be, as a multiple of the original's size:
# tree -> the multiple. The C translation was held to this while it was
# made, as where growth must have gone wrong, and is still. The portable C
# is held to none, on the user's word of 12 October 2026: the setup
# program goes into it, which the original has as a file of its own, and
# nothing else goes by how long the file is. What the machine of the
# tests asks of its layout, check_layout() holds it to.
SIZE_LIMIT = {"c": 3}

# How much of its stack the translation must leave untouched in every
# scenario: one part in this many, half of it. The stack is made long enough
# for its use to need no care while the program is translated, and this is
# where that would have stopped being true. The rest is room for what no
# scenario has: a critical error of DOS at the bottom of deep calls, an
# interrupt that arrives there. The layout of a flat program goes by the
# same number: what a scenario may use of the stack is what must not
# share a page with code.
STACK_LEFT = coverage.STACK_LEFT


@dataclass(frozen=True)
class Batch:
    """Routines translated together."""
    title: str
    source: str                   # the C file, as src/c names it: FILES
    # The assembly procedures it replaces, and stretches of code between
    # procedures, each by its first label, or by a later one for what
    # stands from there on (see coverage.Listing).
    routines: tuple
    # Its functions that other files call, which the assembly called
    # while there was some: each has to have run in the translation.
    functions: tuple
    # Address in the original -> why no scenario executes the instruction.
    unreached: dict = field(default_factory=dict)
    # Address in the original -> why the instruction is not carried over
    # into the tree: for a routine of the setup program that is translated
    # in part, the rest of it being what a program of its own does when
    # it starts or ends and the one program has no place for. Such an
    # instruction is not asked to have run, and is not held to be
    # translated; whether it ran says nothing here.
    not_carried: dict = field(default_factory=dict)


BATCHES = [
    Batch(
        title="Numbers, sizes, dates and times as text",
        source="format.c",
        routines=("Number", "Numb10", "Numb20", "SzDtTm", "PutSize",
                  "PutDat0", "PutDate", "BufDate", "PutTim0", "PutTime",
                  "BufTime", "HexDec"),
        functions=("SzDtTm_c",),
        unreached={
            **dict.fromkeys(
                (0xA420, 0xA422, 0xA424, 0xA427, 0xA429, 0xA42B, 0xA42C,
                 0xA42D),
                "Number, for a country without a thousands separator: the "
                "harness knows of none to configure DOS for"),
            **dict.fromkeys(
                (0xAE02, 0xAE18),
                "PutDate and BufDate, for a country that writes the year "
                "first: the scenarios cover the United States and Russia"),
        },
    ),
    Batch(
        title="Lines of text with markup",
        source="line.c",
        routines=("LnStr", "NgLine", "NgLn30", "CntLine"),
        functions=(),
        unreached={
            **dict.fromkeys(
                (0x99CF, 0x99D0, 0x99D1, 0x99D2),
                "LnStr, for ^A and ^C: it measures texts of the program "
                "only, and none has either"),
            **dict.fromkeys(
                (0x9A9D, 0x9A9F, 0x9AA1, 0x9AA3, 0x9AA5, 0x9AA7, 0x9AA8,
                 0x9AAB, 0x9AAD, 0x9AAE, 0x9AB0, 0x9AB3, 0x9AB5, 0x9AB6,
                 0x9AB8, 0x9ABA, 0x9ABC, 0x9ABD, 0x9AC0, 0x9AC2, 0x9AC5,
                 0x9AC7, 0x9AC9),
                "NgLine, for ^A, ^C and a letter that means nothing: no "
                "text of the program or of its help file has any"),
        },
    ),
    Batch(
        title="Texts, paths and file names",
        source="string.c",
        routines=("AddFile", "MvzLin0", "MvzLine", "FilZero", "StrLen",
                  "LowCase", "FndLet", "CutPath", "NormStr", "PathLin",
                  "LetLin", "ShrtLin", "MainFil"),
        # None has a caller left in the assembly.
        functions=(),
    ),
    Batch(
        title="Colours and the screen buffer",
        source="screen.c",
        routines=("BasAdr", "Color", "ColAddr", "Shadow", "MvsBwc",
                  "MvsBwe", "MvsColr", "MovDz"),
        functions=(),
        unreached={
            0xA28F: "ColAddr, for a palette that is none of the three: "
                    "the Configuration dialog sets no other",
        },
    ),
    Batch(
        title="Hex digits and numbers that are typed",
        source="digits.c",
        routines=("HexByt", "HexCod", "TstHex", "TxtNum"),
        functions=(),
    ),
    Batch(
        title="File names in their fixed form, and masks",
        source="mask.c",
        routines=("Convert", "Conv10", "UnConv", "UnCnv1", "TstMas"),
        functions=(),
    ),
    Batch(
        title="The list of files of a panel: its filter and its order",
        source="files.c",
        routines=("SetMask", "TstExe", "Sort", "CmpPar", "CmpExt", "FnFile",
                  "NearFl", "TotSel"),
        functions=(),
    ),
    Batch(
        title="Where the panels are, and the cursor in them",
        source="place.c",
        routines=("BegBX", "SizDX", "LnsAX", "InvWCB", "AdrNum", "FilNum",
                  "NumAdr", "ChgAdr", "CorPar", "FilePan"),
        functions=("AdrNum_c", "NumAdr_c", "ChgAdr_c"),
    ),
    Batch(
        title="The keys of the panels: the cursor, files selected, the "
              "panels off, on, swapped and of another kind, and Tab",
        source="keys.c",
        routines=("F00_50", "F00_60", "F03_30", "F04_00", "F04_30",
                  "F05_00", "F08_10", "F76_00", "F20_02"),
        functions=("F00_60_c", "F00_00_c", "F00_05_c", "F01_00_c", "F01_05_c",
                   "F02_00_c", "F03_00_c", "F04_00_c", "F05_00_c", "F06_00_c",
                   "F07_00_c", "F08_00_c", "F76_00_c", "F76_01_c", "F09_00_c",
                   "F10_00_c", "F78_00_c", "F11_00_c", "F12_00_c", "F13_00_c",
                   "F14_00_c", "F15_00_c", "F16_00_c", "F17_01_c", "F79_00_c",
                   "F18_00_c", "F20_00_c", "F20_02_c"),
        unreached={
            **dict.fromkeys(
                (0x18C0, 0x18C3, 0x18C6),
                "F20_02, for a current directory that DOS has not "
                "told: the drives of the bench are always ready"),
            **dict.fromkeys(
                (0x18E2, 0x18E5, 0x18E7, 0x18EA, 0x18ED, 0x18F0, 0x18F2,
                 0x18F4, 0x18F5, 0x18FB, 0x1900, 0x1904, 0x1907, 0x1909,
                 0x190B),
                "F20_02, for a directory of the other panel that DOS "
                "will not go to: whatever removes or renames one has "
                "the panels read again, and the drives of the bench "
                "are always ready"),
        },
    ),
    Batch(
        title="A panel of files on the screen: its frame, its path, its "
              "files and the cursor",
        source="panel.c",
        routines=("Panel", "Panel8", "PutPath", "PtPt10", "PutName",
                  "PutNm5", "PutFls", "SetCur", "FilDir"),
        functions=(),
        unreached={
            0xAF9B: "SetCur, for a panel that shows neither files nor the "
                    "tree, or a list with nothing in it: it is called when "
                    "the cursor moves or a file is selected, and such a "
                    "panel has neither",
        },
    ),
    Batch(
        title="The tree of directories in a panel, and where in it the "
              "current directory is",
        source="tree.c",
        routines=("PutTre", "PutTree", "TreeCur", "TrCr10", "TrCr20"),
        functions=(),
        unreached={
            0xC1E9: "PutTree, for a tree with nothing in it: one that is "
                    "read has its root at the least, and the program "
                    "writes no tree file without",
        },
    ),
    Batch(
        title="The tree read from the disk and kept in its file, and the "
              "box in which a directory is chosen from it",
        source="treebox.c",
        routines=("ReadTre", "ChTree", "ChTr50"),
        functions=("ReadTre_c",),
        unreached={
            **dict.fromkeys(
                (0xBBC0, 0xBBC3, 0xBBC4, 0xBBC7, 0xBBC9, 0xBBCB, 0xBBCC,
                 0xBBCF, 0xBBD3, 0xBBD9, 0xBBDE),
                "ReadTre, for a disk that is not there: a drive that is "
                "not ready, a disk that the user would not put into a "
                "drive which stands for two, or a tree file that DOS "
                "opens and will not read. The bench has no such drive"),
            **dict.fromkeys(
                (0xBCE5, 0xBCEA),
                "ReadTre, with hidden directories left out of the tree: "
                "no menu has that option, only a setup file made "
                "elsewhere"),
            **dict.fromkeys(
                (0xBD02, 0xBD05, 0xBD07),
                "ReadTre, for more directories than the tree has room "
                "for, 766 and the root: a step makes one"),
            0xBD0E: "ReadTre, for a drive that is no longer ready while "
                    "its directories are read",
        },
    ),
    Batch(
        title="The Info panel: memory, the disk, the files of the other "
              "panel and the text of dirinfo",
        source="info.c",
        routines=("PutInf", "PtIn20"),
        functions=(),
        unreached={
            0xC38E: "PutInf, for a panel that shows neither files nor the "
                    "tree nor the Info: nothing sets such a kind",
            0xC48D: "PutInf, for files that take one byte in all: a file "
                    "takes whole clusters, unless the size of a cluster "
                    "is not known or is 64K, and the disks of the bench "
                    "have clusters of 512 bytes",
        },
    ),
    Batch(
        title="The menu bar and its pull-down menus",
        source="menu.c",
        routines=("F40_00", "F40_10", "F40_20", "F40_30", "F40_40",
                  "F40_50"),
        functions=("F39_00_c",),
        unreached={
            **dict.fromkeys(
                (0x2E6F, 0x2E71),
                "the Commands menu, for a display that is neither EGA "
                "nor VGA, where EGA lines is dark: the machine has a "
                "VGA"),
            **dict.fromkeys(
                (0x2F32, 0x2F34),
                "the menu bar, where a menu opens on an item that is "
                "dark and so is every item below it: the last item of "
                "every menu can always be chosen"),
        },
    ),
    Batch(
        title="The commands of the menus: the kind of a panel, its order "
              "and its filter, the tree in a box, the sizes of "
              "directories, EGA lines, two directories compared, the "
              "configuration, the files to edit and the setup saved",
        source="commands.c",
        # Four stretches of code that is no procedure, with the three
        # procedures between them: M01_00 with M01_01; M03_00 with M06_00,
        # M08_00, M16_00 and M16_10; M21_00 with M41_00, M42_00, F69_00
        # and M49_00; and M60_00 with the commands of the Options menu
        # down to M75_00, and on into the commands that VCF1.INC begins
        # with, which are listed with their own batch as F37_00.
        routines=("M01_00", "M02_10", "M03_00", "M16_20", "M21_00",
                  "M49_10", "M60_00"),
        functions=("M01_00_c", "M01_01_c", "M03_00_c", "M06_00_c",
                   "M06_10_c", "M08_00_c", "M16_00_c", "M16_01_c",
                   "M16_10_c", "M21_00_c", "M41_00_c", "M42_00_c",
                   "F69_00_c", "M49_00_c", "M60_00_c", "M61_00_c",
                   "M53_00_c", "M62_00_c", "M63_00_c", "M64_00_c",
                   "M66_00_c", "M67_00_c", "M69_00_c", "M70_00_c",
                   "M71_00_c", "M72_00_c", "M73_00_c", "M75_00_c"),
        unreached={
            0x3105: "M03_00, for a panel that is to show neither the tree "
                    "nor the Info: the menu comes with the third and the "
                    "fourth item, and the keys with a kind that is not 0, "
                    "but for ^R in a tree if DOS answered that it wrote "
                    "none of the one byte to the tree's file, which it "
                    "does not do to a file that it has read",
            0x32B2: "F69_00, for a display that is neither EGA nor VGA: "
                    "the bench has a VGA",
        },
    ),
    Batch(
        title="Reading what a panel shows: the files of a directory, and "
              "for the Info the disk, its label and the text of dirinfo",
        source="read.c",
        routines=("ReDir", "RdInfo", "GetVol", "DirSize", "TstInfo"),
        functions=("ReDir_c",),
        unreached={
            **dict.fromkeys(
                (0xB8BE, 0xB8C0, 0xB8C2, 0xB8C5, 0xB8C7, 0xB8C9, 0xB8CB,
                 0xB8CD, 0xB8CF),
                "ReDir, for a list too long to be kept while the "
                "directory is read: the room for it is the memory behind "
                "the panels, some 490K on the bench, and a list is 64K "
                "at the most"),
            **dict.fromkeys(
                (0xB95E, 0xB962, 0xB965, 0xB983, 0xB986),
                "ReDir, for a drive that has no current directory to go "
                "to when that of the panel is gone: DOS answers for "
                "every drive that it has, and the program offers no "
                "other"),
            0xB9E9: "ReDir, for more files than a panel has room for: "
                    "2700 of them, where a step makes one",
            0xBA61: "ReDir, for a drive that is not ready when the "
                    "extension file is opened, just after its directory "
                    "has been read",
            0xBF56: "RdInfo, for a panel that shows neither files nor the "
                    "tree nor the Info: nothing sets such a kind",
            **dict.fromkeys(
                (0xCBB7, 0xCBBA, 0xCBBC, 0xCBBE, 0xCBC1, 0xCBC2, 0xCBC4,
                 0xCBC6, 0xCBC7, 0xCBC9, 0xCBCA),
                "GetVol, for a drive of a network, which has the name it "
                "is shared by for a label: the bench has none"),
        },
    ),
    Batch(
        title="The viewer: the lines of a file, the part of it that is "
              "in the memory, what the screen shows of it, and a text "
              "looked for",
        source="view.c",
        routines=("NextLin", "PrevLin", "V20_00", "V25_00", "V30_00",
                  "V31_00", "V32_00", "V40_00", "V50_00", "V51_00",
                  "V55_00", "V60_00", "V81_00", "V82_00"),
        functions=("NextLin_c", "PrevLin_c", "V20_00_c", "V25_00_c",
                   "V30_00_c", "V31_00_c", "V32_00_c", "V40_00_c",
                   "V50_00_c", "V51_00_c", "V55_00_c", "V60_00_c",
                   "V81_00_c", "V82_00_c"),
        unreached={
            **dict.fromkeys(
                (0x5AC5, 0x5AD7, 0x5AD9, 0x5ADA),
                "PrevLin, for a place that is not where a line begins, "
                "or with another line of the table than its own: every "
                "caller has both from V20_00, from PrevLin itself or "
                "from the end of the table"),
        },
    ),
    Batch(
        title="The viewer: opening a file for it or for the editor, the "
              "keys and the mouse, a text looked for, and the way back to "
              "the panels",
        source="view.c",
        routines=("V00_00", "V80_00"),
        functions=("V00_02_c", "V00_10_c", "V13_00_c", "V13_01_c",
                   "V80_00_c"),
        unreached={
            **dict.fromkeys(
                (0x541D, 0x541F),
                "the viewer, for a line moved to the left as far as it "
                "goes, by 65455 columns: 1637 times Ctrl+Right, and a "
                "text that is found moves it by less than a part of a "
                "file is long"),
            0x549F: "the viewer, for Up to a line that has none behind "
                    "it: the line it came from is behind it",
            **dict.fromkeys(
                (0x54DE, 0x54E1, 0x54E3, 0x54E8, 0x54E9, 0x54EA, 0x54F7,
                 0x54FA),
                "the viewer, for Down on the last line of a part that "
                "ends in the first half of the memory, with more of the "
                "file to come: the table of lines takes the other half "
                "only where the file has a line end in every two bytes, "
                "16376 of them on the bench, which has no such file"),
            **dict.fromkeys(
                (0x595C, 0x595E),
                "the viewer, for a text found by reading the file on, "
                "less than 2000 bytes into it: the file is read on "
                "behind what is in the memory, and where the file has "
                "more, that is half of what the viewer has room for at "
                "the least, 32752 bytes on the bench"),
        },
    ),
    Batch(
        title="The editor: where in the memory its text is, what it shows "
              "of it and how it saves it",
        source="edit.c",
        routines=("T57_00", "T60_00", "T62_00", "T63_00", "T70_00",
                  "T71_00", "T72_00", "T73_00", "T73_10", "T74_00",
                  "T75_00", "T76_00", "T80_00"),
        functions=("T57_00_c", "T57_01_c", "T57_02_c", "T62_00_c",
                   "T63_01_c", "T70_00_c", "T71_00_c", "T72_00_c",
                   "T73_00_c", "T73_10_c", "T74_00_c", "T75_00_c",
                   "T76_00_c", "T80_00_c"),
        unreached={
            **dict.fromkeys(
                (0x778C, 0x778F, 0x7791, 0x7794),
                "T80_00, for a disk that is full, where it deletes what "
                "it has written: the hard disks of the bench have 32 MB "
                "free, and its floppy is write-protected"),
            **dict.fromkeys(
                (0x7824, 0x7827, 0x7829),
                "T80_00, for a file that DOS has written and closed and "
                "then refuses the attributes of"),
        },
    ),
    Batch(
        title="The editor: the line in its buffer, and from a line of the "
              "text to the next and the one before",
        source="edit.c",
        routines=("T50_00", "T51_00", "T53_00", "T53_10", "T54_00",
                  "T55_00", "T55_10", "T56_00", "T64_00"),
        functions=("T50_00_c", "T51_00_c", "T53_00_c", "T53_10_c",
                   "T54_00_c", "T55_00_c", "T56_00_c", "T64_00_c"),
        unreached={
            **dict.fromkeys(
                (0x7292, 0x7295, 0x7298),
                "T53_00 in hex, for 16 bytes that go beyond the end of "
                "their segment: the address it gets is one that T61_00 "
                "has made, with an offset below 16, or at most a window "
                "of lines further on"),
        },
    ),
    Batch(
        title="The editor: opening a file, the screen brought up to where "
              "the cursor is, and what every key does",
        source="edit.c",
        routines=("T00_00",),
        functions=("T00_00_c",),
        unreached={
            **dict.fromkeys(
                (0x6215, 0x6216),
                "T00_00, for a file that DOS will not read, or a key that "
                "stops the reading: the key would have stopped the "
                "viewer's code before, which looks for one when it has "
                "opened the file"),
            0x63B7: "T00_25, for a cursor that is below the window in a "
                    "line whose number is less than the window has rows: "
                    "the window then begins at line 0 and shows it",
        },
    ),
    Batch(
        title="The help: its file, the index and the pages in their "
              "window, and the keys",
        source="help.c",
        routines=("Help", "Help70", "Help71", "Help72", "Help77", "RdEntry",
                  "SeekDsk", "ReadDsk", "DeCode"),
        functions=(),
        unreached={
            0x7C17: "Help, for a click below the last line of the index: "
                    "the index has 62 lines, and the window shows 13 of "
                    "them, or 38 on a screen of 50 lines",
            0x7E26: "SeekDsk, for a file that is not where DOS was asked "
                    "to put it: DOS answers with the place asked for, "
                    "behind the end of the file too",
        },
    ),
    Batch(
        title="Find file: the search of a directory by name and by "
              "contents, its list, and the keys taken while it runs",
        source="find.c",
        routines=("FindFil", "FnFl60", "FndBack", "FndStuf", "FndMove",
                  "FndCurs", "FndLine", "FndAdr", "FndStat"),
        functions=(),
        unreached={
            0x8A7E: "FindFil, for more lines than the memory has room "
                    "for, some 7,000: the disks of the bench have fewer "
                    "files",
            **dict.fromkeys(
                (0x8BC8, 0x8BCA, 0x8BCC, 0x8BCE, 0x8BD0),
                "FindFil, for less than 64 KB of memory to read a file "
                "into: the bench has some 500 KB free"),
            **dict.fromkeys(
                (0x8C0D, 0x8C0E, 0x8C0F, 0x8C11, 0x8C14),
                "FindFil, stopped between two parts of a file that it "
                "reads, or by a part that DOS will not read: a key gets "
                "to a search only where it begins, with the key that "
                "starts it, or behind the help, and both are before the "
                "first file is opened; later it would be pressed at no "
                "moment that the harness can name"),
        },
    ),
    Batch(
        title="Deleting files and directories, setting attributes, and "
              "the files of a directory gone through",
        source="dirfunc.c",
        routines=("F38_10", "F38_40", "F38_60", "F38_70", "M30_50",
                  "M30_60", "DirFunc", "DelTree"),
        functions=("DirFunc_c", "DelTr0_c"),
        unreached={
            **dict.fromkeys(
                (0x3B78, 0x3B7B, 0x3B7E, 0x3B80, 0x3B83, 0x3B86, 0x3B89,
                 0x3B8B),
                "M30_50, for a file whose attributes DOS refuses for "
                "another reason than a disk that cannot be written: the "
                "file is one that DOS has just listed"),
            0x3C0B: "M30_60, for a country that writes the year first: "
                    "the scenarios cover the United States and Russia",
            **dict.fromkeys(
                (0xD881, 0xD888, 0xD88A, 0xD88D, 0xD890, 0xD892),
                "DirFunc, for a path of 68 characters or more: DOS has "
                "no directory with a path that long"),
        },
    ),
    Batch(
        title="The commands that make a directory, delete, set attributes "
              "and change the volume label, and the files of a mask",
        source="dirfunc.c",
        # Three stretches of code that is no procedure: F37_00 with
        # F47_00 and F38_00; F48_00 with F58_00 and M30_00; F54_00 with
        # FncMas.
        routines=("F37_00", "F48_00", "M30_80", "M30_82", "M30_83",
                  "F54_00"),
        functions=("F37_00_c", "F47_00_c", "F38_00_c", "F48_00_c",
                   "F58_00_c", "M30_00_c", "F54_00_c", "FncMas_c"),
    ),
    Batch(
        title="Find file: the command, its box, the drives it searches "
              "and Go to",
        source="find.c",
        routines=("F67_00",),
        functions=("F67_00_c",),
        unreached={
            **dict.fromkeys(
                (0x87C2, 0x87C4),
                "F67_00, for a drive that the CD-ROM extensions answer "
                "for: the bench has none"),
            0x89ED: "F67_00, Go to a directory that DOS has just searched "
                    "and will not change to, or with a current drive "
                    "whose disk the user will not put in: no scenario "
                    "has the floppy drive under its other letter for "
                    "the current one",
        },
    ),
    Batch(
        title="The dialogs: a box with its lines, the fields of a dialog "
              "and their keys, a line of buttons",
        source="dialog.c",
        routines=("DialBox", "DiBx30", "Dialog", "Dial85", "InpStr",
                  "InSt10", "Select", "Slct20", "FlipFl", "MinMnu",
                  "MnMn10", "MnMn40", "MnMn41", "MnMn50", "MnMn60"),
        functions=("Dialog_c",),
        unreached={
        },
    ),
    Batch(
        title="The user menu and the commands of its lines, the command "
              "line, the history, the speed search, and the box for the "
              "drive of a panel",
        source="usermenu.c, cmdline.c and drive.c",
        # F80_00 is Ctrl+I, with F10 and Shift+F2 behind it; F61_00 the
        # box of the drives, with the history behind it; and F81_00 the
        # command line and the speed search, with what makes the
        # commands of a line behind them.
        routines=("F80_00", "F42_10", "F32_00", "F32_60", "F32_70",
                  "F61_00", "F68_30", "F81_00", "FileLst"),
        functions=("F80_00_c", "F32_00_c", "F62_01_c",
                   "F68_00_c", "F82_01_c", "F74_00_c", "F73_00_c",
                   "F77_00_c"),
        unreached={
            0x2201: "The box of the drives, for a DOS that cannot say "
                    "which letter a floppy drive goes by",
            0x2245: "The box of the drives, for more drives than fit a "
                    "box as wide as its title: the bench has three",
            **dict.fromkeys(
                (0x2475, 0x2477, 0x247A),
                "The box of the drives, for a drive that is not ready: "
                "the bench has none"),
            0x2714: "F75_00, for the key that follows the cursor of the "
                    "tree arriving when the panel is no tree: Input "
                    "makes that key only for a tree",
            **dict.fromkeys(
                (0x29D6, 0x29D9),
                "FileLst, for a list file whose attributes DOS will not "
                "set though it is there: of a directory of that name "
                "FreeDOS sets them"),
            **dict.fromkeys(
                (0x2A76, 0x2A78),
                "FileLst, for a list that DOS will not write: the disk "
                "of the program is not full"),
        },
    ),
    Batch(
        title="The key bar, the menu bar and the command line",
        source="bars.c",
        # PutMnu3 loaded a 3, for Options lit, and ran into PutMnu: its
        # callers, which are C, hand PutMnu the 3.
        routines=("ClsLine", "PutKey", "PutKey0", "PutMnu3", "PutMnu",
                  "PutPrm", "DosCom"),
        functions=(),
    ),
    Batch(
        title="The entries of an extension file",
        source="extfile.c",
        routines=("FndItem", "FindExt"),
        functions=(),
    ),
    Batch(
        title="Waiting for a key, and a line that is typed into",
        source="input.c",
        routines=("Input", "Input0", "Rnd", "EdLin", "EdSub1", "EdSub2",
                  "CmpSym", "Search"),
        functions=(),
    ),
    Batch(
        title="Copying, renaming and moving a file or a directory",
        source="copy.c",
        routines=("F35_40", "C00_00", "C00_30", "C40_00", "C40_05",
                  "C40_10", "C50_00", "SetDate", "PerCent"),
        functions=(),
        unreached={
            **dict.fromkeys(
                (0x432B, 0x435A, 0x435D, 0x4BAB, 0x4BAE, 0x4CC7, 0x4CCA,
                 0x4CD9),
                "C00_00, for a drive that is not ready, or one that "
                "shares the floppy drive and whose disk the user will "
                "not put in, where no box is open for a file: the disks "
                "of the bench are always ready, and it has no second "
                "floppy drive to be asked for"),
            **dict.fromkeys(
                (0x44EF, 0x44F2),
                "C00_00, renaming onto a file that DOS will not delete "
                "though it is not read-only, or will not make writable"),
            0x46EA: "C00_00, for a device that gives something to copy: "
                    "the console is refused, NUL gives nothing, and the "
                    "bench has no other that comes to an end",
            0x4714: "C00_00, for a device that can be opened to look at "
                    "and not to write to",
            0x485D: "C00_00, for a file to append to whose last byte "
                    "cannot be read",
            **dict.fromkeys(
                (0x496C, 0x496E, 0x49B3, 0x49C3, 0x49C6),
                "C00_00, for a disk that is full and DOS says otherwise, "
                "or will not write what it says there is room for, or "
                "delete or give its attributes to what was written"),
            **dict.fromkeys(
                (0x4A3D, 0x4A40, 0x4A42, 0x4A44, 0x4A48, 0x4A4A, 0x4A4B,
                 0x4A4C, 0x4A4F, 0x4A51, 0x4A52, 0x4A53, 0x4A55, 0x4A57,
                 0x4A59, 0x4A5B, 0x4A5F, 0x4A61, 0x4A63, 0x4A65, 0x4A68,
                 0x4A6A, 0x4A6D, 0x4A6F, 0x4A71, 0x4A74, 0x4A76, 0x4A78,
                 0x4A79, 0x4A7B, 0x4A7F),
                "C00_00, going on with another disk when one is full, "
                "where it makes the directories of the target on the "
                "new one: a hard disk is not taken out, and the floppy "
                "of the bench is write-protected"),
            **dict.fromkeys(
                (0x4AA6, 0x4AA9),
                "C00_00, for a file that DOS has written and closed and "
                "then refuses the attributes of"),
            **dict.fromkeys(
                (0x4AE9, 0x4AEC, 0x4AEE, 0x4AF1, 0x4AF4, 0x4AF7, 0x4AF9,
                 0x4AFC, 0x4AFF, 0x4B02),
                "C00_00, moving a file that DOS will not delete once it "
                "is copied, though it is not read-only, or will not "
                "make writable"),
            0x4C31: "C00_00, for a directory that DOS will not remove, "
                    "and not because something is in it or because it "
                    "is the current one",
            0x4E03: "PerCent, for more read and written than twice the "
                    "size of the file: the size is what DOS says, or "
                    "for a device what was read",
        },
    ),
    Batch(
        title="What Copy and Rename ask before anything is copied",
        source="copy.c",
        routines=("F36_00", "F35_50", "F35_60", "F35_70"),
        functions=("F35_01_c", "F45_01_c"),
    ),
    Batch(
        title="The map of the memory: its lines, the vectors that point "
              "into them, and programs released",
        source="memory.c",
        routines=("F65_00", "MemAddr", "MemColr", "MemCurs", "MemEnd",
                  "MemItem"),
        functions=("F65_00_c",),
        unreached={
            **dict.fromkeys(
                (0x7F7D, 0x7F7E),
                "F65_00, for a version of DOS with two digits before "
                "the full stop"),
            **dict.fromkeys(
                (0x800D, 0x800E, 0x8011, 0x8013, 0x8014, 0x8016, 0x8018),
                "F65_00, for a DOS before version 4, which is asked "
                "whether it is DR DOS: the bench has FreeDOS, which "
                "says 7.10"),
            **dict.fromkeys(
                (0x8035, 0x8036, 0x8037, 0x803A),
                "F65_00, for a block of memory whose owner has no "
                "block of its own and is no number up to 8, "
                '"unknown": the bench has none'),
            **dict.fromkeys(
                (0x8052, 0x8054, 0x8058, 0x805A, 0x805B, 0x805C),
                "F65_00, for a DOS before version 4, where the first "
                "command processor is named by guessing"),
        },
    ),
    Batch(
        title="The main loop, the panels drawn and read, a drive that "
              "is not ready, and leaving",
        source="main.c",
        routines=("Init40", "Init46", "Mvz2lin", "Init49", "Init90",
                  "Func01", "Func15", "Func20", "Exit", "Quit", "RestVct",
                  "HookVct", "ErrRdy"),
        # HookVct has no caller left in the assembly.
        functions=("TrnMain_c",),
        unreached={
            **dict.fromkeys(
                (0x0D87, 0x0D88, 0x0D8B, 0x0D8C, 0x0D8D, 0x0D8F, 0x0D92,
                 0x0D93),
                "Mvz2lin, which nothing calls"),
            0x0E99: "Init60, where a drive has been chosen for a panel "
                    "that could not be read and none was noted as the "
                    "one that failed: Init59 notes the panel's drive "
                    "whenever a panel cannot be read",
            **dict.fromkeys(
                (0x108C, 0x108F, 0x1090, 0x1093, 0x1095, 0x1098, 0x109A,
                 0x109D, 0x109F, 0x10A1),
                "Func08, where the cursor of the tree has moved in the "
                "last 0.6 seconds of a minute, of an hour and of a day: "
                "the machine's clock is counted in instructions, of "
                "which the translation takes others than the original, so "
                "no step falls into that time for both"),
            0x10DA: "Func10, for a screen that is neither EGA nor VGA, "
                    "whose key bar has no Alt-F9: the bench has VGA",
            0x128C: "Func61, where the button of a second click comes up "
                    "off the entry: the pointer of the bench does not "
                    "move while a button is down",
            0x91E5: "ErRdy3, for the middle button or both in the box of "
                    "a drive that is not ready: a step of the bench "
                    "presses the left button or the right one",
        },
    ),
    Batch(
        title="DOS and the BIOS at every turn: the timer, the keys that "
              "wait, the bell, capitals, DOS with its errors, a file "
              "opened, the current directory, the disk of a drive",
        source="system.c",
        routines=("Timer", "ClrKbd", "TstBrk", "Beep", "UpCase", "SetDTA",
                  "TstDrv", "Intr21", "SetPth", "Fantom", "ChgDisk",
                  "OpenFil"),
        # ClrKbd, Beep and TstDrv have no caller left in the assembly.
        functions=(),
        unreached={
            **dict.fromkeys(
                (0xCC11, 0xCC13, 0xCC15),
                "OpenFil, for a file that DOS will not open because "
                "another program has it open: DOS says so only where "
                "files are shared, and on the bench nothing else has a "
                "file open"),
        },
    ),
    Batch(
        title="The mouse: finding it, its pointer, where it is, and a "
              "second click",
        source="mouse.c",
        routines=("IniMous", "TypMous", "SavMous", "CurMous", "GetMous",
                  "DoublMo", "InWind"),
        # TypMous and DoublMo have no caller left in the assembly.
        functions=(),
        unreached={
            **dict.fromkeys(
                (0x93F3, 0x93F5),
                "IniMous, for a mouse driver that does not know the "
                "call which resets it without the mouse, 21h, and is "
                "asked by call 0 instead: the driver of the bench, "
                "CuteMouse, knows it"),
            **dict.fromkeys(
                (0x9407, 0x940A, 0x940E, 0x9412),
                "IniMous, where there is no mouse: every machine of "
                "the bench has one"),
            **dict.fromkeys(
                (0x950D, 0x950F, 0x9512, 0x9514),
                "DoublMo, for a first click in the last 40 hundredths "
                "of a minute, where the time by which a second one "
                "counts is in the next minute, or the next hour: the "
                "machine's clock is counted in instructions, and the "
                "clicks of the scenarios do not fall there"),
        },
    ),
    Batch(
        title="The screen: its mode and its cursor, windows of the screen "
              "buffer kept, put back and put on the screen",
        source="window.c",
        routines=("ChkMode", "Cursor", "Cursor1", "ChgCur", "HidCur",
                  "Int10m", "ScrOffs", "SftEGA", "SavWin", "ResWin",
                  "ResUser", "Window", "SavScr", "MovScr"),
        # None has a caller left in the assembly.
        functions=(),
        unreached={
            **dict.fromkeys(
                (0x95A4, 0x95A6),
                "ScrOffs, for a screen that shows another page than its "
                "first: DOS has the bench's on page 0"),
            **dict.fromkeys(
                (0x968D, 0x968E, 0x9690, 0x9692, 0x9693, 0x9694, 0x9696,
                 0x96F5, 0x96F6, 0x96F9, 0x96FA, 0x96FC, 0x96FE, 0x96FF,
                 0x9702, 0x9703, 0x9704, 0x9706, 0x9708, 0x970A),
                "Window and MovScr, waiting to write where a display "
                "shows snow: a CGA, which the program finds out for "
                "itself as it starts. The bench has a VGA, and Snow is "
                "in no menu"),
            **dict.fromkeys(
                (0x96A7, 0x96A9),
                "Window, telling TopView or DESQview what has changed: "
                "the bench runs neither"),
        },
    ),
    Batch(
        title="The frame of a box, its shadow, and the box growing",
        source="box.c",
        routines=("MinBox", "MinBx3", "MinBx5", "ShadBox", "BoxDubl",
                  "ShadLin", "BoxLine", "BoxLn0", "StdLine", "OncZoom"),
        functions=(),
    ),
    Batch(
        title="Small routines: the end of a line, a switch turned over, a "
              "sum of bytes, a page up and down in a list",
        source="misc.c",
        # IsCrLf0, which stepped back two bytes for IsCrLf, has no caller
        # left: what called it is C and does that itself.
        routines=("IsCrLf0", "IsCrLf", "InvVar", "ChkSum", "PgUp", "PgDn"),
        functions=(),
    ),
    Batch(
        title="Enter and the keys that change the directory, the commands "
              "before and after, the commands that view and edit a file, "
              "and going through the selected files",
        source="enter.c, view.c and dirfunc.c",
        # F21_00 is Enter, as far as F21_40, which saves a panel while a
        # command runs; F22_00 the keys that change the directory, with
        # the history behind them and then what an earlier batch has
        # listed from F80_00 on; F43_00 the commands of the viewer and
        # the editor; FncSel is in the program's data, with FncExt.
        # IsCrLfS took a word of the text for IsCrLf, for F34_30 alone,
        # which reads it itself.
        routines=("F21_00", "F21_40", "F22_00", "F43_00", "F34_30",
                  "F34_40", "FncSel", "IsCrLfS"),
        functions=("F21_00_c", "F21_10_c", "F21_25_c", "F21_30_c",
                   "F21_31_c", "F22_00_c", "F23_00_c", "F24_00_c",
                   "F24_01_c", "F24_02_c", "F25_00_c", "F26_00_c",
                   "F27_00_c", "F30_00_c", "F75_00_c", "F43_00_c",
                   "F44_00_c", "F63_00_c", "F64_00_c", "F33_00_c",
                   "F34_00_c", "F34_10_c", "FncSel_c", "FncExt_c",
                   "FncEx1_c"),
    ),
    Batch(
        title="The setup: what its file holds, and the file written",
        source="setup.c",
        routines=("SaveSet", "MakeIni", "MkIni1"),
        # Only C code calls them: the main loop and Shift-F9.
        functions=(),
        unreached={
            0xC60C: "SaveSet, where the disk of the program's drive is "
                    "refused, or fewer bytes are written than the setup "
                    "has: the program is on D:, a hard disk, and a write "
                    "comes up short only on a disk that is full. D: could "
                    "be filled as copy-full fills C:, with the setup file "
                    "deleted first; that is the longest scenario by far, "
                    "once more, for this instruction",
        },
    ),
    Batch(
        title="The screen blanker, and the word that the program is idle",
        source="blank.c",
        routines=("LeaveTm", "Blank"),
        # Only C code calls them: Input0.
        functions=(),
        unreached={
            **dict.fromkeys(
                (0xB44A, 0xB44D, 0xB44F, 0xB452, 0xB454, 0xB457),
                "LeaveTm, giving the time to the other programs of "
                "DESQview: the bench does not run it"),
        },
    ),
    Batch(
        title="The size of a file added to that of its directory",
        source="read.c",
        routines=("DiSi10",),
        functions=(),
    ),
    Batch(
        title="The command of a key: the last table of keys that Case "
              "went by",
        source="main.c",
        routines=("Case",),
        functions=(),
    ),
    Batch(
        title="The start of the program: the setup file and the "
              "defaults, the memory for the main data, and what the "
              "machine says of itself",
        source="start.c",
        routines=("Init00", "CurNl", "OutStr1", "MainPth", "ReadOne",
                  "Expand"),
        functions=("Init00_c", "Init11_c", "IniData_c"),
        unreached={
            0x09E3: "Init11, where DOS refuses to make the block of the "
                    "code larger for another reason than too little "
                    "memory: the chain of its blocks would have to be "
                    "broken",
            0x0C1E: "Init32, where the status of the display has its "
                    "lowest bit set seven times running: QEMU's VGA "
                    "turns the bit over every time it is read",
            **dict.fromkeys(
                (0x0C44, 0x0C46),
                "Init34, under TopView or DESQview, which show a buffer "
                "in place of the display's memory: the bench has "
                "neither"),
            **dict.fromkeys(
                (0x0C6B, 0x0C6D, 0x0C74, 0x0C77, 0x0C79),
                "Init35, for a DOS before 5.0, which does not answer "
                "for its true version, and for version 4.00: the bench "
                "has FreeDOS, which is 7.10"),
        },
    ),
    Batch(
        title="The programs that stay in the memory: DOS's interrupts "
              "taken to note them, their records, and one released",
        source="resident.c",
        # CtrlC is code between procedures: the routine for Ctrl-C and
        # Ctrl-Break, and behind it New21 and New27, which the listing
        # has as far procedures. The functions are New21 and New27, where
        # the interrupts arrive, the interrupt functions that those go on
        # in, by the names that the compiler gives them, and Release,
        # which the map of the memory calls. CtrlC, an interrupt function
        # too, runs in no scenario, as in the original.
        routines=("CtrlC", "ExeProg", "AddProg", "FreeBlk", "ChkBlk",
                  "Release", "GetStat", "SetStat"),
        functions=("New21", "New27", "Int21_", "Int27_", "Release"),
        unreached={
            0x04AB: "CtrlC, where Ctrl-Break or DOS's Ctrl-C arrives: "
                    "the BIOS of the bench makes no interrupt of Ctrl "
                    "and ScrollLock, which was tried, and DOS does not "
                    "look for Ctrl-C in the calls that the program makes",
        },
    ),
    Batch(
        title="Commands run from the part that stays in the memory, the "
              "rest of the program brought back, and the program left",
        source="resident.c",
        # Exec is code between procedures, as far as NotFind: with the
        # commands run, Exec10, which loads the program again, AlocEr,
        # and Exit00 to Exit05. The assembly calls none of it any more.
        # The functions are SetBrk, which the part that goes calls, and
        # Exit01 with Exit05, the interrupt function that it goes on in:
        # where DOS goes on when it has ended the program, which the
        # scenario ended has it do.
        routines=("Exec", "NotFind", "OutStr0", "SetBrk0", "SetBrk"),
        functions=("SetBrk", "Exit01", "Exit05_"),
        unreached={
            0x032B: "Exec11, where DOS has no block for the part of "
                    "the program that is loaded again after a command: "
                    "the block that the program gave back for the "
                    "command is free again, since COMMAND.COM ran the "
                    "command in it and has ended. A program that stays "
                    "with most of the memory, which was tried, stays "
                    "behind that block",
        },
    ),
    Batch(
        title="The entry of the program: its check sum, the second copy "
              "that brings the program back, and the block of memory "
              "made of the part that goes",
        source="resident.c",
        # The code that begins the program runs on into the code that
        # starts it, Init00, which is another batch's: it is named from
        # its label to that one. DOS enters Alloc0, no stub does.
        routines=("Alloc0..Init00",),
        functions=("Alloc0",),
        unreached={
            0x072C: "Alloc0, where a second copy finds the file of "
                    "another length than the first copy had it: the "
                    "file would have to change while a command runs, "
                    "into another that runs",
            **dict.fromkeys(
                (0x0742, 0x0745, 0x0746, 0x0749, 0x074C, 0x074E),
                "Alloc3, for a DOS before 3.20: the bench has FreeDOS, "
                "which says 7.10"),
            **dict.fromkeys(
                (0x0779, 0x077B, 0x077D, 0x077F, 0x0781, 0x0783, 0x0785,
                 0x0786, 0x0788, 0x0789, 0x078C, 0x078D, 0x078E, 0x078F,
                 0x0791, 0x0793, 0x0794),
                "Alloc5, where DOS will not make the program's block "
                "as long as the program needs it and the part that "
                "goes is moved to a block elsewhere: how much memory "
                "there is to it follows the size of the program. The "
                "translation has no such way: its block holds its file, "
                "which is as long as the program in the memory"),
            **dict.fromkeys(
                (0x07D8, 0x07DB, 0x07DC, 0x07DF, 0x07E2, 0x07E5),
                "Alloc9, where there is no memory for the program "
                "when it begins"),
        },
    ),
    Batch(
        title="The critical error of DOS: the box that names the drive "
              "or the device and asks whether to try again",
        source="main.c",
        # FatErr is an interrupt function, which DOS calls and no stub:
        # it begins by loading DS with a number that is relocated, which
        # coverage.py knows of. In the portable C the entry alone is
        # the system's, DeviceEntry of sys_dos.c (see RENAMED below), and
        # calls FtErr1 of main.c, which the program hands the system.
        routines=("FatErr",),
        functions=("FatErr",),
        unreached={
            **dict.fromkeys(
                (0x90C9, 0x90CA, 0x90CD, 0x90CF),
                "FtErr1, for an error in the file allocation table of "
                "a disk, as DOS hands it over: the disks of the bench "
                "are whole"),
            0x9128: "FtErr5, for an error that DOS does not allow to be "
                    "tried again, where the box has Ok alone: FreeDOS "
                    "allows it for every error that the bench can make",
        },
    ),
]


# The routines of the original's setup program, VCSETUP.COM, as they are
# translated into a tree that has the setup program: batches like those
# above, with three things otherwise.
#
#   - `routines` are procedures and labels of the setup program's listing,
#     build/shareware-layout/VCSETUP.LST, and the addresses of `unreached`
#     and `not_carried` are of that listing. What has to have run of them
#     ran in VCSETUP.COM, in a scenario of the setup program.
#     `python3 harness/reach.py --setup NAME...` says what is left of a
#     routine, as reach.py says it of the main program's.
#   - A routine that the setup program has word for word as the main
#     program has it is not listed here: it is not translated twice, and
#     the batch above that has it holds it.
#   - `functions` are named as the tree has them, with St in front of the
#     routine's name: StMain00 for Main00.
#
# A batch of the plan's pieces, as an example of the form:
#
#     Batch(
#         title="The setup program: its menu",
#         source="stmain.c",
#         routines=("Main00",),
#         functions=("StMain00",),
#         unreached={
#             0x0417: "Main10, for ...: no scenario can ...",
#         },
#         not_carried={
#             **dict.fromkeys(
#                 (0x0405, 0x0408),
#                 "the screen saved when the setup program starts: the "
#                 "one program has it saved already"),
#         },
#     ),
SETUP_BATCHES = [
    Batch(
        title="The setup program: entered and left, its setup file, its "
              "menu, the setup that the program has without a file, and "
              "what its key bars and its dialogs do otherwise",
        source="stmain.c",
        # Init15 is the start of the setup program from where it reads
        # the setup file, with the box for a file that is missing or
        # wrong and the menu behind it, as far as Main40. Dialog is the
        # setup program's copy of the main program's, which differs from
        # it: its fields run in the scenarios of the options. Main50,
        # the mouse at the menu, is the main program's F32_70 word for
        # word under a name of its own, and the menu calls that: it is
        # listed here because nothing but its text says that the two are
        # one, and a routine that is listed has to have run whole.
        routines=("Init15", "Main40", "Main50", "F08_00..F09_00", "Exit",
                  "Default", "PutKey0", "Dialog"),
        functions=("StSetup", "StUser"),
        unreached={
            **dict.fromkeys(
                (0x039B, 0x039D),
                "the mouse set up when the setup is entered, for a mouse "
                "driver that does not know the call which resets it "
                "without the mouse, 21h, and is asked by call 0 instead: "
                "the driver of the bench, CuteMouse, knows it"),
            **dict.fromkeys(
                (0x1BFD, 0x1CC0, 0x1D8E, 0x1DB1),
                "Dialog, the buttons of a box that begins in the right "
                "half of the screen: every box of the setup program "
                "begins in the left"),
            **dict.fromkeys(
                (0x1ECE, 0x1ED0, 0x1ED2),
                "Dialog, F10 and Alt-F10 as answers: no dialog of the "
                "setup program has the flag for them"),
        },
        not_carried={
            **dict.fromkeys(
                (0x0228, 0x022B, 0x022D, 0x0231, 0x0233, 0x0236, 0x0238,
                 0x0239, 0x023C, 0x023E, 0x0242, 0x0246, 0x0249, 0x024B,
                 0x024D, 0x0250, 0x0252),
                "Ctrl-C not looked for at every call of DOS, and Ctrl-C "
                "and Ctrl-Break taken, when the setup program starts: "
                "the one program has both so since it started"),
            **dict.fromkeys(
                (0x0253, 0x0256, 0x0259, 0x025B, 0x025E, 0x0261, 0x0264,
                 0x0265, 0x0266, 0x0268, 0x026A, 0x026C, 0x026D, 0x0271,
                 0x0272, 0x0274, 0x0275, 0x0276, 0x0278, 0x027A, 0x027B,
                 0x027D, 0x027F, 0x0281, 0x0283, 0x0285, 0x0289, 0x028B),
                "the country and the table of the small letters, asked "
                "for when the setup program starts: the one program has "
                "them, of IniData"),
            **dict.fromkeys(
                (0x029C, 0x029E, 0x02A0, 0x02A2, 0x02A5),
                "the mode and the page of the screen, asked for when the "
                "setup program starts: the one program has them, of "
                "IniData"),
            **dict.fromkeys(
                (0x02B1, 0x02B4, 0x02B6, 0x02B9, 0x02BC, 0x02BE, 0x02C1,
                 0x02C6, 0x02C8, 0x02CC, 0x02CE),
                "the shape of the cursor corrected when the setup "
                "program starts: the one program has corrected it, in "
                "IniData, and the setup notes the shape as it is"),
            **dict.fromkeys(
                (0x02D5, 0x02DA, 0x02DC, 0x02DE, 0x02E0, 0x02E1, 0x02E2,
                 0x02E4, 0x02E6, 0x02E8, 0x02EA, 0x02EC, 0x02ED, 0x02EE,
                 0x02F0, 0x02F2, 0x02F4, 0x02F6, 0x02FB, 0x02FD, 0x02FF,
                 0x0303, 0x0305, 0x0306, 0x0307, 0x0309, 0x030C, 0x030E,
                 0x030F, 0x0310, 0x0312, 0x0314, 0x0315, 0x0317, 0x0319,
                 0x031B, 0x031D, 0x0320, 0x0321, 0x0323, 0x0325, 0x0327,
                 0x0329, 0x032B, 0x032D, 0x0331, 0x0332, 0x0336, 0x0339,
                 0x033E, 0x0340, 0x0343, 0x0347, 0x0348, 0x034A, 0x034C,
                 0x034E, 0x0350, 0x0351, 0x0353, 0x0355, 0x0357, 0x035C,
                 0x035F, 0x0363, 0x0365, 0x0369, 0x036C, 0x036F, 0x0372,
                 0x0374, 0x0375, 0x0378),
                "what the setup program finds out about the display when "
                "it starts: an EGA and its lines and where the cells of "
                "the screen are, which the one program has, of IniData; "
                "and a buffer of TopView, DESQview, and a display that "
                "shows snow, which the portable C does not know of "
                "(src/portable_c/DEVIATIONS.md)"),
            **dict.fromkeys(
                (0x0FB0, 0x0FB1, 0x0FB5, 0x0FB8, 0x0FBA, 0x0FEA, 0x0FEE,
                 0x0FF1),
                "Ctrl-Break given back and Ctrl-C looked for as before, "
                "when the setup program ends: the one program goes on, "
                "and keeps both as it has them"),
            **dict.fromkeys(
                (0x1053, 0x1056, 0x1059, 0x105C, 0x105E, 0x1061, 0x1063,
                 0x1065),
                "the call of DOS by which the setup program asks a main "
                "program that is running where its setup is: the one "
                "program knows, and does what follows the call with its "
                "own setup"),
            **dict.fromkeys(
                (0x1097, 0x109A),
                "the setup program ended, by DOS's call: the one program "
                "goes on"),
            0x109C:
                "what the setup program has Ctrl-C and Ctrl-Break go to, "
                "a routine that only returns: the one program has its "
                "own, NothingEntry of sys_dos.c",
        },
    ),
    Batch(
        title="The setup program: its help, the pages read from the help "
              "file and one of them in a window",
        source="sthelp.c",
        # RdHelp alone calls SeekDsk and ReadDsk. DeCode, which it calls
        # too, is the main program's word for word, and DeCode of help.c
        # serves both: the batch of the main program's help holds it.
        routines=("RdHelp", "Help", "HlpLine", "SeekDsk", "ReadDsk"),
        functions=("StRdHelp", "StHelp"),
        unreached={
            0x1482: "SeekDsk, for DOS having gone to another place of the "
                    "help file than it was asked for: it does not",
        },
        not_carried={
            **dict.fromkeys(
                (0x113D, 0x1140, 0x1142),
                "RdHlp2, the line that the setup program prints through "
                "DOS when it starts and cannot read the help file, "
                "\"Error reading data from the help file.\": the setup "
                "of the one program says nothing of what a program of "
                "its own said at its start (src/portable_c/"
                "DEVIATIONS.md)"),
        },
    ),
    Batch(
        title="The setup program: the list of the colours of a palette",
        source="stcolor.c",
        # F05_00 is the three items of the menu, F5, F6 and F7, and the
        # list behind them as far as it is left, F07_57: the code of the
        # setup program up to F07_60, the first of the routines that it
        # calls. The menu calls three functions for it, one for each
        # palette.
        routines=("F05_00", "F07_60", "F07_65", "F07_70", "F07_75",
                  "F07_80", "F07_85"),
        functions=("StF05_00", "StF06_00", "StF07_00"),
        unreached=dict.fromkeys(
            (0x07F2, 0x09C7, 0x0B39),
            "for a monochrome display, mode 7, which has \"Monochrome\" "
            "for its name and the second colour of each pair in a "
            "palette: the bench has a colour display"),
    ),
    Batch(
        title="The setup program: the picture in which the colour of an "
              "item of a palette is chosen",
        source="stcolor.c",
        # Col00 is the picture from where the list enters it to where it
        # is left for the list, Col55, with the mouse in it and the cells
        # painted anew for another colour: the code up to F08_00. Nothing
        # but the list calls it, so it has no function that another file
        # calls. ShadCol is what a colour becomes in a shadow, a routine
        # that the setup program has by itself and the main program
        # inside Shadow: the function is of screen.c, for both.
        routines=("Col00..F08_00", "ShadCol"),
        functions=("ShadCol",),
    ),
    Batch(
        title="The setup program: the Configuration, Screen and Mouse, "
              "the Panel options with the delay that is typed there, and "
              "the Country info",
        source="stopts.c",
        # F02_00 is the stretch with F03_00 and F04_00, as far as F04_10.
        routines=("F02_00", "F04_10", "F09_00", "F09_10", "F09_20"),
        functions=("StF02_00", "StF03_00", "StF04_00", "StF04_10",
                   "StF09_00"),
        unreached={
            0x0F4E:
                "Country info, the date of a country that writes the "
                "year first: the benches are the United States and "
                "Russia, which write the month or the day first",
            0x0F68:
                "Country info, the number of a country that has no "
                "character between thousands: both benches have one",
        },
        not_carried={
            0x06FF:
                "SetSnow called behind Screen and Mouse, which finds out "
                "whether to wait for a display that shows snow: the "
                "portable C has no such wait "
                "(src/portable_c/DEVIATIONS.md)",
        },
    ),
]


# Functions of the batches that a tree has no more: tree -> function ->
# why. Each has to be gone from the tree's link map.
GONE = {
    "portable_c": {
        "M73_00_c": "Memory allocation of the Options menu: the portable "
                    "C stays in the memory whole and has no such option",
        "F65_00_c": "the map of the memory, Alt+F5: the portable C has "
                    "no such map",
        "F69_00_c": "EGA lines, Alt+F9, which gives the screen 43 or 50 "
                    "rows by a font of the BIOS: the portable C has no "
                    "such command and takes the rows of the screen as it "
                    "finds them",
        **dict.fromkeys(
            ("New27", "Int27_", "Release"),
            "of the watch on the programs that stay in the memory, which "
            "the map of the memory lists and releases: the portable C "
            "keeps no such watch, does not take INT 27h, and releases "
            "nothing"),
        **dict.fromkeys(
            ("New21", "Int21_"),
            "the entry of INT 21h, for that watch and for the program's "
            "own call, by which the original's setup program asks a "
            "program that is running for its data: the portable C has "
            "the setup in itself, nothing makes that call, and it does "
            "not take INT 21h"),
        **dict.fromkeys(
            ("Exit01", "Exit05_"),
            "where DOS goes on when it has ended the program by another "
            "program's hand: under an extender nothing of the program "
            "can run then, and DOS goes on in a few instructions of real "
            "mode that sys_dos.c keeps below the megabyte, Ended_T there, "
            "which are bytes of a table and no function. That they ran "
            "is held by what the scenario ended shows of the registers"),
        "T62_00_c": "the address of the first line on the editor's "
                    "screen, a segment and an offset made of its place: "
                    "the text of the portable C is a pointer and a line "
                    "is known by its place, EdFirst",
    },
}


# Functions of the batches that a tree has under another name: tree ->
# function -> the name that its link map has for it, and why.
RENAMED = {
    "portable_c": {
        "TrnMain_c": ("Main_", "the program from when DOS has loaded it: "
                      "the original's part that stays in the memory "
                      "jumps to the rest of it there, which is no other "
                      "part in the portable C"),
        "FatErr": ("DeviceEntry_", "the entry of DOS's critical error, "
                   "which the portable C takes through its extender: "
                   "DeviceEntry of sys_dos.c, a routine of that file's own, "
                   "which the extender enters from real mode where DOS "
                   "enters FatErr of the original, an interrupt's "
                   "routine"),
        "Alloc0": ("Start_", "where the program is entered, by the "
                   "extender and not by DOS: Start of sys_dos.c, which "
                   "calls Main"),
    },
}


# The C files of the batches that a tree has under another name: tree ->
# the file as src/c names it, which is what a batch says -> the tree's
# name for it, and why. It is for what is printed: a batch's functions
# are found in the link map, whatever their file.
FILES = {
    "portable_c": {
        "resident.c": ("exec.c", "nothing of the portable C stays in the "
                       "memory apart from the rest: the file is the "
                       "running of the commands, with what the program "
                       "does first and when it leaves"),
    },
}


def check_original(listing, ran, batches):
    """Report on the assembly that `batches` say has been replaced;
    return success.

    `listing` is that of the program whose routines they name, and `ran`
    holds the offsets in that program's file of the bytes that ran.
    """
    passed = True
    for batch in batches:
        for name in batch.routines:
            if name not in listing.procedures:
                passed = False
                print(f"  {batch.title}: the listing has no procedure "
                      f"and no code between procedures called {name}")
        every = [line for name in batch.routines
                 if name in listing.procedures
                 for line in listing.procedure(name)]
        lines = [line for line in every
                 if line.address not in batch.not_carried]
        missed = [line for line in lines
                  if line.address - COM_ORIGIN not in ran]
        excused = [line for line in missed if line.address in batch.unreached]
        stale = sorted(set(batch.unreached)
                       - {line.address for line in missed})
        print(f"{batch.title}: {len(lines) - len(missed)} of {len(lines)} "
              f"instructions of the original ran, {len(excused)} more are "
              "listed as out of reach"
              + (f"; {len(every) - len(lines)} are not carried over"
                 if batch.not_carried else ""))
        for line in missed:
            if line.address not in batch.unreached:
                passed = False
                print(f"  not executed: {line.address:04X} {line.text}")
        for address in stale:
            passed = False
            print(f"  listed as out of reach, but executed, not carried "
                  f"over or not an instruction of these routines: "
                  f"{address:04X}")
        for address in sorted(set(batch.not_carried)
                              - {line.address for line in every}):
            passed = False
            print(f"  listed as not carried over, and not an instruction "
                  f"of these routines: {address:04X}")
    return passed


def linked_as(symbols, name):
    """Where the map has the function that `functions` of a batch names:
    under that name, which is the one that a #pragma aux of the C
    translation gives it, with _c behind the routine's; or, in a tree
    that leaves the name to the compiler, with the underscore that the
    compiler puts behind the name of a function."""
    for known in (name, name.removesuffix("_c") + "_"):
        if known in symbols:
            return symbols[known]
    raise KeyError(f"the link map has no function {name}")


def check_translation(link_map, executed, tree, place, batches):
    """Report on the C functions that replace what `batches` list; return
    success. `tree` is the tree that is checked: what GONE has for it is
    to be gone. `place` makes of an address of the map what `executed`
    counts by, as coverage.place() says it.

    In a flat program that is compiled for size, several functions may
    be one place of the map: the compiler gives a function no
    instructions of its own where the one behind it in its file has the
    same. Such a function counts as run when another of its place did.

    A function of a batch is looked for in the map among what is not
    static, and then among the static ones, where it has to be there
    once: in the portable C a function that only its file calls is
    static, by clang-tidy's misc-use-internal-linkage (TODO.md, "5b, the
    stub"). The function that a tree has in its place, under another
    name, may be static in its file too, as an entry that only the
    system comes to is."""
    linked = le.Map(link_map)
    symbols = dict(linked.public)
    gone = GONE.get(tree, {})
    renamed = RENAMED.get(tree, {})
    wanted = [name for name, _ in renamed.values()]
    if tree != "c":
        wanted += [name.removesuffix("_c") + "_" for batch in batches
                   for name in batch.functions
                   if name not in gone and name not in renamed]
    for name in wanted:
        found = linked.named(name)
        if name not in symbols and len(found) == 1:
            symbols[name] = (found[0].first, found[0].offset)
    passed = True
    for name in sorted(set(renamed) - {name for batch in batches
                                       for name in batch.functions}):
        passed = False
        print(f"  listed as renamed in {tree}, and no function of a "
              f"batch: {name}")
    for name in sorted(set(gone) - {name for batch in batches
                                    for name in batch.functions}):
        passed = False
        print(f"  listed as gone from {tree}, and no function of a batch: "
              f"{name}")
    for batch in batches:
        for name in batch.functions:
            if name in gone:
                try:
                    linked_as(symbols, name)
                except KeyError:
                    print(f"  gone from {tree}: {name}, {gone[name]}")
                else:
                    passed = False
                    print(f"  listed as gone from {tree}, but linked: "
                          f"{name}")
                continue
            if name in renamed:
                if name in symbols:
                    passed = False
                    print(f"  listed as renamed in {tree}, but linked: "
                          f"{name}")
                    continue
                print(f"  in {tree} {name} is {renamed[name][0]}: "
                      f"{renamed[name][1]}")
                name = renamed[name][0]
            try:
                start = place(*linked_as(symbols, name))
            except KeyError as error:
                passed = False
                print(f"  {error.args[0]}")
                continue
            if start not in executed:
                passed = False
                print(f"  not executed in the translation: {name}")
        source = batch.source
        for file, (now, _) in FILES.get(tree, {}).items():
            source = re.sub(rf"\b{re.escape(file)}", now, source)
        print(f"{batch.title}: {len(batch.functions)} C functions of "
              f"{source} checked")
    return passed


def check_stack(program, outcomes):
    """Report on how much of its stack `program`, a build of a C tree,
    left untouched in the scenarios that it passed, of which `outcomes`
    are the compare.Outcome; return success. A scenario that fails has
    used its stack for what failed it, and says nothing here."""
    size = coverage.stack_size(program.contents, program.linked)
    limit = size // STACK_LEFT
    left = {outcome.name: outcome.run.stack_left for outcome in outcomes
            if outcome.passed and outcome.run.stack_left is not None}
    names = list(left)
    if not left:
        print(f"The stack: {size} bytes, of which 1/{STACK_LEFT}, {limit}, "
              "are to be left untouched in every scenario that passes: "
              "none passed with its stack looked at")
        return True
    least = min(left, key=left.get)
    print(f"The stack: {left[least]} of its {size} bytes are left untouched "
          f"at the least, in {least}; the limit is 1/{STACK_LEFT} of it, "
          f"{limit}")
    print(textwrap.fill(
        "in each scenario: " + ", ".join(f"{name} {left[name]}"
                                         for name in names),
        width=78, initial_indent="  ", subsequent_indent="    ",
        break_on_hyphens=False))
    for name in names:
        if left[name] < limit:
            print(f"  too little of the stack left: {left[name]} bytes in "
                  f"{name}")
    return left[least] >= limit


def check_layout(program):
    """Report on how `program`, a build of a C tree, is laid out, as far
    as the machine of the tests and the program itself care; return
    success.

    For a program of 16 bits, the C translation, tools/mkexe.c has seen
    to it when the file was built, and says why at its top: no segment
    of code reaches into the last 4K of its 64K, where QEMU gets a near
    call wrong that goes round the end of the segment, and no code
    stands within 4K of the stack, QEMU being many times slower at a
    write to a page that has code which it has run.

    Of a flat program of 32 bits, the portable C, the first is gone with
    the segments of 64K. The second is there in another form: nothing
    that a scenario writes lies in a page of 4K that has code, of the
    stack the part that check_stack() lets a scenario use. That, and
    where the driver of the tests finds the stack, is
    harness/layoutcheck.py's to say and to hold, which this asks: the
    test of the layout does the same without a machine, and holds the
    stub of the file besides."""
    if not le.linear(program.contents):
        return True
    check = layoutcheck.Check(program.contents, program.linked)
    passed = check.all()
    for problem in check.problems:
        print(textwrap.fill(problem, width=78, initial_indent="  ",
                            subsequent_indent="    ",
                            break_on_hyphens=False))
    print(textwrap.fill(
        "The layout, as harness/layoutcheck.py holds a flat program to "
        "it: " + "; ".join(check.found), width=78, subsequent_indent="  ",
        break_on_hyphens=False))
    return passed


def check_place(program, outcomes):
    """Report on where `program` was in the memory of the machine, if it
    is a flat program; return success. The driver of the tests reads
    what it watches of such a program at addresses of the memory, which
    the harness works out with the program's first object at le.BENCH,
    where the extender put it in every run that was looked at and where
    nothing promises it: at another place the driver reads other bytes,
    a step ends when it should not or never, and no stack is what it
    seems. QEMU's log of the code that ran says where the program was
    (coverage.executed).

    A scenario that starts the program a second time, from the command
    line of the first, has it in two places. The driver is told where a
    copy is too, by the rule that le.py has for it, as many of them as
    driver.COPIES, and reads of the copy that idles what is that
    copy's: a copy that ran has to have been at one of those places, as
    the first has."""
    flat = le.linear(program.contents)
    if not flat:
        return True
    places = flat.copies(COPIES, kept=le.KEPT)
    ran = {outcome.name: outcome.run.executed.at for outcome in outcomes
           if outcome.run is not None}
    elsewhere = {name: at for name, at in ran.items()
                 if at and (places[0] not in at or set(at) - set(places))}
    never = [name for name, at in ran.items() if not at]
    # A copy of the program, which a scenario starts from the command
    # line of the one before, is loaded behind it.
    copies = {name: at for name, at in ran.items()
              if len(at) > 1 and name not in elsewhere}
    for name, at in elsewhere.items():
        print(f"  in {name} the program ran at "
              + ", ".join(f"{address:08X}h" for address in at)
              + ", and the driver of the tests took it and its copies to "
              "be at " + ", ".join(f"{address:08X}h" for address in places)
              + ": see BENCH and behind() of harness/le.py, and "
              "harness/drivercheck.py --trace")
    print(textwrap.fill(
        f"The place: in {len(ran) - len(elsewhere) - len(never)} of the "
        f"{len(ran)} scenarios that the machine got through, the "
        f"program's code ran with its first object at {places[0]:08X}h, "
        "where the driver of the tests reads it"
        + (f"; in {len(never)} none of it was seen to run: "
           f"{', '.join(never)}" if never else "")
        + (f". In {len(copies)} a copy of it ran as well, started from "
           "the command line of the first, where the driver reads that "
           "copy: "
           + ", ".join(
               f"{name} at "
               + " and ".join(f"{address:08X}h" for address in at
                              if address != places[0])
               for name, at in copies.items()) if copies else ""),
        width=78, subsequent_indent="  ", break_on_hyphens=False))
    return not elsewhere


def check_declared(tree, names):
    """Report on what the tree `tree` is declared to do otherwise than
    the original, of which `names` are the scenarios it was taken
    through; return success. A scenario that is not run for the tree has
    to be one, and an entry of the tree's DEVIATIONS.md has to name it.
    A patch has to be of a scenario that the tree is taken through, and
    its header to name entries of the tree's DEVIATIONS.md that name the
    scenario; that it fits its golden was seen before anything was run
    (compare.compare)."""
    declared = deviations.declared(tree)
    document = f"src/{tree}/DEVIATIONS.md"
    entries = deviation_entries(tree)
    passed = True
    for name, why in sorted(declared.not_run.items()):
        if name not in scenarios.SCENARIOS:
            passed = False
            print(f"  declared for {tree}, and no scenario: {name}")
        elif not any(f"`{name}`" in entry for entry in entries.values()):
            passed = False
            print(f"  not run for {tree}, and no entry of {document} names "
                  f"the scenario, as `{name}`: what is not run is "
                  "declared there, or is run")
    every = {step for scenario in scenarios.SCENARIOS.values()
             for step in scenario.steps}
    for step, (instead, _) in declared.in_place_of.items():
        if step not in every:
            passed = False
            print(f"  {tree} is declared to be taken through {instead} in "
                  f"place of a step that no scenario has: {step}")
    for name, gone in declared.left_out_of.items():
        if name not in names:
            passed = False
            print(f"  steps are left out of {name} for {tree}, which is "
                  "not run for it")
            continue
        steps = [step for step in scenarios.SCENARIOS[name].steps
                 if step not in declared.left_out]
        for some in gone:
            if not deviations.found(steps, some):
                passed = False
                print(f"  left out of {name} for {tree}, and the scenario "
                      f"has no such steps one behind the other: "
                      f"{', '.join(some)}")
    for name, others in declared.in_place_in.items():
        if name not in names:
            passed = False
            print(f"  {tree} is declared to be taken through other steps "
                  f"in {name}, which is not run for it")
            continue
        steps = [scenarios.SCENARIOS[name].steps[at] for at in
                 replace(declared, in_place_in={}).kept(
                     scenarios.SCENARIOS[name].steps, name)]
        for some, (instead, _) in others.items():
            if len(instead) != len(some):
                passed = False
                print(f"  {tree} is declared to be taken through "
                      f"{len(instead)} steps in place of {len(some)} of "
                      f"{name}: a step for each, or None, "
                      f"{', '.join(some)}")
            elif not deviations.found(steps, some):
                passed = False
                print(f"  {tree} is declared to be taken through other "
                      f"steps in place of some that {name} has not one "
                      f"behind the other: {', '.join(some)}")
    # The scenarios that the tree is held to an expectation of its own
    # in (goldens.py, "What a tree is expected to show where the
    # original cannot go"): those of deviations.OWN alone, each named by
    # an entry, and with nothing else declared of it. That the file is
    # there and its header filled in was seen before anything was run.
    own = {}
    for name in sorted(declared.own):
        if name not in deviations.OWN:
            passed = False
            print(f"  {tree} is declared to be held to an expectation of "
                  f"its own in {name}: only "
                  f"{', '.join(deviations.OWN)} may be, on the user's "
                  "word of 14 October 2026, and every other scenario is "
                  "held against the original's golden")
            continue
        if name not in names:
            passed = False
            print(f"  {tree} is declared to be held to an expectation of "
                  f"its own in {name}, which is not run for it")
            continue
        if name in declared.left_out_of or name in declared.in_place_in:
            passed = False
            print(f"  {tree} is held to an expectation of its own in "
                  f"{name}, and steps of it are declared to be left out "
                  "or taken in place of others: its steps are its own")
        golden = goldens.read(name, tree)
        own[name] = golden.own
        lines = golden.own.read_text(encoding="utf-8").split("\n")
        for number in goldens.read_header(
                lines[:next(at for at, line in enumerate(lines)
                            if line.startswith("code page "))], tree)[0]:
            if f"`{name}`" not in entries.get(number, ""):
                passed = False
                print(f"  {goldens.shown(golden.own)} says that it is of "
                      f"deviation {number}, and {document} "
                      + ("does not name the scenario there, as "
                         f"`{name}`" if number in entries else
                         "has no such entry"))
    patched = {}
    for file in goldens.patches(tree):
        name = file.stem
        if file.suffix == ".txt" and own.get(name) == file:
            continue
        if file.suffix != ".patch" or name not in names or name in own:
            passed = False
            print(f"  {goldens.shown(file)} is neither the patch of a "
                  f"scenario that {tree} is held against the original's "
                  "golden in nor the expectation of its own of a "
                  "scenario that deviations.OWN allows")
            continue
        patch = goldens.read(name, tree).patch
        patched[name] = patch.count()
        for number in patch.deviations:
            if f"`{name}`" not in entries.get(number, ""):
                passed = False
                print(f"  {goldens.shown(file)} says that it is of "
                      f"deviation {number}, and {document} "
                      + ("does not name the scenario there, as "
                         f"`{name}`" if number in entries else
                         "has no such entry"))
    shorter = [name for name in names
               if len(declared.kept(scenarios.SCENARIOS[name].steps, name))
               < len(scenarios.SCENARIOS[name].steps)]
    held = [name for name in names
            if name not in patched and name not in shorter
            and name not in own]
    # The rules of the tree: what each makes of the goldens. A rule that
    # finds nothing to change on any screen says nothing any more.
    counts, screens, some = [0] * len(declared.rules), 0, 0
    for name in [name for name in names if name not in own] \
            if declared.rules else ():
        each, any_ = goldens.ruled(name, tree)
        counts = [a + b for a, b in zip(counts, each)]
        screens += any_
        some += bool(any_)
    print(textwrap.fill(
        f"The goldens: {len(held)} scenarios were held against the "
        "original's as it is"
        + (f", or as the rules of {tree} leave it: they expect "
           f"{screens} screens of {some} scenarios otherwise than the "
           "original shows them" if declared.rules else ""),
        width=78, subsequent_indent="  ", break_on_hyphens=False))
    for rule, count in zip(declared.rules, counts):
        what = getattr(rule, "what", None) or rule[0].pattern
        print(f"  a rule of {tree}, {what}: {count} screens")
        if not count:
            passed = False
            print(f"    which is no screen of any scenario that {tree} is "
                  "taken through: the rule is to go from deviations.py")
    for what, some in (
            ("against the original's without the steps that are left out "
             f"for {tree}", shorter),
            ("against the original's with a patch, the lines that the "
             "patch says are expected otherwise in brackets",
             [f"{name} ({count})" for name, count in patched.items()])):
        if some:
            print(textwrap.fill(
                f"{what}: {', '.join(some)}", width=78,
                initial_indent="  ", subsequent_indent="    ",
                break_on_hyphens=False))
    for name, file in own.items():
        print(textwrap.fill(
            f"held against {tree}'s own expectation, "
            f"{goldens.shown(file)}, which was recorded from its build and "
            f"read row by row, and not against the original's: {name}, "
            f"{declared.own[name]}", width=78, initial_indent="  ",
            subsequent_indent="    ", break_on_hyphens=False))
    for name, why in scenarios.not_run(tree).items():
        if name in declared.not_run or deviations.owner(name) is None:
            continue
        print(textwrap.fill(f"not run for {tree}: {name}, {why}", width=78,
                            initial_indent="  ", subsequent_indent="    ",
                            break_on_hyphens=False))
    for name, why in declared.not_run.items():
        print(textwrap.fill(f"not run for {tree}: {name}, {why}", width=78,
                            initial_indent="  ", subsequent_indent="    ",
                            break_on_hyphens=False))
    of_setup = [name for name in scenarios.names(setup=True)
                if name not in declared.not_run]
    if declared.no_setup and of_setup:
        print(textwrap.fill(
            f"not run for {tree}: the scenarios of the setup program, "
            f"{', '.join(of_setup)}; {declared.no_setup}", width=78,
            initial_indent="  ", subsequent_indent="    ",
            break_on_hyphens=False))
    for step, (instead, why) in declared.in_place_of.items():
        print(textwrap.fill(
            f"{tree} is taken through {instead} in place of {step}: {why}",
            width=78, initial_indent="  ", subsequent_indent="    ",
            break_on_hyphens=False))
    for name, others in declared.in_place_in.items():
        for some, (instead, why) in others.items():
            print(textwrap.fill(
                f"in {name}, {tree} is taken through "
                + ", ".join(step or "nothing" for step in instead)
                + f" in place of {', '.join(some)}: {why}",
                width=78, initial_indent="  ", subsequent_indent="    ",
                break_on_hyphens=False))
    return passed


def deviation_entries(tree):
    """The entries of the tree's DEVIATIONS.md, number -> text: each
    from its heading, "### 7. ...", to the next heading of an entry or
    of a part, "## ". The entries of the program for Linux, "### L1. "
    and on in a part of their own, are not among them: they declare
    what that program does otherwise than the portable C, and what the
    portable C is declared to do is named in an entry of its own.
    Nothing holds the scenarios that the program for Linux is not run
    for (deviations.LINUX_NOT_RUN) to an entry that names them."""
    file = deps.ROOT / "src" / tree / "DEVIATIONS.md"
    if not file.exists():
        return {}
    parts = re.split(r"^(?:### (\w+)\. |## )",
                     file.read_text(encoding="utf-8"), flags=re.MULTILINE)
    return {int(number): text
            for number, text in zip(parts[1::2], parts[2::2])
            if number is not None and number.isdigit()}


def check_comparison(outcomes, program):
    """Report on whether the comparison notices differences; return
    success.

    It is tried on the first scenario of `outcomes`, the
    compare.Outcome of `program`, that the machine got through: the
    character in the top left corner of its first screen is changed, and
    then the first entry of its drive C: is left out, if the run was
    not cut and has its disks.
    """
    outcome = next((outcome for outcome in outcomes
                    if outcome.run is not None), None)
    if outcome is None:
        print("The comparison: the machine got through no scenario, and "
              "there is no run to try it on")
        return True
    name, run = outcome.name, outcome.run
    screen = run.screens[0]
    cells = bytearray(screen.cells)
    cells[0] ^= 1
    changes = {
        "a changed cell on the first screen": replace(
            run, screens=[replace(screen, cells=bytes(cells))]
            + run.screens[1:]),
    }
    if not run.cut:
        entries = dict(run.disks["C"])
        del entries[min(entries)]
        changes["an entry missing from drive C:"] = replace(
            run, disks={**run.disks, "C": entries})
    golden = goldens.read(name, program.tree)
    # Reported is what is otherwise than in the run as it was: a
    # scenario that fails is otherwise than its golden as it is.
    before = goldens.differences(golden, run, {program.filename})
    passed = True
    for what, changed in changes.items():
        if goldens.differences(golden, changed, {program.filename}) != before:
            print(f"The comparison: {what} of {name} is reported")
        else:
            passed = False
            print(f"  not reported by the comparison: {what} of {name}")
    return passed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("translation", help="the VC.EXE of a C tree: "
                                            "build/c/VC.EXE or "
                                            "build/portable_c/VC.EXE")
    parser.add_argument("link_map", nargs="?",
                        help="its link map (default: VC.MAP next to it)")
    parser.add_argument("listing", nargs="?", default=setups.LISTING,
                        help="the listing of VC.COM laid out like the "
                             "original; that of VCSETUP.COM is taken to "
                             "be next to it (default: that of the build, "
                             "build/shareware-layout/VC.LST)")
    args = parser.parse_args()

    original = setups.original_vc()
    translation = setups.Program.load(args.translation)
    if args.link_map:
        translation = replace(translation, linked=Path(args.link_map))
    tree = translation.tree
    size = len(translation.contents)
    times = SIZE_LIMIT.get(tree)
    too_long = times is not None and size > times * len(original.contents)
    print(f"VC.EXE is {size} bytes, {size / len(original.contents):.2f} "
          f"times the original's {len(original.contents)}; "
          + (f"the limit is {times} times" if times is not None else
             f"{tree} is held to no multiple of that"))

    if not check_layout(translation):
        sys.exit(1)
    names = scenarios.names(tree)
    # What a flat program has in its memory at the end of each scenario
    # is kept next to it, for a piece of work to hold against what the
    # build before had: see dataimage.py. It judges nothing here.
    images = Path(args.translation).parent / dataimage.DIRECTORY \
        if le.linear(translation.contents) else None
    failed, outcomes = compare.compare(names, translation, trace=True,
                                       stack=True, images=images)
    # What ran in the original is of every scenario, also of one that
    # the tree is not taken through: of VC.COM by the scenarios of the
    # main program, and of VCSETUP.COM by those of the setup program.
    in_translation = {}
    for outcome in outcomes:
        if outcome.run is not None:
            in_translation.update(outcome.run.executed)
    batches = list(BATCHES)
    passed = check_original(coverage.Listing(args.listing), goldens.ran(),
                            BATCHES)
    if not deviations.declared(tree).no_setup:
        of_setup = Path(args.listing).with_name(setups.SETUP_LISTING.name)
        passed = check_original(coverage.Listing(of_setup),
                                goldens.ran(setup=True),
                                SETUP_BATCHES) and passed
        batches += SETUP_BATCHES
    passed = check_translation(translation.linked, in_translation, tree,
                               coverage.place(translation.contents),
                               batches) and passed
    passed = check_place(translation, outcomes) and passed
    passed = check_stack(translation, outcomes) and passed
    passed = check_declared(tree, names) and passed
    passed = check_comparison(outcomes, translation) and passed
    sys.exit(failed or not passed or too_long)


if __name__ == "__main__":
    main()
