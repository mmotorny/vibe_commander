"""A disposable DOS PC: FreeDOS booted in QEMU from disks built on the fly.

Needs `qemu-system-i386` and `mtools` on the PATH.
"""
import calendar
import json
import os
import re
import socket
import struct
import subprocess
import time
import zipfile
from dataclasses import dataclass, replace
from pathlib import Path

from image import Image

COLUMNS, ROWS = 80, 25
TEXT_MEMORY = 0xB8000             # the colour text screen: character, attribute
BIOS_DATA = 0x400
KEYBOARD_BUFFER = 0x41A           # where it is read, where it is filled

TICK = 65536 / 1193182            # of the BIOS clock, in seconds

# Hard disks are 64 cylinders x 16 heads x 63 sectors: 31.5 MB of FAT16.
GEOMETRY = (64, 16, 63)

# What DOS prints on the serial port once it has booted, just before it runs
# the commands it was given.
BOOTED = b"BOOTED"

# A screen that alternates between two states differing in at most this many
# bytes is showing a blinking indicator, and counts as standing still.
BLINK_BYTES = 4

# How long Shift, Ctrl and Alt are held down before and after the key they
# go with, or on their own, in seconds.
KEY_TIME = 0.02
MODIFIERS = {"ctrl", "alt", "shift"}

# Names for the keys of a US keyboard, as written in scenarios, and the
# QEMU key codes they stand for. Letters, digits and F1-F12 name themselves.
KEYS = {
    "Enter": "ret", "Esc": "esc", "Tab": "tab", "Space": "spc",
    "Backspace": "backspace", "Ins": "insert", "Del": "delete",
    "Home": "home", "End": "end", "PgUp": "pgup", "PgDn": "pgdn",
    "Up": "up", "Down": "down", "Left": "left", "Right": "right",
    "GrayPlus": "kp_add", "GrayMinus": "kp_subtract",
    "GrayStar": "kp_multiply",
    "Ctrl": "ctrl", "Alt": "alt", "Shift": "shift",
}
PUNCTUATION = {
    " ": "spc", "-": "minus", "=": "equal", "[": "bracket_left",
    "]": "bracket_right", ";": "semicolon", "'": "apostrophe",
    "`": "grave_accent", "\\": "backslash", ",": "comma", ".": "dot",
    "/": "slash",
}
SHIFTED = dict(zip('!@#$%^&*()_+{}:"~|<>?', "1234567890-=[];'`\\,./"))

CP437_CONTROLS = ("\u0020\u263a\u263b\u2665\u2666\u2663\u2660\u2022\u25d8"
                  "\u25cb\u25d9\u2642\u2640\u266a\u266b\u263c\u25ba\u25c4"
                  "\u2195\u203c\u00b6\u00a7\u25ac\u21a8\u2191\u2193\u2192"
                  "\u2190\u221f\u2194\u25b2\u25bc")


def timestamp(year, month, day, hour=0, minute=0, second=0):
    """Seconds since the epoch for a wall-clock time as DOS will show it."""
    return calendar.timegm((year, month, day, hour, minute, second))


class FatImage:
    """A FAT disk image, filled through mtools.

    Everything written carries an explicit timestamp, and directories and the
    volume label get `epoch`, so the same inputs give the same image.
    """

    def __init__(self, path, workdir, epoch, partitioned):
        self.path = Path(path)
        self._staging = Path(workdir) / f"{self.path.name}.staging"
        config = Path(workdir) / f"{self.path.name}.mtoolsrc"
        partition = " partition=1" if partitioned else ""
        config.write_text(f'drive x: file="{self.path}"{partition}\n'
                          "mtools_skip_check=1\n")
        self._env = dict(os.environ, MTOOLSRC=str(config), TZ="UTC",
                         SOURCE_DATE_EPOCH=str(epoch))

    @classmethod
    def hard_disk(cls, path, workdir, epoch, label, serial):
        """Create an empty FAT16 hard disk with a single partition."""
        cylinders, heads, sectors = GEOMETRY
        with open(path, "wb") as f:
            f.truncate(cylinders * heads * sectors * 512)
        disk = cls(path, workdir, epoch, partitioned=True)
        geometry = ["-t", str(cylinders), "-h", str(heads), "-s", str(sectors)]
        disk._mtools("mpartition", "-I", *geometry, "x:")
        disk._mtools("mpartition", "-c", "-a", *geometry, "x:")
        disk._mtools("mformat", "-N", serial, "-v", label, "x:")
        return disk

    def _mtools(self, *args):
        done = subprocess.run(args, env=self._env, capture_output=True,
                              text=True)
        if done.returncode:
            raise RuntimeError(f"{' '.join(args)}: {done.stdout}{done.stderr}")

    def mkdir(self, name):
        self._mtools("mmd", f"x:/{name}")

    def write(self, name, data, mtime):
        self._staging.write_bytes(data)
        os.utime(self._staging, (mtime, mtime))
        self._mtools("mcopy", "-o", "-m", str(self._staging), f"x:/{name}")
        self._staging.unlink()

    def delete(self, name):
        self._mtools("mdel", f"x:/{name}")

    def set_size(self, name, size):
        """Make the directory entry of the empty file `name` claim `size`
        bytes. The name must not occur in another directory.

        DOS and Volkov Commander then show a file of that size, which lets
        a small disk hold "files" of any size, as long as nobody reads them.
        """
        base, _, extension = name.partition(".")
        entry = f"{base:<8}{extension:<3}".encode("ascii")
        with open(self.path, "r+b") as image:
            disk = image.read()
            at = disk.find(entry)
            if at < 0 or disk.find(entry, at + 1) >= 0 or at % 32:
                raise RuntimeError(f"cannot find the one entry of {name}")
            image.seek(at + 28)
            image.write(size.to_bytes(4, "little"))


