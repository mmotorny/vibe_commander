"""A disposable DOS PC: FreeDOS booted in QEMU from disks built on the fly.

Needs `qemu-system-i386` and `mtools` on the PATH.
"""
import calendar
import json
import os
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

# The port that the driver of the tests writes to (see DRIVER.ASM): QEMU's
# debug console, which QEMU keeps in a file.
DRIVER_PORT = 0xE9

# Hard disks are 64 cylinders x 16 heads x 63 sectors: 31.5 MB of FAT16.
GEOMETRY = (64, 16, 63)

# What DOS prints on the serial port once it has booted, just before it runs
# the commands it was given.
BOOTED = b"BOOTED"

# How long the harness waits, in seconds, before it takes something for
# stuck. These are for what hangs, and are long on purpose: a machine that
# many tests share is slow, and must not look like one that hangs. A QEMU
# that has ended is noticed at once, whatever these say.
START_TIME = 60     # QEMU, to open the socket it is spoken to through
LOAD_TIME = 120     # a saved state, to be loaded
SAVE_TIME = 120     # a state, to be saved
BOOT_TIME = 300     # DOS, to boot; and a screen, to stand still after it

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

    def read(self, name):
        """The contents of the file `name`."""
        self._mtools("mcopy", "-o", f"x:/{name}", str(self._staging))
        data = self._staging.read_bytes()
        self._staging.unlink()
        return data

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

    It is the stock FreeDOS 1.4 boot floppy, with its installer replaced by
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

    def lines(self, codepage=437):
        """The characters on screen as text, one string per row, as they
        show in a code page."""
        chars = self.cells[0::2]
        return ["".join(_glyph(c, codepage)
                        for c in chars[r * COLUMNS:(r + 1) * COLUMNS])
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


def _glyph(code, codepage=437):
    if code < 32:
        return CP437_CONTROLS[code]
    if code == 127:
        return "\u2302"
    return bytes([code]).decode(f"cp{codepage}")


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

    What the guest writes to its debug console, as the driver of the tests
    does, is kept in a file: written() returns it.

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
        self._written = self._workdir / "written.bin"
        control = self._workdir / "qmp.sock"
        command = [
            "qemu-system-i386", "-machine", "pc", "-cpu", "486", "-m", "16",
            "-nodefaults", "-vga", "std", "-display", "none", "-no-reboot",
            "-rtc", f"base={clock},clock=vm", "-boot", "a",
            "-serial", f"file:{self._serial}",
            "-qmp", f"unix:{control},server=on,wait=off",
            "-chardev", f"file,id=written,path={self._written}",
            "-device", f"isa-debugcon,iobase={DRIVER_PORT:#x},chardev=written",
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
                        != "inmigrate", LOAD_TIME,
                       "the saved state did not load")

    def _connect(self, path):
        connection = socket.socket(socket.AF_UNIX)
        deadline = time.monotonic() + START_TIME
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
            if self._qemu.poll() is None:
                self._command("quit")
            self._qemu.wait(timeout=10)
        except (OSError, ValueError, MachineError, subprocess.TimeoutExpired):
            self._qemu.kill()
            self._qemu.wait()
        for channel in (self._channel, self._socket):
            try:
                channel.close()
            except OSError:
                pass                    # QEMU has gone, with what was unsent

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

    def wait_for_boot(self, timeout=BOOT_TIME):
        """Wait until DOS has booted and is about to run its commands."""
        self.until(lambda: BOOTED in self._serial.read_bytes(), timeout,
                    "DOS did not finish booting", pause=0.05)

    def save(self, path):
        """Pause the machine and save its state, for others to start from."""
        self._command("stop")
        self._command("migrate", uri=f"file:{path}")
        self.until(lambda: self._command("query-migrate").get("status")
                    == "completed", SAVE_TIME,
                   "the machine's state was not saved")

    def pause(self):
        self._command("stop")

    def resume(self):
        self._command("cont")

    def written(self):
        """What the guest has written to its debug console so far."""
        return self._written.read_bytes() if self._written.exists() else b""

    def wait_for_end(self, limit):
        """Wait until the machine has ended, as it does when the guest
        resets it. Raises MachineError if the guest writes nothing to its
        debug console for `limit` seconds: the machine is then paused, for
        its screen to be looked at."""
        size, since = -1, time.monotonic()
        while self._qemu.poll() is None:
            now = self._written.stat().st_size \
                if self._written.exists() else 0
            if now != size:
                size, since = now, time.monotonic()
            elif time.monotonic() - since > limit:
                self.pause()
                raise MachineError(f"nothing for {limit} s")
            try:
                self._qemu.wait(0.01)
            except subprocess.TimeoutExpired:
                pass

    def _keyboard(self):
        """Where the BIOS keyboard buffer is read and where it is filled."""
        head, tail = struct.unpack("<HH", self._memory(KEYBOARD_BUFFER, 4))
        return head, tail

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

    def wait_for_screen(self, settle=2.0, timeout=BOOT_TIME):
        """Wait until the text screen stands still, then return it.

        This is a guess, for where nothing better is known about the
        guest: on a busy host a screen may stand still before the guest
        has finished.

        The screen stands still when every key pressed has been taken and
        nothing has changed for `settle` seconds, other than by blinking:
        going back and forth between two states. It blinks once the first
        of the two has been seen to come back; until then a change is a
        change, however small. Blinking is seen as such only if `settle`
        covers both states. Of the two the screen returned is the one with
        the greater bytes, and its `blinking` lists the cells with what
        they show in the other.

        The machine is left paused; call resume() to go on.
        """
        size = COLUMNS * ROWS * 2
        deadline = time.monotonic() + timeout
        seen = []                       # the states since the last real change
        blinks = False                  # the first of them has come back
        last = None
        changed = time.monotonic()
        while True:
            now = time.monotonic()
            cells = self._memory(TEXT_MEMORY, size)
            if cells != last:
                if cells in seen and len(seen) == 2:
                    blinks = True
                else:
                    if len(seen) != 1:
                        seen = []
                    blinks = False
                    changed = now
                    seen.append(cells)
                last = cells
            if now - changed >= settle and len(set(self._keyboard())) == 1:
                break
            if now > deadline:
                raise MachineError("the screen did not settle")
            time.sleep(0.01)
        shown = max(seen) if blinks else last
        while True:
            self._command("stop")
            if self._memory(TEXT_MEMORY, size) == shown:
                break
            self._command("cont")
            if time.monotonic() > deadline:
                raise MachineError("the screen did not settle")
            time.sleep(0.01)
        other = min(seen) if blinks else shown
        blinking = tuple((i // 2 % COLUMNS, i // 2 // COLUMNS,
                          other[i - i % 2], other[i - i % 2 + 1])
                         for i in range(0, size, 2)
                         if shown[i:i + 2] != other[i:i + 2])
        return replace(self.screen(), blinking=blinking)

    def screenshot(self):
        """The display as the video card renders it."""
        self._command("screendump", filename=str(self._scratch))
        return Image.from_ppm(self._scratch.read_bytes())



