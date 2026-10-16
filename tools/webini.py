#!/usr/bin/env python3
"""The setup file that the web page ships in C:\\VC, VC.INI, as the
program saves it (CLAUDE.md, "The web page"; TODO.md, "Step 7, the web
page", "C:\\VC as it is shipped"): the program for the web under Node,
build/portable_c/web-node/vc.js, started in C:\\VC of a drive laid out
as the page's, with no setup file, so on its own defaults, is taken by
its test channel through the keys below, and what it saves is the file.
The clock is switched on, which the defaults have off, so that a user
of the page has it; and the left panel, which the defaults have off, is
switched on and the right one taken to C:\\, so that the page opens on
C:\\VC on the left and C:\\ on the right, the left one current. The tests' bench keeps its own setup
file, with the clock off (CLAUDE.md, "Scenarios").

    python3 tools/webini.py --node NODE --program vc.js --out vc.ini
"""

import argparse
import os
import pathlib
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent
                       / "harness"))
import posixrun  # noqa: E402  the keys of the test channel

# The clock on (Options, cLock), the left panel on, the right one, which
# is current, to the root of C:, the left one made current, and the setup
# saved.
STEPS = ("F9 o l", "Ctrl+F1", "Ctrl+\\", "Tab", "Shift+F9 Enter")


def main():
    told = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    told.add_argument("--node", required=True)
    told.add_argument("--program", required=True)
    told.add_argument("--out", required=True)
    told = told.parse_args()
    with tempfile.TemporaryDirectory() as work:
        work = pathlib.Path(work)
        own = work / "c" / "vc"
        own.mkdir(parents=True)
        channel = work / "channel"
        channel.mkdir()
        (channel / "keys").write_text(posixrun.keys(STEPS))
        environment = {
            "PATH": os.environ.get("PATH", "/usr/bin:/bin"), "TZ": "UTC",
            "HOME": str(work), "VC_TEST": str(channel),
            "VC_TEST_CLOCK": posixrun.CLOCK,
            "VC_DRIVE_C": str(work / "c"), "VC_LABEL_C": "VC",
        }
        done = subprocess.run(
            [told.node, str(pathlib.Path(told.program).resolve())],
            cwd=own, env=environment, stdin=subprocess.DEVNULL,
            capture_output=True, timeout=120)
        said = done.stderr.decode(errors="replace").strip()
        saved = own / "vc.ini"
        if done.returncode != 2 or "keys have run out" not in said \
                or not saved.exists():
            sys.exit(f"webini.py: the program saved no setup file "
                     f"({done.returncode}): {said}")
        pathlib.Path(told.out).write_bytes(saved.read_bytes())


if __name__ == "__main__":
    main()
