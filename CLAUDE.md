# Instructions for Claude

Standing instructions for working in this repository. Record new ones here,
not in Claude's memory: memory does not travel between computers.

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
  them: see "The portable C".
- Naming such tools is fine. Documentation should say plainly what the
  original was built with and where a reference came from; the restriction
  is on fetching and running them, not on mentioning them.
- Anything the work downloads is fetched at build or run time from its
  official source and pinned, by SHA-256 (see `harness/deps.py`) or by git
  commit (see `CMakeLists.txt`).

## Naming

- The project is called Vibe Commander. The name of the checkout directory
  or of the remote may be temporary; never write either into source,
  documentation or commit messages.

## Committing and pushing

- Show the result of a piece of work, then wait for confirmation before
  committing it. Work that agents do in a run is the exception: see "Work by
  several agents".
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
  it against the binaries its author shipped.
- `src/c/` is that program translated to C for DOS, the C translation: it
  behaves as the original does, bugs included, and every scenario holds
  it against the original. It is finished and frozen. It changes only to
  mend a place where it is found not to behave as the original, and then
  toward the original.
- `src/portable_c/` is the portable C: the program on its way from DOS to
  modern operating systems. It began as a copy of `src/c/`. This is where
  work goes on.

In prose the first is the assembly or the original source, the second the
C translation, the third the portable C. No tree is called "the port".
The tests are named by the tree they judge: `asm-bytes`, `c-data`,
`c-arith`, `c-scenarios`, and those of the portable C with `portable_c-`
in front.

## The C translation

These held while `src/c/` was made, and they say what it is.

- Fidelity means the same behaviour, bugs included. A bug of the original
  was ported as it is and reported.
- Its routines and data keep the names they have in the assembly.
- It has no assembly file: its code and its data are C. Where C could be
  written in more than one way, portable C came before C that is tied to
  this compiler or this machine, that before the compiler's intrinsics,
  and those before assembly, which is written inline in a C file and only
  where nothing else works. That a thing would be smaller or faster was no
  reason to go down that list. The README lists what rests on Open Watcom
  and every piece of inline assembly.
- The mouse is tested like the keys: the bench has one, and what the
  program does with it is held against the original in scenarios. An
  instruction that no scenario can reach is listed as out of reach with
  its reason.
- It is larger than the original and uses more memory, the part that
  stays in memory while a command runs included. No effort went into
  making it small.
- How it was made, a batch of routines at a time, and what the batches
  learnt of the assembly, of Open Watcom's 16-bit compiler and of QEMU,
  is in `DIARY.md`, under the days of the translation. Read it there
  before mending the C translation.

## The portable C

- The translation of the program is complete and verified, on the user's
  word of 12 October 2026, so work for other operating systems may go in.
  It goes into `src/portable_c/` and nowhere else.
