#!/usr/bin/env python3
"""The figures of the portable C that follow its size and its source, in
the one place that has them: a table of the README.

    figures.py              print the table
    figures.py --write      put it into the README, where its two marks are
    figures.py --check      say whether the README's table is this one

Every step of the work changes the size of the program, and agents that
work side by side each change it: a figure written into a sentence by hand
is stale a commit later, and a conflict at every rebase. So the README has
them once, between two marks, and this writes what stands between. Run it
with --write when a piece of work lands, after the build; everything else
says the thing without the number, or points to the table. A figure that
is history, what a step cost when it was made, stays where it tells that.

No test holds the table: one that is a commit stale is no failure.

What is measured, of build/portable_c/VC.EXE and its link map: what the
harness's reader of such a file, harness/le.py, finds in it, the length
of the file and of the extender's stub in it, its code and its data, the
addresses that the extender sets, the stack; how many words of its
tables are pointers where the original has a word of 16 bits, and how
many of those are such addresses, as the test of its data counts them,
harness/datacheck.py; how much room the layout that it rests on has
left, as the test of that works it out, harness/layoutcheck.py; and the
length of the same program compiled for finding a fault. What is
counted, in src/portable_c/, comments left out: inline assembly, by its
pieces and instructions, and the lines of #pragma aux and pack; and the
calls that still hand registers to an interrupt, which tools/calls.py
counts. The other things that only Open Watcom reads, by their kinds,
are the host check's to count, harness/hostcheck.py, and its table is
in TODO.md.

The portable C was a program of 16 bits until the tests came to judge
it as one of 32, and the table then had what such a program keeps of
DOS's memory while a command runs and what that leaves a command on the
bench, worked out from the length of its file. Neither follows from the
file of a program that an extender loads above the megabyte: what a
command finds of the memory then is what the scenario `command` shows.
"""
import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "harness"))

import calls  # noqa: E402  (of tools/, next to this)
import coverage  # noqa: E402  (of harness/, not the package of that name)
import datacheck  # noqa: E402
import hostcheck  # noqa: E402
import layoutcheck  # noqa: E402
import le  # noqa: E402

README = ROOT / "README.md"
SOURCE = ROOT / "src" / "portable_c"
BUILD = ROOT / "build"

BEGIN = ("<!-- The figures: written by `python3 tools/figures.py --write`, "
         "not by hand. -->")
END = "<!-- The end of the figures. -->"

# Where the memory ends that DOS/32A hands out on the bench of the tests,
# a machine of 16 MB, as was seen there (TODO.md, "X, the extender's
# memory on the bench", row 1): the BIOS's 15,232K above the megabyte.
BENCH_END = 0xFE0000
# The least of the room, LEAST_ROOM of vc.h.
LEAST_ROOM = 8000


def number(value):
    return f"{value:,}"


def code(path):
    """The text of a C file without its comments."""
    return re.sub(r"/\*.*?\*/", "", path.read_text(), flags=re.S)


def sources():
    """The files of the portable C, as name -> text without comments."""
    return {path.name: code(path)
            for path in sorted(SOURCE.glob("*.[ch]"))}


def inline_assembly(texts):
    """The pieces of inline assembly: (file, name, instructions). A piece
    is a #pragma aux with its instructions in quotes, or the _asm of a
    function; a label is no instruction."""
    pieces = []
    for name, text in texts.items():
        for match in re.finditer(
                r"^#pragma aux (\w+) = \\\n((?:[^\n]*\\\n)*[^\n]*\n)", text,
                flags=re.M):
            pieces.append((name, match[1], len(re.findall(r'"[^"\n]*"',
                                                          match[2]))))
        heads = [(match.start(), match[1]) for match in re.finditer(
            r"^\w[^\n;{]*?(\w+)\([^();{]*\)\s*{", text, flags=re.M)]
        for match in re.finditer(r"\b_asm\s*{(.*?)}", text, flags=re.S):
            function = [head for at, head in heads if at < match.start()][-1]
            lines = [line.strip() for line in match[1].splitlines()]
            pieces.append((name, function, sum(
                1 for line in lines if line and not line.endswith(":"))))
    return pieces


