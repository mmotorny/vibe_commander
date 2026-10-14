"""The words of the addresses in the portable C, counted file by file
and held to a record.

    python3 harness/words.py            hold the tree to the record
    python3 harness/words.py --record   write the record anew
    python3 harness/words.py --show WORD|FILE
                                        the lines that were counted

The portable C says where a thing is in the memory through a few words
of vc.h, which stand for what a segment and an offset were on DOS: `AT`,
`PANEL`, `OFFSET`, `MD_PLACE` and the others of WORDS below. Step 2 of
the road takes them out, piece by piece, as a segment becomes a pointer
(TODO.md, "The portable C in a flat memory: the plan"). This is the
measure of that work: how often each word is written in each file of
`src/portable_c`, in `harness/words.txt`. The test `portable_c-words`
fails for a file that has another count than the record, fewer as well
as more, so that a commit which changes a count carries the record's
lines that say so: write the record anew with `--record`, read what its
diff says, and commit it with the change. A count may go up: the pieces
that come first write `SEG_OF` and `AT` where they meet what another
piece still has as a segment, and the last piece takes those out.

What is counted is the text, as hostcheck.py's words of the segments
are: a word as a whole word, outside comments, strings and characters,
and not where it is being defined, `#define WORD`. Three kinds are
written with more than a name:

  - `md.NAME`, a word of the main data that holds a segment or a place
    of the main data, where code reads or writes it. Its declaration in
    `struct MainDat` is not counted, and stays when the word has left:
    the main data keeps the original's layout;
  - `u16 panel` and `u16 segment`, a segment kept in a number, as a
    parameter, a variable or a member is declared. A segment that is
    kept under another name is not found by this, nor is one in a
    `u16` that is declared with others on one line;
  - `Name(`, a function that makes or takes an address in the old way.

The table that a run prints says for every word how many places and
files have it, which file the most, and which piece of the plan is to
have it at nothing, or that it stays. The record does not hold a piece
to that: it shows what went, and the table says what is left to go.
"""
import argparse
import re
import sys
from collections import Counter
from pathlib import Path

from hostcheck import bare

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c"
RECORD = Path(__file__).resolve().parent / "words.txt"

# Each word: its name in the record, the pattern that finds it, the
# piece that is to have it at nothing, or None for one that stays, and
# what the plan says of it.
PLAIN = r"\b{}\b"
OF_MAIN = r"\bmd\s*\.\s*{}\b"
CALLED = r"\b{}\s*\("


def _word(name, by, what, pattern=PLAIN, text=None):
    return name, re.compile(pattern.format(text or name)), by, what


