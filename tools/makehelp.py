#!/usr/bin/env python3
"""Make the project's own help file, VC.HLP, from its readable source.

    python3 tools/makehelp.py [SOURCE [OUTPUT]]

SOURCE is src/portable_c/help/vc-help.txt unless given, OUTPUT
build/portable_c/VC.HLP. The build runs this (the target
portable_c-help), so the file is made, not kept: the source is what the
repository has. The text is the project's own. Nothing of the help that
the shareware distribution of Volkov Commander ships goes into it, and
nothing here reads that file.

The source. A line that begins with `;;` is a comment. `@page N Title`
begins page N, with its title; the lines up to the next `@page` are its
text, the empty lines at its end left out. The title is also the page's
line in the index. The pages are numbered from 1 and every number from
1 to the last is to be there, since the index has a line for each and a
page is known by its line. A line may have the markup of NgLine in
line.c: ^B bold, ^U underlined, ^R reversed, each switched off by
itself or by ^N, ^^ for a caret. The text is written in UTF-8 and kept
in the file in the code page 437, which the program shows.

The file, as help.c and sthelp.c read it:

  - at 8, the version: "VC 4.05 Help" and a zero byte, as the program
    holds it (Help_F of help.c);
  - at 194h, where the index is in the file, four bytes;
  - each entry: 26 bytes that describe it and its text. The 26 bytes
    are the mode, 0 for the index and 1 for a page; the length of the
    text; its lines, the title counted for a page; a word not read;
    for a page its line in the index, from 0; for a page where the
    index is, and FFFFFFFFh for the index; and twelve bytes that the
    program does not take from the file. A page's text is its title
    and its lines, each ended by a zero byte. The index's text is six
    bytes for every line, the last four of which are where its page
    is, and then the lines, each ended by a zero byte;
  - the place at 194h, the 26 bytes and the texts are kept with every
    byte exclusive-ored with 1Ah, as DeCode of help.c takes it off.

The pages are behind the index in their order. That matters for the
setup's pages, 55 to 61, which the setup reads as one piece from the
55th on, of 8,155 bytes at most (sthelp.c): the script refuses a source
whose seven are longer. It refuses a line wider than the window's 62
columns, a text longer than the 2,967 bytes that the help has room for,
and a page missing from the numbers.
"""
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c" / "help" / "vc-help.txt"
OUTPUT = ROOT / "build" / "portable_c" / "VC.HLP"

VERSION = b"VC 4.05 Help\0"     # at 8: Help_F of help.c, from HlpHead
INDEX_AT = 0x194                # where the place of the index is
MARK = 0x1A                     # what every byte is exclusive-ored with
WIDTH = 62                      # the columns of a line in the window
ROOM = 3000 - 32 - 1            # the longest text: below LenHelp - 20h
SETUP_FIRST, SETUP_PAGES = 55, 7        # the setup's pages (sthelp.c)
SETUP_ROOM = 0xFEE0 - 0xDF03 - 2        # what the setup reads of them
NOTE = (b"Vibe Commander help. Made by tools/makehelp.py from "
        b"src/portable_c/help/vc-help.txt: text of the project's own.\r\n")


def shown(line):
    """The cells that a line of markup takes on the screen."""
    cells, at = 0, 0
    while at < len(line):
        if line[at] == ord("^") and at + 1 < len(line):
            letter = chr(line[at + 1]).upper()
            if letter == "^":
                cells += 1
            elif letter in "AC":
                cells += letter == "C"
                at += 2
            at += 2
            continue
        cells += 1
        at += 1
    return cells


def read(source):
    """The pages of `source`, {number: (title, [lines])}, as bytes of
    the code page 437."""
    pages, current, problems = {}, None, []
    for number, text in enumerate(source.read_text("utf-8").splitlines(),
                                  1):
        if text.startswith(";;"):
            continue
        where = f"{source.name}:{number}"
        found = re.fullmatch(r"@page (\d+) (.+)", text)
        if found:
            page = int(found.group(1))
            if page in pages:
                problems.append(f"{where}: page {page} twice")
            current = pages[page] = (found.group(2).strip(), [])
            text = current[0]
        elif current is None:
            if text.strip():
                problems.append(f"{where}: text in front of the first page")
            continue
        else:
            current[1].append(text.rstrip())
        try:
            encoded = text.rstrip().encode("cp437")
        except UnicodeEncodeError as error:
            problems.append(f"{where}: not in the code page 437: {error}")
            continue
        if shown(encoded) > WIDTH:
            problems.append(f"{where}: {shown(encoded)} columns, more "
                            f"than {WIDTH}")
    out = {}
    for page, (title, lines) in pages.items():
        while lines and not lines[-1]:
            lines.pop()
        out[page] = (title.encode("cp437"),
                     [line.encode("cp437") for line in lines])
    missing = sorted(set(range(1, max(out, default=0) + 1)) - set(out))
    if missing or not out:
        problems.append(f"{source.name}: no page {missing or 1}")
    return out, problems


def entry(mode, text, lines, line, index):
    """An entry: its 26 bytes and its text."""
    return struct.pack("<9H8x", mode, len(text), lines, 0, line,
                       index & 0xFFFF, index >> 16, 0, 0) + text


def marked(data):
    return bytes(byte ^ MARK for byte in data)


def make(pages):
    """The help file of `pages`, and what is wrong with it."""
    problems = []
    numbers = sorted(pages)
    texts = {page: b"".join(line + b"\0" for line in [title, *lines])
             for page, (title, lines) in pages.items()}
    index_lines = b"".join(b" " + pages[page][0] + b"\0" for page in numbers)
    index_size = 26 + 6 * len(numbers) + len(index_lines)
    index_at = INDEX_AT + 4
    places, at = {}, index_at + index_size
    for page in numbers:
        places[page] = at
        at += 26 + len(texts[page])
    index_text = b"".join(struct.pack("<HI", page, places[page])
                          for page in numbers) + index_lines
    for name, text in [("the index", index_text),
                       *((f"page {page}", texts[page]) for page in numbers)]:
        if len(text) > ROOM:
            problems.append(f"{name}: {len(text)} bytes of text, more than "
                            f"{ROOM}")
    setup = [page for page in numbers
             if SETUP_FIRST <= page < SETUP_FIRST + SETUP_PAGES]
    length = sum(26 + len(texts[page]) for page in setup)
    if length > SETUP_ROOM:
        problems.append(f"the setup's pages: {length} bytes, more than "
                        f"{SETUP_ROOM}")
    head = bytearray(INDEX_AT)
    head[8:8 + len(VERSION)] = VERSION
    head[0x20:0x20 + len(NOTE)] = NOTE
    data = bytes(head) + marked(struct.pack("<I", index_at))
    data += marked(entry(0, index_text, len(numbers), 0, 0xFFFFFFFF))
    for page in numbers:
        data += marked(entry(1, texts[page], 1 + len(pages[page][1]),
                             page - 1, index_at))
    return data, problems


def main(argv):
    source = Path(argv[1]) if len(argv) > 1 else SOURCE
    output = Path(argv[2]) if len(argv) > 2 else OUTPUT
    pages, problems = read(source)
    data, more = make(pages)
    problems += more
    if problems:
        for problem in problems:
            print(f"makehelp: {problem}", file=sys.stderr)
        return 1
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(data)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
