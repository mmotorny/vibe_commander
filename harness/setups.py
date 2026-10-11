"""The machines the harness builds for Volkov Commander.

Two setups share one clock and one source of files:

  fresh install   a single hard disk with the shareware distribution unpacked
                  in C:\\VC, and VC started there. This is what a user saw
                  on first run, and what the picture in the README shows.

  test bench      the program under test on its own disk as D:\\VC\\VC.COM,
                  next to the distribution's help and configuration files,
                  and a small fixed set of files on C:. VC is started in
                  C:\\WORK. Keeping the program off the disk it displays
                  means its size cannot show up as free space on screen.
"""
import zipfile

import deps
from dos import FatImage, Machine, boot_floppy, timestamp

# The PC's clock at power-on, and the date given to directories and to the
# test bench's files: noon on the day Volkov Commander 4.05 was released.
CLOCK = "2000-06-16T12:00:00"
EPOCH = timestamp(2000, 6, 16, 12)

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


def distribution():
    """The files of the shareware distribution: name -> (contents, date)."""
    with zipfile.ZipFile(deps.fetch(deps.VC_SHAREWARE)) as archive:
        return {member.filename: (archive.read(member),
                                  timestamp(*member.date_time))
                for member in archive.infolist()}


def original_vc():
    """The original VC.COM."""
    return distribution()["VC.COM"][0]


def _floppy(workdir, commands):
    return boot_floppy(workdir / "a.img", workdir, deps.fetch(deps.FREEDOS),
                       EPOCH, commands)


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


def test_bench(workdir, vc_com):
    """Boot the test bench with `vc_com` as the program under test."""
    work = FatImage.hard_disk(workdir / "c.img", workdir, EPOCH,
                              label="WORK", serial="20000616")
    work.mkdir("WORK")
    for name in WORK_DIRECTORIES:
        work.mkdir(f"WORK/{name}")
    for name, data in WORK_FILES:
        work.write(f"WORK/{name}", data, EPOCH)

    program = FatImage.hard_disk(workdir / "d.img", workdir, EPOCH,
                                 label="PROGRAM", serial="20000617")
    program.mkdir("VC")
    files = distribution()
    program.write("VC/VC.COM", vc_com, EPOCH)
    for name in SUPPORT_FILES:
        data, date = files[name]
        program.write(f"VC/{name}", data, date)

    floppy = _floppy(workdir, ["C:", r"CD \WORK", r"D:\VC\VC"])
    return Machine(floppy.path, [work.path, program.path], workdir, CLOCK)
