#!/usr/bin/env python3
"""The scenarios taken through the web page itself, in a real browser.

The test portable_c-web-scenarios takes the program for the web through
the scenarios under Node (posixrun.py, the link web-node), past the page.
This takes the page, build/portable_c/web/vc.html, through them in the
machine's own Chromium, headless, as a user has it: the page is served
by its own server (src/portable_c/web/server.mjs) with the bench's two
drives, the keys of each step are the browser's key events and the mouse
its pointer events, which keys.js and mouse.js make the program's input
as they make a user's, and each screen is read from what the page shows.
harness/webpagerun.mjs runs a scenario; the top of it says how.

The program's test channel (machine_posix.c, "The test channel") keeps
the time as on the bench: it starts at the bench's clock, says when an
event is due and when a step has ended, and writes its own record of
each screen. Through the page (VC_TEST_PAGE, sys_web.c) it asks the page
for each event, and waits until the browser has given it; and it tells
the page each screen that it records, and waits until the page has been
read. The page's one addition for that is its test hook, page.js.

A screen is read twice: from the <pre>, its characters and the classes
of its spans ("dom"), and from its pixels, a screenshot of the grid at
one device pixel to the CSS pixel, each cell's 8 by 16 pixels held to
the glyph of the page's font, rendered alone by the same Chromium, in
the VGA's colours of the cell ("pixels"). Each is held to the goldens
with the declarations and the masks of the tree "web", as the program
under Node is; a cell whose pixels are not what the <pre> has is
counted besides, and its pixels decoded into the cell that they show.

What the page's file system cannot hold: the four files of C:\\STAMPS of
a gigabyte or more (setups.STAMPS_FILES), which are sparse on the host
and would be read whole into the page's memory, are laid out empty on
the server's drive and made as long as the bench has them in the page's
file system once it is loaded, read as zeros (the hook's `standIns`), and
the disks after a run have them as long as the bench does where nothing
has changed them.

    webpagerun.py [--first] [--together N] [--out DIR] [SCENARIO...]

runs the scenarios (all that the tree "web" is taken through, where none
is named, but those that press a key which the browser keeps for itself,
KEPT, unless --kept) and writes a line for each, with the first screen
that differs, into DIR/results.txt, and the failures grouped by where
they first differ, or by why they stopped, into DIR/summary.txt
(build/webpagerun where not said). Exits 0 where every scenario passed
both ways, 1 where not, 77 where there is no Chromium or no page.
"""
import argparse
import re
import copy
import dataclasses
import json
import os
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import deps
import posixrun
from dos import COLUMNS, ROWS, Screen, MachineError

ROOT = deps.ROOT
PAGE = ROOT / "build" / "portable_c" / "web" / "vc.html"
WORK = ROOT / "build" / "webpagerun"
RUNNER = Path(__file__).resolve().parent / "webpagerun.mjs"

# The keys that are no character, as the browser has them: `code`, `key`,
# the virtual key of Windows, the text that they type, and where on the
# keyboard they are (3 the pad).
NAMED = {
    "Esc": ("Escape", "Escape", 27, None, 0),
    "Backspace": ("Backspace", "Backspace", 8, None, 0),
    "Tab": ("Tab", "Tab", 9, None, 0),
    "Enter": ("Enter", "Enter", 13, "\r", 0),
    "Space": ("Space", " ", 32, " ", 0),
    "GrayStar": ("NumpadMultiply", "*", 106, "*", 3),
    "GrayMinus": ("NumpadSubtract", "-", 109, "-", 3),
    "GrayPlus": ("NumpadAdd", "+", 107, "+", 3),
    "Home": ("Home", "Home", 36, None, 0),
    "Up": ("ArrowUp", "ArrowUp", 38, None, 0),
    "PgUp": ("PageUp", "PageUp", 33, None, 0),
    "Left": ("ArrowLeft", "ArrowLeft", 37, None, 0),
    "Right": ("ArrowRight", "ArrowRight", 39, None, 0),
    "End": ("End", "End", 35, None, 0),
    "Down": ("ArrowDown", "ArrowDown", 40, None, 0),
    "PgDn": ("PageDown", "PageDown", 34, None, 0),
    "Ins": ("Insert", "Insert", 45, None, 0),
    "Del": ("Delete", "Delete", 46, None, 0),
}
for _number in range(1, 13):
    NAMED[f"F{_number}"] = (f"F{_number}", f"F{_number}", 111 + _number,
                            None, 0)
