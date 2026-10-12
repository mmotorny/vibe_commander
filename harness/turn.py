"""A turn at the computer, for a run that has machines.

The project may be worked on in several checkouts on one computer at the
same time, and each runs its machines in QEMU. Where the program says when
it has finished a step, a run only gets slower when others run beside it.
Where it cannot say, the harness waits for the screen to stand still, and
on a busy computer the screen of a machine stands still before the program
has finished: the original then comes out with other goldens, and nothing
says that they are wrong.

So a run takes a turn before it starts a machine, and has it until its
last one has ended:

  alone     for a run that is timed by a still screen. No other run on
            the computer has a turn meanwhile.
  shared    for a run of a build that signals. Any number of them have
            their turns together.

A run that wants to be alone waits for the turns that others have to end,
and those who come after it wait behind it, so that it is not kept waiting
by a row of runs that overlap.

The turn is a lock on a file in /tmp, the same one for every checkout,
which the system takes off when the process ends, however it ends. Next to
it is a log with a line for every turn: when it ended, the tool, its
checkout, alone or shared, how many seconds it waited and how many it had
the turn. Not in the directory that TMPDIR names: that may be another
from one shell to the next, and then nothing would be kept apart.
"""
import fcntl
import os
import sys
import time
from contextlib import contextmanager
from pathlib import Path

import deps

DIRECTORY = Path("/tmp/vibe-commander")
LOCK = DIRECTORY / "turn.lock"
# Whoever wants a turn passes through here one at a time, and waits here
# while the one before waits for the lock.
GATE = DIRECTORY / "turn.gate"
LOG = DIRECTORY / "turn.log"


@contextmanager
def taken(alone):
    """Wait for a turn and have it while the `with` statement lasts:
    `alone`, or shared with other runs that do not ask to be alone."""
    tool = Path(sys.argv[0]).name
    said = []

    def wait(file, kind):
        try:
            fcntl.flock(file, kind | fcntl.LOCK_NB)
        except BlockingIOError:
            if not said:
                print(f"{tool}: waiting for a turn at the computer, which "
                      f"another run has (see {LOG})",
                      file=sys.stderr, flush=True)
                said.append(True)
            fcntl.flock(file, kind)

    DIRECTORY.mkdir(exist_ok=True)
    asked = time.monotonic()
    gate = os.open(GATE, os.O_RDWR | os.O_CREAT, 0o666)
    lock = os.open(LOCK, os.O_RDWR | os.O_CREAT, 0o666)
    try:
        wait(gate, fcntl.LOCK_EX)
        wait(lock, fcntl.LOCK_EX if alone else fcntl.LOCK_SH)
        fcntl.flock(gate, fcntl.LOCK_UN)
        began = time.monotonic()
        try:
            yield
        finally:
            ended = time.monotonic()
            line = (f"{time.strftime('%Y-%m-%d %H:%M:%S')}  {tool}  "
                    f"{deps.ROOT}  {'alone' if alone else 'shared'}  "
                    f"waited {began - asked:.1f} s  "
                    f"held {ended - began:.1f} s\n")
            # One write to a file opened for appending: the lines of
            # several processes do not run into each other.
            with open(LOG, "a") as log:
                log.write(line)
    finally:
        os.close(lock)
        os.close(gate)
