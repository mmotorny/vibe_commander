#!/usr/bin/env python3
"""Hold what the program for Linux draws in a terminal to its grid.

The piece L4 of step 6 (TODO.md, "L4, the terminal"): the program for
Linux draws its screen of 80 by 25 cells in the terminal that it is
started in, by escape sequences of its own, and takes its keys there.
What it draws is held, on the user's word of 14 October 2026 (CLAUDE.md,
step 6), to the grid of cells that the test channel writes, by replaying
it through pyte, a terminal emulator in Python, pinned (harness/deps.py).

The program is run three times in a directory of its own, with HOME
there and no setup file, so that it takes its defaults, in which the
clock is off; the files that it lists are the test's, with fixed times:

  1. Through the test channel, VC_TEST, with the keys of KEYS as the
     BIOS's words, one step each and a first step with none: the grid of
     each step.
  2. In a pseudo-terminal, with the same keys typed as a terminal sends
     them, COLORTERM=truecolor: each key is typed when the program has
     written nothing for QUIET seconds, and what it has written by then is
     the step's screen. pyte's screen is held to the grid cell by cell,
     the character as the VGA's font shows the byte and the colours as
     the VGA's 16 of mode 3 in 24 bits, with blink for the attribute's
     bit 7, and the cursor: where it is, and whether it is shown. The run
     begins in a terminal of 70 by 20, which is to show the line that
     asks for 80 by 25 and nothing of the program, and is then made 80
     by 25.
  3. As 2, without COLORTERM: the colours are ANSI's 16, by the codes
     that pyte reads them from. Behind the keys, the program is stopped
     by SIGTSTP, which is to give the terminal back as it was, its modes
     and its own screen, and goes on by SIGCONT, which is to draw it
     whole again: held to the last grid once more.

Each terminal run ends by SIGTERM, which is to end the program by that
signal with the terminal's modes as they were before it started and its
own screen back. The program is the sanitized build, posix-asan/vc;
anything that the sanitizers say fails the test.

pyte wants wcwidth, the width of a character in a terminal, which is not
fetched: nothing else of it is used, and the download that the user
allowed is pyte. In its place stands WcWidth below, of Python's own
unicodedata, which gives every character that the program draws, the
glyphs of the code page 437, the width 1 that wcwidth gives them.

The test exits with 77, which the build takes for a test that is
skipped, where the program has not been built (no clang). It runs no
machine and takes some fifteen seconds.
"""

import argparse
import errno
import fcntl
import os
import select
import signal
import struct
import subprocess
import sys
import termios
import time
import types
import unicodedata
from pathlib import Path

import deps
import dos

SKIPPED = 77
QUIET = 0.4                     # seconds without output: the step is done
TIMEOUT = 20                    # seconds for a step at the most
STAMP = 1760443200              # the time of the test's files: 14 October
                                # 2025, 12:00 UTC

# The steps: the BIOS's words of each, and what a terminal sends for it.
ESC = b"\x1b"
KEYS = [
    ("Down", [0x5000], ESC + b"[B"),
    ("Alt-D, the speed search", [0x2000], ESC + b"d"),
    ("F3, the viewer", [0x3D00], ESC + b"OR"),
    ("Esc", [0x011B], ESC),
    ("F9, the menu bar", [0x4300], ESC + b"[20~"),
    ("Right", [0x4D00], ESC + b"[C"),
    ("Enter", [0x1C0D], b"\r"),
    ("Esc", [0x011B], ESC),
    ("Esc", [0x011B], ESC),
    ("Alt-F1, the drive", [0x6800], ESC + b"[1;3P"),
    ("Esc", [0x011B], ESC),
    ("typed text", [0x1265, 0x2E63, 0x2368, 0x186F],
     "echo".encode()),
    ("Backspace", [0x0E08], b"\x7f"),
    ("Ctrl-F1, the left panel", [0x5E00], ESC + b"[1;5P"),
    ("Tab", [0x0F09], b"\t"),
    ("End", [0x4F00], ESC + b"[F"),
    ("Ctrl-PgUp", [0x8400], ESC + b"[5;5~"),
    ("F9", [0x4300], ESC + b"[20~"),
    ("o, Options", [0x186F], b"o"),
    ("m, More setup", [0x326D], b"m"),
    ("F6, the colours", [0x4000], ESC + b"[17~"),
    ("Enter, the grid of colours, half of which blink", [0x1C0D], b"\r"),
]

