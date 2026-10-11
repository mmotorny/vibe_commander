"""Where a build is meant to behave differently from the original.

Each deviation is declared here, with its reason, and the comparison then
overlooks exactly that. Everything else has to match.
"""
import re
from dataclasses import replace

from dos import COLUMNS


def declared(screen):
    """`screen` with every declared deviation blanked out."""
    cells = bytearray(screen.cells)
    for row, line in enumerate(screen.lines()):
        for pattern in (EDITION,):
            match = pattern.search(line)
            if match:
                for column in range(*match.span(1)):
                    cells[(row * COLUMNS + column) * 2] = ord("#")
    return replace(screen, cells=bytes(cells))


# The banner that the program prints when it starts names its edition. The
# published source is of another edition than the binaries shipped in 2000,
# and what those have in its place is not reproduced here. The banner stays
# on the screen behind the panels.
EDITION = re.compile(r"^The Volkov Commander, Version 4\.05 (.{9})")