- The road, on the user's word of 12 October 2026, in this order:
  0. The setup program, `VCSETUP`, is ported into the portable C, so that
     there is one program: setup is reached by an item of its Options
     menu, and is judged by scenarios whose goldens are recorded from
     the original setup program. The call by which the setup program
     asked a running copy for its data goes then, with the last of the
     watch on DOS. This comes before the 32 bits.
  1. The portable C becomes a 32-bit program for DOS, built by Open
     Watcom's 32-bit compiler with a DOS extender, and judged on the same
     bench by the same scenarios. When it passes them all, its 16-bit
     build goes: `src/c/` is the 16-bit program. The extender is
     DOS/32A, the one that runs on the bench: its stub is linked from the
     pinned Open Watcom download when the program is built and never
     committed, its licence text is kept in the repository, and the
     README says, as that licence asks, "This product uses DOS/32
     Advanced DOS Extender technology." This step follows the setup
     program without waiting for the user; the 16-bit build is not
     deleted before the user has said so. The user said so on 13
     October 2026, with the last two differences of the program of 32
     bits declared: the tests then judge that program as the portable
     C, and the 16-bit build goes with the switch, in one piece. That
     is done, the same day: the portable C is the program of 32 bits
     and has no other build, and steps 0 and 1 are behind it.
  2. It uses what 32 bits give: one flat memory, with pointers where
     there were segments and offsets, and numbers and values of 32 bits.
     A variable is widened wherever wrapping round at 16 bits would be
     unexpected behaviour, and stays 16 bits only where the program
     counts on the wrapping. A number that is only ever true or false
     becomes a boolean. On the user's word of 13 October 2026 the
     memory for the panels and the texts comes from the extender from
     this step on, as much as the machine has, where step 1 had 640 KB
     of the program's own: what a user then sees otherwise, an editor
     that opens whatever fits, is declared. The widening is of this
     step and does not wait for the layer: the pointers first, then
     the booleans and the widths together, with the reason written
     down for every number that stays 16 bits. A limit
     of the original that goes when a number is widened is a
     deviation, and each is asked as the work comes to it. For an hour
     of that day the widening was put behind the layer's design: the
     reasons did not hold, since most of what is widened never crosses
     to the system, and the memory from the extender makes the limits
     of 16 bits reachable on DOS.
  3. The abstraction layer is DOS, on the user's word of 14 October
     2026. The program goes on asking the system what it asks DOS and
     the BIOS, as `sys.h` has it: functions named for DOS's services,
     in DOS's terms and values (drive letters, names of 8.3, paths of
     67 characters, the code page, the BIOS's key words, cells of the
     screen, packed dates, DOS's numbers of errors). Every other system
     emulates DOS behind those functions as well as it can, as DOSBox
     does but at the level of the services and not of the processor or
     the interrupts: a drive letter is a directory of the system's, by
     a setting, as DOSBox's `mount`; a name longer than 8.3 has an
     alias of DOS's form, `LONGNA~1`, that stays the same and leads back
     to it; names are converted between UTF-8 and the code page where
     they cross; the screen is a grid of cells drawn in a terminal; a
     key is made a word of the BIOS; a command runs through `$SHELL
     -c`. A design from the user's screens and what they are used for,
     made anew for modern systems, is the right way for a redesign and
     was set aside as more than is wanted now: its surveys and what was
     decided in that direction on 13 and 14 October 2026 are in git's
     history only, and none of those decisions is on the road but
     those that the user named again; what the surveys found of Linux
     is in `TODO.md`, "What the surveys of the layer found that step 6
     needs". That the program saves its preferences and its session by
     itself, on every platform, DOS included, decided on 14 October
     2026 with the rule of the harness for it (`TODO.md`, "The program
     saves its setup and its session by itself"), is of the program and
     not of the layer, and stands until the user says otherwise; the
     setup file keeps the original's format, which works wherever DOS
     is emulated.
  4. The implementation for DOS is `sys_dos.c` (which was `dos32.c`)
     behind `sys.h`, as it is.
  5. A second implementation is made that builds and is not meant to
     run: a stub. Done on 14 October 2026, `sys_stub.c` with the test
     `portable_c-stub`; on the user's word of that day it goes once
     `sys_posix.c` is done, its test's checks moving to the build for
     Linux. It went on 15 October 2026, with Linux done, and those
     checks are the test `portable_c-tidy`.
  6. The implementation for Linux, of 64 bits, built with clang, a
     program that runs in a terminal, emulating DOS as step 3 says;
     whatever of the x86 is still in the common code goes behind
     `sys.h`, as Chromium has it: files `_dos.c` and `_posix.c` beside
     common headers, and macros that each platform defines in its own
     way. What the user decided of steps 5 and 6 on 14 October 2026:
     one `CMakeLists.txt` builds every implementation, and finds clang
     for those that want it; the files are `sys_dos.c` (which
     `dos32.c` becomes), `stack_dos.c` (`stack.c`), `sys_stub.c` (gone
     since) and `sys_posix.c`. On Linux there are two drives, R:, the root of the
     file system, labelled ROOT, and H:, the user's home directory,
     labelled HOME, always, the program starting on H: where it is
     started under the home directory and on R: otherwise (the user,
     on 15 October 2026, in place of one drive C: at the root, as a
     way round DOS's 67 characters of a path, which stay: a plan to
     let paths grow on Linux was made and dropped as deeper than
     wanted); a directory whose path does not fit even so is reached
     through the aliases, or not, the program starting then in the
     deepest one above it that fits; the code page, and the
     country with it, is 437, always; nothing of it is a setting, for
     now (the user, later the same day, taking back a file of settings
     and the code page by `LANG`). The terminal is driven by the program's own escape
     sequences and termios, no library, with the kitty protocol of
     the keyboard where the terminal offers it. A command runs on the
     terminal's own screen, which Ctrl-O shows, as on DOS. The program
     for Linux is judged by scenarios driven as those of DOS are, with
     its own drives in its tests too, and held to the
     goldens of the original with masks: named elements of the screen
     that the comparison leaves out for that tree, each declared with
     its reason (the path in a panel's title, on the prompt and the
     command line, what DOS had shown behind the panels), found by a
     rule and not written as cells, every other cell compared, and an
     element left out held whole in a scenario about it where one can
     be; an expectation of the tree's own only where a mask cannot do
     (the user, later the same day, in place of expectations of its
     own; the masks are made with the harness for Linux; on 15 October
     2026 the user named among them the drive's letter and its label,
     but in a scenario about them, and the time of a directory that the
     run has written into; and while a test drives the program for
     Linux its clock starts at the bench's time and goes on only by the
     steps' waits and where the program waits on the timer itself, as
     the machine's clock goes by what it runs on the bench, a user
     always having the real one; and, the same day: a scenario
     that went to the original's directory, `D:\VC`, or to the
     diskette in A: only for files to work on works in a directory of
     the bench's laid out alike on every platform instead, its golden
     derived again from the original, and only one about the program's
     own files has their place masked; the Linux program starts
     from a setup file that it saves itself, once for a bench, after
     its own menus have taken the panels from the original's file to
     H:\WORK and H:\, where the original's has C:; a
     scenario that rests on what only DOS has is not run on Linux,
     with its reason, as a tree's scenarios are declared not run; and
     the tests run the build with the sanitizers that go together, the sanitizer of memory a build of its own, run once and
     then kept for audits, not by every test (the user, the same day)): told so by a variable of the environment, it
     writes its grid of cells whenever it waits for a key, a harness
     drives it in a pseudo-terminal with the scenarios' keys, and what
     it draws in the terminal is held to that grid by replaying it
     through pyte, a terminal emulator in Python, pinned, which is
     the download that this allows; a test may give it the clock to
     start from. Of what the abandoned design had decided for other
     systems, the user named three again for Linux on 14 October 2026:
     Enter on a file that the program cannot run hands it to
     `xdg-open`, the program asking `sys.h` whether a file runs where it
     asked its extension, so that on Linux a file that the user may
     execute runs whatever its name, the panel's filter of programs
     showing then what Enter would run (the user, on 15 October 2026),
     and a name of DOS's form in a
     command, a word that names a file of the current directory, is
     handed to the shell as the host's name (the user, on 15 October
     2026); the PC's sixteen colours are drawn as DOS has them,
     exactly, in 24 bits where the terminal allows it; and the screen
     is 80 columns by 25 rows and no other size, whatever the
     terminal's. The rest of that design is dropped. Of what the first
     piece of step 6 asked, decided the same day: the program for Linux
     is called `vc`, and keeps its setup file, the original's `VC.INI`,
     at `~/.config/vc/vc.ini` (`$XDG_CONFIG_HOME/vc/vc.ini` where that
     is set); a character that the code page
     lacks, or a byte that is no UTF-8, shows as `_`, as Windows makes
     a short name, the alias leading back to the name as it is; what
     the program makes on the host is named in lower case; aliases are
     numbered in the order of the host's names by their bytes and kept
     for the run; and a directory is never shown read-only. Of what
     the second piece asked, the same day: a file of four gigabytes or
     more shows the most that DOS can say, and is never copied or moved
     cut short, the program refusing it with an error instead; the free
     room of a disk shows at most four gigabytes less 64K, in clusters
     of up to 64K; a file whose name begins with a dot may be
     overwritten; hidden, system and archive are taken when set and not
     kept, the host having nowhere for them; a pipe, a socket or a
     device is listed as an empty file that cannot be opened; the
     drives' labels are ROOT and HOME, and making one is refused; DOS's names of devices, NUL, CON and the like, are
     ordinary names of files on Linux, and the steps of scenarios that
     use them as devices are not run there, declared (the user, on 15
     October 2026); and an error of the
     host's with no number of DOS's is DOS's general failure. Once the stub compiles on Linux, on the user's
     word of 14 October 2026, every build with clang has the
     sanitizers that fit it, a high level of warnings and clang-tidy,
     and a warning of any of them stops the build; which warnings and
     which checks of clang-tidy is recommended to the user from what
     popular projects of open source use, and the user decides. The
     user decided it the same day, from a survey of curl, systemd,
     git, PostgreSQL, libuv, OpenSSL, LLVM and the guide of the
     OpenSSF: a list of warnings of clang's, written out, close to
     curl's, and not `-Weverything`, each an error; clang-tidy with a
     broad set, the analyser, `bugprone-*` and `misc-*` and a few of
     `readability-*`, where each check that is turned off has its
     reason in `.clang-tidy`; a build with the sanitizers of
     addresses and of undefined behaviour, and of integers and of
     implicit conversions, all fatal, and one with the sanitizer of
     memory, apart; the usual hardening in the build that is not
     sanitized; and a fall-through of a `switch` and a table of bytes
     without its zero said by words of `compiler.h`, not by turning
     the warnings off. Where the analyser names the program reading
     the main data as one block across its fields, as the original
     does, that read goes through the main data as bytes at an offset,
     by a piece of its own, so that nothing is turned off for it, on
     the user's word of the same day. A word of 16 or 32 bits read from or
     written into a buffer of bytes, which the program does at an
     address that may be odd as the original does, is made of its
     bytes, `p[0] | p[1] << 8`, by words of `vc.h` (`GET16`, `PUT16`,
     `GET32`, `PUT32`), not read through a cast of the pointer, so that
     the sanitizer's check of alignment stays on, fatal, and nothing
     rests on the order of a processor's bytes, on the user's word of
     15 October 2026. The room of the editor and the viewer on Linux
     is as much as the machine has; a cap comes with iOS, where the
     system ends a program that takes too much.
  7. A static web page, when Linux is done, on the user's word of 15
     October 2026: the program built to WebAssembly by Emscripten,
     which is the download that this allows, fetched and pinned as any
     other, its key loop made to wait by Emscripten's Asyncify so that
     any static host serves it; behind `sys.h` a file of its own; the
     screen drawn as text, the 80 by 25 cells as lines of a `<pre>`
     with the colours of DOS in spans, not on a canvas. It comes before
     iOS as a rehearsal of what iOS has too: no shell, a file system of
     the program's own, an event loop. What the user decided of its
     plan (`TODO.md`, "Step 7, the web page"), the same day: one
     drive, C:, which starts as a small tree of sample files written
     for the project and served with the page, nothing proprietary,
     and which the user may change, what is changed being kept in the
     browser; a command says on DOS's screen that commands cannot be
     run in a web page; Enter on a file that is no program opens it in
     a new tab of the browser, where the user may view it or save it;
     the user's own files come in by a button and by dragging them onto
     the page, into the current directory; the keys that a browser
     keeps for itself are declared, and a button for the full screen
     locks the keyboard where the browser allows it; the font is
     VileR's Web437 IBM VGA 8x16 (CC BY-SA 4.0), pinned and put into
     the page, and credited there; Emscripten is fetched only for a
     build that asks for the web page, and Open Watcom likewise only
     for one that asks for DOS, where that makes sense.
  8. iOS, when the web page is done. Windows and macOS are not targets
     for now, on the user's word of 14 October 2026.
