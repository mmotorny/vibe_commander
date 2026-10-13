#!/usr/bin/env python3
"""Link the portable C whole as a flat program of 32 bits, to see how
long it is: nothing that it makes runs.

    python3 harness/trial32/whole.py

It copies src/portable_c/ to build/trial32/whole/, and changes there
what Open Watcom's 32-bit compiler refuses of the tree as it stands:

  - `LAID_OUT(BOXHD, 10)` and `LAID_OUT(BOXMN, 11)` of tables.h are
    made 12 and 13: the header and a field of a dialog each hold one
    pointer, which has four bytes;
  - arith.c is left out, the two helpers of the 16-bit compiler;
  - dos.c is left out, and in its place every function of sys.h is
    written as one that does nothing, with the three that vc.h declares
    of DOS alone, and `__GETDS`, which an interrupt function of the
    compiler's asks for.

Every other file compiles as it is, with warnings, which the build
takes for errors, in resident.c alone. Then it links them as
build.sh links a toy, with DOS/32A for the stub, and prints what the
file has: its length, its objects, and its relocations. The project
has to be built once, for the compiler.

What it shows of the tree as it was on 13 October 2026 is in TODO.md,
"The portable C in 32 bits: the plan". That the file links says nothing
of whether the program would run: a place of the main data is still
kept in 16 bits there, and a panel is a segment's number.
"""
import re
import shutil
import struct
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SOURCE = ROOT / "src" / "portable_c"
WATCOM = ROOT / "build" / "_deps" / "open_watcom-src"
BIN = WATCOM / "binl64"
OUT = ROOT / "build" / "trial32" / "whole"


def stub():
    """A file with every function of sys.h, each doing nothing."""
    text = re.sub(r"/\*.*?\*/", "", (SOURCE / "sys.h").read_text(), flags=re.S)
    lines = ['#include "vc.h"',
             "static void Clear(void *at, u32 size)",
             "{ u8 *to = at; while (size--) *to++ = 0; }"]
    for found in re.finditer(r"^((struct \w+|u16|u32|void)\s+\**\w+"
                             r"\([^;{]*\));", text, re.M):
        function = " ".join(found.group(1).split())
        answer = found.group(2)
        if answer == "void":
            lines.append(f"{function} {{ }}")
        elif answer.startswith("struct"):
            lines.append(f"{function} {{ {answer} none; "
                         "Clear(&none, sizeof none); return none; }")
        else:
            lines.append(f"{function} {{ return 0; }}")
    lines += ["u32 Vector(u16 number) { return number; }",
              "void SetVector(u16 number, u32 to) { }",
              "void IntOn(void) { }",
              "void GetDs(void) { }",
              '#pragma aux GetDs "__GETDS"']
    return "\n".join(lines) + "\n"


def facts(file):
    """What an LE file has: its objects, and its relocations by their
    length in the file."""
    whole = file.read_bytes()
    le = whole[struct.unpack_from("<I", whole, 0x3C)[0]:]

    def at(offset):
        return struct.unpack_from("<I", le, offset)[0]

    objects = [struct.unpack_from("<6I", le, at(0x40) + 24 * number)[:2]
               for number in range(at(0x44))]
    pages = at(0x14)
    spans = struct.unpack_from(f"<{pages + 1}I", le, at(0x68))
    place, end = at(0x6C), at(0x6C) + spans[-1] - spans[0]
    lengths = Counter()
    while place < end:
        source, flags = le[place], le[place + 1]
        length = 4 + (2 if flags & 0x40 else 1)
        if source & 0x0F != 2:          # not a selector alone
            length += 4 if flags & 0x10 else 2
        lengths[length] += 1
        place += length
    return len(whole), objects, lengths


def main():
    if OUT.exists():
        shutil.rmtree(OUT)
    OUT.mkdir(parents=True)
    for path in sorted(SOURCE.glob("*.[ch]")):
        if path.name not in ("arith.c", "dos.c"):
            shutil.copy(path, OUT / path.name)
    tables = (OUT / "tables.h").read_text()
    tables = tables.replace("LAID_OUT(BOXHD, 10);", "LAID_OUT(BOXHD, 12);")
    tables = tables.replace("LAID_OUT(BOXMN, 11);", "LAID_OUT(BOXMN, 13);")
    (OUT / "tables.h").write_text(tables)
    (OUT / "nosystem.c").write_text(stub())
    environment = {"WATCOM": str(WATCOM), "PATH": "/usr/bin:/bin"}
    objects = []
    for path in sorted(OUT.glob("*.c")):
        made = subprocess.run(
            [BIN / "wcc386", "-q", "-3r", "-mf", "-os", "-s", "-zl", "-zls",
             f"-i={WATCOM / 'h'}", f"-fo={path.with_suffix('.obj')}", path],
            cwd=OUT, env=environment, capture_output=True, text=True)
        if made.returncode:
            sys.exit(f"{path.name}:\n{made.stdout}")
        if "Warning" in made.stdout + made.stderr:
            print(f"{path.name}: "
                  f"{(made.stdout + made.stderr).count('Warning')} warnings")
        objects += ["file", path.with_suffix(".obj")]
    subprocess.run(
        [BIN / "wlink", "option", "quiet", "format", "os2", "le",
         "name", OUT / "VC.EXE", *objects,
         "option", "start=Alloc0_",
         "option", f"stub={WATCOM / 'binw' / 'dos32a.exe'}",
         "option", f"map={OUT / 'VC.MAP'},static",
         "order", "clname", "CODE", "clname", "MAIN", "clname", "DATA",
         "clname", "BSS", "clname", "STACK"],
        cwd=OUT, env=environment, check=True)
    length, objects, lengths = facts(OUT / "VC.EXE")
    print(f"{(OUT / 'VC.EXE').relative_to(ROOT)}: {length:,} bytes")
    for number, (size, base) in enumerate(objects, 1):
        print(f"  object {number}: {size:,} bytes, linked at {base:08X}h")
    count = sum(lengths.values())
    total = sum(length * many for length, many in lengths.items())
    print(f"  {count:,} relocations, {total:,} bytes of the file: "
          + ", ".join(f"{many:,} of {length} bytes"
                      for length, many in sorted(lengths.items())))
    for line in (OUT / "VC.MAP").read_text().splitlines():
        if re.match(r"\w+ +\w+ +(DGROUP|AUTO) +000[12]:", line):
            print("  " + " ".join(line.split()))


if __name__ == "__main__":
    main()