# What the VGA's font shows for the bytes below 20h and for 7Fh; the rest
# is the code page 437 as Python has it.
GLYPHS = (" ☺☻♥♦♣♠•◘○◙"
          "♂♀♪♫☼►◄↕‼¶"
          "§▬↨↑↓→←∟↔▲"
          "▼")

# The VGA's 16 colours of mode 3.
VGA = ["000000", "0000aa", "00aa00", "00aaaa", "aa0000", "aa00aa",
       "aa5500", "aaaaaa", "555555", "5555ff", "55ff55", "55ffff",
       "ff5555", "ff55ff", "ffff55", "ffffff"]
# ANSI's number of the PC's colours 0 to 7.
ANSI = [0, 4, 2, 6, 1, 5, 3, 7]


def wcwidth(character):
    """The width of a character in a terminal, as wcwidth has it: 0 for
    what combines and for the formats, 2 for what East Asian text has
    wide, 1 for the rest, -1 for a control."""
    if unicodedata.category(character) in ("Cc",):
        return -1
    if (unicodedata.combining(character)
            or unicodedata.category(character) in ("Mn", "Me", "Cf")):
        return 0
    if unicodedata.east_asian_width(character) in ("W", "F"):
        return 2
    return 1


def load_pyte():
    sys.modules["wcwidth"] = types.SimpleNamespace(wcwidth=wcwidth)
    sys.path.insert(0, str(deps.fetch(deps.PYTE)))
    import pyte
    return pyte


def glyph(byte):
    if byte < 0x20:
        return GLYPHS[byte]
    if byte == 0x7F:
        return "⌂"
    return bytes([byte]).decode("cp437")


def lay_out(top):
    """A run's directory: HOME, with the setup file's directory there and
    nothing in it, and HERE in it with the files, every time fixed. Each
    run has one of its own, since a run may save the setup there."""
    home = top / "home"
    here = home / "here"
    (home / ".config" / "vc").mkdir(parents=True)
    here.mkdir()
    (here / "readme.txt").write_bytes(
        b"Vibe Commander in a terminal.\r\n" * 40)
    (here / "data.bin").write_bytes(bytes(range(256)) * 8)
    (here / "zero").write_bytes(b"")
    for path in [here / "readme.txt", here / "data.bin", here / "zero",
                 home / ".config" / "vc", home / ".config", here, home]:
        os.utime(path, (STAMP, STAMP))
    return home, here


def environment(home, **more):
    env = {"HOME": str(home), "PATH": os.environ.get("PATH", "/usr/bin"),
           "LANG": "C.UTF-8", "TERM": "xterm-256color",
           "ASAN_OPTIONS": "detect_leaks=0"}
    env.update(more)
    return env


def channel(program, home, here, work):
    """The grids of the steps, through the test channel."""
    test = work / "channel" / "test"
    test.mkdir()
    lines = [""] + [" ".join(f"{word:04X}" for word in words)
                    for _, words, _ in KEYS]
    (test / "keys").write_text("\n".join(lines) + "\n")
    done = subprocess.run([str(program)], cwd=here,
                          env=environment(home, VC_TEST=str(test)),
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          timeout=60)
    data = (test / "screens").read_bytes()
    records = [data[i:i + 4016] for i in range(0, len(data), 4016)]
    if done.returncode != 2 or len(records) != len(KEYS) + 2:
        sys.exit(f"the test channel: ended with {done.returncode} and "
                 f"{len(records)} screens, where 2 and {len(KEYS) + 2} "
                 f"were to be:\n{done.stderr.decode(errors='replace')}")
    return records[:-1]


def winsize(fd, columns, rows):
    fcntl.ioctl(fd, termios.TIOCSWINSZ,
                struct.pack("HHHH", rows, columns, 0, 0))


