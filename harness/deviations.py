"""Where a build is meant to behave differently from the original.

Each deviation is declared here, with its reason. A golden is derived from
the original with these differences made to it (see goldens.py), so that it
holds exactly what a build is expected to show.

Figures that follow the size of the program are not held against the
original at all, and are declared here too: see IGNORED.

That much holds for both C trees. The portable C is declared to behave
otherwise than the original in more, in src/portable_c/DEVIATIONS.md; what
the harness goes by of that is at the end of this file: see TREES. What
such a tree is expected to show in a scenario is still the original's
golden, with the declared difference made to it when the golden is read
for that tree, and nothing else:

  - steps that the tree is not taken through are left out, with their
    screens: `left_out` and `left_out_of` of TREES;
  - steps that get the tree to the same thing by other keys are put in
    the place of the original's, one for one: `in_place_of` of TREES,
    and `in_place_in` for those of one scenario;
  - a difference that is on many screens is a rule for that tree alone,
    `rules` of TREES: of the form that DEVIATIONS has, or, for a text
    that is of another length in the tree or that something may cover in
    part, a Part, which says the text as the original shows it and as
    the tree does, cell for cell;
  - and what one scenario shows otherwise is a patch, a file of the rows
    that differ in harness/goldens/TREE/, written by whoever declared
    the deviation: the top of goldens.py says how it is written, read
    and made.
"""
import re
from dataclasses import dataclass, field, replace
from functools import cache, cached_property
from operator import eq

from dos import COLUMNS, ROWS


