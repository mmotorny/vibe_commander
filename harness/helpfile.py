#!/usr/bin/env python3
"""Hold the project's own help file to what F1 shows in the program for
Linux: the test portable_c-help-file.

    python3 harness/helpfile.py PROGRAM HELPFILE [--show]

HELPFILE is the VC.HLP that tools/makehelp.py makes of
src/portable_c/help/vc-help.txt, with text of the project's own (the
shareware's help is never used). The program, the sanitized build for
Linux, is started through its test channel (posixrun.py) in a
directory of its own in /tmp, with HOME there and no setup file, so
that it takes its defaults, and the help file as vc.hlp in the
directory of its setup file, where it reads its help. Then, step by
step:

  - F1 at the panels shows the index, with the title of every page of
    the source as a line, the first under the cursor;
  - Enter there shows the main page, "Vibe Commander", with its first
    line of text;
  - Enter on Next shows the second page, "The panels";
  - PgDn moves the text of that page;
  - Index goes back to the index, Esc leaves the help;
  - F7 and F1 show the page of Make directory, the dialog's own;
  - Shift-F4, a name and Enter open the editor, and F1 there shows the
    editor's page;
  - More setup of the Options menu, and F1 at the setup's menu, shows
    the page of Configuration, the first of the setup's, which the
    setup reads by itself.

Each screen is held to the titles and lines that the source has for
it, read from the source, so that a change of the text needs no change
here. With --show the screens are printed as text.

It runs no machine, and exits with 77, which the build takes for a
test that is skipped, where the program has not been built (no clang).
"""
import os
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE.parent / "tools"))

import makehelp     # noqa: E402
import posixrun     # noqa: E402

STEPS = (
    "F1",                       # the index
    "Enter",                    # page 1, the main page
    "Enter",                    # Next: page 2
    "PgDn",                     # its text moved by a page
    "Right Right Enter",        # Index
    "Esc",                      # out of the help
    "F7",                       # Make directory
    "F1",                       # its page
    "Esc Esc",
    'Shift+F4 "help.txt" Enter Enter',  # the editor, a new file
    "F1",                       # its page
    "Esc Esc",                  # out of the help and the editor
    "F9 o",                     # the Options menu
    "End Enter",                # More setup: no setup file, says the box
    "Enter",                    # Ok: the setup's menu
    "F1",                       # the page of Configuration, the 55th
)


def run(program, helpfile):
    """The screens of STEPS, the first that of the start."""
    bench = posixrun._bench_directory()
    try:
        home, config = bench / "home", bench / "vc"
        home.mkdir()
        config.mkdir()
        (config / "vc.hlp").write_bytes(helpfile.read_bytes())
        channel = bench / "channel"
        channel.mkdir()
        (channel / "keys").write_text(posixrun.keys(STEPS))
        environment = {
            "PATH": os.environ.get("PATH", "/usr/bin:/bin"),
            "TZ": "UTC", "HOME": str(home), "XDG_CONFIG_HOME": str(bench),
            "VC_TEST": str(channel), "VC_TEST_CLOCK": posixrun.CLOCK,
        }
        done = subprocess.run([str(program)], cwd=home, env=environment,
                              stdin=subprocess.DEVNULL, capture_output=True,
                              timeout=60)
        said = done.stderr.decode(errors="replace").strip()
        if done.returncode not in (0, 2) or (
                done.returncode == 2 and "keys have run out" not in said):
            raise SystemExit(f"helpfile: the program ended with "
                             f"{done.returncode}: {said}")
        records = posixrun._screens((channel / "screens").read_bytes())
    finally:
        posixrun._remove(bench)
    return [screen.lines() for kind, _, screen in records if kind == "step"]


def main(argv):
    show = "--show" in argv
    argv = [arg for arg in argv if arg != "--show"]
    program, helpfile = Path(argv[1]).resolve(), Path(argv[2])
    if not program.exists():
        print(f"helpfile: {program} has not been built", file=sys.stderr)
        return 77
    pages, problems = makehelp.read(makehelp.SOURCE)
    if problems:
        raise SystemExit("\n".join(problems))
    title = {page: text.decode("cp437") for page, (text, _) in pages.items()}

    def line(page, number):
        """Line `number` of a page's text as it shows, its markup gone."""
        text = pages[page][1][number].decode("cp437")
        for mark in ("^B", "^U", "^R", "^N"):
            text = text.replace(mark, "")
        return text.strip()

    screens = run(program, helpfile)
    if show:
        for number, screen in enumerate(screens):
            print(f"-- after step {number}: {(('',) + STEPS)[number]}")
            print("\n".join(screen))
    if len(screens) != len(STEPS) + 1:
        print(f"helpfile: {len(screens)} screens for {len(STEPS) + 1} "
              f"steps", file=sys.stderr)
        return 1
    # What each step is to show: texts that are to be on the screen.
    expected = {
        1: [title[1], title[2], title[13], " Help "],
        2: [title[1], line(1, 0), "Next", "Previous", "Index"],
        3: [title[2], line(2, 0)],
        4: [title[2]],
        5: [title[1], title[2]],
        8: [title[22], line(22, 0)],
        11: [title[5], line(5, 0)],
        16: [title[55], line(55, 0), "[ Ok ]"],
    }
    failed = []
    for step, texts in expected.items():
        text = "\n".join(screens[step])
        for wanted in texts:
            if wanted not in text:
                failed.append(f"after {(('',) + STEPS)[step]!r} (step "
                              f"{step}): {wanted!r} is not on the screen")
    # PgDn moved the text: the first line of page 2 is gone from where
    # it was, the title stays.
    if line(2, 0) in "\n".join(screens[4]):
        failed.append("PgDn on page 2 did not move its text")
    # Esc left the help: no title of it on the screen.
    if " Help " in "\n".join(screens[6]):
        failed.append("Esc did not leave the help")
    for problem in failed:
        print(f"helpfile: {problem}", file=sys.stderr)
    if not failed:
        print(f"helpfile: {len(expected) + 2} checks of {len(screens)} "
              f"screens passed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
