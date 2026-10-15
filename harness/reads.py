#!/usr/bin/env python3
"""How much the C code reads of each of the program's constants.

The test of a C tree's data (datacheck.py) compares as many bytes
as an object has. The program reads more of some: it copies a name as 13
bytes whatever its length, a title as 80, and writes a line with a table of
colours of which the line may ask for the fourth. What lies behind such
an object in the original is read too, so the translation has to have the
object that long, with those bytes; then the comparison holds them.

This lists every place where the code names a constant, a text or a table
of the files of constants or of the code's own, and says
how many bytes of it are read there. Most places it knows by their form:
a loop to a count, MvzLine, MvsBw and MainFil with their lengths, a text
that is read as far as its zero byte, a name that the system is handed
or bytes that it is to write, a box or a dialog that is handed on. The
others are in REVIEWED below, each with what reading the code found. A place that is neither is reported, and so is
an object that is shorter than what is read of it: datacheck.py fails
for both.

What this cannot see: a read that does not begin with the name of the
constant in a file of code. Those are the texts and tables that a box or
a dialog names, which are read as far as a zero byte or as their kind is
laid out, and datacheck.py looks at the zero; and a table that is read at
an index which the program does not keep inside it, where REVIEWED says
"own" for one whose indexes come from its own data. It goes by the text
of the C, not by what the compiler made of it: a constant that is handed
through a variable is known only as far as REVIEWED says.

A constant of the setup program has St in front of its name in a tree
that has that program in itself (see datacheck.py), and is a constant
like any other here: its places are found by that name, and REVIEWED
has it by that name, ("stopts.c", "StConf_P"). Which of the original's
two programs its bytes are held against is datacheck.py's to know, not
this tool's. A function of the setup's that reads a box or a dialog
handed to it goes into READERS. The files of the setup
program are SETUP_FILES: a tree that is declared to have no setup
program has none of them, and what REVIEWED has for them is not held
against it; nor is what it has for a table of the setup's in a file
that both programs have, SETUP_PLACES.

    reads.py BUILD_DIR [SOURCE_DIR]
                                every place, with the bytes read there:
                                build/c for the C translation, whose
                                files are src/c, build/portable_c for
                                the portable C; and with the directory
                                of its files, a build that is named
                                otherwise than its tree

Nothing here is of 16 bits in kind: the places are found in the text of
the source, and a count is of bytes of a text or of a table of bytes,
which are the same in a program of 16 bits and in a flat one of 32. What
differs is which file of the system's functions the program is built of,
SYSTEM_FILE, and how long an object with pointers in it is, a box or a
dialog, of which no place reads a count of bytes: it is handed on and
read as it is laid out.

A constant is handed on in one of two forms, and both are known. The C
translation has it as a number, its offset in the segment of the tables,
OFFSET(Name), and reads it through a far pointer made of that,
AT(AsmSeg(), OFFSET(Name) + 1). The portable C has it as a pointer: Name
and Name + 1 for a text or a table of bytes, which a file of code
declares as an array of them, and &Name for a box or a dialog, which it
declares as the structure that the thing begins with, `extern const
struct BOXTOP Name;` and `extern const struct BOXHD Name;`. What was
held against the offset, `box == OFFSET(Name)`, is held against the
pointer, `box == &Name`, and reads nothing either way.
"""
import re
import sys
from collections import defaultdict
from pathlib import Path

import deviations

ZERO = "as far as its zero"     # a text
TABLE = "as it is laid out"     # a box, a dialog, buttons: by their kind
OWN = "within itself"           # by an index that its own data bounds
NONE = "not read"               # only its address or its size is taken

# The functions of src/portable_c/sys.h that take a name, a text that
# ends in a zero byte.
NAMED = ("FileCreate", "FileOpen", "FileOpenBare", "FileDelete",
         "FileRename", "FileAttributes", "FileSetAttributes",
         "FileTrueName", "FindFirst", "DirMake", "DirRemove", "DirChange",
         "ProgramRun", "ProgramRunBare", "ProgramOfFile", "ProgramOpenFile")

