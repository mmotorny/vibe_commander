"""Hold the web page's static site and the page's drive to what they do.

The pieces W3b and W3c of step 7 (TODO.md, "Step 7, the web page"): the
page is served as a static site, src/portable_c/web/static.mjs makes it
and src/portable_c/web/serve.mjs serves it on this machine, and the page
loads its one drive, C:, from the site into Emscripten's file system,
where every change stays until the page is closed (src/portable_c/web/
drives.js). All three are of Node's own modules, and this test runs them
in Node, harness/website.mjs, with no browser and nothing downloaded:

  1. The site: made of a page, with C: as it is shipped, its directory
     vc alone, and with C: the tree of a directory, the test's own files
     laid on it (harness/webfiles.mjs), parents first, with the contents,
     the times and the read-only bit, and no link of the host's in it.
  2. The server: the page at the root, the files of the site with their
     types, HEAD; and 404, with nothing served from outside the site,
     for a name that is not there, `..` plain and escaped, a
     backslash, a NUL, a link of the host's to a file or to a directory
     outside, and any method but GET and HEAD.
  3. The page's drive, drives.js, with a file system of the shape of
     Emscripten's, loaded from the site as served: the files with their
     contents, times and read-only bit, the root's time; a file brought
     in by the user, and a name that no file can have refused; a site
     that serves no drive, or one that is no tree, refused.
  4. The page's script as bundle.mjs makes it for the shell: one classic
     script, which compiles, takes from each module only what that module
     exports, and puts Module up.

What the browser does with the page is held by portable_c-web-browser.
The test exits with 77, which the build takes for a test that is
skipped, where there is no Node.
"""

import shutil
import subprocess
import sys
from pathlib import Path

import dos

SKIPPED = 77
HERE = Path(__file__).resolve().parent
WEB = HERE.parent / "src" / "portable_c" / "web"


def main():
    node = shutil.which("node")
    if node is None:
        print("no Node: skipped")
        return SKIPPED
    with dos.workdir() as work:
        return subprocess.run([node, str(HERE / "website.mjs"), str(WEB),
                               work]).returncode


if __name__ == "__main__":
    sys.exit(main())