PUNCTUATION = {"-": ("Minus", 189), "=": ("Equal", 187),
               "[": ("BracketLeft", 219), "]": ("BracketRight", 221),
               "\\": ("Backslash", 220), ";": ("Semicolon", 186),
               "'": ("Quote", 222), "`": ("Backquote", 192),
               ",": ("Comma", 188), ".": ("Period", 190),
               "/": ("Slash", 191)}
# The modifiers by their bits of the BIOS's (posixrun.HELD), and CDP's.
MODIFIERS = {0x02: ("ShiftLeft", "Shift", 16, 1), 0x01: ("ShiftRight", "Shift",
                                                          16, 2),
             0x04: ("ControlLeft", "Control", 17, 1),
             0x08: ("AltLeft", "Alt", 18, 1)}


def cdp_modifiers(held):
    """CDP's bits of the modifiers for what the BIOS has held."""
    return (1 if held & 8 else 0) | (2 if held & 4 else 0) | \
        (8 if held & 3 else 0)


def key_action(name, held):
    """What the browser is given for the key `name`, as the driver names
    it, with `held` down."""
    mods = cdp_modifiers(held)
    if name in NAMED:
        code, key, virtual, text, location = NAMED[name]
        if held & 0x0C:
            text = None
        return {"key": {"code": code, "key": key, "virtual": virtual,
                        "text": text, "location": location}, "mods": mods}
    char = name.lower()
    if char not in posixrun.CHARACTERS:
        raise ValueError(f"unknown key {name!r}")
    _, shift, _ = posixrun.CHARACTERS[char]
    key = shift if held & 3 else char
    if char.isalpha():
        code, virtual = f"Key{char.upper()}", ord(char.upper())
    elif char.isdigit():
        code, virtual = f"Digit{char}", ord(char)
    else:
        code, virtual = PUNCTUATION[char]
    return {"key": {"code": code, "key": key, "virtual": virtual,
                    "text": None if held & 0x0C else key, "location": 0},
            "mods": mods}


def _keys_of(step):
    """The keys of `step` by their names, with what is held where each is
    pressed, in the order in which posixrun.events() makes their words."""
    names = []
    for name in step.split():
        if "@" in name and not name.startswith('"') or \
                name.startswith("Wait*"):
            continue
        if name.startswith('"'):
            chords = [["Shift", posixrun.BY_SHIFTED[c]]
                      if c in posixrun.BY_SHIFTED else [c]
                      for c in name.strip('"')]
        else:
            chords = [name.split("+")]
        for chord in chords:
            bits = 0
            for key in chord:
                bits |= posixrun.HELD.get(key, 0)
            names += [(key, bits) for key in chord
                      if key not in posixrun.HELD]
    return names


def actions(steps):
    """For each event of the channel's keys for `steps`, as posixrun.keys()
    has them, in order: what the browser is given for it, a key with what
    is held, a modifier going down or up, the mouse to a cell with its
    buttons; or None for what the browser has no part in, a wait and a
    count of asks."""
    out = []
    held = 0
    for step in ("",) + tuple(steps):
        keys = iter(_keys_of(step))
        for word in posixrun.events(step):
            if word.startswith("held="):
                now = int(word[5:], 16)
                changed = now ^ held
                if changed not in MODIFIERS:
                    raise ValueError(f"a change of what is held: {word}")
                code, key, virtual, location = MODIFIERS[changed]
                out.append({"modifier": {"code": code, "key": key,
                                         "virtual": virtual,
                                         "location": location},
                            "down": bool(now & changed),
                            "mods": cdp_modifiers(now)})
                held = now
            elif word.startswith("mouse="):
                column, row, buttons = word[6:].split(",")
                out.append({"mouse": [int(column), int(row),
                                      int(buttons, 16)],
                            "mods": cdp_modifiers(held)})
            elif word.startswith(("wait=", "asks=")):
                out.append(None)
            else:
                name, bits = next(keys)
                if f"{posixrun.word(name, bits):04X}" != word.rstrip("+"):
                    raise ValueError(f"{name} is not {word}")
                out.append(key_action(name, bits))
    return out


