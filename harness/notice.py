#!/usr/bin/env python3
"""Say whether the scenarios notice a change to the port.

Every instruction of a routine that has been translated ran in the
original, in some scenario (see ported.py). That does not mean that a
screen shows what the instruction is for: a branch can run and leave
nothing to see. This makes one change to a copy of the port, builds it,
and takes it through scenarios until one differs from its golden:

    notice.py port/string.c "c == ':' || " "" --routine CutPath

says which scenario notices that CutPath no longer stops at a colon, or
that none does. Then the scenarios cannot tell a right translation of that
branch from a wrong one, and one of them should be made to.

The change is one piece of text in FILE, a file of port/, replaced by
another. The text has to be there exactly once. In the assembly it is
written with its tabs, $'\\tMOV\\tAL,0\\r\\n' in a shell.

With --routine only those scenarios are run in which the original ran that
procedure of the assembly; the others cannot notice. Without, all are.

This is for a branch whose effect cannot be pointed to in a golden. It is
not for the registers that a stub hands back: the scenarios seldom reach a
caller that uses them, and what a stub owes its callers is settled by
reading them.

The copy is built by the project's CMakeLists.txt, in build/notice, with
the assembler's source and the compiler that build/ has fetched; build/
has to exist. The first run compiles the assembler again, which takes a
minute. Exits with status 0 if a scenario notices, 1 if none does, and 2
if the changed port does not build.
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
# What the port is built from, of all that the project holds.
BUILT_FROM = ["CMakeLists.txt", "tools/mkexe.c", "port"]
COM_ORIGIN = 0x100                # where a .COM file is loaded


def copy_port():
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


def build():
    """Build the port of COPY that signals; return it, or None with what
    the build said printed."""
    fetched = BUILD / "_deps"
    steps = [
        ["cmake", "-S", COPY, "-B", COPY_BUILD,
         f"-DFETCHCONTENT_SOURCE_DIR_JWASM={fetched / 'jwasm-src'}",
         f"-DFETCHCONTENT_SOURCE_DIR_OPEN_WATCOM="
         f"{fetched / 'open_watcom-src'}"],
        ["cmake", "--build", COPY_BUILD, "--target", "port-signalling"],
    ]
    for step in steps:
        done = subprocess.run([str(word) for word in step],
                              capture_output=True, text=True)
        if done.returncode:
            print(done.stdout + done.stderr)
            return None
    return setups.Program.load(COPY_BUILD / "port" / "signalling" / "VC.EXE")


def scenarios_running(routines):
    """The scenarios in which the original ran any of `routines`."""
    listing = coverage.Listing(LISTING)
    for name in routines:
        if name not in listing.procedures:
            sys.exit(f"no procedure {name} in {LISTING}")
    addresses = {line.address - COM_ORIGIN for name in routines
                 for line in listing.procedure(name)}
    return [name for name in with_goldens()
            if addresses & goldens.read_ran(name)]


def with_goldens():
    """The scenarios that have a golden to be held against; the others
    are named."""
    for name in scenarios.SCENARIOS:
        if not goldens.derived(name):
            print(f"{name} has no golden yet and is left out",
                  file=sys.stderr)
    return [name for name in scenarios.SCENARIOS if goldens.derived(name)]


def noticed(program, names):
    """Take `program` through the scenarios `names` and stop at the first
    of them that differs from its golden. Returns its name and how it
    differs, or None.

    The first is run by itself: a break that shows everywhere is then
    found at once. The others run side by side, as compare.py has them."""
    def one(name):
        scenario = scenarios.SCENARIOS[name]
        try:
            run = scenarios.run(scenario, program,
                                benches[scenario.country], signals=True)
        except MachineError as error:
            return str(error).splitlines()
        return goldens.differences(goldens.read(name), run,
                                   {program.filename})

    with setups.Benches(alone=False) as benches, \
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
    parser.add_argument("file", help="the file of port/ to change, from "
                                     "the project's directory")
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
    if "port" not in target.relative_to(COPY).parts[:1]:
        sys.exit(f"{args.file} is not a file of port/")
    names = scenarios_running(args.routine) if args.routine \
        else with_goldens()
    if not names:
        sys.exit("no scenario ran " + ", ".join(args.routine)
                 + " in the original")

    copy_port()
    if not target.is_file():
        sys.exit(f"no file {args.file}")
    # The assembly is CP866 text; the C is ASCII.
    old, new = args.old.encode("cp866"), args.new.encode("cp866")
    contents = target.read_bytes()
    if contents.count(old) != 1:
        sys.exit(f"the text to replace is in {args.file} "
                 f"{contents.count(old)} times; it has to be there once")
    target.write_bytes(contents.replace(old, new))

    program = build()
    if not program:
        print("The changed port does not build.")
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
