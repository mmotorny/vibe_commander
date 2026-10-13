"""Where a build is meant to behave differently from the original.

Each deviation is declared here, with its reason. A golden is derived from
the original with these differences made to it (see goldens.py), so that it
holds exactly what a build is expected to show.

Figures that follow the size of the program are not held against the
original at all, and are declared here too: see IGNORED.

That much holds for both C trees. The portable C is declared to behave
otherwise than the original in more, in src/portable_c/DEVIATIONS.md; what
the harness goes by of that is at the end of this file: see TREES.
"""
import re
from dataclasses import dataclass, field, replace

from dos import COLUMNS


def expected(screen):
    """`screen` of the original as a build is expected to show it."""
    cells = bytearray(screen.cells)
    lines = screen.lines()
    for row, line in enumerate(lines):
        for pattern, text, below in DEVIATIONS:
            match = pattern.search(line)
            if match and below:
                # Known by the line below it as well.
                under = lines[row + 1] if row + 1 < len(lines) else ""
                match = below.search(under) and match
            if match:
                # As much of it as shows: a panel may cover the rest.
                start, end = match.span(1)
                for column in range(start, end):
                    cells[(row * COLUMNS + column) * 2] = \
                        ord(text[column - start])
    return replace(screen, cells=bytes(cells))


def compared(screen):
    """`screen` with what is not compared blanked out. When a build is held
    against a golden, both screens are made like this, that of the golden
    too: it may have been derived before a figure was left out. A golden
    that is derived now holds its screens like this already, and blanking
    what is blanked changes nothing."""
    cells = bytearray(screen.cells)
    for row, line in enumerate(screen.lines()):
        for pattern in IGNORED:
            match = pattern.search(line)
            if match:
                for column in range(*match.span(1)):
                    cells[(row * COLUMNS + column) * 2] = ord(BLANK)
    return replace(screen, cells=bytes(cells))


# What a build is to show otherwise than the original: the part of a line
# that the pattern's group matches, the text that a build has there, and,
# for a line that is known by the one below it as well, the pattern of
# that one.
DEVIATIONS = [
    # The banner that the program prints when it starts names its edition.
    # The published source is of the freeware edition; the binaries shipped
    # in 2000 are of another, with a name as long as nine letters. The
    # banner stays on the screen behind the panels.
    (re.compile(r"^The Volkov Commander, Version 4\.05 ([a-z]{1,9})"),
     "freeware ", None),
    # The same banner behind a panel on the left with none on the right:
    # the panel covers the 40 cells that the line is known by, and the
    # last four letters of the edition show next to its frame, with a
    # letter less in a build. It is the banner if nothing else is on the
    # line, as far as where the clock would be, and the line below has
    # behind a panel's frame the end of the second line that the program
    # prints, the last two letters of its author's name, and nothing else.
    (re.compile(r"^[╔║╟╚].{38}[╗║╢╝](ware) {30}"), "are ",
     re.compile(r"^[╔║╟╚].{38}[╗║╢╝]ov {30}")),
    # The program names its own file when it cannot load it again after a
    # command. The original is VC.COM; the C translation is VC.EXE.
    (re.compile(r"^Bad or missing D:\\VC\\VC\.(COM)"), "EXE", None),
    # And in the map of the memory, Alt+F5, which has a line for every
    # program with the name of its file. No scenario can show the map
    # without that line; the user has said that the deviation may stand
    # here too.
    (re.compile(r"║ . [0-9A-F]{4}[ \d]{6}[ \d,]{10} {3}VC\.(COM) "), "EXE",
     None),
]

# What stands in a golden for a character that is not compared.
BLANK = "#"

