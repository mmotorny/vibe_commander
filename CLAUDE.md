# Instructions for Claude

Standing instructions for working in this repository: the rules, what
the user has decided that still governs the work, and how the work is
done. Record new ones here, not in Claude's memory: memory does not
travel between computers. What is still to do goes into `TODO.md`, not
here; what was done and how is in git's history.

## Working together

- Give an honest opinion, including disagreement, not compliance.

## Third-party software

- Never commit anything proprietary. That includes the shareware Volkov
  Commander distribution and any commercial tool.
- That also covers anything reconstructed from the shareware binaries: no
  decompiled code, no disassembly, none of their strings. Where they hold
  material that the published source lacks, the source reserves room with
  the `ShwCode` and `ShwData` macros of `src/VCLAYOUT.INC`, which emit
  filler only. Recording where such material sits and how long it is, is
  fine; reproducing it is not.
- Only fetch and run software that has a clean licence: open-source
  software, or software obtained from its rights holder on terms that allow
  this use. The Volkov Commander shareware distribution qualifies, because
  it comes from its author's site under his shareware terms. Copies of
  commercial software from mirrors or abandonware sites do not, however
  useful; Borland's TASM and TLINK, which the original build uses, are the
  standing example.
- The same goes for their output. Do not use reference binaries, hashes or
  other results obtained by running such a tool on the side, and do not
  shape code to match them. The binaries Volkov shipped are a different
  matter: they are his own output, obtained from his site, and matching
  them byte for byte is the project's test of fidelity.
- What the original does when it is run is not proprietary: the screens it
  shows and the files it leaves, recorded as goldens in `harness/goldens/`,
  are committed. They are derived by `harness/goldens.py` and never edited
  by hand. What a tree is declared to show otherwise is not written into
  them: see "Deviations".
- Naming such tools is fine. Documentation should say plainly what the
  original was built with and where a reference came from; the restriction
  is on fetching and running them, not on mentioning them.
- Anything the work downloads is fetched at build or run time from its
  official source and pinned, by SHA-256 (see `harness/deps.py`) or by git
  commit (see `CMakeLists.txt`), and fetched only for a build that needs
  it: Open Watcom for a build for DOS, Emscripten for the web page,
  where that makes sense. Anything new to download is asked first. The
  exception, on the user's word of 16 October 2026: the web page's font,
  `src/portable_c/web/font/Web437_IBM_VGA_8x16.woff`, one small file
  under CC BY-SA 4.0, is kept in the repository unchanged with its
  licence, `LICENSE.FONT`, because its site, int10h.org, does not serve
  the build machines reliably.
- No GitHub Actions workflow runs by itself, on the user's word of 16
  October 2026: every workflow's only trigger is `workflow_dispatch`, run
  by hand, never `push`, `pull_request`, `schedule` or any other event,
  so that no push spends the account's Actions minutes. Adding any other
  trigger is asked of the user first. `pages.yml` is so, and stays so.
- DOS/32A, the extender of the program for DOS: its stub is linked from
  the pinned Open Watcom download when the program is built and never
  committed, its licence text is kept in the repository, and the README
  says, as that licence asks, "This product uses DOS/32 Advanced DOS
  Extender technology."

## Naming

- The project is called Vibe Commander. The name of the checkout directory
  or of the remote may be temporary; never write either into source,
  documentation or commit messages.

## Committing and pushing

- Show the result of a piece of work, then wait for confirmation before
  committing it. Work that agents do in a run is the exception: see "Work
  by several agents".
- Commit and push together. A commit is never left unpushed.

## History

- The history is meant to be read: one deliberate, self-contained commit per
  step, described accurately. Get a commit right before it is pushed rather
  than fixing it afterwards.

## The three trees

The source is three trees, and they are a story in that order.

- `src/asm/` is Volkov Commander 4.05 as its author published it, in
  assembly, changed only as far as building it with free tools took. Its
  files are CP866 text with CRLF line endings: edit them as bytes, and do
  not let an editor or a tool re-encode them or change line endings. The
  first commit holds them exactly as published, in `src/` itself, so
  `git diff $(git rev-list --max-parents=0 HEAD):src HEAD:src/asm` shows
  every change made since. The test `asm-bytes` builds this tree and holds
  it against the binaries its author shipped. To read a file, `iconv -f
  cp866` it into a copy in `build/`.
- `src/c/` is that program translated to C for DOS, the C translation, a
  program of 16 bits built by Open Watcom's `wcc`: it behaves as the
  original does, bugs included, and every scenario holds it against the
  original. It is finished and frozen. It changes only to mend a place
  where it is found not to behave as the original, and then toward the
  original.
- `src/portable_c/` is the portable C: the program on its way from DOS to
  modern operating systems. It began as a copy of `src/c/`. This is where
  work goes on.

In prose the first is the assembly or the original source, the second the
C translation, the third the portable C. No tree is called "the port".
The tests are named by the tree they judge: `asm-bytes`, `c-data`,
`c-arith`, `c-scenarios`, and those of the portable C with `portable_c-`
in front.

## The C translation

- Fidelity means the same behaviour, bugs included. A bug of the original
  was translated as it is and reported.
- Its routines and data keep the names they have in the assembly, and its
  data is laid out as the original's, held by `c-data`.
- Where C could be written in more than one way, portable C came before C
  that is tied to this compiler or this machine, that before the
  compiler's intrinsics, and those before inline assembly.
  `docs/translation.md` lists what rests on Open Watcom and every piece
  of inline assembly.
- The mouse is tested like the keys. An instruction of the original that
  no scenario can reach is listed as out of reach in `harness/ported.py`
  with its reason; a new scenario that reaches one fails `c-scenarios`
  until the entry goes.
- How it was made, a batch of routines at a time, and what the batches
  learnt of the assembly, of Open Watcom's 16-bit compiler and of QEMU,
  is in `DIARY.md`, under the days of the translation. Read it there
  before mending the C translation; its build stops by itself
  (`tools/mkexe.c`) where a limit of its layout is crossed.

## The portable C

### The road

The portable C goes from DOS to other systems by a road that the user has
laid out. Steps 0 to 6 are done; step 7 is under way.

0. Done. The setup program, `VCSETUP`, is a part of the program: setup is
   reached by an item of its Options menu, and is judged by scenarios
   whose goldens are recorded from the original setup program.
1. Done. The portable C is a program of 32 bits for DOS, built by Open
   Watcom's `wcc386` in the flat model under the extender DOS/32A, and
   has no other build for DOS: `src/c/` is the program of 16 bits.
2. Done. It uses what 32 bits give: one flat memory, pointers where there
   were segments and offsets, numbers widened where wrapping round at 16
   bits would be unexpected, booleans for what is only true or false. The
   memory for the panels and the texts comes from the system, as much as
   the machine has. A limit of the original that goes when a number is
   widened is a deviation, asked as the work comes to it; those that went
   are entries of `src/portable_c/DEVIATIONS.md`.
