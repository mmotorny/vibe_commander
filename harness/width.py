#!/usr/bin/env python3
"""The numbers without a sign of the portable C, and why each is one.

Step 2 of the road has every number of the portable C, src/portable_c/,
be as wide as what it counts, and signed unless there is a reason for
it not to be: a number of 8 or 16 bits becomes an i32 or a boolean, or
stays as it is with its reason written down, and a u32 becomes an i32
or stays with its reason (TODO.md, "The portable C in a flat memory:
the plan", "The rule for a number"). And where a number then meets one
of another sign or is put into fewer bits, the code says so with a
cast, which the compiler's warnings hold and no list beside the code
(CLAUDE.md, "The portable C"). This finds the numbers, and the test
`portable_c-width` of the build holds the tree to a record of them,
harness/width.txt, so that a number that is added later is sorted too,
and so that a commit which widens shows in the record what it widened;
and it holds every file that is sorted to clang's warnings.

What is found. clang reads every file as harness/int16.py has it read,
and this walks what it understood with the finder of that tool, which
knows of every expression between which two numbers it lies.

A number is something without a sign that the file declares, a u8, a
u16 or a u32, or an array of them: what a function returns, a
parameter, a variable, a field of a structure. The table counts those
of 8 or 16 bits and those that are a u32 apart, since the first are
widened and the second only given a sign. Of a function it is counted
where the
function is written, not where a header declares it. The structures of
the headers are found too, once, under the header's name.

A cut is where a number that may not fit is put into fewer bits with
no cast, by the types alone, a variable being taken to hold any number
of its type as int16.py takes it:

    store     kept in a variable or a field of 8 or 16 bits, `=` and
              `|=`, `<<=` and their like
    step      a variable of 8 or 16 bits stepped, `++`, `--`, `+=`,
              `-=`: all of them, but the variable that a loop counts with
    handed    handed to a parameter of 8 or 16 bits
    returned  returned as 8 or 16 bits

The cuts are printed for whoever asks, `--show`, and are neither
recorded nor counted, and no test holds them: they are what a cluster
reads before it widens a number, since a wider variable keeps another
number exactly there. A parameter that is widened no longer cuts what
it is handed, and a return that is widened is cut where a caller keeps
it in 16 bits. A cut that is written, `(u16)(a + b)`, is none. What of
them a file that is sorted may not have is clang's to say and not this
finder's: see "The casts", below.

And one kind of place is no cut and has an entry in the record with its
reason, since no cast says it and no warning names it:

    sign      a number of 32 bits with a sign, an i32, which may be
              below 0 by its type, is shifted to the right or divided,
              or a remainder is taken of it or by it. That is what it
              was without a sign only for a number that is not below 0,
              and the reason says why it is not

The record. A file, or a function or a structure of one, is in it in
one of four ways.

  - `not sorted`, with how many numbers it has: nobody has read it for
    the rule yet.
  - Sorted: every number that it still has without a sign, of 8, 16
    or 32 bits, and every place of a sign, is an entry with its reason,
    written by whoever read it. A number that was made an i32 or a
    boolean is no entry: the table at the top of the record counts
    those, by file. A cut is no entry either.
  - `laid out`, a structure whose layout is not the program's to
    change, with whose it is, once for all its fields.
  - `as a whole`, a file that no cluster of step 2 reads: WHOLE below
    says why.

The test fails where the tree is not as the record has it: a count that
has moved, fewer as well as more; a number or a place of a sign of
something sorted that the record has not; one of the record that is
gone; an entry without a reason.

The casts. A file that is sorted is held to clang's warnings, WARNINGS
below, each switched on and none switched off for a line: the test
fails for whatever clang says, and for a line of the tree that switches
a warning off, `#pragma clang diagnostic` and its like. A place that is
named gets the type that holds its number, which is tried first, or a
cast, with a comment where the program counts on what the cast cuts
off; which cast the compiler keeps is the matter of harness/cut.py.
What clang names is what is held, and nothing beyond it, on the user's
word of 13 October 2026: no rule of the project's own on top of the
compiler's. What the warnings are:

  -Wconversion, all of it. Its parts that can speak of this tree:
      -Wimplicit-int-conversion    a number put into fewer bits, by
                                   `=`, at a call, by `return`, in a
                                   declaration; with it
                                   -Wimplicit-int-conversion-on-negation,
                                   `byte = -byte`, and
                                   -Wshorten-64-to-32, which has nothing
                                   to say while the tree is read for a
                                   machine of 32 bits and has no number
                                   of 64 (harness/cut.py fails for one)
      -Wsign-conversion            a number made one of the other sign
                                   with no cast, an i32 handed to a
                                   function that takes a u32
      -Wconstant-conversion,       a constant that does not fit where
      -Wbitfield-constant-         it is put
       conversion
      -Wenum-conversion            a value of one enumeration taken for
                                   one of another
      -Wbool-conversion,           a pointer or a constant taken for
      -Wnull-conversion,           true or false or for no pointer;
      -Wint-conversion             a number for a pointer
    The others are of floating types, of C++ or of Objective-C, and say
    nothing of C without a floating number; all of the group is held so
    that nothing has to be said of a part that is left out.
  -Wsign-compare                   a number with a sign held against one
                                   of 32 bits without, where C makes
                                   the first a huge number if it is
                                   below 0
  -Wtautological-constant-         of the booleans: a comparison that a
   out-of-range-compare,           type decides, and a number where
  -Wint-in-bool-context            true or false is asked

What clang does not name, with any warning that it has. Its warning of
a conversion takes arithmetic on numbers of 8 or 16 bits to stay within
them, on purpose, so `word = word + 1` and `byte = a - b` are silent
where `word = number` is not; and it says nothing of a step, `++`,
`--`, `+=`, `|=`. Such a place, on a variable that stays 8 or 16 bits
for a reason, stays as it is written: it wraps in its own width, which
is what C says and what a reader expects, and it gets no cast for the
finder's sake. The finder's cuts have them, to be read when a cluster
decides what is widened, which is how most of them go: a step of a
count that becomes an i32 is no cut. They are three quarters of the
cuts of the tree as it was before the clusters, 902 of 1,074. gcc's
-Wconversion names `word += number` and not `++`, nor a step by a
number that is written, nor `word = word + 1`.

Where clang and Open Watcom read a line otherwise. clang reads the tree
for a machine of 32 bits with a char that has no sign, as wcc386 has
it: an int, a long and a pointer are 32 bits for both, and the
promotions are C's for both, so a line converts the same numbers. Two
things differ. An enumeration is an int for clang and as few bytes as
its values need for wcc386, one for three values: a number kept in a
variable of an enumeration would be cut by wcc386 and named by nobody,
and a variable of one put into a u8 would be named by clang where
wcc386 converts nothing. The tree has no variable, parameter or field
of an enumeration, only their constants, which are an int for both. And
what wcc386 does wrongly with a cast that is written, a byte with a
sign made a u16, clang knows nothing of: harness/cut.py holds that.
When the tree is read for a machine of 64 bits, step 6, a difference of
two pointers and a size are 64 bits, and -Wshorten-64-to-32 will name
each that is kept in an i32.

    python3 harness/width.py                 the table, and the test
    python3 harness/width.py --show FILE     the numbers and the cuts of
                             FILE:SCOPE      a file or of one function
                             NAME            or structure of it, or what
                                             cuts what it hands to a
                                             function, in every file
    python3 harness/width.py --sort FILE     make entries of a file, or
                             FILE:SCOPE      of one function of it, that
                                             the record has as not sorted
    python3 harness/width.py --record        the record anew

`--record` keeps every reason whose entry is still there, takes out
what is gone, counts anew what is not sorted, and writes what is new in
something sorted without a reason: that is then written under the
entry, in harness/width.txt, by whoever has read the place. A
structure is made `laid out` by hand there: the line `FILE, struct
NAME: laid out, 1 numbers` with the reason under it, in the place of
its entries or of its line `not sorted`. `--warned FILE` prints what
clang says of a file under the warnings, sorted or not: what a cluster
has to mend before the file is sorted.

What a reason is. For a number that is as wide as it is, or without a
sign, by what it is, a character, a colour, a cell of the screen, a row
and a column, bits, the size of a file, it is
one word of a fixed list, WORDS below, which has what each word says,
once; what is not plain of the one number goes behind a colon. The
table counts those in a column of their own, `by word`, and a single
word that is not of the list fails the test. The code has no type for
such a thing: a name for what a number merely means is not made, since
C holds it apart from nothing (CLAUDE.md, "The portable C"), so the
word is in the record and nowhere else. For any other number that
stays it is a sentence. For a place of a sign it is always a sentence:
what at the place or just above it keeps the number from being below
0, as harness/int16.txt has it.

What it cannot see is what int16.py cannot see, and this besides: a
number that is only ever 0 or 1 is not named as one. Whether a number
is a boolean is read, by every value that is put into it and every use.

It is held against lines that were written for it, harness/widthcheck.c,
every time it runs, as int16.py is against its own.

Exits with status 0 if the tree is as the record has it, 1 if not, and
77 if there is no clang to ask, which the build takes for a test that
was skipped.
"""
import argparse
import difflib
import json
import re
import textwrap
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hostcheck                        # noqa: E402
import int16                            # noqa: E402