# What a call reads of a constant that is handed to it, by its offset
# or as a pointer.
READERS = ("Dialog", "DialBox", "OncZoom", "MinMnu", "MnMn10", "MnMn40",
           "Ask")

# The files of code of the setup program, in a tree that has it in itself
# (TODO.md, "The setup program in the portable C: the plan"). An entry of
# REVIEWED for one of them is none for a tree that is declared to have no
# setup program, deviations.TREES.
SETUP_FILES = ("stmain.c", "stopts.c", "stcolor.c", "sthelp.c")

# And the constants of the setup program that a file of the main program
# has, where a routine that both programs have reads the setup's tables:
# the entries of REVIEWED for them are none for such a tree either.
SETUP_PLACES = (("bars.c", "StKeyB_T"), ("bars.c", "StKeyC_T"),
                ("bars.c", "StKey_S"))

# The file of the system's functions, by the kind of program that a
# tree is: whether the program is a flat one of 32 bits -> the file. A
# tree has one of the two, the C translation dos.c and the portable C
# sys_dos.c, and what REVIEWED has for the other is not held against the
# program.
SYSTEM_FILE = {False: "dos.c", True: "sys_dos.c"}

# A file of code that a tree has under another name than the C
# translation, which is the name that REVIEWED and GONE have it by: tree
# -> the tree's name -> that name. An entry of the two tables is one for
# both trees so, and what is printed of a place names the tree's file.
FILES = {"portable_c": {"exec.c": "resident.c"}}

