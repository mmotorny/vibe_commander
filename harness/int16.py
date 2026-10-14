#!/usr/bin/env python3
"""Where the portable C can tell an int of 32 bits from one of 16.

The portable C, src/portable_c/, was written for a compiler whose int
has 16 bits, translated from assembly and judged so, and is built by
one whose int has 32: the program of 16 bits is gone (TODO.md, "The
portable C in 32 bits: the plan"). So "DOS" and "the two builds" below
are the code as it was written and judged, with an int of 16 bits, and
the code as it is compiled now. A place is where the program can do
otherwise than what it was translated from by the int alone, and the
record says of each why it does not.

That is also where step 2 of the road widens (TODO.md, "The portable C
in a flat memory: the plan"): a variable is to become wider wherever
wrapping round at 16 bits would be unexpected, and what is found here
is every number of 16 bits that is looked at before it is cut, with
the reason why it stays within. The plan makes this the finder of the
widths, and its record the start of theirs; until then the test stays
as it is and the record is kept up.

The numbers of the code are u8, u16 and u32, as wide for the one int as
for the other, so what it keeps is the same in both.
What it works out on the way is not. C makes an int of every number
that is narrower than one before it adds, takes away, multiplies or
shifts, and of two numbers of 16 bits without a sign that is

  - on DOS an int without a sign, of 16 bits: the result wraps round;
  - where an int has 32 bits an int with a sign: it does not, and a
    difference can be below 0.

Put back into 16 bits the two are the same number. Looked at as they
are, they are not: `(a + b) >> 1`, `a - b < c`, `a + b < a`, `(u32)(a *
b)`. This finds every place where such a number is looked at, and the
test `portable_c-int16` of the build holds the tree to a record of them.

How. clang reads a file for a machine whose int has 32 bits, as
harness/hostcheck.py has gcc read it, and prints what it understood,
every expression with its type. Nothing is compiled or run. This walks
that and works out of every expression, by the rules of C,

  - the type that it has where an int has 16 bits, and where it has 32;
  - between which two numbers its value lies where an int has 32 bits.
    A variable is taken to hold any number of its type. Three things
    are known beyond the types: a number that is written; how long a
    thing is and where a field is in it, sizeof and offsetof, which
    clang is asked for; and a variable that a loop counts with, inside
    that loop, `for (i = 0; i < 13; i++)`.

What the expression is on DOS is that value wrapped round into the type
of 16 bits, as long as nothing has been looked at on the way: adding,
taking away, multiplying, shifting left, `&`, `|`, `^`, `~` and `-`
all give the same low 16 bits whatever the bits above them were. So an
expression is the same in both builds, for every number that its
variables can hold, when its value fits the type that it has on DOS.
`a + b` of two u16 lies between 0 and 131,070 and does not fit;
`(u16)(a + b)` does; `a - b & 0xFF` does; of two u8, `a + b` does and
`a << 8` does not, since the int of DOS has a sign.

A place is where an expression that does not fit is looked at (KINDS
below): compared, divided, shifted right, made a number of 32 bits,
asked whether it is 0, added to a pointer or made one. And where a
number that is written below 0 is kept as one of 16 bits without a
sign, which is no arithmetic and the same number in both builds, but
64K less that much on where it is later added to a pointer. And where
two numbers that do fit are held against one another or divided, of which
one has a sign and may be below 0 and the other is of 16 bits without:
C makes both numbers without a sign on DOS and both numbers with one
where an int has 32 bits. Three things are known to be the same and
are no places: whether a number is 0, and whether two are equal, unless
they can be 65,536 or a multiple of it apart; and a number added to a
pointer that is neither below 0 nor beyond 65,535, whatever its type,
since DOS adds it to an offset of 16 bits.

A place is where the two builds can differ by the types alone, not
where they do: whether the number ever leaves 16 bits there is for a
reader to say. So every place is in the record, harness/int16.txt, with
what a reader found: why it is the same in both builds. Where it is
not, or is so only by something that the code does not say, the code
gets the cast that says what DOS does, `(u16)(a - b) < c`, which is the
same code on DOS, and the place is gone. The test fails

  - for a place that the record has not: a line that was written or
    changed since, which is then read;
  - for a place of the record that is no more, so that the record is
    always what is so;
  - for a place of the record without a reason.

A place is known by its file, its function, its kind and the text of
its line, not by the number of the line, so the record stays as it is
when lines are added above. A line that is changed is another place,
and is read again.

    python3 harness/int16.py                the counts, and the test
    python3 harness/int16.py --show view.c  the places of a file or of
                                            a kind, with their lines
    python3 harness/int16.py --record       the record anew

`--record` keeps the reason of every place that is still there, takes
out what is gone, and writes what is new without a reason: that is then
written into the file by whoever has read the place, under its line.

What is read: every file of code and of tables. dos32.c, the system's
functions, was written for an int of 32 bits and never compiled for
another, so a place there has nothing to be the same as: its reason
says that the number is meant as it is. What a macro makes is a place
where the macro is used. The words of the addresses in vc.h, WORDS
below, hold a megabyte of segments and offsets in a memory without
segments (TODO.md, "P1, the words in one place"): they are of the 32
bits alone, and what is written in one is no place. What a word is
handed is read like anything else, and it cuts every number that it is
handed to 16 bits.

What it cannot see:

  - A number of 16 bits that has wrapped round already and is then
    added to a pointer: FFFEh for two back. On DOS the pointer wraps
    round with it; in a flat memory it lands 64K further on. Only where
    the sum is made in the place itself is it found, as `index`. And
    where the number is written, `(u16)-160`, the place where it is
    made one of 16 bits is found, as `minus`: where it goes from there,
    through a variable or into a function, is for the reader to follow.
    One that is worked out, `first - shown`, is found nowhere.
  - A pointer that is stepped past the end of its segment or back
    before its beginning, which begins again at the other end on DOS:
    `text--`, `text - count`. Only where what is added is worked out
    in the place itself and may be below 0, `text[i - 1]`, is it
    found, as `back`. That steps back in both builds, to the same byte
    as long as the pointer is not before the beginning of its segment
    then. Where the pointer is the beginning of a segment, as the
    viewer's text is, the cast says what DOS does, `text[(u16)(i -
    1)]`; of any other pointer nothing says it.
  - Two pointers held against one another, which goes by their offsets
    alone on DOS.
  - What a variable can hold. It is taken to hold any number of its
    type, so a place may be one where nothing ever differs; and a
    number that is kept in a variable of 16 bits is as right as what
    was kept.
  - How long a thing is on DOS where it holds a pointer, which is two
    bytes there: a length is what clang says, for 32 bits. Only the
    headers of the tables hold pointers, BOXHD and BOXMN of tables.h.
  - Anything that clang does not take. The finder stops with clang's
    words then, since what clang refuses is not in its account.
  - What is right C and means another thing without the layout of DOS
    under it: harness/hostcheck.py and TODO.md have those.

It is held against lines that were written for it, harness/int16check.c,
each with the kinds that are to be found in it or with none, every time
it runs: clang's account of a file is not promised to stay as it is
from one version to the next, and a finder that finds nothing says
nothing.

Exits with status 0 if the tree is as the record has it, 1 if not, and
77 if there is no clang to ask, which the build takes for a test that
was skipped.
"""
import argparse
import json
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import hostcheck                        # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c"
RECORD = Path(__file__).resolve().parent / "int16.txt"
CHECK = Path(__file__).resolve().parent / "int16check.c"
SKIPPED = 77