ROOT = int16.ROOT
SOURCE = int16.SOURCE
RECORD = Path(__file__).resolve().parent / "width.txt"
CHECK = Path(__file__).resolve().parent / "widthcheck.c"
SKIPPED = 77

# Where a number is put into fewer bits with no cast, by its kind; and
# the one kind of place that has an entry in the record, with its reason.
CUTS = ["store", "step", "handed", "returned"]
SIGN = "sign"
HEADERS = ("vc.h", "sys.h", "tables.h")

# The clusters of step 2, as TODO.md has them, each with its files. A
# file of two or three clusters is in the row of them all.
CLUSTERS = [
    ("N0", "digits.c mask.c line.c mouse.c blank.c"),
    ("N1", "string.c format.c box.c screen.c misc.c"),
    ("N2", "dialog.c"),
    ("N3, N4", "view.c"),
    ("N5", "find.c"),
    ("N6 to N8", "edit.c"),
    ("N9", "copy.c"),
    ("N10", "dirfunc.c"),
    ("N11", "files.c read.c place.c"),
    ("N12", "panel.c info.c drive.c"),
    ("N13", "tree.c treebox.c extfile.c"),
    ("N14", "cmdline.c enter.c exec.c"),
    ("N15", "input.c usermenu.c"),
    ("N16", "main.c"),
    ("N17a", "keys.c"),
    ("N17b", "commands.c"),
    ("N18", "menu.c help.c"),
    ("N19a", "bars.c window.c system.c setup.c"),
    ("N19b", "start.c"),
    ("N20", "stcolor.c stopts.c"),
    ("N21", "stmain.c sthelp.c"),
]