@dataclass(frozen=True)
class Part:
    """A part of the screen that a tree shows otherwise than the original,
    wherever it shows and as much of it as shows: a rule of the second
    kind, for a text that is not as long in the tree as in the original,
    or stands elsewhere on its row, or for a block of rows that the tree
    has other rows in, and that a menu or a box may cover in part.

    It says the part as the original shows it when nothing covers it and
    as the tree shows it then, cell for cell, character and colour. On a
    screen of the golden where the part is known to stand, each of its
    cells that the golden has exactly as the original's part has it,
    character and colour, is expected as the tree's part has that cell;
    every other cell there is what covers the part, and is expected as
    the golden has it. So a rule of this kind leaves nothing out of the
    comparison, and it can be wrong of a cell in two ways, both of which
    fail the row, so that the screen wants a patch. What covers the part
    may happen to have a cell as the part has it, a space in the same
    colour: the rule then expects the tree's part there, which does not
    show. And the part may show in a cell in another colour, under the
    mouse's pointer or in the shadow of a box: the rule takes that for
    something that covers it, and expects the original's character. It
    is not taught the pointer's colours: what the pointer makes of grey
    on black is the colour of a box, black on grey, and it would take the
    box for the part. The shadow of a box it knows where it is told to,
    `shaded`: a shadow changes the colour of a cell and leaves its
    character, so a cell of the part in a shadow has the part's
    character in what the shadow makes of the part's colour, and the
    tree's has the tree's character in what it makes of the tree's."""
    # What the part is, in a few words, for the test's report.
    what: str
    # The rows of the part as the original shows them and as the tree
    # does, one under the other, each from the first column of the part
    # on: as many of the one as of the other. Where a row of the tree's is
    # longer than the original's or shorter, the other goes on in spaces:
    # the part is as wide as the longer of the two, and what stands
    # behind the shorter text belongs to it, as the blank screen does
    # behind a line that DOS has written. A figure that the comparison
    # leaves out (IGNORED) is in a row of the original's as the golden
    # has it, in BLANK.
    was: tuple
    now: tuple
    # The colour of its cells in the original: one number for a part of
    # one colour, or for each row its colours as a golden has them, runs
    # of an attribute byte, "1Bx10 1Ex7 1Bx23", which are to be as many
    # cells as the part is wide.
    colour: object
    # The columns at which the part may begin, on any row.
    columns: tuple = (0,)
    # How the part is known to stand at a place: at least this many of
    # the original's characters that are no spaces show there, in their
    # cells and in their colours. Eight were too few to be sure: a line
    # of the editor had nine letters of the original's title in the cells
    # and in the colour of an Info panel's, "ompa" and "ersion" of
    # "incompatible version" under "Commander, Version".
    least: int = 12
    # The colours in the tree, written as `colour` is, or None where
    # every cell has the colour that the original's part has there.
    now_colour: object = None
    # Characters of the original's part that it is not known by, though
    # they are no spaces: those of a frame and of a line across a panel,
    # which every panel has in the same cells.
    frame: str = ""
    # Whether the part is known in the shadow of a box as well, which
    # makes of a colour what the program makes of it as it is
    # distributed (ShadCol of src/portable_c/screen.c). A character in a
    # shadow then counts among those that the part is known by.
    shaded: bool = False

    @cached_property
    def rows(self):
        """The rows of the part as bytes of the screen: of each the
        original's characters and the tree's, as long as each other; the
        original's with a zero byte for every character that the part is
        not known by, a space or one of the frame; and the colours of the
        original's cells and of the tree's."""
        if len(self.was) != len(self.now):
            raise ValueError(f"a part of {len(self.was)} rows that the "
                             f"tree shows as {len(self.now)}")
        rows = [(was.ljust(len(now)).encode("cp437"),
                 now.ljust(len(was)).encode("cp437"))
                for was, now in zip(self.was, self.now)]
        unknown = bytes.maketrans((" " + self.frame).encode("cp437"),
                                  bytes(len(self.frame) + 1))
        colours = self._colours(self.colour, rows)
        now = colours if self.now_colour is None \
            else self._colours(self.now_colour, rows)
        return [(was, to, was.translate(unknown), before, after)
                for (was, to), before, after in zip(rows, colours, now)]

    def _colours(self, colour, rows):
        """The colours `colour` of the part's rows, cell for cell."""
        if isinstance(colour, int):
            return [bytes((colour,)) * len(was) for was, _ in rows]
        if len(colour) != len(rows):
            raise ValueError(f"{self.what}: colours of {len(colour)} rows "
                             f"for a part of {len(rows)}")
        cells = [b"".join(bytes((int(run[:2], 16),)) * int(run[3:])
                          for run in row.split()) for row in colour]
        for (was, _), row in zip(rows, cells):
            if len(row) != len(was):
                raise ValueError(f"{self.what}: colours of {len(row)} "
                                 f"cells for a row of {len(was)}")
        return cells

    def places(self, screen):
        """Where `screen` has the part: the row and the column of each
        place. The row is above the screen, below 0, where the first
        rows of the part have gone off its top. No place has a cell of
        another's: where a beep has moved the screen up a row and what
        the program then put back has left a row of the part twice, the
        part could be known at two places that lie over each other, and
        it stands where more of it shows."""
        codes, colours = screen.cells[0::2], screen.cells[1::2]
        known = []
        for row in range(1 - len(self.rows), ROWS - len(self.rows) + 1):
            for column in self.columns:
                # Whether enough characters are there at all, in whatever
                # colour, is quickly told, and on most rows none is. A
                # cell of the screen with a zero byte in it counts here
                # where the part has a space, and not below.
                on = [(at, each) for at, each in enumerate(self.rows, row)
                      if at >= 0]
                there = [codes[at * COLUMNS + column:
                               at * COLUMNS + column + len(by)]
                         for at, (_, _, by, _, _) in on]
                if sum(sum(map(eq, by, some))
                       for (_, (_, _, by, _, _)), some
                       in zip(on, there)) < self.least:
                    continue
                shown = 0
                for (at, (_, _, by, before, _)), some in zip(on, there):
                    at = at * COLUMNS + column
                    shown += sum(
                        a == b != 0 and (colour == was or self.shaded
                                         and colour == shaded(was))
                        for a, b, colour, was in zip(
                            by, some, colours[at:at + COLUMNS], before))
                if shown >= self.least:
                    known.append((shown, row, column))
        places = []
        width = max(len(was) for was, *_ in self.rows)
        for _, row, column in sorted(known, key=lambda place: -place[0]):
            if all(abs(row - other) >= len(self.rows)
                   or abs(column - beside) >= width
                   for other, beside in places):
                places.append((row, column))
        return sorted(places)

    def made(self, screen, cells):
        """Make the part to `cells`, those of `screen`: wherever the
        screen has it, every cell that is the original's becomes the
        tree's."""
        for row, column in self.places(screen):
            for at, (was, now, _, before, after) in enumerate(self.rows,
                                                               row):
                if at < 0:
                    continue
                begin = (at * COLUMNS + column) * 2
                for cell in range(len(was)):
                    here = begin + cell * 2
                    shown = screen.cells[here:here + 2]
                    if shown == bytes((was[cell], before[cell])):
                        cells[here:here + 2] = now[cell], after[cell]
                    elif self.shaded and shown == bytes(
                            (was[cell], shaded(before[cell]))):
                        cells[here:here + 2] = \
                            now[cell], shaded(after[cell])