# How clang is asked: for the 386, whose int has 32 bits and whose long
# has too, as C of 1999, with a char that has no sign, as Open Watcom's
# has none; no warning is wanted, and what it understood is printed.
OPTIONS = ["--target=i386-pc-linux-gnu", "-x", "c", "-std=c99",
           "-fsyntax-only", "-funsigned-char", "-w", "-ferror-limit=0",
           "-Xclang", "-ast-dump=json"]

# Where a field is in a structure, offsetof, clang works out and does not
# print. So it is written for clang as the length of an array of that
# many bytes, which is the same number of the same type, and lengths are
# asked for: see `lengths`. The header that defines offsetof leaves one
# that is defined already as it is.
OFFSETOF = ("-Doffsetof(type,field)="
            "sizeof(char[__builtin_offsetof(type,field)])")

# The words of the addresses, of vc.h: what is written in one is no
# place. PLACE is of harness/int16check.c, which has it written as one
# of them, to hold this to.
WORDS = {"ARENA", "MAIN_SEG", "AT", "SEG_OF", "OFFSET_IN_SEG",
         "MAIN_AS_PANEL", "PANEL_AS_MAIN", "NUMBER_OF", "NEAR", "MD_PLACE",
         "MD_POINTER", "TREE_OFFSET", "TREE_ENTRY", "PLACE"}

# The kinds, in the order of the summary: the name that the record has
# for each, and what is done there with a number that the two builds may
# have otherwise. Held against another, it may be below 0 or beyond
# 65,535 where an int has 32 bits and is wrapped round on DOS. `signed`
# is two numbers that are the same in both builds until C makes them
# one type to hold them against one another: without a sign on DOS,
# with one where an int has 32 bits. Added to a pointer, a number of
# 65,536 or more is 64K further on in a flat memory and wrapped round on
# DOS; one below 0 is back in both, but on DOS only as far as the
# segment begins. `minus` is no arithmetic: a number written below 0
# that is kept in 16 bits without a sign, -160 as 65,376, which is the
# way back only where it is added to a number of 16 bits and not to a
# pointer.
KINDS = [
    ("compare", "held against another number"),
    ("signed", "one with a sign held against one of 16 bits without"),
    ("divide", "divided, or a remainder taken of it"),
    ("shift", "shifted right, or the count of a shift"),
    ("widen", "made a number of 32 bits"),
    ("truth", "asked whether it is 0"),
    ("index", "added to a pointer, and it may be 65,536 or more"),
    ("back", "added to a pointer, and it may be below 0"),
    ("minus", "written below 0 and kept as a number of 16 bits without "
              "a sign"),
    ("address", "made a pointer"),
]

# A type, as (has a sign, bits), and the types of C by name, each as it
# is where an int has 16 bits and where it has 32. u32 is a long on DOS
# and an int elsewhere (vc.h), and 32 bits without a sign in both.
S8, U8 = (True, 8), (False, 8)
S16, U16 = (True, 16), (False, 16)
S32, U32 = (True, 32), (False, 32)
S64, U64 = (True, 64), (False, 64)
BOOL = (False, 1)                       # 0 or 1, whatever it is made of
INT, UNSIGNED = (S16, S32), (U16, U32)
TYPES = {
    "u8": (U8, U8), "unsigned char": (U8, U8), "char": (U8, U8),
    "signed char": (S8, S8),
    "u16": (U16, U16), "unsigned short": (U16, U16), "short": (S16, S16),
    "int": INT, "unsigned int": UNSIGNED, "unsigned": UNSIGNED,
    "__size_t": UNSIGNED, "size_t": UNSIGNED, "__ptrdiff_t": INT,
    "ptrdiff_t": INT,
    "long": (S32, S32), "unsigned long": (U32, U32), "u32": (U32, U32),
    "long long": (S64, S64), "unsigned long long": (U64, U64),
    # What step 2 of the road widens to: harness/width.py.
    "_Bool": (BOOL, BOOL), "bool": (BOOL, BOOL), "i32": (S32, S32),
}
POINTER = "pointer"
WORD = 1 << 16