# Run in the pseudo-terminal as its session's leader, which makes it the
# controlling one, and starts the program in a group of its own that is
# the terminal's foreground: so the program's group is not orphaned, and
# SIGTSTP stops it as a shell's job is stopped. It tells the test the
# program's process on the first pipe that it is named, and how the
# program ended on the second.
LEADER = r"""
import fcntl, os, signal, sys, termios
told, ended = int(sys.argv[1]), int(sys.argv[2])
fcntl.ioctl(0, termios.TIOCSCTTY, 0)
pid = os.fork()
if pid == 0:
    os.close(told)
    os.close(ended)
    os.setpgid(0, 0)
    signal.signal(signal.SIGTTOU, signal.SIG_IGN)
    os.tcsetpgrp(0, os.getpgrp())
    signal.signal(signal.SIGTTOU, signal.SIG_DFL)
    os.execv(sys.argv[3], sys.argv[3:])
os.write(told, b"%d\n" % pid)
os.close(told)
while True:
    _, status = os.waitpid(pid, os.WUNTRACED)
    if os.WIFSTOPPED(status):
        continue
    os.write(ended, b"%d\n" % status)
    break
"""


class Terminal:
    """The program in a pseudo-terminal, and pyte's screen of it."""

    def __init__(self, pyte, program, home, here, work, columns, rows,
                 **env):
        self.pyte = pyte
        self.master, slave = os.openpty()
        winsize(slave, columns, rows)
        self.before = termios.tcgetattr(slave)
        self.screen = pyte.Screen(columns, rows)
        self.stream = pyte.ByteStream(self.screen)
        self.written = b""
        told_r, told_w = os.pipe()
        ended_r, ended_w = os.pipe()
        self.errors = open(work / f"stderr-{len(env)}-{columns}", "wb")
        self.process = subprocess.Popen(
            [sys.executable, "-I", "-c", LEADER, str(told_w), str(ended_w),
             str(program)],
            cwd=here, env=environment(home, **env), stdin=slave,
            stdout=slave, stderr=self.errors, start_new_session=True,
            pass_fds=(told_w, ended_w))
        os.close(slave)
        os.close(told_w)
        os.close(ended_w)
        with os.fdopen(told_r) as told:
            self.pid = int(told.readline())
        self.ended = ended_r

    def read(self, quiet=QUIET):
        """What the program writes, until it has written nothing for
        `quiet` seconds."""
        last = start = time.monotonic()
        while time.monotonic() - last < quiet:
            if time.monotonic() - start > TIMEOUT:
                sys.exit("the program does not stop writing")
            ready, _, _ = select.select([self.master], [], [], 0.05)
            if not ready:
                continue
            try:
                data = os.read(self.master, 65536)
            except OSError as error:
                if error.errno == errno.EIO:
                    return
                raise
            if not data:
                return
            self.written += data
            self.stream.feed(data)
            last = time.monotonic()

    def type(self, data):
        os.write(self.master, data)
        self.read()

    def resize(self, columns, rows):
        winsize(self.master, columns, rows)
        self.screen.resize(rows, columns)
        os.kill(self.pid, signal.SIGWINCH)
        self.read()

    def state(self):
        """Whether the program is stopped, by /proc."""
        text = Path(f"/proc/{self.pid}/stat").read_text()
        return text[text.rindex(")") + 2]

    def end(self):
        """SIGTERM, and how the program ended."""
        os.kill(self.pid, signal.SIGTERM)
        self.read(0.2)
        with os.fdopen(self.ended) as ended:
            status = int(ended.readline())
        self.process.wait(timeout=10)
        self.errors.close()
        return status

    def modes(self):
        return termios.tcgetattr(self.master)


def compare(what, pyte, screen, record, colours24):
    """The places where pyte's screen is not the grid, as lines."""
    wrong = []
    for row in range(25):
        line = screen.buffer[row]
        for column in range(80):
            at = 8 + (row * 80 + column) * 2
            byte, attribute = record[at], record[at + 1]
            fore, back = attribute & 0x0F, attribute >> 4 & 7
            if colours24:
                want_fore, want_back = VGA[fore], VGA[back]
            else:
                want_fore = (pyte.graphics.FG_AIXTERM[90 + ANSI[fore & 7]]
                             if fore & 8 else
                             pyte.graphics.FG_ANSI[30 + ANSI[fore]])
                want_back = pyte.graphics.BG_ANSI[40 + ANSI[back]]
            want = (glyph(byte), want_fore, want_back, bool(attribute & 0x80))
            cell = line[column]
            have = (cell.data, cell.fg, cell.bg, cell.blink)
            if have != want:
                wrong.append(f"{what}: row {row}, column {column}: pyte has "
                             f"{have}, the grid {want} (byte {byte:02X}, "
                             f"colour {attribute:02X})")
    column, row = record[4008], record[4009]
    first, last = record[4010], record[4011]
    shown = not (first & 0x20 or (first & 0x1F) > (last & 0x1F)
                 or row >= 25 or column >= 80)
    have = (screen.cursor.x, screen.cursor.y, not screen.cursor.hidden)
    want = (column, row, True) if shown else (have[0], have[1], False)
    if have != want:
        wrong.append(f"{what}: the cursor: pyte has {have}, the grid {want}")
    return wrong