def shaded(colour):
    """What the shadow of a box makes of the colour `colour` of a cell,
    as the program is distributed: it takes the background and the
    blinking off it and turns its brightness round."""
    return colour & 0x8F ^ 8


def expected(screen, rules=None):
    """`screen` of the original as a build is expected to show it: by
    the rules of DEVIATIONS, which hold for every build and are made to
    a golden when it is derived. With `rules`, by those instead: the
    rules of one tree (Declared.rules), which are made to the golden's
    screens when it is read for that tree. Every rule is found on the
    screen as the original has it, whatever the rules before it have
    made of it."""
    cells = bytearray(screen.cells)
    line = cache(screen.line)
    for rule in DEVIATIONS if rules is None else rules:
        if isinstance(rule, Part):
            rule.made(screen, cells)
            continue
        pattern, text, below = rule
        for row in range(ROWS):
            match = pattern.search(line(row))
            if match and below:
                # Known by the line below it as well.
                under = line(row + 1) if row + 1 < ROWS else ""
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
    # The original's setup program is a program of its own, and a
    # scenario of it starts it from the command line of the main program
    # (SETUP_COMMAND, below), which writes the prompt and the command
    # where its command line is before it runs a command. That line is
    # among what DOS has shown, which is what stands behind the setup
    # program for as long as it is open. In a C tree the setup is a part
    # of the one program, chosen from its menu: no command starts it, and
    # what DOS has shown has no such line. It is known by the command,
    # which nothing else types, behind whatever prompt, where it is all
    # that a row has, and is nothing but spaces in a build. A beep moves
    # it up a row with the rest of what DOS has shown, and it is known
    # there too.
    (re.compile(r"^(\S[^>]*>d:\\vc\\vcsetup>nul) *$"), " " * COLUMNS, None),
]