def span(type_):
    """The least and the greatest number of a type."""
    signed, bits = type_
    if signed:
        return -(1 << bits - 1), (1 << bits - 1) - 1
    return 0, (1 << bits) - 1


def within(low, high, type_):
    least, greatest = span(type_)
    return least <= low and high <= greatest


def promoted(type_, int_bits):
    """What C makes of a type before it works with it: an int, of
    whatever is narrower than one."""
    return (True, int_bits) if type_[1] < int_bits else type_


def usual(left, right, int_bits):
    """The type that C works in, of two."""
    left, right = promoted(left, int_bits), promoted(right, int_bits)
    if left == right:
        return left
    if left[1] != right[1]:
        return left if left[1] > right[1] else right
    return (False, left[1])


def type_of(type_):
    """Of what clang says the type of a thing is: the two types of a
    number, POINTER, or None for what is neither."""
    names = [" ".join(re.sub(r"\b(const|volatile)\b", "", name).split())
             for name in (type_.get("qualType", ""),
                          type_.get("desugaredQualType", ""))]
    for name in names:
        if name in TYPES:
            return TYPES[name]
        if name.startswith("enum "):
            return INT
    name = names[1] or names[0]
    if name.endswith("*") or name.endswith("]") or "(*" in name:
        return POINTER
    return None


def literal16(text, value):
    """The type on DOS of a number that is written: the first of those
    that C has for its form which holds it."""
    found = re.fullmatch(r"(0[xX][0-9a-fA-F]+|\d+)([uUlL]*)", text or "")
    if found:
        digits, suffix = found.group(1), found.group(2).lower()
        decimal = not (digits.startswith("0") and len(digits) > 1)
    else:                               # not to be had: as in hexadecimal
        suffix, decimal = "", False
    if "u" in suffix:
        types = [U32] if "l" in suffix else [U16, U32]
    elif "l" in suffix:
        types = [S32, U32]
    elif decimal:
        types = [S16, S32, U32]
    else:
        types = [S16, U16, S32, U32]
    for type_ in types:
        if within(value, value, type_):
            return type_
    return U32


class Value:
    """A number as the two builds have it: its type in each, and between
    which two numbers it lies where an int has 32 bits. On DOS it is
    that, wrapped round into the type of 16 bits."""

    def __init__(self, t16, t32, low, high):
        self.t16, self.t32 = t16, t32
        if not within(low, high, t32):
            low, high = span(t32)       # wrapped round in 32 bits too
        self.low, self.high = low, high

    def exact(self):
        """Is it the same number in both builds, whatever it is?"""
        return within(self.low, self.high, self.t16)


def whole(types):
    """Any number of a type: a variable, what a function returns."""
    return Value(types[0], types[1], *span(types[1]))


def one(types, value):
    return Value(types[0], types[1], value, value)


def truth_value():
    return Value(S16, S32, 0, 1)


def worked(op, a, b, type_):
    """Between which numbers the result of `op` lies, of two that have
    been made the type that C works in."""
    if a.low == a.high and b.low == b.high and op in "&|^":
        value = {"&": a.low & b.low, "|": a.low | b.low,
                 "^": a.low ^ b.low}[op]
        return value, value
    if op == "+":
        return a.low + b.low, a.high + b.high
    if op == "-":
        return a.low - b.high, a.high - b.low
    if op == "*":
        corners = [x * y for x in (a.low, a.high) for y in (b.low, b.high)]
        return min(corners), max(corners)
    if op in ("<<", ">>"):
        if b.low < 0 or b.high > 31:
            return span(type_)
        corners = [x << y if op == "<<" else x >> y
                   for x in (a.low, a.high) for y in (b.low, b.high)]
        return min(corners), max(corners)
    if op == "/" and a.low >= 0 and b.low >= 0:
        return a.low // max(b.high, 1), a.high // max(b.low, 1)
    if op == "%" and a.low >= 0 and b.low >= 0:
        return 0, min(a.high, max(b.high - 1, 0))
    if op == "&":                       # a mask without a sign cuts
        if a.low >= 0 and b.low >= 0:
            return 0, min(a.high, b.high)
        if a.low >= 0:
            return 0, a.high
        if b.low >= 0:
            return 0, b.high
    if op in "|^" and a.low >= 0 and b.low >= 0:
        return 0, (1 << max(a.high, b.high).bit_length()) - 1
    return span(type_)


class Lines:
    """Where a thing is written. clang says the file and the line of a
    place only where they are other than at the place before, so its
    account is walked in the order it was written in. A place that a
    macro makes has two: where the macro is used, and where the text is
    written, in the macro's definition or, for what the macro was
    handed, where it is used."""

    def __init__(self):
        self.file, self.line = "", 0

    def bare(self, loc):
        if "file" in loc:
            self.file = loc["file"]
        if "line" in loc:
            self.line = loc["line"]
        return (self.file, self.line, loc.get("offset"), loc.get("tokLen"))

    def at(self, loc):
        """(where it is used, where it is written)."""
        if "spellingLoc" in loc or "expansionLoc" in loc:
            found = {}
            for key in loc:             # in the order they were written
                if key in ("spellingLoc", "expansionLoc"):
                    found[key] = self.bare(loc[key])
            used = found.get("expansionLoc") or found["spellingLoc"]
            return used, found.get("spellingLoc") or used
        here = self.bare(loc)
        return here, here

    def mark(self, node):
        at = None
        for key in list(node):
            if key == "loc":
                self.at(node["loc"])
            elif key == "range":
                begin = self.at(node["range"].get("begin", {}))
                end = self.at(node["range"].get("end", {}))
                at = (begin[0], begin[1], end[1], end[0])
            elif key == "inner":
                if at is None:
                    here = (self.file, self.line, None, None)
                    at = (here, here, here, here)
                for inner in node["inner"]:
                    if isinstance(inner, dict):
                        self.mark(inner)
        if at is None:
            here = (self.file, self.line, None, None)
            at = (here, here, here, here)
        node["_at"] = at


