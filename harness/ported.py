#!/usr/bin/env python3
"""Check the port: the program that is part assembly and part C.

The port is taken through every scenario and must show the same screens
and leave the same disks as the original did, which the goldens record
(see compare.py and goldens.py). That alone would say little about a
routine no scenario reaches, so for the routines translated so far, listed
below, this also checks that

  - every instruction of the assembly they replace ran in the original, in
    some scenario, unless it is listed here with the reason why none gets
    there,
  - every C function that replaces them ran in the port,
  - the port is no more than three times the size of the original, and
  - the port leaves half of its stack untouched in every scenario.

Compiled code uses more of the stack than the assembly it replaces, so
the port has a stack many times as long as the original's 256 bytes:
LenStk in port/asm/VC.ASM. When the program has been loaded its stack
holds what its file has there, which is nothing, zeros; how much of that
is still there, from the bottom up, is looked at every time a step ends
with the program waiting for a key (see driver.py). A
stack used beyond its bottom overwrites what the program
saves of its panels and its setup while a command runs, and no screen
would show that. How long the stack is, is taken from the link map, so
the limit follows it. What every scenario leaves is reported, not only
the least.

A comparison that finds nothing is only worth something if it would have
found a difference. So one cell of a screen that the port has shown is
changed, and then one entry is taken off a disk it has left, and the
comparison must report each.

The build of the port that is tested is the one that is shipped. What
ran in the original was recorded with the goldens; what runs in the port
is read from QEMU's log of the code it executed (see coverage.py).
The assembly is identified by the listing of the build that is laid out
like the original, and the C functions by the port's link map.

Exits with status 0 if all of this holds and 1 otherwise.
"""
import argparse
import sys
import textwrap
from dataclasses import dataclass, field, replace

import compare
import coverage
from driver import COM_ORIGIN, EXE_HEADER
import goldens
import scenarios
import setups


# How large the port may grow, as a multiple of the original's size. The
# aim is to stay well below; this is where growth must have gone wrong.
SIZE_LIMIT = 3

# How much of its stack the port must leave untouched in every scenario:
# one part in this many, half of it. The stack is made long enough for
# its use to need no care while the program is translated, and this is
# where that would have stopped being true. The rest is room for what no
# scenario has: a critical error of DOS at the bottom of deep calls, an
# interrupt that arrives there.
STACK_LEFT = 2

# The bottom and the top of the stack, as the link map names them.
STACK = ("DataSav", "VcStack")


