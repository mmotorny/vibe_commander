"""The program for Linux taken through the scenarios.

(Not posix.py, which Python has a module of its own by.)

The program for Linux, build/portable_c/posix-asan/vc (or posix/vc), is
judged by the scenarios as the program for DOS is (CLAUDE.md, "The
portable C", step 6), but runs on the host, not in a machine: it is
started as a process of its own with its test channel (the top of the
test channel in src/portable_c/sys_posix.c), which does from inside what
the driver of the tests does from outside on the bench. This module lays
out the bench of a run, makes a scenario's steps into the channel's
`keys`, runs the program, and makes of what it wrote and left a
scenarios.Run, so that compare.py and goldens.py take it as they take a
build for DOS. What the program for Linux is declared to show otherwise
is in deviations.py, under the tree "posix".

The bench of a run is a directory of /tmp of its own, removed behind it:

  - `home`, which is HOME and so the drive H:, laid out as the bench's
    C: is, WORK and STAMPS with the same files and the same times
    (setups.WORK_FILES, STAMPS_FILES; the large sizes as sparse files),
    the program started in home/WORK, so on H:\\WORK where the original
    is on C:\\WORK;
  - `vc`, the directory of the setup file, which XDG_CONFIG_HOME names,
    with what the bench has beside the program in D:\\VC
    (setups.SUPPORT_FILES, from the pinned distribution, with their
    dates), named in small letters as the program names what it makes,
    and the bench's setup file as it is (setups.Bench.setup).

The bench's directory is in /tmp, not in build/tmp, so that the path of
`vc` holds in DOS's 63 characters on R: whatever the checkout's path
is: the program ends where DOS cannot name the directory of its setup
file (TODO.md, L3a).

The environment has TZ=UTC, under which the host's times of the files
are the bench's, PATH, and the channel's two variables: VC_TEST and
VC_TEST_CLOCK, the bench's clock when the program starts (CLOCK), from
which the program's clock goes on only while the test waits, as the
machine's does on the bench.

A key is made the word that the bench's BIOS makes of it, the scan code
above the character, as INT 16h 00h answers it: the table of the IBM
PC's BIOS for the keys of a US keyboard (IBM's Technical Reference; RBIL,
INT 16h), with the cursor's keys of the gray block, which the driver
presses (driver.EXTENDED), as 00h answers them, the character E0h made
0. Shift, Ctrl and Alt are `held=` around the key, as the bench has them
held while it is pressed: 02h for Shift, the left one, 01h for the right
one, 04h for Ctrl, 08h for Alt, as the BIOS keeps them at 40:17h. The
keys of one chord, "Enter+F10", are put into the buffer at once, as the
driver puts them. A step of the mouse is the channel's mouse= and asks=,
as driver.py has its packets (pointer()).
"""
import dataclasses
import hashlib
import os
import random
import re
import shutil
import string
import subprocess
import threading
import time
from pathlib import Path

import deps
import driver
import fat
import setups
from dos import COLUMNS, ROWS, Screen, MachineError

ROOT = deps.ROOT
# The program for Linux, with the sanitizers that go together.
PROGRAM = ROOT / "build" / "portable_c" / "posix-asan" / "vc"

# The time and the date of a directory that the run has written into:
# none of DOS's, whose date has no month 15 (see goldens.disk_lines).
WRITTEN_INTO = 0xFFFF

# The bench's clock when the program starts: the machine's starts at
# setups.CLOCK, and DOS has booted and the program drawn its first screen
# some five seconds later (CLAUDE.md, "Scenarios"). A file that the
# program writes is stamped by this clock, and the goldens have such a
# stamp as "during the run" only where it is after setups.CLOCK, as it
# always is on the bench: from setups.CLOCK itself, a file written at
# once had the bench's time of the files laid out.
CLOCK = "2000-06-16T12:00:05"

# A record of the channel's `screens`.
RECORD = 4 + 4 + COLUMNS * ROWS * 2 + 8