def in_words(text):
    """The lines of a file that are the body of a word of the addresses:
    of a #define of one of WORDS, with the lines that it goes on in."""
    lines, inside = set(), False
    for number, line in enumerate(text.split("\n"), 1):
        if not inside:
            found = re.match(r"\s*#\s*define\s+(\w+)", line)
            inside = bool(found) and found.group(1) in WORDS
        if inside:
            lines.add(number)
            inside = line.rstrip().endswith("\\")
    return lines


class Finder:
    """The places of one file."""

    def __init__(self, name, text, sizes):
        self.name = name
        self.sizes = sizes
        self.texts = {"<stdin>": text}
        self.words = {}
        self.enums = {}
        self.function = ""
        self.counted = {}               # a loop's variable: its bounds
        self.own = set()                # the variables of a function
        self.pointed = set()            # those whose address it takes
        self.places = set()             # (file, line, kind, function)

    # Where things are.

    def text(self, file):
        if file not in self.texts:
            try:
                self.texts[file] = Path(file).read_text(encoding="latin-1")
            except OSError:
                self.texts[file] = ""
        return self.texts[file]

    def in_word(self, written):
        """Is this written in a word of the addresses?"""
        file, line = written[0], written[1]
        if file not in self.words:
            self.words[file] = in_words(self.text(file))
        return line in self.words[file]

    def report(self, node, kind):
        used, begins, ends, end_used = node["_at"]
        if (self.in_word(begins) and self.in_word(ends)
                and used[:3] == end_used[:3]):
            return                      # all of it one word's body
        file = self.name if used[0] == "<stdin>" else Path(used[0]).name
        self.places.add((file, used[1], kind, self.function))

    # What a thing is.

    def of(self, node):
        types = type_of(node.get("type", {}))
        if types is None or types == POINTER:
            return types
        return whole(types)

    def same(self, value, types):
        """A number that has been looked at and reported is the same in
        both builds from there on, for what is done with it next."""
        least, greatest = span(types[0])
        if value.low > greatest or value.high < least:
            return whole((types[0], types[0]))
        return Value(types[0], types[1], max(value.low, least),
                     min(value.high, greatest))

    def convert(self, value, types, node):
        """A number made another type, as an assignment, a cast or the
        types of C's arithmetic make it. Made a number of 32 bits, it
        has to be the same in both builds before."""
        if types[0][1] > 16 and not value.exact():
            self.report(node, "widen")
            value = self.same(value, (value.t16, value.t32))
        return Value(types[0], types[1], value.low, value.high)

    def kept(self, value, types, node):
        """A number that is written, below 0, and made one of 16 bits
        without a sign, by a cast, as what a function takes or returns,
        or as what a variable is given: 65,376 for -160. It is the same
        number in both builds, and no arithmetic is where it is used.
        But added to a pointer it steps back on DOS, where the offset
        wraps round, and 64K less that much on in a flat memory, so
        whoever reads the place follows the number to where it is
        used."""
        if types[0] == U16 and value.low == value.high < 0:
            self.report(node, "minus")

    def meet(self, a, b, node, kind, equal=False):
        """Two numbers held against one another or divided: both are
        made the type that C works in, and have to be the same in both
        builds then. Returns the two as they are worked with, and the
        types."""
        types = (usual(a.t16, b.t16, 16), usual(a.t32, b.t32, 32))
        made = [self.convert(a, types, node), self.convert(b, types, node)]
        differ = [before for before, after in zip((a, b), made)
                  if not after.exact()]
        if equal and types[0][1] == 16:
            # Equal on DOS is 65,536 or a multiple of it apart.
            if (made[0].high - made[1].low < WORD
                    and made[1].high - made[0].low < WORD):
                differ = []
        if differ:
            self.report(node, kind if any(not before.exact()
                                          for before in differ) else "signed")
        return [self.same(value, types) for value in made], types

    def arithmetic(self, op, a, b, node):
        if op in ("/", "%"):
            (a, b), types = self.meet(a, b, node, "divide")
        elif op in ("<<", ">>"):
            types = (promoted(a.t16, 16), promoted(a.t32, 32))
            a = self.convert(a, types, node)
            if not b.exact() or op == ">>" and not a.exact():
                self.report(node, "shift")
            if op == ">>":
                a = self.same(a, types)
        else:
            types = (usual(a.t16, b.t16, 16), usual(a.t32, b.t32, 32))
            a = self.convert(a, types, node)
            b = self.convert(b, types, node)
        return Value(types[0], types[1], *worked(op, a, b, types[1]))

    def truth(self, node):
        value = self.value(node)
        if (isinstance(value, Value) and not value.exact()
                and (value.low <= -WORD or value.high >= WORD)):
            self.report(node, "truth")  # 0 on DOS is a multiple of 65,536

    def index(self, value, node):
        """A number added to a pointer. Where it is neither below 0 nor
        beyond 65,535, the two builds step as far, whatever its type:
        DOS adds it to an offset of 16 bits, and what the int of DOS
        made of its sign is of no account."""
        if isinstance(value, Value) and not value.exact():
            if value.high >= WORD:
                self.report(node, "index")
            elif value.low < 0:
                self.report(node, "back")

    def operand(self, node):
        """What is worked with: the number before C made it the type of
        the arithmetic, which is worked out here for both builds."""
        while (node.get("kind") == "ImplicitCastExpr"
               and node.get("castKind") == "IntegralCast"):
            node = node["inner"][0]
        return self.value(node)

    def cast(self, node, inner):
        how = node.get("castKind")
        types = type_of(node.get("type", {}))
        value = self.value(inner)
        if how in ("LValueToRValue", "NoOp"):
            return value
        if (how == "IntegralCast" and isinstance(value, Value)
                and types not in (None, POINTER)):
            self.kept(value, types, node)
            return self.convert(value, types, node)
        if how == "IntegralToPointer":
            if isinstance(value, Value) and not value.exact():
                self.report(node, "address")
            return POINTER
        return self.of(node)

    def literal(self, node):
        value = int(node["value"])
        t32 = type_of(node.get("type", {}))[1]
        file, _, offset, length = node["_at"][1]
        text = None
        if offset is not None and length:
            text = self.text(file)[offset:offset + length]
        return Value(literal16(text, value), t32, value, value)

    def value(self, node):
        """What an expression is; and every place in it is reported on
        the way. A statement or a declaration is walked for what is in
        it."""
        kind = node.get("kind")
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]

        if inner and kind in ("ParenExpr", "ConstantExpr",
                              "ExprWithCleanups"):
            return self.value(inner[0])
        if kind == "IntegerLiteral":
            return self.literal(node)
        if kind == "CharacterLiteral":
            return one(INT, int(node["value"]))
        if kind == "UnaryExprOrTypeTraitExpr":
            size = self.sizes.get(sized(node))
            if size is None:            # a length, of whatever
                return Value(U16, U32, 0, WORD - 1)
            return one(UNSIGNED, size)
        if kind == "DeclRefExpr":
            named = node.get("referencedDecl", {})
            if named.get("kind") == "EnumConstantDecl":
                if named.get("name") in self.enums:
                    return one(INT, self.enums[named["name"]])
                return whole(INT)
            value = self.of(node)
            if named.get("id") in self.counted and isinstance(value, Value):
                low, high = self.counted[named["id"]]
                return Value(value.t16, value.t32, max(low, value.low),
                             min(high, value.high))
            return value
        if kind in ("ImplicitCastExpr", "CStyleCastExpr") and inner:
            return self.cast(node, inner[0])
        if kind == "UnaryOperator":
            return self.unary(node, inner[0])
        if kind == "BinaryOperator":
            return self.binary(node, *inner)
        if kind == "CompoundAssignOperator":
            return self.compound(node, *inner)
        if kind == "ConditionalOperator":
            self.truth(inner[0])
            a, b = self.operand(inner[1]), self.operand(inner[2])
            if isinstance(a, Value) and isinstance(b, Value):
                types = (usual(a.t16, b.t16, 16), usual(a.t32, b.t32, 32))
                a = self.convert(a, types, node)
                b = self.convert(b, types, node)
                return Value(types[0], types[1], min(a.low, b.low),
                             max(a.high, b.high))
            return self.of(node)
        if kind == "ArraySubscriptExpr":
            self.value(inner[0])
            self.index(self.operand(inner[1]), node)
            return self.of(node)

        if kind == "FunctionDecl":
            self.function = node.get("name", "")
            self.own, self.pointed = set(), set()
            self.variables(node)
            for child in inner:
                self.value(child)
            self.function = ""
            return None
        if kind == "EnumDecl":
            number = -1
            for child in inner:
                if child.get("kind") != "EnumConstantDecl":
                    continue
                said = [x for x in child.get("inner", [])
                        if isinstance(x, dict) and "value" in x]
                number = int(said[0]["value"]) if said else number + 1
                self.enums[child.get("name")] = number
            return None
        if kind == "VarDecl":
            types = type_of(node.get("type", {}))
            if inner and types not in (None, POINTER):
                value = self.operand(inner[-1])
                if isinstance(value, Value):
                    self.kept(value, types, inner[-1])
                    self.convert(value, types, inner[-1])
            else:
                for child in inner:
                    self.value(child)
            return None
        if kind in ("IfStmt", "WhileStmt"):
            self.truth(inner[0])
            for child in inner[1:]:
                self.value(child)
            return None
        if kind == "DoStmt":
            self.value(inner[0])
            self.truth(inner[1])
            return None
        if kind == "ForStmt":
            parts = [child if isinstance(child, dict) and child.get("kind")
                     else None for child in node.get("inner", [])]
            for place, child in enumerate(parts[:4]):
                if child and place == 2:
                    self.truth(child)
                elif child:
                    self.value(child)
            counts = self.counts(parts) if len(parts) == 5 else None
            if counts:
                self.counted[counts[0]] = counts[1:]
            if parts[-1]:
                self.value(parts[-1])
            if counts:
                del self.counted[counts[0]]
            return None
        if kind == "SwitchStmt":
            value = self.operand(inner[0])
            if isinstance(value, Value):
                types = (promoted(value.t16, 16), promoted(value.t32, 32))
                if not self.convert(value, types, inner[0]).exact():
                    self.report(inner[0], "compare")
            for child in inner[1:]:
                self.value(child)
            return None
        if kind in ("TypedefDecl", "RecordDecl"):
            return None

        # Anything else: a call, whose arguments clang has made the
        # types that are taken; a field; a statement. What is in it is
        # read, and it is what its type says.
        for child in inner:
            self.value(child)
        return self.of(node) if "type" in node else None

    def variables(self, node):
        """Note the variables that a function has for itself, and those
        whose address it takes: one of those, as one that is not the
        function's own, can change where nothing names it."""
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
        if node.get("kind") in ("VarDecl", "ParmVarDecl") and (
                "storageClass" not in node):
            self.own.add(node.get("id"))
        if node.get("kind") == "UnaryOperator" and node.get("opcode") == "&":
            named = inner[0]
            while named.get("kind") == "ParenExpr":
                named = named["inner"][0]
            if named.get("kind") == "DeclRefExpr":
                self.pointed.add(named.get("referencedDecl", {}).get("id"))
        for child in inner:
            self.variables(child)

    def counts(self, parts):
        """Of a loop that counts, `for (i = 0; i < 13; i++)`, the
        variable and the least and the greatest number that it is
        inside the loop, as (variable, least, greatest); None of any
        other. The variable is one of the function, set to a number
        that is written, stepped by one and held against a limit;
        nothing inside the loop changes it, nothing in the function
        takes its address, and the loop has no label that a jump could
        enter it by. The limit need be no constant where the loop goes
        on while the variable is below it: it is below the greatest
        number that the limit can be then."""
        first, _, limit, step, body = parts
        if not (first and limit and step and body):
            return None

        def variable(node):
            node = unwrapped(node)
            named = node.get("referencedDecl", {})
            if node.get("kind") == "DeclRefExpr" and named.get("kind") in (
                    "VarDecl", "ParmVarDecl"):
                return named.get("id")
            return None

        def unwrapped(node):
            while node.get("kind") in ("ParenExpr", "ImplicitCastExpr"):
                node = node["inner"][0]
            return node

        def changed(node, name):
            kind, inner = node.get("kind"), [
                x for x in node.get("inner", []) if isinstance(x, dict)]
            if kind == "LabelStmt":
                return True
            if (kind == "UnaryOperator" and node.get("opcode") in (
                    "++", "--") or kind == "CompoundAssignOperator"
                    or kind == "BinaryOperator" and node.get("opcode") == "="):
                if variable(inner[0]) == name:
                    return True
            return any(changed(child, name) for child in inner)

        first, limit, step = unwrapped(first), unwrapped(limit), unwrapped(
            step)
        if first.get("kind") != "BinaryOperator" or first.get("opcode") != "=":
            return None
        name = variable(first["inner"][0])
        begin = self.operand(first["inner"][1])
        if (name not in self.own or name in self.pointed
                or not isinstance(begin, Value) or begin.low != begin.high
                or changed(body, name) or changed(limit, name)):
            return None
        begin = begin.low
        if (step.get("kind") != "UnaryOperator" or step.get("opcode") not in (
                "++", "--") or variable(step["inner"][0]) != name
                or limit.get("kind") != "BinaryOperator"
                or variable(limit["inner"][0]) != name):
            return None
        up = step["opcode"] == "++"
        how = limit.get("opcode")
        end = self.operand(limit["inner"][1])
        if not isinstance(end, Value) or not end.exact():
            return None
        if up and how == "<" and begin >= 0:
            return name, begin, max(begin, end.high - 1)
        if up and how == "<=" and begin >= 0:
            return name, begin, max(begin, end.high)
        if up and how == "!=" and end.low == end.high and begin <= end.low:
            return name, begin, max(begin, end.low - 1)
        if not up and how in ("!=", ">") and end.low == end.high == 0 and (
                begin >= 1):
            return name, 1, begin
        return None

    def unary(self, node, inner):
        op = node.get("opcode")
        if op == "!":
            self.truth(inner)
            return truth_value()
        if op in ("-", "~", "+"):
            value = self.operand(inner)
            if not isinstance(value, Value):
                return self.of(node)
            types = (promoted(value.t16, 16), promoted(value.t32, 32))
            value = self.convert(value, types, node)
            if op == "-":
                low, high = -value.high, -value.low
            elif op == "~":
                low, high = -1 - value.high, -1 - value.low
            else:
                low, high = value.low, value.high
            if not types[1][0] and op != "+":   # no sign where int has 32
                top = span(types[1])[1] + 1
                if value.low == value.high:
                    low = high = low % top
                else:
                    low, high = span(types[1])
            return Value(types[0], types[1], low, high)
        self.value(inner)               # ++, --, &, *
        return self.of(node)

    def binary(self, node, left, right):
        op = node.get("opcode")
        if op == ",":
            self.value(left)
            return self.value(right)
        if op in ("&&", "||"):
            self.truth(left)
            self.truth(right)
            return truth_value()
        if op == "=":
            self.value(left)
            types = type_of(left.get("type", {}))
            if types in (None, POINTER):
                self.value(right)
                return types
            value = self.operand(right)
            if isinstance(value, Value):
                self.kept(value, types, node)
                return self.convert(value, types, node)
            return whole(types)
        a, b = self.operand(left), self.operand(right)
        numbers = isinstance(a, Value) and isinstance(b, Value)
        if op in ("<", ">", "<=", ">=", "==", "!="):
            if numbers:
                self.meet(a, b, node, "compare", op in ("==", "!="))
            return truth_value()
        if numbers:
            return self.arithmetic(op, a, b, node)
        if a == POINTER and b == POINTER and op == "-":
            return whole(INT)           # how far two pointers are apart
        if POINTER in (a, b):
            self.index(a, node)
            self.index(b, node)
            return POINTER
        return self.of(node)

    def compound(self, node, left, right):
        op = node.get("opcode")[:-1]
        a, b = self.value(left), self.operand(right)
        if a == POINTER:
            self.index(b, node)
            return POINTER
        if isinstance(a, Value) and isinstance(b, Value):
            types = type_of(left.get("type", {}))
            return self.convert(self.arithmetic(op, a, b, node), types,
                                node)
        return self.of(node)


