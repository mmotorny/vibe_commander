"""Run the web page's program against the page's server, in Node.

The piece W3b of step 7 (TODO.md, "Step 7, the web page"): the page,
build/portable_c/web/vc.html, is the program built to WebAssembly with
the page's script, and keeps its two drives on the server,
src/portable_c/web/server.mjs, asking it for every change before it makes
the change in its own file system (drives.js, by the hooks of
files_posix.c and sys_web.c, awaited under Asyncify). No browser is
downloaded for a test, so harness/webpage.mjs runs the page's scripts as
the browser would, in Node, with a document of nothing but what the page
asks of one, against the server on a port of its own with two drives in
a directory of build/tmp, and types into it as the page's keys do:

  - the drives loaded and the panel drawn;
  - F7, a directory made on the server;
  - Enter on a file, opened in a tab with its bytes;
  - Shift-F4 and the editor, a file written, which reaches the server
    when the editor closes it;
  - F8, a directory removed on the server;
  - the server refusing (D:'s root made read-only on the host): DOS's
    error on the screen, and nothing made;
  - Shift-F9, the setup saved on C: of the server, C:\\VC\\VC.INI;
  - F10, the program ended, and the page saying that it has.

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
