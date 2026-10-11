"""Read what a FAT16 hard disk image holds, to compare disks after a run.

Only what the comparison needs: the tree of files and directories on the
first partition, with each entry's attributes, time stamp and contents.
"""
import hashlib
import struct
from dataclasses import dataclass

SECTOR = 512
DIRECTORY, VOLUME_LABEL, LONG_NAME = 0x10, 0x08, 0x0F


@dataclass(frozen=True)
class Entry:
    path: str                     # as DOS shows it, e.g. "WORK\\README.TXT"
    attributes: int
    time: int                     # DOS time and date words
    date: int
    size: int
    sha256: str                   # of a file's contents; "" for a directory

    @property
    def is_directory(self):
        return bool(self.attributes & DIRECTORY)


def read_tree(image_path, codepage=437):
    """All files and directories of the image's first partition, by path.
    Names are read as they show in `codepage`."""
    with open(image_path, "rb") as f:
        disk = f.read()
    start = struct.unpack_from("<I", disk, 0x1BE + 8)[0] * SECTOR
    (sector_size, cluster_sectors, reserved, fats, root_entries, _, _,
     fat_sectors) = struct.unpack_from("<HBHBHHBH", disk, start + 11)
    fat = start + reserved * sector_size
    root = fat + fats * fat_sectors * sector_size
    data = root + root_entries * 32
    cluster_size = cluster_sectors * sector_size

    def chain(cluster):
        blocks = []
        while 2 <= cluster < 0xFFF0:
            at = data + (cluster - 2) * cluster_size
            blocks.append(disk[at:at + cluster_size])
            cluster = struct.unpack_from("<H", disk, fat + cluster * 2)[0]
        return b"".join(blocks)

    entries = {}

    def walk(directory, prefix):
        for at in range(0, len(directory), 32):
            raw = directory[at:at + 32]
            if raw[0] == 0:
                break
            attributes = raw[11]
            if (raw[0] == 0xE5 or attributes & LONG_NAME == LONG_NAME
                    or attributes & VOLUME_LABEL or raw[:1] == b"."):
                continue
            name = raw[:8].decode(f"cp{codepage}").rstrip()
            extension = raw[8:11].decode(f"cp{codepage}").rstrip()
            if extension:
                name += "." + extension
            time, date, cluster, size = struct.unpack_from("<HHHI", raw, 22)
            path = prefix + name
            if attributes & DIRECTORY:
                entries[path] = Entry(path, attributes, time, date, 0, "")
                walk(chain(cluster), path + "\\")
            else:
                contents = chain(cluster)[:size]
                entries[path] = Entry(path, attributes, time, date, size,
                                      hashlib.sha256(contents).hexdigest())

    walk(disk[root:data], "")
    return entries