# Places that are not known by their form: (file, name) -> (what is read,
# how that is known). A number is a count of bytes from the start of the
# object. It holds for every place in the file that names the constant
# and is not known otherwise.
REVIEWED = {
    ("bars.c", "KeyB_T"): (OWN, "the index is held below its size"),
    ("bars.c", "KeyC_T"): (OWN, "a row of KeyB_T and one of ten keys"),
    ("bars.c", "Key_S"): (OWN, "a name of KeyC_T, six bytes each"),
    ("bars.c", "StKeyB_T"): (OWN, "the index is held below its size"),
    ("bars.c", "StKeyC_T"): (OWN, "a row of StKeyB_T and one of ten keys"),
    ("bars.c", "StKey_S"): (OWN, "a name of StKeyC_T, six bytes each"),
    ("bars.c", "PtMnu_B"): (1, "the colours of the bar's names, PtMnu_S, "
                               "which has no markup: the first only"),
    ("bars.c", "PtMnu_S"): (OWN, "the names of the menus, counted by "
                                 "their marks, five of them"),
    ("cmdline.c", "AltSr_P"): (4, "the colours of a line: any of four"),
    ("cmdline.c", "Mask_S"): (ZERO, "copied as far as its zero"),
    ("cmdline.c", "AltSr_T"): (OWN, "compared up to its size"),
    ("cmdline.c", "Ascii_T"): (OWN, "by the place found in AltSr_T"),
    ("commands.c", "Delay_T"): (OWN, "as far as its 0, or by a line of "
                                     "the dialog, one of six"),
    ("commands.c", "Menu_F"): (13, "M64_02 copies a name as 13 bytes"),
    ("commands.c", "Ext_F"): (13, "M64_02 copies a name as 13 bytes"),
    ("commands.c", "ViewE_F"): (13, "M64_02 copies a name as 13 bytes"),
    ("commands.c", "EditE_F"): (13, "M64_02 copies a name as 13 bytes"),
    ("commands.c", "User_S"): (80, "M64_02 copies a title as 80 bytes"),
    ("commands.c", "ExFl_S"): (80, "M64_02 copies a title as 80 bytes"),
    ("commands.c", "UsMnu_S"): (ZERO, "the include line of M64_02's box"),
    ("commands.c", "ExtFl_S"): (ZERO, "the include line of M64_02's box"),
    ("copy.c", "Copy0_S"): (20, "F35_57 copies the title as the 20 bytes "
                                "from CpVar1 to CpVar2"),
    ("copy.c", "Ren0_S"): (20, "F35_57 copies the title as the 20 bytes "
                               "from CpVar1 to CpVar2"),
    ("copy.c", "Move0_S"): (ZERO, "an include line"),
    ("copy.c", "RnMv_S"): (15, "F35_57 copies 15 bytes of words"),
    ("copy.c", "CopFi_S"): (16, "four bytes from the twelfth on"),
    ("copy.c", "CopTo_S"): (5, "three bytes from the second on"),
    ("copy.c", "RenEr_B"): (TABLE, "a box, handed to C10_10 for Ask"),
    ("copy.c", "DelEr_B"): (TABLE, "a box, handed to C10_10 for Ask"),
    ("dirfunc.c", "DlWr4_S"): (81, "F48_00 copies a path and a name, "
                                   "LenPath + 13 bytes"),
    ("dirfunc.c", "Mask_S"): (6, "five bytes from the first or the second "
                                 "on: DirFunc"),
    ("dirfunc.c", "Tree_F"): (14, "copied as 14 bytes"),
    ("dirfunc.c", "MkDi1_B"): (TABLE, "a dialog, for F47_01"),
    ("dirfunc.c", "MkDi2_B"): (TABLE, "a dialog, for F47_01"),
    ("dirfunc.c", "DelEr_B"): (TABLE, "a dialog, handed to F38_43 for "
                                      "Dialog"),
    ("dirfunc.c", "CntDi_B"): (TABLE, "a dialog, handed to F38_43 for "
                                      "Dialog"),
    ("dirfunc.c", "NoAtr_S"): (ZERO, "an include line"),
    ("dirfunc.c", "NoFnd_S"): (ZERO, "an include line"),
    ("dirfunc.c", "Empty_S"): (ZERO, "an include line"),
    ("dirfunc.c", "Subdi_S"): (ZERO, "an include line"),
    ("dirfunc.c", "Del_S"): (ZERO, "an include line"),
    ("dirfunc.c", "Fnc_T"): (OWN, "by FncAddr, one of five numbers"),
    ("edit.c", "Menu_F"): (13, "T64_00 compares a name of 13 bytes at the "
                               "most"),
    ("edit.c", "Ext_F"): (13, "T64_00 compares a name of 13 bytes at the "
                              "most"),
    ("edit.c", "ViewE_F"): (13, "T64_00 compares a name of 13 bytes at "
                                "the most"),
    ("edit.c", "EditE_F"): (13, "T64_00 compares a name of 13 bytes at "
                                "the most"),
    ("edit.c", "EdFl_S"): (ZERO, "an include line, and a title"),
    ("edit.c", "EdtH_P"): (1, "the colour of the help's lines, which have "
                              "no markup: the first only"),
    ("edit.c", "EdtH_S"): (ZERO, "six lines, each to its zero: it has six"),
    ("edit.c", "EdtH1_S"): (ZERO, "a title and six lines: it has seven"),
    ("edit.c", "EdtH2_S"): (ZERO, "a title and six lines: it has seven"),
    ("find.c", "Mask_S"): (8, "four bytes from the first or the fifth on: "
                              "FindFil"),
    ("find.c", "FindColors"): (4, "the colours of a line: any of four"),
    ("find.c", "FnFl_P"): (3, "the colours of buttons, three of them"),
    ("help.c", "Help_P"): (4, "the colours of a line: any of four"),
    ("help.c", "Title_S"): (ZERO, "written as a line"),
    ("help.c", "HlMn1_S"): (ZERO, "written as a line"),
    ("help.c", "HlMn2_S"): (ZERO, "written as a line"),
    ("help.c", "Help_F"): (20, "13 bytes from the seventh on, the header "
                               "of the help file"),
    ("help.c", "HlMn1_D"): (TABLE, "buttons, for MinMnu"),
    ("help.c", "HlMn2_D"): (TABLE, "buttons, for MinMnu"),
    ("info.c", "PtIn_P"): (2, "the colours of the Info's lines, which "
                              "have ^r and no other: two"),
    ("keys.c", "Sel_S"): (ZERO, "an include line"),
    ("keys.c", "Unsl_S"): (ZERO, "an include line"),
    ("keys.c", "Inv_S"): (ZERO, "an include line"),
    ("main.c", "AltSr_T"): (38, "compared up to ALT_KEYS"),
    ("main.c", "Menu_T"): (OWN, "the word of one of its six tables, which "
                                "F39_00 of menu.c numbers; and of that "
                                "table, which this word names, the word of "
                                "the item chosen, one of the menu's, and "
                                "of the first eight of the Files menu for "
                                "the two tables that are eight long"),
    ("main.c", "Err_S"): (7, "six bytes from the first on, for the box "
                             "of a critical error"),
    ("main.c", "OnDr_S"): (11, "copied as 11 bytes"),
    ("main.c", "OnDv_S"): (10, "copied as 10 bytes"),
    ("main.c", "Errs_S"): (OWN, "the texts of the errors, counted by "
                                "their zeros: the number is held below "
                                "how many there are"),
    ("main.c", "FatEr_S"): (ZERO, "copied as far as its zero"),
    ("memory.c", "Memo_P"): (4, "one of four colours"),
    ("memory.c", "Memo1_P"): (3, "the colours of buttons, three of them"),
    ("memory.c", "DosMe_S"): (ZERO, "four bytes, and from the fourth on "
                                    "as far as the zero"),
    ("menu.c", "PtMnu_S"): (OWN, "F40_50 goes through the names of the "
                                 "menus by their marks, five of them, as "
                                 "PtMnu1 of bars.c does"),
    ("menu.c", "MenuLn"): (OWN, "the lines of a menu, by MnuTab; a menu "
                                "that is none reads elsewhere, as the "
                                "original does: see TODO.md"),
    ("menu.c", "MnuHlp"): (OWN, "by MnuTab and the item; a menu that is "
                                "none reads elsewhere: see TODO.md"),
    ("menu.c", "MnuTab"): (OWN, "by the menu, one of five; a menu that "
                                "is none reads in front of it: see "
                                "TODO.md"),
    ("menu.c", "KeyTab"): (OWN, "compared up to its size"),
    ("menu.c", "Ctrl_S"): (OWN, "as far as a hyphen: it has two"),
    ("misc.c", "Title_S"): (NONE, "AsmSeg takes its segment, which is "
                                  "that of every constant"),
    ("panel.c", "Panel_P"): (2, "the colours of the titles of the "
                                "columns, which have ^r and no other"),
    ("panel.c", "PutPt_P"): (2, "one colour, of the second for the "
                                "current panel: the line is ^V or a "
                                "title, without markup"),
    ("panel.c", "PtFs_P"): (1, "the colour of the line that counts "
                               "the selected files, numbers and words "
                               "without markup: the first only. The "
                               "name of a file is not written as a "
                               "line"),
    ("panel.c", "Panel_T"): (OWN, "a row as far as its 0"),
    ("panel.c", "BytIn_S"): (25, "copied in pieces, 25 bytes in all"),
    ("format.c", "PowersOfTen"): (OWN, "nine of them"),
    ("format.c", "DirMarker"): (OWN, "one of two rows of nine"),
    ("memory.c", "Command"): (ZERO, "CutPath goes to its zero, and "
                                    "F65_29 copies as far as that"),
    ("resident.c", "Vers_S"): (ZERO, "written as far as its zero"),
    ("resident.c", "Command"): (ZERO, "NotFind writes it as far as its "
                                      "zero"),
    **{("resident.c", text): (ZERO, "OutStr0 writes it as far as its zero")
       for text in ("NoRun_S", "FnFil_S", "BadFl_S", "Retr_S", "Aloc_S")},
    ("sys_dos.c", "Ended_T"): (OWN, "ProgramEndedTake copies it whole, by "
                                  "its size"),
    ("sys_dos.c", "NoStart_S"): (35, "DOS writes it as far as its dollar "
                                   "sign, which is its last byte but the "
                                   "zero"),
    ("start.c", "NcEnv"): (OWN, "compared at its size"),
    ("start.c", "TempEnv"): (OWN, "compared at its size"),
    ("start.c", "Comsp"): (OWN, "compared at its size"),
    ("start.c", "Init_F"): (13, "ReadOne copies a name as 13 bytes"),
    ("start.c", "Ext_F"): (13, "ReadOne copies a name as 13 bytes"),
    ("stmain.c", "StPalets"): (330, "StDefault copies the setup, CmnIni "
                                    "bytes"),
    ("stmain.c", "StMainHlp"): (OWN, "by the item of the menu that the "
                                     "cursor is on, one of nine"),
    ("stmain.c", "StStErr_S"): (49, "DOS writes it as far as its dollar "
                                    "sign, which is its last byte"),
    ("stmain.c", "StNoFnd_S"): (ZERO, "an include line"),
    ("stmain.c", "StNoIni_S"): (ZERO, "an include line"),
    ("stopts.c", "StDelay_T"): (OWN, "as far as its 0, or by a line of "
                                     "the dialog, one of six"),
    ("stopts.c", "StInf05_S"): (44, "StF09_00 has the capitals and the "
                                    "small letters of 40 characters "
                                    "from the fifth on"),
    ("stopts.c", "StInf09_S"): (47, "StF09_00 has the capitals and the "
                                    "small letters of 40 characters "
                                    "from the eighth on"),
    ("tree.c", "PutTr_P"): (4, "the colours of the tree, four"),
    ("treebox.c", "Scan_P"): (4, "the colours of a line: any of four"),
    ("treebox.c", "ChTr_P"): (4, "the colours of the tree and of a line: "
                                 "four"),
    ("start.c", "CrLf_S"): (ZERO, "copied as far as a zero byte, with "
                                  "the text behind it"),
    ("usermenu.c", "CrLf_S"): (2, "written to the list file, two bytes"),
    ("view.c", "Byts_S"): (6, "six bytes, or five for one byte"),
    ("view.c", "Menu_F"): (13, "F34_40 compares an extension with its "
                               "four characters"),
    ("view.c", "ViewE_F"): (13, "copied as a name, 13 bytes"),
    ("view.c", "EditE_F"): (13, "copied as a name, 13 bytes"),
    ("view.c", "DrIn_F"): (13, "F34_10 copies a name as 13 bytes"),
    ("view.c", "Run2_S"): (80, "the box of a program that cannot be run "
                               "copies 80 bytes"),
    ("view.c", "ViFl_S"): (ZERO, "an include line, and a title"),
    ("view.c", "EdFl_S"): (ZERO, "an include line, and a title"),
    ("view.c", "RunEr_S"): (ZERO, "an include line"),
    ("view.c", "Run1_S"): (ZERO, "an include line"),
    # The colours of the setup program.
    ("stcolor.c", "StPalets"): (240, "a colour of one of the six "
                                     "palettes, 40 colours of six bytes "
                                     "each, which the object begins "
                                     "with: by the place that the colour "
                                     "table has it at"),
    ("stcolor.c", "StColrs_S"): (OWN, "the names of the items, found by "
                                      "counting their zeros up to an item "
                                      "that there is, and counted up to "
                                      "the empty name that ends them"),
    ("stcolor.c", "StColNm_S"): (OWN, "the names of the colours, found by "
                                      "counting their zeros: a colour "
                                      "below 16, and 16 or 17 for what "
                                      "Shadow can be besides, of 18"),
    ("stcolor.c", "StColNm_P"): (1, "the colour of a colour's name, and "
                                    "of \"on\" with one put in: neither "
                                    "asks for another colour"),
    ("stcolor.c", "StColPage"): (OWN, "by the item under the cursor, "
                                      "which is one of the 39"),
    ("stcolor.c", "StPagTab"): (OWN, "by the picture that StColPage has "
                                     "for an item, 1 to 4"),
    **dict.fromkeys(
        (("stcolor.c", "StColMd_S"), ("stcolor.c", "StMonMd_S"),
         ("stcolor.c", "StBwPal_S"), ("stcolor.c", "StLapTp_S")),
        (ZERO, "the name of the display or of the palette, which "
               "StF07_01 writes as far as its zero")),
    # The help of the setup program.
    ("sthelp.c", "StHelp_F"): (20, "13 bytes from the seventh on, the "
                                   "header of the help file"),
    ("sthelp.c", "StHelp_P"): (4, "the colours of a line: any of four"),
}

