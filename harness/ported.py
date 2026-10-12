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
LenStk in port/asm/VC.ASM, a few bytes less in the build that is tested.
That build marks its stack when it has set it up, and how much of the
mark is left is read from the machine every time the program waits for a
key (see scenarios.py). A stack used beyond its bottom overwrites what the
program saves of its panels and its setup while a command runs, and no
screen would show that. How long the stack is, is taken from the link
map, so the limit follows it. What every scenario leaves is reported, not
only the least.

A comparison that finds nothing is only worth something if it would have
found a difference. So one cell of a screen that the port has shown is
changed, and then one entry is taken off a disk it has left, and the
comparison must report each.

The build of the port that is tested is the one that signals where it is.
What ran in the original was recorded with the goldens; what runs in the
port is read from QEMU's log of the code it executed (see coverage.py).
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
import goldens
import scenarios
import setups

COM_ORIGIN = 0x100                # where a .COM file is loaded
EXE_HEADER = 0x20                 # of the port's VC.EXE, see tools/mkexe.c

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
    # procedures, each by its first label (see coverage.Listing).
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
        functions=("HexDec_c", "Number_c", "PutDate_c", "BufDate_c",
                   "PutTime_c", "BufTime_c", "SzDtTm_c"),
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
        functions=("LnStr_c", "NgLine_c", "CntLine_c"),
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
        functions=("AddFile_c", "MvzLine_c", "FilZero_c", "StrLen_c",
                   "LowCase_c", "FndLet_c", "CutPath_c", "NormStr_c",
                   "PathLin_c", "LetLin_c", "ShrtLin_c", "MainFil_c"),
    ),
    Batch(
        title="Colours and the screen buffer",
        source="screen.c",
        routines=("BasAdr", "Color", "ColAddr", "Shadow", "MvsBwc",
                  "MvsBwe", "MvsColr", "MovDz"),
        functions=("BasAdr_c", "Color_c", "ColAddr_c", "Shadow_c",
                   "MvsBw_c", "MvsColr_c", "MovDz0_c"),
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
        functions=("HexByt_c", "TstHex_c", "TxtNum_c"),
    ),
    Batch(
        title="File names in their fixed form, and masks",
        source="mask.c",
        routines=("Convert", "Conv10", "UnConv", "UnCnv1", "TstMas"),
        functions=("Convert_c", "UnConv_c", "TstMas_c"),
    ),
    Batch(
        title="The list of files of a panel: its filter and its order",
        source="files.c",
        routines=("SetMask", "TstExe", "Sort", "CmpPar", "CmpExt", "FnFile",
                  "NearFl", "TotSel"),
        functions=("SetMask_c", "TstExe_c", "Sort_c", "FnFile_c",
                   "NearFl_c", "TotSel_c"),
    ),
    Batch(
        title="Where the panels are, and the cursor in them",
        source="place.c",
        routines=("BegBX", "SizDX", "LnsAX", "InvWCB", "AdrNum", "FilNum",
                  "NumAdr", "ChgAdr", "CorPar", "FilePan"),
        functions=("BegBX_c", "SizDX_c", "LnsAX_c", "InvWCB_c", "AdrNum_c",
                   "NumAdr_c", "ChgAdr_c", "CorPar_c", "FilePan_c"),
    ),
    Batch(
        title="A panel of files on the screen: its frame, its path, its "
              "files and the cursor",
        source="panel.c",
        routines=("Panel", "Panel8", "PutPath", "PtPt10", "PutName",
                  "PutNm5", "PutFls", "SetCur", "FilDir"),
        functions=("Panel_c", "PutPath_c", "PtPt10_c", "PutFls_c",
                   "SetCur_c", "FilDir_c"),
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
        functions=("PutTre_c", "PutTree_c", "TreeCur_c", "TrCr10_c",
                   "TrCr20_c"),
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
        functions=("ReadTre_c", "ChTree_c"),
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
            **dict.fromkeys(
                (0xC94E, 0xC9C1, 0xC9C3, 0xC9C6, 0xC9C8, 0xC9CA, 0xC9CD,
                 0xC9CF, 0xC9D1, 0xC9D3, 0xC9D5, 0xC9D7, 0xC9D9, 0xC9DB,
                 0xC9DD, 0xC9DF, 0xC9E2, 0xC9E6, 0xC9E8, 0xC9E9, 0xC9EA,
                 0xC9EE, 0xC9F2, 0xC9F4, 0xC9F7, 0xC9F8, 0xC9F9, 0xC9FB,
                 0xCA00, 0xCA02, 0xCA04, 0xCA06, 0xCA08, 0xCA0A, 0xCA0C,
                 0xCA0F, 0xCA11, 0xCA13, 0xCA15, 0xCA17, 0xCA19, 0xCA1B,
                 0xCA1D, 0xCA1E, 0xCA20, 0xCA23, 0xCA25, 0xCA28, 0xCA2A,
                 0xCA2C, 0xCA2E, 0xCA30, 0xCA32, 0xCA34, 0xCA39, 0xCA3B,
                 0xCA3C, 0xCA3D, 0xCA3F, 0xCA43, 0xCA46, 0xCA47, 0xCA48,
                 0xCA4A, 0xCA4D, 0xCA52, 0xCA54, 0xCA56, 0xCA58, 0xCA59,
                 0xCA5B, 0xCA5C, 0xCA5D, 0xCA5E, 0xCA62, 0xCA64, 0xCA67,
                 0xCA69, 0xCA6C, 0xCA6D, 0xCA6E, 0xCA6F, 0xCA71, 0xCA73,
                 0xCA77, 0xCA78, 0xCA7A, 0xCA7C, 0xCA7E, 0xCA81, 0xCA83,
                 0xCA85, 0xCA88),
                "ChTree, for the mouse: a button held on a line of the "
                "tree, above the lines or below them, a second click on "
                "a line, and a click outside the box. The bench has no "
                "mouse"),
        },
    ),
    Batch(
        title="The Info panel: memory, the disk, the files of the other "
              "panel and the text of dirinfo",
        source="info.c",
        routines=("PutInf", "PtIn20"),
        functions=("PutInf_c",),
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
        title="Reading what a panel shows: the files of a directory, and "
              "for the Info the disk, its label and the text of dirinfo",
        source="read.c",
        routines=("ReDir", "RdInfo", "GetVol", "DirSize", "TstInfo"),
        functions=("ReDir_c", "ReDr45_c", "ReDr50_c", "RdInfo_c",
                   "GetVol_c", "DirSize_c", "TstInfo_c"),
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
            **dict.fromkeys(
                (0xB988, 0xB989, 0xB98B, 0xB98C),
                "ReDir, for a directory that cannot be read at all: the "
                "disks of the bench are always ready, and it has no "
                "second floppy drive to be asked for"),
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
            0x5B88: "V25_00, for a viewer without a status line: it has "
                    "one whenever its window is 80 columns wide, and "
                    "only the editor is given a narrower one",
        },
    ),
    Batch(
        title="The editor: where in the memory its text is, what it shows "
              "of it and how it saves it",
        source="edit.c",
        routines=("T57_00", "T60_00", "T62_00", "T63_00", "T70_00",
                  "T71_00", "T72_00", "T73_00", "T73_10", "T74_00",
                  "T75_00", "T76_00", "T80_00"),
        functions=("T57_00_c", "T57_01_c", "T57_02_c", "T60_00_c",
                   "T61_00_c", "T62_00_c", "T63_01_c", "T70_00_c",
                   "T71_00_c", "T72_00_c", "T73_00_c", "T73_10_c",
                   "T74_00_c", "T75_00_c", "T76_00_c", "T80_00_c"),
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
        title="The help: its file, the index and the pages in their "
              "window, and the keys",
        source="help.c",
        routines=("Help", "Help70", "Help71", "Help72", "Help77", "RdEntry",
                  "SeekDsk", "ReadDsk", "DeCode"),
        functions=("Help00_c",),
        unreached={
            **dict.fromkeys(
                (0x7B5F, 0x7B60, 0x7B63, 0x7B66, 0x7B68, 0x7B6A, 0x7B6C,
                 0x7B6E, 0x7B71, 0x7B74, 0x7B77, 0x7B79, 0x7B7B, 0x7B7C,
                 0x7B7E, 0x7B81, 0x7B82, 0x7B84, 0x7B86, 0x7B87, 0x7B88,
                 0x7B8C, 0x7B90, 0x7B93, 0x7B94, 0x7B95, 0x7B97, 0x7B9C,
                 0x7B9E, 0x7BA0, 0x7BA5, 0x7BA7, 0x7BA9, 0x7BAB, 0x7BAC,
                 0x7BAD, 0x7BAF, 0x7BB1, 0x7BB2, 0x7BB4, 0x7BB7, 0x7BB9,
                 0x7BBB, 0x7BBD, 0x7BBF, 0x7BC1, 0x7BC4, 0x7BC6, 0x7BC8,
                 0x7BCA, 0x7BCB, 0x7BCE, 0x7BCF, 0x7BD1, 0x7BD2, 0x7BD5,
                 0x7BD7, 0x7BD9, 0x7BDC, 0x7BDE, 0x7BE0, 0x7BE1, 0x7BE5,
                 0x7BE8, 0x7BE9, 0x7BEB, 0x7BEC, 0x7BEF, 0x7BF4, 0x7BF9,
                 0x7BFB, 0x7BFC, 0x7BFE, 0x7C01, 0x7C03, 0x7C05, 0x7C09,
                 0x7C0B, 0x7C0F, 0x7C10, 0x7C11, 0x7C13, 0x7C15, 0x7C17,
                 0x7C19, 0x7C1A, 0x7C1D, 0x7C1E, 0x7C1F, 0x7C21, 0x7C23,
                 0x7C25, 0x7C28, 0x7C2B, 0x7C2D, 0x7C30, 0x7C31, 0x7C32,
                 0x7C34, 0x7C36, 0x7C3B, 0x7C41, 0x7C43, 0x7C46, 0x7C47),
                "Help, for the mouse: a button held where the text is, "
                "above it or below it, a click on a line of the index "
                "and one outside the window. The bench has no mouse"),
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
        functions=("FindFil_c", "FndMove_c", "FndCurs_c", "FndAdr_c"),
        unreached={
            0x8A7E: "FindFil, for more lines than the memory has room "
                    "for, some 7,000: the disks of the bench have fewer "
                    "files",
            **dict.fromkeys(
                (0x8BC8, 0x8BCA, 0x8BCC, 0x8BCE, 0x8BD0),
                "FindFil, for less than 64 KB of memory to read a file "
                "into: the bench has some 500 KB free"),
            **dict.fromkeys(
                (0x8D60, 0x8D61, 0x8D64, 0x8D67, 0x8D69, 0x8D6B, 0x8D6D,
                 0x8D6F, 0x8D72, 0x8D75, 0x8D78, 0x8D7A, 0x8D7C, 0x8D7D,
                 0x8D7F, 0x8D82, 0x8D83, 0x8D85, 0x8D87, 0x8D88, 0x8D8B,
                 0x8D8C, 0x8D8D, 0x8D91, 0x8D95, 0x8D98, 0x8D99, 0x8D9A,
                 0x8D9C, 0x8DA1, 0x8DA3, 0x8DA7, 0x8DAB, 0x8DAD, 0x8DAF,
                 0x8DB1, 0x8DB3, 0x8DB4, 0x8DB7, 0x8DB9, 0x8DBB, 0x8DBD,
                 0x8DBF, 0x8DC1, 0x8DC2, 0x8DC4, 0x8DC6, 0x8DC8, 0x8DCB,
                 0x8DCD, 0x8DCF, 0x8DD0, 0x8DD4, 0x8DD7, 0x8DD8, 0x8DDA,
                 0x8DDB, 0x8DDE, 0x8DDF, 0x8DE2, 0x8DE7, 0x8DEB, 0x8DED,
                 0x8DEF, 0x8DF0, 0x8DF2, 0x8DF5, 0x8DF7, 0x8DF9, 0x8DFD,
                 0x8DFF, 0x8E00, 0x8E01, 0x8E03, 0x8E05, 0x8E07, 0x8E09,
                 0x8E0A, 0x8E0C, 0x8E0D, 0x8E0E, 0x8E10, 0x8E12, 0x8E14,
                 0x8E17, 0x8E1A, 0x8E1C, 0x8E1F, 0x8E20, 0x8E21, 0x8E22,
                 0x8E24, 0x8E26, 0x8E28, 0x8E2B, 0x8E2E),
                "FndMove, for the mouse: the bench has none"),
        },
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
    parser.add_argument("port", help="the port's VC.EXE, the build that "
                                     "signals")
    parser.add_argument("link_map", help="its link map")
    parser.add_argument("listing", help="the listing of VC.COM laid out "
                                        "like the original")
    parser.add_argument("shipped", help="the port's VC.EXE as it is shipped, "
                                        "for its size")
    args = parser.parse_args()

    original, port = setups.original_vc(), setups.Program.load(args.port)
    size = len(setups.Program.load(args.shipped).contents)
    limit = SIZE_LIMIT * len(original.contents)
    print(f"VC.EXE is {size} bytes, {size / len(original.contents):.2f} "
          f"times the original's {len(original.contents)}; the limit is "
          f"{SIZE_LIMIT} times")

    names = list(scenarios.SCENARIOS)
    failed, runs = compare.compare(names, port, signals=True, trace=True)
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