def sized(node):
    """Of a sizeof, the type that it is the length of, as clang names
    it; None for anything else."""
    if node.get("kind") != "UnaryExprOrTypeTraitExpr" or (
            node.get("name") != "sizeof"):
        return None
    if "argType" in node:
        return node["argType"].get("qualType")
    inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
    return inner[0].get("type", {}).get("qualType") if inner else None


def lengths(command, source, text, ast):
    """What every sizeof of a file is, by the name of its type. clang
    does not say it where it stands, so it is asked again, with each
    length as a constant of a name at the end of the file, and only for
    those. What it cannot say so, the length of a type that only a
    function knows, is not in the answer."""
    names = set()

    def walk(node):
        name = sized(node)
        if name and "(" not in name:
            names.add(name)
        for inner in node.get("inner", []):
            if isinstance(inner, dict):
                walk(inner)

    walk(ast)
    names = sorted(names)
    if not names:
        return {}
    asked = "".join(f"\nenum {{ int16_length_{number} = sizeof({name}) }};"
                    for number, name in enumerate(names))
    result = subprocess.run(
        [command, *OPTIONS, OFFSETOF, "-Xclang",
         "-ast-dump-filter=int16_length_", *hostcheck.OWN, "-I", str(source),
         "-"],
        input=(text + asked).encode("latin-1"), capture_output=True)
    sizes = {}
    said, at = result.stdout.decode("latin-1"), 0
    while True:
        at = said.find("{", at)
        if at < 0:
            break
        try:
            node, at = json.JSONDecoder().raw_decode(said, at)
        except ValueError:
            break
        found = re.fullmatch(r"int16_length_(\d+)", node.get("name", ""))

        def value(node):
            if "value" in node:
                return int(node["value"])
            for inner in node.get("inner", []):
                if isinstance(inner, dict) and value(inner) is not None:
                    return value(inner)
            return None

        if found and value(node) is not None:
            sizes[names[int(found.group(1))]] = value(node)
    return sizes


