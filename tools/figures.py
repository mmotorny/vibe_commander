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

What is measured, of build/portable_c/VC.EXE and its link map: the length
of the file, the relocations in its header, where its segments of code
end. What is counted, in src/portable_c/, comments left out: inline
assembly, by its pieces and instructions, and the lines of #pragma aux
and pack; and the calls that still hand registers to an interrupt, which
tools/calls.py counts. The words __far, FAR, STACK and CODE are counted
by the host check, harness/hostcheck.py, whose function is called here;
the other things that only Open Watcom reads, by their kinds, are that
test's to count, and its table is in TODO.md. What is worked out:

  - What the program keeps while a command runs: what DOS loads of the
    file, which is the file less its header, and the 256 bytes of the PSP
    in front, made whole paragraphs; and two panels of 256 files, each
    made whole paragraphs, which is what the block of the panels is cut
    down to for a command (struct WCBdef of vc.h).
  - What DOS has for a command on the bench of the tests: BENCH less
    that. BENCH is not worked out but was seen, once, while the Info
    panel of the portable C still said how much memory is free: with
    COMMAND.COM not loaded yet, which is 72,496 more than a command
    gets, and what the program kept then added to that. The portable C
    has no such line any more (src/portable_c/DEVIATIONS.md, 11). The
    original has, and its figure is of the same bench: `python3
    harness/show.py --last Ctrl+l` prints its first screen with the
    Info panel switched on, 485,344 Bytes Free, which is 59,808 more
    than BENCH: the 72,496, less the 12,704 that the original keeps
    while a command runs, and a paragraph. If the bench changes, see
    that again, and BENCH is as much more or less as it is; if the
    program comes to keep something besides its file and its panels,
    that is to be added above.
"""
import argparse
import math
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "harness"))

import calls  # noqa: E402  (of tools/, next to this)
import coverage  # noqa: E402  (of harness/, not the package of that name)
import hostcheck  # noqa: E402
import ported  # noqa: E402

README = ROOT / "README.md"
SOURCE = ROOT / "src" / "portable_c"
BUILD = ROOT / "build"

BEGIN = ("<!-- The figures: written by `python3 tools/figures.py --write`, "
         "not by hand. -->")
END = "<!-- The end of the figures. -->"

# What the bench has for the program, its panels and a command together,
# in bytes: see above.
BENCH = 425_536
PARAGRAPH = 16
PSP = 256


def number(value):
    return f"{value:,}"


def whole(length):
    """`length` made whole paragraphs."""
    return math.ceil(length / PARAGRAPH) * PARAGRAPH


def code(path):
    """The text of a C file without its comments."""
    return re.sub(r"/\*.*?\*/", "", path.read_text(), flags=re.S)


# The functions of dos.c for a program of 32 bits, which is not built
# yet: the figures are of the program that is, so this file is not read
# for them. The test portable_c-host reads it, and its count of the
# words has it.
NOT_BUILT = ("dos32.c",)


def sources():
    """The files of the portable C that the program is built of, as
    name -> text without comments."""
    return {path.name: code(path)
            for path in sorted(SOURCE.glob("*.[ch]"))
            if path.name not in NOT_BUILT}


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


def panel(texts):
    """How long a panel of 256 files is: what LAID_OUT holds struct
    WCBdef to."""
    match = re.search(r"LAID_OUT\(WCBdef, (\w+)\)", texts["vc.h"])
    return int(match[1], 0)


def table():
    """The table, as lines of Markdown."""
    program = (BUILD / "portable_c" / "VC.EXE").read_bytes()
    linked = BUILD / "portable_c" / "VC.MAP"
    original = len((BUILD / "shareware-layout" / "VC.COM").read_bytes())
    translation = len((BUILD / "c" / "VC.EXE").read_bytes())
    texts = sources()

    size = len(program)
    last, pages, relocations, header = struct.unpack_from("<4H", program, 2)
    loaded = (pages - 1) * 512 + (last or 512) - header * PARAGRAPH
    kept = whole(loaded + PSP) + 2 * whole(panel(texts))

    rows = [
        ("`VC.EXE`", f"{number(size)} bytes"),
        (f"as a multiple of the original's {number(original)} bytes",
         f"{size / original:.2f}"),
        (f"more than the C translation's {number(translation)} bytes",
         f"{number(size - translation)} bytes"),
        ("The relocations in its header, which DOS applies",
         number(relocations)),
        ("What it keeps of the memory while a command runs: the program "
         "and its panels", f"{number(kept)} bytes"),
        ("What DOS has for a command then, on the bench of the tests",
         f"{number(BENCH - kept)} bytes"),
    ]
    for name, kind, _, offset, length in coverage.segments(linked):
        if kind == "CCODE" and length:
            end = offset + length
            rows.append((
                f"The code segment `{name}` ends at",
                f"{end:04X}h, {number(ported.CODE_END - end)} bytes below "
                f"the limit, {ported.CODE_END:04X}h"))

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
                 + ", ".join(f"`{name}`" for name in intrinsics)))
    rows.append(("Pairs of `#pragma pack`",
                 str(sum(len(re.findall(r"^#pragma pack\(push", text,
                                        flags=re.M))
                         for text in texts.values()))))
    # The words that say what segment a thing is in, as the host check
    # counts them and prints them in its summary: one count, its own.
    places = hostcheck.segment_words(SOURCE)
    rows.append((
        ", ".join(f"`{word}`" for word in hostcheck.SEGMENT_WORDS)
        + ", as the test `portable_c-host` counts them: each place where "
        "the word stands in the code",
        ", ".join(number(places[word])
                  for word in hostcheck.SEGMENT_WORDS)))
    # What asks the system by handing registers to an interrupt, or
    # reads the machine's memory at a fixed address, and not through
    # sys.h: tools/calls.py counts it, by file. There is to be none.
    left = calls.count(SOURCE)
    total = sum(sum(counts) for _, counts in left)
    rows.append((
        "Calls outside `dos.c` that hand registers to an interrupt, or "
        "name a fixed address of the machine, where everything is to go "
        "through `sys.h`, as `tools/calls.py` counts them",
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