_NUMBER = {"LenPath": 68, "LenStr": 64}


def _count(text, name, sizes):
    text = text.strip()
    if text.isdigit():
        return int(text)
    if text in _NUMBER:
        return _NUMBER[text]
    # A sizeof may be made a number with a sign first, for a loop whose
    # index is one: `i < (i32)sizeof Name`.
    match = re.fullmatch(r"(?:\(\s*\w+\s*\)\s*)?sizeof\s*\(?\s*(\w+)\s*\)?",
                         text)
    if match and match[1] in sizes:
        return sizes[match[1]]
    return None


def _form(statement, name, sizes):
    """What `statement` reads of `name` if its form says so: a number of
    bytes or one of the words above, or None."""
    n = re.escape(name)
    # A far pointer made of the constant's offset is the constant.
    statement = re.sub(rf"\bAT\(\s*\w+(?:\(\))?\s*,\s*OFFSET\(\s*{n}\s*\)"
                       rf"(\s*\+\s*\d+)?\s*\)", rf"{name}\1", statement)
    rest = re.sub(rf"sizeof\s*\(?\s*{n}\b\s*\)?", "", statement)
    # Held against a variable, as the number that it is in the one form
    # or as the pointer to it in the other: nothing of it is read.
    rest = re.sub(rf"[!=]=\s*(?:OFFSET\(\s*{n}\s*\)|&?\s*{n}\b"
                  rf"(?!\s*[\[+(.-]))", "", rest)
    if not re.search(rf"\b{n}\b", rest):
        return NONE
    plus = rf"\b{n}\b(?:\s*\+\s*(\d+))?"
    match = re.search(rf"\b(MvzLine|MvsBw)\s*\([^;]*?,\s*{plus}\s*,\s*"
                      rf"([\w ()]+?)\s*[,)]", statement)
    if match:
        count = _count(match[3], name, sizes)
        if count is not None:
            return int(match[2] or 0) + count
    if re.search(rf"\bMainFil\s*\([^,]+,\s*\b{n}\b\s*\)", statement):
        return 13
    if re.search(rf"(\.(line|include)|->name|\.name)\s*=\s*{plus}\s*;"
                 rf"|\b(MvzTxt|OutStr1|OpenFil|ChDir|MovDz0|PUT|F65_29)\s*"
                 rf"\([^;]*\b{n}\b", statement):
        return ZERO
    # The system is handed a name, which it reads as far as its zero, or
    # bytes to write, with their count: the functions of sys.h.
    if re.search(rf"\b({'|'.join(NAMED)})\s*\([^;]*\b{n}\b", statement):
        return ZERO
    match = re.search(rf"\bFileWrite\s*\([^;]*?,\s*{plus}\s*,\s*"
                      rf"([\w ()]+?)\s*\)", statement)
    if match:
        count = _count(match[2], name, sizes)
        if count is not None:
            return int(match[1] or 0) + count
    # Handed on to a routine that reads it as its kind is laid out, or
    # kept for one: by its offset, or by a pointer to it, which is its
    # name where it is an array of bytes and its address where the file
    # declares it as what it begins with.
    handed = rf"(?:OFFSET\(\s*&?{n}\s*\)|&?\s*{n}\s*(?=[,;)]))"
    if re.search(rf"\b({'|'.join(READERS)})\s*\((?:[^();]|\([^();]*\))*"
                 rf"(?<![\w.>]){handed}", statement) or re.search(
                     rf"\b(box|report|result|shown)\s*=\s*{handed}", statement):
        return TABLE
    # A loop to a count, over an index into the constant.
    loop = re.search(r"for\s*\(\s*(?:(\w+)\s*=\s*(\d+))?\s*;\s*(\w+)\s*<\s*"
                     r"([\w ()]+?)\s*;", statement)
    index = re.search(rf"\b{n}\s*\[\s*(?:(\d+)\s*\+\s*)?(\w+)"
                      rf"(?:\s*\+\s*(\d+))?\s*\]", statement)
    if loop and index and index[2] == loop[3]:
        count = _count(loop[4], name, sizes)
        if count is not None:
            return int(index[1] or index[3] or 0) + count
    index = re.search(rf"\b{n}\s*\[\s*(\d+)\s*\]", statement)
    if index and not re.search(rf"\b{n}\s*\[\s*[A-Za-z_]", statement):
        return int(index[1]) + 1
    return None


