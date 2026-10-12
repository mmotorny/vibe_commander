"""Where a build is meant to behave differently from the original.

Each deviation is declared here, with its reason. A golden is derived from
the original with these differences made to it (see goldens.py), so that it
holds exactly what a build is expected to show.

Two figures that a build shows are not held against the original at all,
and are declared here too: see IGNORED and IGNORED_IN.
"""
import re
from dataclasses import replace

from dos import COLUMNS


def expected(screen):
    """`screen` of the original as a build is expected to show it."""
    cells = bytearray(screen.cells)
    for row, line in enumerate(screen.lines()):
        for pattern, text in DEVIATIONS:
            match = pattern.search(line)
            if match:
                # As much of it as shows: a panel may cover the rest.
                start, end = match.span(1)
                for column in range(start, end):
                    cells[(row * COLUMNS + column) * 2] = \
                        ord(text[column - start])
    return replace(screen, cells=bytes(cells))


def compared(screen, scenario):
    """`screen` of `scenario` with what is not compared blanked out. When a
    build is held against a golden, both screens are made like this, that
    of the golden too: it may have been derived before a figure was left
    out. A golden that is derived now holds its screens like this already,
    and blanking what is blanked changes nothing."""
    cells = bytearray(screen.cells)
    for row, line in enumerate(screen.lines()):
        for pattern in IGNORED + IGNORED_IN.get(scenario, []):
            match = pattern.search(line)
            if match:
                for column in range(*match.span(1)):
                    cells[(row * COLUMNS + column) * 2] = ord(BLANK)
    return replace(screen, cells=bytes(cells))


DEVIATIONS = [
    # The banner that the program prints when it starts names its edition.
    # The published source is of the freeware edition; the binaries shipped
    # in 2000 are of another, with a name as long as nine letters. The
    # banner stays on the screen behind the panels.
    (re.compile(r"^The Volkov Commander, Version 4\.05 ([a-z]{1,9})"),
     "freeware "),
    # The program names its own file when it cannot load it again after a
    # command. The original is VC.COM; the port is VC.EXE.
    (re.compile(r"^Bad or missing D:\\VC\\VC\.(COM)"), "EXE"),
]

# What stands in a golden for a character that is not compared.
BLANK = "#"

# What a build shows that is not compared with the original: the part of a
# line that the pattern's group matches. Its colours are compared.
IGNORED = [
    # The editor says in its first line how many bytes it has free for the
    # text. That is what the program leaves of the memory, and the port is
    # larger than the original, by more with every routine translated. An
    # expected number would have to follow the size of the build that is
    # tested; the number says too little about the program to be worth
    # that. The field is the number and the room in front of it.
    re.compile(r"^Edit: .* Col \d+( [\d, ]+) Free"),
]

# The same, for what is not compared in the scenarios named and is
# compared in every other.
IGNORED_IN = {
    # The Info panel says how much memory is free. As the program is
    # configured that is what a command gets: all but the part of the
    # program that stays while one runs, which is assembly still and as
    # long in the port as in the original. So the number is the same, and
    # every scenario that shows the Info compares it. With Memory
    # allocation switched off it is what is left with all of the program
    # loaded, and follows the size of the build like the editor's. This
    # scenario switches it off, and has the number left out on all of its
    # screens. The field is the number only: the line is centred, so a
    # number of another length would move the words too, and that is
    # noticed.
    "memory": [re.compile(r"(\d[\d,]*) Bytes Free")],
}

# Deviations that show on no screen, and so change no golden. They are
# listed here all the same, so that this is the whole list.
#
# - When the program file cannot be loaded again after a command and ENTER
#   is pressed to try again, the original stores the size it expects
#   through a segment register left on the DOS environment. The word goes
#   into other data of the program: which, depends on how long the
#   environment is. And the place it was meant for keeps what the last
#   attempt left there, which is wrong if that attempt found a file and it
#   was not the program. The port stores the size where it belongs.
#   (Exec12 in port/asm/VC.ASM.)
#
# - In a line of text, ^A and two hex digits set a colour and ^C and two
#   hex digits stand for a character: that is the markup of Norton Guides,
#   and how the original's LnStr counts the cells of a line. Its NgLine,
#   which writes the line, takes ^A for the character and ^C for the
#   colour. The port's NgLine takes them as LnStr does. No text of the
#   program or of its help file has either. (NgLine in port/line.c.)
