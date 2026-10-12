#!/usr/bin/env python3
"""How much the C code reads of each of the program's constants.

The test of the port's data (datacheck.py) compares as many bytes as an
object has. The program reads more of some: it copies a name as 13 bytes
whatever its length, a title as 80, and writes a line with a table of
colours of which the line may ask for the fourth. What lies behind such
an object in the original is read too, so the port has to have the object
that long, with those bytes; then the comparison holds them.

This lists every place where the code names a constant, a text or a table
of the files of constants or of the code's own, and says
how many bytes of it are read there. Most places it knows by their form:
a loop to a count, MvzLine, MvsBw and MainFil with their lengths, a text
that is read as far as its zero byte, a box or a dialog that is handed
on by its offset. The others are in REVIEWED below, each with what
reading the code found. A place that is neither is reported, and so is
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

    reads.py PORT_DIR           every place, with the bytes read there
"""
import re
import sys
from collections import defaultdict
from pathlib import Path

PORT = Path(__file__).resolve().parent.parent / "port"

ZERO = "as far as its zero"     # a text
TABLE = "as it is laid out"     # a box, a dialog, buttons: by their kind
OWN = "within itself"           # by an index that its own data bounds
NONE = "not read"               # only its address or its size is taken

# What a call reads of a constant that is handed to it by its offset.
READERS = ("Dialog", "DialBox", "OncZoom", "MinMnu", "MnMn10", "MnMn40",
           "Ask")

# Places that are not known by their form: (file, name) -> (what is read,
# how that is known). A number is a count of bytes from the start of the
# object. It holds for every place in the file that names the constant
# and is not known otherwise.
REVIEWED = {
    ("bars.c", "KeyB_T"): (OWN, "the index is held below its size"),
    ("bars.c", "KeyC_T"): (OWN, "a row of KeyB_T and one of ten keys"),
    ("bars.c", "Key_S"): (OWN, "a name of KeyC_T, six bytes each"),
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
    ("dirfunc.c", "DlWr4_S"): (81, "F48_00 copies a path and a name, "
                                   "LenPath + 13 bytes"),
    ("dirfunc.c", "Mask_S"): (6, "five bytes from the first or the second "
                                 "on: DirFunc"),
    ("dirfunc.c", "Tree_F"): (14, "copied as 14 bytes"),
    ("dirfunc.c", "MkDi1_B"): (TABLE, "a dialog, for F47_01"),
    ("dirfunc.c", "MkDi2_B"): (TABLE, "a dialog, for F47_01"),
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
    ("start.c", "NcEnv"): (OWN, "compared at its size"),
    ("start.c", "TempEnv"): (OWN, "compared at its size"),
    ("start.c", "Comsp"): (OWN, "compared at its size"),
    ("start.c", "Init_F"): (13, "ReadOne copies a name as 13 bytes"),
    ("start.c", "Ext_F"): (13, "ReadOne copies a name as 13 bytes"),
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
}

_NUMBER = {"LenPath": 68, "LenStr": 64}


def _count(text, name, sizes):
    text = text.strip()
    if text.isdigit():
        return int(text)
    if text in _NUMBER:
        return _NUMBER[text]
    match = re.fullmatch(r"sizeof\s*\(?\s*(\w+)\s*\)?", text)
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
    rest = re.sub(rf"[!=]=\s*OFFSET\(\s*{n}\s*\)", "", rest)
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
    if re.search(rf"\b({'|'.join(READERS)})\s*\((?:[^();]|\([^();]*\))*"
                 rf"OFFSET\(\s*&?{n}\s*\)", statement) or re.search(
                     rf"\b(box|report|result)\s*=\s*OFFSET\(\s*{n}\s*\)",
                     statement):
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


def places(objects, variables=()):
    """Every place where a file of code names one of `objects`, a list
    of (file, name, address, length) as datacheck.Port has them:
    (file, line, name, the file of the object, its length, what is read
    there, how that is known). `variables` are names among them that are
    no constants."""
    by_name = defaultdict(list)
    for module, name, _, length in objects:
        if name not in variables:
            by_name[name].append((module, length))
    names = sorted(by_name, key=len, reverse=True)
    pattern = re.compile(r"\b(" + "|".join(map(re.escape, names)) + r")\b")
    found = []
    for path in sorted(PORT.glob("*.c")):
        if path.name.endswith("_t.c"):
            continue
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
                # file: Command of main.c, and that of resident.c.
                if line[match.end():].lstrip().startswith("("):
                    continue
                if re.match(rf"\s*(extern|static|const)\b[^=;(]*\b{name}\b"
                            rf"\s*\[", line) or re.match(
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
                if read is None or (path.name, name) in REVIEWED and \
                        read in (None, TABLE, ZERO):
                    read, how = REVIEWED.get((path.name, name),
                                             (read, how))
                found.append((path.name, number + 1, name, module, length,
                              read, how))
    return found


def check(objects, contents, variables=()):
    """What is wrong with how the code reads the constants: a list of
    lines. `contents` gives the bytes of an object by its address and
    length."""
    wrong = []
    used = set()
    where = {(module, name): address
             for module, name, address, _ in objects}
    for file, line, name, module, length, read, how in places(objects,
                                                              variables):
        used.add((file, name))
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
    for file, name in sorted(set(REVIEWED) - used):
        wrong.append(f"{file} and {name} are listed in "
                     f"{Path(__file__).name}, and the file does not name "
                     f"the constant")
    return wrong


def main():
    import datacheck
    port = datacheck.Port(Path(sys.argv[1]))
    for file, line, name, module, length, read, how in places(
            port.objects, port.variables):
        print(f"{file + ':' + str(line):18} {name:11} {length:4} of "
              f"{module:13} {read}{'' if how == 'by its form' else ': ' + how}")
    for line in check(port.objects, port.bytes, port.variables):
        print("WRONG   ", line)


if __name__ == "__main__":
    main()
