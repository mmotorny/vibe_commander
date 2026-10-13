#!/usr/bin/env python3
"""Say whether the scenarios notice a change to a C tree.

Every instruction of a routine that has been translated ran in the
original, in some scenario (see ported.py). That does not mean that a
screen shows what the instruction is for: a branch can run and leave
nothing to see. This makes one change to a copy of a C tree, builds it,
and takes it through scenarios until one differs from its golden:

    notice.py src/portable_c/string.c "c == ':' || " "" --routine CutPath

says which scenario notices that CutPath no longer stops at a colon, or
that none does. Then the scenarios cannot tell a right translation of that
branch from a wrong one, and one of them should be made to.

The change is one piece of text in FILE, a file of src/portable_c/, the
portable C, or of src/c/, the C translation, replaced by another: the
tree that the file is of is the one that is built and run. The text has
to be there exactly once.

With --routine only those scenarios are run in which the original ran that
procedure of the assembly; the others cannot notice. Without, all are.

This is for a branch whose effect cannot be pointed to in a golden. It is
not for the registers that a stub hands back: the scenarios seldom reach a
caller that uses them, and what a stub owes its callers is settled by
reading them.

The copy is built by the project's CMakeLists.txt, in build/notice, with
the compiler that build/ has fetched; build/ has to exist. Exits with
status 0 if a scenario notices, 1 if none does, and 2 if the changed
translation does not build.
"""
import argparse
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

import compare
import coverage
import deps
import goldens
import scenarios
import setups
from dos import MachineError

BUILD = deps.ROOT / "build"
LISTING = BUILD / "shareware-layout" / "VC.LST"
COPY = BUILD / "notice" / "source"
COPY_BUILD = BUILD / "notice" / "build"
# The C trees, by the names of their directories in src/ and in build/.
TREES = ("c", "portable_c")
# What they are built from, of all that the project holds: the build
# makes a tool of each file of tools/ that it names, whichever tree is
# asked for.
BUILT_FROM = ["CMakeLists.txt", "tools/mkexe.c", "tools/stub.c"] + [
    f"src/{tree}" for tree in TREES]
COM_ORIGIN = 0x100                # where a .COM file is loaded


def copy_translation():
    """Make COPY hold what the project holds of BUILT_FROM. A file that is
    the same already is left alone, so that it is not built again."""
    wanted = {}
    for name in BUILT_FROM:
        path = deps.ROOT / name
        files = [path] if path.is_file() else \
            [file for file in path.rglob("*") if file.is_file()]
        for file in files:
            wanted[COPY / file.relative_to(deps.ROOT)] = file.read_bytes()
    if COPY.exists():
        for file in COPY.rglob("*"):
            if file.is_file() and file not in wanted:
                file.unlink()
    for file, contents in wanted.items():
        if not file.exists() or file.read_bytes() != contents:
            file.parent.mkdir(parents=True, exist_ok=True)
            file.write_bytes(contents)


def build(tree):
    """Build the tree `tree` of COPY; return its program, or None with
    what the build said printed."""
    fetched = BUILD / "_deps"
    steps = [
        ["cmake", "-S", COPY, "-B", COPY_BUILD,
         f"-DFETCHCONTENT_SOURCE_DIR_JWASM={fetched / 'jwasm-src'}",
         f"-DFETCHCONTENT_SOURCE_DIR_OPEN_WATCOM="
         f"{fetched / 'open_watcom-src'}"],
        ["cmake", "--build", COPY_BUILD, "--target", tree],
    ]
    for step in steps:
        done = subprocess.run([str(word) for word in step],
                              capture_output=True, text=True)
        if done.returncode:
            print(done.stdout + done.stderr)
            return None
    return setups.Program.load(COPY_BUILD / tree / "VC.EXE")


def scenarios_running(routines, tree):
    """The scenarios of the tree `tree` in which the original ran any of
    `routines`."""
    listing = coverage.Listing(LISTING)
    for name in routines:
        if name not in listing.procedures:
            sys.exit(f"no procedure {name} in {LISTING}")
    addresses = {line.address - COM_ORIGIN for name in routines
                 for line in listing.procedure(name)}
    # The routines are of VC.COM: what a scenario of the setup program
    # recorded as having run is of VCSETUP.COM, at other offsets.
    main = set(scenarios.names(tree, setup=False))
    return [name for name in with_goldens(tree)
            if name in main and addresses & goldens.read_ran(name)]


def with_goldens(tree):
    """The scenarios that the tree `tree` is taken through and that have
    a golden to be held against; the others of them are named."""
    names = scenarios.names(tree)
    for name in names:
        if not goldens.derived(name):
            print(f"{name} has no golden yet and is left out",
                  file=sys.stderr)
    return [name for name in names if goldens.derived(name)]


def noticed(program, names):
    """Take `program` through the scenarios `names` and stop at the first
    of them that differs from its golden. Returns its name and how it
    differs, or None.

    The first is run by itself: a break that shows everywhere is then
    found at once. The others run side by side, as compare.py has them."""
    def one(name):
        scenario = scenarios.of(name, program.tree)
        try:
            run = scenarios.run(scenario, program,
                                benches[scenario.country])
        except MachineError as error:
            return str(error).splitlines()
        return goldens.differences(goldens.read(name, program.tree), run,
                                   {program.filename})

    with setups.Benches() as benches, \
            ThreadPoolExecutor(compare.TOGETHER) as pool:
        report = one(names[0])
        if report:
            return names[0], report
        waiting = [pool.submit(one, name) for name in names[1:]]
        for name, result in zip(names[1:], waiting):
            report = result.result()
            if report:
                for other in waiting:
                    other.cancel()      # those that have not begun
                return name, report
    return None


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("file", help="the file of src/portable_c/ or of "
                                     "src/c/ to change, from the project's "
                                     "directory")
    parser.add_argument("old", help="the text to replace")
    parser.add_argument("new", help="what to put in its place")
    parser.add_argument("--routine", action="append", default=[],
                        metavar="NAME",
                        help="run only the scenarios in which the original "
                             "ran this procedure; may be given again")
    args = parser.parse_args()

    if not (BUILD / "_deps").is_dir():
        sys.exit(f"{BUILD} has to be built first")
    target = COPY / args.file
    parts = target.relative_to(COPY).parts
    if len(parts) != 3 or parts[0] != "src" or parts[1] not in TREES:
        sys.exit(f"{args.file} is not a file of src/portable_c/ or of "
                 "src/c/")
    names = scenarios_running(args.routine, parts[1]) if args.routine \
        else with_goldens(parts[1])
    if not names:
        sys.exit("no scenario ran " + ", ".join(args.routine)
                 + " in the original")

    copy_translation()
    if not target.is_file():
        sys.exit(f"no file {args.file}")
    # The assembly is CP866 text; the C is ASCII.
    old, new = args.old.encode("cp866"), args.new.encode("cp866")
    contents = target.read_bytes()
    if contents.count(old) != 1:
        sys.exit(f"the text to replace is in {args.file} "
                 f"{contents.count(old)} times; it has to be there once")
    target.write_bytes(contents.replace(old, new))

    program = build(parts[1])
    if not program:
        print("The changed translation does not build.")
        sys.exit(2)
    found = noticed(program, names)
    if not found:
        print(f"Not noticed by any of {len(names)} scenario"
              f"{'s' if len(names) > 1 else ''}: {', '.join(names)}")
        sys.exit(1)
    name, report = found
    print(f"Noticed by {name}:")
    print("\n".join(report[:8]))
    if len(report) > 8:
        print(f"... and {len(report) - 8} more lines")


if __name__ == "__main__":
    main()