# The keys that a browser keeps for itself in an ordinary tab, which
# never reach the page (W4 of src/portable_c/DEVIATIONS.md): Ctrl-PgUp and
# Ctrl-PgDn go to another tab, Ctrl-W, Ctrl-T and Ctrl-N close and open
# one, Ctrl-F4 and Alt-F4 close the tab or the window. A scenario that
# presses one is not taken through the page.
KEPT = re.compile(r"(?:Ctrl\+(?:Shift\+)?[wtnWTN]|Ctrl\+(?:Shift\+)?Tab|"
                  r"Ctrl\+(?:PgUp|PgDn|F4)|Alt\+F4)")


def kept_keys(steps):
    """The keys of `steps` that the browser keeps for itself."""
    return sorted({name for step in steps for name in step.split()
                   if not name.startswith('"') and KEPT.fullmatch(name)})


def tree_of(root):
    """The files and directories under `root`, parents first, as the
    page's server answers a drive's tree, and the root's time."""
    import base64
    entries = []
    for path in sorted(root.rglob("*")):
        status = path.lstat()
        entry = {"path": "/".join(path.relative_to(root).parts),
                 "directory": path.is_dir(),
                 "mtime": int(status.st_mtime * 1000),
                 "readonly": not os.access(path, os.W_OK)}
        if not entry["directory"]:
            entry["data"] = base64.b64encode(path.read_bytes()).decode()
        entries.append(entry)
    return {"root": "/drive/d", "entries": entries,
            "mtime": int(root.stat().st_mtime * 1000)}


def put_tree(root, tree):
    """`root` made what `tree` has, the page's D: at the end of a run,
    with the times and the read-only bits, so that the disks are read as
    under Node."""
    import base64
    for path in sorted(root.rglob("*"), reverse=True):
        if path.is_dir():
            path.chmod(0o700)
            path.rmdir()
        else:
            path.chmod(0o600)
            path.unlink()
    for entry in tree["entries"]:
        path = root / entry["path"]
        if entry["directory"]:
            path.mkdir()
        else:
            path.write_bytes(base64.b64decode(entry["data"]))
    for entry in reversed(tree["entries"]):
        path = root / entry["path"]
        when = entry["mtime"] / 1000
        os.utime(path, (when, when))
        if entry["readonly"]:
            path.chmod(0o444)
    when = tree["mtime"] / 1000
    os.utime(root, (when, when))


def chromium():
    """The machine's own Chromium, or None."""
    return shutil.which("chromium") or shutil.which("chromium-browser")


# The files of C:\STAMPS that the page cannot hold (see the top).
STAND_IN_FROM = 1 << 20


