"""The web page in a real browser: the Chromium that the machine has.

The test portable_c-web-browser (CLAUDE.md, "The web page"; TODO.md,
"W6"): harness/webbrowser.mjs opens build/portable_c/web/vc.html as its
server serves it and the static site, build/portable_c/web/static, as a
host of pages with no server serves it, in a headless Chromium driven over
its DevTools protocol from Node, on a desktop and as an iPhone held both
ways, and holds what it does: see the top of that file. The Chromium is
the one on the path, `chromium` or `chromium-browser`, not fetched or
pinned; where there is none, or no Node, or no page, the test exits with
77, which the build takes for a test that is skipped. Its work directory
is in the build, since Chromium of a snap reads no hidden directory and
not the real /tmp.

For screenshots to look at, run harness/webbrowser.mjs with --shots, as
its top says.
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

SKIPPED = 77
HERE = Path(__file__).resolve().parent
CHROMIUMS = ("chromium", "chromium-browser")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("page", type=Path, help="build/portable_c/web/vc.html")
    parser.add_argument("site", type=Path, help="build/portable_c/web/static")
    parser.add_argument("work", type=Path, help="a directory of the build's")
    args = parser.parse_args()
    node = shutil.which("node")
    chromium = next((found for found in map(shutil.which, CHROMIUMS) if found),
                    None)
    if node is None or chromium is None:
        print("no Node or no Chromium on the path: skipped")
        return SKIPPED
    if not args.page.exists() or not (args.site / "index.html").exists():
        print("no page or no static site: skipped")
        return SKIPPED
    return subprocess.run([node, str(HERE / "webbrowser.mjs"),
                           "--chromium", chromium,
                           "--page", str(args.page.resolve()),
                           "--site", str(args.site.resolve()),
                           "--work", str(args.work.resolve())],
                          timeout=600).returncode


if __name__ == "__main__":
    sys.exit(main())
