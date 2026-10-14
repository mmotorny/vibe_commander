#!/usr/bin/env python3
"""A cut to 16 bits that the compiler loses, kept out of the portable C.

The fault. Open Watcom's code generator extends a number with a sign
that is converted to a wider type without a sign, and from there to a
still wider type, with its sign both times, where C says that the
second time fills with zeros. It takes three sizes, so a program of 32
bits that has no number of 64 meets it in one shape: a byte with a sign
that is converted to a number of 16 bits without a sign and from there
goes on into 32 bits. For

    u16 line = (u16)(signed char)byte;

it hands `line` on as `movsx eax,al`, FFFFFFFEh for the byte FEh, where C
says 0000FFFEh. It does so wherever the number is then used in 32 bits:
returned or handed over as 32 bits, kept in a variable of 32 bits, added
to, compared with or multiplied by something, shifted, divided, also
where the result is cut again behind a division or a shift to the right.
It does not where the number is only used in 16 bits: kept in a field of
16 bits, handed to a parameter of 16 bits, which the function cuts for
itself, or compared with a number of 16 bits.

Which forms. The conversion itself, written or not: `(u16)(signed
char)x`, `u16 line = x` for an `x` that is a signed char, and the same
by way of something that changes no number, `(u16)(int)(signed char)x`,
`(u16)(u32)(signed char)x`, a `+` in front, a choice of two such. With
every option of the optimiser, -os, -ot, -ox and none at all; -od alone,
which the debugging build has, is right, and so is clang. The compiler
for C++ of the same download has it, and the one for 16 bits, which the
C translation is built by, has it for a byte made a word that goes on
into a long, `cbw` and `cwd`: no listing of that tree has a `cwd`.

What is right. A conversion to a number of 16 bits with a sign between
the two,

    u16 line = (u16)(short)(signed char)byte;

which is `cbw` and `movzx eax,ax`. That is the form that the tree is
written in, and src/portable_c/vc.h says so at "A byte with a sign made
a word". Right as well, and not used: a mask, `& 0xFFFF`, and arithmetic
on the byte before the cut, `(u16)((signed char)x - 1)`, which is no
conversion of the byte.

Why it matters. Step 2 of the road makes numbers of 16 bits into numbers
of 32, and each takes a cut away or moves one. Where a number was kept
in 16 bits, the wrong form was right by that; widened, it is wrong.

The test, `portable_c-cut` of the build, does two things.

It reads the tree, src/portable_c/, through clang, as harness/int16.py
has it read, and fails for every place where a signed char is converted
to a number of 16 bits without a sign, with nothing between that changes
the number and no `short`. It fails for all of them, also where the
number is only used in 16 bits today: the next widening would make such
a place wrong without a line of it changing. It fails as well for a
number of 64 bits, declared or made by a cast: the tree has none, the
fault is there one size up, for a number with a sign made 32 bits
without one and then 64, and the finder does not look for that.

And it compiles harness/cutcheck.c with the compiler of the build and
the options that the build compiles a file of the tree with, which the
build writes into build/portable_c/compiled.txt, and holds every
function of it to the instructions that are written in front of it
there. The functions called Lost are the fault, held to the wrong
instructions: a compiler that no longer has it fails the test, and this
check, the form and the note of vc.h are then looked at again. Those
called Kept are what the tree rests on, held to the right ones: the
form of the tree, and every other cast that narrows, of a number of 32
bits with a sign or without to a u16 and to a u8, of a number of 16 bits
to a u8, of a short to a u16, and a byte made 32 bits with its sign,
each handed to a parameter of 32 bits, added to a number of 32 bits,
compared with one and made an index. The tree is to say every narrowing
by such a cast, so it leans on the compiler keeping each: a compiler
that loses one fails the test. A function that is added gets its
instructions from what the test then says the compiler makes of it,
read before they are written down.

The finder is held against the same file first: it has to find the
functions called Lost and no other.

    python3 harness/cut.py --compiled build/portable_c/compiled.txt \\
        --out build/cutcheck          the test
    python3 harness/cut.py --show     the places of the tree, and no more
    python3 harness/cut.py --clang NAME ...   another clang than `clang`

Without `--compiled` the compiler is not asked. Without clang the tree is
not read: the test says so and counts as skipped if the compiler's half
has passed, and fails if not.
"""