3. The abstraction layer is DOS. The program asks the system what it
   asks DOS and the BIOS, as `sys.h` has it: functions named for DOS's
   services, in DOS's terms and values (drive letters, names of 8.3,
   paths of 67 characters, the code page, the BIOS's key words, cells of
   the screen, packed dates, DOS's numbers of errors). Every other system
   emulates DOS behind those functions as well as it can, as DOSBox does
   but at the level of the services and not of the processor: a drive
   letter is a directory of the system's, as DOSBox's `mount`; a name
   longer than 8.3 has an alias of DOS's form, `LONGNA~1`, that stays the
   same for the run and leads back to it; names are converted between
   UTF-8 and the code page where they cross; the screen is a grid of
   cells; a key is made a word of the BIOS. A redesign for modern systems
   from the user's screens was set aside as more than is wanted now.
   That the program saves its preferences and its session by itself, on
   every platform, DOS included, is decided and not started (`TODO.md`,
   "The program saves its setup and its session by itself"); it is of the
   program, not of the layer. The setup file keeps the original's format.
4. The implementation for DOS is `sys_dos.c` behind `sys.h`, with
   `stack_dos.c`.
5. Done, and gone: a stub that built and did not run. Its checks are the
   test `portable_c-tidy`.
6. Done. The program for Linux: see "The program for Linux".
7. Under way. A static web page: see "The web page".

Windows and macOS are not targets.

### One program, one place for the system

- What the program asks of the system, it asks in one place, `sys.h`;
  the rest of it does not know which system that is. Behind it is a file
  of each implementation: `sys_dos.c`, `sys_posix.c` with the other
  `*_posix.c` files of the emulation, and a file of the web page's.
  Whatever of the x86 is still in the common code goes behind `sys.h` as
  Chromium has it: files `_dos.c` and `_posix.c` beside common headers,
  and macros that each platform defines in its own way (`compiler.h`).
  A file whose name ends in no implementation's name is common code.
- One `CMakeLists.txt` builds every implementation, and finds clang for
  those that want it.
- Clang is where this is going, for the platforms it reaches and for its
  sanitizers and static checks. Open Watcom builds the program for DOS for
  as long as DOS is where it is judged. No other toolchain is taken on.
- DOS stays a target of the portable C for as long as DOS is where it is
  judged: every piece of work builds for DOS and passes its scenarios
  there before it lands.
- The order of the translation holds, and is now the point of the work:
  portable C first, then C tied to a compiler or a machine, then
  intrinsics, then assembly inline in a C file. What `docs/portable-c.md`
  lists as resting on Open Watcom is to shrink, not grow.
- The program is Vibe Commander 0.1. Its title and the first line of its
  banner are "Vibe Commander 0.1", and where the original names itself
  without a version it is "Vibe Commander". It says "based on Volkov
  Commander 4.05" wherever the original named its version: behind the
  name in the banner, and in the Info panel on a line of its own between
  the title and the line under it, where the two lines on the memory are
  gone. The banner's second line keeps the author's copyright and adds
  "(C) 2026 Maksym Motorny". No line is added anywhere for it. The C
  translation keeps the original's name.
- Names may change here once a thing no longer is what its name in the
  assembly says. A comment at the new name says what it was, so that the
  code can still be held against `src/c` and `src/asm`.
- The jumps that the translation kept from the assembly, `goto`, go where
  they are easy: in a function that only jumps forward, or whose jumps
  back make a loop with one way in. Those where loops are entered in the
  middle or paths cross (`V00_10` of `view.c`, `C20_00` of `copy.c`,
  `F32_00` and `Run_00` of `usermenu.c`, `F39_00` of `menu.c` among them)
  keep theirs until one is rewritten for another reason. A label that
  goes leaves the assembly's name in a comment where it stood.

### The program for DOS

- It is built by `wcc386` and linked with the stub of DOS/32A:
  `build/portable_c/VC.EXE`, and a build with checks and debugging
  information, `build/portable_c/debug/VC.EXE`.
- It stays in the memory whole while a command runs, and takes the memory
  for the panels and the texts from the extender, as much as there is.
- The drive B: that shares the floppy drive with A: stays, with its prompt
  and its setting, for as long as DOS is a target.
- Alt with a letter names the letter of a US keyboard, as the BIOS gives
  it.
- A search for a text is three to four times slower than the original's on
  the machine's clock, since every byte's capital is asked of DOS through
  the extender (`CaseMap` of `sys_dos.c`, a round to real mode and back).
  Remembering the answers was offered and not taken: it stays so.
- What differs from the original in the program for DOS is decided by the
  user one difference at a time. Decided and declared: what is inherent in
  running under the extender and shows on no screen; that a command finds
  no memory above the megabyte while the extender holds it; and that a
  program of another's which has stayed in the memory, started from this
  one, is cut off from its interrupts when this one is left, with the
  extender's lines of warning on the screen. When DOS ends the program by
  another's hand, the program is told, by a small routine of real mode
  that `sys_dos.c` keeps for it.

### The program for Linux

Step 6, as the user decided it. The tests and `harness/goldens/posix/`
name this section as "CLAUDE.md, step 6".

- A program of 64 bits, built with clang, called `vc`, that runs in a
  terminal and emulates DOS as step 3 says. It keeps its setup file, the
  original's `VC.INI`, at `~/.config/vc/vc.ini`
  (`$XDG_CONFIG_HOME/vc/vc.ini` where that is set).
- There are two drives, always: R:, the root of the file system, labelled
  ROOT, and H:, the user's home directory, labelled HOME. The program
  starts on H: where it is started under the home directory and on R:
  otherwise. DOS's 67 characters of a path stay; a directory whose path
  does not fit even so is reached through the aliases, or not, the
  program starting then in the deepest directory above it that fits.
  Making a label is refused.
- The code page is 437, and the country with it, always; nothing of it is
  a setting. A character that the code page lacks, or a byte that is no
  UTF-8, shows as `_`, as Windows makes a short name, the alias leading
  back to the name as it is. Aliases are numbered in the order of the
  host's names by their bytes and kept for the run. What the program
  makes on the host is named in lower case.
- A directory is never shown read-only. A file of four gigabytes or more
  shows the most that DOS can say, and is never copied or moved cut
  short: the program refuses it with an error. The free room of a disk
  shows at most four gigabytes less 64K, in clusters of up to 64K. A file
  whose name begins with a dot may be overwritten. Hidden, system and
  archive are taken when set and not kept. A pipe, a socket or a device
  is listed as an empty file that cannot be opened. DOS's names of
  devices, NUL, CON and the like, are ordinary names of files. An error
  of the host's with no number of DOS's is DOS's general failure.
- The terminal is driven by the program's own escape sequences and
  termios, no library, with the kitty protocol of the keyboard where the
  terminal offers it. The screen is 80 columns by 25 rows and no other
  size, whatever the terminal's. The PC's sixteen colours are drawn as
  DOS has them, exactly, in 24 bits where the terminal allows it.
- A command runs through `$SHELL -c` on the terminal's own screen, which
  Ctrl-O shows, as on DOS. A name of DOS's form in a command, a word that
  names a file of the current directory, is handed to the shell as the
  host's name.
- Enter on a file runs it if the user may execute it, whatever its name:
  the program asks `sys.h` whether a file runs, where it asked its
  extension, and the panel's filter of programs shows what Enter would
  run. A file that does not run is handed to `xdg-open`.
- The room of the editor and the viewer is as much as the machine has.
- A file of the panel's size stays 32 bits on every platform: a file of
  four gigabytes or more is shown and copied as DOS would.
- How it is judged: see "The scenarios of the program for Linux" under
  "How the portable C is judged". What step 6 left is in
  `TODO.md`, "Step 6, the program for Linux: what is left".

### The web page

Step 7, as the user decided it (`TODO.md` has the plan and its pieces).

