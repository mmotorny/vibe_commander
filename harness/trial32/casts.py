#!/usr/bin/env python3
"""The places of the portable C where a pointer is kept in a number or
made of one, by what they are.

harness/hostcheck.py counts those places, with gcc, and says which
macro makes each (`--macros`). This asks gcc the same way and sorts
them by what the number is: a place of the main data, a table by its
offset, a panel by its segment and so on, the classes of the plan of
the 32 bits (TODO.md, "The portable C in 32 bits: the plan"), each with
its files. It goes by the macros that a place is made through and by
the words of its line, and a line that it takes for the wrong thing is
counted wrongly: it is for seeing how much there is of each, and what
is left of a class when work has been done on it.

    python3 harness/trial32/casts.py            the classes
    python3 harness/trial32/casts.py N view.c   the places of a class,
                                                in a file or in all
"""
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "harness"))
import hostcheck                        # noqa: E402

SOURCE = ROOT / "src" / "portable_c"

CLASSES = {
    "T": "a table by its offset in the tables' segment",
    "N": "a place of the main data as a number of 16 bits",
    "P": "a panel by its segment",
    "R": "a far pointer that a routine returns, kept as a near one",
    "M": "the main data's segment as a number",
    "V": "the memory of the viewer, the editor and a copy, by segment",
    "G": "a segment handed on, the main data's or a panel's",
    "D": "of DOS: a far pointer taken apart or made for it",
}


def places():
    """Every place, as a dict: the file, the kind that gcc says, the
    macros that it is made through from the inside out, and the line
    where it is used."""
    found = []
    for path in sorted(SOURCE.glob("*.c")):
        name = path.name
        _, text, _ = hostcheck.own(name, path.read_text(encoding="latin-1"))
        said = subprocess.run(
            ["gcc", *hostcheck.OPTIONS, *hostcheck.OWN, "-I", str(SOURCE),
             "-"], input=f'#line 1 "{name}"\n{text}'.encode("latin-1"),
            capture_output=True, env=dict(os.environ, LC_ALL="C"))
        place = None
        for line in said.stderr.decode("latin-1").splitlines():
            parts = re.match(r"(.+?):(\d+):(?:\d+:)? (warning|error|note): "
                             r"(.*)", line)
            if not parts:
                continue
            where, number, severity, message = parts.groups()
            where = Path(where).name
            if severity != "note":
                kind = next((kind for pattern, kind in hostcheck.SAYS
                             if re.search(pattern, message)), None)
                place = None
                if kind in hostcheck.CASTS:
                    place = dict(file=name, kind=kind, macros=[],
                                 use=(where, int(number)))
                    found.append(place)
            elif place is not None:
                macro = re.match(r"in expansion of macro '(\w+)'", message)
                if macro:
                    place["macros"].append(macro.group(1))
                    place["use"] = (where, int(number))
    lines = {}
    for place in found:
        where, number = place["use"]
        if where not in lines:
            lines[where] = (SOURCE / where).read_text(
                encoding="latin-1").split("\n")
        place["line"] = lines[where][number - 1].strip()
    return found


def sort(place):
    """The class of a place."""
    name, line, macros = place["file"], place["line"], place["macros"]
    if place["kind"] == "function" or name == "dos.c":
        return "D"
    if ({"DOS_AT", "ROUTINE"} & set(macros) or "PspSeg" in line
            or "SetVector" in line or "(void __far *)Stack" in line
            or "(u32)MAIN_SEG << 16" in line):
        return "D"
    if "WAS" in macros:
        return "N"
    if macros[:1] in (["PANEL"], ["WCB_OF"]):
        return "P"
    if macros[:1] == ["TEXT"]:
        return "V"
    if {"OFFSET", "WORD", "ASM"} & set(macros):
        return "T"
    if macros[:1] == ["AT"] or not macros and "<< 16" in line:
        if (re.search(r"AT\((code|assembly|AsmSeg\(\))", line)
                or "FIELD" in macros
                or "WORD_AT" in macros and name == "dialog.c"
                or name in ("stcolor.c", "stmain.c")):
            return "T"
        return "G" if name in ("dialog.c", "bars.c") else "V"
    if {"MAIN_SEG", "MAIN_DATA", "SEGMENT"} & set(macros):
        return "M"
    if "NEAR" in macros or re.search(r"\((const )?u8 \*\)\(u16\)\(u32\)",
                                     line):
        return "R"
    return "T" if name == "misc.c" else "N"


def main(arguments):
    found = places()
    if arguments:
        for place in found:
            if sort(place) == arguments[0] and (
                    len(arguments) < 2 or place["file"] == arguments[1]):
                where, number = place["use"]
                print(f"{where}:{number}: {place['kind']} "
                      f"[{'/'.join(place['macros'])}] {place['line']}")
        return
    files = defaultdict(Counter)
    for place in found:
        files[sort(place)][place["file"]] += 1
    for letter, counts in sorted(files.items(),
                                 key=lambda item: -sum(item[1].values())):
        print(f"{letter} {sum(counts.values()):4} in {len(counts):2} files"
              f"  {CLASSES[letter]}")
        print("      " + ", ".join(f"{name} {count}" for name, count
                                   in counts.most_common()))
    print(f"  {len(found):4} in all")


if __name__ == "__main__":
    main(sys.argv[1:])
