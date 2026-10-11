#!/usr/bin/env python3
"""Third-party files the harness needs.

They are downloaded on demand into downloads/ (which git ignores) and are
never stored in the repository. Each one is pinned by its SHA-256.

Run this file to fetch them all.
"""
import hashlib
import sys
import urllib.request
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOWNLOADS = ROOT / "downloads"


@dataclass(frozen=True)
class Download:
    filename: str
    url: str
    sha256: str
    description: str

    @property
    def path(self):
        return DOWNLOADS / self.filename


# The Volkov Commander 4.05 shareware distribution, from its author's site.
VC_SHAREWARE = Download(
    "vc405sw.zip",
    "https://vc.vvv.kyiv.ua/download/vc405sw.zip",
    "3c43b8b12d9b986d4f09c70179d845694bda5872ebc2531357ed07ca5bd0ed6b",
    "Volkov Commander 4.05 shareware distribution")

# The FreeDOS 1.4 Floppy Edition, from the project's download host; its
# boot floppy is the DOS the harness runs.
FREEDOS = Download(
    "FD14-FloppyEdition.zip",
    "https://download.freedos.org/1.4/FD14-FloppyEdition.zip",
    "45b1fa7c52dd996c3bfa5e352ffcd410781b952a6ad629f15a4c9ec4bbaefc5a",
    "FreeDOS 1.4 Floppy Edition")

# The FreeDOS 1.4 Lite USB image, from the same host. It holds the packages
# of the release, and among them the keyboard driver and its layouts, which
# the boot floppy lacks.
FREEDOS_LITE = Download(
    "FD14-LiteUSB.zip",
    "https://download.freedos.org/1.4/FD14-LiteUSB.zip",
    "857dcd2ebf9d3d094320154db5fb5b830acba6fb98f981a95a0ca7ab3350338b",
    "FreeDOS 1.4 Lite USB image")

ALL = [VC_SHAREWARE, FREEDOS, FREEDOS_LITE]


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            digest.update(block)
    return digest.hexdigest()


def fetch(download):
    """Return the local path of a download, fetching it if necessary."""
    path = download.path
    if not path.exists():
        DOWNLOADS.mkdir(exist_ok=True)
        print(f"downloading {download.description}", file=sys.stderr)
        partial = path.with_name(path.name + ".part")
        with urllib.request.urlopen(download.url) as response, \
                open(partial, "wb") as f:
            while block := response.read(1 << 16):
                f.write(block)
        partial.rename(path)
    actual = sha256(path)
    if actual != download.sha256:
        sys.exit(f"{path}: SHA-256 is {actual},\n"
                 f"expected {download.sha256}; delete the file to retry")
    return path


if __name__ == "__main__":
    for item in ALL:
        print(fetch(item))