# The keys that are no character, by the driver's name: the BIOS's word
# alone, with Shift, with Ctrl and with Alt; None where the BIOS makes
# nothing of the key so held.
SPECIAL = {
    "Esc": (0x011B, 0x011B, 0x011B, 0x0100),
    "Backspace": (0x0E08, 0x0E08, 0x0E7F, 0x0E00),
    "Tab": (0x0F09, 0x0F00, 0x9400, 0xA500),
    "Enter": (0x1C0D, 0x1C0D, 0x1C0A, 0x1C00),
    "Space": (0x3920, 0x3920, 0x3920, 0x3920),
    "GrayStar": (0x372A, 0x372A, 0x9600, 0x3700),
    "GrayMinus": (0x4A2D, 0x4A2D, 0x8E00, 0x4A00),
    "GrayPlus": (0x4E2B, 0x4E2B, 0x9000, 0x4E00),
    # The gray block of the cursor: E0h for the character, which INT 16h
    # 00h makes 0.
    "Home": (0x4700, 0x4700, 0x7700, 0x9700),
    "Up": (0x4800, 0x4800, 0x8D00, 0x9800),
    "PgUp": (0x4900, 0x4900, 0x8400, 0x9900),
    "Left": (0x4B00, 0x4B00, 0x7300, 0x9B00),
    "Right": (0x4D00, 0x4D00, 0x7400, 0x9D00),
    "End": (0x4F00, 0x4F00, 0x7500, 0x9F00),
    "Down": (0x5000, 0x5000, 0x9100, 0xA000),
    "PgDn": (0x5100, 0x5100, 0x7600, 0xA100),
    "Ins": (0x5200, 0x5200, 0x9200, 0xA200),
    # The bench's BIOS makes no word of Alt-Del.
    "Del": (0x5300, 0x5300, 0x9300, None),
}
for _number in range(1, 11):
    SPECIAL[f"F{_number}"] = tuple((code + _number - 1) << 8 for code in
                                   (0x3B, 0x54, 0x5E, 0x68))
SPECIAL["F11"] = (0x8500, 0x8700, 0x8900, 0x8B00)
SPECIAL["F12"] = (0x8600, 0x8800, 0x8A00, 0x8C00)

# The keys of characters, by the character that they type alone: the
# scan code, and the character with Shift and with Ctrl, or None.
_ROWS = [("1234567890-=", 0x02), ("qwertyuiop[]", 0x10),
         ("asdfghjkl;'`", 0x1E), ("\\zxcvbnm,./", 0x2B)]
_SHIFTED = dict(zip("1234567890-=[];'`\\,./",
                    "!@#$%^&*()_+{}:\"~|<>?"))
CHARACTERS = {}
for _keys, _scan in _ROWS:
    for _at, _char in enumerate(_keys):
        if _char.isalpha():
            shift, ctrl = _char.upper(), chr(ord(_char) - 0x60)
        else:
            shift = _SHIFTED[_char]
            ctrl = {"[": "\x1b", "]": "\x1d", "\\": "\x1c", "-": "\x1f",
                    "2": "\x00", "6": "\x1e"}.get(_char)
        CHARACTERS[_char] = (_scan + _at, shift, ctrl)
# Alt with a digit or with - and =, which the BIOS gives scan codes of
# their own: 78h for Alt-1 to 83h for Alt-=.
ALT_DIGITS = {char: 0x78 + at for at, char in enumerate("1234567890-=")}
# Shift with a character that it types, and the key that is pressed.
BY_SHIFTED = {shift: char for char, (_, shift, _) in CHARACTERS.items()}

HELD = {"Shift": 0x02, "RightShift": 0x01, "Ctrl": 0x04, "Alt": 0x08}


def word(name, held):
    """The BIOS's word for the key `name`, as the driver names it, with
    the keys of HELD that `held` has down."""
    column = 3 if held & 8 else 2 if held & 4 else 1 if held & 3 else 0
    if name in SPECIAL:
        found = SPECIAL[name][column]
    else:
        char = name.lower() if len(name) == 1 else None
        if char not in CHARACTERS:
            raise ValueError(f"unknown key {name!r}")
        scan, shift, ctrl = CHARACTERS[char]
        if column == 3:
            found = (ALT_DIGITS[char] << 8) if char in ALT_DIGITS \
                else scan << 8
        elif column == 2:
            found = None if ctrl is None else scan << 8 | ord(ctrl)
        elif column == 1:
            found = scan << 8 | ord(shift)
        else:
            found = scan << 8 | ord(char)
    if found is None:
        raise ValueError(f"the BIOS makes nothing of {name!r} with "
                         f"{held:02X}h held")
    return found


