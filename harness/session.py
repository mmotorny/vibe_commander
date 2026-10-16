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
  by harness/websession.mjs on build/portable_c/web/vc.html, made a
  static site with its one drive, C:, labelled VC, which holds C:\\VC
  alone, where the page starts, in the Chromium that the machine has
  (`chromium` or `chromium-browser` on the path), headless, with the
  keys as a keyboard sends them to the page. A frame is the page's own
  cells once the screen has stayed the same for a while. The page's
  clock starts at STAMP, in UTC, so that what the session makes on C:
  has the same time at every run.
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
VileR's Web437 IBM VGA 8x16 (CC BY-SA 4.0, LICENSE.FONT), which the web
page's build fetches and pins (CMakeLists.txt) and which is put into
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
PAGE = ROOT / "build" / "portable_c" / "web" / "vc.html"
WORK = ROOT / "build" / "session"   # Chromium of a snap reads no hidden
                                    # directory and not the real /tmp
CHROMIUMS = ("chromium", "chromium-browser")
LIVE = "https://mmotorny.github.io/vibe_commander/"
FONT = (ROOT / "build" / "_deps" / "vga_font-src" / "woff - Web (webfonts)"
        / "Web437_IBM_VGA_8x16.woff")
FILES = ROOT / "harness" / "session_files"
OUT = ROOT / "docs" / "vibe-commander-session.svg"
STAMP = 961156800               # the time of the files: 16 June 2000,
                                # 12:00 UTC, the day of 4.05's release
CELL_W, CELL_H = 8, 16
ESC = "\x1b"

# The steps of the session on the web page, and how long the frame behind
# each is shown, in seconds: a key by the name that harness/browser.mjs
# gives it, with what it is pressed with, or text typed into the editor a
# character at a time, a frame behind each. The first is the page as it
# starts, in C:\VC.
def key(name, seconds, **held):
    return dict(press=name, seconds=seconds, **held)


def typed(text, seconds=0.07):
    return [dict(type=character, seconds=seconds) for character in text]


SESSION = [
    dict(seconds=2.0),                      # the page as it starts
    key("F1", 1.6, ctrl=True),              # Ctrl-F1: the left panel too
    key("F7", 1.0),                         # F7: a directory made
    *typed("NOTES", 0.12),
    key("Enter", 1.4),                      # NOTES in the panel
    key("F4", 1.0, shift=True),             # Shift-F4: a file to edit
    *typed("hello.txt", 0.1),
    key("Enter", 1.2),                      # the question of a new file
    key("Enter", 1.0),                      # the editor
    *typed("Hello from Vibe Commander,"),
    key("Enter", 0.3),
    *typed("in a web page of 80 by 25."),
    key("Enter", 0.3),
    *typed("Its screen is DOS's, cell for cell."),
    key("F2", 1.2),                         # F2: saved
    key("Escape", 1.2),                     # Esc: hello.txt in the panel
    key("Down", 0.8),                       # Down, to hello.txt
    key("F3", 2.5),                         # F3: the viewer
    key("Escape", 1.0),                     # Esc
    key("F9", 1.0),                         # F9: the menu bar, at Right
    key("Enter", 1.2),                      # Enter: its menu
    key("Left", 1.2),                       # Left: the menu of Options
    key("Left", 1.2),                       # Left: of Commands
    key("Left", 1.2),                       # Left: of Files
    key("Right", 0.8),                      # Right
    key("Right", 1.0),                      # Right: of Options again
    key("Down", 0.5),                       # Down
    key("Up", 0.8),                         # Up, to Configuration
    key("Enter", 3.5),                      # Enter: its dialog
    key("Escape", 1.2),                     # Esc
    key("F10", 2.5),                        # F10: the question whether to quit
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
    if not PAGE.exists():
        sys.exit(f"{PAGE} is not built: configure with -DVC_WEB=ON")
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
                    "--chromium", chromium, "--page", str(PAGE),
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
        sys.exit(f"{FONT} is not there: configure with -DVC_WEB=ON")
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