def places(source, objects, variables=()):
    """Every place where a file of code in the directory `source` names
    one of `objects`, a list of (file, name, address, length) as
    datacheck.Translation has them: (file, line, name, the file of the
    object, its length, what is read there, how that is known).
    `variables` are names among them that are no constants."""
    by_name = defaultdict(list)
    for module, name, _, length in objects:
        if name not in variables:
            by_name[name].append((module, length))
    names = sorted(by_name, key=len, reverse=True)
    pattern = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    found = []
    for path in sorted(source.glob("*.c")):
        if path.name.endswith("_t.c"):
            continue
        listed_as = FILES.get(source.name, {}).get(path.name, path.name)
        text = re.sub(r"/\*.*?\*/",
                      lambda match: re.sub(r"[^\n]", " ", match[0]),
                      path.read_text(), flags=re.S)
        lines = text.split("\n")
        sizes = {name: length for name in by_name
                 for module, length in by_name[name] if module == path.name}
        for number, line in enumerate(lines):
            for match in pattern.finditer(line):
                name = match[1]
                # A function may have the name of a constant of another
                # file: Command of main.c, and that of resident.c, which
                # the portable C has as exec.c.
                if line[match.end():].lstrip().startswith("("):
                    continue
                if re.match(rf"\s*(extern|static|const)\b[^=;(]*\b{name}\b"
                            rf"\s*\[", line) or re.match(
                                rf"\s*extern\s+const\s+struct\s+\w+\s+"
                                rf"{name}\s*;", line) or re.match(
                                rf"\s*#\s*(pragma\s+aux|define)\s+{name}\b",
                                line) or re.match(r"\s*#\s*define\b", line):
                    continue
                # The statement: back to the end of the one before, on to
                # its own.
                first = number
                while first > 0 and number - first < 5 and not re.search(
                        r"[;{}]\s*$", lines[first - 1]):
                    first -= 1
                last = number
                while last < len(lines) - 1 and last - number < 5 and \
                        ";" not in lines[last]:
                    last += 1
                statement = " ".join(part.strip()
                                     for part in lines[first:last + 1])
                # The object: the file's own, or the one that every file
                # may name.
                own = [(module, length) for module, length in by_name[name]
                       if module == path.name]
                other = [(module, length) for module, length in by_name[name]
                         if module.endswith("_t.c")]
                module, length = (own or other or by_name[name])[0]
                read = _form(statement, name, sizes | {name: length})
                how = "by its form"
                if read is None or (listed_as, name) in REVIEWED and \
                        read in (None, TABLE, ZERO):
                    read, how = REVIEWED.get((listed_as, name),
                                             (read, how))
                found.append((path.name, number + 1, name, module, length,
                              read, how))
    return found


