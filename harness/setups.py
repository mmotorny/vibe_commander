"""The machines the harness builds for Volkov Commander.

Two setups share one clock and one source of files:

  fresh install   a single hard disk with the shareware distribution unpacked
                  in C:\\VC, and VC started there. This is what a user saw
                  on first run, and what the picture at the top of the
                  README showed until the portable C's, which is made
                  on the test bench (see screenshot.py), took its place.

  test bench      the program under test on its own disk in D:\\VC, next to
                  the distribution's help and configuration files, and a
                  small fixed set of files on C:. VC is started in
                  C:\\WORK. A scenario that wants files to work on
                  that are not the bench's own, programs of FreeDOS and
                  a file of the distribution, has them in C:\\FILES, on
                  its bench alone (files_layout()). Keeping the program
                  off the disk it displays
                  means its size cannot show up as free space on screen.
                  The clock in the corner of the screen is off, so that
                  no screen depends on how long it took to get there,
                  and the panel on the right lists files, where the
                  distribution has the Info panel: what that shows is
                  of the machine and of the system, and would be on
                  nearly every screen (see Bench). Both panels sort
                  their files by name.
                  DOS has loaded a mouse driver, for the PS/2 mouse of
                  three buttons that the machine has, and the driver of
                  the tests (see
                  driver.py), which takes the program through a
                  scenario's steps from a script on a third disk, one
                  without a file system.

                  For a scenario of the setup program the original has
                  its setup program next to it in D:\\VC, VCSETUP.COM
                  of the same distribution, which the scenario starts
                  from the program's command line (see beside(), and
                  the top of scenarios.py). No other machine has that
                  file.
"""
import functools
import io
import os
import shutil
import subprocess
import threading
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

import deps
import deviations
import dos
import driver
from dos import BOOT_TIME, FatImage, Machine, MachineError, boot_floppy, \
    timestamp

# The PC's clock at power-on, and the date given to directories and to the
# test bench's files: noon on the day Volkov Commander 4.05 was released.
CLOCK = "2000-06-16T12:00:00"
EPOCH = timestamp(2000, 6, 16, 12)
DOS_CLOCK = ((2000 - 1980) << 9 | 6 << 5 | 16, 12 << 11)   # date, time words

# What VC reads from its own directory besides the program itself.
SUPPORT_FILES = ["VC.HLP", "VC.INI", "VC.EXT", "VCVIEW.EXT", "VCEDIT.EXT",
                 "VC.MNU", "ARCHIVES.MNU", "FORMAT.MNU"]
SETUP_FILE = "VC.INI"
# The setup program of the distribution: a program of its own in the
# original, and a part of the one program in the portable C.
SETUP_PROGRAM = "VCSETUP.COM"

# The keys that switch the clock from on to off or back, in the Options
# menu; those that make the panel on the right one of files, Brief in the
# Right menu, which is the form that the left panel has as the program is
# distributed, so that the two look alike; those that have both panels
# sort their files by name, Name in the Left and the Right menu, where
# the distribution has them unsorted; and those that save the setup.
CLOCK_KEYS = "F9 o l"
RIGHT_PANEL_KEYS = "F9 r b"
BY_NAME_KEYS = ["F9 l n", "F9 r n"]
SAVE_SETUP_KEYS = "Shift+F9 Enter"
# The contents of C:\WORK on the test bench, in the order of the
# directory, which the panels do not show since the bench sorts them by
# name.
WORK_DIRECTORIES = ["DOCS", "SRC"]
WORK_FILES = [
    ("README.TXT", b"Files for exercising Volkov Commander.\r\n"),
    ("HELLO.TXT", b"Hello, world!\r\n"),
    ("NOTES.DOC", b"Line one\r\nLine two\r\nLine three\r\n"),
    ("DATA.BIN", bytes(range(256)) * 4),
    ("RUN.BAT", b"@ECHO OFF\r\nECHO run\r\n"),
    ("EMPTY.DAT", b""),
]