def read(command, source, name, text=None):
    """One file through clang: its places, as (file, line, kind,
    function), or None if clang's account of it is not to be read."""
    if text is None:
        text = (source / name).read_text(encoding="latin-1")
    _, text, _ = hostcheck.own(name, text)
    result = subprocess.run(
        [command, *OPTIONS, OFFSETOF, *hostcheck.OWN, "-I", str(source),
         "-"],
        input=text.encode("latin-1"), capture_output=True)
    refused = [line for line in result.stderr.decode("latin-1").splitlines()
               if re.search(r": (fatal )?error: ", line)]
    if refused or result.returncode:
        # What clang refuses is not in its account, and would be passed
        # over without a word.
        sys.exit(f"int16: clang does not take {name}:\n"
                 + "\n".join(refused[:10]))
    try:
        ast = json.loads(result.stdout)
    except ValueError:
        return None
    Lines().mark(ast)
    finder = Finder(name, text, lengths(command, source, text, ast))
    finder.value(ast)
    return sorted(finder.places)


def survey(command, source):
    """Every place of the tree, as (file, line, kind, function), and the
    names of the files that were read."""
    names = sorted(path.name for path in source.glob("*.c"))
    with ThreadPoolExecutor() as pool:
        results = list(pool.map(lambda name: read(command, source, name),
                                names))
    places = set()
    for name, found in zip(names, results):
        if found is None:
            sys.exit(f"int16: clang's account of {name} cannot be read")
        places |= set(found)            # a header's are found by every file
    return sorted(places), names


