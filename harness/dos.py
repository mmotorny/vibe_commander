"""A disposable DOS PC: FreeDOS booted in QEMU from disks built on the fly.

Needs `qemu-system-i386` and `mtools` on the PATH.
"""
import calendar
import json
import os
import re
import select
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
BIOS_SHIFT_KEYS = 0x417           # which of Shift, Ctrl and Alt are held
SHIFT_BITS = {"shift_r": 1, "shift": 2, "ctrl": 4, "alt": 8}

# The port a program built for the tests writes to, to say where it is.
SIGNAL_PORT = 0xE9

# Hard disks are 64 cylinders x 16 heads x 63 sectors: 31.5 MB of FAT16.
GEOMETRY = (64, 16, 63)

# What DOS prints on the serial port once it has booted, just before it runs
# the commands it was given.
BOOTED = b"BOOTED"

# How long Shift, Ctrl and Alt are held down before and after the key they
# go with, or on their own, in seconds.
KEY_TIME = 0.02
MODIFIERS = {"ctrl", "alt", "shift", "shift_r"}

# Names for the keys of a US keyboard, as written in scenarios, and the
# QEMU key codes they stand for. Letters, digits and F1-F12 name themselves.
KEYS = {
    "Enter": "ret", "Esc": "esc", "Tab": "tab", "Space": "spc",
    "Backspace": "backspace", "Ins": "insert", "Del": "delete",
    "Home": "home", "End": "end", "PgUp": "pgup", "PgDn": "pgdn",
    "Up": "up", "Down": "down", "Left": "left", "Right": "right",
    "GrayPlus": "kp_add", "GrayMinus": "kp_subtract",
    "GrayStar": "kp_multiply",
    "Ctrl": "ctrl", "Alt": "alt", "Shift": "shift", "RightShift": "shift_r",
}
PUNCTUATION = {
    " ": "spc", "-": "minus", "=": "equal", "[": "bracket_left",
    "]": "bracket_right", ";": "semicolon", "'": "apostrophe",
    "`": "grave_accent", "\\": "backslash", ",": "comma", ".": "dot",
    "/": "slash",
}
SHIFTED = dict(zip('!@#$%^&*()_+{}:"~|<>?', "1234567890-=[];'`\\,./"))

# The Cyrillic letters of the Russian keyboard, by the keys of a US one
# that they are on. With them selected the full stop and the comma are on
# one key, and the digits and the underscore where they always are.
RUSSIAN = dict(zip(
    "\u0439\u0446\u0443\u043a\u0435\u043d\u0433\u0448\u0449\u0437"
    "\u0445\u044a\u0444\u044b\u0432\u0430\u043f\u0440\u043e\u043b"
    "\u0434\u0436\u044d\u044f\u0447\u0441\u043c\u0438\u0442\u044c"
    "\u0431\u044e\u0451",
    "qwertyuiop[]asdfghjkl;'zxcvbnm,.`"))