@dataclass(frozen=True)
class Batch:
    """Routines translated together."""
    title: str
    source: str                   # the C file in port/
    # The assembly procedures it replaces, and stretches of code between
    # procedures, each by its first label, or by a later one for what
    # stands from there on (see coverage.Listing).
    routines: tuple
    functions: tuple              # its functions that the assembly calls
    # Address in the original -> why no scenario executes the instruction.
    unreached: dict = field(default_factory=dict)


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
        functions=("AddFile_c", "MvzLine_c", "CutPath_c", "PathLin_c"),
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
            0xA29C: "ColAddr, for a monochrome display: the machine has "
                    "a colour one",
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
        functions=("ReDir_c", "ReDr45_c", "ReDr50_c", "RdInfo_c"),
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
            **dict.fromkeys(
                (0x6400, 0x6402),
                "T00_28, for a window that moves up by fewer lines than "
                "the cursor is below its first: it moves up only to the "
                "line of the cursor"),
        },
    ),
    Batch(
        title="The help: its file, the index and the pages in their "
              "window, and the keys",
        source="help.c",
        routines=("Help", "Help70", "Help71", "Help72", "Help77", "RdEntry",
                  "SeekDsk", "ReadDsk", "DeCode"),
        functions=("Help00_c",),
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
        functions=("FindFil_c",),
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
        functions=("F38_10_c", "F38_40_c", "M30_50_c", "M30_60_c",
                   "DirFunc_c", "DelTr0_c"),
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
        unreached={
            **dict.fromkeys(
                (0xB1E6, 0xB1EE, 0xB1F0, 0xB1F3),
                "Input0, when the minute has come in which the screen is "
                "blanked, or midnight has passed before it: a minute at "
                "the least of the machine's time in which no key is "
                "pressed, and the harness presses the next one as soon "
                "as the program waits. The blanking itself runs, for the "
                "mouse in the corner of the screen"),
        },
    ),
    Batch(
        title="Copying, renaming and moving a file or a directory",
        source="copy.c",
        routines=("F35_40", "C00_00", "C00_30", "C40_00", "C40_05",
                  "C40_10", "C50_00", "SetDate", "PerCent"),
        functions=("C00_00_c",),
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
        functions=("F35_01_c", "F45_01_c", "F35_61_c"),
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
            **dict.fromkeys(
                (0x8186, 0x8193, 0x8195, 0x8197, 0x8199),
                "F65_00, for a map of more lines than its window has, "
                "13 on a screen of 25 lines, where the cursor leaves "
                "the window: the bench has eight at the most, with "
                "the one program on it that stays in the memory"),
            **dict.fromkeys(
                (0x8237, 0x823A, 0x823C, 0x823E, 0x8240, 0x8242, 0x8243,
                 0x8248, 0x824B, 0x824D, 0x824E, 0x824F, 0x8251, 0x8254,
                 0x8255, 0x8257, 0x8259),
                "F65_00, scrolling the lines by one: for a map of "
                "more lines than its window has"),
            **dict.fromkeys(
                (0x84EA, 0x84EE, 0x84F0, 0x84F2, 0x84F5, 0x84F7, 0x84F9,
                 0x84FB, 0x8501, 0x8504),
                "F65_00, for a program that stays in the memory and "
                "is not released while another is: the bench has one "
                "such program, FDAPM, which does not stay twice"),
        },
    ),
    Batch(
        title="The main loop, the panels drawn and read, a drive that "
              "is not ready, and leaving",
        source="main.c",
        routines=("Init40", "Init46", "Mvz2lin", "Init49", "Init90",
                  "Func01", "Func15", "Func20", "Exit", "Quit", "RestVct",
                  "HookVct", "ErrRdy"),
        functions=("MainLoop_c", "Exit_c", "HookVct_c"),
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
                "which the port takes others than the original, so no "
                "step falls into that time for both"),
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
        # ResUser and SavScr have no caller left in the assembly.
        functions=("ChkMode_c", "Cursor_c", "ScrOffs_c"),
        unreached={
            **dict.fromkeys(
                (0x9376, 0x93B8),
                "ChkMode and ChgCur, for a screen in mode 7, that of a "
                "monochrome display: the bench has a VGA, in mode 3 when "
                "the program starts and when a command has ended"),
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
            **dict.fromkeys(
                (0xB547, 0xB549, 0xB54B, 0xB563, 0xB565, 0xB568, 0xB56B,
                 0xB56D, 0xB56E, 0xB570, 0xB572, 0xB574, 0xB576, 0xB577,
                 0xB579, 0xB57C, 0xB57E, 0xB581, 0xB583),
                "Blank, for a star that flares up or goes out: it has "
                "been out for 88 turns of the sky by then, at the least, "
                "and the sky turns when the timer has ticked. A step "
                "ends when the program has gone round twice; what keeps "
                "it from ending, the mouse moved within the corner, "
                "lets half a tick pass for 500 bytes of a script, which "
                "holds 64K: some 60 turns. A step that lets the program "
                "go round a number of times would reach them: the driver "
                "has the event for it, and no step makes it"),
        },
    ),
    Batch(
        title="The size of a file added to that of its directory",
        source="read.c",
        routines=("DiSi10",),
        functions=("DiSi10_c",),
    ),
    Batch(
        title="The command of a key: the last table of keys that Case "
              "went by",
        source="main.c",
        routines=("Case",),
        functions=(),
    ),
]