# Files that no cluster reads, each as a whole, with why. A file of
# tables, NAME_t.c, is one of them by its name.
TABLES = ("the tables: constants that are laid out as the original has "
          "them, which the test of the data holds byte for byte")
WHOLE = {
    "sys_dos.c": "the system's file: what DOS, the BIOS and the extender "
               "take and answer in, and the layer's to shape, step 3",
    "stack_dos.c": "the stack, an array of bytes",
}

# What clang is asked of every file that is sorted: the top of this file
# says what each names. -Wconversion has -Wsign-conversion,
# -Wimplicit-int-conversion and -Wconstant-conversion in it. Nothing is
# taken out of the group and nothing is switched off behind it.
WARNINGS = ["-Wconversion", "-Wsign-compare",
            "-Wtautological-constant-out-of-range-compare",
            "-Wint-in-bool-context"]

# The types that a number is widened to, by the name that vc.h has for
# each, and the column of the table that counts it. A u32 is none of
# them: it is a number of the record, as one of 8 or 16 bits is.
WIDE = {"bool": "bool", "_Bool": "bool", "i32": "i32", "int": "i32"}

# A number of 32 bits without a sign, as it is written.
UNSIGNED = ("u32", "unsigned int", "unsigned long", "unsigned")

# The words for a number that has its width by what it is, each with
# what it says, once: such a number has the word for its reason, alone
# or with what is not plain behind a colon, `place` or `system: the
# keys that are held`. The code has no name for any of them: a number
# has the type of its width there, and what it is, its name and a
# comment say (CLAUDE.md, "The portable C"). The second part is for a
# field of the main data or of a panel, whose layout is the original's.
WORDS = {
    "character": "a character: a byte as a text has it, or in 16 bits as "
                 "the original has it in a register, the character being "
                 "the low byte",
    "colour": "the number of one of the program's colours, or the "
              "attribute of a cell that Color makes of it: a byte, in 16 "
              "bits where the original has it in a register",
    "cell": "a cell of the screen as the display's memory has it, the "
            "character in the low byte and its colour in the high one, or "
            "an array of them; or the colour alone in its place",
    "place": "a row and a column of the screen in two bytes, the row in "
             "the high one, as the BIOS has the place of the cursor; or as "
             "many rows and columns, the size of a window",
    "key": "a key as the BIOS answers it, the character in the low byte "
           "and the scan code in the high one",
    "time": "a time or a date as DOS has it, in the bytes or the bits of "
            "its answer",
    "handle": "the handle of a file, as DOS answers it",
    "attributes": "the attributes of a file, in DOS's bits",
    "random": "one of the program's random numbers, or of the two words "
              "that they are made of: 16 bits, which wrap round",
    "system": "something else that the system answers in or takes, which "
              "is said behind the word",
    "bits": "bits and no number: a mask, flags, or what is only taken "
            "apart and put together by shifts and masks",
    "size": "the size of a file, a place in one, the size of a disk or of "
            "what is free on it, or a sum of such: it can pass two "
            "gigabytes, and there is no signed type of 64 bits. A u32 "
            "until the layer's design gives the size of a file its type",
    "setup": "a byte of the setup file, which stays a byte",
    "flag": "true or false, in a byte of the layout: the code asks "
            "whether it is 0",
    "text": "characters, or the bytes of a file or of a buffer",
    "address": "a segment or a place of DOS's memory, which a piece of the "
               "addresses takes out of the structure as a pointer",
    "small": "a number that what it counts keeps within its width, which "
             "is said behind the word",
    "unused": "of the original, and neither set nor read here",
}


def word(reason):
    """The word of WORDS that a reason is, or begins with; None for a
    sentence."""
    first = reason.split(":")[0].strip()
    return first if first in WORDS else None


def narrow(type_):
    """Of what clang says the type of a thing is: the type as it is
    written, if it is a number of 8 or 16 bits, or one of 32 without a
    sign, or an array of them."""
    def bare(name):
        name = " ".join(re.sub(r"\b(const|volatile)\b", "", name).split())
        return re.sub(r"\s*(\[\d*\])+$", "", name), name

    written = bare(type_.get("qualType", ""))
    real = bare(type_.get("desugaredQualType", "") or
                type_.get("qualType", ""))
    for element, _ in (written, real):
        if element in ("u8", "u16", "unsigned char", "unsigned short",
                       "char", "signed char", "short") + UNSIGNED:
            return written[1].replace(" [", "[")
    return None


