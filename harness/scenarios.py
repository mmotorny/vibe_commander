"""Scenarios: what is done to Volkov Commander to see how it behaves.

A scenario starts the program on the test bench (see setups.py) and presses
keys, one step at a time. After the start and after every step the screen
is recorded, once the program has done all that the keys make it do; at the
end, so is everything on the hard disks. What the original records is kept
as the scenario's golden (see goldens.py), and a build is right if it
records the same.

Steps are written as Machine.press() takes them: key names separated by
spaces, "Alt+F1" for keys held together, and text to type in double quotes.
Text may be Russian where the bench has the Russian keyboard, once
"Alt+RightShift" has selected its Cyrillic letters. That is pressed where
the program takes text, and in a step that types some: at the panels Alt
by itself brings up the menu or not, as the timing has it, and a step of
nothing but Shift and Alt leaves the program nothing to signal.

Every scenario gets a machine of its own, in the state the bench was saved
in: the program as its distribution configures it but for the clock, which
is off, started in C:\\WORK. A scenario that wants something else arranges
it with its first steps, the way a user would. It may end anywhere, with
the program running or not.

The machine's clock runs while the harness waits, and it waits longer for
the original than for a build that signals. Nothing on the bench shows the
time of day, so that does not matter. A scenario that does show it, by
switching the clock on or by listing a file it has made, has to be done
with that before the machine's first minute is over.
"""
import re
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import coverage
import fat
import setups
from dos import MachineError

# How long the screen must stand still to count as finished, in seconds,
# where that is all there is to go by: for a program that does not signal,
# and for DOS. It has to cover the longest the program waits before it does
# something by itself, 0.6 s for following the cursor to another directory,
# and both states of the blinking colon of its clock.
SETTLE = 0.8
# The same for what DOS shows when the program has ended.
DOS_SETTLE = 0.3
# How long a step may take before the program counts as stuck, in seconds.
STEP_TIME = 20


@dataclass(frozen=True)
class Scenario:
    description: str
    steps: tuple = ()
    country: str = "us"           # see setups.COUNTRIES


@dataclass
class Run:
    """What a scenario showed and left behind."""
    screens: list                 # after the start and after each step
    disks: dict                   # drive letter -> fat.read_tree() of it
    # Offset and length in the program file of every instruction that ran,
    # if the run was traced.
    executed: dict = field(default_factory=dict)
    # The fewest bytes of its stack that the program had never touched,
    # of all the times it waited for a key; None for a program that does
    # not mark its stack.
    stack_left: int = None


# From C:\WORK into C:\STAMPS, whose files have sizes and time stamps of
# every kind (see setups.py), with the left panel showing them: Full in its
# menu.
_STAMPS = (
    "Ctrl+PgUp",
    "Down Down Enter",
    "F9",
    "Enter",                              # the menu of the left panel
    "Down Enter",                         # Full
)

_STAMPS_STEPS = _STAMPS + (
    "Down Down Down",                     # the status line of a file
    "Ctrl+a",                             # its date and time in full
    "Esc",
    "Down",                               # a date and a time before ten
    "Ctrl+a",
    "Esc",
    "Down Down Down",                     # the last date DOS can express
    "Ctrl+a",
    "Esc",
    "Ins Ins",                            # the size of the selected files
    "Alt+F6",                             # the sizes of directories
    "Alt+F7",                             # the file search and its list
    "Enter",
    "Esc",
)

_CLOCK_STEPS = (
    "F9 o",                               # the Options menu: not ticked
    "l",                                  # the clock, over the panels
    "Ctrl+o",                             # over what DOS shows
    "Ctrl+o",
    "F9 o",                               # on the menu bar, and ticked
    "l",                                  # off again
)