# The contents of C:\STAMPS: files whose sizes and time stamps cover what
# the program has to cope with when it shows them. Name, size, time stamp.
# The sizes are only written into the directory; the files are empty.
STAMPS_DIRECTORIES = ["SUB"]
STAMPS_FILES = [
    ("FIRST.DAT", 0, timestamp(1980, 1, 1, 0, 0, 0)),
    ("NIGHT.DAT", 9, timestamp(1999, 12, 31, 0, 0, 58)),
    ("MORNING.DAT", 1234, timestamp(2001, 2, 3, 9, 5, 6)),
    ("NOON.DAT", 1048576, timestamp(2010, 10, 10, 12, 30, 0)),
    ("EVENING.DAT", 999999999, timestamp(2099, 11, 30, 23, 59, 58)),
    ("LAST.DAT", 4294967295, timestamp(2107, 12, 31, 13, 1, 2)),
    ("HUGE.DAT", 1000000000, timestamp(2000, 6, 16, 12)),
]

# C:\FILES: files to work on that are not the bench's own, for the
# scenarios that ask for them (`files` of scenarios.Scenario), and on
# their benches alone, since the bench's disks are on every golden. It
# is laid out alike on every platform, H:\FILES on Linux (CLAUDE.md, step
# 6, the user's word of 15 October 2026): where a scenario went to A:,
# the diskette of FreeDOS, or to D:\VC, the original's directory, only
# for a file to work on, it works here. So it holds what they went there
# for, as FreeDOS's boot diskette and the distribution have it, with
# their times: the files of the diskette's FREEDOS\BIN in BIN, of which
# a panel shows 44 entries with `..`, and three files in front of it,
# of other days, the first two from the diskette, the third from the
# distribution.
FILES_DIRECTORY = "FILES"
_FILES_OF_THE_DISKETTE = (("KERNEL.SYS", "KERNEL.SYS"),
                          ("SETUP.EN", "FREEDOS/NLS/SETUP.EN"))
_FILES_OF_THE_DISTRIBUTION = ("VCVIEW.EXT",)
_FILES_BIN = "BIN"


@functools.cache
def files_layout():
    """What C:\\FILES holds, in the order in which it is made: (path
    under FILES, with / between names; the contents, or None for a
    directory; the time). The files of the diskette are taken out of it
    by mtools, with their times as its directory has them, read as UTC
    as the bench's disks are written."""
    with zipfile.ZipFile(deps.fetch(deps.FREEDOS)) as archive:
        image = archive.read("144m/x86BOOT.img")
    with dos.workdir() as directory:
        directory = Path(directory)
        (directory / "boot.img").write_bytes(image)
        environment = dict(os.environ, TZ="UTC", MTOOLS_SKIP_CHECK="1")
        subprocess.run(["mcopy", "-s", "-m", "-i",
                        str(directory / "boot.img"),
                        "::/KERNEL.SYS", "::/FREEDOS", str(directory)],
                       env=environment, check=True, capture_output=True)

        def found(path):
            # mtools writes a name as the diskette's long name has it,
            # which is in small letters.
            at = directory
            for name in path.split("/"):
                at = next(entry for entry in at.iterdir()
                          if entry.name.upper() == name)
            return at

        def taken(path):
            return path.read_bytes(), int(path.stat().st_mtime)

        out = [(name, *taken(found(source)))
               for name, source in _FILES_OF_THE_DISKETTE]
        files = distribution()
        out += [(name, *files[name]) for name in _FILES_OF_THE_DISTRIBUTION]
        out.append((_FILES_BIN, None, EPOCH))
        out += [(f"{_FILES_BIN}/{path.name.upper()}", *taken(path))
                for path in sorted(found("FREEDOS/BIN").iterdir())]
    return tuple(out)


# The listing of the build of the published source that is laid out as
# the original is, and is the same byte for byte: it says where the
# original has its routines and its variables.
LISTING = deps.ROOT / "build" / "shareware-layout" / "VC.LST"
# And the same of the setup program.
SETUP_LISTING = LISTING.with_name("VCSETUP.LST")