def unsigned(written):
    """Is a type, as narrow has it, of 32 bits without a sign?"""
    return re.sub(r"(\[\d*\])+$", "", written) in UNSIGNED


def wide(type_):
    name = " ".join(re.sub(r"\b(const|volatile)\b", "",
                           type_.get("qualType", "")).split())
    return WIDE.get(re.sub(r"\s*(\[\d*\])+$", "", name))


class Widths(int16.Finder):
    """The numbers and the cuts of one file."""

    def __init__(self, name, text, sizes):
        super().__init__(name, text, sizes)
        self.numbers = {}       # (file, scope, what, name, type): lines
        self.wider = {}         # a column of the table: how many
        self.cuts = {}          # (line, kind, scope): what is called
        self.signs = set()      # (line, scope)
        self.quiet = set()      # the steps of the loops that count
        self.first = {}         # (file, scope): its first line

    # The numbers.

    def where(self, node):
        """The file that a thing is written in, and its line; the file
        is None for what is neither the file's own nor a header's."""
        used = node["_at"][0]
        if used[0] == "<stdin>":
            return self.name, used[1]
        name = Path(used[0]).name
        return (name if name in HEADERS else None), used[1]

    def number(self, node, scope, what):
        file, line = self.where(node)
        if file is None:
            return
        type_ = node.get("type", {})
        if what == "returns":
            type_ = {"qualType": type_.get("qualType", "").split("(")[0]}
        found = narrow(type_)
        name = "" if what == "returns" else node.get("name", "")
        if what == "field" and name == "holds":
            return                      # HOLDS of vc.h: no number
        if found:
            if file in HEADERS and what != "field":
                return
            self.first.setdefault((file, scope), line)
            self.numbers.setdefault(
                (file, scope, what, name, found), []).append(line)
        elif file == self.name and wide(type_):
            column = wide(type_)
            self.wider[column] = self.wider.get(column, 0) + 1

    def declared(self, node, scope=""):
        kind = node.get("kind")
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
        if kind == "FunctionDecl":
            if not any(x.get("kind") == "CompoundStmt" for x in inner):
                return                  # declared, and written elsewhere
            scope = node.get("name", "")
            self.number(node, scope, "returns")
        elif kind == "ParmVarDecl":
            self.number(node, scope, "parameter")
            return
        elif kind == "VarDecl":
            self.number(node, scope, "variable")
        elif kind == "FieldDecl":
            self.number(node, scope, "field")
            return
        elif kind == "RecordDecl":
            if not node.get("completeDefinition"):
                return
            if node.get("name"):
                scope = f"{node.get('tagUsed', 'struct')} {node['name']}"
            elif not scope.startswith(("struct ", "union ")):
                return                  # one without a name, in a function
        for child in inner:
            self.declared(child, scope)

    # The cuts.

    def cut(self, node, kind):
        used, begins, ends, end_used = node["_at"]
        if (self.in_word(begins) and self.in_word(ends)
                and used[:3] == end_used[:3]):
            return
        if used[0] != "<stdin>":
            return
        if kind == SIGN:
            self.signs.add((used[1], self.function))
        else:
            self.cuts.setdefault((used[1], kind, self.function),
                                 node.get("_callee", ""))

    def convert(self, value, types, node):
        if (types[1][1] in (8, 16) and not int16.within(
                value.low, value.high, types[1])
                and node.get("kind") != "CStyleCastExpr"):
            kind = "step" if node.get("opcode") in ("+=", "-=") else "store"
            self.cut(node, node.get("_cut") or kind)
        return super().convert(value, types, node)

    def arithmetic(self, op, a, b, node):
        """What a sign changes: a shift to the right, a division and a
        remainder of a number of 32 bits with a sign, which are those
        of a number without one only while it is not below 0."""
        if op in (">>", "/", "%"):
            for value in (a,) if op == ">>" else (a, b):
                if (value.t16 == value.t32 == int16.S32 and value.low < 0):
                    self.cut(node, SIGN)
        return super().arithmetic(op, a, b, node)

    def value(self, node):
        kind = node.get("kind")
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]

        def unwrapped(node):
            while node.get("kind") in ("ParenExpr", "ImplicitCastExpr"):
                node = node["inner"][0]
            return node

        def cast(child):
            return (child.get("kind") == "ImplicitCastExpr"
                    and child.get("castKind") == "IntegralCast")

        if kind == "CallExpr" and inner:
            called = unwrapped(inner[0]).get("referencedDecl", {})
            for child in inner[1:]:
                if cast(child):
                    child["_cut"] = "handed"
                    child["_callee"] = called.get("name", "")
        elif kind == "ReturnStmt":
            for child in inner:
                if cast(child):
                    child["_cut"] = "returned"
        elif kind == "ForStmt":
            parts = [child if isinstance(child, dict) and child.get("kind")
                     else None for child in node.get("inner", [])]
            counts = self.counts(parts) if len(parts) == 5 else None
            if counts:
                step = unwrapped(parts[3])
                types = int16.type_of(step["inner"][0].get("type", {}))
                if int16.within(counts[1], counts[2] + 1, types[1]):
                    self.quiet.add(id(step))
        elif (kind == "UnaryOperator" and node.get("opcode") in ("++", "--")
              and id(node) not in self.quiet and inner):
            types = int16.type_of(inner[0].get("type", {}))
            if types not in (None, int16.POINTER) and types[1][1] in (8, 16):
                self.cut(node, "step")
        return super().value(node)