- The program built to WebAssembly by Emscripten, fetched and pinned as
  any other download and only for a build that asks for the web page; its
  key loop made to wait by Emscripten's Asyncify; behind `sys.h` a file
  of its own.
- The screen is drawn as text: the 80 by 25 cells as lines of a `<pre>`
  with the colours of DOS in spans, not on a canvas. The font is VileR's
  Web437 IBM VGA 8x16 (CC BY-SA 4.0), kept in the repository and put
  into the page.
- The page is judged in a real browser as well: the Chromium that the
  machine has installed, found on the path and not fetched or pinned,
  simplicity counting for more there than a pin (the user), its test
  skipped where there is none, driven headless over its DevTools
  protocol from Node with nothing from npm, one test of it at a time.
  It opens the page as the static site, on a desktop and as an iPhone
  held both ways, and its screenshots are looked at.
- The tests in Chromium, `portable_c-web-browser` and
  `portable_c-web-page-scenarios`, are kept, under the label `browser`,
  but not run by the pieces, on the user's word of 16 October 2026; the
  user judges the page by hand.
- Every third-party work is credited alike: its licence in a `LICENSE.*`
  file of the repository, and the page, which carries their code and the
  font to whoever opens it, DOS/32A's in the program for DOS of its C:,
  has one small "Licences" link that shows all of those texts, no work
  credited apart from the others.
- One drive of files, on the user's word of 15 October 2026, in place
  of the two that the page had: C:, labelled VC, which holds C:\VC, the
  program's own files. There is no drive of a demo and no sample files.
  The page starts in C:\VC. What C: ships, on the user's word of 16
  October 2026: in C:\VC the project's program for DOS, `VC.EXE`, with
  LICENSE.TXT, every licence that the page carries in one file, which
  the build makes of the repository's (the user, later that day, in
  place of Volkov's and DOS/32A's as two files); the VC.INI that the program saves,
  with the clock on (`tools/webini.py`), the tests' bench keeping its
  own with the clock off; a VC.MNU, VC.EXT, VCVIEW.EXT and VCEDIT.EXT
  of the project's own; and the project's help as `vc.hlp`, which takes
  the place of a README, none being shipped; in C:\ a mock AUTOEXEC.BAT
  and CONFIG.SYS (`src/portable_c/web/drive/`). Nothing of the
  shareware's goes in. Beside it A:, empty and with no label, which the
  page makes, so that changing disks can be tried. The drives are
  ephemeral, on the user's word of 16 October 2026: every load of the
  page starts C: from what is served with the page, and A: empty, in
  the page's own memory, and every change on it, the setup
  file too, lasts until the page is closed or loaded again. Nothing is
  written back to where the page is served from, and there is no store
  of the browser's, IndexedDB or any other. The tests keep the bench's
  layout through the environment, C: the bench's working disk labelled
  VIBE and D: with the program's own files in D:\VC labelled DEMO, so
  that no golden or mask moves; the page's own tests make the files
  that they work on in their drive, and nothing of them is shipped. The
  user's own files come in by a button and by dragging them onto the
  page, into the current directory.
- The project writes its own help in VC.HLP's format, with text of its
  own (`src/portable_c/help/vc-help.txt`, made by `tools/makehelp.py`);
  the shareware's help is never used.
- A command says on DOS's screen that commands cannot be run in a web
  page. Enter on a file that is no program opens it in a new tab of the
  browser, where the user may view it or save it.
- The keys that a browser keeps for itself are declared, and a button for
  the full screen locks the keyboard where the browser allows it. A strip
  of buttons gives the program six of them, Ctrl-PgUp, Ctrl-PgDn,
  Ctrl-T, Ctrl-W, Ctrl-F4 and Alt-F4, small in the bar on a desktop and
  in the row of keys on a phone, with a hint that full screen makes them
  work from the keyboard in Chrome and Edge (the user, 16 October 2026).
  The browser's own mouse pointer is hidden over the grid, where the
  program draws its own.
- It opens on an iPhone: the grid is scaled to fit the screen and fitted
  again when the screen turns or changes size, pinch-zoom allowed; a tap
  is a click on the cell under the finger; where there is no keyboard, a
  row of keys that a phone lacks, F1 to F10, Esc, Tab, Enter, the arrows,
  PgUp and PgDn, and Ctrl and Alt that stay held for the next key, stands
  with a button that brings up the phone's keyboard.
- It is a static site alone, on the user's word of 16 October 2026, in
  place of a server of the project's own that wrote every change to a
  directory of its host and of a static page that kept its changes in
  the browser: the page and the tree that its drive starts from, which
  any host of files serves. A small server of the project's own,
  `web/serve.mjs`, a script for Node with Node's own modules and nothing
  from npm, serves such a site on 127.0.0.1 for the tests and for trying
  the page out, and writes nothing. Publishing the page anywhere but
  GitHub Pages is asked first.
- The static page is published on GitHub Pages, at
  https://mmotorny.github.io/vibe_commander/, on the user's word of 16
  October 2026: by `.github/workflows/pages.yml`, run by hand (`gh
  workflow run pages.yml`) while the repository variable `VC_PAGES` is
  `publish` and Pages has GitHub Actions as its source, both set then.
  A run publishes the page of the commit it runs on; nothing publishes
  it by itself. The README's animation links to it.
- The room of the editor and the viewer has a cap on the web page, since
  the memory of WebAssembly once taken is never given back while the page
  is open; Linux has none. `VC_MEMORY` sets it for a test.
- The README shows a recorded session as an animated SVG made by the
  project's own script from the program's screens (`harness/session.py`):
  the web page as its static site serves it, C: labelled VC and the page
  starting in C:\VC, played in the machine's Chromium and taken from the
  page's own cells, the screen alone: the README links the image and
  the caption under it, "Try it live in your browser", to the published
  page (the user, on 16 October 2026, in place of a button drawn in the
  SVG below the screen).

### Deviations

- What judges the portable C is what judged the C translation: the
  scenarios, with goldens recorded from the original. Where the portable C
  behaves otherwise than the original, that is a deviation.
- Every deviation is declared in `src/portable_c/DEVIATIONS.md`: what the
  original does, what the portable C does, why, and which scenarios show
  it. What a scenario is then expected to show is the original's golden
  with the declared difference applied to it: a patch, a committed file
  of the rows that differ, with its reason, applied to the golden as it is
  derived; or, where one difference is on many screens, as the program's
  name is, a rule that the comparison applies; or, for the program for
  Linux, a mask. A patch that no longer fits the golden it was made for is
  a failure to look at, not to regenerate. The goldens of the original are
  never edited. How each is made: "How the portable C is judged".
- An expectation of a tree's own, in place of the original's golden, is
  allowed only where the original cannot go: `editor-huge` (a file larger
  than the original's room) and `menu-none` (the bar left by the way to
  the menu that is none), and for Linux only where a mask cannot do.
- A feature that only made sense on DOS is removed, not kept as a setting
  that does nothing. Such a removal is declared and made without asking.
  Whether a feature is of that kind is judged by what it gives the user,
  not by how DOS did it: Quick execute ran commands through an interrupt
  that only DOS has, and what it gave, commands run by the shell the
  program was started from, is not DOS's alone. Where that is in doubt, it
  is asked. Any other deviation, a fix of a bug of the original included,
  is asked first: nothing gives leave to remove or break a feature of the
  original without asking.
