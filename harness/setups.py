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
"""
import zipfile
from dataclasses import dataclass
from pathlib import Path

import deps
from dos import FatImage, Machine, boot_floppy, timestamp

# The PC's clock at power-on, and the date given to directories and to the
# test bench's files: noon on the day Volkov Commander 4.05 was released.
CLOCK = "2000-06-16T12:00:00"
EPOCH = timestamp(2000, 6, 16, 12)
DOS_CLOCK = ((2000 - 1980) << 9 | 6 << 5 | 16, 12 << 11)   # date, time words

# What VC reads from its own directory besides the program itself.
SUPPORT_FILES = ["VC.HLP", "VC.INI", "VC.EXT", "VCVIEW.EXT", "VCEDIT.EXT",
                 "VC.MNU", "ARCHIVES.MNU", "FORMAT.MNU"]

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


# Where VC.INI keeps the settings that scenarios change. The file is three
# letters V, the saved state of the left and of the right panel (WCBdef in
# the source, 88 bytes of it), the settings (MainDat in the source), and a
# check sum of two bytes.
INI_PANELS = (3, 3 + 88)
INI_SETTINGS = 3 + 2 * 88
SETTINGS = {
    "LeftFull": INI_PANELS[0] + 1,      # the left panel shows size, date, time
    "Clock": INI_SETTINGS + 247,        # the clock in the top right corner
}


@dataclass(frozen=True)
class Program:
    """A build of Volkov Commander, and the file name it is run under."""
    filename: str
    contents: bytes

    @classmethod
    def load(cls, path):
        path = Path(path)
        return cls(path.name.upper(), path.read_bytes())


def distribution():
    """The files of the shareware distribution: name -> (contents, date)."""
    with zipfile.ZipFile(deps.fetch(deps.VC_SHAREWARE)) as archive:
        return {member.filename: (archive.read(member),
                                  timestamp(*member.date_time))
                for member in archive.infolist()}


def original_vc():
    """The original VC.COM."""
    return Program("VC.COM", distribution()["VC.COM"][0])


def configuration(settings):
    """The distribution's VC.INI with `settings` changed: name -> value."""
    ini = bytearray(distribution()["VC.INI"][0])
    for name, value in settings.items():
        ini[SETTINGS[name]] = value
    ini[-2:] = (sum(ini[:-2]) & 0xFFFF).to_bytes(2, "little")
    return bytes(ini)


def _floppy(workdir, commands, country=None):
    return boot_floppy(workdir / "a.img", workdir, deps.fetch(deps.FREEDOS),
                       EPOCH, commands, country)


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


def test_bench(workdir, program, settings=None, country=None, trace=None):
    """Boot the test bench with `program` as the program under test.

    `settings` changes VC.INI from what the distribution ships, and
    `country` is the country code DOS is configured for, the United States
    if none. The machine returned has a `disks` attribute: drive letter ->
    image file.
    """
    work = FatImage.hard_disk(workdir / "c.img", workdir, EPOCH,
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

    disk = FatImage.hard_disk(workdir / "d.img", workdir, EPOCH,
                              label="PROGRAM", serial="20000617")
    disk.mkdir("VC")
    files = distribution()
    disk.write(f"VC/{program.filename}", program.contents, EPOCH)
    for name in SUPPORT_FILES:
        data, date = files[name]
        if name == "VC.INI":
            data = configuration(settings or {})
        disk.write(f"VC/{name}", data, date)

    floppy = _floppy(workdir, ["C:", r"CD \WORK", r"D:\VC\VC"], country)
    machine = Machine(floppy.path, [work.path, disk.path], workdir, CLOCK,
                      trace=trace)
    machine.disks = {"C": work.path, "D": disk.path}
    return machine