import argparse
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hostcheck                                            # noqa: E402
import int16                                                # noqa: E402

SOURCE = int16.SOURCE
CHECK = Path(__file__).resolve().parent / "cutcheck.c"
SKIPPED = 77

# The conversions that change no number that a signed char can hold,
# through which the finder looks: to these types. A short is not among
# them: with one between, the compiler is right.
THROUGH = {"int", "unsigned int", "long", "unsigned long", "long long",
           "unsigned long long", "unsigned short"}
CASTS = {"CStyleCastExpr", "ImplicitCastExpr"}

# A number of 64 bits, declared or made by a cast: the tree has none, and
# the fault is there one size up, which the finder does not look for.
WIDE = {"long long", "__int64"}
DECLARED = {"VarDecl", "ParmVarDecl", "FieldDecl", "FunctionDecl"}

# What a place is found for, as the line of it says.
LOST = "a signed char made a number of 16 bits without a sign"
WIDER = "a number of 64 bits"


def type_of(node):
    type_ = node.get("type", {})
    return type_.get("desugaredQualType") or type_.get("qualType", "")


def inner(node):
    return [each for each in node.get("inner", []) if isinstance(each, dict)]


def signed_bytes(node):
    """Whether what `node` works out is a signed char that nothing has
    changed: itself, or behind what changes no number."""
    while True:
        kind, parts = node.get("kind"), inner(node)
        if kind == "ParenExpr" and parts:
            node = parts[0]
        elif (kind in CASTS and parts
              and node.get("castKind") in ("IntegralCast", "NoOp")
              and type_of(node) in THROUGH):
            node = parts[-1]
        elif (kind == "UnaryOperator" and node.get("opcode") == "+"
              and parts):
            node = parts[0]
        elif (kind == "BinaryOperator" and node.get("opcode") == ","
              and len(parts) == 2):
            node = parts[1]
        elif kind == "ConditionalOperator" and len(parts) == 3:
            return signed_bytes(parts[1]) or signed_bytes(parts[2])
        else:
            return type_of(node) == "signed char"


def places(ast, name):
    """Where a signed char is made a number of 16 bits without a sign,
    and where a number of 64 bits is: (file, line, which of the two) of
    each, the file being `name` or a header's."""
    found = set()

    def walk(node):
        parts = inner(node)
        if (node.get("kind") in CASTS and parts
                and node.get("castKind") == "IntegralCast"
                and type_of(node) == "unsigned short"
                and signed_bytes(parts[-1])):
            file, line = node["_at"][0][:2]
            file = os.path.basename(file)
            found.add((file if file.endswith(".h") else name, line, LOST))
        elif (node.get("kind") in DECLARED | CASTS
              and not node.get("isImplicit")
              and type_of(node).replace("unsigned ", "") in WIDE):
            file, line = node["_at"][0][:2]
            file = os.path.basename(file)
            found.add((file if file.endswith(".h") else name, line, WIDER))
        for each in parts:
            walk(each)

    walk(ast)
    return found


def read(command, source, name):
    """The places of one file, through clang."""
    text = (source / name).read_text(encoding="latin-1")
    _, text, _ = hostcheck.own(name, text)
    result = subprocess.run(
        [command, *int16.OPTIONS, int16.OFFSETOF, *hostcheck.OWN, "-I",
         str(source), "-"],
        input=text.encode("latin-1"), capture_output=True)
    refused = [line for line in result.stderr.decode("latin-1").splitlines()
               if re.search(r": (fatal )?error: ", line)]
    if refused or result.returncode:
        sys.exit(f"cut: clang does not take {name}:\n"
                 + "\n".join(refused[:10]))
    try:
        ast = json.loads(result.stdout)
    except ValueError:
        sys.exit(f"cut: clang's account of {name} cannot be read")
    int16.Lines().mark(ast)
    return places(ast, name)