WORDS = [
    # What rests on the megabyte: gone when F10 takes it down.
    _word("ARENA", "F10", "where the megabyte begins that segments count "
                         "in"),
    _word("AT", "F10", "a segment and an offset made a pointer: F3 for "
                      "the tables and a dialog's fields, F4 to F6 for the "
                      "texts"),
    _word("SEG_OF", "F10", "a pointer made a segment: the bridge between "
                          "two pieces, up first"),
    _word("OFFSET_IN_SEG", "F10", "a pointer made an offset in a segment: "
                                 "the bridge, too"),
    _word("MAIN_SEG", "F10", "the main data as a segment: what a dialog "
                            "is handed is &md since F3, and what is left "
                            "is what a search looks through, the text of "
                            "F4 and F5 and the list of F7"),
    _word("PANEL", "F2", "a segment made a panel"),
    _word("u16 panel", "F2", "a panel kept as a segment",
          text=r"u16\s+panel"),
    _word("md.WCBcrn", "F2", "the current panel, a segment in the main "
                             "data", OF_MAIN, "WCBcrn"),
    _word("md.WCBsec", "F2", "the other panel", OF_MAIN, "WCBsec"),
    _word("md.WCB1seg", "F2", "the left panel", OF_MAIN, "WCB1seg"),
    _word("md.WCB2seg", "F2", "the right panel", OF_MAIN, "WCB2seg"),
    _word("md.ViewSeg", "F1", "where the memory behind the panels "
                              "begins", OF_MAIN, "ViewSeg"),
    _word("md.MemSeg", "F1", "the block that the panels are in", OF_MAIN,
          "MemSeg"),
    _word("md.FreMem2", "F1", "how far it is to the end of that block, "
                              "in paragraphs", OF_MAIN, "FreMem2"),
    _word("PANEL_PARAS", "F1", "a panel's length in paragraphs"),
    _word("VIEW_PARAS", "F1", "the room of the viewer in paragraphs"),
    # What F1 put in their place, which counts in paragraphs of 16
    # bits: gone when the memory is the extender's and counted in bytes.
    _word("RoomParas", "F9", "the room's length in paragraphs: each of "
                             "the five clusters that count the room "
                             "moves its calls to bytes", CALLED),
    _word("PANEL_BLOCK", "F9", "a panel's block in paragraphs"),
    _word("ROOM_LEAST", "F9", "the least of the room in paragraphs"),
    _word("OFFSET", "F3", "a table as an offset in the tables' segment"),
    _word("TABLES_BY", "F3", "the tables' segment"),
    _word("AsmSeg", "F3", "the same, as a function"),
    _word("NAMED", "F3", "what a word of a table names, as an offset"),
    _word("u16 segment", "F6", "a segment kept in a number: F4 to F6 "
                               "for the texts, a list and a buffer, and "
                               "F7 for the list of the speed search",
          text=r"u16\s+segment"),
    _word("AddressOf", "F5", "a place in the editor's text made a "
                             "segment and an offset", CALLED),
    _word("OFFSET_OF", "F5", "the offset of such a place"),
    _word("md.TreAdr", "F7", "a place of the main data kept in a word "
                             "of it: the tree's", OF_MAIN, "TreAdr"),
    _word("md.TreFrst", "F7", "the tree's first line", OF_MAIN, "TreFrst"),
    _word("md.TreCrnt", "F7", "the tree's current line", OF_MAIN,
          "TreCrnt"),
    _word("md.HstAdr", "F7", "the history's", OF_MAIN, "HstAdr"),
    _word("md.BatAdr", "F7", "the commands'", OF_MAIN, "BatAdr"),
    _word("NEAR", "F8", "a cast that does nothing: F4 to F8, by file"),
    _word("MD_PLACE", "F8", "a pointer into the main data made a "
                            "number: F4 to F8, by file"),
    _word("MD_POINTER", "F8", "and the number made the pointer again"),
    # What stays.
    _word("TRELIST", None, "the tree's list: a pointer since F7"),
    _word("TREE_OFFSET", None, "an entry of the tree's list as an offset "
                               "in it, where one routine moves a cursor "
                               "through a panel's files or the tree alike: "
                               "it rests on the list, not on the megabyte"),
    _word("TREE_ENTRY", None, "and the entry at such an offset"),
    _word("NUMBER_OF", None, "a word of a table that holds a number"),
    _word("ENTRY", None, "an offset in a panel, added to the panel"),
    _word("DSKBUF", None, "where a panel's list begins in it"),
    _word("MAIN_AS_PANEL", None, "the main data where a panel is asked "
                                 "for, as the original has DS left on it: "
                                 "it rests on the two layouts, not on the "
                                 "megabyte"),
    _word("PANEL_AS_MAIN", None, "a panel where the main data is asked "
                                 "for, the other way round: one place, "
                                 "ClsLine for Run_00"),
    _word("MD_AT", None, "in a table: an offset in the main data"),
    _word("WCB_AT", None, "in a table: an offset in a panel"),
    _word("NUMBER", None, "in a table: a number where a name could be"),
]
NAMES = [word[0] for word in WORDS]


def _code(path):
    """The text of a file as it is counted: comments, strings and
    characters blanked, and the lines that define a word of WORDS."""
    code = bare(path.read_text(encoding="latin-1"), True)
    return re.sub(r"^[ \t]*#[ \t]*define[ \t]+(\w+)",
                  lambda found: " " * len(found[0]), code, flags=re.M)


def places(source):
    """Every place of every word: (file, line, word, the line's text)."""
    found = []
    for path in sorted(source.glob("*.[ch]")):
        text = path.read_text(encoding="latin-1").splitlines()
        code = _code(path)
        for name, pattern, _, _ in WORDS:
            for one in pattern.finditer(code):
                line = code.count("\n", 0, one.start())
                found.append((path.name, line + 1, name,
                              text[line].strip()))
    return found


