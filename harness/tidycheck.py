#!/usr/bin/env python3
"""Hold the program for Linux to clang's warnings and to clang-tidy.

From step 5 of the portable C's road on, every build with clang has a
high level of warnings and clang-tidy, a warning of either an error, on
the user's word of 14 October 2026 (CLAUDE.md, "Strictness"). The builds
stop for a warning already. This test says so where the build is not
looked at, and runs clang-tidy, which the build does not:

  1. Every file of the program for Linux, the common code and the files
     of Linux, is compiled again by the command that the build compiles
     it with, which has the full list of warnings and -Werror, to see
     nothing more: -fsyntax-only. Anything that clang says fails the
     test.
  2. clang-tidy reads the same files with the same options and
     `.clang-tidy` at the top of the checkout: the broad set that the
     user decided, each check that is turned off with its reason there.
     Every place that it names fails the test, but those of EXPECTED.

The common code, `sys_posix.c`, the program's binding of sys.h, and
`machine_posix.c`, the part of it that the web page shares, are read
with the command of the program's build, `compiled.txt` of
`build/portable_c/posix`, which `--compiled` names: its sanitized builds
differ in what they make and not in what clang is to say. The files of
the emulation, the other `*_posix.c`, are read with their test,
`harness/posixcheck.c`, by the command of the build of that test,
`compiled.txt` of `build/portable_c/posix-unit` (CMakeLists.txt, the
piece L1 of step 6), which `--posix-compiled` names; and with them the
table of the keys of iOS, `keys_ios.c`, which is built with that command
(the piece I3 of step 8).

The test was `portable_c-stub`, `harness/stubcheck.py`, which read the
common code with the stub of step 5, `sys_stub.c`, until the stub went
with the program for Linux done, on the user's word of 14 October 2026
(DIARY.md, "Take the stub out, and hold the program for Linux to
clang-tidy in its place (L7)").

EXPECTED is the analyser's places where the program reads its main data
as one block across its fields, as the original does: a read in front of
a field or behind it, `clang-analyzer-security.ArrayBound`, and a
difference of two pointers into two fields,
`clang-analyzer-security.PointerSub`. The piece MD mended them, with
the main data read as bytes at an offset (CLAUDE.md, "Numbers";
DIARY.md, "Read the main data as bytes where the program reads it across
fields"), and the list is empty. It stays for a place of that kind that
is not mended at once, by its file, its check, what clang-tidy says and
how many times, so that one more of them fails the test as any other
place does, and one fewer does too. Nothing is turned off for them.

clang-tidy is pinned to its version 23, which the survey of the checks
was made with (DIARY.md, "Hold the stub to clang-tidy: .clang-tidy and
the test portable_c-stub"): another version has other checks under
`bugprone-*` and `misc-*`, which are judged before they are taken, and
the test fails for it.

The test exits with 77, which the build takes for a test that is
skipped, where there is no clang: the build makes no program for Linux
then, and writes no `compiled.txt`. It runs no machine and takes some two minutes
on four processors.
"""

import argparse
import collections
import concurrent.futures
import os
import pathlib
import re
import shutil
import subprocess
import sys

SKIPPED = 77
VERSION = 23

ROOT = pathlib.Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c"
CONFIG = ROOT / ".clang-tidy"

# What the analyser names where the program reads the main data across
# its fields: (file, check, what clang-tidy says) -> how many places.
# The piece MD read the main data as bytes at an offset and took them
# out (DIARY.md, "Read the main data as bytes where the program reads it
# across fields"): none is left.
BOUND = "clang-analyzer-security.ArrayBound"
BEFORE = "Out of bound access to memory preceding "
BEHIND = "Out of bound access to memory after the end of "
SUB = "clang-analyzer-security.PointerSub"
APART = ("Subtraction of two pointers that do not point into the same "
         "array is undefined behavior")
EXPECTED = {}

# A place that clang or clang-tidy names: file:line:column: kind: text,
# with the checks behind it in brackets for clang-tidy.
PLACE = re.compile(r"^(?P<path>[^:\n]+):(?P<line>\d+):(?P<column>\d+): "
                   r"(?P<kind>warning|error): (?P<text>.*?)"
                   r"(?: \[(?P<checks>[^\]]+)\])?$")


# The files of Linux that are the program's and not the emulation's: they
# read what the common code has, and are read with it.
PROGRAM_POSIX = ("sys_posix.c", "machine_posix.c")


def files():
    """The files that are read with the program's command: the common
    code, every C file of the tree but those of an implementation, and
    sys_posix.c and machine_posix.c, as add_portable_c_host of
    CMakeLists.txt takes them for posix/vc."""
    return sorted(path for path in SOURCE.glob("*.c")
                  if not re.search(r"_(dos|posix|web|ios)\.c$", path.name)
                  or path.name in PROGRAM_POSIX)