def boot_floppy(path, workdir, freedos_zip, epoch, commands, country=None):
    """Build a FreeDOS boot floppy that runs `commands` once it has booted.

    It is the stock FreeDOS 1.3 boot floppy, with its installer replaced by
    a minimal configuration. Before running the commands DOS reports on the
    serial port that it is up. `country` is a DOS country code and code
    page, which set how dates, times and numbers are written; without it
    DOS uses the conventions of the United States.
    """
    with zipfile.ZipFile(freedos_zip) as archive:
        Path(path).write_bytes(archive.read("144m/x86BOOT.img"))
    floppy = FatImage(path, workdir, epoch, partitioned=False)
    floppy.delete("SETUP.BAT")
    config = [
        "LASTDRIVE=Z",
        "BUFFERS=20",
        "FILES=40",
        r"SHELL=\FREEDOS\BIN\COMMAND.COM \FREEDOS\BIN /E:1024 /P=\FDAUTO.BAT",
    ]
    if country:
        code, codepage = country
        config.append(rf"COUNTRY={code:03},{codepage},"
                      r"\FREEDOS\BIN\COUNTRY.SYS")
    autoexec = [
        "@ECHO OFF",
        r"SET PATH=A:\FREEDOS\BIN",
        f"ECHO {BOOTED.decode()}>COM1",
        *commands,
    ]
    for name, lines in (("FDCONFIG.SYS", config), ("FDAUTO.BAT", autoexec)):
        text = "".join(line + "\r\n" for line in lines)
        floppy.write(name, text.encode("ascii"), epoch)
    return floppy


@dataclass(frozen=True)
class Screen:
    """The 80x25 text screen as it sits in video memory."""
    cells: bytes                  # character, attribute for each cell
    cursor: tuple                 # column, row
    cursor_lines: tuple           # first and last scan line of the cursor
    video_mode: int
    # The cells seen to blink: column, row, and the character and attribute
    # they show in their other state.
    blinking: tuple = ()

    def lines(self):
        """The characters on screen as text, one string per row."""
        chars = self.cells[0::2]
        return ["".join(_glyph(c) for c in chars[r * COLUMNS:(r + 1) * COLUMNS])
                for r in range(ROWS)]

    def describe(self):
        return {
            "video_mode": self.video_mode,
            "cursor": {"column": self.cursor[0], "row": self.cursor[1],
                       "first_line": self.cursor_lines[0],
                       "last_line": self.cursor_lines[1]},
            "blinking_cells": [{"column": c, "row": r}
                               for c, r, *_ in self.blinking],
        }


def _glyph(code):
    if code < 32:
        return CP437_CONTROLS[code]
    if code == 127:
        return "\u2302"
    return bytes([code]).decode("cp437")


class MachineError(Exception):
    """The machine did not do what was expected of it in time."""


