"""Run the web page's program against the page's server, in Node.

The piece W3b of step 7 (TODO.md, "Step 7, the web page"): the page,
build/portable_c/web/vc.html, is the program built to WebAssembly with
the page's script, and keeps its one drive, C:, on the server,
src/portable_c/web/server.mjs, asking it for every change before it makes
the change in its own file system (drives.js, by the hooks of
files_posix.c and sys_web.c, awaited under Asyncify). No browser is
downloaded for a test, so harness/webpage.mjs runs the page's scripts as
the browser would, in Node, with a document of nothing but what the page
asks of one, against the server on a port of its own with the drive in
a directory of build/tmp, the test's own files on it beside C:\VC
(harness/webfiles.mjs), and types into it as the page's keys do:

  - the drive loaded and the program started in C:\VC;
  - the bar's one link to the licences, which shows the text of every
    file of the repository that the page carries (LICENCES of
    src/portable_c/web/bundle.mjs), and is closed by Esc and by its
    button with no key of it reaching the program;
  - Alt-F2, the drive box with C: alone, and C:\ by ..;
  - F7, a directory made on the server;
  - Enter on a file, opened in a tab with its bytes;
  - Shift-F4 and the editor, a file written, which reaches the server
    when the editor closes it;
  - F8, a directory removed on the server;
  - the server refusing (C:'s root made read-only on the host): DOS's
    error on the screen, and nothing made;
  - Shift-F9, the setup saved on C: of the server, C:\\VC\\VC.INI;
  - F10, the program ended, and the page saying that it has;
  - and nothing kept in the browser: the page did not open IndexedDB.

Then the same page as a static site, made by src/portable_c/web/
static.mjs and served by a host that answers 404 for what it has not, as
GitHub Pages does, its C: as static.mjs ships it, with vc alone, and
then with the test's files served in its place, with a stand-in of the browser's IndexedDB
(harness/fakeidb.mjs) that is one browser across the page's visits (the
piece W3c, TODO.md):

  - the page finding no server and saying that its changes are kept in
    the browser, with the button of the reset;
  - F7, Shift-F4 and the editor, F8 and Shift-F9: a directory made, a
    file written, a file of the test's deleted and the setup saved on C:,
    each kept in IndexedDB;
  - the page loaded again: its drive as it was left;
  - the reset: the store emptied and the page loaded anew, and then the
    drive as it is served;
  - a browser with no IndexedDB: the page saying that changes last as
    long as it does, and a directory made in its memory.

What the browser draws, and its keys, the mouse and the phone, are
checked by hand (TODO.md, "W3b"). The test exits with 77, which the build
takes for a test that is skipped, where there is no Node or no page.
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