- The program in the portable C is Vibe Commander 0.1, on the user's word
  of 13 October 2026, since it no longer is what the original is. Its
  title and the first line of its banner are "Vibe Commander 0.1", the
  short form, and where the original names itself without a version it
  is "Vibe Commander". It says "based on Volkov Commander 4.05", in the
  same short form, wherever the original named its version: behind the
  name in the banner, and in the Info panel on a line of its own
  between the title and the line under it, where the two lines on the
  memory, which mean nothing off DOS, are gone. The banner's second
  line keeps the author's copyright and adds the user's, "(C) 2026
  Maksym Motorny". No line is added anywhere for it. The C translation
  keeps the original's name.
- The jumps that the translation kept from the assembly, `goto`, go
  where they are easy, on the user's word of 14 October 2026: in a
  function that only jumps forward, or whose jumps back make a loop
  with one way in, they become `if`, loops, `break`, `continue` and
  `return`, by pieces of their own beside steps 5 and 6. The
  functions where loops are entered in the middle or paths cross,
  some fifteen, `V00_10` of `view.c`, `C20_00` of `copy.c`, `F32_00`
  and `Run_00` of `usermenu.c`, `F39_00` of `menu.c` among them, keep
  theirs until one is rewritten for another reason. A label that goes
  leaves the assembly's name in a comment where it stood, so that the
  code can still be held against `src/c` and `src/asm`.
