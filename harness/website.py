r"""Hold the web page's static site and the page's drive to what they do.

The pieces W3b and W3c of step 7 (TODO.md, "Step 7, the web page"): the
page is served as a static site, src/portable_c/web/static.mjs makes it
and src/portable_c/web/serve.mjs serves it on this machine, and the page
loads its one drive, C:, from the site into Emscripten's file system,
where every change stays until the page is closed (src/portable_c/web/
drives.js). All three are of Node's own modules, and this test runs them
in Node, harness/website.mjs, with no browser and nothing downloaded:

  1. The site: made of a page, with C: as it is shipped, its directory
     vc and the files that it is told of, and with C: the tree of a
     directory, the test's own files
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

  5. The site that the build has made, --site, as it is published: its
     C: has in C:\VC the program for DOS, VC.EXE, the same bytes as
     --program where the build has one; LICENSE.TXT, every licence that
     the page carries in one file, the project's first, then Volkov's,
     DOS/32A's and the others, each text whole and in that order;
     the setup file that the program saves, 511 bytes with their sum,
     the clock on; the project's VC.MNU, VC.EXT, VCVIEW.EXT, VCEDIT.EXT
     and its help, VC.HLP; and in C:\ AUTOEXEC.BAT and CONFIG.SYS: those
     and nothing else. No file of it is one of the shareware's
     distribution, nor holds a line of the shareware's help, and none
     is called README (TODO.md, "C:\VC as it is shipped").

What the browser does with the page is held by portable_c-web-browser.
The test exits with 77, which the build takes for a test that is
skipped, where there is no Node.
"""

import argparse
import base64
import json
import shutil
import subprocess
import sys
from pathlib import Path

import dos

SKIPPED = 77
HERE = Path(__file__).resolve().parent
WEB = HERE.parent / "src" / "portable_c" / "web"


ROOT = HERE.parent
# C: as the site ships it, each path and the file of the repository that
# it is, or None for what the build makes (CMakeLists.txt, web_shipped).
SHIPPED = {
    "autoexec.bat": WEB / "drive" / "autoexec.bat",
    "config.sys": WEB / "drive" / "config.sys",
    "vc/license.txt": None,
    "vc/vc.ext": WEB / "drive" / "vc" / "vc.ext",
    "vc/vc.hlp": None,
    "vc/vc.ini": None,
    "vc/vc.mnu": WEB / "drive" / "vc" / "vc.mnu",
    "vc/vcedit.ext": WEB / "drive" / "vc" / "vcedit.ext",
    "vc/vcview.ext": WEB / "drive" / "vc" / "vcview.ext",
}
# The licences that C:\VC\LICENSE.TXT holds, in its order (LICENCES of
# web/bundle.mjs), and what code page 437 lacks of them as the file has it.
LICENCES = [ROOT / "LICENSE", ROOT / "src" / "asm" / "LICENSE.TXT",
            ROOT / "LICENSE.DOS32A", ROOT / "LICENSE.EMSCRIPTEN",
            ROOT / "LICENSE.MUSL", ROOT / "LICENSE.FONT"]
PLAIN = {"\u00a9": "(c)", "\u201c": '"', "\u201d": '"', "\u2018": "'",
         "\u2019": "'"}


def licences_held(text):
    """What is wrong with the shipped LICENSE.TXT: each licence's text has
    to be in it whole, in the order of LICENCES, with CRLF and in code
    page 437."""
    failures = []
    at = 0
    for source in LICENCES:
        wanted = source.read_text("utf-8").replace("\r\n", "\n").rstrip("\n")
        for character, plain in PLAIN.items():
            wanted = wanted.replace(character, plain)
        wanted = wanted.replace("\n", "\r\n").encode("cp437")
        found = text.find(wanted, at)
        if found < 0:
            failures.append(f"vc/license.txt lacks {source.relative_to(ROOT)}"
                            " where it is wanted")
        else:
            at = found + len(wanted)
    if b"This product uses DOS/32 Advanced DOS Extender technology." \
            not in text:
        failures.append("vc/license.txt lacks DOS/32A's acknowledgment")
    if b"\n" in text.replace(b"\r\n", b""):
        failures.append("vc/license.txt has a line that does not end in CRLF")
    return failures


# Where the setup file has the clock, and its sum (posixrun.SETUP_SUM).
SETUP_CLOCK = 0x1AA
SETUP_SUM = 0x1FD


def built_site(site, program):
    """The checks of 5, each failure a line; the count of checks."""
    failures = []
    c = json.loads((site / "drives" / "c.json").read_text())
    files = {entry["path"]: base64.b64decode(entry["data"])
             for entry in c["entries"] if not entry["directory"]}
    directories = [entry["path"] for entry in c["entries"]
                   if entry["directory"]]
    wanted = set(SHIPPED) | ({"vc/vc.exe"} if program else set())
    if directories != ["vc"]:
        failures.append(f"C: has the directories {directories}, not vc alone")
    if set(files) != wanted:
        failures.append(f"C: has {sorted(files)}, where {sorted(wanted)} "
                        "are wanted")
    for name, source in SHIPPED.items():
        if source is not None and files.get(name) != source.read_bytes():
            failures.append(f"{name} is not {source.relative_to(ROOT)}")
    if program and files.get("vc/vc.exe") != program.read_bytes():
        failures.append(f"vc/vc.exe is not {program}")
    failures.extend(licences_held(files.get("vc/license.txt", b"")))
    helped = site.parent.parent / "VC.HLP"
    if files.get("vc/vc.hlp") != helped.read_bytes():
        failures.append(f"vc/vc.hlp is not {helped}")
    ini = files.get("vc/vc.ini", b"")
    if len(ini) != SETUP_SUM + 2 or \
            sum(ini[:SETUP_SUM]) & 0xFFFF != \
            int.from_bytes(ini[SETUP_SUM:], "little"):
        failures.append("vc/vc.ini is no setup file with its sum")
    elif ini[SETUP_CLOCK] != 1:
        failures.append("vc/vc.ini has the clock off")
    if any(Path(name).stem.lower() == "readme" for name in files):
        failures.append("C: has a README")
    import setups
    shareware = {name: data for name, (data, *_)
                 in setups.distribution().items()}
    help_lines = {line.strip() for line in
                  shareware["VC.HLP"].split(b"\n") if len(line.strip()) > 24}
    for name, data in files.items():
        for theirs, bytes_ in shareware.items():
            if data == bytes_:
                failures.append(f"{name} is the shareware's {theirs}")
        if any(line in data for line in help_lines):
            failures.append(f"{name} holds a line of the shareware's help")
    for failure in failures:
        print(f"FAIL the site that the build made: {failure}")
    return len(failures), 8


def main():
    told = argparse.ArgumentParser()
    told.add_argument("--site", type=Path)
    told.add_argument("--program", type=Path)
    told = told.parse_args()
    node = shutil.which("node")
    if node is None:
        print("no Node: skipped")
        return SKIPPED
    with dos.workdir() as work:
        code = subprocess.run([node, str(HERE / "website.mjs"), str(WEB),
                               work]).returncode
    if told.site is not None:
        failed, checks = built_site(told.site, told.program)
        print(f"the site that the build made: {checks - min(failed, checks)}"
              f" of {checks} checks passed")
        if failed:
            code = 1
    return code


if __name__ == "__main__":
    sys.exit(main())