- A bug of the original stays as it is until its fix has been decided.
- What has been decided is declared, each with its reason, in
  `DEVIATIONS.md`: what was removed as of DOS alone (Memory allocation,
  TSR manager, Screen output, the map of the memory, EGA lines, Quick
  execute, the program's own call, every figure of free memory, the
  unsorted order of a panel), the bugs of the original that are fixed,
  the limits of the original that went with the widening, and the
  entries of the program for Linux. Two decisions there constrain what
  comes: a panel holds at most 65,536 entries, the most that a directory
  of DOS holds, on every system; and a setup file made from the defaults
  has zeros where the original has 64 bytes of whatever lay behind its
  table.
- The screen of more than 25 rows is untested in the portable C until a
  system gives the program a screen of another size: the scenarios that
  went to a tall screen by EGA lines are not run for it, and `TODO.md`,
  "The tall screen", lists what they alone tested. The code that goes by
  the rows stays. Likewise `editor-big`, which has the editor refuse a
  file that the original has no room for, is not run for the portable C.

### Numbers

- A number is signed unless there is a reason for it not to be: a size, a
  count, an index, a length, a place in a text is an `i32`. Keeping a size
  in a type without a sign is widely held to have been a mistake: a
  difference that goes below nothing becomes a huge number without a
  word, where a signed one is negative and a sanitizer names a signed
  overflow. The reasons for a type without a sign, each written down where
  it is used: bits, a mask, a character, a colour, a cell of the screen;
  wrapping that the program counts on, with its cast written out; a width
  that a file, DOS or the BIOS has; and a quantity that can pass two
  gigabytes, the size of a file, of a disk and of what is free on it, the
  sum of what is selected, which stay `u32`, the width that DOS has.
- A number stays 8 or 16 bits only for a reason, written down for every
  one in `harness/width.txt` (see "The records").
- Where a number meets one of another sign or is put into fewer bits, the
  code says so with a cast: the tree is held to clang's warnings of a
  comparison and of a conversion between signed and unsigned and of a
  conversion into fewer bits, none switched off for a line. A warning goes
  by making the type right, which is the first thing to try, or else by a
  cast written at the place. Writing the cast is the point: it makes
  whoever writes it think what happens at the edges. A step or a sum that
  wraps at its own width and that a run reaches gets its cast too, since
  the sanitizers of integers are fatal, with the word `wraps` in a comment
  where the program counts on it. A cast without such a comment says that
  the number fits, by the lines just above; where it fits by something
  further off, a comment says by what.
- An integer type gets a name of its own only where its width is not the
  same everywhere: none is made while `sys.h` keeps DOS's widths. A name
  for what a number merely means, a count of columns, a colour, is not
  made: C holds such a name apart from nothing. Such a number has the type
  of its width, `u8`, `u16`, `u32`, `i32`, or is a `bool`, and its name
  and a comment say what it is. What crosses to the system has the type of
  its width too, signed or not by the rule above: a count of bytes an
  `i32`, the size of a file a `u32`. Where the compiler is to hold two
  things apart, they are structures.
- A word of 16 or 32 bits read from or written into a buffer of bytes,
  which the program does at an address that may be odd as the original
  does, is made of its bytes by `GET16`, `PUT16`, `GET32` and `PUT32` of
  `vc.h`, not read through a cast of the pointer: the sanitizer's check of
  alignment stays on, and nothing rests on the order of a processor's
  bytes. The packed structures of `vc.h` and of the tables lie at any
  place: a member read as a member is right, a pointer taken of one and
  read through its own type is not.
- Where the analyser names the program reading the main data as one block
  across its fields, as the original does, that read goes through the
  main data as bytes at an offset, so that nothing is turned off for it.

### Strictness

Every build with clang has the sanitizers that fit it, a high level of
warnings and clang-tidy, and a warning of any of them stops the build:

- clang's warnings are a list written out in `CMakeLists.txt`, close to
  curl's, not `-Weverything`, each an error.
- clang-tidy runs a broad set, the analyser, `bugprone-*`, `misc-*` and a
  few of `readability-*`; each check that is turned off has its reason in
  `.clang-tidy`. The test `portable_c-tidy` runs it.
- The tests run the build with the sanitizers of addresses, of undefined
  behaviour, of integers and of implicit conversions, all fatal
  (`build/portable_c/posix-asan/vc`). The sanitizer of memory is a build
  of its own, `posix-msan`, built by its name, run once and then kept for
  audits, not by every test.
- The build that is not sanitized, `posix/vc`, has the usual hardening.
- A fall-through of a `switch` and a table of bytes without its zero are
  said by `FALLTHROUGH` and `NONSTRING` of `compiler.h`, not by turning the
  warnings off. A function that does not return is `NO_RETURN` there.
- A function that only its file calls is `static`, and one that other
  files call is declared in `vc.h` (or `sys.h`), once: no file declares
  another's function for itself. A macro that assigns to an argument has
  it in brackets, `(i) = 0`.

## How the portable C is judged

### The tests

`ctest --test-dir build -R portable_c -j4` runs the tests of the portable
C; `-L portable_c`, `-L frozen` and `-LE machine` pick by label. What
each judges:

- `portable_c-scenarios`: the program for DOS through every scenario that
  it is taken through, on the bench, against the goldens with what is
  declared made to them; also which functions of the batches ran
  (`harness/ported.py`) and how much stack each scenario leaves.
- `portable_c-posix-scenarios`: the program for Linux, sanitized, through
  the scenarios (`harness/posixrun.py`); `portable_c-terminal` holds what
  it draws in a pseudo-terminal to its grid of cells;
  `portable_c-posix-unit` runs `harness/posixcheck.c` against the files of
  the emulation.
