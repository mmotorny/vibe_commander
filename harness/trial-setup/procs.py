#!/usr/bin/env python3
"""The routines of the setup program held against those of the main
program, by the text of the assembly in src/asm.

    procs.py                the routines of VCSETUP.ASM, VCSTHLP.INC and
                            VCSTSUB.INC in three classes: the main
                            program has one of that name with the same
                            text, with another text, or none
    procs.py Dialog Help    and how each of these differs, as a diff from
                            the main program's to the setup program's

A routine is what stands between PROC and ENDP; comments and how a line
is spaced do not count. The code that stands between procedures, most of
VCSETUP.ASM, is not looked at here: classes.py counts it.
"""
import difflib
import re
import sys
from pathlib import Path

SOURCE = Path(__file__).resolve().parent.parent.parent / "src" / "asm"
MAIN = ["VC.ASM", "VCSUB1.INC", "VCSUB2.INC", "VCSUB3.INC", "VCFUNC.INC",
        "VCF1.INC", "VCMENU.INC", "VCCOPY.INC", "VCEDIT.INC", "VCVIEW.INC",
        "VCFIND.INC", "VCHELP.INC", "VCMEM.INC"]
SETUP = ["VCSETUP.ASM", "VCSTHLP.INC", "VCSTSUB.INC"]


def lines(name):
    """The lines of a file of the assembly, which is CP866 with CRLF and
    an end-of-file character."""
    text = (SOURCE / name).read_bytes().decode("cp866")
    return text.replace("\r", "").replace("\x1a", "").split("\n")


def norm(line):
    """A line without its comment, with one space wherever it has
    space."""
    if "'" not in line:
        line = line.split(";")[0]
    else:
        line = re.sub(r";[^']*$", "", line)
    return re.sub(r"\s+", " ", line).strip()


def procs(files):
    """The procedures of `files`: name -> (file, its lines)."""
    found = {}
    for name in files:
        current = None
        for raw in lines(name):
            line = norm(raw)
            if not line:
                continue
            match = re.match(r"(\w+) PROC\b", line)
            if match:
                current = match[1]
                found[current] = (name, [])
            elif current and re.match(rf"{current} ENDP", line):
                current = None
            elif current:
                found[current][1].append(line)
    return found


def is_code(line):
    return not re.match(r"(\w+:?\s+)?(DB|DW|DD|EQU|=)\b", line) \
        and not line.startswith(".") and not re.fullmatch(r"\w+:", line)


def main():
    main_program, setup = procs(MAIN), procs(SETUP)
    classes = {"same": [], "differs": [], "absent": []}
    for name, (file, text) in setup.items():
        count = sum(1 for line in text if is_code(line))
        if name not in main_program:
            classes["absent"].append((name, file, count))
        elif main_program[name][1] == text:
            classes["same"].append((name, file, count,
                                    main_program[name][0]))
        else:
            diff = difflib.unified_diff(main_program[name][1], text,
                                        lineterm="", n=0)
            changed = sum(1 for line in diff if line[:1] in "+-"
                          and line[:3] not in ("+++", "---"))
            classes["differs"].append((name, file, count,
                                       main_program[name][0],
                                       f"{changed} lines"))
    for title, rows in classes.items():
        print(f"== {title}: {len(rows)} routines, "
              f"{sum(row[2] for row in rows)} lines of code")
        for row in rows:
            print("  ", *row)
    for name in sys.argv[1:]:
        print(f"\n##### {name}: {main_program[name][0]} -> "
              f"{setup[name][0]}")
        print("\n".join(difflib.unified_diff(
            main_program[name][1], setup[name][1], lineterm="", n=2)))


if __name__ == "__main__":
    main()
