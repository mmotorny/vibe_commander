#!/usr/bin/env python3
"""Hold a scenario, on the bench that sorts by name, to what it did on the
bench that did not sort.

The bench's panels list files sorted by name since the piece SORT
(TODO.md, "SORT, the bench sorted by name: the plan"), where they had
them as the disk has them. Steps that went to a file by its place, Down
four times onto hello.txt, go to another file on the new bench, and are
rewritten so that each goes where it went. This says whether a scenario
does, from its golden as derived on the new bench and the golden at a
commit of the old one, BEFORE, without running anything:

    sortcheck.py                    every scenario, a line each
    sortcheck.py copy view          those, with what differs
    sortcheck.py --before COMMIT    held against the goldens of COMMIT

For each screen, by its number, it takes of each panel that the screen
shows the directory in the panel's title and the line under the panel's
files, the mini status, which says the file under the cursor, or
that the cursor is on the first entry shown, whichever that is: the
root of C: has STAMPS first now and WORK before, and the cursor of the
panel that is not the current one stays at the top. Of a screen without
a panel, as the viewer's and the editor's are, the first
row, which names the file. A screen that has another of those than the
old golden's is named, with both. The panels' lists themselves are not
held: they are in another order, which is the point. A scenario keeps
its number of steps, the keys of a step being changed, so that its
screens stand against the old ones by number; where the numbers differ
it says so, and holds as many as both have.

And it holds what the scenario ran of the original, its `.ran`, against
what it ran before: an instruction of a batch of ported.py (of the setup
program's batches for a scenario of the setup program) that ran then and
does not now is named. The code that sorts and that keeps a list
unsorted runs otherwise on the new bench, and is named as well; such an
instruction is explained, not mended.

A scenario passes when no screen differs and no instruction is lost. One
that differs where it is right to, and says why, is explained in the
plan, not here.
"""
import argparse
import subprocess
import sys

import coverage
import goldens
import ported
import scenarios
import setups

# The last commit of main with the bench that did not sort: the goldens
# that the scenarios are held to.
BEFORE = "f8bf7584b3da0e3596e06eb3f92a1dede24808ff"

# The instructions that only a list that is not sorted runs, by the
# original's listing: the comparison of CmpPar for the unsorted order. Every
# scenario that showed a panel ran them on the old bench; on the new one
# only a scenario that picks Unsorted does, `sort`, which has to go on
# doing so. They are said apart and do not fail a scenario.
UNSORTED = {0xAB62, 0xAB65}

TOP, SEPARATOR = "╔", "╟"


def screens(text):
    """The screens of a golden: a list of (step, rows)."""
    found = []
    for line in text.splitlines():
        if line.startswith("== "):
            found.append((line.split(": ", 1)[-1], []))
        elif found and line.startswith("|"):
            found[-1][1].append(line[1:81])
    return found


def on_first(row, status):
    """Whether the mini status `status` names the entry that `row`, the
    first row of a panel's files, has in its first column: the cursor
    has not moved from the top, which is another entry where a list
    without .. is sorted otherwise, the root of C: above all."""
    entry = row[1:13]
    name, extension = entry[:8].strip(), entry[9:].strip()
    if not name:
        return False
    shown = f"{name}.{extension}" if extension else name
    words = status.split()
    return bool(words) and words[0].lower() == shown.lower()


def seen(rows):
    """What a screen says of where the cursor is: for each half that has
    a panel, its title and its mini status; without a panel, the first
    row."""
    said = []
    for half, (left, right) in (("left", (0, 40)), ("right", (40, 80))):
        cut = [row[left:right] for row in rows]
        tops = [i for i, row in enumerate(cut) if row.startswith(TOP)]
        lines = [i for i, row in enumerate(cut) if row.startswith(SEPARATOR)]
        if tops and lines and lines[0] + 1 < len(cut):
            status = cut[lines[0] + 1].strip("║ ")
            if tops[0] + 2 < lines[0] and on_first(cut[tops[0] + 2], status):
                status = "the first entry shown"
            said.append((f"{half} panel", cut[tops[0]].strip("╔═╗╤ ")
                         or "-", status))
    if not said and rows:
        said.append(("first row", "", rows[0].rstrip()))
    return said