def functions(text):
    """The functions of the check's file: name, the instructions held,
    and its first and last line."""
    found = []
    for each in re.finditer(
            r"/\* is: ([^\n]*?)\s*\*/\n[\w ]+?\b(\w+)\([^)]*\)\n\{.*?\n\}",
            text, re.S):
        first = text.count("\n", 0, each.start()) + 1
        last = text.count("\n", 0, each.end()) + 1
        found.append((each.group(2), " ".join(each.group(1).split()),
                      first, last))
    return found


def finder_wrong(command):
    """Hold the finder against the check's file: what is wrong, as
    lines to print."""
    found = read(command, CHECK.parent, CHECK.name)
    wrong = []
    for name, _, first, last in functions(CHECK.read_text()):
        within = sorted(line for file, line, _ in found
                        if file == CHECK.name and first <= line <= last)
        if name.startswith("Lost") and not within:
            wrong.append(f"{CHECK.name}: {name} has the form and is not "
                         f"found")
        if not name.startswith("Lost") and within:
            wrong.append(f"{CHECK.name}:{within[0]}: found in {name}, "
                         f"which has not the form")
    return wrong


def runs(listing):
    """Of a listing of wdis, for every function: its instructions from
    the first to the first `ret` or jump, through whatever labels lie
    between, with the number of a label left out."""
    lines = []                          # ("label", name) or ("code", text)
    for line in listing.split("\n"):
        label = re.match(r"^[0-9A-F]{4,8}\t+([\w$]+):\s*$", line)
        code = re.match(r"^[0-9A-F]{4,8}  (?:[0-9A-F]{2} ?)+\t+(\S.*)$", line)
        if label:
            lines.append(("label", label.group(1)))
        elif code:
            text = " ".join(code.group(1).split())
            lines.append(("code", re.sub(r"L\$\d+", "L", text)))
    found = {}
    for at, (kind, name) in enumerate(lines):
        if kind != "label" or name.startswith("L$"):
            continue
        run = []
        for kind, text in lines[at + 1:]:
            if kind != "code":
                continue
            run.append(text)
            if text.split()[0] in ("ret", "jmp"):
                break
        found[name.rstrip("_")] = "; ".join(run)
    return found