# Variables of one tree that the compiler has where it has constants, in a
# code segment, and under a name of the original: tree -> name -> why.
# They are not compared, and what the code reads of them is not looked at.
# There is none: the one that there was, Vect21 of the portable C, where
# INT 21h went before the program took it, went when that tree stopped
# taking the interrupt.
VARIABLES = {
}

# Entries of REVIEWED that one tree has no constant for, since it is
# declared to do without what read it (src/<tree>/DEVIATIONS.md), or
# reaches its tables in another way: tree -> (file, name) -> why. Each
# has to be gone from the tree.
GONE = {
    "portable_c": {
        **dict.fromkeys(
            (("memory.c", "Memo_P"), ("memory.c", "Memo1_P"),
             ("memory.c", "DosMe_S"), ("memory.c", "Command")),
            "of the map of the memory, Alt+F5: the portable C has no "
            "such map, and no file memory.c"),
        ("misc.c", "Title_S"): "AsmSeg took the segment of the tables by "
                               "it: the portable C hands a table on by a "
                               "pointer to it, and has no such segment and "
                               "no AsmSeg",
        # Of resident.c, which is exec.c in this tree: see FILES.
        ("resident.c", "BadFl_S"): "said that the program's file is bad "
                                   "or missing, when the program wanted "
                                   "to load itself again after a command: "
                                   "the portable C stays in the memory "
                                   "whole",
    },
}


