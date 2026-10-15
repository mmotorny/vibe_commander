#!/usr/bin/env python3
"""A cut to 16 bits that the compiler loses, kept out of the portable C.

The fault. Open Watcom's code generator extends a number with a sign
that is converted to a wider type without a sign, and from there to a
still wider type, with its sign both times, where C says that the
second time fills with zeros. Its cause is in TODO.md, "The compiler's
lost cut: what Open Watcom does, and where": the code generator folds
two conversions that follow one another into one, from the first type
to the third, and that is wrong exactly when the first type has a sign,
the second is wider and has none, and the third is wider still or is a
floating type. It takes three sizes, so a program of 32 bits that has
no number of 64 and no floating one meets it in one shape: a byte with
a sign that is converted to a number of 16 bits without a sign and from
there goes on into 32 bits, as a number or added to a pointer. For

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
char)x`, `u16 line = x` for an `x` that is a signed char, a field of
bits among them, and the same by way of something that changes no
number, `(u16)(int)(signed char)x`, `(u16)(u32)(signed char)x`, a
choice of two such, and arithmetic that the compiler folds away, `+ 0`,
`| 0`, `* 1`, `-(-x)`, `x + 1 - 1`. With every option of the optimiser,
-os, -ot, -ox and none at all; -od alone,
which the debugging build has, is right, and so is clang. The compiler
for C++ of the same download has it, and the one for 16 bits, which the
C translation is built by, has it for a byte made a word that goes on
into a long, `cbw` and `cwd`: no listing of that tree has a `cwd`.

What is right. A conversion to a number of 16 bits with a sign between
the two,

    u16 line = (u16)(short)(signed char)byte;

which is `cbw` and `movzx eax,ax`. That is the one form that is right
by how the fold is written: a change of sign at the same size is no
instruction, so the two conversions do not follow one another. The
tree was written in it on 33 lines and is in neither form since: a
byte that the original takes with its sign is a small number there,
taken as it is, or with its sign into an int where that is meant, as
src/portable_c/vc.h says at "A byte with a sign made a word". A mask,
`& 0xFFFF`, and arithmetic with a number that is written, `(u16)((signed
char)x - 1)`, come out right today, by the order of the compiler's
passes and by nothing else: the tree does not write them, and the
finder refuses them.

Right too, and what the tree leans on: a conversion that narrows is
never the first of a fold, and a fold whose second conversion narrows
is right. Of the 392 chains of three among the eight types of whole
numbers, 384 are right in every form that was tried. The eight that
are wrong: a signed char by way of a u16 to 32 or 64 bits, with a sign
or without; a signed char by way of a u32 to 64 bits; a short by way
of a u32 to 64 bits.

Why it matters. Step 2 of the road makes numbers of 16 bits into numbers
of 32, and each takes a cut away or moves one. Where a number was kept
in 16 bits, the wrong form was right by that; widened, it is wrong.

The test, `portable_c-cut` of the build, does two things.

It reads the tree, src/portable_c/, through clang, as harness/int16.py
has it read, and fails for every place where a signed char is converted
to a number of 16 bits without a sign. The rule is closed, and no list
of what changes nothing: the byte is looked for through every
conversion but one to a short, through every operator of one operand,
and through every operator of two of which one is a number that is
written. Only a `short` between, and arithmetic with something that is
no constant, which is done in 32 bits and cut from there, end the
search. It fails for all such places, also where the number is only
used in 16 bits today: the next widening would make such a place wrong
without a line of it changing. It fails as well for a number of 64
bits and for a floating type, declared or made by a cast: the tree has
neither, the fault is there with either as the third type, and the
finder does not look for those chains.

And it compiles harness/cutcheck.c with the compiler of the build and
the options that the build compiles a file of the tree with, which the
build writes into build/portable_c/compiled.txt, and holds every
function of it to the instructions that are written in front of it
there. It fails before that if those options have -oe or -ox, which
expand a function in place: the fault is then also across a call and
across a return, where a function that takes or returns 16 bits cuts
for itself today. The functions called Lost are the fault, held to the
wrong instructions: a compiler that no longer has it fails the test, and this
check and the note of vc.h are then looked at again. Those called Kept
are held to the right ones: the form with a short between, which the
tree no longer writes and which is kept as what the compiler does of
it, and every other cast that narrows, which the tree rests on, of a number of 32
bits with a sign or without to a u16 and to a u8, of a number of 16 bits
to a u8, of a short to a u16, and a byte made 32 bits with its sign,
each handed to a parameter of 32 bits, added to a number of 32 bits,
compared with one and made an index. The tree is to say every narrowing
by such a cast, so it leans on the compiler keeping each: a compiler
that loses one fails the test. A function that is added gets its
instructions from what the test then says the compiler makes of it,
read before they are written down. The two called Chance are the
forms that are right by the order of the passes: held to the right
instructions, so that the day they change is seen, and nothing of the
tree is wrong by it.

The finder is held against the same file first: it has to find the
functions called Lost or Chance and no other.

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

# The conversions through which the finder looks for the signed char:
# to these types, each as wide as 16 bits or wider. A short is not among
# them: it alone breaks the chain. A conversion to a byte without a sign
# is none either: behind it the number has no sign.
THROUGH = {"int", "unsigned int", "long", "unsigned long", "long long",
           "unsigned long long", "unsigned short"}
CASTS = {"CStyleCastExpr", "ImplicitCastExpr"}

# What is a third size besides a number of 32 bits, declared or made by
# a cast: a number of 64 bits, where the fault is one size up, of a
# number with a sign made 32 bits without one, and a floating type,
# which a number of 16 bits without a sign made of a signed char is
# converted to with its sign as well. The tree has neither, and the
# finder does not look for those chains: it finds the types.
WIDE = {"long long", "__int64", "float", "double", "long double"}
DECLARED = {"VarDecl", "ParmVarDecl", "FieldDecl", "FunctionDecl"}

# What a place is found for, as the line of it says.
LOST = "a signed char made a number of 16 bits without a sign"
WIDER = "a number of 64 bits or a floating one"

# The options of the optimiser that expand a function in place, letters
# behind -o: with one of them the fault is also across a call and across
# a return, where the number is handed to a parameter of 16 bits or
# returned as 16 bits, which this takes to be right.
EXPANDING = "ex"


def type_of(node):
    type_ = node.get("type", {})
    return type_.get("desugaredQualType") or type_.get("qualType", "")


def inner(node):
    return [each for each in node.get("inner", []) if isinstance(each, dict)]


def constant(node):
    """Whether `node` is a number that is written: a literal, the length
    of something, a name of an enumeration, and what is made of such."""
    kind, parts = node.get("kind"), inner(node)
    if kind in ("IntegerLiteral", "CharacterLiteral",
                "UnaryExprOrTypeTraitExpr", "OffsetOfExpr"):
        return True
    if kind == "DeclRefExpr":
        return node.get("referencedDecl", {}).get("kind") \
            == "EnumConstantDecl"
    if kind in {"ParenExpr", "ConstantExpr", "UnaryOperator",
                "BinaryOperator", "ConditionalOperator"} | CASTS:
        return bool(parts) and all(constant(each) for each in parts)
    return False


def signed_bytes(node):
    """Whether what `node` works out may be a signed char that the
    compiler takes to be unchanged. The rule is closed, and not a list of
    what changes nothing: the compiler folds `+ 0`, `| 0`, `* 1`, `- 0`,
    `-(-c)`, `c + 1 - 1` and their like away, and other arithmetic with
    a number that is written is right by the order of its passes and by
    nothing else. So the chain is looked for through every conversion to
    a type of 16 bits or more but a short, through every operator of one
    operand, through every operator of two of which one is a number that
    is written, and into both answers of a choice. What breaks it is a
    conversion to a short, and arithmetic with something that is no
    constant, which is done in 32 bits and cut from there."""
    while True:
        kind, parts = node.get("kind"), inner(node)
        if kind in ("ParenExpr", "ConstantExpr") and parts:
            node = parts[0]
        elif (kind in CASTS and parts
              and node.get("castKind") in ("IntegralCast", "NoOp")
              and type_of(node) in THROUGH):
            node = parts[-1]
        elif (kind == "UnaryOperator" and parts
              and node.get("opcode") in ("+", "-", "~")):
            node = parts[0]
        elif (kind == "BinaryOperator" and node.get("opcode") == ","
              and len(parts) == 2):
            node = parts[1]
        elif (kind == "BinaryOperator" and len(parts) == 2
              and node.get("opcode") in ("+", "-", "*", "/", "%", "|",
                                         "&", "^", "<<", ">>")
              and (constant(parts[0]) or constant(parts[1]))):
            if constant(parts[0]) and constant(parts[1]):
                return False
            node = parts[1] if constant(parts[0]) else parts[0]
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
              and type_of(node).replace("unsigned ", "")
              .replace("_Complex ", "") in WIDE):
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
            r"/\* is: ([^\n]*?)\s*\*/\n[\w *]+?\b(\w+)\([^)]*\)\n\{.*?\n\}",
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
        has = name.startswith(("Lost", "Chance"))
        if has and not within:
            wrong.append(f"{CHECK.name}: {name} has the form and is not "
                         f"found")
        if not has and within:
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
    expanding = [part for part in command
                 if re.match(r"^[-/]o[a-z+]", part)
                 and set(re.match(r"^[-/]o([a-z+]*)", part).group(1))
                 & set(EXPANDING)]
    if expanding:
        return [f"the build compiles with {' '.join(expanding)}, which "
                f"expands a function in place. The fault is then also "
                f"where a signed char made a number of 16 bits is handed "
                f"to a parameter of 16 bits or returned as 16 bits and "
                f"goes on into 32 bits in the function that it is "
                f"expanded in, which neither the form of the tree nor "
                f"this test's finder is made for: pin it down again with "
                f"that option before the build has it."]
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
    if any(line.startswith("Chance") for line in wrong):
        wrong.append("A function called Chance is right by the order of "
                     "the compiler's passes alone. The tree has no such "
                     "form, and the finder refuses it: nothing is wrong by "
                     "this, and what the file says of the form is to be "
                     "written anew.")
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
    names = int16.dos_files(args.source)
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
              f"its sign where the number goes on into 32 bits. Take the "
              f"byte as it is, or with its sign into an int: see "
              f"src/portable_c/vc.h, \"A byte with a sign made a word\".")
    if len(found) > lost:
        print(f"cut: {len(found) - lost} places of the tree have a number "
              f"of 64 bits or a floating one. The compiler has the same "
              f"fault with either as the last of three types, for a number "
              f"with a sign made a wider one without and then that, "
              f"LostWide and LostFloating of harness/cutcheck.c, and this "
              f"does not look for it: teach it to before such a number "
              f"goes in.")
    if found:
        return 1
    print(f"cut: no place of the {len(names)} files of the tree has the "
          f"form that the compiler loses.")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
