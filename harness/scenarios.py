"""Scenarios: what is done to Volkov Commander to see how it behaves.

A scenario starts the program on the test bench (see setups.py) and presses
keys, one step at a time. After the start and after every step the screen
is recorded, once the program has done all that the keys make it do; at the
end, so is everything on the hard disks. What the original records is kept
as the scenario's golden (see goldens.py), and a build is right if it
records the same.

Steps are written as Machine.press() takes them: key names separated by
spaces, "Alt+F1" for keys held together, and text to type in double quotes.

Every scenario gets a machine of its own, in the state the bench was saved
in: the program as its distribution configures it, started in C:\\WORK. A
scenario that wants something else arranges it with its first steps, the
way a user would. It may end anywhere, with the program running or not.
"""
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import coverage
import fat
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
    # The guest's time of day in seconds at the first screen and the last.
    began: float
    ended: float
    # Offset and length in the program file of every instruction that ran,
    # if the run was traced.
    executed: dict = field(default_factory=dict)


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
            screens = [wait(machine, "the start")]
            began = machine.seconds()
            for number, step in enumerate(scenario.steps, 1):
                machine.resume()
                machine.press(step)
                screens.append(wait(machine, f"step {number}, {step}"))
            ended = machine.seconds()
            disks = machine.disks
        # The machine has been shut down: the disk images are complete.
        result = Run(screens, {drive: fat.read_tree(path)
                               for drive, path in disks.items()},
                     began, ended)
        if trace:
            result.executed = coverage.executed(log, program.contents)
    return result


def _settled(machine, where):
    try:
        return machine.wait_for_screen(SETTLE, STEP_TIME)
    except MachineError as error:
        raise MachineError(f"after {where}: {error}") from None


def _signalled(machine, where):
    """Wait for the program to signal that it waits for a key, or that it
    has ended, and return the screen. Leaves the machine paused."""
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
        return machine.wait_for_screen(DOS_SETTLE, STEP_TIME)
    machine.pause()
    return machine.screen()