# Files named by hand: with spaces around the name, by paths too long for
# where they are shown, and with a letter that is neither capital nor
# small.
_NAMES_STEPS = (
    "Shift+F3",                           # a file to view
    'Space Space "c:hello.txt" Space',    # on the current drive
    "Enter",                              # C: in capitals, the name small
    "Esc",
    "Shift+F3",                           # by a long path
    '"c:\\work\\..\\work\\..\\work\\..\\work\\readme.txt"',
    "Enter",                              # shortened behind C:\
    "Esc",
    "Shift+F3",                           # and by one from here
    '"..\\work\\..\\work\\..\\work\\..\\work\\hello.txt"',
    "Enter",                              # shortened from its start
    "Esc",
    "Down Down Down",                     # onto a file
    "F5",                                 # a copy of it
    '"under_sc.txt"',                     # with an underscore in its name
    "Enter",                              # the panel lists it
)

SCENARIOS = {
    "start": Scenario(
        "The program as it starts"),

    "panels": Scenario(
        "The file panel in its other modes, the status line and the "
        "drive menu",
        steps=(
            "Down Down Down",             # onto a file: the status line
            "Down Down Down",             # onto the 1 KB file
            "End",
            "Ctrl+l",                     # the Info panel off: two panels
            "Alt+F1",                     # the drive menu of the left panel
            "Esc",
            "Tab",
            "Ctrl+F1",                    # the left panel off and on again
            "Ctrl+F1",
            "Ctrl+l",                     # the Info panel on the left
            "Ctrl+l",
        )),

    "help": Scenario(
        "The help: its index and pages, whose text is bold, underlined "
        "and reversed in places",
        steps=(
            "F1",                         # the index
            "Down Enter",                 # the keyboard reference
            "PgDn",
            "Enter",                      # the next page
            "Esc",
        )),

    "configuration": Scenario(
        "The Configuration dialog, with its frames inside the frame",
        steps=(
            "F9 o Enter",
            "Esc",
        )),

    "editor": Scenario(
        "The editor, as it opens a file",
        steps=(
            "Down Down Down",             # onto a file
            "F4",
            "Esc",
        )),

    "palettes": Scenario(
        "The other two palettes, black and white and laptop, in which a "
        "box throws another shadow",
        steps=(
            "F9 o Enter",
            "Up Space",                   # black and white
            "Enter",
            "F10",                        # a box and its shadow
            "Esc",
            "F9 o Enter",
            "Down Down Space",            # laptop
            "Enter",
            "F10",
            "Esc",
        )),

    "tree": Scenario(
        "The tree of directories, which the other panel follows a moment "
        "after the cursor has come to rest",
        steps=(
            "Ctrl+l",                     # the Info panel off: two panels
            "F9 l t",                     # the tree in the left one
            "Down",                       # the right one follows
            "Down Down",                  # only to where the cursor stops
            "End",
        )),

    "command": Scenario(
        "DOS commands: the program gives up most of its memory while one "
        "runs, and loads itself again afterwards",
        steps=(
            '"ver"',
            "Enter",
            "Ctrl+o",                     # what the command printed
            "Ctrl+o",
            '"mem"',
            "Enter",
            "Ctrl+o",
            "Ctrl+o",
            "Ctrl+PgUp",                  # and the program still works
        )),

    "lost": Scenario(
        "The program file gone when the program wants to load itself "
        "again after a command",
        steps=(
            '"del" Space "d:\\vc\\vc.*"',  # the program, with what it reads
            "Enter",                      # it asks to try again
            "Space",                      # which is not what it asked for
            "Enter",                      # it fails again
            "Esc",                        # and gives up: the DOS prompt
        )),

    "quit": Scenario(
        "Leaving the program, declined once",
        steps=(
            "F10",
            "Esc",
            "F10",
            "Enter",                      # back at the DOS prompt
        )),

    "stamps": Scenario(
        "Sizes, dates and times of files, wherever they are shown",
        steps=_STAMPS_STEPS),

    "stamps-russia": Scenario(
        "The same where the day comes first and the clock has 24 hours",
        country="ru", steps=_STAMPS_STEPS),

    "names": Scenario(
        "Names of files as they are typed: tidied up, shortened to fit, "
        "and with a letter beyond Z",
        steps=_NAMES_STEPS),

    "russian": Scenario(
        "A file with a Russian name, typed on the Russian keyboard",
        country="ru",
        steps=(
            "Down Down Down",             # onto a file
            "F5",                         # a copy of it
            # The keyboard's Cyrillic letters are selected here, where Alt
            # by itself means nothing: at the panels it brings up the
            # menu, if the program sees it pressed and let go in time.
            'Alt+RightShift '
            '"\u043f\u0438\u0441\u044c\u043c\u043e_1.\u0434\u043e\u043a"',
            "Enter",                      # the panel lists it
            "End",                        # the status line names it
            "F3",                         # and so does the viewer
            "Esc",
            "Shift+F3",                   # typed again, in capitals
            '"\u041f\u0418\u0421\u042c\u041c\u041e_1.\u0414\u041e\u041a"',
            "Enter",
            "Esc",
        )),

    "clock": Scenario(
        "The clock in the corner of the screen, which the bench has off",
        steps=_CLOCK_STEPS),

    "clock-russia": Scenario(
        "The same where the clock has 24 hours",
        country="ru", steps=_CLOCK_STEPS),
}


