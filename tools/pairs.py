#!/usr/bin/env python3
"""List the functions of a C tree that return a number of 32 bits.

Where the assembly handed back two registers, or a register and a flag,
the C translation returns one number with a thing in each half, and the
portable C is to return what the two mean (see TODO.md, "The portable C:
two things in one number"). This lists every function that is defined
`u32`, by the file that defines it, with how often its own file calls
it and how often each other file does: what has to change with it.

    pairs.py                    of src/portable_c
    pairs.py src/c              of another tree

It goes by the text: a function is what is defined at the start of a
line as `u32 Name(` or `static u32 Name(`, and a call is its name in
front of a parenthesis anywhere else, outside comments.
"""
import collections
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def main():
    tree = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "src/portable_c"
    texts = {}
    for path in sorted(tree.glob("*.[ch]")):
        texts[path.name] = re.sub(
            r"/\*.*?\*/", lambda match: re.sub(r"[^\n]", " ", match[0]),
            path.read_text(), flags=re.S)
    defined = {}
    for name, text in texts.items():
        for match in re.finditer(r"^(static )?u32 (\w+)\([^)]*\)\s*\n\{",
                                 text, flags=re.M):
            defined[match[2]] = (name, bool(match[1]))
    file = None
    for function, (name, static) in sorted(
            defined.items(), key=lambda found: (found[1][0], found[0])):
        calls = collections.Counter()
        for other, text in texts.items():
            if other.endswith(".h"):
                continue
            count = len(re.findall(rf"\b{function}\s*\(", text)) - len(
                re.findall(rf"^(?:static )?u32 {function}\s*\(", text,
                           flags=re.M))
            if count > 0:
                calls[other] = count
        if name != file:
            print(name)
            file = name
        others = ", ".join(f"{other} {count}"
                           for other, count in sorted(calls.items())
                           if other != name)
        print(f"    {function:10} {'static' if static else '      '} "
              f"its own file {calls.get(name, 0):2}"
              + (f"   others: {others}" if others else ""))
    print(f"{len(defined)} functions")


if __name__ == "__main__":
    main()
