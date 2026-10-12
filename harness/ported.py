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
holds a piece of its file, which the program moves elsewhere as it starts;
how much of that is still there, from the bottom up, is looked at every
time a step ends with the program waiting for a key (see driver.py). A
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
        title="The keys of the panels: the cursor, files selected, the "
              "panels off, on, swapped and of another kind, and Tab",
        source="keys.c",
        routines=("F00_50", "F00_60", "F03_30", "F04_00", "F04_30",
                  "F05_00", "F08_10", "F76_00", "F20_02"),
        functions=("F00_50_c", "F00_60_c", "F00_00_c", "F00_05_c",
                   "F01_00_c", "F01_05_c", "F02_00_c", "F03_00_c",
                   "F04_00_c", "F05_00_c", "F06_00_c", "F07_00_c",
                   "F08_00_c", "F76_00_c", "F76_01_c", "F09_00_c",
                   "F10_00_c", "F78_00_c", "F11_00_c", "F12_00_c",
                   "F13_00_c", "F14_00_c", "F15_00_c", "F16_00_c",
                   "F17_01_c", "F79_00_c", "F18_00_c", "F20_00_c",
                   "F20_02_c"),
        unreached={
            **dict.fromkeys(
                (0x1403, 0x1408, 0x140A, 0x140B, 0x140E, 0x1410, 0x1411,
                 0x1415, 0x1417, 0x141A, 0x141C, 0x1422, 0x1424, 0x142A,
                 0x142E, 0x1430, 0x1436, 0x1439, 0x143B, 0x1441, 0x1443,
                 0x1445, 0x1448, 0x144A, 0x144D, 0x1450),
                "F00_60, for the mouse: files selected by passing over "
                "them with the right button held. The bench has no "
                "mouse"),
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
        title="The menu bar and its pull-down menus",
        source="menu.c",
        routines=("F40_00", "F40_10", "F40_20", "F40_30", "F40_40",
                  "F40_50"),
        functions=("F39_00_c",),
        unreached={
            **dict.fromkeys(
                (0x2ADF, 0x2AE1, 0x2AE3, 0x2AE6, 0x2AE8, 0x2AEA, 0x2AEC,
                 0x2AEE, 0x2AF1, 0x2AF3, 0x2AF6, 0x2AF8, 0x2AFB, 0x2AFD,
                 0x2AFF, 0x2B02, 0x2B04, 0x2B0E, 0x2B34, 0x2B37, 0x2B39,
                 0x2B3B, 0x2B3E, 0x2B40, 0x2B42, 0x2B44, 0x2B46, 0x2B48,
                 0x2B4B, 0x2B4E, 0x2B51, 0x2B53, 0x2B55, 0x2B58, 0x2B5A,
                 0x2B5C, 0x2B5E, 0x2B60, 0x2B69, 0x2B6B, 0x2B6D, 0x2B70,
                 0x2B72, 0x2B75, 0x2B77, 0x2B7A, 0x2B7D, 0x2B7F, 0x2B81,
                 0x2B82, 0x2B83, 0x2B84, 0x2B88, 0x2B8C, 0x2B8E, 0x2B91,
                 0x2B92, 0x2B93, 0x2B94, 0x2B96, 0x2B98, 0x2B99, 0x2B9A,
                 0x2B9C, 0x2BA1, 0x2BA3, 0x2F1A, 0x3057, 0x3058, 0x3059,
                 0x305B, 0x305E, 0x3060, 0x3061, 0x3063, 0x3064, 0x3065,
                 0x3067, 0x3069, 0x306B, 0x306E, 0x3070, 0x3072, 0x3074,
                 0x3076, 0x3077, 0x3079, 0x307B, 0x307C, 0x307E, 0x3080,
                 0x3082, 0x3083, 0x3084),
                "the menu bar, for the mouse: a button down on the bar or "
                "on a menu, a button let go, a menu open with no item "
                "under the cursor, and F40_50, which says whose name on "
                "the bar the mouse is at. The bench has no mouse"),
            0x2CC4: "the menu bar, for Alt pressed by itself while a menu "
                    "is open, which goes back to the bar: Alt is such a "
                    "key only if it comes up within three ticks of the "
                    "timer, and the harness holds it until the program "
                    "has seen it, as the timing has it",
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
            **dict.fromkeys(
                (0x2F99, 0x2F9B, 0x2F9D),
                "the menu bar, for an item of the Files menu chosen "
                "with Shift or Ctrl held down: the program asks the "
                "BIOS for them after it has taken the key, and the "
                "harness holds them for 20 ms of the host for the port, "
                "which a busy host may not give the program in time"),
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
                (0x5351, 0x5354, 0x5359, 0x535E, 0x5364, 0x5366, 0x5369,
                 0x536B, 0x5371, 0x5372, 0x5374, 0x5375, 0x5377, 0x5379,
                 0x537B, 0x537D, 0x537E, 0x5380, 0x5383, 0x5385, 0x5387,
                 0x5389, 0x538B, 0x538E, 0x5390, 0x5392, 0x5394, 0x5396,
                 0x5398, 0x539A, 0x539C, 0x539E, 0x53A0, 0x53A2, 0x53A4,
                 0x53A6),
                "the viewer, for a button of the mouse held in its "
                "window: the bench has no mouse"),
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
            **dict.fromkeys(
                (0x6610, 0x6613, 0x6618, 0x661C, 0x6620, 0x6625, 0x6627,
                 0x6629, 0x662B, 0x662E, 0x662F, 0x6631, 0x6633, 0x6634,
                 0x6636, 0x6639, 0x663C, 0x663E, 0x6643, 0x6648, 0x664A,
                 0x664C, 0x664D, 0x6652, 0x6654, 0x6658, 0x665C, 0x6660,
                 0x6662, 0x6664, 0x6665, 0x6667, 0x6669, 0x666C, 0x666F,
                 0x6671, 0x6673, 0x6678, 0x667A, 0x667C, 0x667E, 0x6680,
                 0x6682, 0x6684, 0x6685, 0x6687, 0x6689, 0x668A, 0x668C,
                 0x668D, 0x668F, 0x6691, 0x6692, 0x6696, 0x6698, 0x6699,
                 0x669B, 0x669D, 0x669F, 0x66A4, 0x66A6, 0x66A8, 0x66AA,
                 0x66AC, 0x66AF, 0x66B1, 0x66B3, 0x66B6, 0x66BA, 0x66BC,
                 0x66BE, 0x66C0, 0x66C2, 0x66C4),
                "T00_71, for the mouse: a click in the window, and a "
                "button held above it or below it. The bench has no "
                "mouse"),
            **dict.fromkeys(
                (0x693D, 0x6940),
                "T06_10, for a cursor behind all that the buffer has of "
                "a line that it holds a part of: the buffer is filled "
                "again whenever the line is drawn with fewer characters "
                "behind the cursor than the window has columns there, "
                "and it is drawn after every key; only a click of the "
                "mouse leaves that out"),
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
        functions=("DialBox_c", "Dialog_c", "InSt10_c", "MinMnu_c"),
        unreached={
            **dict.fromkeys(
                (0x9CE0, 0x9CE1, 0x9CE2, 0x9CE4, 0x9CE7,
                 0x9CE9, 0x9CEC, 0x9CED, 0x9CEF, 0x9CF1,
                 0x9CF3, 0x9CF4, 0x9CF8, 0x9CFB, 0x9CFC,
                 0x9CFE, 0x9D00, 0x9D03, 0x9D06, 0x9D07,
                 0x9D09, 0x9D0B, 0x9D0C, 0x9D0D, 0x9D0F,
                 0x9D13, 0x9D17, 0x9D19, 0x9D1B, 0x9D1D,
                 0x9D1F, 0x9D21, 0x9D24, 0x9D26, 0x9D28,
                 0x9D2A, 0x9D2C, 0x9D2E, 0x9D30, 0x9D32,
                 0x9D34, 0x9D35, 0x9D37, 0x9D39, 0x9D3B,
                 0x9D3D, 0x9D3F, 0x9D41, 0x9D42, 0x9D44,
                 0x9D47, 0x9D49, 0x9D4B, 0x9D4C, 0x9D4E,
                 0x9D51, 0x9D52, 0x9D55, 0x9D57, 0x9D5A,
                 0x9D5D, 0x9D5E, 0x9D5F, 0x9D61, 0x9D64,
                 0x9D66, 0x9D6A, 0x9D6C, 0x9D6D, 0x9D6E,
                 0x9D70, 0x9D74, 0x9D76, 0x9D78, 0x9D7A,
                 0x9D7D, 0x9D81, 0x9D84, 0x9D88, 0x9D8A,
                 0x9D8D, 0x9D91, 0x9D94, 0x9D98, 0x9D9B,
                 0x9D9C, 0x9D9D, 0x9D9E, 0x9DA1, 0x9DA3,
                 0x9DA6, 0x9DAA, 0x9DAC, 0x9DAF, 0x9DB1,
                 0x9DB5, 0x9DB7, 0x9DB9, 0x9DBC),
                "Dialog, for the mouse: a button held, a click on a "
                "field, on a button or outside the box. The bench has no "
                "mouse"),
            **dict.fromkeys(
                (0x9F3E, 0x9F40, 0x9F43, 0x9F45, 0x9F47,
                 0x9F49, 0x9F4C, 0x9F4E, 0x9F50, 0x9F51,
                 0x9F53, 0x9F55, 0x9F57, 0x9F5A, 0x9F5C,
                 0x9F5D, 0x9F5F, 0x9F64, 0x9F66, 0x9F67,
                 0x9F68, 0x9F6A, 0x9F6C, 0x9F6E, 0x9F6F,
                 0x9F71, 0x9F73, 0x9F75, 0x9F77, 0x9F78,
                 0x9F79, 0x9F7B, 0x9F7D, 0x9F7F),
                "InpStr, for the mouse: a click in the line that is typed "
                "into"),
            **dict.fromkeys(
                (0x9FDC, 0x9FDE, 0x9FE1, 0x9FE3, 0x9FE5,
                 0x9FE7, 0x9FEA, 0x9FEC, 0x9FEE, 0x9FEF,
                 0x9FF0, 0x9FF3, 0x9FF5, 0x9FF8, 0x9FFA,
                 0x9FFB, 0x9FFC, 0x9FFE, 0xA000, 0xA002,
                 0xA004, 0xA006, 0xA008),
                "Select, for the mouse: a click on a line of the group"),
            **dict.fromkeys(
                (0xA05F, 0xA060, 0xA062, 0xA065, 0xA069,
                 0xA06B, 0xA070, 0xA072, 0xA073, 0xA074,
                 0xA076, 0xA078, 0xA079),
                "Slct20, which measures a field for a click of the mouse"),
            **dict.fromkeys(
                (0xA092, 0xA094, 0xA097, 0xA099, 0xA09B,
                 0xA09D, 0xA0A0, 0xA0A2, 0xA0A4, 0xA0A5,
                 0xA0A8, 0xA0AA, 0xA0AC, 0xA0AF, 0xA0B1,
                 0xA0B2, 0xA0D2, 0xA0D3, 0xA0D6, 0xA0D8,
                 0xA0D9),
                "FlipFl, for the mouse: a click on the box to tick"),
            **dict.fromkeys(
                (0xA11B, 0xA11D, 0xA120, 0xA122, 0xA124,
                 0xA126, 0xA128, 0xA12A, 0xA12B, 0xA12C,
                 0xA12E, 0xA130, 0xA133, 0xA136, 0xA138,
                 0xA139, 0xA13A, 0xA13C, 0xA13E, 0xA140,
                 0xA142, 0xA146, 0xA148, 0xA14C, 0xA14E,
                 0xA150, 0xA153, 0xA155, 0xA157, 0xA158,
                 0xA159, 0xA15B, 0xA15D, 0xA15F, 0xA161,
                 0xA163, 0xA165, 0xA167, 0xA168, 0xA169,
                 0xA16A, 0xA16E, 0xA171, 0xA173, 0xA175,
                 0xA178, 0xA17A, 0xA17D, 0xA17F, 0xA181,
                 0xA183, 0xA185, 0xA187, 0xA189, 0xA18B,
                 0xA18E, 0xA192, 0xA194, 0xA197, 0xA198,
                 0xA199, 0xA19A, 0xA19C, 0xA19E, 0xA1A1),
                "MnMn10, for the mouse: a click on a button and a button "
                "held on one"),
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
        functions=("F80_00_c", "F32_00_c", "Run_00_c", "F62_01_c",
                   "F68_00_c", "F82_01_c", "F74_00_c", "F73_00_c",
                   "F77_00_c"),
        unreached={
            **dict.fromkeys(
                (0x20AD, 0x20B0, 0x20B3, 0x20B5, 0x20B7, 0x20BA, 0x212F,
                 0x2130, 0x2132, 0x2135, 0x2137, 0x2139, 0x213C, 0x213E,
                 0x2140, 0x2142, 0x2144, 0x2146, 0x2148, 0x214A, 0x214C,
                 0x214E, 0x2151, 0x2155, 0x2157, 0x2158, 0x2159, 0x215D,
                 0x2161, 0x2163, 0x2166, 0x2167, 0x2168, 0x216A, 0x216C,
                 0x216E, 0x2171, 0x2172, 0x2174, 0x2176, 0x2178, 0x217A,
                 0x217C, 0x217E, 0x217F, 0x2180, 0x2182, 0x2186, 0x2189,
                 0x218A, 0x218B, 0x218D, 0x218F, 0x2190, 0x2192, 0x2193,
                 0x2195, 0x2197, 0x2199, 0x219A, 0x219C, 0x219D, 0x219E,
                 0x219F, 0x21A3, 0x21A5, 0x21A8, 0x21AA, 0x21AD, 0x21AE,
                 0x21AF, 0x21B0, 0x21B1, 0x21B3, 0x21B5, 0x21B6, 0x21B8,
                 0x21BA, 0x21BD, 0x21BF),
                "The user menu, for the mouse, with all of F32_70: the "
                "bench has none"),
            0x2201: "The box of the drives, for a DOS that cannot say "
                    "which letter a floppy drive goes by",
            0x2245: "The box of the drives, for more drives than fit a "
                    "box as wide as its title: the bench has three",
            **dict.fromkeys(
                (0x2351, 0x2353, 0x2356, 0x2358, 0x235A, 0x235D, 0x235F,
                 0x2361, 0x2364, 0x2366, 0x2367, 0x2368, 0x236A, 0x236D,
                 0x236F, 0x2371, 0x2373, 0x2375, 0x2377, 0x2379, 0x237C,
                 0x237E, 0x2380, 0x2381, 0x2382, 0x2385, 0x2387, 0x2389,
                 0x238A, 0x238B, 0x238D, 0x238F, 0x2390, 0x2391, 0x2392,
                 0x2394, 0x2395, 0x2396, 0x2398, 0x239A, 0x239D, 0x239F,
                 0x23A2, 0x23A4, 0x23A6, 0x23A7, 0x23A8, 0x23AA, 0x23B1,
                 0x23B3, 0x23B5, 0x23B7, 0x23B9, 0x23BB, 0x23BD, 0x23BF,
                 0x23C2, 0x23C5, 0x23C7, 0x23C9, 0x23CC, 0x23CE, 0x23CF,
                 0x23D0, 0x23D2, 0x23D4, 0x23D6, 0x23D8, 0x23DA, 0x23DC,
                 0x23DD, 0x23DE, 0x23E2, 0x23E5, 0x23E8, 0x23E9, 0x23EA),
                "The box of the drives, for the mouse"),
            **dict.fromkeys(
                (0x2475, 0x2477, 0x247A),
                "The box of the drives, for a drive that is not ready: "
                "the bench has none"),
            **dict.fromkeys(
                (0x25D4, 0x25D7, 0x25DA, 0x25DC, 0x25DE, 0x25E0, 0x2618),
                "The history, for the mouse"),
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
        routines=("ClsLine", "PutKey", "PutKey0", "PutMnu", "PutPrm",
                  "DosCom"),
        functions=("ClsLine_c", "PutKey_c", "PtMnu1_c", "PutPrm_c",
                   "DosCom_c"),
        unreached={
            **dict.fromkeys(
                (0xA867, 0xA869, 0xA86A),
                "PutPrm, for a drive whose current directory DOS will "
                "not tell: the disks of the bench are always ready"),
        },
    ),
    Batch(
        title="The entries of an extension file",
        source="extfile.c",
        routines=("FndItem", "FindExt"),
        functions=("FindExt_c",),
    ),
    Batch(
        title="Waiting for a key, and a line that is typed into",
        source="input.c",
        routines=("Input", "Input0", "Rnd", "EdLin", "EdSub1", "EdSub2",
                  "CmpSym", "Search"),
        functions=("Input_c", "Rnd_c", "EdLin_c", "CmpSym_c", "Search_c"),
        unreached={
            **dict.fromkeys(
                (0xB1E6, 0xB1EE, 0xB1F0, 0xB1F3, 0xB1F5, 0xB1F8, 0xB1FA,
                 0xB1FE, 0xB200, 0xB203, 0xB205, 0xB208, 0xB20A),
                "Input0, when the time has come to blank the screen, or "
                "its minute, or midnight has passed meanwhile: a minute "
                "at the least of the machine's time in which no key is "
                "pressed, and the harness presses the next one as soon "
                "as the program waits"),
            **dict.fromkeys(
                (0xB328, 0xB32C, 0xB32E),
                "Input0, for the mouse in the corner of the screen that "
                "blanks it. The bench has no mouse"),
            **dict.fromkeys(
                (0xB379, 0xB37E, 0xB380, 0xB383, 0xB385, 0xB387, 0xB38B,
                 0xB38D, 0xB38F, 0xB391, 0xB395, 0xB397, 0xB399, 0xB39A,
                 0xB39E, 0xB3A0, 0xB3A3, 0xB3A6, 0xB3A7, 0xB3A9, 0xB3AB,
                 0xB3AD, 0xB3AF, 0xB3B1, 0xB3B3, 0xB3B5, 0xB3B7, 0xB3B9,
                 0xB3BB, 0xB3BE, 0xB3C0, 0xB3C3, 0xB3C5, 0xB3C7, 0xB3CA,
                 0xB3CC, 0xB3CE, 0xB3D1, 0xB3D3, 0xB3D6, 0xB3D9, 0xB3DC,
                 0xB3DE, 0xB3E0, 0xB3E1, 0xB3E4, 0xB3E5, 0xB3E8, 0xB3EA,
                 0xB3ED, 0xB3EE, 0xB3F1, 0xB3F3, 0xB3F8, 0xB3FA, 0xB3FC,
                 0xB3FE, 0xB401, 0xB403, 0xB406, 0xB408, 0xB40A, 0xB40D,
                 0xB40F, 0xB411, 0xB414, 0xB416, 0xB419, 0xB41B, 0xB41D,
                 0xB41F, 0xB421),
                "Input0, for the mouse: a button held on a key of the "
                "key bar, and the key that is pressed when it is let go "
                "there"),
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
                (0x81A1, 0x81A4, 0x81A7, 0x81A9, 0x81AE, 0x81B0, 0x81B5,
                 0x81B8, 0x81BA, 0x81BF, 0x81C1, 0x81C3, 0x81C8, 0x81CA,
                 0x81CC, 0x81CE, 0x81D0, 0x81D2, 0x81D4, 0x81D6, 0x81D9,
                 0x81DC),
                "F65_00, for the mouse: the right button held over "
                "the lines. The bench has no mouse"),
            **dict.fromkeys(
                (0x8237, 0x823A, 0x823C, 0x823E, 0x8240, 0x8242, 0x8243,
                 0x8248, 0x824B, 0x824D, 0x824E, 0x824F, 0x8251, 0x8254,
                 0x8255, 0x8257, 0x8259),
                "F65_00, scrolling the lines by one: for a map of "
                "more lines than its window has"),
            **dict.fromkeys(
                (0x82B9, 0x82BB, 0x82BE, 0x82C1, 0x82C3, 0x82C5, 0x82C7,
                 0x82C9, 0x82CC, 0x82CF, 0x82D2, 0x82D4, 0x82D6, 0x82D7,
                 0x82D9, 0x82DC, 0x82DD, 0x82DF, 0x82E1, 0x82E4, 0x82E7,
                 0x82E8, 0x82E9, 0x82ED, 0x82F1, 0x82F4, 0x82F5, 0x82F6,
                 0x82F8, 0x82FD, 0x82FF, 0x8303, 0x8307, 0x8309, 0x830B,
                 0x830D, 0x830F, 0x8312, 0x8314, 0x8316, 0x8318, 0x831A,
                 0x831C, 0x831F, 0x8321, 0x8323, 0x8324, 0x8325, 0x8327,
                 0x832A, 0x832C, 0x832F, 0x8332, 0x8333, 0x8334, 0x8336,
                 0x8339, 0x833C, 0x833E, 0x8340, 0x8341, 0x8345, 0x8348,
                 0x8349, 0x834B, 0x834E, 0x8351, 0x8356, 0x8357, 0x8359,
                 0x835C, 0x835E, 0x8360, 0x8364, 0x8366, 0x8368, 0x836B,
                 0x836C, 0x836D, 0x836F, 0x8371, 0x8373, 0x8375, 0x8378,
                 0x837A, 0x837C, 0x837D, 0x837E, 0x8380, 0x8382, 0x8384,
                 0x8387, 0x838A, 0x838C, 0x838F, 0x8390, 0x8391, 0x8393,
                 0x8395, 0x839A, 0x83A0, 0x83A2, 0x83A5, 0x83A8, 0x83AB,
                 0x83AD, 0x83AF, 0x83B1, 0x83B4, 0x83B8, 0x83BA, 0x83BD,
                 0x83BE, 0x83C0),
                "F65_00, for the mouse: a button on a line, on the "
                "buttons, outside the window, held above the lines or "
                "below them, the middle button, and a second click on "
                "a line. The bench has no mouse"),
            **dict.fromkeys(
                (0x84EA, 0x84EE, 0x84F0, 0x84F2, 0x84F5, 0x84F7, 0x84F9,
                 0x84FB, 0x8501, 0x8504),
                "F65_00, for a program that stays in the memory and "
                "is not released while another is: the bench has one "
                "such program, FDAPM, which does not stay twice"),
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
