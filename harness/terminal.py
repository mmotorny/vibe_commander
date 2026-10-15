#!/usr/bin/env python3
"""Hold what the program for Linux draws in a terminal to its grid.

The piece L4 of step 6 (TODO.md, "L4, the terminal"): the program for
Linux draws its screen of 80 by 25 cells in the terminal that it is
started in, by escape sequences of its own, and takes its keys there.
What it draws is held, on the user's word of 14 October 2026 (CLAUDE.md,
step 6), to the grid of cells that the test channel writes, by replaying
it through pyte, a terminal emulator in Python, pinned (harness/deps.py).

The program is run seven times, each in a directory of its own, with
HOME there and no setup file, so that it takes its defaults, in which the
clock is off; the files that it lists are the test's, with fixed times:

  1. Through the test channel, VC_TEST, with the events of KEYS, the
     BIOS's words and the mouse, one step each and a first step with
     none: the grid of each step, and what the program received, by
     VC_TRACE, the keys that it read and every change of what was held
     and of the mouse that it saw. The mouse's pointer is held there to
     the cell with its colours exchanged, as the bench's driver draws it.
  2. In a pseudo-terminal, with the same steps typed as a terminal sends
     them, the keys as xterm sends them and the mouse in SGR's form,
     COLORTERM=truecolor: each step is typed when the program has
     written nothing for QUIET seconds, and what it has written by then
     is the step's screen. pyte's screen is held to the grid cell by
     cell, the character as the VGA's font shows the byte and the colours
     as the VGA's 16 of mode 3 in 24 bits, with blink for the
     attribute's bit 7, and the cursor: where it is, and whether it is
     shown; and what the program received to what it received from the
     channel. The run begins in a terminal of 70 by 20, which is to show
     the line that asks for 80 by 25 and nothing of the program, and is
     then made 80 by 25.
  3. As 2, without COLORTERM: the colours are ANSI's 16, by the codes
     that pyte reads them from. Behind the keys, the program is stopped
     by SIGTSTP, which is to give the terminal back as it was, its modes
     and its own screen, and goes on by SIGCONT, which is to draw it
     whole again: held to the last grid once more.
  4. and 5. As 1 and 2 with the steps of KITTY, in a terminal that
     answers the program's question for the kitty protocol of the
     keyboard: the program is to push the protocol's flags, have every
     key by its escape code, Shift, Ctrl and Alt held and let go among
     them, and pop the flags when it is stopped and at its end.
  6. and 7. The keys of DECODE_KITTY in such a terminal, and those of
     DECODE_LEGACY in one without, each followed by Esc twice, held to
     the BIOS's words that the program reads, without their screens.

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
machine and takes some forty seconds.
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

# The steps: the events of the test channel for each, the BIOS's words and
# the mouse (see sys_posix.c, "The test channel"), and what a terminal
# sends for it. A terminal of these has no kitty protocol: it sends the
# keys as xterm does, and the mouse in SGR's form.
ESC = b"\x1b"
SWITCH = ESC + b"[?1049"        # h the alternate screen, l the own one
QUESTION = ESC + b"[?u"         # whether the kitty protocol is there
PUSH = ESC + b"[>31u"           # the flags that the program asks for
POP = ESC + b"[<u"
MOUSE_ON = ESC + b"[?1000h" + ESC + b"[?1002h" + ESC + b"[?1003h" + ESC \
    + b"[?1006h"
# The end of what gives the terminal back: the mouse's and the focus's
# reports off, and the terminal's own screen.
LEFT = ESC + b"[?1004l" + ESC + b"[?1006l" + ESC + b"[?1003l" + ESC \
    + b"[?1002l" + ESC + b"[?1000l" + ESC + b"[?1049l"
# All of it, which the flags of the kitty protocol are popped in front of.
LEAVE = ESC + b"[?2026l" + ESC + b"[0m" + ESC + b"[0 q" + ESC + b"[?25h" \
    + ESC + b"[?7h" + ESC + b">" + LEFT
KEYS = [
    ("Down", ["5000"], ESC + b"[B"),
    ("Alt-D, the speed search", ["2000"], ESC + b"d"),
    ("F3, the viewer", ["3D00"], ESC + b"OR"),
    ("Esc", ["011B"], ESC),
    ("F9, the menu bar", ["4300"], ESC + b"[20~"),
    ("Right", ["4D00"], ESC + b"[C"),
    ("Enter", ["1C0D"], b"\r"),
    ("Esc", ["011B"], ESC),
    ("Esc", ["011B"], ESC),
    ("Alt-F1, the drive", ["6800"], ESC + b"[1;3P"),
    ("Esc", ["011B"], ESC),
    ("typed text", ["1265", "2E63", "2368", "186F"],
     "echo".encode()),
    ("Backspace", ["0E08"], b"\x7f"),
    ("Ctrl-F1, the left panel", ["5E00"], ESC + b"[1;5P"),
    ("Tab", ["0F09"], b"\t"),
    ("the mouse moved", ["mouse=60,10,0"], ESC + b"[<35;61;11M"),
    ("a click on readme.txt", ["mouse=3,4,1", "asks=20", "mouse=3,4,0"],
     ESC + b"[<0;4;5M" + ESC + b"[<0;4;5m"),
    ("the right button on data.bin, which selects it",
     ["mouse=3,3,2", "asks=20", "mouse=3,3,0"],
     ESC + b"[<2;4;4M" + ESC + b"[<2;4;4m"),
    ("the wheel, which is nothing", [], ESC + b"[<64;4;4M"),
    ("End", ["4F00"], ESC + b"[F"),
    ("Ctrl-PgUp", ["8400"], ESC + b"[5;5~"),
    ("F9", ["4300"], ESC + b"[20~"),
    ("o, Options", ["186F"], b"o"),
    ("m, More setup", ["326D"], b"m"),
    ("F6, the colours", ["4000"], ESC + b"[17~"),
    ("Enter, the grid of colours, half of which blink", ["1C0D"], b"\r"),
]

# The steps of a terminal with the kitty protocol of the keyboard, whose
# flags the program has pushed: every key an escape code, with Shift,
# Ctrl and Alt pressed and let go as keys of their own, the key of a US
# layout behind a key and the text that it types.
# The scan codes of a US keyboard for printable ASCII, from 20h, as the
# program makes them of what a terminal types (Scan of term_posix.c).
US_SCAN = bytes.fromhex(
    "39 02 28 04 05 06 08 28 0A 0B 09 0D 33 0C 34 35"
    "0B 02 03 04 05 06 07 08 09 0A 27 27 33 0D 34 35"
    "03 1E 30 2E 20 12 21 22 23 17 24 25 26 32 31 18"
    "19 10 13 1F 14 16 2F 11 2D 15 2C 1A 2B 1B 07 0C"
    "29 1E 30 2E 20 12 21 22 23 17 24 25 26 32 31 18"
    "19 10 13 1F 14 16 2F 11 2D 15 2C 1A 2B 1B 29")


def typed(text):
    """The BIOS's words of a text typed on a US keyboard, and its bytes,
    with Enter behind it: in pieces of eight, which the program takes
    one after the other, since its buffer, as the BIOS's, drops what is
    beyond 16 keys."""
    words = [f"{US_SCAN[ord(c) - 0x20]:02X}{ord(c):02X}" for c in text]
    data = text.encode() + b"\r"
    return words + ["1C0D"], [data[i:i + 8] for i in range(0, len(data), 8)]


# Commands (L6), each typed at the command line and run by Enter, as the
# steps above: what the terminal's own screen is to show of them, in
# order, is OWN_LINES, the prompt and the command, and what the command
# writes.
COMMANDS = [
    ("a command", *typed("echo hello")),
    ("a path of DOS's in a command",
     *typed(r"test -f H:\HERE\README.TXT && echo found")),
    ("a command that the shell does not find", *typed("nosuchcmd")),
]
OWN_LINES = [
    "H:\\HERE>echo hello", "hello",
    r"H:\HERE>test -f H:\HERE\README.TXT && echo found", "found",
    "H:\\HERE>nosuchcmd", "nosuchcmd: not found",
]

CSI = ESC + b"["
KITTY = [
    ("Alt alone, pressed and let go at once: the menu bar",
     ["held=08", "held=00"], CSI + b"57443;3u" + CSI + b"57443;1:3u"),
    ("Esc, pressed and let go", ["011B"], CSI + b"27u" + CSI + b"27;1:3u"),
    ("Shift held: the key bar of Shift", ["held=02"], CSI + b"57441;2u"),
    ("Shift let go", ["held=00"], CSI + b"57441;1:3u"),
    ("Ctrl-F1 with Ctrl held: the left panel off",
     ["held=04", "5E00", "held=00"],
     CSI + b"57442;5u" + CSI + b"1;5P" + CSI + b"1;5:3P" + CSI
     + b"57442;1:3u"),
    ("Ctrl-F1 again", ["held=04", "5E00", "held=00"],
     CSI + b"57442;5u" + CSI + b"1;5P" + CSI + b"1;5:3P" + CSI
     + b"57442;1:3u"),
    ("Alt-D on a Russian layout, by the key of a US one: the speed search",
     ["held=08", "2000", "held=00"],
     CSI + b"57443;3u" + CSI + b"1074::100;3u" + CSI + b"1074::100;3:3u"
     + CSI + b"57443;1:3u"),
    ("Esc", ["011B"], CSI + b"27u"),
    ("typed text, by kitty's text, Shift with c",
     ["1265", "held=02", "2E43", "held=00", "2368", "186F"],
     CSI + b"101;;101u" + CSI + b"57441;2u" + CSI + b"99:67;2;67u"
     + CSI + b"99:67;2:3;67u" + CSI + b"57441;1:3u" + CSI + b"104;;104u"
     + CSI + b"111;;111u"),
    ("Backspace, twice", ["0E08", "0E08"], CSI + b"127u" + CSI + b"127u"),
    ("Esc, with the text left", ["011B"], CSI + b"27u"),
    ("Grey + of the keypad: the group to select", ["4E2B"],
     CSI + b"57413;;43u"),
    ("Esc", ["011B"], CSI + b"27u"),
    ("Right, a key repeated: twice", ["4D00", "4D00"],
     CSI + b"1;1C" + CSI + b"1;1:2C" + CSI + b"1;1:3C"),
    ("a click on zero, pressed and let go at once",
     ["mouse=44,5,1", "asks=20", "mouse=44,5,0"],
     CSI + b"<0;45;6M" + CSI + b"<0;45;6m"),
]

# Keys whose BIOS's words are held without their screens: each is typed
# in a terminal with the kitty protocol, and in one without, followed by
# Esc twice, and what the program reads is held to the words. The words
# are the bench's BIOS's, SeaBIOS's, as INT 16h 00h answers them (see
# term_posix.c, "The keys, and the mouse"); [] where it answers nothing.
DECODE_KITTY = [
    (CSI + b"97;;97u", ["1E61"]),
    (CSI + b"97:65;2;65u", ["1E41"]),                   # Shift-A
    (CSI + b"97;65;65u", ["1E41"]),                     # Caps Lock on
    (CSI + b"1092::97;;1092u", []),             # a letter that 437 lacks
    (CSI + b"233::101;;233u", ["1282"]),        # one that it has, on e
    (CSI + b"97;1:2u", ["1E61"]),                       # a repeat
    (CSI + b"97;1:3u", []),                             # a release
    (CSI + b"97;5u", ["1E01"]),                         # Ctrl-A
    (CSI + b"49;5u", []),                               # Ctrl-1
    (CSI + b"50;5u", ["0300"]),                         # Ctrl-2
    (CSI + b"91;5u", ["1A1B"]),                         # Ctrl-[
    (CSI + b"1092::97;3u", ["1E00"]),                   # Alt with ф: Alt-A
    (CSI + b"91;3u", ["1A00"]),                         # Alt-[
    (CSI + b"49;3u", ["7800"]),                         # Alt-1
    (CSI + b"61;3u", ["8300"]),                         # Alt-=
    (CSI + b"32;3u", ["3920"]),                         # Alt-Space
    (CSI + b"1;5A", ["8D00"]),                          # Ctrl-Up
    (CSI + b"1;3A", ["9800"]),                          # Alt-Up
    (CSI + b"3;3~", []),                                # Alt-Del
    (CSI + b"3;5~", ["9300"]),                          # Ctrl-Del
    (CSI + b"23~", ["8500"]),                           # F11
    (CSI + b"24;2~", ["8800"]),                         # Shift-F12
    (CSI + b"13~", ["3D00"]),                           # F3, kitty's
    (CSI + b"1;2P", ["5400"]),                          # Shift-F1
    (CSI + b"27;3u", ["0100"]),                         # Alt-Esc
    (CSI + b"9u", ["0F09"]),                            # Tab
    (CSI + b"9;2u", ["0F00"]),                          # Shift-Tab
    (CSI + b"9;5u", ["9400"]),                          # Ctrl-Tab
    (CSI + b"127;3u", ["0E00"]),                        # Alt-Backspace
    (CSI + b"127;5u", ["0E7F"]),                        # Ctrl-Backspace
    (CSI + b"13;5u", ["1C0A"]),                         # Ctrl-Enter
    (CSI + b"57399;129;48u", ["5230"]),                 # the keypad's 0
    (CSI + b"57423u", ["4700"]),                        # its Home
    (CSI + b"57423;2u", ["4737"]),                      # Shift with it
    (CSI + b"57427u", ["4C00"]),                        # its 5, Num Lock off
    (CSI + b"57427;5u", ["8F00"]),                      # Ctrl with it
    (CSI + b"57410u", ["352F"]),                        # its /
    (CSI + b"57411;5u", ["9600"]),                      # Ctrl with its *
    (CSI + b"57412;3u", ["4A00"]),                      # Alt with its -
    (CSI + b"57414;3u", ["A600"]),                      # Alt with its Enter
    (CSI + b"57376u", []),                              # F13
    (CSI + b"57361u", []),                              # Print Screen
    (CSI + b"57441;2u" + CSI + b"57441;1:3u", []),      # Shift alone
]
DECODE_LEGACY = [
    (CSI + b"23~", ["8500"]),                           # F11
    (CSI + b"24;5~", ["8A00"]),                         # Ctrl-F12
    (CSI + b"1;5A", ["8D00"]),                          # Ctrl-Up
    (CSI + b"1;3B", ["A000"]),                          # Alt-Down
    (CSI + b"3;3~", []),                                # Alt-Del
    (CSI + b"E", ["4C00"]),                             # the keypad's 5
    (CSI + b"Z", ["0F00"]),                             # Shift-Tab
    (ESC + b"OP", ["3B00"]),                            # F1, by SS3
    (CSI + b"[A", ["3B00"]),                            # F1, the console's
    (ESC + b";", ["2700"]),                             # Alt-;
    (ESC + b"\t", ["A500"]),                            # Alt-Tab
    (ESC + b"\x7f", ["0E00"]),                          # Alt-Backspace
    (ESC + b"A", ["1E00"]),                             # Alt-Shift-A
    ("é".encode(), ["0082"]),
    (b"\x01", ["1E01"]),                                # Ctrl-A
    (ESC + b"Ok", ["4E2B"]),                            # the keypad's +,
    (ESC + b"Om", ["4A2D"]),                            # -, *, / and Enter
    (ESC + b"Oj", ["372A"]),                            # in xterm's
    (ESC + b"Oo", ["352F"]),                            # application mode
    (ESC + b"O5j", ["9600"]),                           # Ctrl with its *
    (ESC + b"Ox", ["4838"]),                            # its 8
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


def channel(program, home, here, work, steps, name):
    """The grids of the steps, through the test channel, and what the
    program received there, by VC_TRACE."""
    test = work / name / "test"
    test.mkdir()
    trace = work / f"trace-{name}"
    lines = [""] + [" ".join(words) for _, words, _ in steps]
    (test / "keys").write_text("\n".join(lines) + "\n")
    done = subprocess.run([str(program)], cwd=here,
                          env=environment(home, VC_TEST=str(test),
                                          VC_TRACE=str(trace)),
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          timeout=60)
    data = (test / "screens").read_bytes()
    records = [data[i:i + 4016] for i in range(0, len(data), 4016)]
    if done.returncode != 2 or len(records) != len(steps) + 2:
        sys.exit(f"the test channel, {name}: ended with {done.returncode} "
                 f"and {len(records)} screens, where 2 and {len(steps) + 2} "
                 f"were to be:\n{done.stderr.decode(errors='replace')}")
    return records[:-1], trace.read_text().splitlines()


def cell(record, column, row):
    at = 8 + (row * 80 + column) * 2
    return record[at] | record[at + 1] << 8


def pointer(records, failures):
    """The mouse's pointer, as the bench's driver draws it with the look
    that the program asks for: the cell with its colours exchanged, in
    the middle of the screen first, and moved by "the mouse moved", which
    puts back the cell that it was on."""
    at = [what for what, _, _ in KEYS].index("the mouse moved")
    before, after = records[at], records[at + 1]
    for column, row, want, name in (
            (40, 12, cell(before, 40, 12) ^ 0x7700, "left"),
            (60, 10, cell(before, 60, 10) ^ 0x7700, "drawn")):
        if cell(after, column, row) != want:
            failures.append(f"the test channel: the mouse's pointer is not "
                            f"{name} at column {column} of row {row}: "
                            f"{cell(after, column, row):04X}, where "
                            f"{want:04X} was to be")


def keys_of(steps):
    """The lines of a trace for the keys that the steps' events have."""
    return [f"key {word}" for _, words, _ in steps for word in words
            if "=" not in word]


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
                 name, kitty=False, **env):
        self.pyte = pyte
        self.kitty = kitty              # answers the question for it
        self.answered = 0
        self.trace = work / f"trace-{name}"
        self.master, slave = os.openpty()
        winsize(slave, columns, rows)
        self.before = termios.tcgetattr(slave)
        # The terminal's two screens, as xterm has them: its own, and the
        # alternate one, which the program draws in and which ?1049h
        # clears; pyte has the one, so the stream is divided between two.
        self.screen = pyte.Screen(columns, rows)
        self.stream = pyte.ByteStream(self.screen)
        self.own = pyte.Screen(columns, rows)
        self.own_stream = pyte.ByteStream(self.own)
        self.alternate = False
        self.pending = b""
        self.written = b""
        told_r, told_w = os.pipe()
        ended_r, ended_w = os.pipe()
        self.errors_path = work / f"stderr-{name}"
        self.errors = open(self.errors_path, "wb")
        self.process = subprocess.Popen(
            [sys.executable, "-I", "-c", LEADER, str(told_w), str(ended_w),
             str(program)],
            cwd=here, env=environment(home, VC_TRACE=str(self.trace), **env),
            stdin=slave,
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
            self.feed(data)
            last = time.monotonic()
            asked = self.written.count(QUESTION)
            if self.kitty and asked > self.answered:
                self.answered = asked
                os.write(self.master, ESC + b"[?0u")

    def feed(self, data):
        """What the program writes, into the screen that shows: the
        alternate one from ?1049h, cleared there, and the terminal's own
        from ?1049l. A sequence that the read has cut waits for the
        rest."""
        data = self.pending + data
        self.pending = b""
        while data:
            at = data.find(SWITCH)
            if at < 0:
                keep = 0
                for length in range(1, len(SWITCH) + 1):
                    if data.endswith(SWITCH[:length]):
                        keep = length
                if keep:
                    self.pending = data[-keep:]
                    data = data[:-keep]
                (self.stream if self.alternate
                 else self.own_stream).feed(data)
                return
            end = at + len(SWITCH) + 1
            (self.stream if self.alternate else self.own_stream).feed(
                data[:at])
            if end > len(data):
                self.pending = data[at:]
                return
            self.alternate = data[end - 1:end] == b"h"
            if self.alternate:
                self.screen.reset()
            data = data[end:]

    def type(self, data, quiet=QUIET):
        os.write(self.master, data)
        self.read(quiet)

    def traced(self):
        """What the program has received, by VC_TRACE: its lines."""
        return self.trace.read_text().splitlines()

    def resize(self, columns, rows):
        winsize(self.master, columns, rows)
        self.screen.resize(rows, columns)
        self.own.resize(rows, columns)
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


def held_to(name, terminal, steps, records, trace, failures):
    """Take the terminal through the steps, each screen held to the grid
    of the channel's step, and what the program received to what it
    received from the channel: the same keys, which are the steps'
    words, and the same changes of what is held and of the mouse."""
    for number, (what, _, typed) in enumerate(
            [("the first screen", None, None)] + steps):
        if isinstance(typed, list):
            for piece in typed:
                terminal.type(piece, 0.1)
        elif typed is not None:
            terminal.type(typed)
        failures += compare(f"{name}, step {number + 1}, {what}",
                            terminal.pyte, terminal.screen, records[number],
                            terminal.colours24)[:10]
    have = terminal.traced()
    if [line for line in trace if line.startswith("key ")] != keys_of(steps):
        failures.append(f"the test channel, for {name}: the program does "
                        f"not read the steps' keys:\n{trace}")
    if have != trace:
        failures.append(f"{name}: the program receives otherwise than from "
                        f"the test channel:\n  terminal {have}\n  "
                        f"channel  {trace}")


def ended(name, terminal, failures):
    """SIGTERM, which is to end the program by that signal and give the
    terminal back; and nothing on standard error."""
    status = terminal.end()
    if not (os.WIFSIGNALED(status)
            and os.WTERMSIG(status) == signal.SIGTERM):
        failures.append(f"{name}: SIGTERM does not end the program by "
                        f"that signal: the status is {status:#x}")
    if terminal.modes() != terminal.before:
        failures.append(f"{name}: the terminal's modes are not given back "
                        "at SIGTERM")
    if not terminal.written.endswith(LEFT):
        failures.append(f"{name}: the terminal's own screen, and the "
                        "mouse's reports, are not given back at SIGTERM")
    errors = terminal.errors_path.read_bytes()
    if errors:
        failures.append(f"{name}: the program says on standard error:\n"
                        f"{errors.decode(errors='replace')}")


def run(pyte, program, home, here, work, channelled, colours24, failures):
    records, trace = channelled
    env = {"COLORTERM": "truecolor"} if colours24 else {}
    name = "24 bits" if colours24 else "16 colours"
    columns, rows = (70, 20) if colours24 else (80, 25)
    terminal = Terminal(pyte, program, home, here, work, columns, rows,
                        "24" if colours24 else "16", **env)
    terminal.colours24 = colours24
    terminal.read()
    if colours24:
        text = "\n".join(terminal.screen.display)
        if "needs a terminal" not in text or "70 by 20" not in text:
            failures.append(f"{name}: a terminal of 70 by 20 does not get "
                            f"the line that asks for 80 by 25:\n{text}")
        terminal.resize(80, 25)
    if MOUSE_ON not in terminal.written:
        failures.append(f"{name}: the mouse's reports are not asked for")
    if ESC + b"=" not in terminal.written:
        failures.append(f"{name}: the keypad's application mode is not "
                        "asked for")
    held_to(name, terminal, KEYS, records, trace, failures)
    if PUSH in terminal.written:
        failures.append(f"{name}: the kitty protocol's flags are pushed "
                        "where the terminal has not said that it has it")
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
        if not terminal.written.endswith(LEFT):
            failures.append(f"{name}: the terminal's own screen is not "
                            "given back while the program is stopped")
        terminal.screen.reset()
        os.kill(terminal.pid, signal.SIGCONT)
        terminal.read()
        failures += compare(f"{name}, after SIGCONT", pyte, terminal.screen,
                            records[-1], colours24)[:10]
    ended(name, terminal, failures)


def run_kitty(pyte, program, home, here, work, channelled, failures):
    """A terminal of 24 bits that has the kitty protocol, through KITTY,
    held as run() holds one without; and stopped and gone on, which is to
    pop the protocol's flags and push them again."""
    records, trace = channelled
    name = "kitty"
    terminal = Terminal(pyte, program, home, here, work, 80, 25, name,
                        kitty=True, COLORTERM="truecolor")
    terminal.colours24 = True
    terminal.read()
    if PUSH not in terminal.written or terminal.answered != 1:
        failures.append(f"{name}: the program does not ask for the kitty "
                        "protocol and push its flags when it is there")
    held_to(name, terminal, KITTY, records, trace, failures)
    os.kill(terminal.pid, signal.SIGTSTP)
    terminal.read()
    if not terminal.written.endswith(POP + LEAVE):
        failures.append(f"{name}: the kitty protocol's flags are not popped "
                        "while the program is stopped")
    os.kill(terminal.pid, signal.SIGCONT)
    terminal.read()
    if terminal.written.count(PUSH) != 2:
        failures.append(f"{name}: the kitty protocol's flags are not "
                        "pushed again when the program goes on")
    ended(name, terminal, failures)
    if not terminal.written.endswith(POP + LEAVE):
        failures.append(f"{name}: the kitty protocol's flags are not popped "
                        "at SIGTERM")


def run_commands(pyte, program, home, here, work, channelled, failures):
    """Commands (L6): COMMANDS typed and run, each screen held to the
    channel's grid and what the program received to what it received
    there, and what the terminal's own screen has of them to OWN_LINES;
    Ctrl-O, which is to show that screen until a key and give the program
    neither; and a command ended by Ctrl-C and one stopped by Ctrl-Z,
    which are to be the command's and not the program's."""
    records, trace = channelled
    name = "commands"
    terminal = Terminal(pyte, program, home, here, work, 80, 25, name)
    terminal.colours24 = False
    terminal.read()
    held_to(name, terminal, COMMANDS, records, trace, failures)
    own = [line.rstrip() for line in terminal.own.display]
    at = 0
    for line in own:
        if at < len(OWN_LINES) and OWN_LINES[at] in line:
            at += 1
    if at < len(OWN_LINES):
        failures.append(f"{name}: the terminal's own screen does not have "
                        f"{OWN_LINES[at]!r} where it is to:\n"
                        + "\n".join(own))
    if not terminal.alternate:
        failures.append(f"{name}: the program does not take the terminal "
                        "again behind a command")
    received = len(terminal.traced())
    terminal.type(b"\x0f")
    if terminal.alternate or not terminal.written.endswith(LEFT):
        failures.append(f"{name}: Ctrl-O does not show the terminal's own "
                        "screen")
    terminal.type(b"x")
    if not terminal.alternate:
        failures.append(f"{name}: a key behind Ctrl-O does not give the "
                        "program the terminal again")
    failures += compare(f"{name}, behind Ctrl-O", pyte, terminal.screen,
                        records[-1], False)[:10]
    if len(terminal.traced()) != received:
        failures.append(f"{name}: the program receives Ctrl-O, or the key "
                        f"behind it: {terminal.traced()[received:]}")
    for key, what, state in ((b"\x03", "Ctrl-C", None),
                             (b"\x1a", "Ctrl-Z", "T")):
        terminal.type(b"sleep 9\r")
        child = [line.split()[0] for line in subprocess.run(
            ["ps", "-o", "pid=,args=", "--ppid", str(terminal.pid)],
            stdout=subprocess.PIPE, text=True).stdout.splitlines()
            if "sleep" in line]
        if terminal.alternate or not child:
            failures.append(f"{name}: a command does not have the terminal "
                            f"before {what}")
            break
        terminal.type(key)
        if not terminal.alternate:
            failures.append(f"{name}: the program does not come back "
                            f"behind {what}")
        if terminal.state() != "S":
            failures.append(f"{name}: {what} reaches the program, which "
                            f"is {terminal.state()}")
        gone = not Path(f"/proc/{child[0]}").exists()
        have = None if gone else Path(f"/proc/{child[0]}/stat") \
            .read_text().rsplit(")", 1)[1].split()[0]
        if have != state:
            failures.append(f"{name}: {what} leaves the command "
                            f"{'gone' if gone else have}, where it is to "
                            f"be {state or 'gone'}")
    terminal.type(ESC + b"[B")
    if terminal.traced()[-1:] != ["key 5000"]:
        failures.append(f"{name}: the program does not have its keys "
                        "behind the commands")
    ended(name, terminal, failures)


def decode(pyte, program, home, here, work, kitty, failures):
    """The keys of DECODE_KITTY or DECODE_LEGACY, each with Esc twice
    behind it, held to the words that the program reads."""
    name = "kitty's keys" if kitty else "legacy keys"
    terminal = Terminal(pyte, program, home, here, work, 80, 25,
                        "decode-kitty" if kitty else "decode-legacy",
                        kitty=kitty)
    terminal.read()
    esc = CSI + b"27u" if kitty else ESC
    want = []
    for typed, words in DECODE_KITTY if kitty else DECODE_LEGACY:
        terminal.type(typed, 0.15)
        terminal.type(esc, 0.15)
        terminal.type(esc, 0.15)
        want += [f"key {word}" for word in words] + ["key 011B"] * 2
    have = [line for line in terminal.traced() if line.startswith("key ")]
    if have != want:
        wrong = [f"  {typed!r}: {words}" for typed, words in
                 (DECODE_KITTY if kitty else DECODE_LEGACY)]
        failures.append(f"{name}: the program reads\n  {have}\nwhere it "
                        f"was to read\n  {want}\nfor\n" + "\n".join(wrong))
    ended(name, terminal, failures)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("program", type=Path,
                        help="the program for Linux, posix-asan/vc")
    args = parser.parse_args()
    if not args.program.exists():
        print(f"{args.program} is not there: no clang, the test is skipped")
        sys.exit(SKIPPED)
    pyte = load_pyte()
    program = args.program.resolve()
    failures = []
    with dos.workdir() as top:
        work = Path(top)
        channelled = {}
        for name, steps in (("channel", KEYS), ("channel-kitty", KITTY),
                            ("channel-commands", COMMANDS)):
            (work / name).mkdir()
            home, here = lay_out(work / name)
            channelled[name] = channel(program, home, here, work, steps,
                                       name)
        pointer(channelled["channel"][0], failures)
        for run_name in ("24", "16", "kitty", "decode-kitty",
                         "decode-legacy", "commands"):
            run_dir = work / ("run-" + run_name)
            run_dir.mkdir()
            home, here = lay_out(run_dir)
            if run_name == "kitty":
                run_kitty(pyte, program, home, here, work,
                          channelled["channel-kitty"], failures)
            elif run_name == "commands":
                run_commands(pyte, program, home, here, work,
                             channelled["channel-commands"], failures)
            elif run_name.startswith("decode"):
                decode(pyte, program, home, here, work,
                       run_name == "decode-kitty", failures)
            else:
                run(pyte, program, home, here, work, channelled["channel"],
                    run_name == "24", failures)
    for failure in failures:
        print(failure)
    if failures:
        sys.exit(1)
    print(f"{len(KEYS) + 1} screens held, in 24 bits and in 16 colours, "
          f"{len(KITTY) + 1} with the kitty protocol; "
          f"{len(DECODE_KITTY)} keys of kitty's and {len(DECODE_LEGACY)} "
          "of other terminals; a terminal too small, SIGTSTP, SIGCONT and "
          f"SIGTERM; {len(COMMANDS)} commands, Ctrl-O, and Ctrl-C and "
          "Ctrl-Z in a command")


if __name__ == "__main__":
    main()
