"""A disposable DOS PC: FreeDOS booted in QEMU from disks built on the fly.

Needs `qemu-system-i386` and `mtools` on the PATH.
"""
import calendar
import json
import os
import re
import socket
import subprocess
import time
import zipfile
from dataclasses import dataclass
from pathlib import Path

from image import Image

COLUMNS, ROWS = 80, 25
TEXT_MEMORY = 0xB8000             # the colour text screen: character, attribute
BIOS_DATA = 0x400

# Hard disks are 64 cylinders x 16 heads x 63 sectors: 31.5 MB of FAT16.
GEOMETRY = (64, 16, 63)

# What DOS prints on the serial port once it has booted, just before it runs
# the commands it was given.
BOOTED = b"BOOTED"

# A screen that alternates between two states differing in at most this many
# bytes is showing a blinking indicator, and counts as standing still.
BLINK_BYTES = 4

# How long a key is held down and how long the keyboard then rests, in
# seconds. The BIOS buffers 15 keys, so a step should not type more.
KEY_TIME = 0.03

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
    serial port that it is up. `country` is a DOS country code, which sets
    how dates, times and numbers are written; without it DOS uses the
    conventions of the United States.
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
        config.append(rf"COUNTRY={country:03},,\FREEDOS\BIN\COUNTRY.SYS")
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
    blinking: tuple = ()          # (column, row) of cells seen to blink

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
                               for c, r in self.blinking],
        }


def _glyph(code):
    if code < 32:
        return CP437_CONTROLS[code]
    if code == 127:
        return "\u2302"
    return bytes([code]).decode("cp437")


class Machine:
    """A PC running in QEMU, controlled over QMP.

    The hard disk images are written to as the guest writes to its disks,
    and can be read once the machine has been shut down. The clock starts
    at `clock` (ISO 8601) on every boot.

    With `trace` set, QEMU logs to that file every piece of code the guest
    executes, the first time it does so; coverage.py reads the log.
    """

    def __init__(self, floppy, hard_disks, workdir, clock, trace=None):
        self._workdir = Path(workdir)
        self._serial = self._workdir / "serial.log"
        self._scratch = self._workdir / "scratch.bin"
        control = self._workdir / "qmp.sock"
        command = [
            "qemu-system-i386", "-machine", "pc", "-cpu", "486", "-m", "16",
            "-nodefaults", "-vga", "std", "-display", "none",
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
        self._qemu = subprocess.Popen(command)
        self._socket = socket.socket(socket.AF_UNIX)
        deadline = time.monotonic() + 10
        while True:
            try:
                self._socket.connect(str(control))
                break
            except (FileNotFoundError, ConnectionRefusedError):
                if time.monotonic() > deadline:
                    self._qemu.kill()
                    raise
                time.sleep(0.05)
        self._channel = self._socket.makefile("rw")
        self._reply()                                    # greeting
        self._command("qmp_capabilities")

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        try:
            self._command("quit")
        except (OSError, ValueError):
            self._qemu.kill()
        self._qemu.wait(timeout=10)
        self._socket.close()

    def _reply(self):
        while True:
            message = json.loads(self._channel.readline())
            if "event" not in message:
                return message

    def _command(self, name, **arguments):
        self._channel.write(json.dumps({"execute": name,
                                        "arguments": arguments}))
        self._channel.flush()
        reply = self._reply()
        if "error" in reply:
            raise RuntimeError(f"{name}: {reply['error']['desc']}")
        return reply["return"]

    def _memory(self, address, size):
        self._command("pmemsave", val=address, size=size,
                      filename=str(self._scratch))
        return self._scratch.read_bytes()

    def wait_for_boot(self, timeout=60):
        """Wait until DOS has booted and is about to run its commands."""
        deadline = time.monotonic() + timeout
        while BOOTED not in self._serial.read_bytes():
            if time.monotonic() > deadline:
                raise TimeoutError("DOS did not finish booting")
            time.sleep(0.1)

    def press(self, keys):
        """Press keys, given as in a scenario: names separated by spaces.

        A name is a key ("F3", "Enter", "a"), keys held down together
        ("Alt+F5", "Ctrl+o"), or a text to type in double quotes, which
        may not contain spaces ("readme.txt").
        """
        for name in keys.split():
            if name.startswith('"'):
                for char in name.strip('"'):
                    self._chord(_character(char))
            else:
                self._chord([_key(part) for part in name.split("+")])

    def _chord(self, codes):
        for down in (True, False):
            events = [{"type": "key",
                       "data": {"down": down,
                                "key": {"type": "qcode", "data": code}}}
                      for code in (codes if down else reversed(codes))]
            self._command("input-send-event", events=events)
            time.sleep(KEY_TIME)

    def wait_for_screen(self, settle=2.0, timeout=30):
        """Wait until the text screen stands still, then return it.

        The screen stands still when it has not changed for `settle`
        seconds, other than by blinking. A blinking indicator is seen as
        such only if `settle` covers both of its states.

        The machine is left paused, showing exactly the screen returned, so
        that a screenshot taken next matches it; call resume() to go on.
        """
        cells = COLUMNS * ROWS * 2
        deadline = time.monotonic() + timeout
        samples = []                                     # (time, cells)
        while True:
            now = time.monotonic()
            samples.append((now, self._memory(TEXT_MEMORY, cells)))
            while now - samples[0][0] > settle + 0.5:
                samples.pop(0)
            states = sorted({dump for _, dump in samples})
            if now - samples[0][0] >= settle and _still(states):
                break
            if now > deadline:
                raise TimeoutError("the screen did not settle")
            time.sleep(0.1)
        # Of two blink states take the greater, so every run picks the same.
        shown = states[-1]
        blinking = sorted({i // 2 for other in states[:-1]
                           for i in range(cells) if other[i] != shown[i]})
        while True:
            self._command("stop")
            if self._memory(TEXT_MEMORY, cells) == shown:
                break
            self._command("cont")
            if time.monotonic() > deadline:
                raise TimeoutError("the screen did not settle")
            time.sleep(0.05)
        bios = self._memory(BIOS_DATA, 0x100)
        return Screen(
            cells=shown,
            cursor=(bios[0x50], bios[0x51]),
            cursor_lines=(bios[0x61], bios[0x60]),
            video_mode=bios[0x49],
            blinking=tuple((i % COLUMNS, i // COLUMNS) for i in blinking))

    def resume(self):
        self._command("cont")

    def screenshot(self):
        """The display as the video card renders it."""
        self._command("screendump", filename=str(self._scratch))
        return Image.from_ppm(self._scratch.read_bytes())


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


def _still(states):
    if len(states) == 1:
        return True
    if len(states) != 2:
        return False
    return sum(a != b for a, b in zip(*states)) <= BLINK_BYTES