@dataclass(frozen=True)
class Program:
    """A build of Volkov Commander, and the file name it is run under."""
    filename: str
    contents: bytes
    # The listing of its assembly, where the build has written one: the
    # driver of the tests reads there where the program keeps what it
    # watches. Two programs are the same if their files are.
    listing: Path = field(default=None, compare=False)
    # Of a C tree, the linker's map: where its stack is.
    linked: Path = field(default=None, compare=False)
    # Of a C tree, which one: the name of its directory in build/, "c"
    # or "portable_c". None for the original. A tree may be declared to
    # behave otherwise than the original (see deviations.py), and is
    # then held against what is declared for it.
    tree: str = field(default=None, compare=False)
    # Of the program for Linux, its file, which is run where it is.
    path: Path = field(default=None, compare=False)

    @classmethod
    def load(cls, path):
        """The program in the file `path`, with the listing that the
        build writes next to it, or with the linker's map if it is the
        translation."""
        path = Path(path)
        contents = path.read_bytes()
        if contents[:4] == b"\x7fELF":
            # The program for Linux, which runs on the host and not in a
            # machine (see posixrun.py): the tree "posix", of the source of
            # the portable C. What the goldens call the program under
            # test is VC.EXE, which its bench has not.
            return cls("VC.EXE", contents, None, None, "posix", path)
        if contents[:2] == b"MZ":
            # The translation has no assembly of its own to list: its main
            # data is laid out as the original's, and what the driver
            # watches there is where the original has it.
            # The tree is the directory that the file is in, or the one
            # above that if it is named as a tree is and this one is
            # not: the portable C for finding a fault is in a directory
            # of that tree's, build/portable_c/debug.
            above = path.resolve().parents
            tree = next((directory.name for directory in above[:2]
                         if directory.name in deviations.TREES),
                        above[0].name)
            return cls(path.name.upper(), contents, LISTING,
                       path.with_suffix(".MAP"), tree)
        listing = path.with_suffix(".LST")
        if not listing.exists():
            # A copy of the original has the listing of the original.
            listing = LISTING if contents == original_vc().contents else None
        return cls(path.name.upper(), contents, listing)


def distribution():
    """The files of the shareware distribution: name -> (contents, date)."""
    with zipfile.ZipFile(deps.fetch(deps.VC_SHAREWARE)) as archive:
        return {member.filename: (archive.read(member),
                                  timestamp(*member.date_time))
                for member in archive.infolist()}


def original_vc():
    """The original VC.COM."""
    return Program("VC.COM", distribution()["VC.COM"][0], LISTING)


def original_setup():
    """The original VCSETUP.COM."""
    return Program(SETUP_PROGRAM, distribution()[SETUP_PROGRAM][0],
                   SETUP_LISTING)


def beside(program):
    """The program that `program` has next to it in D:\\VC for a scenario
    of the setup program: the original VCSETUP.COM for the original,
    which is two programs, and for any other program that is no C tree's;
    none for a build of a C tree, which has the setup in itself, as the
    portable C is to, or not at all."""
    return None if program.tree else original_setup()


def _floppy(workdir, commands, country=None, keyboard=None, bench=None):
    """A boot floppy that runs `commands`, for a country of COUNTRIES and
    with a keyboard of KEYBOARDS, if they are given. With `bench`, the
    name of a country, it has what the test bench of that country has
    besides: the driver of the tests, next to the programs of DOS, for
    the commands to load."""
    if keyboard:
        layout, codepage, layouts = keyboard
        directory = r"A:\FREEDOS\BIN"
        commands = [rf"KEYB {layout},{codepage},{directory}\{layouts}>NUL",
                    *commands]
    floppy = boot_floppy(workdir / "a.img", workdir, deps.fetch(deps.FREEDOS),
                         EPOCH, commands, country)
    # The keyboard driver and its layouts are two packages on the Lite USB
    # image, which is a hard disk, and the mouse driver is a third.
    packages = []
    if keyboard:
        packages += [("keyb", "BIN/KEYB.EXE"), ("keyb_lay", f"BIN/{layouts}")]
    if bench:
        packages += [MOUSE]
    if packages:
        image = workdir / "lite.img"
        with zipfile.ZipFile(deps.fetch(deps.FREEDOS_LITE)) as archive:
            image.write_bytes(archive.read("FD14LITE.img"))
        lite = FatImage(image, workdir, EPOCH, partitioned=True)
        for package, member in packages:
            packed = io.BytesIO(lite.read(f"packages/base/{package}.zip"))
            with zipfile.ZipFile(packed) as archive:
                floppy.write(f"FREEDOS/{member}", archive.read(member), EPOCH)
        image.unlink()
    if bench:
        floppy.write(f"FREEDOS/BIN/{driver.FILENAME}",
                     driver.program(PLACE[bench]), EPOCH)
    return floppy


def fresh_install(workdir):
    """Boot the fresh-install machine and return it with VC starting."""
    # The serial number is the day work on the program began, 16.05.1991.
    disk = FatImage.hard_disk(workdir / "c.img", workdir, EPOCH,
                              label="VOLKOV", serial="19910516")
    disk.mkdir("VC")
    for name, (data, date) in distribution().items():
        disk.write(f"VC/{name}", data, date)
    floppy = _floppy(workdir, ["C:", r"CD \VC", "VC"])
    return Machine(floppy.path, [disk.path], workdir, CLOCK)