def run(scenario, program, bench, signals, trace=False):
    """Run `scenario` on `program` (a setups.Program) and return the Run.

    `bench` is the setups.Bench for the scenario's country. With `signals`
    the program is taken to be a build that signals where it is, and each
    screen is recorded the moment it says that it waits for a key and has
    nothing left to do by itself. Without, the moment is guessed: when the
    screen has stood still for a while.
    """
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, program, log) as machine:
            wait = _signalled if signals else _settled
            screens, left = [], []
            for number, step in enumerate(("",) + scenario.steps):
                if number:
                    machine.resume()
                    machine.press(step)
                where = f"step {number}, {step}" if number else "the start"
                screen, waiting = wait(machine, where)
                screens.append(screen)
                if waiting:
                    left.append(_stack_left(machine))
            disks = machine.disks
        # The machine has been shut down: the disk images are complete.
        codepage = setups.codepage(scenario.country)
        result = Run(screens, {drive: fat.read_tree(path, codepage)
                               for drive, path in disks.items()})
        if trace:
            result.executed = coverage.executed(log, program.contents)
        if left:
            result.stack_left = min(left)
    return result


# What the build that signals fills its stack with when it has set it up:
# StkMark in port/asm/VC.ASM, low byte first. The stack grows down from its
# top, so what it has never reached is still marked, from the bottom up.
STACK_MARK = b"\x5A\xA5"
CONVENTIONAL_MEMORY = 0xA0000     # where DOS programs live, below this


def _stack_left(machine):
    """How many bytes of its stack the program has never touched: the
    longest run of the mark in the machine's memory. That is 0 once the
    stack has been used to its bottom, or beyond."""
    memory = machine._memory(0, CONVENTIONAL_MEMORY)
    runs = re.finditer(b"(?:%s){2,}" % re.escape(STACK_MARK), memory)
    return max((len(run.group()) for run in runs), default=0)


def _settled(machine, where):
    """Wait for the screen to stand still and return it, and that it is
    not known whether the program waits for a key."""
    try:
        return machine.wait_for_screen(SETTLE, STEP_TIME), False
    except MachineError as error:
        raise MachineError(f"after {where}: {error}") from None


def _signalled(machine, where):
    """Wait for the program to signal that it waits for a key, or that it
    has ended, and return the screen and which of the two: whether it
    waits. Leaves the machine paused if it does."""
    heard = ""

    def idle():
        nonlocal heard
        if not machine.keys_taken():
            return False
        # Every key has been taken, and a key that is taken says K at once.
        # So if the last word now is W, the program waits with nothing in
        # hand.
        heard += machine.signals()
        return heard.endswith(("W", "X"))

    try:
        machine.until(idle, STEP_TIME, "")
    except MachineError:
        machine.pause()
        shown = "\n".join(machine.screen().lines())
        raise MachineError(
            f"after {where}: the program did not come to wait for a key "
            f"within {STEP_TIME} s; it signalled {heard!r} and shows\n"
            f"{shown}") from None
    if heard.endswith("X"):
        # The program has ended; what DOS then shows comes without signals.
        return machine.wait_for_screen(DOS_SETTLE, STEP_TIME), False
    machine.pause()
    return machine.screen(), True