def pointer(name):
    """The words of the channel for a step of the mouse, as
    driver._pointer() has its events: "Move@column,row" a move, and
    "Click@column,row*asked>column,row*asked", a button down at the
    first place for as many times as the program asks where the mouse
    is, ASKED unless the step says, moved with it down to each place
    after, and let go at the last."""
    what, _, places = name.partition("@")
    try:
        button = driver.BUTTONS[what]
        stops = []
        for place in places.split(">"):
            place, _, asked = place.partition("*")
            column, row = (int(number) for number in place.split(","))
            asked = int(asked or driver.ASKED)
            if not (0 <= column < COLUMNS and 0 <= row < driver.POINTER_ROWS
                    and 0 < asked < 256):
                raise ValueError
            stops.append((column, row, asked))
        if button is None and (len(stops) > 1 or "*" in places):
            raise ValueError
    except (KeyError, ValueError):
        raise ValueError(f"unknown step of the mouse {name!r}") from None
    if button is None:
        column, row, _ = stops[0]
        return [f"mouse={column},{row},0"]
    out = []
    for column, row, asked in stops:
        out += [f"mouse={column},{row},{button:X}", f"asks={asked}"]
    column, row, _ = stops[-1]
    return out + [f"mouse={column},{row},0"]


def events(step):
    """The words of the channel's `keys` for one step of a scenario, as
    driver.events() has its events: Shift, Ctrl and Alt down first, one
    after the other, and up last; keys of one name that are not those
    one behind the other; a text a key for each character, with Shift
    where the character wants it."""
    out = []
    for name in step.split():
        if "@" in name and not name.startswith('"'):
            out += pointer(name)
            continue
        if name.startswith("Wait*"):
            out.append(f"wait={int(name.partition('*')[2])}")
            continue
        if name.startswith('"'):
            chords = [["Shift", BY_SHIFTED[char]] if char in BY_SHIFTED
                      else [char] for char in name.strip('"')]
        else:
            chords = [name.split("+")]
        for chord in chords:
            held = 0
            mods = [key for key in chord if key in HELD]
            keys = [key for key in chord if key not in HELD]
            for key in mods:
                held |= HELD[key]
                out.append(f"held={held:02X}")
            # The keys of one chord are put into the buffer at once, as
            # the driver puts them: a word with + has the next with it.
            for at, key in enumerate(keys):
                out.append(f"{word(key, held):04X}"
                           + ("+" if at < len(keys) - 1 else ""))
            for key in mods[::-1]:
                held &= ~HELD[key]
                out.append(f"held={held:02X}")
    return out


def keys(steps):
    """The channel's `keys` for a scenario of `steps`: the start, which
    is an empty step, and a line for each."""
    return "".join(" ".join(events(step)) + "\n" for step in ("",) + steps)


# The place of the program's own files on the bench, D:\VC, as a
# scenario types it, and where the run has them: the directory of its
# setup file, `vc` of the run's directory, which is a directory of /tmp
# with a name of eight characters at most, so that its path on R: is the
# name itself and no alias, R:\TMP\VCxxxxxx\VC. A text that a step
# types with that place in it is typed with the run's
# (deviations.OwnPath masks it on the screen).
OWN = re.compile(r"[Dd]:\\[Vv][Cc](?=[\\\"]|$)")


def own(steps, bench):
    """`steps` with the place of the program's own files typed as the
    run's in the directory `bench`."""
    path = "r:" + "\\".join(("",) + bench.parts[1:]) + "\\vc"

    def typed(match):
        return OWN.sub(lambda _: path, match.group(0))
    return tuple(re.sub(r'"[^"]*"', typed, step) for step in steps)


def _remove(path):
    """Remove the directory `path` with what is in it, also what a run
    has made read-only."""
    def again(function, name, _):
        for each in (os.path.dirname(name), name):
            if os.path.exists(each) and not os.path.islink(each):
                os.chmod(each, 0o700)
        function(name)
    shutil.rmtree(path, onexc=again)


def _bench_directory():
    """A directory of its own for a run, in /tmp, of a name of eight
    characters at most (see OWN)."""
    letters = string.ascii_lowercase + string.digits
    while True:
        path = Path("/tmp") / ("vc" + "".join(random.choices(letters,
                                                             k=6)))
        try:
            path.mkdir(mode=0o700)
        except FileExistsError:
            continue
        return path


def _dos_stamp(seconds):
    """DOS's time and date words of a time of the host, in UTC."""
    moment = time.gmtime(seconds)
    year = max(moment.tm_year, 1980)
    return (moment.tm_hour << 11 | moment.tm_min << 5 | moment.tm_sec // 2,
            (year - 1980) << 9 | moment.tm_mon << 5 | moment.tm_mday)