def read(command, source, name, text=None):
    """One file through clang: its finder, when it has walked it."""
    if text is None:
        text = (source / name).read_text(encoding="latin-1")
    _, text, _ = hostcheck.own(name, text)
    result = subprocess.run(
        [command, *int16.OPTIONS, int16.OFFSETOF, *hostcheck.OWN, "-I",
         str(source), "-"],
        input=text.encode("latin-1"), capture_output=True)
    refused = [line for line in result.stderr.decode("latin-1").splitlines()
               if re.search(r": (fatal )?error: ", line)]
    if refused or result.returncode:
        sys.exit(f"width: clang does not take {name}:\n"
                 + "\n".join(refused[:10]))
    try:
        ast = json.loads(result.stdout)
    except ValueError:
        sys.exit(f"width: clang's account of {name} cannot be read")
    int16.Lines().mark(ast)
    finder = Widths(name, text, int16.lengths(command, source, text, ast))
    finder.declared(ast)
    finder.value(ast)
    return finder


def warned(command, source, name, whole=False):
    """What clang says of a file under WARNINGS, as lines: those that
    name a place, or all that it prints of each."""
    text = (source / name).read_text(encoding="latin-1")
    _, text, _ = hostcheck.own(name, text)
    options = [option for option in int16.OPTIONS
               if option not in ("-w", "-Xclang", "-ast-dump=json")]
    result = subprocess.run(
        [command, *options, int16.OFFSETOF, *hostcheck.OWN, "-I",
         str(source), "-Wno-everything", *WARNINGS, "-"],
        input=text.encode("latin-1"), capture_output=True)
    return [line.replace("<stdin>", name)
            for line in result.stderr.decode("latin-1").splitlines()
            if whole or ": warning: " in line]


def switched_off(source):
    """The lines of the tree that switch a warning off for clang or gcc,
    or have a header taken for the system's: as lines to print."""
    found = []
    for path in sorted(source.glob("*.[ch]")):
        for number, line in enumerate(
                path.read_text(encoding="latin-1").split("\n"), 1):
            if re.match(r"\s*#\s*pragma\s+(clang|GCC)\s+"
                        r"(diagnostic|system_header)", line):
                found.append(f"{path.name}:{number}: {line.strip()}")
    return found


class Tree:
    """What was found in every file."""

    def __init__(self, command, source):
        self.names = sorted(path.name for path in source.glob("*.c"))
        with ThreadPoolExecutor() as pool:
            finders = list(pool.map(
                lambda name: read(command, source, name), self.names))
        self.numbers = {}       # (file, scope, what, name, type): lines
        self.cuts = {}          # (file, scope, kind, text): lines
        self.signs = {}         # (file, scope, "sign", text): lines
        self.called = {}        # (file, line): what is called there
        self.wider = {}         # file: {column: how many}
        self.first = {}         # (file, scope): its first line
        for name, finder in zip(self.names, finders):
            lines = (source / name).read_text(encoding="latin-1").split("\n")
            self.numbers.update(finder.numbers)     # a header's: by all
            self.wider[name] = finder.wider
            for key, line in finder.first.items():
                self.first[key] = min(line, self.first.get(key, line))
            for (line, kind, scope), callee in sorted(finder.cuts.items()):
                text = " ".join(lines[line - 1].split())
                self.cuts.setdefault((name, scope, kind, text),
                                     []).append(line)
                self.first.setdefault((name, scope), line)
                if callee:
                    self.called[(name, line)] = callee
            for line, scope in sorted(finder.signs):
                text = " ".join(lines[line - 1].split())
                self.signs.setdefault((name, scope, SIGN, text),
                                      []).append(line)
                self.first.setdefault((name, scope), line)
        self.files = [name for name in HEADERS
                      if any(key[0] == name for key in self.numbers)]
        self.files += self.names

    def scopes(self, file, cuts=False):
        """The scopes of a file that have a number or a place of a sign,
        which is what the record has; with those that have only a cut,
        for whoever asks to see the cuts."""
        tables = (self.numbers, self.signs) + ((self.cuts,) if cuts else ())
        found = {key[:2] for table in tables
                 for key in table if key[0] == file}
        return sorted(found, key=lambda key: (self.first.get(key, 0), key))

    def count(self, file, scope=None):
        """(numbers of 8 or 16 bits, numbers that are a u32, places of
        a sign) of a file, or of one scope of it. The cuts are not
        counted: they are printed for whoever asks, and held by
        nothing."""
        def of(table, wanted=None):
            return sum(len(lines) for key, lines in table.items()
                       if key[0] == file and scope in (None, key[1])
                       and (wanted is None or wanted == unsigned(key[4])))
        return (of(self.numbers, False), of(self.numbers, True),
                of(self.signs))


def whole(file):
    if file in WHOLE:
        return WHOLE[file]
    return TABLES if file.endswith("_t.c") else None


