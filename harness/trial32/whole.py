#!/usr/bin/env python3
"""Link the portable C whole as a flat program of 32 bits, to see how
long it is: nothing that it makes runs.

    python3 harness/trial32/whole.py

The project's build compiles the tree for 32 bits, into
build/portable_c32/, and links no program of it yet. This links one, of
those objects and of what the build does not make: a file that the
build lists as not ready for 32 bits, and tries, of which it leaves
<file>.refused there and no object. Such a file is compiled here with
its warnings, which the build takes for errors. There is none since the
piece S3 of the plan, and nothing of DOS is left in the tree outside
sys.h, which this had as functions that do nothing: every name that
the objects ask for is one of dos32.c.

It has the build make the objects first, links them as build.sh links a
toy, with DOS/32A for the stub and Start of dos32.c for where the
program begins, and prints what the file has: its
length, its objects, and its relocations. The project has to be
configured, and its tools fetched.

What it showed of the tree as it was on 13 October 2026 is in TODO.md,
"The portable C in 32 bits: the plan", and again there with the words
of the addresses of vc.h in their second bodies. Then it compiled the
tree itself, and had every function of sys.h do nothing; since there
is dos32.c, what it links is that. That the file links says nothing of
whether the program would run: 24 functions of dos32.c are not made,
and nothing has run the second bodies of the words of vc.h.
"""
import re
import struct
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
SOURCE = ROOT / "src" / "portable_c"
WATCOM = ROOT / "build" / "_deps" / "open_watcom-src"
BIN = WATCOM / "binl64"
BUILT = ROOT / "build" / "portable_c32"
OUT = ROOT / "build" / "trial32" / "whole"

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
    made = subprocess.run(["cmake", "--build", ROOT / "build",
                           "--target", "portable_c32"],
                          capture_output=True, text=True)
    if made.returncode:
        sys.exit(made.stdout + made.stderr)
    OUT.mkdir(parents=True, exist_ok=True)
    for old in OUT.iterdir():
        old.unlink()
    # A file that the build has refused has no object there. One that
    # has both was refused once, and is ready since.
    waiting = [SOURCE / f"{refused.stem}.c"
               for refused in sorted(BUILT.glob("*.refused"))
               if not refused.with_suffix(".obj").exists()]
    environment = {"WATCOM": str(WATCOM), "PATH": "/usr/bin:/bin"}
    objects = sorted(BUILT.glob("*.obj"))
    for path in waiting:
        object_file = OUT / f"{path.stem}.obj"
        made = subprocess.run(
            [BIN / "wcc386", "-q", "-3r", "-mf", "-os", "-s", "-zl", "-zls",
             f"-i={SOURCE}", f"-i={WATCOM / 'h'}", "-fr",
             f"-fo={object_file}", path],
            cwd=OUT, env=environment, capture_output=True, text=True)
        if made.returncode:
            sys.exit(f"{path.name}:\n{made.stdout}")
        if "Warning" in made.stdout + made.stderr:
            print(f"{path.name}: "
                  f"{(made.stdout + made.stderr).count('Warning')} warnings")
        objects.append(object_file)
    print(f"{len(objects)} objects, {len(waiting)} of them made here")
    # In the order of the files' names, as the program of 16 bits is
    # linked: a segment holds what the objects have of it in their order.
    objects.sort(key=lambda path: path.name)
    subprocess.run(
        [BIN / "wlink", "option", "quiet", "format", "os2", "le",
         "name", OUT / "VC.EXE",
         *(word for path in objects for word in ("file", path)),
         "option", "start=Start_",
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