- Clang is where this is going, for the platforms it reaches and for its
  sanitizers and static checks. Open Watcom builds the program for DOS
  for as long as DOS is where it is judged. No other toolchain is taken
  on.
- What judges the portable C is what judged the C translation: the
  scenarios, with goldens recorded from the original. Where the portable C
  is to behave otherwise than the original, that is a deviation.
- Every deviation is declared, in `src/portable_c/DEVIATIONS.md`: what
  the original does, what the portable C does, why, and which scenarios
  show it. What a scenario is then expected to show is the original's
  golden with the declared difference applied to it, on the user's word
  of 13 October 2026: a patch, a committed file of the rows that differ,
  with its reason, kept apart from the goldens of the original and
  applied to the golden as it is derived, so that deriving the golden
  again gives the expectation again; or, where one difference is on many
  screens, as the program's name is, a rule that the comparison applies.
  A patch that no longer fits the golden it was made for is a failure to
  look at, not to regenerate. The goldens of the original themselves are
  never edited.
- A feature that only made sense on DOS is removed, not kept as a setting
  that does nothing. The first is Memory allocation in the Options menu:
  the portable C stays in memory whole while a command runs. A removal of
  that kind is declared and made without asking. Whether a feature is of
  that kind is judged by what it gives the user, not by how DOS did it:
  Quick execute runs commands through an interrupt that only DOS has,
  and what it gives, commands run by the shell the program was started
  from, is not DOS's alone. Where that is in doubt, it is asked. Any
  other deviation, a fix of a bug of the original included, is asked
  first.