def lay_out(bench, setup, files=False):
    """Lay the bench of a run out in the directory `bench`, with `setup`
    as the setup file: the home directory and the directory of the setup
    file, and the times of what is in them, each directory's as the
    bench's FAT has it, (directory, time) for each made, to be set when
    what is in it has been made. With `files`, the home directory has
    FILES as the bench's C: has it (setups.files_layout())."""
    home, config = bench / "home", bench / "vc"
    stamps = []
    for path in (home, home / "WORK", home / "STAMPS", config):
        path.mkdir(parents=True)
    for name in setups.WORK_DIRECTORIES:
        (home / "WORK" / name).mkdir()
        stamps.append((home / "WORK" / name, setups.EPOCH))
    for name, data in setups.WORK_FILES:
        (home / "WORK" / name).write_bytes(data)
        stamps.append((home / "WORK" / name, setups.EPOCH))
    for name in setups.STAMPS_DIRECTORIES:
        (home / "STAMPS" / name).mkdir()
        stamps.append((home / "STAMPS" / name, setups.EPOCH))
    for name, size, when in setups.STAMPS_FILES:
        with open(home / "STAMPS" / name, "wb") as file:
            file.truncate(size)
        stamps.append((home / "STAMPS" / name, when))
    if files:
        (home / setups.FILES_DIRECTORY).mkdir()
        for name, data, when in setups.files_layout():
            path = home / setups.FILES_DIRECTORY / name
            if data is None:
                path.mkdir()
            else:
                path.write_bytes(data)
            stamps.append((path, when))
        stamps.append((home / setups.FILES_DIRECTORY, setups.EPOCH))
    files = setups.distribution()
    for name in setups.SUPPORT_FILES:
        data, when = files[name]
        path = config / name.lower()
        path.write_bytes(setup if name == setups.SETUP_FILE else data)
        stamps.append((path, when))
    for path in (home / "WORK", home / "STAMPS", config, home):
        stamps.append((path, setups.EPOCH))
    for path, when in stamps:
        os.utime(path, (when, when))
    # What was laid out, with its time and, of a file, its size and the
    # sum that the bench's disk has of it: the files of STAMPS have their
    # sizes only in the bench's directory, and no contents, and are
    # sparse here, so that none of their bytes is read.
    laid = {}
    for path, when in stamps:
        size = path.stat().st_size
        if path.parent.name == "STAMPS" and path.is_file():
            data = b""
        elif path.is_file():
            data = path.read_bytes()
        else:
            data = None
        laid[path] = (when, size,
                      None if data is None else
                      hashlib.sha256(data).hexdigest())
    return laid


def read_tree(directory, prefix, laid):
    """The files and directories under `directory` as fat.read_tree()
    has a disk's, with `prefix` in front of each path: names as DOS has
    them, in capitals; a directory as 10h, a file as 20h, archive, as
    DOS makes one, with 01h where the host does not let it be written.
    A directory whose time is not what `laid` has for it has been
    written into by the run (or made by it), and has WRITTEN_INTO for
    its time and its date, which goldens.disk_lines() says so of. A file
    that is as it was laid out has the sum that the bench has of it."""
    entries = {}
    for path in sorted(directory.rglob("*")):
        name = prefix + "\\".join(part.upper() for part in
                                  path.relative_to(directory).parts)
        stat = path.lstat()
        stamp = _dos_stamp(stat.st_mtime)
        was = laid.get(path)
        if path.is_dir():
            if was is None or was[0] != int(stat.st_mtime):
                stamp = (WRITTEN_INTO, WRITTEN_INTO)
            entries[name] = fat.Entry(name, fat.DIRECTORY, *stamp, 0, "")
            continue
        if was is not None and was[:2] == (int(stat.st_mtime),
                                           stat.st_size):
            sha256 = was[2]
        else:
            sha256 = hashlib.sha256(path.read_bytes()).hexdigest()
        entries[name] = fat.Entry(
            name, 0x20 | (0 if os.access(path, os.W_OK) else 1),
            *stamp, stat.st_size, sha256)
    return entries


def _screens(data):
    """The records of `screens`: (kind, number, Screen)."""
    out = []
    for at in range(0, len(data) - RECORD + 1, RECORD):
        record = data[at:at + RECORD]
        kind, number = record[:4].decode("ascii"), \
            int.from_bytes(record[4:8], "little")
        cells, rest = record[8:8 + COLUMNS * ROWS * 2], record[-8:]
        out.append((kind, number,
                    Screen(cells=cells, cursor=(rest[0], rest[1]),
                           cursor_lines=(rest[2], rest[3]),
                           video_mode=rest[4])))
    return out


