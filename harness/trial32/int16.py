#!/usr/bin/env python3
"""Where the portable C may count on an int of 16 bits: a first finder.

On DOS an int has 16 bits, and two numbers of 16 bits, u16, are added,
taken from one another, multiplied and shifted as numbers of 16 bits
without a sign: the result wraps round. Where an int has 32 bits the
same two are made ints first, and the result is an int with a sign that
does not wrap. As long as it is put back into 16 bits nothing differs.
It differs where the int itself is looked at: compared, divided,
shifted right, made a number of 32 bits, or asked whether it is 0.

clang reads a file of src/portable_c/ for a machine whose int has 32
bits, as harness/hostcheck.py has gcc read it, and this walks what
clang understood, every expression with its type, for those places.
Nothing is compiled or run. It needs clang on this computer.

    python3 harness/trial32/int16.py panel.c read.c     with each line
    python3 harness/trial32/int16.py --all              the counts

A place is where the two builds can differ, not where they do: whether
the number ever leaves 16 bits there is for a reader to say. Where it
can, `(u16)(a - b) < c` is what DOS does, in both builds, and compiles
to the same code on DOS as without the cast.

What it leaves out: what is worked out when the file is compiled, of
numbers that are written and of sizeof; and the files of tables, which
have no code. What it cannot see: a number of 16 bits that has wrapped
round already, FFFFh for one back, and is then added to a pointer,
which on DOS wraps round with it and in a flat memory lands 64K
further on; and a far pointer that is stepped past the end of its
segment. It was made for the plan of the 32 bits (TODO.md) and has
been held against nineteen lines written for it, no more: whoever
makes a test of it reads it first.
"""
import json
import subprocess
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent
while not (ROOT / "harness" / "hostcheck.py").exists():
    ROOT = ROOT.parent
sys.path.insert(0, str(ROOT / "harness"))
import hostcheck                        # noqa: E402

SOURCE = ROOT / "src" / "portable_c"

NARROW = {"u8", "u16", "unsigned char", "unsigned short", "char",
          "signed char", "short"}
U16 = {"u16", "unsigned short"}
U8 = {"u8", "unsigned char", "char"}
SIGNED = {"int", "short", "signed char", "long"}
WRAPS = {"+", "-", "*", "<<"}
COMPARES = {"<", ">", "<=", ">=", "==", "!="}

KINDS = [
    ("compare", "the result is compared: below 0 or beyond 65535 in an "
                "int of 32 bits, wrapped round in one of 16"),
    ("divide", "it is divided, or a remainder taken of it"),
    ("shift", "it is shifted right"),
    ("widen", "it becomes a number of 32 bits, or an int that is kept"),
    ("truth", "it is asked whether it is 0"),
    ("address", "it is made a pointer: the host check counts the place "
                "too, as a number made a pointer"),
    ("index", "it is added to a pointer or is the index of an array"),
    ("signed", "a number of 16 bits compared with one that has a sign: "
               "without a sign on DOS, with one where an int has 32 bits"),
    ("byte", "bytes multiplied or shifted left in an int, which has a "
             "sign on DOS from 8000h on"),
]


def tree(name):
    text = (SOURCE / name).read_text(encoding="latin-1")
    _, text, _ = hostcheck.own(name, text)
    result = subprocess.run(
        ["clang", "--target=i386-pc-linux-gnu", "-x", "c", "-std=c99",
         "-fsyntax-only", "-funsigned-char", "-w", "-ferror-limit=0",
         *hostcheck.OWN, "-I", str(SOURCE), "-Xclang", "-ast-dump=json",
         "-"],
        input=f'#line 1 "{name}"\n{text}'.encode("latin-1"),
        capture_output=True)
    # A file that clang refuses in part, as the tables are for the
    # layout of a dialog, is read as far as it was understood.
    return json.loads(result.stdout)


class Lines:
    """clang says the line and the file of a place only where they are
    other than at the place before: so the tree is walked in the order
    it was written in."""

    def __init__(self):
        self.line = 0
        self.file = ""

    def at(self, loc):
        if "expansionLoc" in loc:
            self.at(loc["spellingLoc"])
            return self.at(loc["expansionLoc"])
        if "file" in loc:
            self.file = loc["file"]
        if "line" in loc:
            self.line = loc["line"]
        return self.file, self.line

    def mark(self, node):
        """Give every node of the tree its file and line."""
        if "loc" in node:
            self.at(node["loc"])
        if "range" in node:
            node["_at"] = self.at(node["range"]["begin"])
            self.at(node["range"]["end"])
        else:
            node["_at"] = (self.file, self.line)
        for inner in node.get("inner", []):
            if isinstance(inner, dict):
                self.mark(inner)


def kind_of(node):
    return node.get("type", {}).get("qualType", "")