- `portable_c-data`: the tables against the original's, by their names
  (`harness/datacheck.py`, with `harness/reads.py` for what is read
  behind an object's end).
- `portable_c-layout` and `portable_c-debug-layout`: where the linker has
  put things (`harness/layoutcheck.py`).
- The records, each a file of the harness that a change moves on purpose:
  `portable_c-host`, `portable_c-int16`, `portable_c-width`,
  `portable_c-words`, and `portable_c-cut` and `portable_c-tidy`, which
  have no record to write: see "The records".

`python3 harness/compare.py build/portable_c/VC.EXE NAME --first` takes a
build through one scenario and stops at its first screen that differs, a
few seconds; give it several names to run them side by side.
`python3 harness/show.py --program build/portable_c/VC.EXE` with steps
shows the screens of a build, `--setup` those of the original's setup
program. `harness/notice.py` tries a change on a copy of the tree
through its scenarios.

When a scenarios' test fails where the change at hand cannot explain it,
derive the goldens again before anything else. If `git diff` then shows
them changed, it is the machine that differs, QEMU or DOS, and not the
program.

### Goldens

- `python3 harness/goldens.py derive` derives every golden from the
  original side by side, in a minute and a half on four processors;
  `derive NAME` one, which is how a new scenario gets its golden.
  `git status` of `harness/goldens/` is then clean if a change to the
  harness has left the judging alone: it is cheap enough to do for every
  such change. A golden of the original that comes out changed is asked
  about ("Work by several agents").
- A new scenario is run for every tree, the frozen C translation
  included, unless it is declared not run for one: a scenario that the C
  translation cannot pass is asked about. The C translation is taken
  through no scenario of the setup program (`no_setup` of
  `harness/deviations.py`).
- A `.ran` next to a golden is what ran of `VC.COM`, or of `VCSETUP.COM`
  for a scenario of the setup program. The top of `harness/scenarios.py`
  says how a scenario is written, and "The scenarios of the setup
  program" there how one of setup is.

### A deviation through the harness

A deviation is declared in `src/portable_c/DEVIATIONS.md`, and then, for
the harness, in tables that each fail their test for an entry that no
longer applies:

- `TREES` of `harness/deviations.py`: the scenarios that are not run for
  the tree, each of which an entry of `DEVIATIONS.md` names as
  `` `NAME` ``; steps left out of every scenario or of one (`left_out`,
  `left_out_of`), steps taken in place of others (`in_place_of`,
  `in_place_in`, the latter by scenario for keys that mean other things
  elsewhere); and `rules`. A scenario written twice in one of these tables
  is the second entry alone, without a word from Python. `IGNORED` and
  `IGNORED_OF` have what the comparison leaves out, for every tree and
  for one.
- `DECLARED` of `harness/datacheck.py`: a table of the program that
  differs, as the places where it departs from the original's, with the
  bytes that the tree has there; the rest of it is still held. `OWN_OF`
  there has an object that only one tree has, `GONE` of
  `harness/reads.py` a constant that the tree no longer has.
- `GONE` of `harness/ported.py`: a function of a batch that the tree no
  longer has; `RENAMED` one that it has under another name, static ones
  included.

Then `python3 harness/compare.py build/portable_c/VC.EXE` says which
scenarios differ, which is the list to declare, each looked at. A
scenario whose steps choose what is gone needs the step left out, not a
patch of the keys going astray. Look also for a figure in a patch that
follows the layout of the program, an address that a program of a
scenario prints: it passes today and fails for the next agent.

### Patches

- `python3 harness/goldens.py patch --tree portable_c NAME` takes the
  build through the scenario and writes `harness/goldens/portable_c/
  NAME.patch`: every row that differs, the original's behind `-` and the
  build's behind `+`, printed with the cells marked. It is a patch when
  whoever declared the deviation has read every row against the entry of
  `DEVIATIONS.md` and written the entry's number and a sentence of why
  into its header; a header not filled in fails the test, and so does one
  that names an entry which does not name the scenario. A row that the
  declaration does not explain is a change that was not meant. The top of
  `harness/goldens.py` has the form.
- The test reads every patch before it runs anything. "does not fit ...
  Nothing was run" means the golden is otherwise than the patch has it,
  which says nothing of the program. "differs from the original with
  NAME.patch" means the patch fits and the program shows otherwise.
- When goldens are derived again on purpose, every patch on a row that
  came out otherwise does not fit, and is made again by the same command,
  which keeps the header and says how the new patch stands to the old:
  rows that expect the same in the same cells need no second reading, the
  others are read like a new patch. `git diff` of the directory is the
  reading list.
- A patch that is made again for a second deviation is read against the
  one before: `goldens.read(name, tree, patched=goldens.read_patch(file,
  tree))` with the old patch from `git show`, the new declaration made to
  that, and what differs is the reading list.
- A scenario where only steps are left out has no patch:
  `python3 harness/goldens.py changes --tree portable_c` lists those next
  to what each patch says, and which step the tree was taken through for
  each screen.
- The mouse pointer exchanges the colours of the cell that it is on, and
  setup puts it in the middle of the screen: a line that moves under it is
  in the patch for that cell.

### Rules

A rule of a tree is `rules` of its entry in `TREES`, made to every screen
of a golden when it is read for that tree, before its patch. The
program's name is said by `Part`s of `harness/deviations.py`: a text as
the original shows it and as the tree does; on a screen where at least
twelve cells of the original's text show in place, character and colour,
each such cell is expected as the tree's. Fewer is a patch.

- A rule is wrong of a cell where something covers the text with a space
  in the text's colour, or the text shows in another colour, under the
  pointer: both fail the row, which then goes into the scenario's patch.
  The rule is not to be taught the pointer's colour (it then wrote the
  banner into the box of F10). `shaded` lets a part be known in the shadow
  of a box, switched on for the top of the Info panel alone.
- A box whose width follows a text cannot be a rule: those are patches.
- On a row that a rule changes, the `-` line of a patch is what the rule
  made, not what the original shows.
- The test says how many screens each rule expects otherwise, and fails
  for a rule that finds none. After a change to `deviations.expected`,
  derive every golden again; a patch on a row that a changed rule now
  changes says "does not fit" and is made again.

### The scenarios of the program for Linux

- The program for Linux is judged by the scenarios driven as those of DOS
  are, with its own drives R: and H: in its tests, and held to the goldens
  of the original with masks: named elements of the screen that the
  comparison leaves out for that tree, each declared with its reason, found
  by a rule and not written as cells, every other cell compared. The masks
  are the path in a panel's title, on the prompt and on the command line;
  what DOS had shown behind the panels; the drive's letter and its label,
  but in a scenario about them; the time of a directory that the run has
  written into; and the place of the program's own files, only in the one
  scenario about them. An element left out is held whole in a scenario
  about it where one can be. They are `LINUX_MASKS` of
  `harness/deviations.py` and the classes of `masked` there.
- A scenario that rests on what only DOS has is not run on Linux, with its
  reason (`LINUX_NOT_RUN`); one that comes to such a thing part of the
  way, a device's name used as a device, a drive or a command that Linux
  lacks, an order of a directory that only DOS keeps, is cut there
  (`LINUX_CUT`) or has those steps left out (`LINUX_LEFT_OUT_OF`), each
  named by an entry of "The program for Linux" in `DEVIATIONS.md`.
- A scenario that went to the original's directory, `D:\VC`, or to the
  diskette in A: only for files to work on works in a directory of the
  bench's laid out alike on every platform, its golden derived again from
  the original.
- The program for Linux starts from a setup file that it saves itself,
  once for a bench, after its own menus have taken the panels from the
  original's file to H:\WORK and H:\, where the original's has C:.
- Told so by a variable of the environment, the program writes its grid of
  cells whenever it waits for a key; the harness drives it in a
  pseudo-terminal with the scenarios' keys, and what it draws is held to
  that grid by replaying it through pyte, pinned. While a test drives it,
  its clock starts at the bench's time, or at one that the test gives, and
  goes on only by the steps' waits and where the program waits on the
  timer itself, as the machine's clock goes on the bench. A user always
  has the real one.
- An expectation of its own, `harness/goldens/posix/`, has the header
  "Decided: CLAUDE.md, step 6", and is made only where a mask cannot do.
- A patch of `harness/goldens/posix/` is made as the test reads it, with
  the masks found on the golden with the patch made to it: on a row that
  the patch states, the host's drive letter, label and time of a written
  directory are written as the bench's, C:, WORK and its clock, so that
  the masks find them there and the web page, which falls back to these
  patches, is held alike (`unmasked` of `harness/goldens.py`). A
  scenario that is cut gets no disks in its patch, as the test holds
  none.

### The records

Each of these tests holds every file of the tree to a record, fewer as
well as more: the change shows in the commit. Write the record anew with
`--record` in the commit that moves it, read what moved, and after a
rebase write it anew rather than merge it.

- `portable_c-host`, `harness/hostcheck.py`: gcc reads the tree for a
  machine of 64 bits and counts, by kind, what would not hold there.
  `--show FILE` lists the places, `--macros` which macros make how many. A
  kind `other` is usually something of Open Watcom's that gcc cannot
  parse: find the first with `--show other`. The counts are gcc's, any
  version; clang counts otherwise and is not asked.