# The step by which a scenario of the setup program starts the original's
# setup program: its file by its path, at the command line of the main
# program, with what it prints sent nowhere. It prints a banner through
# DOS when it starts, which would move what DOS has shown up by four
# lines; a C tree has no such banner. And the keys by which the same is
# chosen in a tree that has the setup program in itself: More setup...,
# the last item of the Options menu. They are the keys by which the
# original switches Memory allocation, a step that is left out for such
# a tree (left_out of TREES): a step of the original is left out before
# another is put in its place, so the two do not meet.
SETUP_COMMAND = '"d:\\vc\\vcsetup>nul" Enter'
SETUP_ITEM = "F9 o m"

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
    # A program that DOS has ended goes on in the program that started
    # it with the registers as it leaves them, and p.com of the scenario
    # ended prints six of them: CX, SI, DX and DS as how far they are
    # from where INT 27h goes, ES and the flags. The original leaves in
    # DS:DX where INT 27h went before it took that, and the line has 0000
    # for both. The portable C takes neither INT 27h nor INT 21h
    # (DEVIATIONS.md, 7 and 8) and leaves DS as it came, which is on the
    # program that started it, p.com: that lies behind the first copy of
    # the program and the COMMAND.COM that the copy ran it by, so its
    # segment follows the size of the program. The line is known by CX
    # and SI as p.com sent them, 2222 and 1111, which only the portable
    # C leaves as they came; the one word is left out, the others
    # compared, DX among them, which is as far from DOS's own entry of
    # INT 27h on every build.
    re.compile(r"^2222 1111 [0-9A-F]{4} ([0-9A-F]{4}) 0000 [0-9A-F]{4} +$"),
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
# A scenario that a deviation changes is still held against what the
# original did, with the declared difference made to it when the golden is
# read for the tree (goldens.read): the steps that are left out here go,
# with their screens, and a step that the tree is taken through in place
# of another is held against the screen of that one; the rules of the
# tree are made to every screen; and
# the rows that one scenario shows otherwise are its patch, a file of
# harness/goldens/TREE/ that says of each such row what the original has
# and what the tree is expected to have, with the deviation it is of and
# why. The patches are not listed here: a scenario has one if the file is
# there, and the top of goldens.py says how one is written. Every other
# scenario is held against the original's golden as the rules leave it,
# which is as it is on every screen that has nothing of what a rule is
# of.


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
    # Steps that the tree is not taken through in one scenario: scenario
    # -> steps that follow one another there -> why. For a look at what
    # the tree has no more on the way to something else, by keys that
    # mean something elsewhere: Esc, which leaves a box in one place and
    # switches the panels off in another. The steps are left out wherever
    # the scenario has them one behind the other.
    left_out_of: dict = field(default_factory=dict)
    # What the tree shows otherwise on whatever screen has it: rules,
    # which are made to every screen of every golden when it is read for
    # this tree, and to no other tree's. For a difference that is on
    # many screens, where a patch for each scenario would say the same
    # thing over and over. A rule is of one of two kinds. The first is
    # the form that DEVIATIONS has, (pattern, text, pattern of the line
    # below or None), which puts a text over the cells that the
    # pattern's group matches, one as long as the other. The second is a
    # Part, above: a text or a block of rows as the original shows it
    # and as the tree does, for what is longer or shorter in the tree,
    # or stands elsewhere on its row, and may be covered in part. A rule
    # is made before a scenario's patch, so the rows of a patch are the
    # golden's as the rules leave them.
    #
    # A rule of either kind says "expect this in their place" of the
    # cells that it finds. It could say "these cells are not compared"
    # as well
    # (TODO.md, "Leaving a part of the screen out of the comparison"),
    # which nothing does today: written as a rule with None for its
    # text, (pattern, None, below). Such cells would be blanked as those
    # of IGNORED are, on the golden's screen and on the build's, and not
    # by expected(), which sees the golden alone: compared() would take
    # the tree, which goldens.differences has, and go through the rules
    # of the tree that have None, matching each on the golden's screen
    # and blanking the cells it finds there on both. That is the form
    # that TODO.md asks for, a cell left out only where the golden has
    # what the part shows when nothing covers it.
    rules: tuple = ()
    # Steps that the tree is taken through in place of others, one for
    # one, in every scenario: the step of the original -> (the step of
    # the tree, why). For what the tree gets to by other keys than the
    # original and then shows alike: the screen behind the tree's step is
    # held against the one behind the original's, and so is every screen
    # after it. Such a step changes no scenario by itself, and is no
    # reason for a patch. The tree's step may be the keys of a step that
    # is left out for it, which then mean another thing there: steps()
    # leaves out first and puts in afterwards.
    in_place_of: dict = field(default_factory=dict)
    # Steps that the tree is taken through in place of others in one
    # scenario: scenario -> steps of the original that follow one another
    # there -> (the tree's step for each of them, or None for one that
    # the tree is not taken through; why). For a place where the tree
    # has other lines than the original, a dialog that a setting has
    # gone from: the keys that get to a line, or the place of a click on
    # it, are others there, and mean something else elsewhere in the
    # same scenario, so the steps are known by those around them. The
    # tree's steps are as many as the original's: each is held against
    # the screen behind the original's step that it stands for, with
    # what the scenario's patch says, and a step that set what the tree
    # has no more is None, left out with its screen. They are put in
    # wherever the scenario has the original's one behind the other,
    # among the steps that left_out and left_out_of have left, and
    # before those of in_place_of.
    in_place_in: dict = field(default_factory=dict)
    # Why the tree is taken through no scenario of the setup program
    # (scenarios.Scenario.setup), whichever there are, or None if it is
    # taken through them: for a tree that has no setup program.
    no_setup: str = None

    def taken(self, steps, name=None):
        """How the tree is taken through `steps`, those of the scenario
        `name`: of each step that it is taken through, its place in
        `steps`, counted from 0, and the step as the tree is taken
        through it."""
        places = [at for at, step in enumerate(steps)
                  if step not in self.left_out]
        for gone in self.left_out_of.get(name, {}):
            for at in reversed(found([steps[at] for at in places], gone)):
                del places[at:at + len(gone)]
        instead = {}
        for some, (others, _) in self.in_place_in.get(name, {}).items():
            for at in found([steps[at] for at in places], some):
                instead.update(zip(places[at:at + len(some)], others,
                                   strict=True))
        return [(at, instead.get(at) or
                 self.in_place_of.get(steps[at], (steps[at],))[0])
                for at in places if instead.get(at, "") is not None]

    def kept(self, steps, name=None):
        """Which of `steps`, those of the scenario `name`, the tree is
        taken through: their places in `steps`, counted from 0."""
        return [at for at, _ in self.taken(steps, name)]

    def steps(self, steps, name=None):
        """The steps of `steps`, those of the scenario `name`, that the
        tree is taken through, each as the tree is taken through it."""
        return tuple(step for _, step in self.taken(steps, name))