def desugared(node):
    type_ = node.get("type", {})
    return type_.get("desugaredQualType", type_.get("qualType", ""))


def bare(node):
    while node.get("kind") in ("ParenExpr", "ConstantExpr") and node.get("inner"):
        node = node["inner"][0]
    return node


def promoted_from(node):
    """The type that an operand had before it was made an int, or its
    own."""
    node = bare(node)
    while (node.get("kind") == "ImplicitCastExpr"
           and node.get("castKind") in ("IntegralCast", "LValueToRValue",
                                        "NoOp")):
        inner = bare(node["inner"][0])
        if node.get("castKind") == "IntegralCast":
            return kind_of(inner).replace("const ", "").replace("volatile ", "")
        node = inner
    return kind_of(node).replace("const ", "").replace("volatile ", "")


def literal(node):
    node = bare(node)
    while node.get("kind") == "ImplicitCastExpr":
        node = bare(node["inner"][0])
    if node.get("kind") == "IntegerLiteral":
        return int(node["value"])
    if node.get("kind") == "CharacterLiteral":
        return int(node["value"])
    if (node.get("kind") == "UnaryExprOrTypeTraitExpr"):
        return None
    return None


def is_pointer(node):
    return desugared(node).rstrip().endswith("*")


def constant(node):
    """Is this worked out when the file is compiled? Then the compiler
    of each build has its own answer, and what is done with it says
    whether they differ: a count of bytes, sizeof, is the same."""
    kind = node.get("kind")
    if kind in ("IntegerLiteral", "CharacterLiteral",
                "UnaryExprOrTypeTraitExpr", "OffsetOfExpr"):
        return True
    if kind == "DeclRefExpr":
        return node.get("referencedDecl", {}).get("kind") == "EnumConstantDecl"
    if kind in ("ParenExpr", "ConstantExpr", "ImplicitCastExpr",
                "CStyleCastExpr", "BinaryOperator", "UnaryOperator",
                "ConditionalOperator"):
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
        return bool(inner) and all(constant(x) for x in inner)
    return False


def hot(node):
    """Is this an int whose bits from 16 up, or whose sign, an int of
    16 bits would not have? 'word' if numbers of 16 bits are in it,
    'byte' if only bytes are, None if it is not."""
    node = bare(node)
    kind = node.get("kind")
    type_ = desugared(node)
    if constant(node):
        return None
    if kind == "ImplicitCastExpr" and node.get("castKind") in (
            "LValueToRValue", "NoOp"):
        return None
    if type_ not in ("int", "unsigned int", "long", "unsigned long"):
        return None
    if kind_of(node) in ("u32",):
        return None
    if kind == "BinaryOperator":
        op = node["opcode"]
        left, right = node["inner"]
        if op == "-" and is_pointer(left) and is_pointer(right):
            return "word"
        if op in WRAPS or op in ("|", "^", "&"):
            if any(kind_of(bare(x)) == "u32" or promoted_from(x) == "u32"
                   for x in (left, right)):
                return None
            heat = [hot(left), hot(right)]
            if op == "&":
                # a mask of 16 bits or fewer cuts the rest off
                for x in (left, right):
                    value = literal(x)
                    if value is not None and 0 <= value <= 0xFFFF:
                        return None
                    if promoted_from(x) in NARROW and not hot(x):
                        return None
            if "word" in heat:
                return "word"
            if op in WRAPS:
                froms = [promoted_from(left), promoted_from(right)]
                if op == "<<":
                    froms = froms[:1]
                if any(f in U16 for f in froms):
                    return "word"
                if any("sizeof" == bare(x).get("name") or
                       bare(x).get("kind") == "UnaryExprOrTypeTraitExpr"
                       for x in (left, right)) and type_ == "unsigned int":
                    return "word"
                if op in ("*", "<<") and any(f in U8 for f in froms):
                    value = literal(right)
                    if op == "<<" and value is not None and value < 8:
                        return heat[0]
                    if op == "*" and value is not None and value < 128:
                        return heat[0]
                    return "byte"
            if "byte" in heat:
                return "byte"
        if op == ",":
            return hot(right)
        return None
    if kind == "UnaryOperator" and node["opcode"] in ("-", "~"):
        inner = node["inner"][0]
        if promoted_from(inner) in U16:
            return "word"
        return hot(inner)
    if kind == "ConditionalOperator":
        heat = [hot(x) for x in node["inner"][1:]]
        return "word" if "word" in heat else ("byte" if "byte" in heat
                                              else None)
    return None