- `portable_c-int16`, `harness/int16.py`: clang finds where a number that
  an int of 16 bits and one of 32 may have otherwise is looked at before
  it is back in 16 bits; `harness/int16.txt` has every such place with
  the reason why it is the same in both. A new place gets the cast that
  says what DOS does, `(u16)(a - b) < c`, or a reason written by hand
  under its line after `--record`. A place of the kind `minus` is a number
  written below 0 and kept in 16 bits; a loop's counter is known only in a
  plain `for`. The finder holds itself against `harness/int16check.c`
  first.
- `portable_c-width`, `harness/width.py`: every number of 8 and 16 bits
  that a file declares, with a reason for each in `harness/width.txt` from
  a fixed list of words (`WORDS` there); a file that is sorted is held to
  clang's `-Wconversion`, `-Wsign-compare` and the warnings of booleans.
  `--warned FILE` prints what clang says of a file, `--show NAME` the
  calls that hand a function more than it takes. What the finder does not
  name and a widening can get wrong, read at every line that names the
  variable: a mask written for 16 bits, `length & 0xFF00`, which is
  `length & ~(u32)0xFF` in 32; a `~` or a `-` of it; a comparison with
  0FFFFh for a number that got there by stepping 0 back; a number that is
  only ever 0 or 1. A function widened moves the cuts of other files, and
  a file may declare another's function for itself: `grep` the name.
- `portable_c-words`, `harness/words.py`: how often each file writes each
  word of the addresses of `vc.h`.
- `portable_c-cut` holds the tree to having no `(u16)(signed char)` lost
  by `wcc386` (see "Notes"), and the compiler to `harness/cutcheck.c`:
  before a cast is relied on in a use that the file has not, a function is
  added there and its instructions read. A failure in a function called
  Lost means the compiler has changed.
- `portable_c-tidy`, `harness/tidycheck.py`: clang's warnings and
  clang-tidy over the program for Linux and the emulation; `EXPECTED`
  there is empty, and a place that the analyser names is mended, not
  listed. `python3 harness/tidycheck.py --compiled
  build/portable_c/posix/compiled.txt` runs it by itself.

### The figures

The figures of the portable C that change with the work, the length of
its file, what it has of the memory, the counts of what only Open Watcom
takes, are in one place: the table of `docs/portable-c.md` under "The figures",
which `python3 tools/figures.py --write` writes of the build and the
source. Run it when a piece of work lands, after the build and the
rebase, and commit `docs/portable-c.md` with the work; `--check` says whether the
table is the build's. Write no such figure anywhere else: say the thing
without the number, or point to the table. A new figure is a row of the
table, added in the script. A figure that is history stays where it tells
that, and a commit message keeps its own.

### The data image

The bytes of the program say little of a change that is to leave it
doing what it did. What says it is the data's image: what the program has
in its main data, in the memory that it was handed and in its variables
at the end of every scenario.

- Before the first change of a piece, `python3 harness/dataimage.py save
  build/portable_c/VC.EXE build/before`, or `cp -r
  build/portable_c/images build/before` when the scenarios' test has just
  run on the build before the change (it writes the images of what it
  judges there). Then `python3 harness/dataimage.py compare build/before
  build/portable_c/images`, and its last lines go into the commit.
- It names what differs, by field and variable; read each. What a piece
  means to change it names with `--changed`: declare a word or a count,
  not a buffer, since what is declared is not looked at. A word that
  leaves the main data for a pointer gets the pointer's symbol in
  `ADDRESSES`.
- What follows the time differs with no fault, since two builds get to a
  step by different counts of instructions: `UNSTEADY` has those, with
  their reasons. Before another is put there, `save --early` of the same
  build says whether it follows the time. The same build gives the same
  image in every run; one that does not has met a flaw of the bench.
- A piece that changes where things lie in the memory is held against a
  third build, not kept: the source of before with the few lines that lay
  the memory out as the piece does.
- A word of a buffer of the main data that leaves it for a pointer may
  still be carried along by a copy by count: `DialBox` copies 81 bytes of
  a box's text whatever its length. List the texts within 81 bytes in
  front of the word before it goes.

### A copy of a build

A build can be tested while the tree goes on changing: copy
`build/portable_c/VC.EXE`, `VC.MAP` and the object files into
`build/copy/portable_c` (the harness knows the tree by the directory's
name, and holds a build of any other name to the original's goldens with
nothing declared), and run `harness/datacheck.py` (at once: it reads the
source), `harness/layoutcheck.py` and `harness/ported.py` on the copy as
their tests in `CMakeLists.txt` run them. The program of before kept
beside the build as `build/NAME/portable_c/VC.EXE` with its `VC.MAP` is
taken for the same tree, and `harness/show.py --program` shows the two
side by side. A tool run while the build changes reads the map again at
every scenario, so "before" is saved from such a copy.

## Notes on the portable C

What the work has learnt that still bites. What was learnt on the way to
here, the 16-bit build, the move to 32 bits and the sweeps that are done,
is in `DIARY.md`.

### Open Watcom's `wcc386`, for the program for DOS

- A declaration in a header can change the program where nothing uses it:
  of the variables without a value the compiler lays out in an order that
  goes by how many names were declared before them. Typedefs and
  prototypes count; a tag of a structure and a macro do not, which is why
  `HOLDS` of `vc.h` is a tag. A change that is to leave the program as it
  was and declares something may move such variables: hold the listings,
  `build/portable_c/*.lst`, against those of the build before; a
  difference in the list of `_BSS` alone is this.
- The objects of two builds differ in every file, by what the compiler
  writes besides the code: hold listings against each other, or the
  places of functions in the two maps. The program's file holds its own
  name; under one name two links of the same objects are the same bytes,
  so `sha256sum build/portable_c/VC.EXE` before and after checks a change
  that is to be no code.
- `-os` gives a function no instructions of its own where the function
  behind it has the same: the map has several names at one place then.
  `-od` gives each its own. It also puts pieces of a function in front of
  its label: the listing says whose an address is, not the map.
- `wcc386 -os` loses a cut to 16 bits between a signed byte and a number
  of 32 bits: `u16 line = (u16)(signed char)c;` is handed on with the sign
  in all 32 bits. The byte is taken into an `i32` where it is reckoned
  with, or with its sign into an int where that is meant, never through a
  word of 16 bits without a sign; `vc.h` says so at "A byte with a sign
  made a word", and `portable_c-cut` holds it. A fault of the compiler is
  pinned down fastest by running its code on this computer: functions
  compiled with the build's options, a driver compiled with `-bt=linux`,
  linked by `wlink system linux`, the output held against gcc's.
- `NO_RETURN` of `compiler.h` is `__declspec(aborts)`. `_Noreturn` and
  `__declspec(noreturn)` are taken without a word and are not the same:
  the caller goes on behind the call.
- A structure of more than four bytes comes back through a pointer to
  room in the caller's frame, a room for each call: a function that calls
  fifty that return one has a large frame (the `enter` at its first line
  in the listing).
- Nothing promises that what has no value is zeros when the program is
  entered: a variable of `sys_dos.c` has a value, `= 0`.
- The order of the segments is the linker's command's, by classes, and a
  class is linked whole; within a segment the objects are in the order in
  which the linker is given them, `sys_dos.obj` last.
