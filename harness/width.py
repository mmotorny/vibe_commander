#!/usr/bin/env python3
"""The numbers of 8 and 16 bits of the portable C, and why each is one.

Step 2 of the road has every number of the portable C, src/portable_c/,
be as wide as what it counts: a number of 16 bits becomes one of 32 or
a boolean, or stays as it is with its reason written down (TODO.md,
"The portable C in a flat memory: the plan", "The rule for a number").
This finds the numbers and where each is cut, and the test
`portable_c-width` of the build holds the tree to a record of them,
harness/width.txt, so that a number that is added later is sorted too,
and so that a commit which widens shows in the record what it widened.

What is found. clang reads every file as harness/int16.py has it read,
and this walks what it understood with the finder of that tool, which
knows of every expression between which two numbers it lies.

A number is something of 8 or 16 bits that the file declares, a u8 or a
u16, or an array of them: what a function returns, a parameter, a
variable, a field of a structure. Of a function it is counted where the
function is written, not where a header declares it. The structures of
the headers are found too, once, under the header's name.

A cut is where a number that may not fit is put into fewer bits, by
the types alone, a variable being taken to hold any number of its type
as int16.py takes it:

    cast      by a cast that is written, `(u16)(a + b)`
    store     kept in a variable or a field of 8 or 16 bits
    step      a variable of 8 or 16 bits stepped, `++`, `--`, `+=`,
              `-=`: all of them, but the variable that a loop counts with
    handed    handed to a parameter of 8 or 16 bits
    returned  returned as 8 or 16 bits

A cut is where a wider variable would keep another number, so the cuts
of a number are what is read before it is widened: `--show`. A
parameter that is widened no longer cuts what it is handed, and a
return that is widened is cut where a caller keeps it in 16 bits: both
show as cuts that go or come in the files of the callers, which the
record counts.

The record. A file, or a function or a structure of one, is in it in
one of four ways.

  - `not sorted`, with how many numbers and cuts it has: nobody has
    read it for the rule yet.
  - Sorted: every number that it still has of 8 or 16 bits, and every
    cut that is left, is an entry with its reason, written by whoever
    read it. A number that was widened or made a boolean is no entry:
    the table at the top of the record counts those, by file.
  - `laid out`, a structure whose layout is not the program's to
    change, with whose it is, once for all its fields.
  - `as a whole`, a file that no cluster of step 2 reads: WHOLE below
    says why.

The test fails where the tree is not as the record has it: a count that
has moved, fewer as well as more; a number or a cut of something sorted
that the record has not; one of the record that is gone; an entry
without a reason.

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
its entries or of its line `not sorted`.

What a reason is: for a number that is as wide as it is by what it is,
the word for that, `character`, `colour`, `key`, `cell`, `place`,
`handle`, with what is not plain behind it; for one that the program
counts on wrapping round, where it wraps; for a cut, what at the place
or just above it keeps the number within, as harness/int16.txt has it.

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

CUTS = ["cast", "store", "step", "handed", "returned"]
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
    ("N17", "keys.c commands.c"),
    ("N18", "menu.c help.c"),
    ("N19", "bars.c window.c system.c setup.c start.c"),
    ("N20", "stcolor.c stopts.c"),
    ("N21", "stmain.c sthelp.c"),
]

# Files that no cluster reads, each as a whole, with why. A file of
# tables, NAME_t.c, is one of them by its name.
TABLES = ("the tables: constants that are laid out as the original has "
          "them, which the test of the data holds byte for byte")
WHOLE = {
    "dos32.c": "the system's file: what DOS, the BIOS and the extender "
               "take and answer in, and the layer's to shape, step 3",
    "stack.c": "the stack, an array of bytes",
}

# The types that a number is widened to, by the name that vc.h and
# sys.h have for each, and the column of the table that counts it.
WIDE = {"u32": "u32", "bool": "bool", "_Bool": "bool", "i32": "signed",
        "FileSize": "u32", "FileStep": "signed", "ByteCount": "u32",
        "MemSize": "u32"}


def narrow(type_):
    """Of what clang says the type of a thing is: the type as it is
    written, if it is a number of 8 or 16 bits or an array of them."""
    def bare(name):
        name = " ".join(re.sub(r"\b(const|volatile)\b", "", name).split())
        return re.sub(r"\s*(\[\d*\])+$", "", name), name

    written = bare(type_.get("qualType", ""))
    real = bare(type_.get("desugaredQualType", "") or
                type_.get("qualType", ""))
    for element, _ in (written, real):
        if element in ("u8", "u16", "unsigned char", "unsigned short",
                       "char", "signed char", "short"):
            return written[1].replace(" [", "[")
    return None


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
        self.cuts.setdefault((used[1], kind, self.function),
                             node.get("_callee", ""))

    def convert(self, value, types, node):
        if types[1][1] in (8, 16) and not int16.within(
                value.low, value.high, types[1]):
            kind = "store"
            if node.get("kind") == "CStyleCastExpr":
                kind = "cast"
            elif node.get("opcode") in ("+=", "-="):
                kind = "step"
            self.cut(node, node.get("_cut") or kind)
        return super().convert(value, types, node)

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


class Tree:
    """What was found in every file."""

    def __init__(self, command, source):
        self.names = sorted(path.name for path in source.glob("*.c"))
        with ThreadPoolExecutor() as pool:
            finders = list(pool.map(
                lambda name: read(command, source, name), self.names))
        self.numbers = {}       # (file, scope, what, name, type): lines
        self.cuts = {}          # (file, scope, kind, text): lines
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
        self.files = [name for name in HEADERS
                      if any(key[0] == name for key in self.numbers)]
        self.files += self.names

    def scopes(self, file):
        found = {key[:2] for key in list(self.numbers) + list(self.cuts)
                 if key[0] == file}
        return sorted(found, key=lambda key: (self.first.get(key, 0), key))

    def count(self, file, scope=None):
        """(numbers, cuts) of a file, or of one scope of it."""
        def of(table):
            return sum(len(lines) for key, lines in table.items()
                       if key[0] == file and scope in (None, key[1]))
        return of(self.numbers), of(self.cuts)


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
            elif what.split(",")[0] in CUTS:
                if len(rows) < 2 or not rows[1].startswith("    | "):
                    sys.exit(f"width: the record has a cut without its "
                             f"line:\n{entry}")
                reason = "\n".join(row.strip() for row in rows[2:]).strip()
                self.reasons[(file, scope, what.split(",")[0],
                              rows[1][6:])] = reason
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


def plural(number, what):
    return f"{number} {what}{'' if number == 1 else 's'}"


def table(tree, record):
    """The counts, by cluster and by file."""
    def row(files):
        numbers = kept = unsorted = cuts = 0
        wider = {}
        for file in files:
            for column, count in tree.wider.get(file, {}).items():
                wider[column] = wider.get(column, 0) + count
            for scope in tree.scopes(file):
                n, c = tree.count(file, scope[1])
                numbers, cuts = numbers + n, cuts + c
                if record.sorted_(file, scope[1]):
                    kept += n
                elif not whole(file) and (file, scope[1]) not in record.laid:
                    unsorted += n
        return (f"{numbers:>8}{unsorted:>11}{kept:>6}{cuts:>6}"
                f"{wider.get('u32', 0):>6}{wider.get('bool', 0):>6}"
                f"{wider.get('signed', 0):>7}")

    head = (f"{'8 or 16':>8}{'not sorted':>11}{'kept':>6}{'cuts':>6}"
            f"{'u32':>6}{'bool':>6}{'signed':>7}")
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
        "The numbers of 8 and 16 bits of the portable C, src/portable_c/,",
        "and where a number is cut to so few: of what has been sorted by",
        "the rule of step 2, every one that is left with why it is as wide",
        "as it is; of the rest, how many. harness/width.py found them, with",
        f"clang {found.group(1) if found else '?'}, and says what the "
        "columns and the kinds are; the reason",
        "under an entry was written by whoever read it. The test",
        "portable_c-width holds the tree to this. `python3 harness/width.py",
        "--record` writes it anew and keeps the reasons; `--sort FILE`",
        "makes entries of a file that is not sorted.", ""]
    lines += table(tree, record)
    lines += ["", "----"]
    bare = []
    for file in tree.files:
        numbers, cuts = tree.count(file)
        counts = f"{plural(numbers, 'number')}, {plural(cuts, 'cut')}"
        if whole(file):
            lines += ["", f"{file}: as a whole, {counts}",
                      f"    {whole(file)}"]
            continue
        if file in record.files or file not in record.known:
            lines += ["", f"{file}: not sorted, {counts}"]
            continue
        for key in tree.scopes(file):
            scope = key[1]
            numbers, cuts = tree.count(file, scope)
            counts = f"{plural(numbers, 'number')}, {plural(cuts, 'cut')}"
            where = f"{file}, {scope or '()'}"
            if key in record.scopes:
                lines += ["", f"{where}: not sorted, {counts}"]
                continue
            if key in record.laid:
                lines += ["", f"{where}: laid out, "
                              f"{plural(numbers, 'number')}"]
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
            for entry, at in tree.cuts.items():
                if entry[:2] == key:
                    entries.append((min(at), entry[2], len(at), entry[3],
                                    entry))
            for _, head, times, text, entry in sorted(
                    entries, key=lambda entry: (entry[3] is not None,
                                                entry[0], entry[1])):
                lines += ["", f"{where}: {head}"
                          + (f", {times} times" if times > 1 else "")]
                if text is not None:
                    lines.append(f"    | {text}")
                reason = record.reasons.get(entry, "")
                lines += [f"    {row}" for row in reason.split("\n") if row]
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
    for key in tree.scopes(file):
        if scope and key[1] != scope:
            continue
        print(f"{file}, {key[1] or '()'}")
        for entry, at in tree.numbers.items():
            if entry[:2] == key:
                print(f"  {','.join(map(str, at)):>6}  {entry[2]} "
                      f"{entry[3]}, {entry[4]}")
        for entry, at in sorted(tree.cuts.items(), key=lambda x: x[1]):
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
            got = sorted(kind for at, kind, _ in finder.cuts if at == number)
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
                             "to a function")
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

    if now != before:
        print("\nNot as harness/width.txt has it, which is the lines with "
              "a minus:")
        print("\n".join(list(difflib.unified_diff(
            (before or "").split("\n"), now.split("\n"), "the record",
            "the tree", n=1, lineterm=""))[2:80]))
        print("\nA count that has moved wants the record written anew, "
              "`python3\nharness/width.py --record`: read what moved. A "
              "number or a cut that is new in\nsomething sorted wants its "
              "reason under it then, or to be widened: `--show\nFILE` has "
              "the lines, and the top of harness/width.py what a reason is.")
        sys.exit(1)
    if bare:
        print("\nThe record has no reason for:")
        print("\n".join(bare))
        sys.exit(1)


if __name__ == "__main__":
    main()
