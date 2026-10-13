#!/usr/bin/env python3
"""Hold the source of the portable C against a compiler of this computer.

The portable C, src/portable_c/, is built by Open Watcom for DOS and
judged there. How far its source is from a program that another
compiler would take, for a machine whose memory is not in segments, no
build for DOS says. This does: it has gcc read every file of the tree
for a machine of 64 bits, as strict C of 1999, and counts what gcc
refuses. Nothing is compiled, linked or run; it takes a second or two.

What is counted, a place at a time, each of a kind (KINDS below):

  - What gcc says of a file, error or warning alike, with the options of
    OPTIONS and no others: a pointer cast to a number that does not hold
    one, a number made a pointer, a structure that is not laid out as
    DOS has it.
  - What only Open Watcom reads, which is counted here, by the word, in
    the source without its comments and its strings: an interrupt
    function, `__loadds`, `__declspec(naked)`, a block of `_asm`, and a
    `#pragma` that is this compiler's. Each is then taken out of gcc's
    sight, so that it can read the rest of the file: the three words by
    a macro that makes nothing of them (OWN below), an `_asm` block by
    blanks, and such a `#pragma` gcc passes over by itself. Nothing
    else is hidden.

The words of Open Watcom that say what segment a thing is in, `__far`,
CODE and STACK, and FAR, which is `__far` for the pointers of sys.h, are
no errors: vc.h takes them for nothing where the
memory has no segments, as a build for such a machine will. The summary
says how many there are.

A place is counted for the file that it is written in, and what a macro
makes is written where the macro is used: DOS_AT of vc.h makes a
pointer of a number in every file that names it, and is counted in
each. What is wrong in a header itself is counted once, for the header,
however many files read it. A file with no place of any kind is clean.

The words of the addresses in vc.h, AT, OFFSET, PANEL, MAIN_SEG, NEAR,
MD_PLACE, MD_POINTER and their like, have a body for a memory without
segments, and that is the one gcc reads here: it casts nothing. So a
segment and an offset that go through those words are not counted, and
what is counted of the two casts is what does not go through them: a
cast that is written out, a macro of a file, and what is DOS's. That
the second bodies compile says nothing of whether a place is right
there: a word of a table read as a number of 16 bits through AT is not,
and gcc has nothing to say of it.

What was counted last is kept in harness/hostcheck.txt, of every file,
and the test `portable_c-host` of the build fails when a file has
another count of any kind than is written there: when a clean file is
clean no more, when a file has more of a kind, and also when it has
fewer or has become clean, so that the record is always what is so.
Then look at what the test names, and if it is as meant, write the
record anew and commit it with the change:

    python3 harness/hostcheck.py --record

What has changed in the record says what the change did to the tree,
which is what it is kept for.

    python3 harness/hostcheck.py --show copy.c
    python3 harness/hostcheck.py --show to-number

lists the places of a file or of a kind, each with its line of the
source, and

    python3 harness/hostcheck.py --macros

says which macros make the casts, and how many each, and how many are
written out in no macro: whoever takes one of those away can see what
is left of it.

It needs gcc, and one that compiles for a machine whose pointers have 64
bits. The counts are gcc's: clang finds the same things and counts some
of them otherwise, more numbers made pointers and no address of a
function kept as that of data, so where `gcc` is not there, or is
another compiler by that name, the test is skipped and holds nothing. gcc 13 and gcc 15
say the same of the tree, place for place. `--cc gcc-13` asks another
gcc than the first one found. The record names the version that wrote
it; should another count differently with nothing changed in the
source, the message that it differs in wants a kind of its own in SAYS.

What it cannot see, since it only reads: arithmetic that counts on an
int of 16 bits, where the sum of two numbers of 16 bits wraps round on
DOS and does not where an int has 32, which harness/int16.py finds, the
test `portable_c-int16`; a number that is written to the
memory at an address that is not even; a pointer made of a number that
is written out, an address of the BIOS's data, of which gcc says
nothing; and anything that is right C and means something else without
DOS under it. TODO.md has these with the kinds.

Exits with status 0 if every file is as the record has it, 1 if not,
and 77 if there is no gcc to ask, which the build takes for a test that
was skipped.
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c"
RECORD = Path(__file__).resolve().parent / "hostcheck.txt"
SKIPPED = 77

# How gcc is asked: the language of 1999 and nothing of its own beside
# it; every error, not the first few; and what a macro makes named
# where the macro is used, not where it is defined. No option adds a
# warning or takes one away: what gcc says by itself in this mode is what
# counts.
OPTIONS = ["-x", "c", "-fsyntax-only", "-std=c99", "-pedantic",
           "-fmax-errors=0", "-fno-diagnostics-show-caret",
           "-fdiagnostics-color=never"]
AT_ITS_USE = "-ftrack-macro-expansion=0"

# What Open Watcom alone reads and the code is not to be read without:
# counted here by the word, and made nothing of for gcc.
OWN = ["-D__interrupt=", "-D__loadds=", "-D__declspec(x)="]

# The kinds, in the order of the summary: the name that the record has
# for each, and what it is. TODO.md says what each waits for.
KINDS = [
    ("to-number", "a pointer cast to a number of 16 or 32 bits"),
    ("to-pointer", "a number of 16 or 32 bits made a pointer"),
    ("layout", "a structure that is not as long as on DOS"),
    ("inconstant", "a length that is no constant"),
    ("function", "a pointer to a function kept as one to data, or back"),
    ("undeclared", "a function that nothing declares or defines"),
    ("interrupt", "__interrupt: a function that an interrupt enters"),
    ("loadds", "__loadds: a function that loads DS when it is entered"),
    ("naked", "__declspec(naked): a function that is its assembly"),
    ("asm", "a block of _asm"),
    ("asm-pragma", "a #pragma aux that is instructions"),
    ("registers", "a #pragma aux with registers, or a name for the linker"),
    ("intrinsic", "#pragma intrinsic"),
    ("data_seg", "#pragma data_seg: a segment for what follows"),
    ("aborts", "a #pragma aux that says a function does not return"),
    ("other", "anything else that gcc says: it wants a kind in SAYS"),
]
NAMES = [name for name, _ in KINDS]

# What gcc says, by kind: the first that is found in a message.
SAYS = [
    (r"\[-Wpointer-to-int-cast\]", "to-number"),
    (r"\[-Wint-to-pointer-cast\]", "to-pointer"),
    (r"size of array '\w+' is negative", "layout"),
    (r"variably modified '\w+' at file scope", "inconstant"),
    (r"forbids conversion of (function|object) pointer to "
     r"(object|function) pointer", "function"),
    (r"'\w+' used but never defined", "undeclared"),
    (r"\[-Wimplicit-function-declaration\]", "undeclared"),
]

# The kinds that macros make most of: see --macros.
CASTS = ["to-number", "to-pointer", "function"]

# The words that vc.h takes for nothing on a machine without segments.
# FAR is its own, for the pointers of sys.h, and is __far where there
# are segments.
SEGMENT_WORDS = ["__far", "FAR", "STACK", "CODE"]


def bare(text, strings):
    """`text` with its comments blanked, and its strings and characters
    too if `strings`: every line stays where it is, and every column."""
    out = []
    i, n = 0, len(text)
    while i < n:
        if text.startswith("/*", i) or text.startswith("//", i):
            if text[i + 1] == "*":
                end = text.find("*/", i + 2)
                end = n if end < 0 else end + 2
            else:
                end = text.find("\n", i)
                end = n if end < 0 else end
            out.append(re.sub(r"[^\n]", " ", text[i:end]))
            i = end
        elif text[i] in "\"'":
            end = i + 1
            while end < n and text[end] != text[i] and text[end] != "\n":
                end += 2 if text[end] == "\\" else 1
            end = min(end + 1, n)
            piece = text[i:end]
            out.append(re.sub(r"[^\n]", " ", piece) if strings else piece)
            i = end
        else:
            out.append(text[i])
            i += 1
    return "".join(out)


def own(name, text):
    """What Open Watcom alone reads in a file: the places, as (file,
    line, kind, ""), where gcc's have what gcc says; `text` with its
    `_asm` blocks blanked, for gcc; and the names of the functions that
    are a #pragma aux of instructions, which gcc will find used and not
    defined."""
    places = []
    code = bare(text, True)

    def line_of(position):
        return text.count("\n", 0, position) + 1

    for word, kind in ((r"\b__interrupt\b", "interrupt"),
                       (r"\b__loadds\b", "loadds"),
                       (r"\b__declspec\s*\(\s*naked\s*\)", "naked")):
        for found in re.finditer(word, code):
            places.append((name, line_of(found.start()), kind, ""))

    for found in re.finditer(r"\b_asm\b\s*\{", code):
        end = code.find("}", found.end())
        end = len(code) if end < 0 else end + 1
        places.append((name, line_of(found.start()), "asm", ""))
        text = (text[:found.start()]
                + re.sub(r"[^\n]", " ", text[found.start():end])
                + text[end:])

    inline = set()
    lines = bare(text, False).split("\n")
    number = 0
    while number < len(lines):
        first = number
        whole = lines[number]
        while whole.rstrip().endswith("\\") and number + 1 < len(lines):
            number += 1
            whole = whole.rstrip()[:-1] + " " + lines[number]
        number += 1
        found = re.match(r"\s*#\s*pragma\s+(\w+)\s*(.*)", whole)
        if not found or found.group(1) == "pack":
            continue                    # pack is every compiler's
        pragma, rest = found.groups()
        kind = "other"
        if pragma == "aux":
            function = re.match(r"\w*", rest).group()
            said = rest[len(function):].strip()
            if said.startswith("="):
                kind = "asm-pragma"
                inline.add(function)
            elif said == "aborts":
                kind = "aborts"
            else:
                kind = "registers"
        elif pragma in ("intrinsic", "data_seg"):
            kind = pragma
        places.append((name, first + 1, kind, ""))
    return places, text, inline


def compiler(asked):
    """The compiler to ask, as (command, what it calls itself), or None
    and the reason why there is none."""
    command = shutil.which(asked)
    if not command:
        return None, f"there is no {asked} on this computer"
    said = subprocess.run([command, "--version"], capture_output=True,
                          text=True).stdout.splitlines()
    said = said[0] if said else asked
    if "clang" in said.lower():         # as `gcc` is on some computers
        return None, f"{asked} is another compiler here, {said}"
    probe = subprocess.run(
        [command, "-x", "c", "-fsyntax-only", "-"], capture_output=True,
        input=b"typedef char wide[sizeof(void *) == 8 ? 1 : -1];\n")
    if probe.returncode:
        return None, (f"{asked} does not compile for a machine whose "
                      f"pointers have 64 bits")
    return command, said


def version(said):
    """The number of a gcc's version, its first part, from what the
    compiler calls itself."""
    found = re.search(r"(\d+)\.\d+", said.split(")")[-1])
    return found.group(1) if found else "?"


def read(command, source, path, at_its_use=True):
    """One file through the compiler: the places in it, as (file, line,
    kind, message), and those in what it includes, apart. What a macro
    makes is a place where the macro is used, or if not `at_its_use`,
    where it is defined."""
    name = path.name
    text = path.read_text(encoding="latin-1")
    places, text, inline = own(name, text)
    result = subprocess.run(
        [command, *OPTIONS, *([AT_ITS_USE] if at_its_use else []), *OWN,
         "-I", str(source), "-"],
        input=f'#line 1 "{name}"\n{text}'.encode("latin-1"),
        capture_output=True, env=dict(os.environ, LC_ALL="C"))
    included = []
    for line in result.stderr.decode("latin-1").splitlines():
        found = re.match(r"(.+?):(\d+):(?:(\d+):)? (?:fatal )?"
                         r"(error|warning): (.*)", line)
        if not found:
            continue
        where, number, column, _, message = found.groups()
        where = Path(where).name
        kind = "other"
        for pattern, of_kind in SAYS:
            if re.search(pattern, message):
                kind = of_kind
                break
        used = re.match(r"'(\w+)' used but never defined", message)
        if used and used.group(1) in inline:
            continue                    # counted as its #pragma aux
        place = (where, int(number), kind, message)
        if where == name:
            places.append(place)
        else:
            included.append(place + (column,))
    return places, included


def survey(command, source):
    """Every place in the tree, as (file, line, kind, message), and the
    names of its files."""
    files = sorted(source.glob("*.c"))
    headers = sorted(source.glob("*.h"))
    with ThreadPoolExecutor() as pool:
        results = list(pool.map(lambda path: read(command, source, path),
                                files))
    places = []
    included = set()
    for own_places, others in results:
        places += own_places
        included |= set(others)
    places += [place[:4] for place in included]
    for header in headers:
        # A header is read by gcc as part of the files that include it;
        # what only Open Watcom reads in it is counted here, once.
        places += own(header.name,
                      header.read_text(encoding="latin-1"))[0]
    return sorted(places), [path.name for path in files + headers]


def makers(command, source, kinds):
    """Of the places of some kinds, how many each macro makes, by its
    name, and how many are written out, under the name "": gcc is asked
    again, for where the cast is written and not where it ends up."""
    files = sorted(source.glob("*.c"))
    with ThreadPoolExecutor() as pool:
        results = list(pool.map(
            lambda path: read(command, source, path, False), files))
    lines = {path.name: path.read_text(encoding="latin-1").split("\n")
             for path in source.glob("*.[ch]")}
    made = {kind: Counter() for kind in kinds}
    for own_places, others in results:
        for where, number, kind, _ in own_places + [o[:4] for o in others]:
            if kind not in made:
                continue
            at = number - 1             # the line that begins the #define
            while at > 0 and lines[where][at - 1].rstrip().endswith("\\"):
                at -= 1
            macro = re.match(r"\s*#\s*define\s+(\w+)", lines[where][at])
            made[kind][macro.group(1) if macro else ""] += 1
    return made


def counts(places, names):
    table = {name: Counter() for name in names}
    for where, _, kind, _ in places:
        table.setdefault(where, Counter())[kind] += 1
    return table


def written(table, said):
    """The record, as text."""
    width = max(len(name) for name in table) + 2
    lines = [
        "What gcc says of the portable C, src/portable_c/, and what only Open",
        "Watcom reads there: of each file, how many places of each kind, or",
        "that it is clean. harness/hostcheck.py counted it, with gcc "
        f"{version(said)}, and",
        "says what the kinds are. Not to be edited: the test portable_c-host",
        "holds the tree to it, and `python3 harness/hostcheck.py --record`",
        "writes it anew.",
        ""]
    for name in sorted(table):
        kinds = table[name]
        what = ", ".join(f"{kind} {kinds[kind]}" for kind in NAMES
                         if kinds[kind]) or "clean"
        lines.append(f"{name:<{width}}{what}")
    return "\n".join(lines) + "\n"


def recorded():
    """The record: the counts of every file, and the version of gcc
    that it was written with."""
    table = {}
    with_gcc = "?"
    if not RECORD.exists():
        return table, with_gcc
    head, _, body = RECORD.read_text().partition("\n\n")
    found = re.search(r"with gcc (\d+|\?)", head)
    if found:
        with_gcc = found.group(1)
    for line in body.splitlines():
        name, _, what = line.partition(" ")
        kinds = Counter()
        if what.strip() != "clean":
            for item in what.split(","):
                kind, number = item.split()
                kinds[kind] = int(number)
        table[name] = kinds
    return table, with_gcc


def segment_words(source):
    """How often each of SEGMENT_WORDS stands in the tree `source`, in
    the code, where it is not being defined. tools/figures.py writes
    these into the README's table."""
    words = Counter()
    for path in sorted(source.glob("*.[ch]")):
        code = bare(path.read_text(encoding="latin-1"), True)
        for word in SEGMENT_WORDS:
            words[word] += (len(re.findall(rf"\b{word}\b", code))
                            - len(re.findall(rf"#define {word}\b", code)))
    return words