# The countries DOS can be configured for here: country code, code page.
COUNTRIES = {"us": None, "ru": (7, 866)}


def codepage(country):
    """The code page that DOS shows characters in for `country`."""
    return (COUNTRIES[country] or (1, 437))[1]


# The keyboards of those countries that are not the one the PC has by
# itself: the layout that the keyboard driver of FreeDOS is loaded with,
# for which code page, and the file of the driver's that has it. The
# Russian keyboard types Latin letters until Alt and the right Shift
# select its Cyrillic letters; Alt and the left Shift go back.
KEYBOARDS = {"ru": ("RU", 866, "KEYBRD2.SYS")}

# The mouse driver of the test bench: FreeDOS's CuteMouse, a package of
# the Lite USB image, and its file there. The bench's machine has a PS/2
# mouse, as every PC of QEMU's has.
MOUSE = ("ctmouse", "BIN/CTMOUSE.EXE")
# The command that loads it. CuteMouse takes a PS/2 mouse for one of two
# buttons unless it is told of the third, with /3; the mouse of the bench
# has three.
LOAD_MOUSE = "CTMOUSE /3>NUL"

# How many paragraphs of memory the driver of the tests keeps beyond its
# own on the bench of each country, which decides where DOS loads the
# program. They are chosen so that the original's stack ends where a page
# of the machine's memory ends, 4,096 bytes: QEMU is slow to write to a
# page that has code in it which it has run, and behind its stack the
# program has its code. The translation's stack ends 249 bytes short of a
# page like that, and few of them are code that runs. place.py says where
# the stacks end, and what to write here should that have to change: the
# memory map of the scenarios shows the driver with what it keeps, so
# their goldens change with it.
PLACE = {"us": 176, "ru": 159}

# What DOS runs on the test bench once it has booted. It loads the driver
# of the tests and then waits for a key: there the driver, which has no
# script yet, stops the machine, and the machine's state is saved. Every
# scenario begins there, with the Enter that its script has first.
BENCH_COMMANDS = [LOAD_MOUSE, Path(driver.FILENAME).stem, "PAUSE>NUL",
                  "CLS", "C:", r"CD \WORK", r"D:\VC\VC"]
# And on a bench with the program at home on drive C: (see Bench), where
# it is started in its own directory.
HOME_COMMANDS = [*BENCH_COMMANDS[:-3], "C:", r"CD \VC", "VC"]