def found(steps, some):
    """Where `steps` has the steps of `some` one behind the other: the
    places, none of them within another."""
    places, at = [], 0
    while at + len(some) <= len(steps):
        if tuple(steps[at:at + len(some)]) == tuple(some):
            places.append(at)
            at += len(some)
        else:
            at += 1
    return places


_NO_MAP = ("it shows the map of the memory, Alt+F5, and the programs "
           "that stay in the memory listed and released there: the "
           "portable C has no such map and keeps no such list")
_NO_CALL = ("it shows the program asked for its data by another "
            "program, by the call of DOS that is the program's own, as "
            "the original's setup program asks: the portable C has the "
            "setup in itself and answers no such call")
_FDAPM = '"a:\\freedos\\bin\\fdapm" Space "apmdos" Enter'
_NO_SETUP_YET = ("the portable C has no setup yet: what the scenario "
                 "shows of it is still to be translated")
# The two dialogs of the setup that three settings have gone from in the
# portable C, and what stands where in them now: see in_place_in below.
_THREE_TABS = ("the line of the screen saver's characters is the fourth "
               "field of Screen and Mouse, with Screen output gone: three "
               "times Tab from the first")
_LEFT_HANDED = ("Left-handed mouse is five rows further up in Screen and "
                "Mouse, where Screen output has gone from above it")

# What the original calls itself and what the portable C does
# (DEVIATIONS.md, 10), as the rules of that tree have them: see `rules`
# below.
_TITLE = "The Volkov Commander, Version 4.05"
_NAME = "Vibe Commander 0.1"
_BASED = "based on Volkov Commander 4.05"
_BANNER = (_TITLE + " freeware",
           "Copyright (C) 1991-2000 Vsevolod V. Volkov")
_BANNER_NOW = (_NAME + ", " + _BASED,
               _BANNER[1] + ", (C) 2026 Maksym Motorny")