class Record:
    """The record as it is written: what is not sorted, what is laid
    out, and the reason of every entry."""

    def __init__(self, text=None):
        self.files, self.scopes = set(), set()      # not sorted
        self.laid = {}                              # (file, scope): reason
        self.reasons = {}                           # an entry's key: reason
        self.known = set()                          # the files it names
        if text is None:
            return
        body = text.split("\n\n----\n\n", 1)[-1]
        for entry in body.split("\n\n"):
            rows = entry.strip("\n").split("\n")
            if not rows[0]:
                continue
            reason = "\n".join(row.strip() for row in rows[1:]).strip()
            head = rows[0]
            found = re.fullmatch(r"(\S+): (not sorted|as a whole), .*", head)
            if found:
                self.known.add(found.group(1))
                if found.group(2) == "not sorted":
                    self.files.add(found.group(1))
                continue
            found = re.fullmatch(r"(\S+?), (.+?): (.*)", head)
            if not found:
                sys.exit(f"width: the record has an entry that cannot be "
                         f"read:\n{entry}")
            file, scope, what = found.groups()
            scope = "" if scope == "()" else scope
            self.known.add(file)
            if what.startswith("not sorted, "):
                self.scopes.add((file, scope))
            elif what.startswith("laid out, "):
                self.laid[(file, scope)] = reason
            elif what.split(",")[0] == SIGN:
                if len(rows) < 2 or not rows[1].startswith("    | "):
                    sys.exit(f"width: the record has a place of a sign "
                             f"without its line:\n{entry}")
                reason = "\n".join(row.strip() for row in rows[2:]).strip()
                self.reasons[(file, scope, SIGN, rows[1][6:])] = reason
            else:
                found = re.fullmatch(
                    r"(returns|parameter|variable|field)(?: (\w+))?, "
                    r"(.+?)(?:, \d+ times)?", what)
                if not found:
                    sys.exit(f"width: the record has an entry that cannot "
                             f"be read:\n{entry}")
                kind, name, type_ = found.groups()
                self.reasons[(file, scope, kind, name or "", type_)] = reason

    def sorted_(self, file, scope):
        """Is this scope of a file one whose entries the record has?"""
        return not (whole(file) or file in self.files
                    or file not in self.known
                    or (file, scope) in self.scopes
                    or (file, scope) in self.laid)

    def done(self, tree):
        """The files of code that have something sorted."""
        return [file for file in tree.names if not whole(file)
                and file in self.known and file not in self.files]


def plural(number, what):
    return f"{number} {what}{'' if number == 1 else 's'}"


def counted(numbers, wide, signs=0):
    """The counts of something that has no entries, as the record says
    them."""
    return (plural(numbers, "number") + (f", {wide} u32" if wide else "")
            + (f", {signs} of a sign" if signs else ""))


def table(tree, record):
    """The counts, by cluster and by file."""
    def row(files):
        numbers = wide = kept = words = unsorted = unsorted32 = 0
        signs = 0
        wider = {}
        for file in files:
            for column, count in tree.wider.get(file, {}).items():
                wider[column] = wider.get(column, 0) + count
            for scope in tree.scopes(file):
                n, w, s = tree.count(file, scope[1])
                numbers, wide, signs = numbers + n, wide + w, signs + s
                if record.sorted_(file, scope[1]):
                    said = sum(len(at) for entry, at in tree.numbers.items()
                               if entry[:2] == scope
                               and word(record.reasons.get(entry, "")))
                    words += said
                    kept += n + w - said
                elif not whole(file) and (file, scope[1]) not in record.laid:
                    unsorted += n
                    unsorted32 += w
        return (f"{numbers:>8}{unsorted:>11}{wide:>6}{unsorted32:>15}"
                f"{words:>8}{kept:>6}{signs:>6}"
                f"{wider.get('bool', 0):>6}{wider.get('i32', 0):>6}")

    head = (f"{'8 or 16':>8}{'not sorted':>11}{'u32':>6}"
            f"{'u32 not sorted':>15}{'by word':>8}{'kept':>6}"
            f"{'sign':>6}{'bool':>6}{'i32':>6}")
    lines = [f"{'cluster':<10}{head}  its files"]
    named = set()
    for name, files in CLUSTERS:
        files = [file for file in files.split() if file in tree.names]
        named |= set(files)
        lines.append(f"{name:<10}{row(files)}  {' '.join(files)}")
    rest = [file for file in tree.names if file not in named
            and not whole(file)]
    if rest:
        lines.append(f"{'none':<10}{row(rest)}  {' '.join(rest)}")
    apart = [file for file in tree.names if whole(file)]
    lines.append(f"{'as a whole':<10}{row(apart)}  the tables, "
                 + " ".join(file for file in apart if file in WHOLE))
    heads = [file for file in tree.files if file in HEADERS]
    lines.append(f"{'headers':<10}{row(heads)}  {' '.join(heads)}")
    lines.append(f"{'all':<10}{row(tree.files)}")
    lines += ["", f"{'file':<14}{head}"]
    for file in tree.files:
        if not whole(file):
            lines.append(f"{file:<14}{row([file])}")
    return lines


