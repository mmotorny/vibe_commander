#!/usr/bin/env python3
"""A session of the program, as an animated SVG for the README.

GitHub shows no page of its own in a README, neither a frame nor a
script, so the README cannot run the program. It shows it in motion
instead: docs/vibe-commander-session.svg, which this script makes, and
which the README links to the web page on GitHub Pages, where the
program runs. Each frame is a screen of the program, shown for the
time that its step gives it.

The screens are taken one of two ways:

- From the web page, the default, as it is published: SESSION is played
  by harness/websession.mjs on the site that the build made,
  build/portable_c/web/static, with C:\\VC and C:\\ as they are shipped
  and A: empty, in the Chromium that the machine has (`chromium` or
  `chromium-browser` on the path), headless, with the keys and the mouse
  as a keyboard and a mouse send them to the page. A frame is the page's
  own cells behind a step. The page's clock starts at STAMP, in UTC, and
  goes on only as the session lets it, so that the clock, the files
  that the session makes and the screen saver's stars are the same at
  every run.
- From the program for Linux, with --linux, as the README's animation
  was first made: SESSION_LINUX is played on build/portable_c/posix/vc
  in a pseudo-terminal of 80 by 25 as harness/terminal.py does, its keys
  as xterm sends them, each when the program has written nothing for a
  while, and a frame is what the terminal shows behind each, replayed
  through pyte (the pinned download of harness/deps.py). The files that
  it lists are harness/session_files, copied with fixed times into a
  home directory of a directory of /tmp of the run's own, as
  harness/posixrun.py has its bench, with an empty directory for the
  setup file beside it, so that the program starts with its defaults;
  their lines are ended as DOS ends them, CR LF.

The frames are drawn as the program draws them: 80 by 25 cells of 8 by
16 pixels, the colours of the VGA's mode 3, and the characters in
VileR's Web437 IBM VGA 8x16 (CC BY-SA 4.0, LICENSE.FONT), which the
repository keeps, src/portable_c/web/font/, and which is put into
the SVG as it is, about 13 KB. A row that several frames have is drawn
once and used by each, and the frames stand one under the other on a
strip that CSS moves up frame by frame: no script, so that GitHub shows
it as an image. The SVG is the screen and nothing else: the README's
link around it and its caption say that the program can be tried live.

    python3 harness/session.py              # docs/vibe-commander-session.svg
    python3 harness/session.py --text       # the frames as text as well
    python3 harness/session.py --linux      # from the program for Linux

with the project built with -DVC_WEB=ON, which fetches the font and
builds the page.
"""

import argparse
import base64
import html
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

import deps
import terminal

ROOT = deps.ROOT
PROGRAM = ROOT / "build" / "portable_c" / "posix" / "vc"
SITE = ROOT / "build" / "portable_c" / "web" / "static"
WORK = ROOT / "build" / "session"   # Chromium of a snap reads no hidden
                                    # directory and not the real /tmp
CHROMIUMS = ("chromium", "chromium-browser")
LIVE = "https://mmotorny.github.io/vibe_commander/"
FONT = ROOT / "src" / "portable_c" / "web" / "font" / "Web437_IBM_VGA_8x16.woff"
FILES = ROOT / "harness" / "session_files"
OUT = ROOT / "docs" / "vibe-commander-session.svg"
STAMP = 961156800               # the time of the files: 16 June 2000,
                                # 12:00 UTC, the day of 4.05's release
CELL_W, CELL_H = 8, 16
ESC = "\x1b"

# The steps of the session on the web page, and how long the frame behind
# each is shown, in seconds: a key by the name that harness/browser.mjs
# gives it, with what it is pressed with, text typed a character at a
# time, a frame behind each, or the mouse's pointer moved onto a cell;
# and, where it is not a second, how long the page's clock runs before the
# frame is taken, in milliseconds (websession.mjs). The first is the page
# as it starts, C:\VC on the left and C:\ on the right.
def key(name, seconds, run=None, **held):
    step = dict(press=name, seconds=seconds, **held)
    if run is not None:
        step["run"] = run
    return step


def typed(text, seconds=0.07):
    return [dict(type=character, seconds=seconds) for character in text]