# What a build shows that is not compared with the original: the part of a
# line that the pattern's group matches. Its colours are compared. Every
# pattern is tried on the line as the screen has it, so two of them can
# each take a number of the same line.
IGNORED = [
    # The editor says in its first line how many bytes it has free for the
    # text. That is what the program leaves of the memory, and the
    # translation is larger than the original, by more with every routine
    # translated. An expected number would have to follow the size of the
    # build that is tested; the number says too little about the program to
    # be worth that. The field is the number and the room in front of it.
    re.compile(r"^Edit: .* Col \d+( [\d, ]+) Free"),
    # The Info panel says how much memory is free. As the program is
    # configured that is what a command gets: all but the part of the
    # program that stays while one runs. That part holds the program's
    # stack, and it is to be translated: it does not stay as long in the
    # translation as in the original. With Memory allocation switched off
    # the number is what is left with all of the program loaded, and follows
    # the size of the build like the editor's. Either way it is left out,
    # in every scenario. The field is the number only, as much of it as a
    # box in front leaves to see, with the comma or the space that a
    # country has between its thousands: the line is centred, so a number
    # of another length would move the words too, and that is noticed.
    re.compile(r"(\d+(?:[, ]\d{3})*) Bytes Free"),
    # The same line where a box covers its words and the end of the
    # number, as the tree of Alt+F10 does: there is nothing to know the
    # line by but its place. It is the one line of the Info that begins
    # ten cells into the panel, a number of seven cells and two words in
    # 38; the total above it begins nine cells in. The box has three cells
    # of room in front of its frame.
    re.compile(r"^║ {10}(\d(?:[\d, ]*\d)?) {3}║"),
    # MEM of DOS, run as a command, says how much memory is in use and how
    # much is free while it runs: the first includes the part of the
    # program that stays, the second is what that part leaves. The fields
    # are the two numbers in each of three lines, and both numbers of the
    # largest program that would fit.
    re.compile(r"^(?:Conventional|Total memory|Total under 1 MB) +[\d,]+K"
               r" +([\d,]+)K +[\d,]+K"),
    re.compile(r"^(?:Conventional|Total memory|Total under 1 MB) +[\d,]+K"
               r" +[\d,]+K +([\d,]+)K"),
    re.compile(r"^Largest executable program size +([\d,]+)K "
               r"\([\d,]+ bytes\)"),
    re.compile(r"^Largest executable program size +[\d,]+K "
               r"\(([\d,]+) bytes\)"),
    # The map of the memory, Alt+F5, has a line for every program and
    # every stretch of free memory: where it begins, a segment in hex, the
    # number of its blocks and its size. The program is larger in the
    # translation, so it has another size there, and what lies behind it,
    # free memory and a program that was started from it and stays, begins
    # elsewhere and has other sizes, and is other blocks: what the program
    # takes again after a command fits before a program that stays, or does
    # not. Left out are the size of the program; of every line of free
    # memory where it begins, its blocks and its size, also of the one that
    # lies before the program, which a line by itself does not tell from the
    # others; and where a program begins that a scenario has stay: FDAPM
    # in memory-map, T.COM and U.COM in stay. A field is left out whole,
    # with the room in front of the number, so that a number with fewer
    # digits leaves out the same cells.
    re.compile(r"║ . ([0-9A-F]{4}[ \d]{6}[ \d,]{10}) {3}free memory "),
    re.compile(r"║ . ([0-9A-F]{4})[ \d]{6}[ \d,]{10} {3}(?:FDAPM|T|U)\.COM "),
    re.compile(r"║ . [0-9A-F]{4}[ \d]{6}([ \d,]{10}) {3}VC\.(?:COM|EXE) "),
]

# Deviations that show on no screen, and so change no golden. They are
# listed here all the same, so that this is the whole list.
#
# - When the program file cannot be loaded again after a command and ENTER
#   is pressed to try again, the original stores the size it expects
#   through a segment register left on the DOS environment. The word goes
#   into other data of the program: which, depends on how long the
#   environment is. And the place it was meant for keeps what the last
#   attempt left there, which is wrong if that attempt found a file and it
#   was not the program. The translation stores the size where it belongs.
#   (Exec12 of the original; Exec10 in src/c/resident.c.)
#
# - In a line of text, ^A and two hex digits set a colour and ^C and two
#   hex digits stand for a character: that is the markup of Norton Guides,
#   and how the original's LnStr counts the cells of a line. Its NgLine,
#   which writes the line, takes ^A for the character and ^C for the
#   colour. The translation's NgLine takes them as LnStr does. No text of
#   the program or of its help file has either. (NgLine in src/c/line.c.)


