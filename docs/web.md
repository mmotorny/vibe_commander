# The web page

The portable C is also a web page: the program built to WebAssembly by
Emscripten, as one file, `vc.html`, that holds the program, its script,
its style and its font, and fetches nothing from another site.

- It shares the DOS emulation of [the program for Linux](linux.md), the
  files `*_posix.c` but `sys_posix.c` and `term_posix.c`, with a file of
  its own behind `sys.h`, `src/portable_c/sys_web.c`, and the page's
  scripts in `src/portable_c/web/`.
- Its key loop waits by Emscripten's Asyncify, which lets it wait on the
  network too.
- The screen is the 80 by 25 cells as lines of text in a `<pre>`, in
  VileR's Web437 IBM VGA 8x16 font and the VGA's exact colours, with the
  VGA's blink and cursor, scaled by whole multiples of the font.

`CLAUDE.md`, "The web page", has the decisions it rests on, and
`TODO.md` what is left to do. What it does otherwise than the original
is declared in `src/portable_c/DEVIATIONS.md`, under "The web page"
(W1 to W10), on top of what is declared for Linux.

## How it works

### Drives

| Drive | Label | What it holds |
|---|---|---|
| C: | VIBE | Where the program lives: its own files, the setup file `VC.INI`, the menu and the extension file, in C:\VC. The page starts at the root of C: |
| D: | DEMO | At first, a small tree of sample files written for the project, `src/portable_c/web/demo`, which the user may change |

Both are ordinary drives to the program. The shareware's help and menu
files are not shipped: F1 shows nothing, as the original does without
its help file, until the user brings `VC.HLP` into C:\VC.

### Served by its server

`src/portable_c/web/server.mjs` is a Node script that uses only Node's
own modules, nothing from npm. It serves the page and the two drives'
directories on the machine it runs on:

- The page loads the drives' trees from it.
- Every change on either drive, the setup file's too, is made on the
  server as it happens, before the program goes on. What the server
  refuses or cannot do is DOS's error for that call.
- It never reaches outside the two directories, and follows no host
  link inside them.
- It listens on 127.0.0.1 only. A reverse proxy such as Tailscale Serve
  can hand it to other devices over HTTPS. Whoever can reach it may
  write, so the proxy is the access control.

### Served statically

The same page on GitHub Pages, or any host without the server:

- Its drives start from what is served with it, `drives/c.json` and
  `drives/d.json`.
- What the user changes is kept in that browser alone, in IndexedDB.
  Reset drives in the page's bar forgets the changes.
- Where the browser gives it no IndexedDB, as some private windows do,
  it keeps the changes in memory until the page is closed.

The page finds out how it is served by asking for `api/server`, which
its server answers and a static host does not. If a server does not
answer, the page says so and does not start, so that a page of the
server never keeps its changes in the browser. A host that answers that
name with an error of its own, or does not let the page ask, is taken
for a static one where it serves the drives beside the page, which the
server never does.

### Files in and out

Upload in the bar, and files dragged onto the page, write into DOS's
current directory, asking before a file is overwritten. Enter on a file
for which the extension file has no command opens it in a new browser
tab, where the user may view or save it.

### The phone

It opens on an iPhone, in Safari:

- The grid is fitted to the screen, and fitted again when the phone
  turns; fingers may zoom it. A tap is a click on the cell under the
  finger.
- Where there is no mouse, a row of the keys a phone lacks stands below
  the grid: F1 to F10, Esc, Tab, the arrows, PgUp and PgDn, Ctrl and Alt
  (which stay held for the next key), and a wider Enter at the end, so
  that the phone's keyboard is not needed for it.
- The bar's Keys button shows or hides that row on any device. Its
  keyboard button brings up the phone's keyboard, whose text is typed as
  keys.

### Keys that a browser keeps

A browser never gives an ordinary tab:

- Ctrl-W, Ctrl-T and Ctrl-N, with their Shift forms;
- Ctrl-Tab and Ctrl-Shift-Tab, Ctrl-PgUp and Ctrl-PgDn;
- Ctrl-F4 and Alt-F4;
- the system's keys around it, Cmd on a Mac and the Windows key.

F11 and F12 are the browser's own in most. Every other key that the
program uses is kept from the browser, Alt by itself too. The bar's Full
screen locks the keyboard where the browser allows it, as Chromium
does, and then most of those keys reach the program; holding Esc leaves
full screen. A dead key or an input method's character is not typed.

### Licences