class Machine:
    """A PC running in QEMU, controlled over QMP.

    The hard disk images are written to as the guest writes to its disks,
    and can be read once the machine has been shut down. The clock starts
    at `clock` (ISO 8601) on every boot.

    The machine boots from the floppy or, given `state`, carries on from
    where save() left another machine with disks of the same kind. In the
    second case it waits, paused, for resume().

    With `trace` set, QEMU logs to that file every piece of code the guest
    executes, the first time it does so; coverage.py reads the log.

    A reset ends the machine instead of restarting it, and nothing here
    waits without a limit: a guest that crashes or hangs raises
    MachineError.
    """

    def __init__(self, floppy, hard_disks, workdir, clock, trace=None,
                 state=None):
        self._workdir = Path(workdir)
        self._serial = self._workdir / "serial.log"
        self._scratch = self._workdir / "scratch.bin"
        control = self._workdir / "qmp.sock"
        command = [
            "qemu-system-i386", "-machine", "pc", "-cpu", "486", "-m", "16",
            "-nodefaults", "-vga", "std", "-display", "none", "-no-reboot",
            "-rtc", f"base={clock},clock=vm", "-boot", "a",
            "-serial", f"file:{self._serial}",
            "-qmp", f"unix:{control},server=on,wait=off",
            "-drive", f"if=floppy,index=0,format=raw,readonly=on,file={floppy}",
        ]
        for index, disk in enumerate(hard_disks):
            command += ["-drive", f"if=ide,index={index},format=raw,"
                                  f"file={disk}"]
        if trace:
            command += ["-d", "in_asm", "-D", str(trace)]
        if state:
            command += ["-S", "-incoming", f"file:{state}"]
        self._qemu = subprocess.Popen(command)
        self._socket = self._connect(control)
        self._channel = self._socket.makefile("rw")
        self._reply()                                    # greeting
        self._command("qmp_capabilities")
        if state:
            self.until(lambda: self._command("query-status")["status"]
                        != "inmigrate", 10, "the saved state did not load")

    def _connect(self, path):
        connection = socket.socket(socket.AF_UNIX)
        deadline = time.monotonic() + 10
        while True:
            try:
                connection.connect(str(path))
                return connection
            except (FileNotFoundError, ConnectionRefusedError):
                if time.monotonic() > deadline or self._qemu.poll() is not None:
                    self._qemu.kill()
                    raise MachineError("QEMU did not start") from None
                time.sleep(0.002)

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        try:
            self._command("quit")
            self._qemu.wait(timeout=10)
        except (OSError, ValueError, MachineError, subprocess.TimeoutExpired):
            self._qemu.kill()
            self._qemu.wait()
        self._socket.close()

    def _reply(self):
        while True:
            line = self._channel.readline()
            if not line:
                raise MachineError("the machine stopped: it was reset, or "
                                   "QEMU ended")
            message = json.loads(line)
            if "event" not in message:
                return message

    def _command(self, name, **arguments):
        try:
            self._channel.write(json.dumps({"execute": name,
                                            "arguments": arguments}))
            self._channel.flush()
        except OSError:
            raise MachineError("the machine stopped: it was reset, or QEMU "
                               "ended") from None
        reply = self._reply()
        if "error" in reply:
            raise RuntimeError(f"{name}: {reply['error']['desc']}")
        return reply["return"]

    def _memory(self, address, size):
        self._command("pmemsave", val=address, size=size,
                      filename=str(self._scratch))
        return self._scratch.read_bytes()

    def until(self, done, timeout, failure, pause=0.002):
        """Wait for done() to hold; raise MachineError(failure) if it does
        not within `timeout` seconds."""
        deadline = time.monotonic() + timeout
        while not done():
            if time.monotonic() > deadline:
                raise MachineError(failure)
            time.sleep(pause)

    def wait_for_boot(self, timeout=60):
        """Wait until DOS has booted and is about to run its commands."""
        self.until(lambda: BOOTED in self._serial.read_bytes(), timeout,
                    "DOS did not finish booting", pause=0.05)

    def save(self, path):
        """Pause the machine and save its state, for others to start from."""
        self._command("stop")
        self._command("migrate", uri=f"file:{path}")
        self.until(lambda: self._command("query-migrate").get("status")
                    == "completed", 30, "the machine's state was not saved")

    def pause(self):
        self._command("stop")

    def resume(self):
        self._command("cont")

    def seconds(self):
        """The guest's time of day in seconds, as its BIOS counts it."""
        ticks = struct.unpack("<I", self._memory(BIOS_DATA + 0x6C, 4))[0]
        return ticks * TICK

    def _keyboard(self):
        """Where the BIOS keyboard buffer is read and where it is filled."""
        head, tail = struct.unpack("<HH", self._memory(KEYBOARD_BUFFER, 4))
        return head, tail

    def press(self, keys):
        """Press keys, given as in a scenario: names separated by spaces.

        A name is a key ("F3", "Enter", "a"), keys held down together
        ("Alt+F5", "Ctrl+o"), or a text to type in double quotes, which
        may not contain spaces ("readme.txt").

        Each key is seen to arrive in the BIOS keyboard buffer before the
        next is pressed, so none is lost however fast or slow the guest is.
        The buffer holds 15 keys; if the guest does not take them, pressing
        more fails.
        """
        for name in keys.split():
            if name.startswith('"'):
                for char in name.strip('"'):
                    self._chord(_character(char))
            else:
                self._chord([_key(part) for part in name.split("+")])

    def _chord(self, codes):
        """Press the keys `codes` together: Shift, Ctrl and Alt go down
        first and come up last, as under a hand, and long enough for the
        guest to see them held."""
        held = [code for code in codes if code in MODIFIERS]
        keys = [code for code in codes if code not in MODIFIERS]
        if held:
            self._keys(held, True)
            time.sleep(KEY_TIME)
        if keys:
            tail = self._keyboard()[1]
            self._keys(keys, True)
            self._keys(keys[::-1], False)
            self.until(lambda: self._keyboard()[1] != tail, 5,
                       f"the key {'+'.join(codes)} did not arrive: the "
                       "keyboard buffer is full, or the guest is stuck")
        if held:
            time.sleep(KEY_TIME)
            self._keys(held[::-1], False)

    def _keys(self, codes, down):
        self._command("input-send-event", events=[
            {"type": "key", "data": {"down": down,
                                     "key": {"type": "qcode", "data": code}}}
            for code in codes])

    def keys_taken(self):
        """Whether the guest has read every key that was pressed."""
        head, tail = self._keyboard()
        return head == tail

    def screen(self):
        """The text screen as it is now. Pause the machine first if the
        cursor and the cells are to belong together."""
        cells = self._memory(TEXT_MEMORY, COLUMNS * ROWS * 2)
        bios = self._memory(BIOS_DATA, 0x100)
        return Screen(
            cells=cells,
            cursor=(bios[0x50], bios[0x51]),
            cursor_lines=(bios[0x61], bios[0x60]),
            video_mode=bios[0x49])

    def wait_for_screen(self, settle=2.0, timeout=30):
        """Wait until the text screen stands still, then return it.

        The screen stands still when every key pressed has been taken and
        nothing has changed for `settle` seconds, other than by blinking:
        going back and forth between two states that differ in a few cells.
        Blinking is seen as such only if `settle` covers both states. Of
        the two the screen returned is the one with the greater bytes, and
        its `blinking` lists the cells with what they show in the other.

        The machine is left paused; call resume() to go on.
        """
        size = COLUMNS * ROWS * 2
        deadline = time.monotonic() + timeout
        seen = []                       # the states since the last real change
        changed = time.monotonic()
        while True:
            now = time.monotonic()
            cells = self._memory(TEXT_MEMORY, size)
            if cells not in seen:
                if seen and not _blinks(seen + [cells]):
                    seen = []
                    changed = now
                seen.append(cells)
            if now - changed >= settle and self.keys_taken():
                break
            if now > deadline:
                raise MachineError("the screen did not settle")
            time.sleep(0.01)
        shown = max(seen)
        while True:
            self._command("stop")
            if self._memory(TEXT_MEMORY, size) == shown:
                break
            self._command("cont")
            if time.monotonic() > deadline:
                raise MachineError("the screen did not settle")
            time.sleep(0.01)
        other = min(seen)
        blinking = tuple((i // 2 % COLUMNS, i // 2 // COLUMNS,
                          other[i - i % 2], other[i - i % 2 + 1])
                         for i in range(0, size, 2)
                         if shown[i:i + 2] != other[i:i + 2])
        return replace(self.screen(), blinking=blinking)

    def screenshot(self):
        """The display as the video card renders it."""
        self._command("screendump", filename=str(self._scratch))
        return Image.from_ppm(self._scratch.read_bytes())


def _blinks(states):
    """Whether these screens are two that differ in a few cells only."""
    return len(states) == 2 and \
        sum(a != b for a, b in zip(*states)) <= BLINK_BYTES


def _key(name):
    if name in KEYS:
        return KEYS[name]
    if re.fullmatch(r"[a-z0-9]|F\d\d?", name):
        return name.lower()
    if name in PUNCTUATION:
        return PUNCTUATION[name]
    raise ValueError(f"unknown key {name!r}")


def _character(char):
    """The keys to hold down to type `char`."""
    if char in SHIFTED:
        return ["shift", _key(SHIFTED[char])]
    if char.isupper():
        return ["shift", char.lower()]
    return [_key(char)]