- What the user has decided of the features that were in doubt, on 13
  October 2026: Quick execute commands goes, the setting and the way
  through INT 2Eh; EGA lines goes, the command that gives the screen 43
  or 50 rows by a font of the BIOS, and the rows of the screen are to
  come from the system later: until a system gives the program a screen
  of another size, the scenarios that went to a tall screen by that
  command are not run for the portable C, and what they alone tested is
  listed as untested; the drive B: that shares the floppy drive
  with A: stays, with its prompt and its setting, for as long as DOS is
  a target. A setup file made from the defaults has zeros where the
  original has 64 bytes of whatever lay behind its table: that stands.
  Where the program of 32 bits does again what the original does and the
  portable C of 16 bits could not, a second copy that has the memory to
  run a command, it is held to the original's golden, and the deviation
  says only what still differs. When DOS ends the program by another's
  hand, the program of 32 bits is to be told as the 16-bit one is, by a
  small routine of real mode that the system's functions keep for it.
  The editor of the program of 32 bits has room for a file that the
  original's has none for: that is declared, the memory is not made
  short for it, and the scenario that has the editor refuse such a
  file, `editor-big`, is not run for that program, with what it alone
  tested listed as untested.
- What differs in the program of 32 bits from the original is decided
  by the user one difference at a time, on the user's word of 13
  October 2026: nothing gives leave to remove or break a feature of
  the original for that program without asking. Decided so far, each
  declared as it is and with no change of the program: what is inherent
  in running under the extender and shows on no screen; that a command
  finds no memory above the megabyte while the extender holds it; and
  that a program of another's which has stayed in the memory, started
  from this one, is cut off from its interrupts when this one is left,
  with the extender's lines of warning on the screen. A mend of the
  last was built and dropped on the user's word: `TODO.md` has it.
- Of the limits of the original that step 2 comes to, each asked as
  the work reaches it, the user has decided, on 13 October 2026: the
  cut of each panel to 256 files while a command runs goes, since a
  panel keeps its memory through a command now, and what was selected
  beyond the 256th file stays selected; that is the removal of what
  only made sense on DOS, and the piece that has `enter.c` makes it
  and declares it.
- Two things that the work on the addresses met, decided by the user
  on 13 October 2026. Go to of the file search asks DOS for the disk
  of the drive that the chosen line is on, where the original, by
  reading the drive's letter at the wrong place, asks for that of the
  current drive: a bug of the original, fixed and declared. And a
  search for a text stays as slow as it is in the program of 32 bits,
  three to four times the original's time on the machine's clock,
  since every byte's capital is asked of DOS through the extender as
  the original asks it: remembering the answers was offered and not
  taken.
- Two more limits of the original that step 2 came to, decided by the
  user on 14 October 2026: the file search that looks for a text reads
  a file in parts no longer than what is left of the room behind its
  list, where the original reads parts as long as the whole room and
  goes past the room's end when the list is long, a bug of the
  original, fixed and declared; and a panel holds as many entries of a
  directory as the memory allows, where the original stops at 2,703,
  lifted by a piece of its own at the end of step 2 and declared;
  the user said of it, the same day, that a panel's memory grows as
  its directory needs, with no ceiling of entries and nothing
  reserved for it beforehand, and that a scenario of it is kept only
  if it runs in reasonable time, the one that took ten minutes being
  dropped; and that a panel stops at 65,536 entries for good, the
  most that a directory of DOS holds, since an entry keeps the
  original's number of 16 bits: a system that is not DOS shows at
  most so many entries of a directory, declared. And
  a copy from a device that answers a piece in several reads counts
  the room down by every read, where the original counts it down by
  the last one only and reads past the room's end: a bug of the
  original, fixed and declared.