SESSION = [
    dict(mouse=(79, 24), seconds=2.0),      # the page as it starts, the
                                            # pointer on the key bar
    key("F2", 0.8, alt=True),               # Alt-F2: the drives of the
                                            # right panel
    key("Home", 0.6),                       # Home, to A:
    key("Enter", 1.2),                      # Enter: A:, empty
    key("Down", 0.6),                       # Down, to LICENSE.TXT
    key("Insert", 0.5),                     # Insert: it and the next two
    key("Insert", 0.5),                     # selected
    key("Insert", 0.8),
    key("F5", 1.2),                         # F5: the dialog of the copy,
                                            # to A:\
    key("Enter", 0.3, run=150),             # Enter: the box of its progress
    dict(seconds=1.8, run=320, quantum=5),  # VC.EXE half copied
    dict(seconds=1.6),                      # A: with the copies
    key("Tab", 0.5),                        # Tab: the right panel, A:
    key("F2", 0.6, alt=True),               # Alt-F2
    key("End", 0.6),                        # End, to C:
    key("Enter", 0.6),                      # Enter: C:\VC, DOS's current
                                            # directory of C:
    key("Enter", 1.0),                      # Enter on ..: C:\
    key("Down", 0.6),                       # Down, to AUTOEXEC.BAT
    key("F3", 1.6),                         # F3: the viewer
    key("Escape", 0.6),                     # Esc
    key("o", 1.4, ctrl=True),               # Ctrl-O: DOS's screen
    key("o", 0.8, ctrl=True),               # Ctrl-O: the panels
    key("Tab", 0.5),                        # Tab: the left panel
    key("l", 1.6, ctrl=True),               # Ctrl-L: the Info panel
    key("F10", 1.6, alt=True),              # Alt-F10: the tree
    key("Escape", 0.6),                     # Esc
    key("Down", 0.5),                       # Down, to LICENSE.TXT
    key("F8", 1.4),                         # F8: the red box of the delete
    key("Escape", 0.6),                     # Esc
    key("F9", 0.7),                         # F9: the menu bar
    key("Enter", 0.8),                      # Enter: the menu of Left
    key("Right", 0.8),                      # Right: of Files
    key("Right", 0.8),                      # Right: of Commands
    key("Right", 0.8),                      # Right: of Options
    key("Right", 0.8),                      # Right: of Right
    key("Escape", 0.6),                     # Esc
    dict(mouse=(79, 0), seconds=1.5, run=4500),  # the pointer in the top
    dict(seconds=1.5, run=1500),            # right corner: the screen
    dict(seconds=1.5, run=1500),            # saver's stars
    dict(seconds=1.5, run=1500),
]

# The keys of the session for Linux, as xterm sends them, and how long
# the frame behind each is shown, in seconds. The first is the program
# started.
SESSION_LINUX = [
    ("", 2.5),                  # the program as it starts
    (ESC + "[1;5P", 1.6),       # Ctrl-F1: the left panel too
    ("\r", 1.2),                # Enter: into CODE
    (ESC + "[B", 0.6),          # Down
    (ESC + "[B", 1.0),          # Down, to hello.c
    (ESC + "OR", 3.5),          # F3: the viewer
    (ESC, 1.0),                 # Esc
    (ESC + "[H", 0.6),          # Home, to ..
    ("\r", 1.2),                # Enter: back to H:\
    ("\t", 1.0),                # Tab: the right panel
    (ESC + "[B", 0.6),          # Down
    (ESC + "[B", 0.8),          # Down, to NOTES
    ("\r", 1.2),                # Enter: into NOTES
    (ESC + "[B", 1.0),          # Down, to ideas.md
    (ESC + "[20~", 1.2),        # F9: the menu bar
    (ESC + "[C", 0.8),          # Right
    (ESC + "[C", 0.8),          # Right
    (ESC + "[C", 0.8),          # Right, to Options
    ("\r", 1.2),                # Enter: its menu
    (ESC + "[D", 1.2),          # Left: the menu of Commands
    (ESC + "[D", 1.2),          # Left: of Files
    (ESC + "[C", 0.8),          # Right
    (ESC + "[C", 1.0),          # Right: of Options again
    (ESC + "[B", 0.5),          # Down
    (ESC + "[B", 0.5),          # Down
    (ESC + "[A", 0.5),          # Up
    (ESC + "[A", 0.8),          # Up, to Configuration
    ("\r", 3.5),                # Enter: its dialog
    (ESC, 1.2),                 # Esc
    (ESC + "[21~", 2.5),        # F10: the question whether to quit
]
QUIT = "\r"                     # Enter: the program ends


def lay_out(work):
    """The home directory with the session's files, and the setup file's
    directory beside it, each time fixed."""
    home = work / "home"
    for source in sorted(FILES.rglob("*")):
        target = home / source.relative_to(FILES)
        if source.is_dir():
            target.mkdir(parents=True)
        else:
            target.write_bytes(source.read_bytes().replace(b"\n", b"\r\n"))
    config = work / "config"
    (config / "vc").mkdir(parents=True)
    for path in sorted(home.rglob("*"), reverse=True) + [home]:
        os.utime(path, (STAMP, STAMP))
    return home, config


