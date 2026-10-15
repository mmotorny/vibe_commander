"""Where a build is meant to behave differently from the original.

Each deviation is declared here, with its reason. A golden is derived from
the original with these differences made to it (see goldens.py), so that it
holds exactly what a build is expected to show.

Figures that follow the size of the program are not held against the
original at all, and are declared here too: see IGNORED, and IGNORED_OF
for those that only one tree shows.

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
        if isinstance(rule, _FOUND):
            continue                    # a mask of its own: masked()
        pattern, text, below = rule
        if text is None:
            continue                    # a mask: see masked()
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


@dataclass(frozen=True)
class WrittenTimes:
    """A mask that goes by the run as well as by the golden: the date and
    the time of a directory that the run has written into, where a panel
    shows them on a line of that directory, `..` or a subdirectory's, on
    the golden's screen. The host moves a directory's time whenever an
    entry is made or removed in it, and DOS does not (the user's word of
    15 October 2026, CLAUDE.md, step 6). Which directory a line is of is
    read from the panel's title above it, the path in its top frame: a
    line whose panel has no title on the screen is left out wherever the
    run has written into any directory. Where the run has written into
    none, nothing is left out."""
    what: str = "the time of a directory that the run wrote into"

    # A line of a directory in a panel, between its frames: in a brief
    # panel's status line, the name and ►SUB-DIR◄ or ►UP--DIR◄, its
    # date and its time; in a full panel, the same between upright lines.
    LINE = re.compile(r"(?:(?<=║)|^)([^║]*(?:UP--DIR|SUB-DIR)[^║]*)")
    # The space in front of each as well: a month or an hour of one digit
    # has one there, which another of two digits takes.
    WHEN = re.compile(r" ?\d{1,2}-\d\d-\d\d| ?\d{1,2}:\d\d[ap]")
    TITLE = re.compile(r"═ ([A-Za-z]:\\[^ ]*) ═")

    def found(self, golden, written):
        """The places, (row, column), that the mask leaves out of the
        screen `golden`, of a run that wrote into the directories
        `written`, paths of the bench in capitals."""
        if not written:
            return
        lines = golden.lines()
        for row, line in enumerate(lines):
            for match in self.LINE.finditer(line):
                start, end = match.span(1)
                words = match.group(1).replace("│", " ").split()
                if not words or self.of(lines, row, start, end, words[0],
                                        written):
                    for when in self.WHEN.finditer(line, start, end):
                        for column in range(*when.span()):
                            yield row, column

    def of(self, lines, row, start, end, name, written):
        """Whether the line at `row`, columns `start` to `end`, of the
        entry `name`, is of a directory in `written`: true where its
        panel's title is not to be seen above it."""
        for above in range(row - 1, -1, -1):
            for title in self.TITLE.finditer(lines[above]):
                if start - 1 <= title.start() and title.end() <= end + 1:
                    path = title.group(1).upper().rstrip("\\")
                    if name == "..":
                        path = path.rsplit("\\", 1)[0]
                    else:
                        path += "\\" + name.upper()
                    if ":" in path and "\\" not in path:
                        path += "\\"
                    return path in written
        return True


@dataclass(frozen=True)
class InfoUnder:
    """A mask of the lines of the Info panel that the rules of text leave
    out whole where they show whole (the room of the drive and what the
    files use, LINUX_MASKS), where a box covers a part of them: the line
    is centred, its figure the host's, so what shows of it beside the box
    stands elsewhere. Known by the panel's title, on the golden's screen,
    and the rows below it that have those lines, the sixth to the eighth:
    the cells between its frames that are in the panel's colours, or in
    what a box's shadow makes of them, are left out."""
    what: str = "the Info panel's lines of the host's figures, in part"

    # What the panel is known by, each with the row that it is on,
    # counted from the title's: the title, the line of the program's
    # name below it, as the original has it or as a rule of the tree has
    # made it of that, the drive's label as far as it shows.
    KNOWN = ((re.compile(r"(╔═+ Info ═+╗)"), 0),
             (re.compile(r"(4\.05  ║|0\.1 {10}║)"), 1),
             (re.compile(r"(: is \S* *║|WORK {6}║)"), 5))
    ROWS = (6, 7, 8)

    def found(self, golden, written):
        lines = golden.lines()
        seen = set()
        for row, line in enumerate(lines):
            for pattern, down in self.KNOWN:
                for known in pattern.finditer(line):
                    # The panel is 40 cells wide, and known by its
                    # right frame, which shows where a box covers the
                    # rest.
                    end = known.end(1)
                    start = max(end - 40, 0)
                    top = row - down
                    if (top, start) in seen:
                        continue
                    seen.add((top, start))
                    at = (row * COLUMNS + end - 1) * 2 + 1
                    frame = golden.cells[at]
                    colours = {frame, frame & 0xF0 | 0x0E}
                    colours |= {shaded(colour) for colour in colours}
                    for below in self.ROWS:
                        if not 0 <= top + below < ROWS:
                            continue
                        for column in range(start + 1, end - 1):
                            at = ((top + below) * COLUMNS + column) * 2 + 1
                            if golden.cells[at] in colours:
                                yield top + below, column