def run(scenario, program, setup=..., limit=60):
    """Take the program for Linux, `program` (a setups.Program of the
    tree "posix"), through `scenario` (its
    steps as the tree "posix" is taken through them) with `setup` as the
    setup file that it starts from, linux_setup()'s unless another is
    given, and return the scenarios.Run, with what is then in the setup
    file's place as `saved`, None if nothing. Raises
    MachineError where the program ends otherwise than the scenario has
    it, or does not end within `limit` seconds."""
    import scenarios
    if setup is ...:
        setup = linux_setup(program)
    steps = ("",) + scenario.steps
    bench = _bench_directory()
    try:
        laid = lay_out(bench, setup, scenario.files)
        channel = bench / "channel"
        channel.mkdir()
        (channel / "keys").write_text(keys(own(scenario.steps, bench)))
        environment = {
            "PATH": os.environ.get("PATH", "/usr/bin:/bin"),
            "TZ": "UTC", "HOME": str(bench / "home"),
            "XDG_CONFIG_HOME": str(bench),
            "VC_TEST": str(channel), "VC_TEST_CLOCK": CLOCK,
        }
        try:
            done = subprocess.run(
                [str(program.path.resolve())], cwd=bench / "home" / "WORK",
                env=environment, stdin=subprocess.DEVNULL,
                capture_output=True, timeout=limit)
        except subprocess.TimeoutExpired:
            raise MachineError(f"the program did not end within {limit} "
                               "s") from None
        records = _screens((channel / "screens").read_bytes())
        said = done.stderr.decode(errors="replace").strip()
        if done.returncode not in (0, 2) or (
                done.returncode == 2 and "keys have run out" not in said):
            raise MachineError(
                f"the program ended with {done.returncode}: {said}")
        shown = [screen for kind, _, screen in records if kind == "step"]
        ending = [screen for kind, number, screen in records
                  if kind == "end " and not (number == 2
                                             and done.returncode == 2)]
        shown += ending
        if len(shown) != len(steps):
            raise MachineError(
                f"after {scenarios._where(steps, len(shown))}: the program "
                f"wrote {len(shown)} screens for {len(steps)} steps"
                + (f"; it said: {said}" if said else ""))
        disks = {"C": read_tree(bench / "home", "", laid),
                 "D": {"VC": fat.Entry("VC", fat.DIRECTORY,
                                       *_dos_stamp(setups.EPOCH), 0, ""),
                       **read_tree(bench / "vc", "VC\\", laid)}}
        # The directories of the bench that the run removed: a directory
        # is emptied before it is removed, and the host moves its time for
        # that, which a screen may show in between. One that the run made
        # and removed is not seen.
        removed = {"C:\\" + "\\".join(part.upper() for part in
                                       path.relative_to(bench / "home").parts)
                   for path in laid
                   if path.is_relative_to(bench / "home") and
                   not path.exists() and laid[path][2] is None}
        root_written = int((bench / "home").stat().st_mtime) \
            != laid[bench / "home"][0]
        # The setup file that the program for Linux started from stands
        # for the bench's where the run has left it as it was: the bench
        # of DOS has that one in its place.
        ini = "VC\\" + setups.SETUP_FILE
        left = disks["D"].get(ini)
        bench_file = bench_setup()
        if left is not None and \
                left.sha256 == hashlib.sha256(setup).hexdigest():
            disks["D"][ini] = dataclasses.replace(
                left,
                size=len(bench_file),
                sha256=hashlib.sha256(bench_file).hexdigest())
        saved = bench / "vc" / setups.SETUP_FILE.lower()
        saved = saved.read_bytes() if saved.exists() else None
        if left is not None and saved is not None and \
                left.sha256 != hashlib.sha256(setup).hexdigest():
            # A setup file that the run has saved is held by the sum of
            # what the bench's would be: its drives made the bench's.
            disks["D"][ini] = dataclasses.replace(
                left, sha256=hashlib.sha256(on_the_bench(saved)).hexdigest())
        # Where the bench has the program under test, which the goldens
        # say no more of.
        disks["D"]["VC\\" + program.filename] = fat.Entry(
            "VC\\" + program.filename, 0x20, 0, 0, 0, "")
    finally:
        _remove(bench)
    ran = scenarios.Run(shown, disks)
    # The directories that the run wrote into, by their paths on the
    # bench, the drive's root among them: the mask of a directory's time
    # (deviations.WrittenTimes) goes by it.
    ran.written = {f"{drive}:\\{name}" for drive, entries in disks.items()
                   for name, entry in entries.items()
                   if entry.time == WRITTEN_INTO}
    if root_written:
        ran.written.add("C:\\")
    ran.written |= removed
    # The first step at which the program ran a command, which it says on
    # standard error, or None: what DOS has shown behind the panels is
    # left out from there on (deviations.DosShown).
    steps_run = [int(found) for found in
                 re.findall(r"a program is run at step (\d+)", said)]
    ran.commanded = min(steps_run) if steps_run else None
    # What the program said on standard error, for whoever looks.
    ran.said = said
    ran.saved = saved
    return ran


