#!/usr/bin/env python3
"""Compile what the web page is to be built of, by emcc, and fail for
anything that it says.

The piece W0 of step 7 of the portable C's road (TODO.md, "Step 7, the
web page"). The build for the web is Emscripten's clang for wasm32, a
machine where an int, a long and a pointer have 32 bits each, of another
version than the clang that builds the program for Linux. Before there is
a program for the web, this compiles each file that it is to have, with
the warnings of the program for Linux and -Werror, so that what such a
compiler and such a machine find in the common code is found early:

  - the common code, every file of src/portable_c/ whose name ends in
    no implementation's;
  - the files of Linux's emulation of DOS that the web shares, every
    `*_posix.c` but those that `portable_c_leaves_web` of CMakeLists.txt
    names, `sys_posix.c` and `term_posix.c`, which are the terminal's and
    the shell's: the configuration writes that list into `leaves.txt`
    beside `compiled.txt`, which this reads, so that the two are one;
  - the web's own, `*_web.c`.

The command is the build's, `compiled.txt` of `build/portable_c/
web-probe`, which `--compiled` names (CMakeLists.txt, "Emscripten"): the
compiler, then its options, as a list of CMake's. Each object is written
into `--out`. Nothing is linked and no machine is started.
"""

import argparse
import concurrent.futures
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SOURCE = ROOT / "src" / "portable_c"

# The implementations of sys.h, by the end of a file's name.
IMPLEMENTATIONS = ("dos", "posix", "web", "ios")

def not_shared(compiled_path):
    """The files of Linux that the web has not: `portable_c_leaves_web` of
    CMakeLists.txt, which the configuration writes into leaves.txt beside
    `compiled_path`, a list of CMake's."""
    leaves = compiled_path.with_name("leaves.txt").read_text().strip()
    return tuple(name for name in leaves.split(";") if name)


def files(not_shared_files):
    """What the build for the web is to compile, in the order of their
    names: the files of Linux that `not_shared_files` names left out."""
    chosen = []
    for path in sorted(SOURCE.glob("*.c")):
        stem = path.stem
        kind = next((name for name in IMPLEMENTATIONS
                     if stem.endswith("_" + name)), None)
        if kind is None or kind == "web" or \
                (kind == "posix" and path.name not in not_shared_files):
            chosen.append(path)
    return chosen


def compiled(path):
    """The compiler and its options, from a compiled.txt."""
    words = path.read_text().strip().split(";")
    return words[0], words[1:]


# What emcc says of itself at its first use, while it fills its cache:
# "shared:INFO: (Emscripten: Running sanity checks)" and the like.
OWN = re.compile(r"^\w+:INFO: .*\n?", re.MULTILINE)


def compile_one(compiler, options, out, path):
    result = subprocess.run(
        [compiler, *options, "-c", "-o", str(out / (path.stem + ".o")),
         str(path)], capture_output=True, text=True)
    return path, result.returncode, OWN.sub("", result.stdout +
                                            result.stderr)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--compiled", type=pathlib.Path, required=True,
                        help="compiled.txt of build/portable_c/web-probe")
    parser.add_argument("--out", type=pathlib.Path, required=True,
                        help="where the objects are written")
    parser.add_argument("--jobs", type=int, default=4,
                        help="files compiled side by side (4)")
    args = parser.parse_args()
    compiler, options = compiled(args.compiled)
    args.out.mkdir(parents=True, exist_ok=True)
    chosen = files(not_shared(args.compiled))
    failed = []
    with concurrent.futures.ThreadPoolExecutor(args.jobs) as pool:
        for path, code, said in pool.map(
                lambda path: compile_one(compiler, options, args.out, path),
                chosen):
            if code != 0 or said.strip():
                failed.append(path.name)
                print(f"{path.name}:")
                print(said.rstrip())
    if failed:
        print(f"{len(failed)} of {len(chosen)} files fail for the web: "
              f"{', '.join(failed)}")
        return 1
    print(f"All {len(chosen)} files compile for the web, with nothing "
          "said.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
