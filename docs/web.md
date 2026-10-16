# The web page

The same source is also a web page: the program
built to WebAssembly by Emscripten, one file, `vc.html`, with the
program, its script, its style and its font in it, and nothing fetched
from another site. `CLAUDE.md`, "The web page", has the decisions that it rests on, and
`TODO.md` what is left of it. It shares the emulation of DOS of [the
program for Linux](linux.md), the files `*_posix.c` but
`sys_posix.c` and `term_posix.c`, with a file of its own behind
`sys.h`, `src/portable_c/sys_web.c`, and the page's own script in
`src/portable_c/web/`. Its key loop waits by Emscripten's Asyncify, which
lets it wait on the network too. The screen is the 80 by 25 cells as
lines of text in a `<pre>`, in VileR's Web437 IBM VGA 8x16 and the VGA's
colours exactly, with the blink and the cursor of the VGA, scaled by
whole multiples of the font.

- **Two drives.** C:, labelled VIBE, is where the program lives: its
  own files, the setup file `VC.INI`, the menu and the extension file,
  are in C:\VC. D:, labelled DEMO, starts as a small tree of sample
  files written for the project, `src/portable_c/web/demo`, which the
  user may change. Both are ordinary drives to the program. The page
  starts at the root of C:. The shareware's help and menu files are not
  shipped: F1 shows nothing, as the original does without its help
  file, until the user brings
  `VC.HLP` into C:\VC.
- **Served by its server.** `src/portable_c/web/server.mjs`, a script
  for Node of Node's own modules, nothing from npm, serves the page and
  the two drives' directories on the machine it runs on: the page loads their
  trees from it, and every change on either drive, the setup file's
  too, is made on the server as it happens, before the program goes
  on; what the server refuses or cannot do is DOS's error for that
  call. It never reaches outside the two directories and follows no
  link of the host's in them. It listens on 127.0.0.1 alone, and
  a reverse proxy such as Tailscale Serve can hand it to other devices
  over HTTPS: whoever can reach it may write, so the proxy is the
  access control.
- **Served statically.** The same page, from GitHub Pages or any host
  with no server, has its drives start from what is served with it,
  `drives/c.json` and `drives/d.json`, and keeps what the user changes
  in the browser, in IndexedDB, in that browser alone; Reset drives in
  its bar forgets the changes. Where the browser gives it no IndexedDB,
  as some private windows do, it keeps them in the page's memory until
  the page is closed. The page knows which way it is served by asking
  where it is served, `api/server`, which its server answers and a
  static host does not; a server that does not answer is said, and the
  page does not start, so that a page of the server never keeps its
  changes in the browser. A host that answers that name by an error of
  its own, or does not let the page ask for it, is taken for a static
  one where it serves the drives beside the page, which the server
  never does.
- **Files in and out.** Upload in the bar, and files dragged onto the
  page, write into DOS's current directory, asking before a file is
  written over. Enter on a file that the extension file has no command
  for opens it in a new tab of the browser, where the user may view it
  or save it.
- **The phone.** It opens on an iPhone, in Safari: the grid is fitted
  to the screen and fitted again when the phone turns, and the fingers
  may zoom it; a tap is a click on the cell under the finger. Where
  there is no mouse, a row of the keys that a phone lacks stands below
  the grid, F1 to F10, Esc, Tab, the arrows, PgUp and PgDn, with Ctrl
  and Alt that stay held for the next key, and Enter, wider, at its
  end, so that the phone's keyboard is not needed for it; the bar's
  Keys button shows it or hides it anywhere; the bar's keyboard button
  brings up the phone's keyboard, whose text is typed as keys.
- **The keys that a browser keeps.** A browser never hands an ordinary
  tab Ctrl-W, Ctrl-T and Ctrl-N with their Shift forms, Ctrl-Tab and
  Ctrl-Shift-Tab, Ctrl-PgUp and Ctrl-PgDn, Ctrl-F4 and Alt-F4, or the
  keys of the system around it, Cmd on a Mac and the Windows key; F11
  and F12 are the browser's own in most. Every other key that the
  program takes is kept from the browser, Alt alone too. The bar's Full
  screen locks the keyboard where the browser allows it, as Chromium's
  do, and then most of those keys reach the program; holding Esc
  leaves the full screen. A dead key or an input method's character is
  not typed.
