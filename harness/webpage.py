"""Run the web page's program as its static site serves it, in Node.

The piece W3b of step 7 (TODO.md, "Step 7, the web page"): the page,
build/portable_c/web/vc.html, is the program built to WebAssembly with
the page's script, served as a static site (src/portable_c/web/
static.mjs), from which it loads its one drive, C:, into its own memory,
where every change stays until the page is closed or loaded again
(drives.js). No browser is downloaded for a test, so harness/webpage.mjs
runs the page's scripts as the browser would, in Node, with a document of
nothing but what the page asks of one, the site made in a directory of
build/tmp with the test's own files on C: beside C:\\VC
(harness/webfiles.mjs) and served by src/portable_c/web/serve.mjs, and
types into it as the page's keys do:

  - the drive loaded and the program started in C:\\VC;
  - the bar's one link to the licences, which shows the text of every
    file of the repository that the page carries (LICENCES of
    src/portable_c/web/bundle.mjs), and is closed by Esc and by its
    button with no key of it reaching the program; and its line that
    changes last until the page is reloaded or closed;
  - Alt-F2, the drive box with A: and C:, and C:\\ by ..;
  - F7, a directory made in the page's memory;
  - Enter of the row of keys on a file, opened in a tab with its bytes;
  - Shift-F4 and the editor, a file written;
  - F8, a directory and a file removed;
  - Shift-F9, the setup saved on C:, C:\\VC\\VC.INI;
  - F10, the program ended, and the page saying that it has;
  - and nothing kept: no store of the browser's asked for, and nothing
    of the site changed.

Then the page loaded again, its drive as it is served, with none of the
changes; the site that the build has made to be shipped, its C:\\VC
with the program's files, C:\\ with AUTOEXEC.BAT and CONFIG.SYS, the
clock on, A: empty and changed to and back, and the strip of the keys that
the browser keeps with its hint, Ctrl-PgUp of it taken; and a host that
serves the page and no drive, where the page says so and does not start.

What the browser draws, and its keys, the mouse and the phone, are held
by portable_c-web-browser. The test exits with 77, which the build takes
for a test that is skipped, where there is no Node or no page.
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

import dos

SKIPPED = 77
HERE = Path(__file__).resolve().parent
WEB = HERE.parent / "src" / "portable_c" / "web"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("page", type=Path, help="build/portable_c/web/vc.html")
    args = parser.parse_args()
    node = shutil.which("node")
    if node is None or not args.page.exists():
        print("no Node or no page: skipped")
        return SKIPPED
    with dos.workdir() as work:
        return subprocess.run([node, str(HERE / "webpage.mjs"), str(WEB),
                               str(args.page.resolve()), work],
                              timeout=600).returncode


if __name__ == "__main__":
    sys.exit(main())
