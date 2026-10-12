"""The machines the harness builds for Volkov Commander.

Two setups share one clock and one source of files:

  fresh install   a single hard disk with the shareware distribution unpacked
                  in C:\\VC, and VC started there. This is what a user saw
                  on first run, and what the picture in the README shows.

  test bench      the program under test on its own disk in D:\\VC, next to
                  the distribution's help and configuration files, and a
                  small fixed set of files on C:. VC is started in
                  C:\\WORK. Keeping the program off the disk it displays
                  means its size cannot show up as free space on screen.
                  The clock in the corner of the screen is off, so that
                  no screen depends on how long it took to get there.
                  DOS has loaded a mouse driver, for the PS/2 mouse of
                  three buttons that the machine has, and the driver of
                  the tests (see
                  driver.py), which takes the program through a
                  scenario's steps from a script on a third disk, one
                  without a file system.
"""
import io
import shutil
import tempfile
import threading
import zipfile
from dataclasses import dataclass, field
from pathlib import Path

import deps
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

# The keys that switch the clock from on to off or back, in the Options
# menu, and those that save the setup.
CLOCK_KEYS = "F9 o l"
SAVE_SETUP_KEYS = "Shift+F9 Enter"
# The contents of C:\WORK on the test bench, in directory order.
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


# The listing of the build of the published source that is laid out as
# the original is, and is the same byte for byte: it says where the
# original has its routines and its variables.
LISTING = deps.ROOT / "build" / "shareware-layout" / "VC.LST"


@dataclass(frozen=True)
class Program:
    """A build of Volkov Commander, and the file name it is run under."""
    filename: str
    contents: bytes
    # The listing of its assembly, where the build has written one: the
    # driver of the tests reads there where the program keeps what it
    # watches. Two programs are the same if their files are.
    listing: Path = field(default=None, compare=False)
    # Of the C translation, the linker's map: where its stack is.
    linked: Path = field(default=None, compare=False)

    @classmethod
    def load(cls, path):
        """The program in the file `path`, with the listing that the
        build writes next to it, or with the linker's map if it is the
        translation."""
        path = Path(path)
        contents = path.read_bytes()
        if contents[:2] == b"MZ":
            # The translation has no assembly of its own to list: its main
            # data is laid out as the original's, and what the driver
            # watches there is where the original has it.
            return cls(path.name.upper(), contents, LISTING,
                       path.with_suffix(".MAP"))
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

    The program is set up as its distribution sets it up, but for the clock
    in the corner of the screen, which the distribution has on. No setup
    file is prepared for that: the original is started once, made to switch
    the clock off in its menu and to save its setup, and what it then
    writes is the VC.INI of every machine. It writes the same in every
    country, so a bench can be given the `setup` of another to use.
    """

    def __init__(self, workdir, country="us", setup=None):
        self._workdir = Path(workdir)
        self._workdir.mkdir(parents=True, exist_ok=True)
        self._floppy = _floppy(self._workdir, BENCH_COMMANDS,
                               COUNTRIES[country], KEYBOARDS.get(country),
                               bench=country).path
        self._work = self._workdir / "c.img"
        _work_disk(self._work, self._workdir)
        self._programs = {}             # Program -> its disk image
        self._lock = threading.Lock()
        self._state = self._workdir / "state"
        shipped = distribution()[SETUP_FILE][0]
        blank = self._workdir / "script.img"
        blank.write_bytes(bytes(driver.SCRIPT_DISK))
        with Machine(self._floppy,
                     [self._work, self._new_disk("boot.img", None, shipped),
                      blank], self._workdir, CLOCK) as machine:
            machine.wait_for_boot()
            # The driver stops the machine when it is loaded and has
            # found no script.
            machine.wait_for_stop(BOOT_TIME,
                                  "the driver of the tests did not load")
            machine.save(self._state)
        self.setup = setup or self._setup_without_clock(shipped)

    def _setup_without_clock(self, shipped):
        """The setup file that the original writes in place of `shipped`,
        the distribution's, once its clock has been switched off."""
        workdir = self._workdir / "setup"
        workdir.mkdir()
        original = original_vc()
        disk = self._new_disk("setup.img", original, shipped)
        script = driver.script([CLOCK_KEYS, SAVE_SETUP_KEYS],
                               driver.facts(original))
        with self._machine(workdir, disk, script) as machine:
            machine.resume()
            machine.wait_for_end(BOOT_TIME)
            if not driver.read(machine.written())[1]:
                raise MachineError("the original did not get through "
                                   "saving its setup")
            written = machine.disks["D"]
        # The machine has been shut down: the disk image is complete.
        setup = FatImage(written, workdir, EPOCH,
                         partitioned=True).read(f"VC/{SETUP_FILE}")
        if setup == shipped:
            raise RuntimeError("the original did not save its setup with "
                               "the clock switched off")
        return setup

    def _new_disk(self, filename, program, setup):
        """Build drive D: in the file `filename`: `program`, if any, and
        what it reads from its directory, with `setup` as its setup file."""
        disk = FatImage.hard_disk(self._workdir / filename, self._workdir,
                                  EPOCH, label="PROGRAM", serial="20000617")
        disk.mkdir("VC")
        files = distribution()
        if program:
            disk.write(f"VC/{program.filename}", program.contents, EPOCH)
        for name in SUPPORT_FILES:
            data, date = files[name]
            disk.write(f"VC/{name}", setup if name == SETUP_FILE else data,
                       date)
        return disk.path

    def _program_disk(self, program):
        """The image of drive D: with `program` on it."""
        with self._lock:                # scenarios may run side by side
            if program not in self._programs:
                self._programs[program] = self._new_disk(
                    f"d{len(self._programs)}.img", program, self.setup)
            return self._programs[program]

    def _machine(self, workdir, program_disk, script, trace=None):
        """A machine with a copy of `program_disk` as drive D: and `script`
        for the driver, in which the program is about to start: paused."""
        disks = {"C": workdir / "c.img", "D": workdir / "d.img"}
        shutil.copyfile(self._work, disks["C"])
        shutil.copyfile(program_disk, disks["D"])
        image = workdir / "script.img"
        with open(image, "wb") as file:
            file.write(script)
            file.truncate(driver.SCRIPT_DISK)
        machine = Machine(self._floppy, [*disks.values(), image], workdir,
                          CLOCK, trace=trace, state=self._state)
        machine.disks = disks
        return machine

    def start(self, workdir, program, script, trace=None):
        """A machine in which `program` is about to start and the driver
        is about to go by `script` (see driver.script): paused, until
        resume().

        It has a `disks` attribute: drive letter -> image file, to read
        once the machine has been shut down.
        """
        return self._machine(Path(workdir), self._program_disk(program),
                             script, trace)


def _work_disk(path, workdir):
    """Build drive C: of the test bench."""
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
    return work


class Benches:
    """A bench for each country that is asked for, booted when first used:
    benches[country]. To be used in a `with` statement, which removes them.
    """

    def __enter__(self):
        self._directory = tempfile.TemporaryDirectory()
        self._benches = {}
        self._lock = threading.Lock()
        return self

    def __exit__(self, *exc):
        self._directory.cleanup()

    def __getitem__(self, country):
        with self._lock:                # scenarios may run side by side
            if country not in self._benches:
                made = [bench.setup for bench in self._benches.values()]
                self._benches[country] = Bench(
                    Path(self._directory.name) / country, country, *made[:1])
            return self._benches[country]