@dataclass(frozen=True)
class DosPrompt:
    """A mask of the prompt that DOS writes when the program has been
    left, a row of a prompt and nothing else with an empty row below it,
    which no screen of the program has: its command line has the key bar
    below it or is the last row. On Linux the shell writes its own, on
    the terminal's screen. The cursor that DOS leaves behind the prompt
    goes with it: where the golden's cursor is on such a row, the build's
    is taken for the golden's."""
    what: str = "the prompt that DOS writes behind the program"

    PROMPT = re.compile(r"^([A-Za-z]:\\[^>]*>) *$")

    def found(self, golden, written):
        lines = golden.lines()
        for row, line in enumerate(lines[:-1]):
            match = self.PROMPT.match(line)
            if match and not lines[row + 1].strip():
                for column in range(*match.span(1)):
                    yield row, column


@dataclass(frozen=True)
class DosShown:
    """A mask of what DOS has shown behind the panels once the run has
    run a command (masked(), `commanded`): on Linux a command writes on
    the terminal's own screen, and the program's copy of DOS's screen has
    the prompts and the commands typed but not what they wrote, so its
    rows are others. Left out are the rows above the command line, the
    row above the key bar or the last row, and of each row a half that
    is not a panel's: a half neither of whose outer cells is of a frame on
    the golden's screen, or a whole row where neither half is. A box
    over DOS's screen is left out with it."""
    what: str = "what DOS has shown behind the panels after a command"

    KEYS = re.compile(r"^1\D{6,7}2")
    FRAME = set("║╔╚╟╠╢╣╗╝╤╧│┌└┐┘├┤")

    def found(self, golden, written):
        lines = golden.lines()
        last = next((row - 1 for row, line in enumerate(lines)
                     if self.KEYS.match(line)), len(lines) - 1)
        for row in range(last):
            line = lines[row]
            for start in (0, 40):
                if line[start] in self.FRAME or \
                        line[start + 39] in self.FRAME:
                    continue
                for column in range(start, start + 40):
                    yield row, column


@dataclass(frozen=True)
class OwnPath:
    """A mask of the place of the program's own files, D:\\VC on the
    bench, which is the directory of its setup file on Linux,
    `$XDG_CONFIG_HOME/vc`, a directory of R: whose path is the run's own
    and LONGER characters longer (posixrun.OWN): where a scenario types a
    path of D:\\VC, the run types that one. A path of the place is masked
    with the field that it stands in, as far as the next text behind its
    spaces or the frame, since what follows it in the field moves: the
    name in the status line of the viewer and the editor, a dialog's line
    with the path typed into it. A box whose text has the path behind
    other words is as wide as that text, centred, and the run's is wider:
    all of it is masked, and as far beside it as it can grow, with a row
    above and below and its shadow. Where the golden's cursor is in what is masked, a path being
    typed there, the build's is taken for the golden's (the user's word
    of 15 October 2026: only a scenario about the program's own files
    has their place masked, and no other shows it)."""
    what: str = "the place of the program's own files, a path of R:"

    PATH = re.compile(r"(?<![A-Za-z0-9])[Dd]:\\[Vv][Cc](?:\\[^ ║│]*)? *")
    LONGER = len("R:\\TMP\\VCxxxxxx\\VC") - len("D:\\VC")

    def spans(self, golden):
        """(row, start, end) of every stretch of a row that is masked on
        the golden's screen."""
        lines = golden.lines()
        for row, line in enumerate(lines):
            for match in self.PATH.finditer(line):
                left = line.rfind("║", 0, match.start())
                right = line.find("║", match.end())
                box = self.box(lines, row, left, right) \
                    if left >= 0 and right >= 0 and \
                    line[left + 1:match.start()].strip() else None
                if box is None:
                    yield row, match.start(), match.end()
                    continue
                # A box may have a row of its colour above its frame and
                # one below, and its shadow below that.
                top, bottom = box
                for at in range(max(top - 1, 0), min(bottom + 3, ROWS)):
                    yield (at, max(left - self.LONGER, 0),
                           min(right + 1 + self.LONGER + 2, COLUMNS))

    @staticmethod
    def box(lines, row, left, right):
        """The rows of the top and the bottom frame of the box whose
        sides are at the columns `left` and `right` of `row`, or None
        where that is no box."""
        top = row - 1
        while top >= 0 and lines[top][left] in "║╟":
            top -= 1
        bottom = row + 1
        while bottom < len(lines) and lines[bottom][left] in "║╟":
            bottom += 1
        if top < 0 or bottom >= len(lines) or \
                lines[top][left] != "╔" or lines[top][right] != "╗" or \
                lines[bottom][left] != "╚" or lines[bottom][right] != "╝":
            return None
        return top, bottom

    def found(self, golden, written):
        for row, start, end in self.spans(golden):
            for column in range(start, end):
                yield row, column


# The masks that are found by a method of their own, `found`.
_FOUND = (WrittenTimes, InfoUnder, DosPrompt, DosShown, OwnPath)