The bar names the program, "Vibe Commander 0.1, based on Volkov
Commander 4.05.", and has one small "Licences" link, on a phone too. It
opens a dialog with the licence of every work that the page carries,
each in the same form with its copyright and home: Vibe Commander's,
Volkov Commander's, Emscripten's, musl's and the font's, the texts of
`LICENSE`, `src/asm/LICENSE.TXT`, `LICENSE.EMSCRIPTEN`, `LICENSE.MUSL`
and `LICENSE.FONT`, which the build puts into the page. Esc and the
dialog's button close it, and no key reaches the program while it is
open.

### What cannot work

- **Commands.** A page has no shell. A command from the command line,
  the user menu or the extension file, and the external viewer and
  editor, write "Commands cannot be run in a web page." on DOS's screen,
  after the prompt and the command, which Ctrl-O shows as on DOS. No
  file is a program.
- **Leaving.** F10 ends the program, and the page says that reloading
  starts it again.
- **The mouse wheel** does nothing, as on Linux.
- **Memory.** The program's blocks are capped at 256 MB together.

## Building it

The web page needs an option of its own, since it fetches Emscripten, a
300 MB download that unpacks to 1.5 GB:

```sh
cmake -B build -DVC_WEB=ON
cmake --build build
```

A second build directory can reuse the first one's download with
`-DFETCHCONTENT_SOURCE_DIR_EMSCRIPTEN=<first>/_deps/emscripten-src`.
`-DVC_DOS=OFF` leaves out Open Watcom and everything for DOS.

| Target | What it makes |
|---|---|
| `portable_c-web` | `build/portable_c/web/vc.html`, the page, at `-O2` with no sanitizer |
| `portable_c-web-static` | `build/portable_c/web/static/`, the static site: the page as `index.html`, the two drives' initial trees, and `.nojekyll` |
| `portable_c-web-node` | `build/portable_c/web-node/vc.js` with its `vc.wasm`: the program under Node with the host's files, sanitized. The scenarios run this one |
| `portable_c-web-serve` | Not built by default. It builds the page and runs its server in the foreground on `http://127.0.0.1:8437/`, with C: in `build/portable_c/web/drives/c` and D: in `build/portable_c/web/drives/d`. D: is made as a copy of the demo the first time; delete the directory to get the demo afresh. Ctrl-C stops it |

## Running it

Use the target above, or run the server with other directories or
another port:

```sh
node src/portable_c/web/server.mjs --port 8437 \
    --page build/portable_c/web/vc.html \
    --drive-c DIR --drive-d DIR [--demo DIR]
```

For a phone, run `tailscale serve --bg 8437` beside it, which hands
`https://<machine>.<tailnet>.ts.net/` to the port. `tailscale serve
status` shows it, and `tailscale serve --https=443 off` removes it.

To try the static site locally, run
`python3 -m http.server -d build/portable_c/web/static 8438` and open
`http://127.0.0.1:8438/`. Opened as a `file://` URL, the page cannot
fetch its drives.

`.github/workflows/pages.yml` publishes the static site to GitHub
Pages. It does nothing until the repository variable `VC_PAGES` is
`publish` and Pages is switched on, which is the repository owner's
choice.

## How it is judged

- The program under Node goes through the scenarios as the program for
  Linux does.
- The page's script and its server are tested in Node, with nothing
  downloaded for them.
- The page itself is opened in the machine's own Chromium, headless,
  both from its server and as the static site, on a desktop and as an
  iPhone held both ways.

What no test can judge, how it looks and feels in Firefox and in Safari
on the iPhone, is checked by hand, from a list in `TODO.md`. To save
screenshots of the page to look at:

```sh
node harness/webbrowser.mjs --page build/portable_c/web/vc.html \
    --site build/portable_c/web/static --work build/browser \
    --shots build/browser/shots
```

For each way of serving, this saves the desktop at its start, with its
two panels and with its Licences, and the iPhone upright and turned, as
`build/browser/shots/{server,static}-*.png`.