- `wcc386` writes `NAME.err` into the directory it is run in when a file
  has a warning: run a trial from `build/`. `build/portable_c/compiled.txt`
  has the command that the build compiles with; the build compiles again
  when it changes. `wdis -s=FILE` puts the source into a listing of an
  object compiled with `-d2`.
- A name that `vc.h` takes is taken in every file that includes it: a
  macro of the same name in a file stops the build (W140).
- A routine that the extender enters from real mode is a
  `__declspec(naked)` function with `_asm` in `sys_dos.c`; the function
  that it calls has a `#pragma aux` with its registers. Instructions of
  real mode are bytes in a table there, each with its instruction in a
  comment; `ndisasm -b16` reads them back, and the bench holds them
  (`harness/trial32/ended.asm`, the scenario `ended`).

### The bench, QEMU and DOS/32A

- The machine's clock is counted in the instructions it executes. While
  the program for DOS idles, a second of the machine costs the host four
  times what it costs for the C translation, for the extender's changes of
  mode: measure such things counted, in instructions, not by the host's
  time. The time that a step has before the program counts as stuck is of
  the host's clock and the machine's, both.
- QEMU 11.1 reads through `cs:` with 32-bit addressing at the wrong place
  in a code segment whose base is not 0: CauseWay stops at its first
  interrupt for it, which is why the extender is DOS/32A. `-d int` shows
  it. A program that fails where nothing explains it and runs with
  `-d nochain` has met a fault of QEMU's like that.
- The extender takes all of the memory above the megabyte while the
  program runs. It keeps 16 bytes of its own in front of every block and
  looks at them all at every call for memory: a write behind the end of a
  block ends the program at the next one with "DOS/32A run-time (6004):
  extended memory blocks have been corrupted". A block is given with what
  the memory held. The program's own pattern of 16 bytes round each block
  is looked at when a scenario ends.
- The extender is entered on the program's stack at every interrupt that
  the program makes, and keeps up to 190 bytes there below the stack
  pointer while a call is under way. It puts a selector into the PSP at
  2Ch, which DOS reads as a segment: `Run` of `sys_dos.c` names the
  environment's segment. Where DOS writes into a buffer of the program's
  in real mode, the bytes go down to the block below the megabyte first
  and come up afterwards.
- As it leaves, the extender puts the interrupts of real mode back as
  they were, and INT 21h of real mode is its own while the program runs.
  When DOS ends the program by another's hand, the extender takes itself
  out. What the extender does is read in its file, `binw/dos32a.exe`, by
  `ndisasm -b16` into `build/`, never committed.
- DOS loads a program into the largest block it has. A second copy of the
  program started from the first is loaded behind it, where `copies` of
  `harness/le.py` works it out; the driver tells which copy idles by the
  programs of that name in DOS's memory.
- When the program leaves by itself, QEMU ends with the script half
  played: a check of what the driver wrote looks once more after the
  machine has ended. A scenario that goes on at DOS's prompt after the
  program ends has `"cls" Enter` as its last command.
- `BREAK` off and Ctrl-Break at DOS's prompt end no step of a script: it
  looks like a machine that has stopped and is none.
- A bench's state is loaded only into a machine of the memory that it was
  saved of: a scenario that asks for less, `memory` of
  `harness/scenarios.py`, gets a bench of its own.
- The disks of the machines are in the checkout's `build/tmp`
  (`workdir` of `harness/dos.py`), since `/tmp` is in the memory and many
  runs filled it. QEMU's socket is there, and its path must be under 108
  characters; a `TMPDIR` that is set is used as it is.
- What the original reads of memory that it put nothing into shows what
  was there on the bench; the translation has zeros there, and the
  scenario says in a comment that the original's screen is of that kind.
- `harness/trial32/` has toys and scripts that ran on the bench while the
  program went to 32 bits, each with the command that runs it in its
  comments: `run.py` starts a machine for a file, `second.c` keeps all but
  some of the extender's memory, `calls.asm` writes what a program asks of
  DOS, `asked.py` runs the toy `sys.c` that asks every function of
  `sys.h`. No test builds that toy.

### The setup program

- The original is two programs to the harness. A scenario of the setup
  program has `setup=True`. Where the main program has a routine or an
  object of the same bytes, setup uses the main program's: what a
  scenario of setup shows otherwise is an `if` on `StOpen`, what nothing
  of setup can show is left out, and a bug of the copy is asked about.
  `harness/vcsetup/` holds the setup program's assembly against the main
  program's.
- Two programs of the original in one machine have routines of the same
  bytes: `coverage.executed` places a program only by blocks that the
  other's file has not, `others`; whatever runs a second program next to
  the one under test says so there.
- The test of the data knows a constant of the setup program by `St` in
  front of its name; a word that names a thing of setup is the tree's
  `StX`, the main program's `X` where that is the same bytes, or a field of
  the main data. `SETUP_NAMED_OTHERWISE` and `SETUP_COUNTED` of
  `harness/datacheck.py` are its tables, and `SETUP_FILES` of
  `harness/reads.py`.
- What setup keeps is in the main data, `struct StData` laid over
  `MenuBuf`, and nothing there is counted on across a command.

### Taking a thing out, or changing a table

- A feature is found by the byte that the main data keeps of it, `grep -a`
  over `src/asm` with all of its output, not by its routine. Read what a
  buffer or a routine of the thing is used for elsewhere before it goes,
  and `grep` the names of the scenarios that go too, in the harness and in
  `DEVIATIONS.md`.
- A line taken out of a menu moves every item below it, and an item is
  known by its number in more places than the tables: the dark items of
  `F39_58` of `menu.c`, a command handed its item's number, a click of a
  scenario by place. A field taken out of a dialog renumbers the fields
  behind it and their links (`LINK_` of `dialog.c`): write down each link
  by the original's rule. A box of two groups side by side has one text a
  row, both halves in it. A dialog a row shorter shows what is behind it,
  a patch for every scenario that shows it.
- An object that is copied by a count takes what follows it with it: the
  test of the data compares as many bytes as an object has, and
  `harness/reads.py` lists every place where code names a constant, with
  the bytes read; a place of no known form is in `REVIEWED` there with how
  it is known.
- What the data test's messages mean, and its tables `ELSEWHERE`, `OWN`,
  `NAMED_OTHERWISE`, `DIFFERENT`, `DECLARED`, are said at its top. A word
  of a table that names something is a pointer of four bytes, so boxes and
  dialogs are held field by field; an object with pointers declared
  otherwise than by `BOX`, `DIALOG` or an array of pointers wants
  `SHAPES`. A box cannot be declared before it is defined; buttons can.
- A function of a batch that no scenario runs any more fails the
  scenarios' test: it is in `GONE` of `harness/ported.py`, or a scenario
  reaches what is left of it.
- A register that the original leaves for the next call, or a place it
  leaves in a register that a later command reads as a number, is read in
  the assembly before it goes: `MD_PLACE` keeps such a place a number,
  with its reason.

### Moving a file to the system's functions

Every file asks the system through `sys.h`; none hands a structure of
registers to an interrupt, and `python3 tools/calls.py` counts none in the
portable C. When a call of the system is written anew or brought over
from the C translation:

- Find the service, the value last put in AX, and read on from the call
  to where each register is next written: every register read on the way
  is what the call has to hand back or the next call has to be handed.