# --- What a tree is declared to do otherwise --------------------------------
#
# The C translation, src/c/, behaves as the original but for the above, and
# is held against the original's goldens in every scenario. The portable C,
# src/portable_c/, is where the program leaves DOS behind: each place where
# it is to behave otherwise than the original is declared in
# src/portable_c/DEVIATIONS.md, with the scenarios that show it, and those
# scenarios are named here again, for the harness to go by.
#
# A scenario that a deviation changes cannot be held against what the
# original did. It gets a golden of the tree's own, recorded from the tree
# (goldens.py record) and read by whoever recorded it, in a directory of
# that tree's name next to the original's goldens. Such a golden shows that
# nothing has changed since it was recorded, not that anything is right.
# Every other scenario is held against the original's golden, as it is.


@dataclass(frozen=True)
class Declared:
    """What a tree is declared to do otherwise than the original, as far
    as the scenarios go."""
    # Scenarios that the tree is not taken through: scenario -> why. For
    # one that only shows what the tree has no more.
    not_run: dict = field(default_factory=dict)
    # Steps that the tree is not taken through, in any scenario: step ->
    # why. For keys that choose what the tree has no more, and would do
    # something else there.
    left_out: dict = field(default_factory=dict)
    # Scenarios that the tree is held in against a golden of its own:
    # scenario -> what changes in it.
    changed: dict = field(default_factory=dict)

    def steps(self, steps):
        """The steps of `steps` that the tree is taken through."""
        return tuple(step for step in steps if step not in self.left_out)


_MAP = ("the map of the memory has the program with a block more, and "
        "with INT 24h among its interrupts")
_HOLE = ("; and where a program has stayed, the memory in front of it, "
         "which the panels have left, is a stretch of free memory more")
_NO_ROOM = ("the second copy has no memory to run a command: each copy "
            "keeps all of itself")

TREES = {
    # The portable C stays in the memory whole while a command runs, and
    # has no Memory allocation in its Options menu.
    "portable_c": Declared(
        not_run={
            "memory": "it switches Memory allocation off and shows the "
                      "menu without its mark: the portable C has no such "
                      "option",
            "passed-on": "it has a program stay in the memory between "
                         "two copies and runs commands from the second: "
                         "with all of each copy kept, the machine of the "
                         "tests has no memory for the second copy behind "
                         "that program. handed-on sees the same with one "
                         "copy",
        },
        left_out={
            "F9 o m": "Memory allocation switched off or on: the Options "
                      "menu of the portable C has no line that M chooses, "
                      "and stays open",
        },
        changed={
            "lost": "the program does not load itself again after a "
                    "command, and so does not miss its file",
            "quick": "Memory allocation is not switched off before the "
                     "last command",
            "clock": "the Options menu, a line shorter",
            "clock-russia": "the Options menu, a line shorter",
            "menu": "the Options menu, a line shorter",
            "menu-items": _MAP,
            "memory-map": _MAP + _HOLE + "; Memory allocation is not "
                          "switched off for a second look at the map",
            "memory-map-russia": _MAP,
            "stay": _MAP + _HOLE,
            "second-copy": _NO_ROOM,
            "start-memory": _NO_ROOM + ", so the command that was to "
                            "leave it without memory for its data does "
                            "not run, and the copy is still there",
            "ended": _MAP + "; " + _NO_ROOM + ", so no program has stayed "
                     "from it when DOS ends it",
            "mouse-memory": _MAP + _HOLE,
            "mouse-middle": _MAP,
            "mouse-drag-lists": _MAP,
        },
    ),
}


def declared(tree):
    """What the tree `tree` is declared to do otherwise: nothing, for the
    original, None, and for a tree that is not in TREES."""
    return TREES.get(tree, Declared())