def counts(found, source):
    table = {path.name: Counter() for path in source.glob("*.[ch]")}
    for name, _, word, _ in found:
        table[name][word] += 1
    return table


HEAD = """\
The words of the addresses in the portable C, src/portable_c/: of each
file, how often it writes each, or that it has none. harness/words.py
counted it, and says what the words are and which piece of step 2 is
to have each at nothing. Not to be edited: the test portable_c-words
holds the tree to it, and `python3 harness/words.py --record` writes it
anew.
"""


def written(table):
    width = max(len(name) for name in table) + 2
    lines = [HEAD]
    for name in sorted(table):
        words = table[name]
        what = ", ".join(f"{word} {words[word]}" for word in NAMES
                         if words[word]) or "none"
        lines.append(f"{name:<{width}}{what}")
    return "\n".join(lines) + "\n"


def recorded():
    table = {}
    if not RECORD.exists():
        return table
    _, _, body = RECORD.read_text().partition("\n\n")
    for line in body.splitlines():
        name, _, what = line.partition(" ")
        words = Counter()
        if what.strip() != "none":
            for item in what.split(","):
                word, _, number = item.strip().rpartition(" ")
                words[word] = int(number)
        table[name] = words
    return table


def summary(table):
    """The table of the words: what is left of each and which piece is
    to have it at nothing."""
    lines = [f"{'word':<15}{'places':>7}{'files':>6}  {'the most':<17}"
             f"{'none by':<9}what it is"]
    left = Counter()
    for name, _, by, what in WORDS:
        number = sum(words[name] for words in table.values())
        files = [(words[name], file) for file, words in table.items()
                 if words[name]]
        most = "{1} {0}".format(*max(files)) if files else ""
        if by:
            left[by] += number
        lines.append(f"{name:<15}{number:>7}{len(files):>6}  {most:<17}"
                     f"{by or 'stays':<9}{what}")
    lines += ["", "Left to go, by the piece that is to have it at "
                  "nothing: "
              + ", ".join(f"{by} {left[by]}" for by in
                          sorted(left, key=lambda by: int(by[1:])))]
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
        lines.append(f"{name}:")
        for word in sorted(set(now) | set(then),
                           key=lambda word: NAMES.index(word)
                           if word in NAMES else len(NAMES)):
            if word not in NAMES:
                lines.append(f"    {word}: in the record, and no word "
                             "that is counted")
            elif now[word] != then[word]:
                way = "more" if now[word] > then[word] else "fewer"
                lines.append(f"    {word}: {now[word]}, where the record "
                             f"has {then[word]}: {way}")
    return lines


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--record", action="store_true",
                        help="write the record anew")
    parser.add_argument("--show", metavar="WORD|FILE",
                        help="print the lines that were counted of a "
                             "word, or in a file")
    parser.add_argument("source", nargs="?", type=Path, default=SOURCE,
                        help="the tree (default: src/portable_c)")
    args = parser.parse_args()
    found = places(args.source)
    if args.show:
        shown = [place for place in found if args.show in place[::2]]
        if not shown:
            sys.exit(f"{args.show} is no word that is counted and no "
                     "file that has one: the words are "
                     + ", ".join(NAMES))
        for name, line, word, text in shown:
            print(f"{name}:{line}: {word}: {text}")
        return
    table = counts(found, args.source)
    print(summary(table))
    if args.record:
        RECORD.write_text(written(table))
        print(f"\nWritten to {RECORD.relative_to(ROOT)}.")
        return
    lines = differences(table, recorded())
    if lines:
        print("\nThe tree is not as the record has it, "
              f"{RECORD.relative_to(ROOT)}:\n" + "\n".join(lines)
              + "\n\nIf the change was to do that, write the record anew, "
                "`python3 harness/words.py --record`, and commit it with "
                "the change: its lines then say what went.")
        sys.exit(1)
    print("\nThe tree is as the record has it.")


if __name__ == "__main__":
    main()
