"""Hold the web page's server and the page's file system to what they do.

The piece W3b of step 7 (TODO.md, "Step 7, the web page"): the page keeps
its one drive, C:, on a server of the project's own, src/portable_c/web/
server.mjs, and mirrors it in Emscripten's file system, which every
change of the program's is asked of the server for first, by
src/portable_c/web/drives.js. Both are of Node's own modules, and this test
runs them in Node, harness/webserver.mjs, with no browser and nothing
downloaded:

  1. The server, listening on a port of its own, with its drive in a
     directory of build/tmp, made with C:\VC, and the test's own files
     laid on it (harness/webfiles.mjs): the page served; its
     answer to `api/server`, by which the page knows a server; a
     drive's tree, parents first, with the contents, and no link of the
     host's in it; a file written whole, with its time and its read-only
     bit, also over a file that is read-only; a directory made, renamed,
     removed, and refused where it is there, not empty, or is renamed
     into itself; a file deleted, and refused where it is a directory or
     is not there; the read-only bit set and cleared. And refused, with
     nothing changed outside the drives: `..` and `.` anywhere, an
     absolute path, a backslash, a NUL, an empty name, a drive that there
     is not, D: among them, an operation that there is not, a body that is no JSON, and
     a link of the host's inside a drive, to a file or to a directory
     outside it, wherever it stands in the path.
  2. The page's file system, drives.js, with a file system of the shape
     of Emscripten's, against the same server: the drive loaded, with
     their times; where a path of the file system is; a file's contents,
     time and read-only bit sent at its close; each change of
     files_posix.c's (enum HostChange) made on the server, with the
     server's refusals made the numbers that sys_web.c makes DOS's
     errors; a file brought in by the user; and a server that is not
     there answering the general failure.
  3. The page with no server, the piece W3c (TODO.md, "W3c"):
     src/portable_c/web/kept.js, its drive loaded from the tree that a
     static page is served with, C: with vc and the test's files, of the server's shape (treeOf of
     server.mjs), and each change kept in the browser's store before it
     is made, once in the page's memory (MemoryStore) and once in
     IndexedDB (IdbStore on harness/fakeidb.mjs, a stand-in of IndexedDB
     in Node): a directory made, a file's contents at its close, a
     directory renamed with what is in it, a file deleted, the read-only
     bit, an upload, and refused, with nothing kept, where the file
     system would refuse it (there already, not empty, not there, a
     directory into itself or out of the drive); the next visit loading
     what was kept and nothing served, what is kept of a drive that the
     page has no longer, D:, forgotten, and a reset the drive as it is
     served again; a store that is full answering DOS's ENOSPC; no
     IndexedDB, or one that refuses to open, the page's memory.
  4. The page's script as bundle.mjs makes it for the shell: one classic
     script, which compiles, takes from each module only what that module
     exports, and puts Module up.

What the browser does with the page is checked by hand (TODO.md, "W3b").
The test exits with 77, which the build takes for a test that is skipped,
where there is no Node.
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
        return subprocess.run([node, str(HERE / "webserver.mjs"), str(WEB),
                               work]).returncode


if __name__ == "__main__":
    sys.exit(main())