def compiler(asked):
    """The clang to ask, as (command, what it calls itself), or None and
    the reason why there is none."""
    command = shutil.which(asked)
    if not command:
        return None, f"there is no {asked} on this computer"
    said = subprocess.run([command, "--version"], capture_output=True,
                          text=True).stdout.splitlines()
    said = said[0] if said else asked
    if "clang" not in said.lower():
        return None, f"{asked} is another compiler here, {said}"
    return command, said


def check(command):
    """Hold the finder against the lines that were written for it: what
    is wrong, as lines to print."""
    text = CHECK.read_text()
    found = read(command, CHECK.parent, CHECK.name, text)
    if found is None:
        return ["clang's account of it cannot be read"]
    wrong = []
    for number, line in enumerate(text.split("\n"), 1):
        said = re.search(r"/\* finds: ([\w, ]*)\*/\s*$", line)
        wanted = sorted(said.group(1).replace(",", " ").split()) if said \
            else []
        got = sorted(kind for _, at, kind, _ in found if at == number)
        if got != wanted:
            wrong.append(f"{CHECK.name}:{number}: finds "
                         f"{', '.join(got) or 'nothing'}, where "
                         f"{', '.join(wanted) or 'nothing'} is to be "
                         f"found: {line.split('/*')[0].strip()}")
    return wrong


def keyed(places, source):
    """The places as the record knows them: (file, function, kind, the
    text of the line), each with the numbers of its lines."""
    lines = {}
    table = {}
    for file, number, kind, function in places:
        if file not in lines:
            lines[file] = (source / file).read_text(
                encoding="latin-1").split("\n")
        text = " ".join(lines[file][number - 1].split())
        table.setdefault((file, function, kind, text), []).append(number)
    return table