def run(pyte, program, home, here, work, records, colours24, failures):
    env = {"COLORTERM": "truecolor"} if colours24 else {}
    name = "24 bits" if colours24 else "16 colours"
    columns, rows = (70, 20) if colours24 else (80, 25)
    terminal = Terminal(pyte, program, home, here, work, columns, rows,
                        **env)
    terminal.read()
    if colours24:
        text = "\n".join(terminal.screen.display)
        if "needs a terminal" not in text or "70 by 20" not in text:
            failures.append(f"{name}: a terminal of 70 by 20 does not get "
                            f"the line that asks for 80 by 25:\n{text}")
        terminal.resize(80, 25)
    steps = [("the first screen", None, None)] + KEYS
    for number, (what, _, typed) in enumerate(steps):
        if typed is not None:
            terminal.type(typed)
        failures += compare(f"{name}, step {number + 1}, {what}", pyte,
                            terminal.screen, records[number], colours24)[:10]
    if not colours24:
        os.kill(terminal.pid, signal.SIGTSTP)
        terminal.read()
        for _ in range(50):
            if terminal.state() == "T":
                break
            time.sleep(0.05)
        else:
            failures.append(f"{name}: SIGTSTP does not stop the program")
        if terminal.modes() != terminal.before:
            failures.append(f"{name}: the terminal's modes are not given "
                            "back while the program is stopped")
        if not terminal.written.endswith(b"\x1b[?1049l"):
            failures.append(f"{name}: the terminal's own screen is not "
                            "given back while the program is stopped")
        terminal.screen.reset()
        os.kill(terminal.pid, signal.SIGCONT)
        terminal.read()
        failures += compare(f"{name}, after SIGCONT", pyte, terminal.screen,
                            records[-1], colours24)[:10]
    status = terminal.end()
    if not (os.WIFSIGNALED(status)
            and os.WTERMSIG(status) == signal.SIGTERM):
        failures.append(f"{name}: SIGTERM does not end the program by "
                        f"that signal: the status is {status:#x}")
    if terminal.modes() != terminal.before:
        failures.append(f"{name}: the terminal's modes are not given back "
                        "at SIGTERM")
    if not terminal.written.endswith(b"\x1b[?1049l"):
        failures.append(f"{name}: the terminal's own screen is not given "
                        "back at SIGTERM")
    errors = (work / f"stderr-{len(env)}-{columns}").read_bytes()
    if errors:
        failures.append(f"{name}: the program says on standard error:\n"
                        f"{errors.decode(errors='replace')}")


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("program", type=Path,
                        help="the program for Linux, posix-asan/vc")
    args = parser.parse_args()
    if not args.program.exists():
        print(f"{args.program} is not there: no clang, the test is skipped")
        sys.exit(SKIPPED)
    pyte = load_pyte()
    failures = []
    with dos.workdir() as top:
        work = Path(top)
        (work / "channel").mkdir()
        home, here = lay_out(work / "channel")
        records = channel(args.program.resolve(), home, here, work)
        for colours24 in (True, False):
            run_dir = work / ("24" if colours24 else "16")
            run_dir.mkdir()
            home, here = lay_out(run_dir)
            run(pyte, args.program.resolve(), home, here, work, records,
                colours24, failures)
    for failure in failures:
        print(failure)
    if failures:
        sys.exit(1)
    print(f"{len(KEYS) + 1} screens held, in 24 bits and in 16 colours; "
          "a terminal too small, SIGTSTP, SIGCONT and SIGTERM")


if __name__ == "__main__":
    main()