def table():
    """The table, as lines of Markdown."""
    directory = BUILD / "portable_c"
    original = len((BUILD / "shareware-layout" / "VC.COM").read_bytes())
    translation = len((BUILD / "c" / "VC.EXE").read_bytes())
    texts = sources()

    check = layoutcheck.Check((directory / "VC.EXE").read_bytes(),
                              directory / "VC.MAP")
    check.all()
    program = check.program
    instructions, data = program.objects
    bottom, top = coverage.flat_stack(program, check.map)
    debug = layoutcheck.Check(
        (directory / "debug" / "VC.EXE").read_bytes(),
        directory / "debug" / "VC.MAP").program
    room = check.room
    # The test of the data, for what it counts: it says nothing here.
    _, tables = datacheck.check(BUILD / "shareware-layout", directory,
                                SOURCE, say=lambda line: None)
    size = len(program.contents)
    # Where the room begins on the bench: behind the two panels, which
    # are behind the objects, as a second copy of the program would be.
    room_begins = program.behind(le.BENCH, le.KEPT)

    rows = [
        ("`VC.EXE`",
         f"{number(size)} bytes, of which the extender's stub is "
         f"{number(program.stub)}"),
        (f"as a multiple of the original's {number(original)} bytes",
         f"{size / original:.2f}"),
        (f"more than the C translation's {number(translation)} bytes",
         f"{number(size - translation)} bytes"),
        ("Its code, and what the file has of its data",
         f"{number(instructions.size)} and {number(data.held)} bytes"),
        ("What it has of the memory above the megabyte when it is "
         "loaded: its code and its data, with what has no value",
         f"{number(instructions.size + data.size)} bytes"),
        ("The addresses in it that the extender sets when it loads it",
         number(len(program.relocations))),
        ("What it has for its room on the bench of the tests, a machine "
         "of 16 MB, where the extender's memory ends at FE0000h: all "
         "that is behind its objects and its two panels, each block "
         "with the extender's 16 bytes in front",
         f"{number(BENCH_END - room_begins)} "
         "bytes"),
        ("The least of that memory with which it gets to its panels: its "
         "objects, its two panels and the least of the room, `LEAST_ROOM`, "
         "8,000 bytes, each block with the extender's 16 bytes, worked out "
         "of the same; with less, the extender or the program says that "
         "there is not enough",
         f"{number(room_begins + LEAST_ROOM - le.BENCH + le.IN_FRONT)} "
         "bytes"),
        ("The words of its boxes, dialogs and tables that are pointers, "
         "four bytes where the original has a word of two; and how many "
         "of them are among those addresses, the others holding a "
         "number: as the test `portable_c-data` holds them",
         f"{number(tables['pointers'])} and "
         f"{number(tables['addresses'])}"),
        ("Its stack", f"{number(top - bottom)} bytes"),
        ("How far its tables may grow, in front of `Title_S` and behind "
         "it, before the offset of one wraps round, as the test "
         "`portable_c-layout` holds them",
         f"{number(room['in front of Title_S'])} and "
         f"{number(room['behind Title_S'])} bytes"),
        ("How much more may stand between `Title_S` and the main data, "
         "and between the main data and the end of `IniSave`, by the "
         "same test",
         f"{number(room['between Title_S and the main data'])} and "
         f"{number(room['behind IniSave'])} bytes"),
        ("The same program compiled for finding a fault, "
         "`build/portable_c/debug/VC.EXE`",
         f"{number(len(debug.contents))} bytes, "
         f"{number(debug.objects[0].size)} of them code"),
    ]

    pieces = inline_assembly(texts)
    by_file = {}
    for name, _, instructions in pieces:
        count, total = by_file.get(name, (0, 0))
        by_file[name] = count + 1, total + instructions
    rows.append((
        "Inline assembly",
        f"{sum(piece[2] for piece in pieces)} instructions in {len(pieces)} "
        "pieces: " + ", ".join(f"{total} in {count} of `{name}`"
                               for name, (count, total) in by_file.items())))
    pragmas = [line for text in texts.values()
               for line in re.findall(r"^#pragma aux [^\n]*", text, flags=re.M)]
    rows.append(("Lines of `#pragma aux`",
                 f"{len(pragmas)}, of which "
                 f"{sum(1 for line in pragmas if line.endswith('aborts'))} "
                 "say that a function does not return"))
    intrinsics = [name.strip() for text in texts.values()
                  for found in re.findall(r"^#pragma intrinsic\(([^)]*)\)",
                                          text, flags=re.M)
                  for name in found.split(",")]
    rows.append(("Intrinsic functions",
                 f"{len(intrinsics)}: "
                 + ", ".join(f"`{name}`" for name in intrinsics)
                 if intrinsics else "none"))
    rows.append(("Pairs of `#pragma pack`",
                 str(sum(len(re.findall(r"^#pragma pack\(push", text,
                                        flags=re.M))
                         for text in texts.values()))))
    # What asks the system by handing registers to an interrupt, or
    # reads the machine's memory at a fixed address, and not through
    # sys.h: tools/calls.py counts it, by file. There is to be none.
    left = calls.count(SOURCE)
    total = sum(sum(counts) for _, counts in left)
    rows.append((
        "Calls outside the file of the system's functions that hand "
        "registers to an interrupt, or name a fixed address of the "
        "machine, where everything is to go through `sys.h`, as "
        "`tools/calls.py` counts them",
        f"{number(total)}, in {len(left)} files" if total else "none"))

    return (["| What | Figure |", "|---|---|"]
            + [f"| {what} | {figure} |" for what, figure in rows])


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--write", action="store_true",
                       help="put the table into the README")
    group.add_argument("--check", action="store_true",
                       help="say whether the README has this table, with "
                            "status 1 if not")
    args = parser.parse_args()

    lines = table()
    if not (args.write or args.check):
        print("\n".join(lines))
        return
    text = README.read_text()
    pattern = re.compile(re.escape(BEGIN) + r"\n(.*?)" + re.escape(END),
                         flags=re.S)
    match = pattern.search(text)
    if not match:
        sys.exit(f"{README.name} has no place for the table: the lines\n"
                 f"{BEGIN}\n{END}")
    was = match[1].strip("\n").split("\n")
    if was == lines and args.check:
        print(f"{README.name} has the figures of this build")
        return
    if args.check:
        for old, new in zip(was, lines):
            if old != new:
                print(f"- {old}\n+ {new}")
        if len(was) != len(lines):
            print(f"{len(was)} lines there, {len(lines)} here")
        sys.exit(1)
    README.write_text(pattern.sub(
        lambda _: BEGIN + "\n\n" + "\n".join(lines) + "\n\n" + END, text,
        count=1))
    print(f"{README.name}: the figures written"
          + (", as they were" if was == lines else ""))


if __name__ == "__main__":
    main()