- **Licences.** The bar names the program, "Vibe Commander 0.1, based
  on Volkov Commander 4.05.", and has one small "Licences", on a phone
  too, which shows in a dialog of the page the licence of every work
  that the page carries, each in the same form, with its copyright and
  its home: Vibe Commander's own, Volkov Commander's, Emscripten's,
  musl's and the font's, the texts of `LICENSE`, `src/asm/LICENSE.TXT`,
  `LICENSE.EMSCRIPTEN`, `LICENSE.MUSL` and `LICENSE.FONT`, which the
  build puts into the page as it makes it. Esc and its button close it,
  and no key reaches the program while it is up.
- **What cannot work.** A command: a page has no shell, so a command of
  the command line, the user menu or the extension file, and the
  external viewer and editor, write "Commands cannot be run in a web
  page." on DOS's screen, behind the prompt and the command, which
  Ctrl-O shows as on DOS. No file is a program. F10 ends the program,
  and the page says that a reload starts it again. The mouse's wheel
  does nothing, as on Linux.

What it does otherwise than the original for all that is declared in
`src/portable_c/DEVIATIONS.md`, under "The web page", W1 to W9, on top of
what is declared of Linux.

## Building it

Only with an option of its own, since it fetches
Emscripten, a download of 300 MB that unpacks to 1.5 GB:

```sh
cmake -B build -DVC_WEB=ON
cmake --build build
```

A second build directory takes the download of the first with
`-DFETCHCONTENT_SOURCE_DIR_EMSCRIPTEN=<first>/_deps/emscripten-src`;
`-DVC_DOS=OFF` beside it leaves Open Watcom and everything of DOS out.

| Target | What it makes |
|---|---|
| `portable_c-web` | `build/portable_c/web/vc.html`, the page, at `-O2` with no sanitizer |
| `portable_c-web-static` | `build/portable_c/web/static/`, the static site: the page as `index.html`, the two drives' trees as they start, and `.nojekyll` |
| `portable_c-web-node` | `build/portable_c/web-node/vc.js` with its `vc.wasm`, the program under Node with the host's files, sanitized: the one that the scenarios run |
| `portable_c-web-serve` | Not built by default: it builds the page and runs its server in the foreground on `http://127.0.0.1:8437/`, C: in `build/portable_c/web/drives/c` and D: in `build/portable_c/web/drives/d`, D: made as a copy of the demo the first time (delete the directory to have the demo afresh); Ctrl-C stops it |

## Running it

The target above, or the server with other
directories or another port:

```sh
node src/portable_c/web/server.mjs --port 8437 \
    --page build/portable_c/web/vc.html \
    --drive-c DIR --drive-d DIR [--demo DIR]
```

and for the phone, beside it, `tailscale serve --bg 8437`, which hands
`https://<machine>.<tailnet>.ts.net/` to the port; `tailscale serve
status` shows it, `tailscale serve --https=443 off` takes it away. The
static site, on this computer: `python3 -m http.server -d
build/portable_c/web/static 8438` and `http://127.0.0.1:8438/`; opened as
`file://` the page cannot fetch its drives. `.github/workflows/pages.yml`
publishes the static site to GitHub Pages, and does nothing until the
repository's variable `VC_PAGES` is `publish` and Pages is switched on:
it is off unless the repository's owner switches it on.

## How it is judged

The program under Node goes through the scenarios
as the program for Linux does; the page's script and its server are
held in Node, with nothing downloaded for them; and the page itself is
opened in the machine's own Chromium, headless, as its server serves it
and as the static site, on a desktop and as an iPhone held both ways.
What a test cannot judge, how it looks and feels in Firefox and in
Safari on the iPhone, is a pass by hand, whose list is in `TODO.md`. For screenshots of the page to look at:

```sh
node harness/webbrowser.mjs --page build/portable_c/web/vc.html \
    --site build/portable_c/web/static --work build/browser \
    --shots build/browser/shots
```

which saves, for each way of serving, the desktop at its start, with
its two panels and with its Licences, and the iPhone held upright and
turned, as `build/browser/shots/{server,static}-*.png`.