- More of what the user decided on 14 October 2026, as the clusters of
  step 2 came to it. Fixes of bugs of the original, each declared: the
  viewer counts the columns in front of a found text in 32 bits (entry
  18); the Info panel counts a file's clusters in 32 bits (entry 19),
  and the size of a cluster becomes 32 bits with the piece M, so that a
  disk of clusters of 64K shows its room; the editor counts its lines
  in 32 bits, so that a text of more than 65,535 lines can be reached,
  held by a scenario of both programs; a tree's file that claims 4,096
  entries or more, or none, is refused as a broken one; and the Info
  panel's test of a label that ends in "S" goes by whether reading the
  label failed. Every figure of free memory goes, on DOS too, as a
  thing that only made sense on DOS: the editor's free bytes are the
  last of them, and the rule that leaves that figure out of the
  comparison follows. One scenario may be held to an expectation of
  the portable C's own, where the original cannot go: `editor-huge`,
  a file larger than the original's room, when the memory comes from
  the extender; and a second, `menu-none`, allowed the same day, where
  the original never comes back to wait for a key: the bar left by
  the way to the menu that is none. On DOS, Alt with a letter goes on naming the letter of
  a US keyboard, as the BIOS gives it. The menu that is none, the
  bar pressed beside the names of the menus and the mouse moved onto
  one, which in the original makes the menu 0 and reads in front of
  the table of the menus and behind it, opens the menu under the
  pointer, as the original meant: a bug of the original fixed and
  declared, decided the same day; and the other way to the menu 0, the
  button held on the bar, the pointer moved off the names and the
  button let go there or moved below the bar, leaves the bar, as
  letting go elsewhere does. Of a help file that is broken,
  decided the same day: where its numbers of lines run past 65,535,
  the help wraps them as the original does; and an entry that says its
  text has no bytes, for which the program of 32 bits would write 64K
  behind the main data where the original went round its own segment,
  is refused as a broken file, a bug of the original fixed and
  declared. A scenario that makes such a file in the editor was
  offered and not taken, since the C translation cannot pass it. The
  editor's line number takes a sixth digit and more by moving Col to
  the right only where it is needed, so that a text of fewer than
  100,000 lines shows as the original's does; and the word Free goes
  with the figure that it named. The rules of the test of the layout
  that only held for the program of 16 bits, the tables within 64K of
  `Title_S` among them, go with the piece F10.
- A bug of the original stays as it is until its fix has been decided.
- DOS stays a target of the portable C for as long as DOS is where it is
  judged: every step builds for DOS and passes its scenarios there before
  it lands.
- The order holds here as it did for the translation, and it is now the
  point of the work: portable C first, then C tied to a compiler or a
  machine, then intrinsics, then assembly inline in a C file. What the
  README lists as resting on Open Watcom is to shrink, not grow.
- What the program asks of the system it is on, it asks in one place; the
  rest of it does not know which system that is. Today that place is
  `sys.h`, with `sys_dos.c` behind it (`dos32.c` until the piece 5a),
  the one file of the system's functions since the 16-bit build went,
  and a file of each other implementation beside it.
- A number is signed unless there is a reason for it not to be, on the
  user's word of 13 October 2026: a size, a count, an index, a length,
  a place in a text is an `i32` when it is widened, and one that is a
  `u32` today is looked at with the same question. Keeping a size in a
  type without a sign is widely held to have been a mistake: a
  difference that goes below nothing becomes a huge number without a
  word, where a signed one is negative and a sanitizer names a signed
  overflow. Here it is also the conversion that changes least, since C
  already reckons with a `u8` or a `u16` as a signed `int`. The reasons
  for a type without a sign, each written down where it is used: bits,
  a mask, a character, a colour, a cell of the screen; wrapping that
  the program counts on, with its cast written out; a width that a
  file, DOS or the BIOS has; and a quantity that can pass two
  gigabytes while there is no signed type of 64 bits, the size of a
  file, of a disk and of what is free on it, the sum of what is
  selected, which the original shows up to four gigabytes: those stay
  `u32`, the width that DOS has, which `sys.h` keeps by step 3: on the
  user's word of 14 October 2026 the size of a file stays 32 bits on
  every platform, Linux too, where a file of four gigabytes or more is
  then shown and copied as DOS would.
