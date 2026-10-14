"""Lines that a merge's conflict leaves, found in the repository.

    python3 harness/conflicts.py        list them, and fail if any

A rebase that stops in a file writes the two sides into it between
lines of its own, `<<<<<<<`, `|||||||`, `=======` and `>>>>>>>`. Where
they are left behind in a text that nothing compiles, a document or a
record, no build stops for them: `DEVIATIONS.md` and `TODO.md` had such
lines three times after a landing. The test `portable_c-words` asks
this of every file that git tracks, but for the assembly as its author
published it, `src/asm`, which is frozen, and the goldens, which are
derived; a file with a byte 0 in it is no text and is passed over.
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# A marker opens a line and is seven characters long; the first three
# kinds may have the name of a side behind them, the fourth stands
# alone.
MARKER = re.compile(rb"^(?:(?:<{7}|\|{7}|>{7})(?: .*)?|={7})\r?$")
LEFT_OUT = ("src/asm/", "harness/goldens/")


def tracked(root=ROOT):
    """The files that git tracks, by their paths from the root."""
    listed = subprocess.run(["git", "ls-files", "-z"], cwd=root,
                            capture_output=True, check=True).stdout
    return [name.decode() for name in listed.split(b"\0") if name]


def stray(root=ROOT, names=None):
    """Every line of a marker, as (path, number of the line, line)."""
    found = []
    for name in tracked(root) if names is None else names:
        if name.startswith(LEFT_OUT):
            continue
        path = Path(root) / name
        if not path.is_file():
            continue
        data = path.read_bytes()
        if b"\0" in data:
            continue
        for number, line in enumerate(data.split(b"\n"), 1):
            if MARKER.match(line):
                found.append((name, number,
                              line.rstrip(b"\r").decode("latin-1")))
    return found


def report(found):
    """What a test prints of them, or None where there are none."""
    if not found:
        return None
    return ("Lines that a merge's conflict left behind:\n"
            + "\n".join(f"  {name}:{number}: {line}"
                        for name, number, line in found)
            + "\nSettle the conflict in each place, keeping what both "
              "sides added, and take the lines out.")


def main():
    said = report(stray())
    if said:
        print(said)
        sys.exit(1)
    print("No line of a merge's conflict is left in the repository.")


if __name__ == "__main__":
    main()