def compiler_wrong(compiled, out):
    """Compile the check's file as the build compiles a file of the
    tree and hold its functions to what is written of them: what is
    wrong, as lines to print."""
    command = [part for part in
               compiled.read_text().strip().split(";") if part]
    tool = next((part for part in command
                 if os.path.basename(part) == "wcc386"), None)
    if tool is None:
        return [f"{compiled} names no wcc386"]
    out.mkdir(parents=True, exist_ok=True)
    # Compiled where the object goes: for a file with a warning the
    # compiler writes NAME.err into the directory that it is run in.
    for name in ("cutcheck.obj", "cutcheck.lst", "cutcheck.err"):
        (out / name).unlink(missing_ok=True)
    done = subprocess.run(command + ["-fo=cutcheck.obj", str(CHECK)],
                          cwd=out, capture_output=True, text=True)
    if done.returncode or not (out / "cutcheck.obj").exists():
        return [f"the compiler does not take {CHECK.name}:",
                *(done.stdout + done.stderr).strip().split("\n")]
    wdis = str(Path(tool).parent / "wdis")
    done = subprocess.run(command[:command.index(tool)]
                          + [wdis, "-l=cutcheck.lst", "cutcheck.obj"],
                          cwd=out, capture_output=True, text=True)
    if done.returncode or not (out / "cutcheck.lst").exists():
        return [f"wdis does not list {CHECK.name}:",
                *(done.stdout + done.stderr).strip().split("\n")]
    made = runs((out / "cutcheck.lst").read_text(errors="replace"))
    held = functions(CHECK.read_text())
    wrong = []
    if len(held) < 10:
        wrong.append(f"{CHECK.name} has {len(held)} functions with their "
                     f"instructions, which is too few to be all of them")
    for name, wanted, _, _ in held:
        got = made.get(name)
        if got == wanted:
            continue
        wrong.append(f"{name}: the compiler makes")
        wrong.append(f"    {got or 'nothing'}")
        wrong.append(f"  of it, where {CHECK.name} has")
        wrong.append(f"    {wanted}")
    if any(line.startswith("Lost") for line in wrong):
        wrong.append("A function called Lost is the fault. If the compiler "
                     "has it right now, the fault is mended or has moved: "
                     "read the top of harness/cut.py, and pin it down "
                     "again before the form of the tree is changed.")
    if any(line.startswith("Kept") for line in wrong):
        wrong.append("A function called Kept is a cut that the tree rests "
                     "on. Read its instructions: if a cut to 16 or 8 bits "
                     "is gone from them, the program is wrong wherever "
                     "the tree has that form.")
    return wrong


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--compiled", type=Path,
                        help="the build's file of the command that compiles "
                             "a file of the tree")
    parser.add_argument("--out", type=Path,
                        help="a directory for what the compiler makes")
    parser.add_argument("--clang", default="clang")
    parser.add_argument("--source", type=Path, default=SOURCE)
    parser.add_argument("--show", action="store_true",
                        help="print the places of the tree and end")
    args = parser.parse_args()

    failed = False
    if args.compiled and not args.show:
        if not args.out:
            parser.error("--compiled wants --out")
        wrong = compiler_wrong(args.compiled, args.out)
        if wrong:
            failed = True
            print("cut: the compiler is not as harness/cutcheck.c has it.")
            print("\n".join("  " + line for line in wrong))
        else:
            print(f"cut: the compiler makes of the "
                  f"{len(functions(CHECK.read_text()))} functions of "
                  f"{CHECK.name} what is written there: the fault is "
                  f"where it was, and the form of the tree is kept.")

    command, said = int16.compiler(args.clang)
    if command is None:
        print(f"cut: the tree was not read: {said}.")
        return 1 if failed else SKIPPED
    wrong = finder_wrong(command)
    if wrong:
        print("cut: the finder does not find what it is to find.")
        print("\n".join("  " + line for line in wrong))
        return 1
    names = sorted(path.name for path in args.source.glob("*.c"))
    with ThreadPoolExecutor() as pool:
        found = set().union(*pool.map(
            lambda name: read(command, args.source, name), names))
    for file, line, what in sorted(found):
        text = (args.source / file).read_text(encoding="latin-1")
        text = " ".join(text.split("\n")[line - 1].split())
        print(f"{file}:{line}: {what}: {text}")
    lost = sum(1 for place in found if place[2] == LOST)
    if lost:
        print(f"cut: {lost} places of the tree make a signed char a number "
              f"of 16 bits without a sign, which the compiler extends with "
              f"its sign where the number goes on into 32 bits. Write "
              f"(u16)(short)(signed char): see src/portable_c/vc.h, \"A "
              f"byte with a sign made a word\".")
    if len(found) > lost:
        print(f"cut: {len(found) - lost} places of the tree have a number "
              f"of 64 bits. The compiler has the same fault one size up, "
              f"for a number with a sign made 32 bits without one and then "
              f"64, LostWide of harness/cutcheck.c, and this does not look "
              f"for it: teach it to before such a number goes in.")
    if found:
        return 1
    print(f"cut: no place of the {len(names)} files of the tree has the "
          f"form that the compiler loses.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