def snapshot(screen):
    """A frame: the rows, each a tuple of its cells (character, colour,
    background, blink), and the cursor, (column, row) or None."""
    rows = tuple(tuple((cell.data, cell.fg, cell.bg, cell.blink)
                       for cell in (screen.buffer[row][column]
                                    for column in range(80)))
                 for row in range(25))
    cursor = None if screen.cursor.hidden else (screen.cursor.x,
                                                screen.cursor.y)
    return rows, cursor


# The VGA's 16 colours of mode 3, as the program writes them in 24 bits
# and as the page's render.js has them.
VGA = ["000000", "0000aa", "00aa00", "00aaaa", "aa0000", "aa00aa", "aa5500",
       "aaaaaa", "555555", "5555ff", "55ff55", "55ffff", "ff5555", "ff55ff",
       "ffff55", "ffffff"]


def record_web():
    """The frames of SESSION played on the web page, each with its time,
    in the form of a frame of the terminal (snapshot)."""
    if not (SITE / "index.html").exists():
        sys.exit(f"{SITE} is not built: configure with -DVC_WEB=ON")
    node = shutil.which("node")
    chromium = next((found for found in map(shutil.which, CHROMIUMS)
                     if found), None)
    if node is None or chromium is None:
        sys.exit("the web page's session wants node and chromium on the path")
    WORK.mkdir(parents=True, exist_ok=True)
    steps, frames = WORK / "steps.json", WORK / "frames.json"
    steps.write_text(json.dumps(SESSION))
    frames.unlink(missing_ok=True)
    subprocess.run([node, str(ROOT / "harness" / "websession.mjs"),
                    "--chromium", chromium, "--site", str(SITE),
                    "--work", str(WORK), "--steps", str(steps),
                    "--stamp", str(STAMP * 1000), "--out", str(frames)],
                   check=True, timeout=600)
    taken = []
    for frame in json.loads(frames.read_text()):
        rows = tuple(tuple((character, VGA[attribute & 0x0F],
                            VGA[attribute >> 4 & 0x07],
                            attribute & 0x80 != 0)
                           for character, attribute in row)
                     for row in frame["rows"])
        cursor = frame["cursor"]
        taken.append(((rows, None if cursor is None
                       else (cursor[0], cursor[1])), frame["seconds"]))
    return taken


def record():
    """The frames of SESSION_LINUX, each with its time."""
    if not PROGRAM.exists():
        sys.exit(f"{PROGRAM} is not built")
    pyte = terminal.load_pyte()
    with tempfile.TemporaryDirectory(prefix="vc-session-") as work:
        return played(pyte, Path(work))


def played(pyte, work):
    home, config = lay_out(work)
    term = terminal.Terminal(pyte, PROGRAM, home, home, work, 80, 25,
                             "session", COLORTERM="truecolor", TZ="UTC",
                             XDG_CONFIG_HOME=str(config))
    frames = []
    try:
        term.read(1.0)
        for keys, seconds in SESSION_LINUX:
            if keys:
                term.type(keys.encode(), 0.6)
            frames.append((snapshot(term.screen), seconds))
        os.write(term.master, QUIT.encode())
        term.read(0.5)
        with os.fdopen(term.ended) as ended:
            status = int(ended.readline() or -1)
        term.process.wait(timeout=10)
        term.errors.close()
    finally:
        if term.process.poll() is None:
            os.kill(term.pid, 9)
            term.process.kill()
    if status != 0:
        sys.exit(f"the program ended with the status {status}, where it "
                 f"is to end with 0: "
                 f"{term.errors_path.read_text(errors='replace')}")
    return frames


def colour(value, default):
    """A colour of pyte's, which the program writes in 24 bits: a
    terminal's own, "default", only where it has written nothing."""
    return "#" + value if len(value) == 6 else default


def row_svg(cells):
    """A row drawn at the top of its own coordinates: the runs of a
    background as rectangles, the runs of a colour as texts."""
    parts = []
    column = 0
    while column < 80:
        end = column
        while end < 80 and cells[end][2] == cells[column][2]:
            end += 1
        back = colour(cells[column][2], "#000000")
        if back != "#000000":
            parts.append(f'<rect x="{column * CELL_W}" width="'
                         f'{(end - column) * CELL_W}" height="{CELL_H}" '
                         f'fill="{back}"/>')
        column = end
    column = 0
    while column < 80:
        end = column
        key = cells[column][1], cells[column][3]
        while end < 80 and (cells[end][1], cells[end][3]) == key:
            end += 1
        text = "".join(cell[0] for cell in cells[column:end])
        stripped = text.rstrip(" ")
        lead = len(stripped) - len(stripped.lstrip(" "))
        stripped = stripped.lstrip(" ")
        if stripped:
            blink = ' class="b"' if key[1] else ""
            parts.append(f'<text x="{(column + lead) * CELL_W}" y="12" '
                         f'fill="{colour(key[0], "#aaaaaa")}"{blink}>'
                         f'{html.escape(stripped, quote=False)}</text>')
        column = end
    return "".join(parts)