def written(tree, record, said):
    """The record, as text; and the entries that have no reason."""
    found = re.search(r"(\d+)\.\d+", said)
    lines = [
        "The numbers without a sign of the portable C, src/portable_c/, of",
        "8, 16 and 32 bits: of what has been sorted by the rule of step 2,",
        "every one that is left with why it is as wide as it is and has no",
        "sign, and every place where a number with a sign is shifted to the",
        "right or divided, with why it is not below 0 there; of the rest,",
        "how many. That a number is put into fewer bits, or meets one of",
        "another sign, is not here: where clang names such a place, the",
        "code says it with a cast, and nothing else is held of it.",
        "harness/width.py found them, with",
        f"clang {found.group(1) if found else '?'}, and says what the "
        "columns and the kinds are; the reason",
        "under an entry was written by whoever read it. The test",
        "portable_c-width holds the tree to this. `python3 harness/width.py",
        "--record` writes it anew and keeps the reasons; `--sort FILE`",
        "makes entries of a file that is not sorted.", "",
        "The columns: the numbers of 8 or 16 bits that there are, and how",
        "many of those are in what nobody has sorted; the same of the",
        "numbers that are a u32; of the numbers of any of those widths that",
        "are sorted, how many are what they are by a word of the list below",
        "and how many are kept with a reason of their own, the rest being",
        "of a structure that is laid out or of a file as a whole; the",
        "places of a sign; and how many numbers are declared as bool and as",
        "i32, which is what a number becomes that is widened. A cluster",
        "takes the two `not sorted` of its row to nothing.", "",
        "The words, for a number that has its width, or no sign, by what it",
        "is. Such a number has its word for a reason, alone or with what is",
        "not plain behind a colon. The code has no name for any of them.",
        ""]
    for name, what in WORDS.items():
        rows = textwrap.wrap(what, 56)
        lines += [f"    {name:<12}{rows[0]}"]
        lines += [f"    {'':<12}{row}" for row in rows[1:]]
    lines += [""]
    lines += table(tree, record)
    lines += ["", "----"]
    bare = []
    for file in tree.files:
        counts = counted(*tree.count(file))
        if whole(file):
            lines += ["", f"{file}: as a whole, {counts}",
                      f"    {whole(file)}"]
            continue
        if file in record.files or file not in record.known:
            lines += ["", f"{file}: not sorted, {counts}"]
            continue
        for key in tree.scopes(file):
            scope = key[1]
            numbers, wide, signs = tree.count(file, scope)
            where = f"{file}, {scope or '()'}"
            if key in record.scopes:
                lines += ["", f"{where}: not sorted, "
                              f"{counted(numbers, wide, signs)}"]
                continue
            if key in record.laid:
                lines += ["", f"{where}: laid out, "
                              f"{counted(numbers, wide)}"]
                lines += [f"    {row}" for row in
                          record.laid[key].split("\n") if row]
                if not record.laid[key]:
                    bare.append(f"{where}: laid out")
                continue
            entries = []
            for entry, at in tree.numbers.items():
                if entry[:2] == key:
                    _, _, what, name, type_ = entry
                    head = f"{what}{' ' + name if name else ''}, {type_}"
                    entries.append((min(at), head, len(at), None, entry))
            for entry, at in tree.signs.items():
                if entry[:2] == key:
                    entries.append((min(at), SIGN, len(at), entry[3], entry))
            for _, head, times, text, entry in sorted(
                    entries, key=lambda entry: (entry[3] is not None,
                                                entry[0], entry[1])):
                lines += ["", f"{where}: {head}"
                          + (f", {times} times" if times > 1 else "")]
                if text is not None:
                    lines.append(f"    | {text}")
                reason = record.reasons.get(entry, "")
                lines += [f"    {row}" for row in reason.split("\n") if row]
                if re.fullmatch(r"\w+", reason) and (
                        text is not None or reason not in WORDS):
                    # One word: of the list, and for a number.
                    bare.append(f"{where}: {head}: `{reason}` is no "
                                + ("reason for a place of a sign"
                                   if text is not None
                                   else "word of the list"))
                if not reason:
                    bare.append(f"{where}: {head}"
                                + (f": {text}" if text else ""))
    return "\n".join(lines) + "\n", bare


def show(tree, asked):
    file, _, scope = asked.partition(":")
    if file not in tree.files:          # a function: what is handed to it
        for (name, where, kind, text), at in sorted(tree.cuts.items()):
            for line in at:
                if kind == "handed" and tree.called.get((name, line)) == file:
                    print(f"{name}:{line}: {where or '()'}: {text}")
        return
    for key in tree.scopes(file, True):
        if scope and key[1] != scope:
            continue
        print(f"{file}, {key[1] or '()'}")
        for entry, at in tree.numbers.items():
            if entry[:2] == key:
                print(f"  {','.join(map(str, at)):>6}  {entry[2]} "
                      f"{entry[3]}, {entry[4]}")
        places = list(tree.cuts.items()) + list(tree.signs.items())
        for entry, at in sorted(places, key=lambda x: x[1]):
            if entry[:2] == key:
                for line in at:
                    callee = tree.called.get((file, line), "")
                    print(f"  {line:>6}  {entry[2]}"
                          f"{' to ' + callee if callee else ''}: {entry[3]}")