class Bench:
    """The test bench: a machine for every scenario, each the same.

    DOS is booted once, configured for `country`, and the machine's state
    is saved as it waits for a key before starting the program, where the
    driver of the tests, loaded and without a script, has stopped it.
    start() then makes a machine in that state, with fresh disks and a
    script, in a fraction of a second. Nothing a scenario does can reach
    the next one, and nothing of the host's time is in that state: the
    machine has executed the same instructions by then every time it is
    booted, and its clock is counted in them (see CLOCK_SHIFT in dos.py).

    The program is set up as its distribution sets it up, but for three
    things ("Scenarios" in CLAUDE.md). The clock in the corner of the
    screen, which the distribution has on, is off. And the panel on the
    right lists files, in the form of the left one, where the distribution
    has the Info panel there: that shows the size of the memory, the label
    of the disk and what is free on it, things of the machine and of the
    system, and with it on nearly every screen of nearly every scenario a
    difference there was a difference everywhere. A scenario that is about
    the Info panel switches it on, Ctrl+l. And both panels sort their
    files by name, where the distribution has them unsorted, as the disk
    has them, an order that means something on DOS alone and that the
    portable C has not. No setup file is prepared for any of the three:
    the original is started once, made to switch the clock off, to make
    the right panel one of files and to sort both by name in its menus,
    and to save its setup, and what it then writes is the VC.INI of every machine. It
    writes the same in every country, so a bench can be given the `setup`
    of another to use.

    With `home` "C", the program is not on a disk of its own: it is in
    C:\\VC, with what it reads there, next to C:\\WORK and C:\\STAMPS,
    and is started in that directory, and D: is an empty disk. That is
    for a picture of the program (see screenshot.py), which then shows
    it as a user has it; no scenario is run so, and the setup file is
    the one that the original saves on such a bench.
    """

    def __init__(self, workdir, country="us", setup=None, memory=None,
                 home="D", files=False):
        self._workdir = Path(workdir)
        self._home = home
        # The machine's memory in megabytes, dos.MEMORY if None: a state
        # is loaded into a machine of the memory that it was saved of.
        self._memory = memory or dos.MEMORY
        self._workdir.mkdir(parents=True, exist_ok=True)
        self._floppy = _floppy(self._workdir,
                               HOME_COMMANDS if home == "C" else
                               BENCH_COMMANDS,
                               COUNTRIES[country], KEYBOARDS.get(country),
                               bench=country).path
        self._work = self._workdir / "c.img"
        # C:\FILES, on the bench of the scenarios that ask for it, is on
        # its disk from the start: DOS has been booted with that disk.
        _work_disk(self._work, self._workdir, files)
        if home == "C":
            self._empty = FatImage.hard_disk(
                self._workdir / "empty.img", self._workdir, EPOCH,
                label="PROGRAM", serial="20000617").path
        self._programs = {}             # Program, and the one beside it
                                        # or None -> the images of C:, D:
        self._lock = threading.Lock()
        self._state = self._workdir / "state"
        shipped = distribution()[SETUP_FILE][0]
        blank = self._workdir / "script.img"
        blank.write_bytes(bytes(driver.SCRIPT_DISK))
        with Machine(self._floppy,
                     [self._work, self._new_disk("boot.img", None, shipped),
                      blank], self._workdir, CLOCK,
                     memory=self._memory) as machine:
            machine.wait_for_boot()
            # The driver stops the machine when it is loaded and has
            # found no script.
            machine.wait_for_stop(BOOT_TIME,
                                  "the driver of the tests did not load")
            machine.save(self._state)
        self.setup = setup or self._setup_of_the_bench(shipped)

    def _setup_of_the_bench(self, shipped):
        """The setup file that the original writes in place of `shipped`,
        the distribution's, once its clock has been switched off, its
        right panel made one of files and both panels made to sort by
        name."""
        workdir = self._workdir / "setup"
        workdir.mkdir()
        original = original_vc()
        disks = self._disks("setup", original, shipped)
        script = driver.script([CLOCK_KEYS, RIGHT_PANEL_KEYS, *BY_NAME_KEYS,
                                SAVE_SETUP_KEYS], driver.facts(original))
        with self._machine(workdir, disks, script) as machine:
            machine.resume()
            machine.wait_for_end(BOOT_TIME)
            if not driver.read(machine.written())[1]:
                raise MachineError("the original did not get through "
                                   "saving its setup")
            written = machine.disks[self._home]
        # The machine has been shut down: the disk image is complete.
        setup = FatImage(written, workdir, EPOCH,
                         partitioned=True).read(f"VC/{SETUP_FILE}")
        if setup == shipped:
            raise RuntimeError("the original did not save its setup with "
                               "the clock switched off and files in its "
                               "right panel")
        return setup

    def _new_disk(self, filename, program, setup, beside=None):
        """Build drive D: in the file `filename`: `program`, if any, and
        what it reads from its directory, with `setup` as its setup file;
        and `beside`, a second program, if any, behind those: what every
        disk has is the first of the directory on every disk."""
        disk = FatImage.hard_disk(self._workdir / filename, self._workdir,
                                  EPOCH, label="PROGRAM", serial="20000617")
        return _fill(disk, program, setup, beside)

    def _disks(self, name, program, setup, beside=None):
        """The images of drives C: and D: with `program` in its home, made
        under names that begin with `name`."""
        if self._home == "D":
            return self._work, self._new_disk(f"{name}.img", program,
                                              setup, beside)
        work = _work_disk(self._workdir / f"{name}-c.img", self._workdir)
        return _fill(work, program, setup, beside), self._empty

    def _program_disk(self, program, beside=None):
        """The image of drive D: with `program` on it, and with `beside`
        next to it, if that is given."""
        return self._program_disks(program, beside)[1]

    def _program_disks(self, program, beside=None):
        """The images of drives C: and D: with `program` in its home, and
        with `beside` next to it, if that is given."""
        with self._lock:                # scenarios may run side by side
            if (program, beside) not in self._programs:
                self._programs[program, beside] = self._disks(
                    f"d{len(self._programs)}", program, self.setup, beside)
            return self._programs[program, beside]

    def _machine(self, workdir, images, script, trace=None, traced=None):
        """A machine with copies of `images` as drives C: and D: and
        `script` for the driver, in which the program is about to start:
        paused."""
        disks = {"C": workdir / "c.img", "D": workdir / "d.img"}
        for image, disk in zip(images, disks.values()):
            shutil.copyfile(image, disk)
        image = workdir / "script.img"
        with open(image, "wb") as file:
            file.write(script)
            file.truncate(driver.SCRIPT_DISK)
        machine = Machine(self._floppy, [*disks.values(), image], workdir,
                          CLOCK, trace=trace, state=self._state,
                          memory=self._memory, traced=traced)
        machine.disks = disks
        return machine

    def start(self, workdir, program, script, trace=None, beside=None,
              traced=None):
        """A machine in which `program` is about to start and the driver
        is about to go by `script` (see driver.script): paused, until
        resume(). With `beside`, that program is in D:\\VC as well, for
        the steps to start: see beside().

        It has a `disks` attribute: drive letter -> image file, to read
        once the machine has been shut down.
        """
        return self._machine(Path(workdir),
                             self._program_disks(program, beside),
                             script, trace, traced)