def masked(golden, screen, rules, written=None, commanded=False):
    """The screen `golden` of a golden and the screen `screen` of a
    build, with the cells that a mask among `rules` finds on the
    golden's left out of both, character and colour: a mask is a rule
    with None for its text (Declared.rules), of a tree whose program
    shows there what nothing can expect, a thing of the host's. Every
    match of its pattern on a row is found, and every group of a match
    is left out; with a pattern for the line below, only a row whose
    next one has that. A WrittenTimes among them goes by `written` as
    well, the directories that the run wrote into; it, an InfoUnder and
    a DosPrompt find their cells by themselves, and a DosShown does
    where `commanded`, the run having run a command by this screen."""
    masks = [rule for rule in rules
             if not isinstance(rule, (Part,) + _FOUND)
             and rule[1] is None]
    by_run = [rule for rule in rules if isinstance(rule, _FOUND)]
    if not masks and not by_run:
        return golden, screen
    out = [bytearray(golden.cells), bytearray(screen.cells)]
    lines = golden.lines()
    cursor = screen.cursor
    for rule in by_run:
        if isinstance(rule, DosShown) and not commanded:
            continue
        for row, column in rule.found(golden, written):
            for cells in out:
                at = (row * COLUMNS + column) * 2
                cells[at:at + 2] = MASKED
            if isinstance(rule, DosPrompt) and golden.cursor[1] == row:
                cursor = golden.cursor
        if isinstance(rule, OwnPath):
            column, row = golden.cursor
            if any(row == at and start <= column <= end
                   for at, start, end in rule.spans(golden)):
                cursor = golden.cursor
    for pattern, _, below in masks:
        for row, line in enumerate(lines):
            if below and not (row + 1 < ROWS
                              and below.search(lines[row + 1])):
                continue
            for match in pattern.finditer(line):
                for group in range(1, (pattern.groups or 0) + 1):
                    start, end = match.span(group)
                    for column in range(max(start, 0), end):
                        for cells in out:
                            at = (row * COLUMNS + column) * 2
                            cells[at:at + 2] = MASKED
    return (replace(golden, cells=bytes(out[0])),
            replace(screen, cells=bytes(out[1]), cursor=cursor))


# What a masked cell is made on both screens: a character and a colour
# that the program never shows together.
MASKED = bytes((0, 0))