def check(source, objects, contents, variables=(), flat=False):
    """What is wrong with how the code in the directory `source` reads
    the constants: a list of lines. `contents` gives the bytes of an
    object by its address and length. `flat` is whether the program
    is a flat one of 32 bits, for which of SYSTEM_FILE is the tree's."""
    wrong = []
    used = set()
    gone = GONE.get(Path(source).name, {})
    files = FILES.get(Path(source).name, {})
    where = {(module, name): address
             for module, name, address, _ in objects}
    for file, line, name, module, length, read, how in places(
            source, objects, variables):
        used.add((files.get(file, file), name))
        if read is None:
            wrong.append(f"{file}:{line}: how much of {name} is read here "
                         f"is not known: see REVIEWED in "
                         f"{Path(__file__).name}")
        elif isinstance(read, int) and read > length:
            wrong.append(f"{module}: {name} is {length} bytes, and {read} "
                         f"of them are read at {file}:{line} ({how}). "
                         f"Write it that long, with what the original has "
                         f"behind it")
        elif read == ZERO and 0 not in contents(where[module, name], length):
            wrong.append(f"{module}: {name} is read as far as a zero byte "
                         f"at {file}:{line}, and has none: write it with "
                         f"what the original has behind it, as far as the "
                         f"zero")
    listed = {(file, name) for file, name in REVIEWED
              if file != SYSTEM_FILE[not flat]}
    if deviations.declared(Path(source).name).no_setup:
        listed = {(file, name) for file, name in listed
                  if file not in SETUP_FILES
                  and (file, name) not in SETUP_PLACES}
    for file, name in sorted(listed - used - set(gone)):
        wrong.append(f"{file} and {name} are listed in "
                     f"{Path(__file__).name}, and the file does not name "
                     f"the constant")
    for file, name in sorted(set(gone) & used):
        wrong.append(f"{file} and {name} are listed in "
                     f"{Path(__file__).name} as gone from "
                     f"{Path(source).name}, and the file names the "
                     f"constant")
    for file, name in sorted(set(gone) - set(REVIEWED)):
        wrong.append(f"{file} and {name} are listed in "
                     f"{Path(__file__).name} as gone from "
                     f"{Path(source).name}, and are not in REVIEWED")
    return wrong


def main():
    import datacheck
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__)
    translation = datacheck.translated(Path(sys.argv[1]),
                                       *map(Path, sys.argv[2:]))
    flat = isinstance(translation, datacheck.Flat)
    variables = translation.variables | set(
        VARIABLES.get(translation.source.name, {}))
    for file, line, name, module, length, read, how in places(
            translation.source, translation.objects, variables):
        print(f"{file + ':' + str(line):18} {name:11} {length:4} of "
              f"{module:13} {read}{'' if how == 'by its form' else ': ' + how}")
    for line in check(translation.source, translation.objects,
                      translation.bytes, variables, flat):
        print("WRONG   ", line)


if __name__ == "__main__":
    main()
