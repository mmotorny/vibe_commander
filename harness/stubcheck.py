#!/usr/bin/env python3
"""Hold the portable C to clang's warnings and to clang-tidy.

Step 5 of the portable C's road is a stub, `sys_stub.c` behind `sys.h`,
so that the common code is built for this computer by clang; and from
then on every build with clang has a high level of warnings and
clang-tidy, a warning of either an error, on the user's word of 14
October 2026 (CLAUDE.md, "The portable C", the road, step 6). The build
of the stub stops for a warning already. This test says so where the
build is not looked at, and runs clang-tidy, which the build does not:

  1. Every file of the stub, the common code and `sys_stub.c`, is
     compiled again by the command that the build compiles it with,
     `compiled.txt` of the stub's directory, which has the full list of
     warnings and -Werror, to see nothing more: -fsyntax-only. Anything
     that clang says fails the test.
  2. clang-tidy reads the same files with the same options and
     `.clang-tidy` at the top of the checkout: the broad set that the
     user decided, each check that is turned off with its reason there.
     Every place that it names fails the test, but those of EXPECTED.

The files of Linux, `*_posix.c`, are held so too, and with them their
test, `harness/posixcheck.c`, by the command of the build of that test,
`compiled.txt` of `build/portable_c/posix-unit` (CMakeLists.txt, the
piece L1 of step 6), which `--posix-compiled` names: the common code is
not among them until the program for Linux is built of it.

EXPECTED is the analyser's places where the program reads its main data
as one block across its fields, as the original does: a read in front of
a field or behind it, `clang-analyzer-security.ArrayBound`, and a
difference of two pointers into two fields,
`clang-analyzer-security.PointerSub`. The piece MD mended them, with
the main data read as bytes at an offset (CLAUDE.md, the same item;
TODO.md, "MD, the main data read as bytes"), and the list is empty. It
stays for a place of that kind that is not mended at once, by its file,
its check, what clang-tidy says and how many times, so that one more of
them fails the test as any other place does, and one fewer does too.
Nothing is turned off for them.

clang-tidy is pinned to its version 23, which the survey of the checks
was made with (TODO.md, "5b, the stub"): another version has other
checks under `bugprone-*` and `misc-*`, which are judged before they are
taken, and the test fails for it.

The test exits with 77, which the build takes for a test that is
skipped, where there is no clang: the build makes no stub then, and
writes no `compiled.txt`. It runs no machine and takes some two minutes
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
# out (TODO.md, "MD, the main data read as bytes"): none is left.
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


def files():
    """The files of the stub: every C file of the tree but those of the
    other implementations, as add_portable_c_host of CMakeLists.txt
    takes them."""
    return sorted(path for path in SOURCE.glob("*.c")
                  if not re.search(r"_(dos|posix)\.c$", path.name))


def posix_files():
    """The files of Linux and their test, as the build of
    portable_c-posix-unit in CMakeLists.txt takes them."""
    return sorted(SOURCE.glob("*_posix.c")) + [ROOT / "harness" /
                                                "posixcheck.c"]


def compiled(path):
    """The command that the build compiles the stub's files with: the
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
                        help="compiled.txt of the stub's build")
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
              "made no stub. Skipped.")
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
              "taken. See harness/stubcheck.py.")
        sys.exit(1)

    passed = True
    # Each file with the options of the build that takes it.
    sources = [(path, options) for path in files()]
    if args.posix_compiled is not None:
        if not args.posix_compiled.exists():
            print(f"{args.posix_compiled} is not there, where the stub's "
                  "build is: the build of the test of the files of Linux "
                  "is missing.")
            sys.exit(1)
        posix_compiler, posix_options = compiled(args.posix_compiled)
        if posix_compiler != compiler:
            print(f"{args.posix_compiled} has another compiler than the "
                  f"stub's, {posix_compiler}.")
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
                  f"EXPECTED of harness/stubcheck.py, and named "
                  f"{counted[key]} times")
    expected = sum(EXPECTED.values())
    print(f"clang-tidy named {len(unexpected)} places that are not "
          f"expected, and {sum(counted[key] for key in EXPECTED)} of the "
          f"{expected} expected where the main data is read across its "
          "fields.")
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