def offsets(text):
    found = set()
    for run in text.split():
        first, last = run.split("-")
        found.update(range(int(first, 16), int(last, 16) + 1))
    return found


def old(path, before):
    return subprocess.run(["git", "show", f"{before}:{path}"],
                          capture_output=True, text=True,
                          cwd=goldens.DIRECTORY.parent.parent).stdout


def check(name, before, listings, verbose):
    """Print what differs; return whether nothing does."""
    scenario = scenarios.SCENARIOS[name]
    golden = goldens.DIRECTORY / f"{name}.txt"
    if not golden.exists():
        print(f"{name}: not derived, goldens.py derive {name}")
        return False
    relative = golden.relative_to(goldens.DIRECTORY.parent.parent)
    then, now = screens(old(relative, before)), screens(golden.read_text())
    out = []
    if len(then) != len(now):
        out.append(f"  {len(now)} screens where there were {len(then)}: "
                   f"held by number as far as both go")
    differ = 0
    for number, ((step, rows), (old_step, old_rows)) in enumerate(
            zip(now, then)):
        a, b = seen(old_rows), seen(rows)
        if a != b:
            differ += 1
            out.append(f"  screen {number}: {step}"
                       + (f" (was {old_step})" if step != old_step else ""))
            for where, title, line in a:
                out.append(f"    then {where} {title}: {line}")
            for where, title, line in b:
                out.append(f"    now  {where} {title}: {line}")
    listing, batches = listings[scenario.setup]
    ran = goldens.DIRECTORY / f"{name}.ran"
    lost_bytes = (offsets(old(ran.relative_to(
        goldens.DIRECTORY.parent.parent), before))
        - offsets(ran.read_text()))
    lost = []
    for batch in batches:
        for routine in batch.routines:
            if routine not in listing.procedures:
                continue
            for line in listing.procedure(routine):
                if line.address - coverage.COM_ORIGIN in lost_bytes:
                    lost.append((line.address, routine, line.text.strip()))
    lost = sorted(set(lost))
    for address, routine, text in lost:
        out.append(f"  lost: {address:04X} {routine}: {text}"
                   + (" (the unsorted order's)" if address in UNSORTED
                      else ""))
    lost = [line for line in lost if line[0] not in UNSORTED]
    shown = min(len(then), len(now))
    print(f"{name}: {shown - differ} of {shown} screens as before, "
          f"{differ} differ; {len(lost)} instructions of the batches "
          f"lost, besides the unsorted order's ({len(lost_bytes)} bytes "
          f"of the file)")
    if verbose:
        print("\n".join(out))
    return not differ and not lost and len(then) == len(now)


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("scenario", nargs="*")
    parser.add_argument("--before", default=BEFORE,
                        help="the commit whose goldens are held to")
    parser.add_argument("--all", action="store_true",
                        help="with no scenario named: every scenario, with "
                        "what differs")
    args = parser.parse_args()
    listings = {
        False: (coverage.Listing(setups.LISTING), ported.BATCHES),
        True: (coverage.Listing(setups.SETUP_LISTING), ported.SETUP_BATCHES),
    }
    names = args.scenario or scenarios.names()
    unknown = [name for name in names if name not in scenarios.SCENARIOS]
    if unknown:
        parser.error(f"no scenario called {', '.join(unknown)}")
    passed = [check(name, args.before, listings,
                    bool(args.scenario) or args.all) for name in names]
    print(f"{sum(passed)} of {len(passed)} as before")
    return 0 if all(passed) else 1


if __name__ == "__main__":
    sys.exit(main())
