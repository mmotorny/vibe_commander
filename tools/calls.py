#!/usr/bin/env python3
"""List what a C tree asks of the system by handing registers to an
interrupt.

The portable C asks the system through the functions of its sys.h, which
say what is asked and not how (see CLAUDE.md, "Moving a file to the
system's functions"), and its dos32.c alone knows a register. The C
translation fills a structure of registers and hands it to an interrupt
from every file, and so did each file of the portable C until it went
over. This counts, by file, the calls of that kind:

    DOS      Intr21, Intr21f, Int21d, Int21e, Int21f, and CALL, which
             copy.c of the C translation has for the first
    screen   Int10
    keys     Int16
    mouse    Int33, MousAt
    other    Int2F, Int28, and DosTime and SelDrv of system.c
    memory   a fixed address of the machine, or the address of the
             display's memory, Screen of the main data

    calls.py                    of src/portable_c, which has none left
    calls.py src/c              of the C translation, which has them all

It goes by the text, outside comments: a call is one of those names in
front of a parenthesis, where the line is not the function's own
definition or its declaration. The file of the system's functions is
left out, which is where they are: dos.c of the C translation, dos32.c
of the portable C. The portable C no longer declares the names of the
first five kinds, so the compiler refuses a call of one there; of the
sixth nothing does, and this is what would show it. tools/figures.py writes the sum,
and in how many files, into the README's table.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

KINDS = (
    ("DOS", r"\b(Intr21f?|Int21[def]|CALL)\s*\("),
    ("screen", r"\bInt10\s*\("),
    ("keys", r"\bInt16\s*\("),
    ("mouse", r"\b(Int33|MousAt)\s*\("),
    ("other", r"\b(Int2F|Int28|DosTime|SelDrv)\s*\("),
    ("memory", r"0x0040[0-9A-Fa-f]{4}UL|0x0000[0-9A-Fa-f]{4}UL|md\.Screen"),
)


def count(tree):
    """The calls that are left in the tree `tree`: a list of (file, counts),
    the counts in the order of KINDS, of the files that have any."""
    rows = []
    for path in sorted(tree.glob("*.c")):
        if path.name in ("dos.c", "dos32.c"):
            continue
        text = re.sub(r"/\*.*?\*/", "", path.read_text(encoding="latin-1"),
                      flags=re.S)
        text = re.sub(r"^\s*#\s*define\s+CALL\b.*$", "", text, flags=re.M)
        # A definition or a declaration is no call.
        text = re.sub(r"^(?:static\s+)?(?:u16|u32|void|struct \w+)\s+\w+\s*"
                      r"\([^;{]*\)\s*;?\s*$", "", text, flags=re.M)
        counts = [len(re.findall(pattern, text)) for _, pattern in KINDS]
        if sum(counts):
            rows.append((path.name, counts))
    return rows


def main():
    tree = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "src/portable_c"
    rows = count(tree)
    print(f"{'':12}" + "".join(f"{name:>8}" for name, _ in KINDS)
          + f"{'all':>8}")
    total = [0] * len(KINDS)
    for name, counts in rows:
        print(f"{name:12}" + "".join(f"{count or '':>8}" for count in counts)
              + f"{sum(counts):>8}")
        total = [had + count for had, count in zip(total, counts)]
    print(f"{'all':12}" + "".join(f"{count:>8}" for count in total)
          + f"{sum(total):>8}")


if __name__ == "__main__":
    main()