def recorded():
    """The record: of each place, how often it is there and the reason
    that is written for it."""
    table = {}
    if not RECORD.exists():
        return table
    _, _, body = RECORD.read_text().partition("\n\n")
    for entry in body.split("\n\n"):
        rows = entry.strip("\n").split("\n")
        found = re.fullmatch(r"(\S+), (\S+): (\w+)(?:, (\d+) times)?",
                             rows[0])
        if not found or len(rows) < 2 or not rows[1].startswith("    | "):
            sys.exit(f"int16: the record has an entry that cannot be "
                     f"read:\n{entry}")
        file, function, kind, times = found.groups()
        reason = "\n".join(row.strip() for row in rows[2:]).strip()
        table[(file, function.strip("()"), kind, rows[1][6:])] = (
            int(times or 1), reason)
    return table


def written(table, reasons, said):
    """The record, as text."""
    found = re.search(r"(\d+)\.\d+", said)
    lines = [
        "Where the portable C, src/portable_c/, can tell an int of 32 bits",
        "from one of 16, and why each place is the same in both builds all",
        "the same. harness/int16.py found the places, with clang "
        f"{found.group(1) if found else '?'}, and says",
        "what the kinds are; the reason under a place was written by whoever",
        "read it. The test portable_c-int16 holds the tree to this: a place",
        "that is not here is read, and gets the cast that says what DOS does",
        "or its reason. `python3 harness/int16.py --record` writes the places",
        "anew and keeps the reasons."]
    for key in sorted(table):
        file, function, kind, text = key
        times = len(table[key])
        lines += ["", f"{file}, {function or '()'}: {kind}"
                  + (f", {times} times" if times > 1 else ""),
                  f"    | {text}"]
        reason = reasons.get(key, (0, ""))[1]
        lines += [f"    {row}" for row in reason.split("\n") if row]
    return "\n".join(lines) + "\n"


def summary(table, names, said):
    files = {key[0] for key in table}
    lines = [f"{said}, for an int of 32 bits",
             f"{len(set(names) - files)} of {len(names)} files have no "
             f"place", "",
             f"{'kind':<10}{'places':>7}{'files':>7}  what is done with "
             f"the number"]
    total = 0
    for name, what in KINDS:
        number = sum(len(at) for key, at in table.items() if key[2] == name)
        where = len({key[0] for key in table if key[2] == name})
        total += number
        if number:
            lines.append(f"{name:<10}{number:>7}{where:>7}  {what}")
    lines.append(f"{'all':<10}{total:>7}{len(files):>7}")
    return "\n".join(lines)


def differences(table, before):
    """Where the tree is not as the record has it: lines to print."""
    lines = []
    for key in sorted(set(table) | set(before)):
        file, function, kind, text = key
        where = f"{file}, {function or '()'}: {kind}"
        if key not in before:
            at = ", ".join(str(number) for number in table[key])
            lines += [f"{where}: not in the record, at line {at}:",
                      f"    | {text}"]
        elif key not in table:
            lines += [f"{where}: in the record, and no place of the tree:",
                      f"    | {text}"]
        elif len(table[key]) != before[key][0]:
            lines += [f"{where}: {len(table[key])} times, where the record "
                      f"has {before[key][0]}:", f"    | {text}"]
        elif not before[key][1]:
            lines += [f"{where}: the record has no reason for it:",
                      f"    | {text}"]
    return lines


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--record", action="store_true",
                        help="write harness/int16.txt anew, with the "
                             "reasons that it has")
    parser.add_argument("--show", metavar="FILE|KIND",
                        help="list the places of a file or of a kind")
    parser.add_argument("--cc", metavar="CLANG", default="clang",
                        help="the clang to ask, if not the first one found")
    parser.add_argument("source", nargs="?", type=Path, default=SOURCE,
                        help="the tree, if not src/portable_c")
    arguments = parser.parse_args()

    command, said = compiler(arguments.cc)
    if not command:
        print(f"int16: skipped: {said}. It needs clang.")
        sys.exit(SKIPPED)

    wrong = check(command)
    if wrong:
        print(f"{said} does not find what the finder is known by:")
        print("\n".join(wrong))
        print("\nIf nothing of harness/int16.py or its lines has changed, "
              "this clang gives\nanother account of a file than the finder "
              "was written for: see its top.")
        sys.exit(1)

    places, names = survey(command, arguments.source)
    table = keyed(places, arguments.source)

    if arguments.show:
        for key in sorted(table, key=lambda key: (key[0], table[key])):
            file, function, kind, text = key
            if arguments.show in (file, kind):
                for number in table[key]:
                    print(f"{file}:{number}: {function or '()'}: {kind}: "
                          f"{text}")
        return
    print(summary(table, names, said))
    before = recorded()
    if arguments.record:
        RECORD.write_text(written(table, before, said))
        print(f"\nWritten to {RECORD.relative_to(ROOT)}.")
        return

    wrong = differences(table, before)
    if wrong:
        print("\nNot as harness/int16.txt has it:")
        print("\n".join(wrong))
        print("\nRead each place that is new: `--show FILE` has the lines. "
              "Where the number\ncan leave 16 bits, or stays in them only by "
              "what the code does not say,\nwrite the cast that says what DOS "
              "does. Where it cannot, write the record\nanew, `python3 "
              "harness/int16.py --record`, and the reason under the place.\n"
              "Of a `minus`, follow the number to where it is used: added "
              "to a pointer it\ngoes 64K less that much on in a flat memory, "
              "and wants to be added to a place,\na number, of which the "
              "pointer is then made.")
        sys.exit(1)


if __name__ == "__main__":
    main()