def check_original(listing, ran):
    """Report on the assembly that has been replaced; return success.

    `ran` holds the offsets in the original's file of the bytes that ran.
    """
    passed = True
    for batch in BATCHES:
        lines = [line for name in batch.routines
                 for line in listing.procedure(name)]
        missed = [line for line in lines
                  if line.address - COM_ORIGIN not in ran]
        excused = [line for line in missed if line.address in batch.unreached]
        stale = sorted(set(batch.unreached)
                       - {line.address for line in missed})
        print(f"{batch.title}: {len(lines) - len(missed)} of {len(lines)} "
              f"instructions of the original ran, {len(excused)} more are "
              "listed as out of reach")
        for line in missed:
            if line.address not in batch.unreached:
                passed = False
                print(f"  not executed: {line.address:04X} {line.text}")
        for address in stale:
            passed = False
            print(f"  listed as out of reach, but executed or not an "
                  f"instruction of these routines: {address:04X}")
    return passed


def check_port(link_map, executed):
    """Report on the C functions that replace it; return success."""
    symbols = coverage.link_map(link_map)
    passed = True
    for batch in BATCHES:
        for name in batch.functions:
            segment, offset = symbols[name]
            start = segment * 16 + offset
            if start - COM_ORIGIN + EXE_HEADER not in executed:
                passed = False
                print(f"  not executed in the port: {name}")
        print(f"{batch.title}: {len(batch.functions)} C functions of "
              f"{batch.source} checked")
    return passed


def check_stack(link_map, names, runs):
    """Report on how much of its stack the port left untouched; return
    success."""
    symbols = coverage.link_map(link_map)
    (segment, bottom), (same, top) = (symbols[name] for name in STACK)
    assert segment == same
    size = top - bottom
    limit = size // STACK_LEFT
    left = {name: run.stack_left for name, run in zip(names, runs)}
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


def check_comparison(name, run, program):
    """Report on whether the comparison notices differences; return
    success.

    `run` is that of `program` in scenario `name`. The character in the
    top left corner of its first screen is changed, and then the first
    entry of its drive C: is left out.
    """
    screen = run.screens[0]
    cells = bytearray(screen.cells)
    cells[0] ^= 1
    entries = dict(run.disks["C"])
    del entries[min(entries)]
    changes = {
        "a changed cell on the first screen": replace(
            run, screens=[replace(screen, cells=bytes(cells))]
            + run.screens[1:]),
        "an entry missing from drive C:": replace(
            run, disks={**run.disks, "C": entries}),
    }
    golden = goldens.read(name)
    passed = True
    for what, changed in changes.items():
        if goldens.differences(golden, changed, {program.filename}):
            print(f"The comparison: {what} of {name} is reported")
        else:
            passed = False
            print(f"  not reported by the comparison: {what} of {name}")
    return passed


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("port", help="the port's VC.EXE")
    parser.add_argument("link_map", help="its link map")
    parser.add_argument("listing", help="the listing of VC.COM laid out "
                                        "like the original")
    args = parser.parse_args()

    original, port = setups.original_vc(), setups.Program.load(args.port)
    size = len(port.contents)
    limit = SIZE_LIMIT * len(original.contents)
    print(f"VC.EXE is {size} bytes, {size / len(original.contents):.2f} "
          f"times the original's {len(original.contents)}; the limit is "
          f"{SIZE_LIMIT} times")

    names = list(scenarios.SCENARIOS)
    failed, runs = compare.compare(names, port, trace=True, stack=True)
    in_original, in_port = set(), {}
    for name, run in zip(names, runs):
        in_original |= goldens.read_ran(name)
        in_port.update(run.executed)
    passed = check_original(coverage.Listing(args.listing), in_original)
    passed = check_port(args.link_map, in_port) and passed
    passed = check_stack(args.link_map, names, runs) and passed
    passed = check_comparison(names[0], runs[0], port) and passed
    sys.exit(failed or not passed or size > limit)


if __name__ == "__main__":
    main()