- AX is `value`, also where the call did not fail, and `failed` is
  whether DOS set the carry flag. A register that a call answers in is a
  member with a name; if the call failed it is 0, where the original went
  on with what was there: where a line reads it without looking at
  `failed`, read the assembly for what it held, and write that down.
- A register that the service does not take was whatever the code before
  left there; the functions hand over what is asked and nothing else.
  That is declared in entry 6 of `DEVIATIONS.md`, with a row for every
  leftover dropped, read in the assembly, in the order of the files. Four
  leftovers are read by something, and the entry says what the functions
  do about each; one that you are not sure of is asked about. Where a DOS
  leaves in AX what it was handed, and a caller reads it, the leftover is
  behaviour: ask.
- A function that the interface lacks is declared in `sys.h`, in the group
  of its kind, with a comment that says what is asked, what it answers and
  what it answers when it fails, in DOS's values and with no register in
  it; and written in every implementation, `sys_dos.c` and `sys_posix.c`
  and the web page's, where the comment at the head of its group says the
  service. If it takes a name, it goes into `NAMED` of `harness/reads.py`.
  `docs/portable-c.md` counts the functions by group under "The system
  in one place"; a table of `sys_dos.c` alone is in `OWN_OF` of
  `harness/datacheck.py` and in `REVIEWED` of `harness/reads.py`; the toy
  `harness/trial32/sys.c` asks every function; and the host check's
  record moves.
- What a system does when it has ended the program cannot be a function of
  the program's, since the program is gone then: the program says
  beforehand what is to be put back, a value, and each implementation puts
  it back its own way (`ProgramEndedBreakCheck`). A thing that the system
  enters of the program, as the critical error, is two things: where it
  arrives, the system's, and what the program does about it, a function of
  a type that `sys.h` names, handed over where the program takes the
  thing.

## Work by several agents

Work that can be divided is done by several agents at a time, without
waiting for the user between its steps.

- Work is run side by side wherever it can be, within what the machine
  takes: a piece that need not wait for another starts at once, in a
  checkout of its own, and a plan or a reading that is cheap goes beside
  the heavy work.
- Each piece is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not do
  the work: it starts each agent, checks what the agent left, which is the
  commits, the push and `LastTest.log`, and keeps its report. Nor does it
  run or read what prints much: the suite on `main`, a search through the
  documents, a small correction of a text are an agent's too, and the
  session keeps the conclusion. Its context is for the queue and for the
  user.
- An agent is given no time limit for work that is going well. The
  session watches what its agents are doing, turns one round that is
  spending its time on something of little use, and looks for what an
  agent has left running.
- An agent knows only what the repository tells it. What one learns that
  the next needs is written down before its work is committed: a rule or a
  how-to here, what is to do in `TODO.md`, what happened in `DIARY.md`, and
  what is about scenarios at the top of `harness/scenarios.py`.
- Agents that work at the same time each have a checkout of their own, a
  plain clone of the same origin with its own build, and a part of the
  program that the others leave alone. The session names both. Temporary
  files go in the checkout's `build/`, not in a scratchpad that agents
  share.
- Work gets onto `main` one piece at a time, in a queue that the session
  runs: an agent commits in its checkout when the tests pass and says that
  it is ready; at its turn it does `git pull --rebase`, settles what the
  others changed keeping what both sides added, builds, passes the tests
  once more, and pushes. Nothing else lands meanwhile. After a rebase, look
  at everything the others added that names what is yours: two agents
  have made the same thing under one name, and git saw no conflict. A
  count that both sides changed may merge without a word and be wrong.
- An agent whose work changes only `src/portable_c/`, or what is the
  portable C's alone in the harness, runs the tests of the portable C,
  `ctest --test-dir build -R portable_c -j4`, and not those of the frozen
  trees, which cannot change by it. A change to what the trees share, the
  harness, the build or a tool, wants the whole suite. The session runs the
  whole suite on `main` itself, after what has landed. The build may not
  change while the scenarios' tests run; the tests that read the source
  are done in their first seconds.
- Nothing in the harness takes turns: recordings of the original and tests
  run at the same time, in as many checkouts as there are, and other
  machines only make them slower. Where a recording or a test does come out
  different under load, that is a flaw to report, not something to wait
  out.
- The tools and the harness are improved as the work goes, without waiting
  for confirmation, each improvement a commit of its own. What decides is
  the total cost: an improvement is made when it saves more time over the
  work still to come than it takes to make, counting time that has
  actually been lost, not time one might lose.
- Stop and ask, with the work at hand left unpushed, before any of these.
  An agent that meets one hands its work back as it is, and the session
  asks the user:
  - in the C translation, behaviour that differs from the original's;
  - in the portable C, a deviation that is not the removal of something
    that only made sense on DOS;
  - a change to what the comparison leaves out, other than a figure of the
    kind that "Scenarios" has left out, or to a limit that a test holds the
    build to;
  - a golden of the original that comes out changed when it is derived
    again;
  - anything new to download;
  - a change to these instructions, other than a note on what was learned.

## Scenarios

- A scenario starts from the program as its distribution configures it, on
  a machine of its own, and sets up whatever else it needs through the
  program's own menus. It does not rely on prepared configuration files.
- The test bench changes three things, and gives every scenario the setup
  file that the original saves after making them in its own menus: the
  clock is off; the panel on the right lists files, where the distribution
  has the Info panel there (a scenario that is about the Info panel
  switches it on); and the panels list files sorted by name. The bench
  makes its directories and files in the order of their names, so that
  the disk's order is the names' on DOS too and the tree, which lists
  directories as they stand, is alphabetical. Steps move the cursor by
  places in a sorted list.
- A scenario that shows the time of day, with the clock switched on or in
  the time stamp of a file it has made, does so within the first minute of
  its machine. The machine's clock is counted in the instructions it
  executes, and a C tree takes more of them than the original to get to
  the same step: a tenth to a third more as a rule, and twice as many where
  a step searches a text (the top of `harness/scenarios.py` has the
  figures). The first screen is at about five seconds past the minute. So
  such a scenario is tried in every tree that it is run for, and is done
  showing the time by about fifty seconds past in the slowest of them. A
  scenario that switches the Info panel on takes some five seconds more,
  once, when the panel is first drawn.
- A figure on the screen that follows the time of day or the size of the
  program, such as the clock or a count of free memory, is left out of the
  comparison, in `harness/deviations.py`. The exception is a scenario that
  is made to test that figure.
- The countries tested are the United States and Russia.
- Russian text is typed on the Russian keyboard of the Russian bench. Its
  Cyrillic letters are selected in a step that types text into an input
  line, never at the panels: there Alt by itself brings up the menu.
- The tests run the build of a tree that is shipped; no build is made for
  them. The original and the programs for DOS are driven alike, by a
  program of ours that stays resident in the machine, `harness/DRIVER.ASM`:
  it puts the keys of a step in through the keyboard controller, so that
  the BIOS makes of them what it makes of a keyboard's, and writes the
  screen out when the program idles with none left. It watches and feeds,
  and answers nothing in the program's place or the BIOS's. It knows the
  program's idling by the program's call of INT 28h. The program for Linux
  is driven through its own test channel, as "The scenarios of the program
  for Linux" has it.
- A test does not put a stand-in where something is that the project does
  not own: DOS, the BIOS, the machine. What a test knows of them it gets
  from the real ones. A part of the program itself may be replaced in a
  test build, since its source says what it is supposed to do.