- Where a number meets one of another sign or is put into fewer bits,
  the code says so with a cast, on the user's word of 13 October 2026:
  a file of the portable C whose numbers have been sorted is held to
  clang's warnings of a comparison and of a conversion between signed
  and unsigned, and of a conversion into fewer bits, each switched on,
  none switched off for a line. A warning goes by making the type
  right, which is the first thing to try and is what moves numbers
  toward a sign, or else by a cast written at the place, with a
  comment where the program counts on what the cast cuts off. Writing
  the cast is the point: it makes whoever writes it think what happens
  at the edges. A list beside the code of the places and their reasons
  is not kept in place of that. What clang does not name stays as it
  is written, by the same word: a step or a sum that is kept at its
  own width, `word++` or `byte = a - b` of numbers of 8 or 16 bits,
  wraps in that width, which is what C says of it; no rule of the
  project's own is put on top of the compiler's. That changed on the
  user's word of 14 October 2026, with the build for Linux: its
  sanitizers of integers are fatal, so a step or a sum that wraps at
  its own width and that a run reaches gets its cast too, with the
  word `wraps` in a comment where the program counts on it, as any
  other cut.
- An integer type gets a name of its own only where its width is not
  the same everywhere, on the user's word of 13 October 2026: none is
  made while `sys.h` keeps DOS's widths, by step 3, and the size of a
  file, which was to be such a type, is DOS's `u32`. A name for what
  a number merely means, a count of columns, a colour, is not made: C
  holds such a name apart from nothing, a `Colour` that is a `u16` is
  taken wherever a `u16` is, so it is a comment that looks like a
  type. Such a number has the type of its width, `u8`, `u16`, `u32`,
  `i32`, or is a `bool`, and its name and a comment say what it is.
  What crosses to the system has the type of its width too, signed or not by the rule above: a count of
  bytes an `i32`, the size of a file a `u32`. Where the compiler is to hold two things apart, they are
  structures.
- Names may change here once a thing no longer is what its name in the
  assembly says. The C translation keeps the old names, and a comment at
  the new name says what it was.

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
  reason (`LINUX_NOT_RUN`), and the steps that use DOS's names of devices
  as devices are not run there (`LINUX_CUT`), each named by an entry of
  "The program for Linux" in `DEVIATIONS.md`.
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
takes, are in one place: the table of the README under "The figures",
which `python3 tools/figures.py --write` writes of the build and the
source. Run it when a piece of work lands, after the build and the
rebase, and commit the README with the work; `--check` says whether the
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
  The README counts the functions by group under "The system in one
  place"; a table of `sys_dos.c` alone is in `OWN_OF` of
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
  takes, on the user's word of 15 October 2026: a piece that need not
  wait for another starts at once, in a checkout of its own, and a plan
  or a reading that is cheap goes beside the heavy work.
- Each piece is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not do
  the work: it starts each agent, checks what the agent left, which is
  the commits, the push and `LastTest.log`, and keeps its report. Nor
  does it run or read what prints much, on the user's word of 13
  October 2026: the suite on `main`, a search through the documents, a
  small correction of a text are an agent's too, and the session keeps
  the conclusion. Its context is for the queue and for the user.
- An agent is given no time limit for work that is going well. The
  session watches what its agents are doing, and turns one round that is
  spending its time on something of little use. It also looks for what
  an agent has left running.
- An agent knows only what the repository tells it. What one learns that
  the next needs is written down before its work is committed: in the
  notes here, at the top of `harness/scenarios.py`, in `TODO.md`.
- Agents that work at the same time each have a checkout of their own, a
  plain clone of the same origin with its own build, and a part of the
  program that the others leave alone. The session names both.
- Work gets onto `main` one piece at a time, in a queue that the session
  runs: an agent commits in its checkout when the tests pass and says
  that it is ready; at its turn it does `git pull --rebase`, settles what
  the others changed keeping what both sides added, builds, passes the
  tests once more, and pushes. Nothing else lands meanwhile.
- An agent whose work changes only `src/portable_c/`, or what is the
  portable C's alone in the harness, runs the tests of the portable C,
  `ctest --test-dir build -R portable_c`, and not those of the frozen
  trees, which cannot change by it. A change to what the trees share, the
  harness, the build or a tool, wants the whole suite. The session runs
  the whole suite on `main` itself, after what has landed.