def summary(table, source, said):
    """What is printed of a run."""
    clean = sum(1 for kinds in table.values() if not kinds)
    lines = [f"{said}, as {' '.join(OPTIONS[3:5])}",
             f"{clean} of {len(table)} files are clean", ""]
    lines.append(f"{'kind':<12}{'places':>7}{'files':>7}  what it is")
    total = 0
    for name, what in KINDS:
        number = sum(kinds[name] for kinds in table.values())
        files = sum(1 for kinds in table.values() if kinds[name])
        total += number
        if number:
            lines.append(f"{name:<12}{number:>7}{files:>7}  {what}")
    dirty = len(table) - clean
    lines.append(f"{'all':<12}{total:>7}{dirty:>7}")
    words = segment_words(source)
    lines += ["", "Taken for nothing where the memory has no segments: "
              + ", ".join(f"{word} in {words[word]} places"
                          for word in SEGMENT_WORDS)]
    return "\n".join(lines)


def differences(table, before):
    """Where the tree is not as the record has it: lines to print."""
    lines = []
    for name in sorted(set(table) | set(before)):
        if name not in before:
            lines.append(f"{name}: not in the record")
            continue
        if name not in table:
            lines.append(f"{name}: in the record, and no file of the tree")
            continue
        now, then = table[name], before[name]
        if now == then:
            continue
        if not then:
            lines.append(f"{name}: was clean, and is not:")
        elif not now:
            lines.append(f"{name}: has become clean")
            continue
        else:
            lines.append(f"{name}:")
        for kind in NAMES:
            if now[kind] != then[kind]:
                way = "more" if now[kind] > then[kind] else "fewer"
                lines.append(f"    {kind}: {now[kind]}, where the record "
                             f"has {then[kind]}: {way}")
    return lines


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--record", action="store_true",
                        help="write harness/hostcheck.txt anew")
    parser.add_argument("--show", metavar="FILE|KIND",
                        help="list the places of a file or of a kind")
    parser.add_argument("--macros", action="store_true",
                        help="say which macros make the casts, and how many")
    parser.add_argument("--cc", metavar="GCC", default="gcc",
                        help="the gcc to ask, if not the first one found")
    parser.add_argument("source", nargs="?", type=Path, default=SOURCE,
                        help="the tree, if not src/portable_c")
    arguments = parser.parse_args()

    command, said = compiler(arguments.cc)
    if not command:
        print(f"hostcheck: skipped: {said}. It needs gcc, for a machine "
              f"of 64 bits.")
        sys.exit(SKIPPED)

    places, names = survey(command, arguments.source)
    table = counts(places, names)

    if arguments.show:
        lines = {}
        for where, number, kind, message in places:
            if arguments.show not in (where, kind):
                continue
            if where not in lines:
                lines[where] = (arguments.source / where).read_text(
                    encoding="latin-1").split("\n")
            print(f"{where}:{number}: {kind}: "
                  f"{lines[where][number - 1].strip()}")
            if message and kind not in ("to-number", "to-pointer"):
                print(f"    {message}")     # gcc says more than the kind
        return
    if arguments.macros:
        made = makers(command, arguments.source, CASTS)
        for kind in CASTS:
            number = sum(kinds[kind] for kinds in table.values())
            print(f"{kind}, {number} places: by the macro that makes them")
            for macro, count in made[kind].most_common():
                print(f"{count:>7}  {macro or 'written out, in no macro'}")
            if sum(made[kind].values()) != number:
                print("         which do not add up: a macro makes some "
                      "of these in a header itself")
        return
    print(summary(table, arguments.source, said))
    if arguments.record:
        RECORD.write_text(written(table, said))
        print(f"\nWritten to {RECORD.relative_to(ROOT)}.")
        return

    before, with_gcc = recorded()
    wrong = differences(table, before)
    if wrong:
        print("\nNot as harness/hostcheck.txt has it:")
        print("\n".join(wrong))
        print("\nIf that is what the change at hand was to do, write the "
              "record anew,\n`python3 harness/hostcheck.py --record`, and "
              "commit it with the change.\n`--show FILE` lists the places "
              "of a file.")
        if with_gcc != version(said):
            print(f"The record was written with gcc {with_gcc}, and this "
                  f"is gcc {version(said)}: if nothing\nof the source has "
                  f"changed, the two count differently. See the top of\n"
                  f"harness/hostcheck.py.")
        sys.exit(1)


if __name__ == "__main__":
    main()
