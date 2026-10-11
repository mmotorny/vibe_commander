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

# The official FreeDOS 1.3 Floppy Edition; its boot floppy is the DOS the
# harness runs.
FREEDOS = Download(
    "FD13-FloppyEdition.zip",
    "https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/"
    "1.3/official/FD13-FloppyEdition.zip",
    "75a4e11a7fce6f124e20927b3022b4b715a2a3f7324c5f5bfea42d90d80eb072",
    "FreeDOS 1.3 Floppy Edition")

# The keyboard driver of FreeDOS and its layouts, two packages of the
# FreeDOS 1.3 repository. The boot floppy has neither.
FREEDOS_KEYB = Download(
    "keyb.zip",
    "https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/"
    "1.3/base/keyb.zip",
    "8d1f804ef46ab89abe9f278c02a6a185eeb867caed4675e64a081ac16ca89dc8",
    "FreeDOS keyboard driver")
FREEDOS_KEYB_LAYOUTS = Download(
    "keyb_lay.zip",
    "https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/repositories/"
    "1.3/base/keyb_lay.zip",
    "ccb8425dd2df8e3c6937acc4b7edda54c4d6498e01aa810f9c721e88f8e38a7e",
    "FreeDOS keyboard layouts")

ALL = [VC_SHAREWARE, FREEDOS, FREEDOS_KEYB, FREEDOS_KEYB_LAYOUTS]


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
