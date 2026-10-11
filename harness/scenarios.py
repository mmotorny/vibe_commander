"""Scenarios: what is done to Volkov Commander to see how it behaves.

A scenario starts the program on the test bench (see setups.py) and presses
keys, one step at a time. After the start and after every step the screen
is recorded once it stands still; at the end, so is everything on the hard
disks. Run on the original and on a candidate, a scenario must give the
same record.

Steps are written as Machine.press() takes them: key names separated by
spaces, "Alt+F1" for keys held together, and text to type in double quotes.

Unless a scenario says otherwise the clock in the corner of the screen is
switched off. Its blinking colon would otherwise make every step wait for
a full blink to be sure that nothing else moves.
"""
import tempfile
from dataclasses import dataclass, field
from pathlib import Path
from types import MappingProxyType

import coverage
import fat
import setups

# How long the screen must stand still before it is recorded, in seconds:
# after the program has started, when the disks have been busy and DOS has
# printed its own lines, and after a step.
START_SETTLE = 2.0
STEP_SETTLE = 0.5


@dataclass(frozen=True)
class Scenario:
    description: str
    steps: tuple = ()
    # VC.INI settings changed from what the distribution ships.
    settings: MappingProxyType = MappingProxyType({"Clock": 0})
    # The country DOS is configured for; the United States if None.
    country: int = None
    settle: float = STEP_SETTLE


@dataclass
class Run:
    """What a scenario showed and left behind."""
    screens: list                 # after the start and after each step
    disks: dict                   # drive letter -> fat.read_tree() of it
    # Offset and length in the program file of every instruction that ran,
    # if the run was traced.
    executed: dict = field(default_factory=dict)


def _settings(**changed):
    return MappingProxyType({"Clock": 0, **changed})


# From C:\WORK into C:\STAMPS, whose files have sizes and time stamps of
# every kind (see setups.py).
_IN_STAMPS = ("Ctrl+PgUp", "Down Down Enter")

_STAMPS_ABROAD = (
    "Down Down Down Down",                # a date and a time before ten
    "Ctrl+a",
    "Esc",
    "Ins",
    "Alt+F7",
    "Enter",
    "Esc",
)

SCENARIOS = {
    "start": Scenario(
        "The start screen, with the settings the distribution ships",
        settings=MappingProxyType({}), settle=START_SETTLE),

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
        settle=2.0,
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

    "stamps": Scenario(
        "Sizes, dates and times of files, wherever they are shown",
        settings=_settings(LeftFull=1),
        steps=_IN_STAMPS + (
            "Down Down Down",             # the status line of a file
            "Ctrl+a",                     # its date and time in full
            "Esc",
            "Down",                       # a date and a time before ten
            "Ctrl+a",
            "Esc",
            "Down Down Down",             # the last date DOS can express
            "Ctrl+a",
            "Esc",
            "Ins Ins",                    # the size of the selected files
            "Alt+F6",                     # the sizes of directories
            "Alt+F7",                     # the file search and its list
            "Enter",
            "Esc",
        )),

    "stamps-germany": Scenario(
        "The same where the day comes first and the clock has 24 hours",
        settings=_settings(LeftFull=1), country=49,
        steps=_IN_STAMPS + _STAMPS_ABROAD),

    "stamps-japan": Scenario(
        "The same where the year comes first",
        settings=_settings(LeftFull=1), country=81,
        steps=_IN_STAMPS + _STAMPS_ABROAD),
}


def run(scenario, program, trace=False):
    """Run `scenario` on `program` (a setups.Program) and return the Run."""
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with setups.test_bench(workdir, program, scenario.settings,
                               scenario.country, log) as machine:
            machine.wait_for_boot()
            screens = [machine.wait_for_screen(START_SETTLE)]
            for step in scenario.steps:
                machine.resume()
                machine.press(step)
                screens.append(machine.wait_for_screen(scenario.settle))
            disks = machine.disks
        # The machine has been shut down: the disk images are complete.
        result = Run(screens, {drive: fat.read_tree(path)
                               for drive, path in disks.items()})
        if trace:
            result.executed = coverage.executed(log, program.contents)
    return result