TREES = {
    # The C translation is the main program and nothing else.
    "c": Declared(
        no_setup="the setup program is not translated in the C "
                 "translation, which is the main program alone, finished "
                 "and frozen: the original's is a file of its own, which "
                 "that tree has not",
    ),
    # The portable C stays in the memory whole while a command runs, and
    # has no Memory allocation in its Options menu; it has no map of
    # the memory, Alt+F5, and no watch on the programs that stay in the
    # memory; and it has the setup program in itself, an item of its
    # Options menu, so it answers no call of DOS of its own and does not
    # take INT 21h.
    "portable_c": Declared(
        not_run={
            "memory": "it switches Memory allocation off and shows the "
                      "menu without its mark: the portable C has no such "
                      "option",
            "passed-on": "it shows what a second copy of the program "
                         "hands on of the calls of DOS that pass it, by a "
                         "program that stays in the memory between the two "
                         "copies: the portable C does not take INT 21h, "
                         "and no call of DOS passes it. Nor has the "
                         "machine of the tests the memory for a second "
                         "copy behind that program, with all of each copy "
                         "kept",
            "memory-map": _NO_MAP,
            "memory-map-russia": _NO_MAP,
            "mouse-memory": _NO_MAP,
            "stay": _NO_MAP + ". What it has besides, a program that asks "
                    "the program for its data, the portable C does not "
                    "answer either: see asked",
            "asked": _NO_CALL,
            "handed-on": "it shows how the program hands on the calls of "
                         "DOS that pass it, by a program that puts itself "
                         "behind the program's entry of INT 21h: the "
                         "portable C does not take that interrupt, and no "
                         "call of DOS passes it",
            # The scenarios of the setup program, until the tree has the
            # code that each shows: an entry goes with the commit that
            # makes its scenario pass (TODO.md, "The setup program in the
            # portable C: the plan"). None waits now.
        },
        left_out={
            "F9 o m": "Memory allocation switched off or on: the Options "
                      "menu of the portable C has no such line, and M "
                      "chooses another there, More setup",
        },
        left_out_of={
            "menu-items": {
                ("F9 c i", "Esc"):
                    "Memory info of the Commands menu, and the map left: "
                    "the menu has no line that I chooses, and stays open",
            },
            "mouse-middle": {
                ("Alt+F5", "MiddleClick@30,8"):
                    "the map of the memory, and the middle button on a "
                    "line of it: Alt+F5 is no key of the portable C",
            },
            "mouse-drag-lists": {
                (_FDAPM, "Alt+F5", "Click@30,14>30,3*4",
                 "Click@30,8>30,19*6", "RightClick@30,13>30,14>30,15",
                 "Click@30,14 Click@30,14>30,10", "Click@30,4>47,20",
                 "Alt+F5", "Click@30,4>30,1"):
                    "a program that stays in the memory, and a button of "
                    "the mouse moved in the map of the memory",
            },
            "ended": {
                ("Alt+F5", "Esc"):
                    "a look at the map of the memory, for what has stayed",
            },
        },
        # The program's name (DEVIATIONS.md, 10). The original has its
        # title or its banner on a thousand screens of the scenarios,
        # behind the panels or beside them, in the Info and in the help,
        # and a menu, a box or a panel covers a part of it on many: each
        # text is a Part, known by twelve of its characters that show.
        rules=(
            # The banner, which the program prints when it starts and
            # which stays on the screen behind the panels, grey on black
            # as DOS writes: its first line is the name with the version
            # and what it is based on, where the original has its title
            # and the word of how it was distributed, and is seven cells
            # longer.
            Part("the first line of the banner",
                 was=_BANNER[:1], now=_BANNER_NOW[:1], colour=0x07),
            # Its second line has the copyright of this program behind
            # that of the original's author, 25 cells more. The lines are
            # rules of their own, since the second shows without the
            # first: at the top of the screen the bar of the menus covers
            # the first, or it has gone off the top with what DOS has
            # shown moved up a row.
            Part("the second line of the banner",
                 was=_BANNER[1:], now=_BANNER_NOW[1:], colour=0x07),
            # Behind a panel on the left with none on the right, only
            # the ends of the two lines show, three letters and two,
            # which is nothing to know them by: one screen, of the
            # scenario banner, which has the two rows in its patch.
            # The title of the Info panel, in the middle of its second
            # row, in the panel on the left or on the right: the name and
            # the version, 18 cells in the middle where the original's
            # title has 34.
            Part("the title of the Info panel",
                 was=(_TITLE,), now=(_NAME.center(len(_TITLE)),),
                 colour=0x1B, columns=(3, 43)),
            # The title of the help's window, over the list of its
            # pages, in the middle of the 62 columns of the window's
            # text: the name, the version and what it is based on, 50
            # cells, where the original's title is the 34 in the middle
            # of those.
            Part("the title of the help's window",
                 was=(_TITLE.center(50),), now=(_BANNER_NOW[0],),
                 colour=0x30, columns=(15,)),
        ),
        in_place_of={
            SETUP_COMMAND: (SETUP_ITEM,
                            "the setup is no program of its own in the "
                            "portable C, and no command starts it: it is "
                            "an item of the Options menu, More setup..."),
        },
        # The setup has no setting for what the program has no more:
        # the Configuration is without Memory allocation and TSR
        # manager, and Screen and Mouse without Screen output
        # (DEVIATIONS.md, 1, 7 and 5). What stood below them in the
        # right half of each box stands further up, and the fields go
        # to each other as they stand: a scenario gets to the same
        # setting by fewer keys or by a click further up, and a step
        # that set one of the three is left out. The rows that then
        # differ are in the scenario's patch.
        in_place_in={
            "setup-config": {
                ("Down", "Space", "Down Space", "Down Space"):
                    (("Down", None, "Space", None),
                     "below the last box of the left half is Quick "
                     "execute commands, the one box of Execute options "
                     "now: it is ticked where the cursor is, and Memory "
                     "allocation above it and TSR manager below it are "
                     "not there to be unticked"),
            },
            "setup-config-mouse": {
                ("Click@42,6",):
                    (("Click@42,5",),
                     "Quick execute commands is a row further up, where "
                     "Memory allocation was"),
                ("Click@42,10",):
                    (("Click@42,8",),
                     "Alt alone selects menu is two rows further up, "
                     "with the other Keyboard options"),
            },
            "setup-screen": {
                ("Right", "Space", "Left", "Right Down Down Down", "Space",
                 "Up", "Down Down"):
                    (("Right", None, "Left", "Right", "Space", "Up",
                      "Right Up"),
                     "right of Screen colors is Zooming boxes, where "
                     "Screen output was, which is not there to have "
                     "Synchronized access chosen or to be gone down "
                     "through; above Zooming boxes, the first of the "
                     "right half now, is the last line of Screen blank "
                     "delay, as it was above Screen output, and from "
                     "there the line of characters is right, at "
                     "Left-handed mouse, and up"),
                ("Left Left", "Down Space"):
                    (("Left Left", "Down Down Down Down Space"),
                     "left of the line of characters is Screen colors, "
                     "which it stands beside now, and 20 minutes is "
                     "three lines further down from there than from the "
                     "first line of Screen blank delay"),
                ("Tab Tab Tab Tab",): (("Tab Tab Tab",), _THREE_TABS),
                ("Click@45,6",):
                    ((None,),
                     "Asynchronized access of Screen output, which is "
                     "not there to be clicked"),
                ("Click@45,10",):
                    (("Click@45,5",),
                     "Zooming boxes is five rows further up, where "
                     "Screen output has gone from above it"),
                ("Click@62,11",): (("Click@62,6",),
                                   "the line of characters is five rows "
                                   "further up, as Zooming boxes is"),
                ("Click@60,11",): (("Click@60,6",), "the same line"),
                ("Click@64,11",): (("Click@64,6",), "the same line"),
                ("Click@45,14",): (("Click@45,9",), _LEFT_HANDED),
                ("Up Up Up",):
                    (("Up Up",),
                     "above Left-handed mouse are the line of "
                     "characters and Zooming boxes, and no Screen "
                     "output above that: Left from Zooming boxes is "
                     "Screen colors, as it was from Screen output"),
            },
            "setup-options-russia": {
                ('Tab Tab Tab Tab Alt+RightShift "abcde"',):
                    (('Tab Tab Tab Alt+RightShift "abcde"',), _THREE_TABS),
            },
            "setup-left-handed": {
                ("Click@45,14",): (("Click@45,9",), _LEFT_HANDED),
            },
            "setup-help-screens": {
                ("Tab Tab Tab Tab",): (("Tab Tab Tab",), _THREE_TABS),
            },
        },
    ),
}


def ruled(screen, rule):
    """Whether `rule`, a rule of a tree, expects anything of `screen` of
    the original that the original does not show there."""
    return expected(screen, (rule,)).cells != screen.cells


def declared(tree):
    """What the tree `tree` is declared to do otherwise: nothing, for the
    original, None, and for a tree that is not in TREES."""
    return TREES.get(tree, Declared())