def posix_files():
    """The files of the emulation and their test: those that the build of
    portable_c-posix-unit in CMakeLists.txt takes, which leaves out
    sys_posix.c for its main and machine_posix.c for the main data; and
    keys_ios.c, which the build of portable_c-ios-keys compiles with the
    same command."""
    return sorted(path for path in SOURCE.glob("*_posix.c")
                  if path.name not in PROGRAM_POSIX) + [
                      SOURCE / "keys_ios.c",
                      ROOT / "harness" / "posixcheck.c"]


def compiled(path):
    """The command that the build compiles a set of files with: the
    compiler, then its options."""
    words = path.read_text().strip().split(";")
    return words[0], words[1:]


def version(clang_tidy):
    """The major version of clang-tidy, or None."""
    said = subprocess.run([clang_tidy, "--version"], capture_output=True,
                          text=True).stdout
    found = re.search(r"version (\d+)\.", said)
    return int(found[1]) if found else None


def compile_again(compiler, options, path):
    result = subprocess.run([compiler, *options, "-fsyntax-only", str(path)],
                            capture_output=True, text=True)
    return path, result.returncode, result.stdout + result.stderr


def tidy(clang_tidy, options, path):
    result = subprocess.run(
        [clang_tidy, "--quiet", f"--config-file={CONFIG}", str(path), "--",
         *options], capture_output=True, text=True)
    return path, result.returncode, result.stdout + result.stderr


def places(said):
    """The places in what clang-tidy said: (file, line, column, check,
    text). The checks of a place are its own and
    -warnings-as-errors."""
    for line in said.splitlines():
        match = PLACE.match(line)
        if not match:
            continue
        checks = [check for check in (match["checks"] or "").split(",")
                  if check and not check.startswith("-")]
        yield (pathlib.Path(match["path"]).name, int(match["line"]),
               int(match["column"]), checks[0] if checks else
               "clang-diagnostic-error", match["text"])


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--compiled", type=pathlib.Path, required=True,
                        help="compiled.txt of the build of the program for "
                             "Linux, posix/vc")
    parser.add_argument("--posix-compiled", type=pathlib.Path,
                        help="compiled.txt of the build of the test of the "
                             "files of Linux")
    parser.add_argument("--clang-tidy", default="clang-tidy",
                        help="the clang-tidy to run")
    parser.add_argument("--jobs", type=int,
                        default=min(4, os.cpu_count() or 1))
    args = parser.parse_args()

    if not args.compiled.exists():
        print(f"{args.compiled} is not there: the build had no clang, and "
              "made no program for Linux. Skipped.")
        sys.exit(SKIPPED)
    compiler, options = compiled(args.compiled)
    clang_tidy = shutil.which(args.clang_tidy)
    if not shutil.which(compiler) or not clang_tidy:
        print("No clang or no clang-tidy on this computer. Skipped.")
        sys.exit(SKIPPED)
    found = version(clang_tidy)
    if found != VERSION:
        print(f"{clang_tidy} is of version {found}, and the checks of "
              f".clang-tidy were judged with version {VERSION}: another "
              "version has other checks, which are judged before they are "
              "taken. See harness/tidycheck.py.")
        sys.exit(1)

    passed = True
    # Each file with the options of the build that takes it.
    sources = [(path, options) for path in files()]
    if args.posix_compiled is not None:
        if not args.posix_compiled.exists():
            print(f"{args.posix_compiled} is not there, where the "
                  "program's build is: the build of the test of the files "
                  "of Linux is missing.")
            sys.exit(1)
        posix_compiler, posix_options = compiled(args.posix_compiled)
        if posix_compiler != compiler:
            print(f"{args.posix_compiled} has another compiler than the "
                  f"program's, {posix_compiler}.")
            sys.exit(1)
        sources += [(path, posix_options) for path in posix_files()]
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for path, code, said in pool.map(
                lambda source: compile_again(compiler, source[1], source[0]),
                sources):
            if code != 0 or said.strip():
                passed = False
                print(f"clang on {path.name}, with the build's options:")
                print(said.rstrip())
        print(f"{len(sources)} files compiled with the build's warnings.")

        named = set()
        for path, code, said in pool.map(
                lambda source: tidy(clang_tidy, source[1], source[0]),
                sources):
            listed = list(places(said))
            named.update(listed)
            if code != 0 and not listed:
                passed = False
                print(f"clang-tidy on {path.name} failed:")
                print(said.rstrip())

    # A place of a header is named once for every file that includes it.
    counted = collections.Counter((file, check, text)
                                  for file, _, _, check, text in named)
    unexpected = sorted(place for place in named
                        if (place[0], place[3], place[4]) not in EXPECTED)
    for file, line, column, check, text in unexpected:
        passed = False
        print(f"{file}:{line}:{column}: {text} [{check}]")
    for key, count in sorted(EXPECTED.items()):
        if counted[key] != count:
            passed = False
            file, check, text = key
            print(f"{file}: {text} [{check}]: listed {count} times in "
                  f"EXPECTED of harness/tidycheck.py, and named "
                  f"{counted[key]} times")
    expected = sum(EXPECTED.values())
    print(f"clang-tidy named {len(unexpected)} places that are not "
          f"expected, and {sum(counted[key] for key in EXPECTED)} of the "
          f"{expected} expected where the main data is read across its "
          "fields.")
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