- Two agents have made the same thing under one name with different
  arguments, and git saw no conflict in it. After a rebase, look at
  everything the others added that names what is yours.
- Nothing in the harness takes turns: recordings of the original and
  tests run at the same time, in as many checkouts as there are, and
  other machines only make them slower. Where a recording or a test does
  come out different under load, that is a flaw to report, not something
  to wait out.
- The tools and the harness are improved as the work goes, without
  waiting for confirmation, each improvement a commit of its own. What
  decides is the total cost: an improvement is made when it saves more
  time over the work still to come than it takes to make, counting time
  that has actually been lost, not time one might lose.
- Stop and ask, with the work at hand left unpushed, before any of
  these. An agent that meets one hands its work back as it is, and the
  session asks the user:
  - in the C translation, behaviour that differs from the original's;
  - in the portable C, a deviation that is not the removal of something
    that only made sense on DOS;
  - a change to what the comparison leaves out, other than a figure of
    the kind that "Scenarios" has left out, or to a limit that a test
    holds the build to;
  - a golden of the original that comes out changed when it is derived
    again;
  - anything new to download;
  - a change to these instructions, other than a note on what was
    learned.

## Scenarios

- A scenario starts from the program as its distribution configures it, on
  a machine of its own, and sets up whatever else it needs through the
  program's own menus. It does not rely on prepared configuration files.
- The test bench changes two things, and gives every scenario the setup
  file that the original saves after making them in its own menus. The
  clock is off. And the panel on the right lists files, where the
  distribution has the Info panel there, on the user's word of 13 October
  2026: what the Info panel shows is of the machine and of the system,
  and with it on every screen every difference there was a difference in
  every scenario. A scenario that is about the Info panel switches it on.
- A scenario that shows the time of day, with the clock switched on or in
  the time stamp of a file it has made, does so within the first minute of
  its machine. The machine's clock is counted in the instructions it
  executes, and a C tree takes more of them than the original to get to
  the same step: a tenth to a third more as a rule, and twice as many
  where a step searches a text, as counted over every scenario on 13
  October 2026 (the top of `harness/scenarios.py` and `TODO.md` have
  the figures; the sixth that stood here was of two scenarios). The
  first screen is at about five seconds past the minute. So such a
  scenario is tried in every tree that it is run for, and is done
  showing the time by about fifty seconds past in the slowest of them,
  which for most scenarios is by about forty in the original. A scenario
  that switches
  the Info panel on takes some five seconds more, once, when the panel
  is first drawn.
- A figure on the screen that follows the time of day or the size of the
  program, such as the clock or a count of free memory, is left out of
  the comparison, in `harness/deviations.py`. The exception is a scenario
  that is made to test that figure.
- The panels of the bench list files sorted by name, on the user's
  word of 15 October 2026: the bench's setup file is the one that the
  original saves with that set in its own menus, and the goldens are
  derived again with it. In the portable C there is no unsorted order:
  the order of a directory means something on DOS alone, so it goes,
  as a thing that only made sense on DOS, declared, and a panel sorts
  by name unless the user picks another order. Since the scenarios move the
  cursor by places in a list, their steps are rewritten for it, each
  to go to the files that it meant, the user choosing that on the
  same day over keeping the bench unsorted (`TODO.md`, "SORT"). The
  tree, which has no order of its own and lists directories as they
  stand on the disk, is made alphabetical the same way: the bench makes
  its directories in the order of their names, so that the disk's order
  is the names' on DOS too, on the user's word of 15 October 2026.
- The countries tested are the United States and Russia.
- Russian text is typed on the Russian keyboard of the Russian bench. Its
  Cyrillic letters are selected in a step that types text into an input
  line, never at the panels: there Alt by itself brings up the menu.
- The tests run the build of a C tree that is shipped; no build is made
  for them. The original and the C trees are driven alike, by a program
  of ours that stays resident in the machine, `harness/DRIVER.ASM`: it puts
  the keys of a step in through the keyboard controller, so that the BIOS
  makes of them what it makes of a keyboard's, and writes the screen out
  when the program idles with none left. It watches and feeds, and
  answers nothing in the program's place or the BIOS's.
- A test does not put a stand-in where something is that the project does
  not own: DOS, the BIOS, the machine. What a test knows of them it gets
  from the real ones. A part of the program itself may be replaced in a
  test build, since its source says what it is supposed to do.