def check(command):
    """Hold the finder against the lines that were written for it: what
    is wrong, as lines to print."""
    text = CHECK.read_text()
    finder = read(command, CHECK.parent, CHECK.name, text)
    wrong = []
    for number, line in enumerate(text.split("\n"), 1):
        said = re.search(r"/\* (cuts|number): ([\w, ]*)\*/\s*$", line)
        wanted = sorted(said.group(2).replace(",", " ").split()) if said \
            else []
        if said and said.group(1) == "number":
            got = sorted(key[2] for key, at in finder.numbers.items()
                         for line in at if line == number)
        else:
            got = sorted([kind for at, kind, _ in finder.cuts
                          if at == number]
                         + [SIGN for at, _ in finder.signs if at == number])
        if got != wanted:
            wrong.append(f"{CHECK.name}:{number}: finds "
                         f"{', '.join(got) or 'nothing'}, where "
                         f"{', '.join(wanted) or 'nothing'} is to be "
                         f"found: {line.split('/*')[0].strip()}")
    return wrong


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--record", action="store_true",
                        help="write harness/width.txt anew, with the "
                             "reasons that it has")
    parser.add_argument("--show", metavar="FILE[:SCOPE]|FUNCTION",
                        help="list the numbers and the cuts of a file or "
                             "of a scope, or what is cut in being handed "
                             "to a function: printed, and held by nothing")
    parser.add_argument("--warned", metavar="FILE", action="append",
                        default=[],
                        help="what clang says of a file under the "
                             "warnings, sorted or not")
    parser.add_argument("--sort", metavar="FILE[:SCOPE]", action="append",
                        default=[],
                        help="make entries of a file, or of one scope of "
                             "it, and write the record anew")
    parser.add_argument("--cc", metavar="CLANG", default="clang",
                        help="the clang to ask, if not the first one found")
    parser.add_argument("source", nargs="?", type=Path, default=SOURCE,
                        help="the tree, if not src/portable_c")
    arguments = parser.parse_args()

    command, said = int16.compiler(arguments.cc)
    if not command:
        print(f"width: skipped: {said}. It needs clang.")
        sys.exit(SKIPPED)

    wrong = check(command)
    if wrong:
        print(f"{said} does not find what the finder is known by:")
        print("\n".join(wrong))
        sys.exit(1)

    if arguments.warned:
        for name in arguments.warned:
            print("\n".join(warned(command, arguments.source, name, True)))
        return
    tree = Tree(command, arguments.source)
    if arguments.show:
        show(tree, arguments.show)
        return
    before = RECORD.read_text() if RECORD.exists() else None
    record = Record(before)
    for asked in arguments.sort:
        file, _, scope = asked.partition(":")
        if file not in tree.files or whole(file):
            sys.exit(f"width: {file} is no file to sort")
        if file in record.files or file not in record.known:
            # Its scopes one by one from here on, each as it was.
            record.files.discard(file)
            record.known.add(file)
            record.scopes |= set(tree.scopes(file))
        if scope:
            record.scopes.discard((file, scope))
        else:
            record.scopes = {key for key in record.scopes if key[0] != file}
    now, bare = written(tree, record, said)
    print("\n".join(table(tree, record)))
    if arguments.record or arguments.sort:
        RECORD.write_text(now)
        print(f"\nWritten to {RECORD.relative_to(ROOT)}.")
        if bare:
            print(f"{len(bare)} entries want a reason.")
        return

    failed = False
    if now != before:
        failed = True
        print("\nNot as harness/width.txt has it, which is the lines with "
              "a minus:")
        print("\n".join(list(difflib.unified_diff(
            (before or "").split("\n"), now.split("\n"), "the record",
            "the tree", n=1, lineterm=""))[2:80]))
        print("\nA count that has moved wants the record written anew, "
              "`python3\nharness/width.py --record`: read what moved. A "
              "number or a place of a sign that\nis new in something "
              "sorted wants its reason under it then, or the number to "
              "be\nwidened: `--show FILE` has the lines, and the top of "
              "harness/width.py what a\nreason is.")
    elif bare:
        failed = True
        print("\nThe record has no reason for:")
        print("\n".join(bare))
    with ThreadPoolExecutor() as pool:
        said = [line for lines in pool.map(
            lambda name: warned(command, arguments.source, name),
            record.done(tree)) for line in lines]
    hidden = switched_off(arguments.source)
    if hidden:
        failed = True
        print("\nA warning is switched off in the source, which is not "
              "done:")
        print("\n".join(hidden))
    if said:
        failed = True
        print(f"\nclang says of what is sorted, under {' '.join(WARNINGS)}:")
        print("\n".join(said))
        print("\nA number that is put into fewer bits, or that meets one "
              "of another sign, wants\nthe type that holds it, which is "
              "tried first, or else a cast at the place: one\nthat the "
              "compiler keeps (TODO.md, \"N0c\"), with a comment where the "
              "program counts\non what is cut off. A boolean that is none "
              "wants to be a number. No warning is\nswitched off for a "
              "line, and nothing is asked beyond what clang names.")
    if failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