def _fill(disk, program, setup, beside):
    """Put `program`, if any, into the directory VC of `disk` with what it
    reads there, with `setup` as its setup file, and `beside`, a second
    program, if any, behind those: what every disk has is the first of
    the directory on every disk. The image's file."""
    disk.mkdir("VC")
    files = distribution()
    if program:
        disk.write(f"VC/{program.filename}", program.contents, EPOCH)
    for name in SUPPORT_FILES:
        data, date = files[name]
        disk.write(f"VC/{name}", setup if name == SETUP_FILE else data,
                   date)
    if beside:
        disk.write(f"VC/{beside.filename}", beside.contents, EPOCH)
    return disk.path


def _work_disk(path, workdir, files=False):
    """Build drive C: of the test bench, with C:\\FILES if `files`."""
    work = FatImage.hard_disk(path, workdir, EPOCH,
                              label="WORK", serial="20000616")
    work.mkdir("WORK")
    for name in WORK_DIRECTORIES:
        work.mkdir(f"WORK/{name}")
    for name, data in WORK_FILES:
        work.write(f"WORK/{name}", data, EPOCH)
    work.mkdir("STAMPS")
    for name in STAMPS_DIRECTORIES:
        work.mkdir(f"STAMPS/{name}")
    for name, size, time in STAMPS_FILES:
        work.write(f"STAMPS/{name}", b"", time)
        work.set_size(name, size)
    if files:
        work.mkdir(FILES_DIRECTORY)
        for name, data, time in files_layout():
            if data is None:
                work.mkdir(f"{FILES_DIRECTORY}/{name}")
            else:
                work.write(f"{FILES_DIRECTORY}/{name}", data, time)
    return work


class Benches:
    """A bench for each country that is asked for, booted when first used:
    benches[country], or benches[country, memory] for a machine of so
    many megabytes (see `memory` of scenarios.Scenario), or
    benches[country, memory, True] for one with C:\\FILES (`files`
    there; memory None for the bench's): a scenario says which by its
    `bench`. To be used in a `with` statement, which
    removes them.
    """

    def __enter__(self):
        self._directory = dos.workdir()
        self._benches = {}
        self._lock = threading.Lock()
        # One for each bench, so that a scenario whose bench is there is
        # not kept waiting while another bench boots; and the setup file
        # that the first bench made, which every other is given.
        self._locks = {}
        self._first = threading.Lock()
        self._setup = None
        return self

    def __exit__(self, *exc):
        self._directory.cleanup()

    def __getitem__(self, key):
        country, memory, files = (key + (False,))[:3] \
            if isinstance(key, tuple) else (key, None, False)
        with self._lock:                # scenarios may run side by side
            lock = self._locks.setdefault(key, threading.Lock())
        with lock:
            if key not in self._benches:
                directory = Path(self._directory.name) / (
                    country + (f"-{memory}" if memory else "")
                    + ("-files" if files else ""))
                with self._first:
                    if self._setup is None:
                        bench = Bench(directory, country, memory=memory,
                                      files=files)
                        self._setup = bench.setup
                    else:
                        bench = None
                if bench is None:
                    bench = Bench(directory, country, self._setup,
                                  memory=memory, files=files)
                self._benches[key] = bench
            return self._benches[key]