# Where the setup file has the paths of the two panels, 88 bytes each
# from the panel's place in it, the bench's C:\WORK and C:\; and where it
# has its sum, a word of its bytes before it.
SETUP_PATHS = (0x17, 0x6F)
SETUP_SUM = 0x1FD


def on_the_bench(saved):
    """The setup file `saved` of the program for Linux as the bench's
    would be (CLAUDE.md, step 6: the design's rule, made backwards): a
    panel's path on H:, the home directory, which is the bench's C:, made
    one on C:, and the sum made again. A path of another drive, R:, is
    left as it is, and the file does not hold."""
    out = bytearray(saved)
    if len(out) < SETUP_SUM + 2:
        return saved
    for at in SETUP_PATHS:
        if out[at:at + 3] == b"H:\\":
            out[at] = ord("C")
    total = sum(out[:SETUP_SUM]) & 0xFFFF
    out[SETUP_SUM:SETUP_SUM + 2] = total.to_bytes(2, "little")
    return bytes(out)


# The bench's setup file, the one that the original saves once its
# clock is off and its right panel one of files: made on a bench of DOS
# the first time it is wanted, which starts a machine, and kept in
# build/ beside the bench's other things. Every golden has its sum.
SETUP = ROOT / "build" / "posix-bench" / "VC.INI"

# The setup file that the program for Linux starts from in a scenario,
# the one that it saves itself, once for a bench, after its own keys have
# taken the panels from the bench's setup file to H:\WORK and H:\, where
# that file has C:\WORK and C:\ (CLAUDE.md, step 6; the user, on 15
# October 2026). Started on the bench's file, it finds no C: and has both
# panels on H:\WORK, where it was started (TODO.md, L3b); the right panel
# is taken to the root and the setup saved. Made again when the program
# is newer than it, and kept beside the bench's.
LINUX_SETUP = ROOT / "build" / "posix-bench" / "vc.ini"
LINUX_SETUP_STEPS = ("Tab", "Ctrl+\\", "Tab", "Shift+F9 Enter")


class _Steps:
    """A scenario of no name with `steps`, for run()."""
    name = "the setup file of the program for Linux"
    files = False                 # no C:\FILES (lay_out)

    def __init__(self, steps):
        self.steps = steps


def linux_setup(program):
    """The setup file that the program for Linux, `program` (a
    setups.Program), starts from in a scenario: see LINUX_SETUP.
    Scenarios run side by side as threads (compare.py), and the first
    of them makes the file while the others wait: without the lock one
    read it half written, and its program said "VC.INI is not
    correct"."""
    with _SETUP_LOCK:
        bench_file = bench_setup()
        if not LINUX_SETUP.exists() or \
                LINUX_SETUP.stat().st_mtime < program.path.stat().st_mtime \
                or LINUX_SETUP.stat().st_mtime < SETUP.stat().st_mtime:
            ran = run(_Steps(LINUX_SETUP_STEPS), program, bench_file)
            if ran.saved is None:
                raise MachineError(
                    "the program for Linux saved no setup file")
            LINUX_SETUP.parent.mkdir(parents=True, exist_ok=True)
            LINUX_SETUP.write_bytes(ran.saved)
        return LINUX_SETUP.read_bytes()


_SETUP_LOCK = threading.Lock()


def bench_setup():
    """The bench's setup file, made on the bench of DOS where build/ has
    none, or one that is not the file that the goldens have (the golden
    of `start` has its sum): the bench's setup changes now and then, as
    when it came to sort by name."""
    import goldens
    known = goldens.read("start").disks["D"]["VC\\" + setups.SETUP_FILE]
    if not SETUP.exists() or hashlib.sha256(
            SETUP.read_bytes()).hexdigest()[:16] != known.split()[-1]:
        with setups.Benches() as benches:
            setup = benches["us"].setup
        SETUP.parent.mkdir(parents=True, exist_ok=True)
        SETUP.write_bytes(setup)
    return SETUP.read_bytes()