def maybe_negative(node):
    """An operand of a comparison that has a sign and may be below 0."""
    value = literal(node)
    node = bare(node)
    if value is not None:
        return False
    if node.get("kind") == "UnaryOperator" and node["opcode"] == "-":
        return literal(node["inner"][0]) is not None
    if (node.get("kind") == "DeclRefExpr" and node.get(
            "referencedDecl", {}).get("kind") == "EnumConstantDecl"):
        return False                    # the program's are all from 0 up
    if node.get("kind") == "BinaryOperator" and node["opcode"] in (
            COMPARES | {"&&", "||"}):
        return False                    # 0 or 1
    return promoted_from(node) in SIGNED and not hot(node) and (
        node.get("kind") in ("ImplicitCastExpr", "CStyleCastExpr",
                             "DeclRefExpr"))


def find(node, name, found, parent=None, role=None):
    kind = node.get("kind")
    inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
    here = node.get("_at", ("", 0))

    def report(what, heat="word"):
        if here[0] in ("<stdin>", ""):
            found.append((here[1] - 1, "byte" if heat == "byte" and what in (
                "compare", "divide", "shift", "widen", "address") else what))

    def seen(child, what):
        heat = hot(child)
        if heat:
            report(what, heat)

    if kind == "BinaryOperator":
        op = node["opcode"]
        left, right = inner
        if op in COMPARES:
            for child in (left, right):
                seen(child, "compare")
            if not hot(left) and not hot(right):
                froms = [promoted_from(left), promoted_from(right)]
                if (froms[0] in U16 and maybe_negative(right)
                        or froms[1] in U16 and maybe_negative(left)):
                    report("signed")
        elif op in ("/", "%", "/=", "%="):
            for child in (left, right):
                seen(child, "divide")
        elif op in (">>", ">>="):
            seen(left, "shift")
        elif op in ("&&", "||"):
            for child in (left, right):
                seen(child, "truth")
        elif op in WRAPS and (is_pointer(left) or is_pointer(right)) and not (
                is_pointer(left) and is_pointer(right)):
            for child in (left, right):
                seen(child, "index")
        elif op in WRAPS or op in ("|", "^", "&"):
            if any(kind_of(bare(x)) == "u32" or promoted_from(x) == "u32"
                   for x in (left, right)):
                for child in (left, right):
                    seen(child, "widen")
        elif op in ("=", "+=", "-=", "|=", "^=", "&=", "*=", "<<="):
            if desugared(left) in ("unsigned int", "int", "long",
                                   "unsigned long") or is_pointer(left):
                seen(right, "index" if is_pointer(left) else "widen")
    elif kind in ("ImplicitCastExpr", "CStyleCastExpr"):
        to = desugared(node)
        if node.get("castKind") == "IntegralCast" and to in (
                "unsigned int", "long", "unsigned long", "long long",
                "unsigned long long"):
            seen(inner[0], "widen")
        elif node.get("castKind") == "IntegralToPointer":
            seen(inner[0], "address")
    elif kind == "ArraySubscriptExpr":
        seen(inner[1], "index")
    elif kind == "UnaryOperator" and node.get("opcode") == "!":
        seen(inner[0], "truth")
    elif kind in ("IfStmt", "WhileStmt", "ConditionalOperator"):
        seen(inner[0], "truth")
    elif kind == "DoStmt":
        seen(inner[1], "truth")
    elif kind == "ForStmt":
        if len(inner) > 2 and inner[2].get("kind"):
            seen(inner[2], "truth")
    elif kind == "SwitchStmt":
        seen(inner[0], "compare")
    elif kind == "ReturnStmt" and inner:
        seen(inner[0], "widen")
    elif kind == "VarDecl" and inner and desugared(node) in (
            "int", "unsigned int", "long", "unsigned long"):
        seen(inner[-1], "widen")
    elif kind == "CallExpr":
        for child in inner[1:]:
            seen(child, "widen")
    for child in inner:
        find(child, name, found, node)


def main(names):
    if names == ["--all"]:
        names = sorted(p.name for p in SOURCE.glob("*.c")
                       if not p.name.endswith("_t.c"))
    total = Counter()
    for name in names:
        ast = tree(name)
        Lines().mark(ast)
        found = []
        find(ast, name, found)
        found = sorted(set(found))
        lines = (SOURCE / name).read_text(encoding="latin-1").split("\n")
        counts = Counter(kind for _, kind in found)
        total += counts
        if len(names) <= 4:
            for line, kind in found:
                print(f"{name}:{line}: {kind}: {lines[line - 1].strip()}")
        print(f"{name:<14}" + ", ".join(
            f"{kind} {counts[kind]}" for kind, _ in KINDS if counts[kind])
            or f"{name:<14}none")
    print(f"{'all':<14}" + ", ".join(
        f"{kind} {total[kind]}" for kind, _ in KINDS if total[kind]),
        "=", sum(total.values()))


if __name__ == "__main__":
    main(sys.argv[1:])