def svg(frames, title):
    """The animated SVG of the frames."""
    # Frames that are the same one behind the other are one.
    merged = []
    for frame, seconds in frames:
        if merged and merged[-1][0] == frame:
            merged[-1][1] += seconds
        else:
            merged.append([frame, seconds])
    total = sum(seconds for _, seconds in merged)
    rows = {}
    for (lines, _), _ in merged:
        for line in lines:
            rows.setdefault(line, f"r{len(rows):x}")
    width, height = 80 * CELL_W, 25 * CELL_H
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" '
           f'xmlns:xlink="http://www.w3.org/1999/xlink" '
           f'viewBox="0 0 {width} {height}" width="{width}" '
           f'height="{height}">',
           f"<title>{title}</title>",
           "<desc>A session of Vibe Commander 0.1, made by "
           f"harness/session.py; the program runs live at {LIVE}. The font is VileR's Web437 IBM VGA 8x16 "
           "of the Ultimate Oldschool PC Font Pack, (c) 2016-2020 VileR, "
           "https://int10h.org/oldschool-pc-fonts/, under the Creative "
           "Commons Attribution-ShareAlike 4.0 International licence, "
           "https://creativecommons.org/licenses/by-sa/4.0/, as it comes, "
           "unchanged.</desc>",
           "<style>"]
    font = base64.b64encode(FONT.read_bytes()).decode()
    out.append('@font-face{font-family:"Web437 IBM VGA 8x16";'
               f'src:url(data:font/woff;base64,{font}) format("woff")}}')
    out.append('text{font-family:"Web437 IBM VGA 8x16",monospace;'
               'font-size:16px;white-space:pre;font-kerning:none;'
               'font-variant-ligatures:none}')
    keys = []
    at = 0.0
    for index, (_, seconds) in enumerate(merged):
        keys.append(f"{at / total * 100:.3f}%{{transform:translateY("
                    f"-{index * height}px)}}")
        at += seconds
    out.append(f"#film{{animation:film {total:.2f}s steps(1,end) "
               f"infinite}}@keyframes film{{{''.join(keys)}"
               f"100%{{transform:translateY(-{(len(merged) - 1) * height}"
               "px)}}")
    out.append(".b,.c{animation:blink .53s steps(1,end) infinite}"
               "@keyframes blink{0%{opacity:1}50%{opacity:0}}")
    out.append("</style>")
    out.append(f'<rect width="{width}" height="{height}" fill="#000"/>')
    out.append("<defs>")
    out.append(f'<clipPath id="screen"><rect width="{width}" '
               f'height="{height}"/></clipPath>')
    for line, name in rows.items():
        out.append(f'<g id="{name}">{row_svg(line)}</g>')
    out.append("</defs>")
    out.append('<g clip-path="url(#screen)"><g id="film">')
    for index, ((lines, cursor), _) in enumerate(merged):
        out.append(f'<g transform="translate(0 {index * height})">')
        for row, line in enumerate(lines):
            out.append(f'<use xlink:href="#{rows[line]}" y="{row * CELL_H}"/>')
        if cursor and cursor[1] < 25 and cursor[0] < 80:
            fore = colour(lines[cursor[1]][cursor[0]][1], "#aaaaaa")
            out.append(f'<rect class="c" x="{cursor[0] * CELL_W}" '
                       f'y="{cursor[1] * CELL_H + 14}" width="{CELL_W}" '
                       f'height="2" fill="{fore}"/>')
        out.append("</g>")
    out.append("</g></g>")
    out.append("</svg>")
    return "\n".join(out) + "\n", len(merged), total, len(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--out", type=Path, default=OUT)
    parser.add_argument("--text", action="store_true",
                        help="print the frames as text")
    parser.add_argument("--linux", action="store_true",
                        help="take the screens from the program for Linux")
    args = parser.parse_args()
    if not FONT.exists():
        sys.exit(f"{FONT} is not there")
    if args.linux:
        frames = record()
        title = "Vibe Commander 0.1 in a terminal"
    else:
        frames = record_web()
        title = "Vibe Commander 0.1 in a web page"
    if args.text:
        for ((lines, cursor), seconds) in frames:
            print(f"--- {seconds}s, the cursor {cursor}")
            print("\n".join("".join(cell[0] for cell in line)
                            for line in lines))
    text, count, total, rows = svg(frames, title)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text)
    print(f"{args.out}: {count} frames, {total:.1f} seconds, {rows} rows, "
          f"{len(text.encode()):,} bytes")


if __name__ == "__main__":
    main()
