#!/usr/bin/env python3
"""The data of the setup program held against the main program's, by the
text of the assembly in src/asm: what a label has of DB, DW, DD, BOXHD
and BOXMN behind it, in three classes: the main program has an object of
that name with the same text, with another text, or none. The sizes are
rough: a number that the assembly works out counts as one.

What has no value, the variables, is left out.
"""
import re

import procs

OBJECT = re.compile(r"(\w+) (DB|DW|DD|BOXHD|BOXMN|CNTRY)\b(.*)")
MORE = re.compile(r"(DB|DW|DD|BOXHD|BOXMN)\b(.*)")
SKIPPED = re.compile(r"IFDEF|ELSE|ENDIF|ShwData")
SIZES = {"DB": 1, "DW": 2, "DD": 4}


def data(files):
    """The objects of `files`: name -> [file, its lines]."""
    found = {}
    for name in files:
        current = None
        for raw in procs.lines(name):
            line = procs.norm(raw)
            if not line:
                continue
            match = OBJECT.match(line)
            if match and match[1] not in SIZES:
                current = match[1]
                found[current] = [name, [match[2] + match[3]]]
            elif MORE.match(line) and current:
                found[current][1].append(line)
            elif not SKIPPED.match(line):
                current = None
    return found


def size(lines):
    count = 0
    for line in lines:
        kind, _, rest = line.partition(" ")
        if kind == "BOXHD":
            count += 10
        elif kind == "BOXMN":
            count += 11
        elif "DUP" in rest:
            times = re.search(r"(\w+)\s*DUP", rest)[1]
            if times.isdigit():
                count += int(times) * SIZES.get(kind, 1)
        else:
            for part in re.findall(r"'[^']*'|\"[^\"]*\"|[^,]+", rest):
                part = part.strip()
                if part[:1] in ("'", '"'):
                    count += len(part) - 2
                elif part:
                    count += SIZES.get(kind, 1)
    return count


def main():
    main_program, setup = data(procs.MAIN), data(procs.SETUP)
    same, differ, alone = [], [], []
    for name, (_, lines) in setup.items():
        if "?" in " ".join(lines):
            continue
        if name not in main_program:
            alone.append((name, size(lines)))
        elif main_program[name][1] == lines:
            same.append((name, size(lines)))
        else:
            differ.append((name, size(lines)))
    for title, rows in (("the same text as the main program's", same),
                        ("the name of one of the main program's, other "
                         "contents", differ),
                        ("the setup program's alone", alone)):
        print(f"== {title}: {len(rows)} objects, about "
              f"{sum(row[1] for row in rows)} bytes")
        print("   " + " ".join(row[0] for row in rows))


if __name__ == "__main__":
    main()