RUSSIAN_KEYS = {".": ["slash"], ",": ["shift", "slash"],
                "_": ["shift", "minus"]}

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
        signals = self._workdir / "signals.sock"
        command = [
            "qemu-system-i386", "-machine", "pc", "-cpu", "486", "-m", "16",
            "-nodefaults", "-vga", "std", "-display", "none", "-no-reboot",
            "-rtc", f"base={clock},clock=vm", "-boot", "a",
            "-serial", f"file:{self._serial}",
            "-qmp", f"unix:{control},server=on,wait=off",
            # What the guest writes to port E9h arrives on a socket.
            "-chardev", f"socket,id=signals,path={signals},server=on,wait=off",
            "-device", f"isa-debugcon,iobase={SIGNAL_PORT:#x},chardev=signals",
            "-drive", f"if=floppy,index=0,format=raw,readonly=on,file={floppy}",
            # A debugger may attach here, to stop the guest at an address.
            "-gdb", f"unix:{self._workdir / 'gdb.sock'},server=on,wait=off",
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
        self._signals = self._connect(signals)
        self._heard = ""                                 # signals not yet used
        self._stale = 0                 # how many are from before the last key
        self._debugger = None
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
        self._signals.close()

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

    def _keyboard(self):
        """Where the BIOS keyboard buffer is read and where it is filled."""
        head, tail = struct.unpack("<HH", self._memory(KEYBOARD_BUFFER, 4))
        return head, tail

    def press(self, keys, watch=None):
        """Press keys, given as in a scenario: names separated by spaces.

        A name is a key ("F3", "Enter", "a"), keys held down together
        ("Alt+F5", "Ctrl+o"), or a text to type in double quotes, which
        may not contain spaces ("readme.txt"). Keys of one name that are
        not Shift, Ctrl or Alt go down one behind the other with nothing
        in between ("Enter+Home"): when the program takes the first, the
        others are waiting, which is how a key gets to a program that
        looks for keys while it works.

        A text with Russian letters is typed on the keys that the Russian
        keyboard has them on. It comes out as written only where that
        keyboard is loaded and its Cyrillic letters have been selected,
        with "Alt+RightShift", and it may hold digits, a full stop, a
        comma and an underscore besides, but no Latin letters.

        Each key is seen to arrive in the BIOS keyboard buffer before the
        next is pressed, so none is lost however fast or slow the guest is.
        The buffer holds 15 keys; if the guest does not take them, pressing
        more fails.

        With `watch`, which knows where the program in the guest is (see
        watch.py), a key is pressed when the program waits for one, and
        Shift, Ctrl and Alt are held until it has seen them; the machine
        is left paused. Without, they are held for KEY_TIME.
        """
        chords = []
        for name in keys.split():
            if name.startswith('"'):
                text = name.strip('"')
                keys_of = (_russian if any(char.lower() in RUSSIAN
                                           for char in text) else _character)
                chords += [keys_of(char) for char in text]
            else:
                chords.append([_key(part) for part in name.split("+")])
        for codes in chords:
            self._chord(codes, watch)

    def _chord(self, codes, watch=None):
        """Press the keys `codes` together: Shift, Ctrl and Alt go down
        first, one after the other in the order given, and come up last,
        as under a hand, and long enough for the guest to see each held."""
        held = [code for code in codes if code in MODIFIERS]
        keys = [code for code in codes if code not in MODIFIERS]
        if watch:
            watch.before_key(self)
        for code in held:
            self._shift(code, True, watch)
        if keys:
            tail = self._keyboard()[1]

            def send():
                # What the guest has signalled so far says nothing about
                # this key.
                self._listen()
                self._stale = len(self._heard)
                if watch:
                    # Down and up in one: the program may act on the key
                    # and have the machine stopped before a second command.
                    self._command("input-send-event", events=[
                        _key_event(code, down) for code, down in
                        [(code, True) for code in keys]
                        + [(code, False) for code in keys[::-1]]])
                else:
                    self._keys(keys, True)
                    self._keys(keys[::-1], False)
                self.until(lambda: self._keyboard()[1] != tail, 5,
                           f"the key {'+'.join(codes)} did not arrive: the "
                           "keyboard buffer is full, or the guest is stuck")
            if watch:
                watch.key(self, send)
            else:
                send()
        for code in held[::-1]:
            self._shift(code, False, watch)

    def _shift(self, code, down, watch):
        """Press or release one of Shift, Ctrl and Alt, and wait until
        the program that `watch` watches has seen it."""
        if watch:
            watch.shift(self, lambda: self._keys([code], down), lambda: bool(
                self._memory(BIOS_SHIFT_KEYS, 1)[0] & SHIFT_BITS[code])
                == down)
            return
        if not down:
            time.sleep(KEY_TIME)
        self._keys([code], down)
        if down:
            time.sleep(KEY_TIME)

    def _keys(self, codes, down):
        self._command("input-send-event", events=[
            _key_event(code, down) for code in codes])

    def keys_taken(self):
        """Whether the guest has read every key that was pressed."""
        head, tail = self._keyboard()
        return head == tail

    def keyboard(self):
        """Where the BIOS takes a key from its keyboard buffer and where
        it puts the next: the buffer is empty when they are the same."""
        return self._keyboard()

    def signals(self):
        """What the guest has signalled since this was last asked, as text:
        the bytes it wrote to the signal port. And how many of them it
        had written before the last key was pressed, if one has been
        pressed since this was last asked: otherwise 0."""
        self._listen()
        heard, stale = self._heard, self._stale
        self._heard, self._stale = "", 0
        return heard, stale

    def _listen(self):
        """Take what the guest has signalled by now."""
        self._signals.setblocking(False)
        try:
            while True:
                data = self._signals.recv(4096)
                if not data:
                    break
                self._heard += data.decode("latin-1")
        except BlockingIOError:
            pass

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
            if now - changed >= settle and self.keys_taken():
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

    def wait_for_prompt(self, timeout=30):
        """Wait until DOS shows its prompt and waits for a command, then
        return the screen: the cursor stands behind a ">", and every key
        pressed has been taken. The machine is left paused.

        A command that prints a ">" at the end of what it has printed so
        far would be taken for the prompt.
        """
        deadline = time.monotonic() + timeout
        while True:
            self._command("stop")
            screen = self.screen()
            column, row = screen.cursor
            if column and row < ROWS and self.keys_taken() and \
                    screen.cells[(row * COLUMNS + column - 1) * 2] == 0x3E:
                return screen
            self._command("cont")
            if time.monotonic() > deadline:
                raise MachineError("DOS did not come to its prompt")
            time.sleep(0.002)

    def _debug(self, packet=None, timeout=5, reply=True):
        """Send a packet of GDB's remote protocol, if one is given, and
        return the reply, or None if none came within `timeout` seconds.
        Without `reply` it is not waited for. Attaching stops the
        machine."""
        if self._debugger is None:
            self._debugger = self._connect(self._workdir / "gdb.sock")
            self._unread = b""
        if packet is not None:
            self._debugger.sendall(b"$%s#%02x" % (packet, sum(packet) % 256))
        if not reply:
            return None
        self._debugger.settimeout(timeout)
        while True:
            match = re.search(rb"\$([^#]*)#..", self._unread, re.S)
            if match:
                self._unread = self._unread[match.end():]
                self._debugger.sendall(b"+")
                return match[1]
            try:
                data = self._debugger.recv(4096)
            except socket.timeout:
                return None
            if not data:
                raise MachineError("the machine stopped: it was reset, or "
                                   "QEMU ended")
            self._unread += data

    def start(self, addresses):
        """Let the paused machine run until it is about to execute the
        instruction at one of `addresses`, or stopped() is called."""
        self._stops = list(addresses)
        for address in self._stops:
            self._debug(b"Z0,%x,1" % address)
        self._debug(b"c", reply=False)
        # It runs once QEMU has got to that, unless it has stopped again
        # already, which QEMU reports.
        def stopped_again():
            while select.select([self._debugger], [], [], 0)[0]:
                self._unread += self._debugger.recv(4096)
            return b"$" in self._unread

        self.until(lambda: self._command("query-status")["running"]
                   or stopped_again(), 5, "the machine did not go on")

    def stopped(self, timeout=0):
        """Wait `timeout` seconds for the machine that start() let run to
        come to one of its addresses, then pause it wherever it is. Returns
        the address it came to, or None. `registers` has its registers."""
        reply = self._debug(timeout=timeout) if timeout else None
        if reply is None:
            self._command("stop")
            reply = self._debug()
        for address in self._stops:
            self._debug(b"z0,%x,1" % address)
        registers = struct.unpack("<16I", bytes.fromhex(
            self._debug(b"g")[:128].decode()))
        self.registers = dict(zip(
            "ax cx dx bx sp bp si di ip flags cs ss ds es fs gs".split(),
            registers))
        at = self.registers["cs"] * 16 + self.registers["ip"]
        return at if reply.startswith(b"T05") and at in self._stops else None

    def run_to(self, addresses, timeout):
        """Let the paused machine run until it is about to execute the
        instruction at one of `addresses`, and return which; None if it
        has not come to any within `timeout` seconds. It is left paused."""
        self.start(addresses)
        return self.stopped(timeout)

    def step(self):
        """Have the paused machine execute one instruction."""
        self._debug(b"s")

    def screenshot(self):
        """The display as the video card renders it."""
        self._command("screendump", filename=str(self._scratch))
        return Image.from_ppm(self._scratch.read_bytes())


def _key_event(code, down):
    return {"type": "key", "data": {"down": down,
                                    "key": {"type": "qcode", "data": code}}}


def _key(name):
    if name in KEYS:
        return KEYS[name]
    if re.fullmatch(r"[a-z0-9]|F\d\d?", name):
        return name.lower()
    if name in PUNCTUATION:
        return PUNCTUATION[name]
    raise ValueError(f"unknown key {name!r}")


def _russian(char):
    """The keys to hold down to type `char` of a text with Russian
    letters."""
    if char in RUSSIAN_KEYS:
        return RUSSIAN_KEYS[char]
    if char.isdigit():
        return [char]
    if char.lower() not in RUSSIAN:
        raise ValueError(f"{char!r} cannot be typed with Russian letters")
    keys = _character(RUSSIAN[char.lower()])
    return keys if char.islower() else ["shift", *keys]


def _character(char):
    """The keys to hold down to type `char`."""
    if char in SHIFTED:
        return ["shift", _key(SHIFTED[char])]
    if char.isupper():
        return ["shift", char.lower()]
    return [_key(char)]