| Test | What it checks |
|---|---|
| `portable_c-web-probe` | Only with `-DVC_WEB=ON`: every file that the build for the web is to compile, the common code, the files of the emulation of DOS that the web shares with Linux and the web's own, compiled by Emscripten's emcc for wasm32 with the warnings of the program for Linux and `-Werror`, each failure printed. `harness/webprobe.py`, which takes the list of what the web leaves of Linux's files from the configuration |
| `portable_c-web-scenarios` | Only with `-DVC_WEB=ON`: the program for the web under Node, `web-node/vc.js`, built by Emscripten with Asyncify and the sanitizers of addresses and of undefined behaviour, through the scenarios as `portable_c-posix-scenarios` takes the program for Linux, on the same bench, its drive C: the bench's home directory and D: a directory of the run's with the program's own files in D:\VC. Its screens and its files are held to the original's goldens with what is declared for the portable C, for Linux but the masks of the drive's letter, and for the web, each scenario that is not run on the web or is cut there named by an entry of Linux or of the web page of `DEVIATIONS.md` |
| `portable_c-web-page` | Only with `-DVC_WEB=ON`: `vc.html` itself, its scripts run in Node with a document of what the page asks of one. Against its server: the drives loaded, the bar's one link to the licences showing the text of each file that the page carries and closed by Esc and its button with no key reaching the program, a directory made, a file opened in a tab, a file written in the editor reaching the server at its close, a directory removed, a refusal of the server shown as DOS's error, the setup saved on C:, and the page's end. As a static site, served by a host that answers 404 for what it has not, on a stand-in of IndexedDB: changes made and kept, the page loaded again with them, the reset, and a browser with no IndexedDB; by hosts that answer 200 with a page or 500 for what they have not, a static site all the same; and a server that is away, by 502, for which the page does not start. `harness/webpage.py` |
| `portable_c-web-browser` | Only with `-DVC_WEB=ON`, and skipped where the path has no `chromium`: the page in the machine's own Chromium, headless, driven over its DevTools protocol from Node with nothing from npm, as its server serves it, on a free port, and as the static site behind a host that answers 404 for what it has not. On a desktop of 1280 by 800: the start with nothing that says it stopped and the bar's line of where changes are kept right for the way it is served, the VGA's font loaded, the grid 640 by 400, the VGA's colours, Ctrl-F1, Alt-F2 and D:'s demo, the cursor's keys and Tab, F1, the editor's file written (on the server, or kept across a reload of the static page), Enter on a text file opening a tab of a blob, the Licences opened and closed by Esc and by its button with no key reaching the program. As an iPhone, 390 by 844 at three pixels to one, with touch and Safari's agent: the row of keys, Ctrl and Alt of it held for F1 and F2, a tap on a file moving the cursor there, the grid within the screen's width above the bar; turned, 844 by 390, the grid fitted again. And no error in the console, but the 404 by which a static page knows that it has no server. `harness/webbrowser.py`, `harness/webbrowser.mjs` |
| `portable_c-web-render` | The page's modules that draw and take the keys and the mouse, `render.js`, `keys.js` and `mouse.js`, in Node: the grids that the program for Linux writes through its test channel drawn, the HTML read back and held to the cells and to the rows that changed; the keys and the mouse's reports that the modules make of the browser's events typed into the program for Linux in a pseudo-terminal, and what it reads held to the BIOS's words. `harness/webrender.py`; skipped where there is no Node or no clang |
| `portable_c-web-server` | The server and the page's file system in Node: the server's API and its refusals of every path outside the two drives, links of the host's among them; the page's file system against it, with a file system of Emscripten's shape; what the static page keeps in the browser, `kept.js`, in memory and on a stand-in of IndexedDB, every change and refusal, the next visit, the reset, a full store; and the page's script bundled, compiled and run up to the program. `harness/webserver.py`; skipped where there is no Node |

What is declared for the web is `ON_WEB` of `harness/deviations.py`, on
top of the portable C's as Linux's is, with Linux's scenarios not run
and cut, and Linux's masks but those of the drive's letter, since the
tests map the letters as the bench has them; a patch of the web's own
would be in `harness/goldens/web/`, which has none.

## What it is made with

Emscripten 6.0.12, under the MIT and the
University of Illinois/NCSA licences, which builds its C library from
musl, under MIT: the page has Emscripten's runtime in its script and
musl's code in its wasm, and their notices are `LICENSE.EMSCRIPTEN` and
`LICENSE.MUSL` (see [the licences](../README.md#licences-and-credits)). The font, from the Ultimate
Oldschool PC Font Pack 2.2 by VileR, is under CC BY-SA 4.0, put into the
page as it comes; its licence is `LICENSE.FONT`. The page's "Licences"
shows these three with the project's own and Volkov Commander's, all
alike. Both are fetched by the build, pinned by SHA-256, only
with `-DVC_WEB=ON`: [Downloads](../README.md#downloads).