def drive(page, bench, keys_text, steps, environment, cwd, limit):
    """Take the page `page`, a vc.html, through a scenario on the bench
    laid out in `bench`:
    the records of its screens, (kind, number, Screen) as
    posixrun._screens() has them, with the cells as the pixels show them
    and the cursor as the channel recorded it; what the program said; the
    code that it ended with; and the run's other findings, a dict."""
    home, d = bench / "home", bench / "d"
    stand_ins = []
    for name, size, when in posixrun.setups.STAMPS_FILES:
        if size >= STAND_IN_FROM:
            path = home / "STAMPS" / name
            os.truncate(path, 0)
            os.utime(path, (when, when))
            stand_ins.append([f"/drive/c/STAMPS/{name}", size])
    job = {
        "page": str(Path(page).resolve()), "chromium": chromium(),
        "profile": str(WORK / "profiles" / bench.name),
        "drives": {"c": str(home.resolve())},
        # The page has one drive, C:, on its server; the bench's D:, with
        # the program's own files, is given to the page's file system by
        # the hook, and taken back from it at the end.
        "d": tree_of(d), "cHasVc": (home / "vc").exists(),
        "cTime": int(home.stat().st_mtime * 1000),
        "environment": environment, "keys": keys_text,
        "actions": actions(steps), "cwd": cwd, "standIns": stand_ins,
        "limit": limit,
        "shots": str(WORK / "shots" / bench.name),
        "viewport": os.environ.get("VC_PAGE_VIEWPORT", "desktop"),
    }
    WORK.mkdir(parents=True, exist_ok=True)
    job_file = WORK / f"{bench.name}.job.json"
    out_file = WORK / f"{bench.name}.out.json"
    job_file.write_text(json.dumps(job))
    try:
        done = subprocess.run([posixrun.NODE, str(RUNNER), str(job_file),
                               str(out_file)], capture_output=True,
                              timeout=limit + 60)
        if done.returncode != 0 or not out_file.exists():
            raise MachineError("the page's runner ended with "
                               f"{done.returncode}: "
                               + done.stderr.decode(errors="replace")[-2000:])
        out = json.loads(out_file.read_text())
    except subprocess.TimeoutExpired:
        raise MachineError(f"the page did not end within {limit} s") \
            from None
    finally:
        for file in (job_file, out_file):
            file.unlink(missing_ok=True)
        shutil.rmtree(WORK / "profiles" / bench.name, ignore_errors=True)
    said = "\n".join(out["said"])
    if out.get("d"):
        put_tree(d, out["d"])
    if out["error"]:
        raise MachineError(f"the page's run failed: {out['error']}"
                           + (f"; it said: {said}" if said else ""))
    import base64
    records = posixrun._screens(base64.b64decode(out["records"]))
    if len(records) != len(out["screens"]):
        raise MachineError(f"the channel recorded {len(records)} screens "
                           f"and the page was read {len(out['screens'])} "
                           "times")
    findings = {"dom": [], "wrong": [], "off": [], "cursor": []}
    pixel_records = []
    for (kind, number, recorded), shown in zip(records, out["screens"]):
        def screen(cells):
            data = bytearray()
            for cell in cells:
                cell = max(cell, 0)
                data += bytes((cell & 0xFF, cell >> 8))
            return dataclasses.replace(recorded, cells=bytes(data))
        dom, pixels = screen(shown["dom"]), screen(shown["pixels"])
        findings["dom"].append((kind, number, dom))
        pixel_records.append((kind, number, pixels))
        if shown["wrong"]:
            findings["wrong"].append((kind, number, shown["wrong"]))
        if shown["offCount"]:
            findings["off"].append((kind, number, shown["offCount"],
                                    shown["off"]))
        # The cursor as the page draws it, held to the one that the
        # channel recorded (render.js, cursorOf).
        first, last = recorded.cursor_lines
        column, row = recorded.cursor
        want = None
        if not (first & 0x20 or (first & 0x1F) > (last & 0x1F) or
                row >= ROWS or column >= COLUMNS):
            want = {"column": column, "row": row,
                    "block": (last & 0x1F) - (first & 0x1F) > 3}
        if want != shown["cursor"]:
            findings["cursor"].append((kind, number, want, shown["cursor"]))
    code = 2 if "keys have run out" in said else next(
        (number for kind, number, _ in records if kind == "end "), 0)
    findings["ended"] = out["ended"]
    return pixel_records, said, code, findings


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("scenario", nargs="*")
    parser.add_argument("--together", type=int, default=3)
    parser.add_argument("--out", default=str(WORK))
    parser.add_argument("--page", default=str(PAGE))
    parser.add_argument("--kept", action="store_true",
                        help="take the page also through the scenarios "
                             "that press a key that the browser keeps")
    parser.add_argument("--viewport", default="desktop",
                        choices=("desktop", "portrait", "landscape"),
                        help="the browser's viewport: on a phone's, the "
                             "pixels are held to a lattice of cells of "
                             "their colours, not to the glyphs")
    args = parser.parse_args()
    os.environ["VC_PAGE_VIEWPORT"] = args.viewport
    if chromium() is None or not Path(args.page).exists():
        print("no Chromium, or no page: skipped")
        sys.exit(77)
    import goldens
    import scenarios
    import setups
    program = setups.Program.load(args.page)
    names = args.scenario or scenarios.names(program.tree)
    kept = {name: kept_keys(scenarios.of(name, program.tree).steps)
            for name in names}
    if not args.kept:
        names = [name for name in names if not kept[name]]
    expected = {name: goldens.read(name, program.tree) for name in names}
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    def one(name):
        began = time.monotonic()
        try:
            run = scenarios.run(scenarios.of(name, program.tree), program,
                                None, False, limit=600)
        except MachineError as error:
            return name, None, str(error), time.monotonic() - began
        return name, run, None, time.monotonic() - began

    failed = False
    lines = []
    causes = {}                   # a cause -> the scenarios of it
    with ThreadPoolExecutor(args.together) as pool:
        for name, run, stopped, took in pool.map(one, names):
            golden = expected[name]
            if stopped:
                failed = True
                line = f"{name}: stopped: {stopped}"
                lines.append(line)
                cause = re.sub(r"\d+", "N", stopped.split("; it said")[0]
                               .split("\n")[0])
                causes.setdefault("stopped: " + cause, []).append(name)
                print(line.split("\n")[0], flush=True)
                continue
            found = run.page
            judged = {}
            for way, screens in (("pixels", run.screens),
                                 ("dom", found["dom"])):
                ran = copy.copy(run)
                ran.screens = screens
                judged[way] = list(goldens.departures(
                    golden, ran, {program.filename}))
            notes = []
            for kind in ("wrong", "off", "cursor"):
                if found[kind]:
                    notes.append(f"{kind} {len(found[kind])}")
            ok = not judged["pixels"] and not judged["dom"] and \
                not found["wrong"] and not found["off"] and \
                not found["cursor"]
            failed |= not ok
            head = (f"{name}: {'passed' if ok else 'FAILED'}"
                    f" ({took:.0f} s; {len(run.screens)} screens)"
                    + (f" [{', '.join(notes)}]" if notes else ""))
            lines.append(head)
            if not ok:
                way = "dom" if judged["dom"] else "pixels" \
                    if judged["pixels"] else None
                if way:
                    where, rows = judged[way][0]
                    cause = f"{way} first differs {where}"
                    if rows and rows[0].startswith("row"):
                        cause += f", {rows[0].split()[0]} {rows[0].split()[1]}"
                else:
                    cause = "the page's pixels, cursor or rows: " + \
                        ", ".join(notes)
                causes.setdefault(cause, []).append(name)
            for way in ("dom", "pixels"):
                if judged[way]:
                    where, rows = judged[way][0]
                    lines.append(f"  {way}: first {where}")
                    lines += ["    " + row for row in rows[:3]]
            for kind in ("wrong", "cursor"):
                for item in found[kind][:2]:
                    lines.append(f"  {kind}: {item}")
            for kind, number, count, cells in found["off"][:2]:
                lines.append(f"  pixels not the <pre> at {kind.strip()} "
                             f"{number}: {count} cells, first "
                             + ", ".join(f"r{r}c{c} {w:04X}->{g:04X} by {o}"
                                         for r, c, w, g, o in cells[:6]))
            if run.said:
                lines.append("  said: " + run.said.replace("\n", " | ")[:300])
            print(head, flush=True)
    for name, keys in kept.items():
        if keys and name not in names:
            lines.append(f"{name}: not taken through the page: the browser "
                         f"keeps {', '.join(keys)} (W4)")
    (out / "results.txt").write_text("\n".join(lines) + "\n")
    passed = sum(1 for line in lines if ": passed (" in line)
    summary = [f"{passed} passed, {sum(map(len, causes.values()))} failed, "
               f"{sum(1 for keys in kept.values() if keys)} not taken "
               "through the page (W4)", ""]
    for cause, which in sorted(causes.items(), key=lambda c: -len(c[1])):
        summary.append(f"{len(which)}: {cause}")
        summary.append("    " + " ".join(which))
    (out / "summary.txt").write_text("\n".join(summary) + "\n")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