| Test | What it checks |
|---|---|
| `portable_c-web-probe` | Only with `-DVC_WEB=ON`. Every file that the web build compiles (the common code, the DOS emulation it shares with Linux, and its own) compiled by emcc for wasm32 with the Linux program's warnings and `-Werror`, each failure printed. `harness/webprobe.py`, which takes from the configuration the list of Linux files that the web leaves out |
| `portable_c-web-scenarios` | Only with `-DVC_WEB=ON`. `web-node/vc.js`, built with Asyncify and the address and undefined-behaviour sanitizers, through the scenarios as `portable_c-posix-scenarios` takes the program for Linux, on the same bench: its C: is the bench's home directory, and D: a directory of the run's with the program's own files in D:\VC. Screens and files are held to the original's goldens with what is declared for the portable C, for Linux (but the masks of the drive's letter) and for the web. Each scenario not run or cut on the web is named by an entry for Linux or for the web page in `DEVIATIONS.md` |
| `portable_c-web-page` | Only with `-DVC_WEB=ON`. `vc.html` itself, its scripts run in Node with a stand-in document. Against its server: the drives loaded; the Licences link showing each carried file's text, closed by Esc and its button with no key reaching the program; a directory made; a file opened in a tab; a file written in the editor reaching the server when closed; a directory removed; a server refusal shown as DOS's error; the setup saved on C:; and the page's end. As a static site, behind a host that answers 404 for what it lacks, on a stand-in IndexedDB: changes made and kept, the page reloaded with them, the reset, and a browser with no IndexedDB. Behind hosts that answer 200 with a page, or 500, for what they lack, still a static site; and a server that is away, answering 502, for which the page does not start. `harness/webpage.py` |
| `portable_c-web-browser` | Only with `-DVC_WEB=ON`, and skipped where the path has no `chromium`. The page in the machine's own Chromium, headless, driven over the DevTools protocol from Node with nothing from npm, from its server on a free port and as the static site behind a host that answers 404. On a 1280 by 800 desktop: a start with no error, the bar's line about where changes are kept right for the way it is served, the VGA font loaded, the grid 640 by 400, the VGA colours, Ctrl-F1, Alt-F2 and D:'s demo, the cursor keys and Tab, F1, a file written in the editor (on the server, or kept across a reload of the static page), the mouse, a click on a file, a double click into a directory, a click on the menu bar and one on the key bar, each followed by a key that has to take effect, Enter on a text file opening a tab of a blob, and the Licences opened and closed by Esc and its button with no key reaching the program. As an iPhone, 390 by 844 at three pixels to one, with touch and Safari's user agent: the row of keys, its Ctrl and Alt held for F1 and F2, a tap on a file moving the cursor there, a tap on the key bar with the row's Enter behind it, and the grid within the screen's width above the bar; turned to 844 by 390, the grid fitted again. And no console error but the 404 by which a static page knows it has no server. `harness/webbrowser.py`, `harness/webbrowser.mjs` |
| `portable_c-web-render` | The page's modules for drawing, keys and the mouse, `render.js`, `keys.js` and `mouse.js`, in Node. Grids that the program for Linux writes through its test channel are drawn, and the HTML is read back and checked against the cells and the rows that changed. The keys and mouse reports that the modules make of browser events are typed into the program for Linux in a pseudo-terminal, and what it reads is checked against the BIOS's words. `harness/webrender.py`; skipped without Node or clang |
| `portable_c-web-server` | The server and the page's file system in Node: the server's API and its refusal of every path outside the two drives, host links among them; the page's file system against it, with a file system of Emscripten's shape; what the static page keeps in the browser, `kept.js`, in memory and on a stand-in IndexedDB, with every change and refusal, the next visit, the reset and a full store; and the page's script bundled, compiled and run up to the program. `harness/webserver.py`; skipped without Node |

What is declared for the web is `ON_WEB` of `harness/deviations.py`. It
goes on top of the portable C's as Linux's does, with Linux's scenarios
not run and cut, and Linux's masks except those of the drive's letter,
since the tests map the letters as the bench has them. A patch of the
web's own would be in `harness/goldens/web/`, which has none.

## What it is made with

- **Emscripten 6.0.12**, under the MIT and University of Illinois/NCSA
  licences, which builds its C library from **musl**, under MIT. The
  page has Emscripten's runtime in its script and musl's code in its
  WebAssembly; their notices are `LICENSE.EMSCRIPTEN` and
  `LICENSE.MUSL`.
- **The font**, Web437 IBM VGA 8x16 from the Ultimate Oldschool PC Font
  Pack 2.2 by VileR, under CC BY-SA 4.0, put into the page unchanged;
  its licence is `LICENSE.FONT`.

The page's "Licences" shows these three alongside the project's own
licence and Volkov Commander's. Emscripten and the font are fetched by
the build, pinned by SHA-256, only with `-DVC_WEB=ON`: see
[Downloads](../README.md#downloads) and [the
licences](../README.md#licences-and-credits).
