#!/usr/bin/env python3
"""The places of the portable C where a variable of 16 bits is added to
a pointer as it is, for reading by eye.

harness/int16.py finds where a number that is worked out is added to a
pointer, and where a number written below 0 is kept in 16 bits. What it
cannot see is a variable of 16 bits that holds a number which has
wrapped round, FF60h for 160 back, and is added to a pointer: on DOS
the offset wraps round with it, and in a flat memory it goes 64K less
that much on (DIARY.md, "Move the rows of the editor's window by a place
of 16 bits, not by a pointer", which met one). No rule says of a
variable what it holds, so this only lists the places: every `pointer +
variable`, `pointer - variable`, `pointer += variable` and
`pointer[variable]` where the variable is a function's own or one that
it is handed, of 16 bits without a sign, and not one that a loop counts
with. With each, what the pointer is, as far as one word says it, and
the variable. A field, `text + line->at`, is not listed, nor a sum,
which int16.py has.

A place is right where the variable cannot hold a number that stands
for one below 0; where the pointer is the beginning of a segment, as
`TEXT(segment)` and a panel are, it is right whatever the variable
holds. The tree had 158 when this was written, and none that is wrong.

    python3 harness/trial32/added.py            the places
    python3 harness/trial32/added.py view.c     those of a file

It uses the finder of harness/int16.py for clang's account of a file,
and needs the clang that the finder needs.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(ROOT / "harness"))
import int16                            # noqa: E402

SOURCE = ROOT / "src" / "portable_c"


def bare(node):
    """An expression without its brackets and without the types that C
    makes of it by itself."""
    while (node.get("kind") in ("ParenExpr", "ImplicitCastExpr")
           and node.get("inner")):
        node = node["inner"][0]
    return node


class Added(int16.Finder):
    """The finder, noting besides its own places every variable of 16
    bits that is added to a pointer: the kind of such a place is how it
    is added, the pointer and the variable."""

    def note(self, pointer, number, node, how):
        number = bare(number)
        if number.get("kind") != "DeclRefExpr":
            return
        named = number.get("referencedDecl", {})
        if named.get("id") in self.counted:
            return
        if int16.type_of(number.get("type", {})) != (int16.U16, int16.U16):
            return
        pointer = bare(pointer)
        what = pointer.get("kind")
        if what == "DeclRefExpr":
            what = pointer.get("referencedDecl", {}).get("name", what)
        handed = " (handed)" if named.get("kind") == "ParmVarDecl" else ""
        used = node["_at"][0]
        file = self.name if used[0] == "<stdin>" else Path(used[0]).name
        self.places.add((file, used[1],
                         f"{what} {how} {named.get('name')}{handed}",
                         self.function))

    def value(self, node):
        kind = node.get("kind")
        inner = [x for x in node.get("inner", []) if isinstance(x, dict)]
        if kind == "ArraySubscriptExpr" and len(inner) == 2:
            self.note(inner[0], inner[1], node, "[]")
        elif (kind in ("BinaryOperator", "CompoundAssignOperator")
              and len(inner) == 2
              and node.get("opcode") in ("+", "-", "+=", "-=")):
            left, right = (int16.type_of(side.get("type", {}))
                           for side in inner)
            if left == int16.POINTER and right != int16.POINTER:
                self.note(inner[0], inner[1], node, node["opcode"])
            elif (right == int16.POINTER and left != int16.POINTER
                  and node["opcode"] == "+"):
                self.note(inner[1], inner[0], node, "+")
        return super().value(node)


def main():
    asked = set(sys.argv[1:])
    command, said = int16.compiler("clang")
    if not command:
        sys.exit(f"added: {said}")
    int16.Finder = Added
    places, _ = int16.survey(command, SOURCE)
    kinds = {name for name, _ in int16.KINDS}
    texts = {}
    count = 0
    for file, line, kind, function in places:
        if kind in kinds or asked and file not in asked:
            continue
        if file not in texts:
            texts[file] = (SOURCE / file).read_text(
                encoding="latin-1").split("\n")
        print(f"{file}:{line}: {function}: {kind}\n"
              f"    | {texts[file][line - 1].strip()}")
        count += 1
    print(f"\n{count} places")


if __name__ == "__main__":
    main()