def compared(screen, tree=None):
    """`screen` with what is not compared blanked out: with `tree`, a
    screen of a build of that tree, without what every build leaves out
    (IGNORED) and what the builds of that tree do (IGNORED_OF); without
    one, a screen of a golden, without what any tree leaves out. When a
    build is held against a golden, both screens are made like this, that
    of the golden too: it may have been derived before a figure was left
    out. A golden that is derived now holds its screens like this
    already, and blanking what is blanked changes nothing."""
    patterns = IGNORED + [pattern for some, its in IGNORED_OF.items()
                          if tree in (None, some) for pattern in its]
    cells = bytearray(screen.cells)
    for row, line in enumerate(screen.lines()):
        for pattern in patterns:
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
# each take a number of the same line. These hold for the builds of every
# tree; what only those of one tree leave out is in IGNORED_OF, below.
IGNORED = [
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

# What only the builds of one tree show and is not compared with the
# original: tree -> patterns, as those of IGNORED. A golden has none of
# these figures, whichever tree it is read for.
IGNORED_OF = {
    "c": [
        # The editor says in its first line how many bytes it has free
        # for the text. That is what the program leaves of the memory,
        # and the translation is larger than the original. An expected
        # number would have to follow the size of the build that is
        # tested; the number says too little about the program to be
        # worth that. The field is the number and the room in front of
        # it.
        #
        # This is of the C translation alone. The portable C has no such
        # figure, nor the word Free behind it (src/portable_c/
        # DEVIATIONS.md, 24): where the golden has them, the rule of that
        # tree for the editor's first line expects spaces, and nothing of
        # that line is left out. That the portable C is held to what it
        # shows there is the user's word of 14 October 2026, with the
        # decision that every figure of free memory goes.
        re.compile(r"^Edit: .* Col \d+( [\d, ]+) Free"),
        # The Info panel says how much memory is free. As the program is
        # configured that is what a command gets: all but the part of
        # the program that stays while one runs, which is larger in the
        # translation than in the original. With Memory allocation
        # switched off the number is what is left with all of the program
        # loaded, and follows the size of the build like the editor's.
        # Either way it is left out, in every scenario. The field is the
        # number only, as much of it as a box in front leaves to see,
        # with the comma or the space that a country has between its
        # thousands: the line is centred, so a number of another length
        # would move the words too, and that is noticed.
        #
        # This is of the C translation alone. The portable C has no such
        # line (src/portable_c/DEVIATIONS.md, 11): where the golden has
        # it, the rule of that tree for the top of the Info panel
        # expects an empty row, cell for cell, and nothing of that row
        # is left out.
        re.compile(r"(\d+(?:[, ]\d{3})*) Bytes Free"),
        # The same line where a box covers its words and the end of the
        # number, as the tree of Alt+F10 does: there is nothing to know
        # the line by but its place. It is the one line of the Info that
        # begins ten cells into the panel, a number of seven cells and
        # two words in 38; the total above it begins nine cells in. The
        # box has three cells of room in front of its frame.
        re.compile(r"^║ {10}(\d(?:[\d, ]*\d)?) {3}║"),
    ],
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
#
# One scenario is held against no golden of the original at all, where the
# original cannot go: `own` below, and OWN, which says which scenario may
# be. Its expectation is the tree's own, recorded from the tree's build
# and read row by row by whoever recorded it ("What a tree is expected to
# show where the original cannot go" at the top of goldens.py).

# The scenarios that a tree may be held in to an expectation of its own,
# recorded from its build, where the original cannot go: `editor-huge`
# alone, on the user's word of 14 October 2026, a text larger than the
# original's editor has room for, when the memory comes from the
# extender (CLAUDE.md, "The portable C"); and `menu-none`, on the user's
# word of the same day, the menu bar where the original opens the menu
# that is none and does not come back. goldens.read refuses any other,
# and the scenarios' test fails for a tree that declares one.
OWN = ("editor-huge", "menu-none")


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
    # Scenarios that the tree alone is taken through, and is held to an
    # expectation of its own in, which is recorded from its build:
    # scenario -> why the original cannot go there. Only those of OWN
    # may be. Neither the original nor any other tree is taken through
    # them (scenarios.not_run), and nothing else is declared of them:
    # no patch, no step left out or taken in place of another, and the
    # rules of the tree are not made to what is its own already.
    own: dict = field(default_factory=dict)
    # Scenarios that the tree is taken through only as far as a step:
    # scenario -> (the number of the first step that it is not taken
    # through, and none behind it, counted from 1 among the scenario's
    # own steps; why). For a scenario of which what comes before that
    # step is worth holding the tree to, and what comes from there on
    # rests on what the tree cannot do yet, or at all.
    cut: dict = field(default_factory=dict)

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
        if name in self.cut:
            # The step's number, counted from 1 as a golden counts them,
            # in the scenario's own steps.
            first = self.cut[name][0]
            places = [at for at in places if at < first - 1]
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
# EGA lines is gone from the portable C (DEVIATIONS.md, 12), and the
# screen of more than 25 rows is not tested there for now, on the user's
# word of 13 October 2026: until a system gives the program such a
# screen (TODO.md, "The tall screen: untested in the portable C until a
# system gives it rows").
_NO_LINES = ("it is about a screen of 50 rows, which the original gets by "
             "EGA lines, Alt+F9: the portable C has no such command, and "
             "nothing else on the bench gives it such a screen. On the "
             "user's word of 13 October 2026 the tall screen is not "
             "tested in the portable C until a system gives it rows")
_TALL = ("EGA lines, Alt+F9, and what the scenario does on the screen of "
         "50 rows before the key gives it 25 again: Alt+F9 is no key of "
         "the portable C")
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
_FIVE_DOWN = "Down Down Down Down Down Space"
_FOUR_DOWN = "Down Down Down Down Space"
_CLEAR_BUFFER = ("Clear keyboard buffer is the fifth box of the Advanced "
                 "options, below Alternative editor, where Quick execute "
                 "commands was: four times Down goes there, and a fifth "
                 "would go on to the buttons")
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
    # take INT 21h; and it has no EGA lines, Alt+F9, which is the one
    # way that a scenario has to a screen of more than 25 rows. And it
    # has no Quick execute commands: every command is run by a
    # COMMAND.COM of its own, and neither the Advanced options nor the
    # setup's Configuration has the setting. And it is a flat program of
    # 32 bits, whose editor has room for a file that the original's has
    # none for.
    "portable_c": Declared(
        not_run={
            "quick": "it switches Quick execute commands on and runs "
                     "commands so, by the interrupt of the COMMAND.COM "
                     "that the program was started from: the portable C "
                     "has neither the setting nor that way of running a "
                     "command",
            "memory": "it switches Memory allocation off and shows the "
                      "menu without its mark: the portable C has no such "
                      "option",
            "passed-on": "it shows what a second copy of the program "
                         "hands on of the calls of DOS that pass it, by a "
                         "program that stays in the memory between the two "
                         "copies: the portable C does not take INT 21h, "
                         "and no call of DOS passes it",
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
            "setup-lines": _NO_LINES + ". It shows the setup over what "
                           "DOS has shown there, and the corner where "
                           "the original's setup program writes its key "
                           "bar a cell too far, which the portable C "
                           "does not do",
            "setup-colors-lines": _NO_LINES + ". It shows the setup's "
                                  "list of colours with room for all 39",
            "setup-help-lines": _NO_LINES + ". It shows the setup's help "
                                "in a window of 38 lines",
            # The editor has room for a file that the original's has
            # none for, on the user's word of 13 October 2026
            # (DEVIATIONS.md, 13). What the scenario alone tested is
            # listed in TODO.md, "The editor's room: untested in the
            # program of 32 bits".
            "editor-big": "it has the editor refuse a file of 381,338 "
                          "bytes, which the original's editor has no room "
                          "for and the editor of the portable C has, in "
                          "the 640 KB of its own that its text lies in: "
                          "the portable C opens the file where the "
                          "original shows its box",
            # The scenarios of the setup program, until the tree has the
            # code that each shows: an entry goes with the commit that
            # makes its scenario pass (TODO.md, "The setup program in the
            # portable C: the plan"). None waits now.
        },
        # On the user's word of 14 October 2026 (DEVIATIONS.md, 13):
        # its expectation is harness/goldens/portable_c/editor-huge.txt.
        own={
            "editor-huge": "it has the editor open a text of 750,264 "
                           "bytes and 75,028 lines, which the original's "
                           "editor has no room for, go to its last line, "
                           "and find a text near its end, where the "
                           "original counts no further than the 65,536th "
                           "line: the original would refuse the file, and "
                           "what the portable C is to show there is its "
                           "own",
            # On the user's word of 14 October 2026 (DEVIATIONS.md, 28):
            # its expectation is harness/goldens/portable_c/menu-none.txt.
            "menu-none": "it has a button on the menu bar moved off the "
                         "names and let go, or moved below the bar, at "
                         "the look after, where the original opens the "
                         "menu that is none from behind its table of the "
                         "menus and does not come back to wait for a key: "
                         "no golden of the original can be derived, and "
                         "what the portable C is to show there, the bar "
                         "left, is its own",
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
                ("F9 c e", "F9 c e"):
                    "EGA lines of the Commands menu, chosen twice, for 50 "
                    "rows and for 25 again: the menu has no line that E "
                    "chooses, and stays open",
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
            # Unsorted of the Left and the Right menu is gone
            # (DEVIATIONS.md, 29): its step is left out, and the menu
            # has no line that U chooses.
            "sort": {
                ("F9 l u",):
                    "Unsorted of the Left menu: the menu has no line that "
                    "U chooses, and stays open",
            },
            # EGA lines, Alt+F9, is gone (DEVIATIONS.md, 12), and with
            # it the one way that a scenario has to a screen of more
            # than 25 rows: where a scenario passes through such a
            # screen on the way, the steps there are left out, from the
            # key that makes it tall to the key that makes it 25 rows
            # again.
            "help-keys": {
                ("Alt+F9", "F1", "End", "F10", "Alt+F9"): _TALL,
            },
            "scroll": {
                ("Alt+F9", "Alt+F9"): _TALL,
            },
            "user-menu": {
                ("Alt+F9", "F2 Enter", "Esc", "Alt+F9"): _TALL,
            },
            "history-drives": {
                ("Alt+F9", "Alt+F8", "Esc", "Alt+F9"): _TALL,
            },
            "menu-panel": {
                ("Ctrl+o", "Alt+F9", "Alt+F9", "Ctrl+o"):
                    _TALL + ". They are the last four steps, with the "
                    "panels switched off before and on again behind: "
                    "the command line left high up by 50 rows",
            },
            "mouse-help": {
                ("Alt+F9", "Click@1,49", "Click@20,30",
                 "Click@20,10>20,48*20", "Click@20,43 Click@20,43",
                 "Click@30,12>30,47*8", "MiddleClick@70,40", "Move@40,40",
                 "Alt+F9"):
                    _TALL + ". They are the last nine steps: the mouse "
                    "in the help below the 25th row",
            },
        },
        # The program's name (DEVIATIONS.md, 10), and with it the top
        # of the Info panel, which has no lines on the memory (11). The
        # original has its title or its banner on a thousand screens of
        # the scenarios, behind the panels or beside them, in the Info
        # and in the help, and a menu, a box or a panel covers a part
        # of it on many: each is a Part, known by twelve of its
        # characters that show.
        rules=(
            # The editor's first line has no figure of free bytes and no
            # word Free behind it (DEVIATIONS.md, 24): from behind the
            # number of the column to the end of the word, which the
            # golden has in BLANK but for the space in front of the word,
            # it is spaces in the colour of the line, as the rest of the
            # line behind it is in both.
            (re.compile(r"^Edit: .* Col \d+(#+ Free)"), " " * 40, None),
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
            # The top of the Info panel, in the panel on the left or on
            # the right, its second row to its fifth with the frame on
            # either side (DEVIATIONS.md, 10 and 11). The original has
            # its title there, the line under it, and two lines on the
            # memory: what the machine has, which is the bench's and the
            # same on every screen, and what is free, a figure that is
            # left out of the comparison and that the golden has in
            # BLANK, seven cells of it. The portable C has its name with
            # the version, 18 cells in the middle where the original's
            # title has 34; what it is based on, in the row where the
            # line was; the line, a row further down; and an empty row.
            # The two figures are yellow in the original, and nothing is
            # in the tree's four rows. A menu over the panel throws its
            # shadow on these rows, and the part is known there too. It
            # is known by its letters and figures, not by its frame or
            # its line, which the foot of any panel has.
            Part("the top of the Info panel",
                 was=("║  " + _TITLE + "  ║",
                      "╟" + "─" * 38 + "╢",
                      "║" + "653,296 Bytes Memory".center(38) + "║",
                      "║" + (BLANK * 7 + " Bytes Free").center(38) + "║"),
                 now=("║" + _NAME.center(38) + "║",
                      "║" + _BASED.center(38) + "║",
                      "╟" + "─" * 38 + "╢",
                      "║" + " " * 38 + "║"),
                 colour=("1Bx40", "1Bx40", "1Bx10 1Ex7 1Bx23",
                         "1Bx11 1Ex7 1Bx22"),
                 now_colour=0x1B, columns=(0, 40), frame="║╟─╢",
                 shaded=True),
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
        # the Configuration is without its Execute options, Memory
        # allocation, Quick execute commands and TSR manager, and
        # Screen and Mouse without Screen output (DEVIATIONS.md, 1, 14,
        # 7 and 5). What stood below them in the right half of each box
        # stands further up, and the fields go to each other as they
        # stand: a scenario gets to the same setting by fewer keys or
        # by a click further up, and a step that set one of the four is
        # left out. The rows that then differ are in the scenario's
        # patch.
        #
        # And the Advanced options of the program are without Quick
        # execute commands (14), the box above Clear keyboard buffer,
        # which most scenarios of the editor untick first: a Down less
        # gets there. The step is put in behind the one that opens the
        # dialog, since the same keys are others elsewhere: in the
        # setup's Configuration they tick the Clock.
        in_place_in={
            **{name: {("F9 o o", step):
                      (("F9 o o", step.replace(_FIVE_DOWN, _FOUR_DOWN)),
                       _CLEAR_BUFFER)}
               for step, names in (
                   (_FIVE_DOWN,
                    ("command-edit", "comspec", "dirinfo", "ended",
                     "sizes", "view-search", "editor-hex", "editor-long",
                     "editor-save", "editor-words", "editor-full",
                     "editor-keys", "editor-hex-keys", "editor-buffer",
                     "editor-dirinfo", "mouse-edit", "start-setup",
                     "start-screen", "start-memory", "setup-delay-file",
                     "setup-help-file", "stay-left", "editor-lines")),
                   (_FIVE_DOWN + " Enter",
                    ("user-menu", "user-menu-run", "view-commands")))
               for name in names},
            "setup-config": {
                ("Down", "Space", "Down Space", "Down Space",
                 "Down Space"):
                    (("Down", None, None, None, "Space"),
                     "below the last box of the left half is Alt alone "
                     "selects menu, the first of the Keyboard options, "
                     "which are all of the right half now: it is "
                     "unticked where the cursor is, and Memory "
                     "allocation, Quick execute commands and TSR "
                     "manager are not there to be unticked or ticked "
                     "on the way"),
            },
            "setup-config-mouse": {
                ("Click@42,6",):
                    ((None,),
                     "Quick execute commands, which is not there to be "
                     "clicked"),
                ("Click@42,10",):
                    (("Click@42,5",),
                     "Alt alone selects menu is five rows further up, "
                     "with the other Keyboard options, at the top of "
                     "the right half"),
            },
            # The Commands menu is two lines shorter, without Memory
            # info and EGA lines (DEVIATIONS.md, 7 and 12), and the
            # items below them are that much further up.
            "mouse-menu": {
                ("Click@30,12",):
                    (("Click@30,10",),
                     "Compare directories, which is dark beside the "
                     "Info, is two rows further up in the Commands menu, "
                     "where Memory info and EGA lines have gone from "
                     "above it: row 12 is Menu file edit there, which a "
                     "click would choose"),
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


# The trees that are of another's source and are declared as that one is,
# with what is declared for them besides: tree -> the tree of the source.
# The program for Linux is of the portable C's source, and what the
# portable C is declared to do otherwise it does too: its scenarios not
# run, its steps, its rules and its patches (goldens.read). What is of
# Linux alone is ON_LINUX, below.
BASE = {"posix": "portable_c"}


def _on_top(base, more):
    """What `base` declares with what `more` declares besides: the
    scenarios not run and the steps of both, and the rules of `base`
    followed by those of `more`."""
    def joined(name):
        return {**getattr(base, name), **getattr(more, name)}

    def by_scenario(name):
        # A scenario that both declare steps of has the steps of both.
        mine, others = getattr(base, name), getattr(more, name)
        return {scenario: {**mine.get(scenario, {}),
                           **others.get(scenario, {})}
                for scenario in {**mine, **others}}
    return replace(
        base, not_run=joined("not_run"), left_out=joined("left_out"),
        left_out_of=by_scenario("left_out_of"), rules=base.rules + more.rules,
        in_place_of=joined("in_place_of"),
        in_place_in=by_scenario("in_place_in"), cut=joined("cut"))


# What the program for Linux shows that nothing can expect, a thing of
# the host's, each left out where the golden has the original's: masks
# (CLAUDE.md, step 6; the user's word of 15 October 2026), rules with
# None for their text (masked()).
LINUX_MASKS = (
    # The drive's letter: the bench's C:, which is H: on Linux, the
    # home directory, in a title, on the prompt, in a dialog's line and
    # in the Info panel.
    (re.compile(r"(?<![A-Za-z0-9])([Cc]):(?=\\|[ >]|$)"), None, None),
    # And in front of a name, where a step has typed a path of the
    # drive's current directory, "c:hello.txt", or has typed into one
    # behind the colon.
    (re.compile(r"(?<![A-Za-z0-9])([Cc]):(?=[A-Za-z0-9_.])"), None, None),
    # And on a prompt of the drive alone, $N$G, "C>".
    (re.compile(r"^(C)>"), None, None),
    # And where the rest of the prompt is written over, as a panel's
    # mini status does when a beep has moved the screen up a row: the
    # letter alone at the start of the row above the key bar, which is
    # the command line's.
    (re.compile(r"^(C)(?![:>])"), None, re.compile(r"^1\D{6,7}2")),
    # And in the setup file's bytes, which the Linux program has saved
    # with H: where the bench's has C: (CLAUDE.md, step 6), where the
    # editor shows them in hex: the drive of the right panel's path, the
    # last byte of the row of 60h (posixrun.SETUP_PATHS).
    (re.compile(r"^ 00000060(?:  [0-9A-F]{2}(?: [0-9A-F]{2}){3}){3}"
                r"  [0-9A-F]{2}(?: [0-9A-F]{2}){2} (43)  .{15}(C) $"),
     None, None),
    # Its label: the bench's C: is WORK, and H: is HOME; behind the
    # drive's letter also where a box covers "Volume in drive".
    (re.compile(r": is (WORK)\b"), None, None),
    # And in the box of Ctrl+F4, in its question and its line, which is
    # as wide as the question.
    (re.compile(r'Change volume label "(WORK)" to'), None, None),
    (re.compile(r"║ (WORK) {26}║"), None, re.compile(r"╚═{31}╝")),
    # And where no more of the line shows than its end, in the Info
    # panel, where it is centred: the label six cells from the frame.
    (re.compile(r"(WORK)(?= {6}║)"), None, None),
    # The drive menu's letters, the bench's A C D and the host's H R,
    # the row between its frames above the bottom one: the box is as
    # wide as its title.
    (re.compile(r"║( {3,}[A-Z](?:   [A-Z])* {3,})║"), None,
     re.compile(r"╚═+╝")),
    # The room of the drive in the Info panel, of the host's file
    # system (L2), and what the files of a directory use there, in
    # clusters of the host's: the whole line, which is centred, so the
    # figure's length moves it.
    (re.compile(r"║(.*total bytes on drive.*)║"), None, None),
    (re.compile(r"║(.*bytes free on drive.*)║"), None, None),
    (re.compile(r"║(.*files? uses? .* bytes in.*)║"), None, None),
    # The same lines where a box covers a part of them.
    InfoUnder(),
    # The time of a directory that the run wrote into.
    WrittenTimes(),
    # The prompt that DOS writes when the program has been left.
    DosPrompt(),
    # What DOS has shown behind the panels, once a command has run.
    DosShown(),
    # The place of the program's own files.
    OwnPath(),
)

# The scenarios that the program for Linux is not taken through, not on
# this platform, each with its reason (the user's word of 15 October
# 2026: a scenario that rests on what only DOS has is not run on Linux).
_RUSSIA = ("Russia, code page 866: the program for Linux has the code "
           "page 437, always (CLAUDE.md, step 6)")
_FLOPPY = ("the diskette drive, which Linux has not: the drives are R: "
           "and H: (CLAUDE.md, step 6)")
_DOS = ("a program of DOS or a command of DOS is what it is about, its "
        "output or the program coming back from it")
LINUX_NOT_RUN = {
    **dict.fromkeys(
        ["stamps-russia", "russian", "clock-russia", "clock-path-russia",
         "clock-night-russia", "attributes-russia", "setup-options-russia"],
        _RUSSIA),
    **dict.fromkeys(["drive-b", "not-ready", "phantom",
                     "diskette-command"], _FLOPPY),
    "device": "a device of DOS's and DOS's critical error, which the "
              "program for Linux is never entered for (L3a)",
    "copy-full": "a disk that is full, which a test on the host cannot "
                 "make without a file system of its own to mount",
    "find": "a text looked for in every file of the drive, those of "
            "STAMPS among them, which have their sizes on the bench in "
            "their directory alone and nothing to read, and on the host "
            "are as long as they say, up to four gigabytes",
    "find-drives": "every fixed drive, where R: is the host's root: the "
                   "search would go through the host's whole file system",
    "label": "making and deleting the drive's label, which the program "
             "for Linux refuses, its labels being ROOT and HOME always "
             "(CLAUDE.md, step 6): from its first label on, every screen "
             "is another",
    "editor-full": "memory that runs out, where Linux gives what the "
                   "machine has (CLAUDE.md, step 6)",
    "copy-big": "memory that runs out, where Linux gives what the "
                "machine has (CLAUDE.md, step 6)",
    **dict.fromkeys(
        ["command", "comspec", "ended", "stay-left", "start-screen",
         "start-memory", "second-copy", "lost", "enter-many",
         "enter-many-selected", "start-setup", "setup-defaults"], _DOS),
    # Of those that run a command, those whose value is the command
    # itself or comes behind a command of DOS's that the host's shell has
    # not, DEL, RD, TIME, ATTRIB (TODO.md, "L3b"): the others are run
    # with what DOS has shown masked behind a command (DosShown), or as
    # far as a step, LINUX_CUT.
    "enter": "Enter on a file that DOS runs, and the commands before and "
             "after it: what it holds is DOS running them",
    "user-menu-run": "the commands of the user menu, whose parameters "
                     "show only in what the command run prints",
    "menu-panel": "a panel's menu after a command, DOS's RD at step 3",
    "setup-missing": "its setup file taken away by a command, its first "
                     "step",
    "setup-missing-saved": "its setup file taken away by a command, its "
                           "first step",
    "setup-save-fails": "its setup file made read-only by a command of "
                        "DOS, ATTRIB, its first step",
    "setup-new": "a command at step 2, behind which all of it is",
    "clock-morning": "the time of day set by DOS's TIME, at step 2, "
                     "which the host's shell has not",
}

# The scenarios that the program for Linux is taken through only as far
# as a step, where what follows rests on what Linux has not or waits on a
# question of the user's: scenario -> (the number of the step, counted
# from 1 as the golden counts them, which is not taken; what is held
# before it). Most stop at a command that the host's shell has not, the
# step at which the program asks for a program to be run (ProgramRun of
# sys_posix.c, which says so on standard error by TestProgramRun of
# machine_posix.c).
# DOS's names of devices are ordinary names of files on Linux, and the
# steps that use them as devices are not run there (CLAUDE.md, step 6,
# the user's word of 15 October 2026).
_DEVICE = ("DOS's names of devices, NUL and CON, which are ordinary names "
           "of files on Linux (CLAUDE.md, step 6)")
LINUX_CUT = {
    "editor-save": (14, f"the editor's errors on saving: at step 14 the "
                        f"text saved to NUL, {_DEVICE}"),
    "help-broken": (24, "a help file that cannot be read: at step 24 "
                        "DOS's DEL"),
    "menu-files": (24, "the menu and extension files edited: at step 24 "
                       "DOS's ATTRIB"),
    "history-drives": (60, "the history of commands and the drive box: "
                           "at step 60 the diskette in A:, which Linux has "
                           "not"),
    "setup-help-file": (22, "the setup's help file: at step 22 DOS's "
                            "CLS"),
    "setup-damaged": (21, "the setup file damaged: at step 21 DOS's "
                          "CLS, and COPY behind it"),
    "user-menu": (62, f"the user menu's lines: at step 62 the menu of a "
                      f"line whose file is NUL.MNU, the device, {_DEVICE}; "
                      "from step 77 its file, which DOS's DEL takes away "
                      "at step 80"),
    "view-commands": (17, "F3 and F4 and their extension files: at step "
                          "17 DOS's DEL"),
    "delete-mask": (19, "a mask of files deleted: at step 19 B:, the "
                        "diskette, which Linux has not"),
    "view-window": (6, "the viewer's window on the Info panel: at step 6 "
                       "a file of STAMPS shown in the Info panel, which on "
                       "the bench has nothing to read and on the host its "
                       "size in zeros, as for `find`"),
    "copy-dirs": (14, f"directories copied, renamed and moved: at step 14 "
                      f"the files of one copied to NUL, {_DEVICE}, after "
                      "which the original's screen is a row higher"),
    "tree-choose": (15, "a directory chosen from the tree: at step 15 the "
                        "diskette in A:, which Linux has not"),
    "copy-ask": (31, "a copy that asks: at step 31 the tree of D:, which "
                     "Linux has not"),
    "panel-keys": (46, "masks, the panels off and on, and commands: at "
                       "step 46 D:, which Linux has not"),
    "stamps": (18, "the time stamps of files: at step 18 the file "
                   "search through C:, which lists a directory as it "
                   "stands on the disk, where the bench has made the "
                   "directories in front of the files, and the program "
                   "for Linux, whose emulation of DOS lists a directory "
                   "by its names when it first reads it, has SUB behind "
                   "the files of STAMPS"),
    "mouse-find": (14, "the mouse in the list of the file search: at step "
                       "14 a search of A:, which Linux has not"),
    "copy": (62, f"a file copied: from step 62 a file of NUL's and one "
                 f"to CON, {_DEVICE}"),
    "filter": (29, "the panel's filter of programs, which on Linux shows "
                   "what the user may execute and not the files of DOS's "
                   "three extensions, here copies of a text (L8): at step "
                   "29 only programs, and all files again behind it"),
    # Of those that went to A: or to D:\VC, the ones that went there for
    # more than files to work on, which the others find in C:\FILES,
    # H:\FILES on Linux (CLAUDE.md, step 6).
    "move": (4, "a file or a directory moved to another drive, D:, which "
                "Linux has not: its other drive is R:, the host's root"),
    "attributes-directory": (19, "the diskette in A:, write-protected, "
                                 "and DOS's critical error for it, which "
                                 "Linux has neither of"),
    "mouse-drive": (5, "the letter A: chosen in the drive box by the "
                       "mouse, where the box of Linux has H and R"),
}

# The steps that the program for Linux is not taken through in one
# scenario, where a scenario uses a name of DOS's devices as the device
# on the way to something else, and leaves no trace of it on DOS: the
# screens after them are the original's (CLAUDE.md, step 6, the user's
# word of 15 October 2026). Where the original's screen is a row higher
# after such a step, as when the BIOS beeps, the steps are not left out
# but the scenario is cut there (LINUX_CUT).
LINUX_LEFT_OUT_OF = {
    "copy": {
        ("F5", '"nul" Enter', "F5", '"con" Enter', "Enter"):
            f"a file copied to NUL, and to CON, refused: {_DEVICE}",
    },
    "delete": {
        ("Shift+F8", '"nul"', "Enter", "Esc"):
            f"NUL deleted by a mask, which DOS finds and cannot delete: "
            f"{_DEVICE}",
    },
}

# What the program for Linux is expected to show otherwise than the
# original where no mask can do, by what the user has decided of Linux:
# the hidden and the system attribute are taken when they are set and
# not kept (CLAUDE.md, step 6), so the mark of a hidden file between
# its name and its extension in a panel, ░, is none, and that of a
# hidden file that is selected, █, the mark of a selected one, ▐. The
# name is eight cells from the frame, the extension three; a text is
# written as the bytes of the code page, ▐ DEh.
LINUX_RULES = (
    (re.compile(r"(?<=[║│][^║│]{8})(░)(?=[^║│]{3}[║│])"), " ", None),
    (re.compile(r"(?<=[║│][^║│]{8})(█)(?=[^║│]{3}[║│])"), "\xDE", None),
)

# The steps that the program for Linux is taken through in place of
# others: a drive of the bench's that Linux has under another letter,
# typed (the design of L3b, which the user decided on 15 October 2026).
_ON_H = "the bench's C: is H: on Linux, the home directory"
LINUX_IN_PLACE_IN = {
    "copy": {
        ('"c:readme.txt" Enter',): (('"h:readme.txt" Enter',), _ON_H),
    },
    "names": {
        ('Space Space "c:hello.txt" Space',):
            (('Space Space "h:hello.txt" Space',), _ON_H),
        ('"c:\\work\\..\\work\\..\\work\\..\\work\\readme.txt"',):
            (('"h:\\work\\..\\work\\..\\work\\..\\work\\readme.txt"',),
             _ON_H),
    },
}

ON_LINUX = Declared(not_run=LINUX_NOT_RUN, rules=LINUX_MASKS + LINUX_RULES,
                    cut=LINUX_CUT, in_place_in=LINUX_IN_PLACE_IN,
                    left_out_of=LINUX_LEFT_OUT_OF)

TREES["posix"] = _on_top(TREES["portable_c"], ON_LINUX)


def ruled(screen, rule):
    """Whether `rule`, a rule of a tree, expects anything of `screen` of
    the original that the original does not show there."""
    return expected(screen, (rule,)).cells != screen.cells


def owner(name):
    """The tree whose own expectation the scenario `name` is held to,
    or None for a scenario of the original (Declared.own)."""
    return next((tree for tree, declared in TREES.items()
                 if name in declared.own), None)


def declared(tree):
    """What the tree `tree` is declared to do otherwise: nothing, for the
    original, None, and for a tree that is not in TREES."""
    return TREES.get(tree, Declared())
