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

## Notes on the portable C

What the work on `src/portable_c/` has written down for whoever goes on
with it. Like the notes above they are notes, not rules; the rules are
in "The portable C".

- Since the 16-bit build went, on 13 October 2026, the portable C is
  the program of 32 bits and that alone (`TODO.md`, "W and Z1, the
  switch and the end of the 16-bit build"). The notes below were
  written as the work went, most of them while the tree was a program
  of 16 bits or was built both ways, and they are kept as they were
  written, for what the work met. What a note says of two builds, or of
  the tree as a program of 16 bits, is of before, and is read so:
  - `dos.c`, of the portable C, is the file that is gone, and what it
    held is `dos32.c`'s: the one file of the system's functions, which
    keeps its name, since the C translation has a `dos.c` and the
    tables of the harness that are one for both trees tell the two
    files apart by their names (`SYSTEM_FILE` of `harness/reads.py`).
    `resident.c` is `exec.c`, which the harness knows under the C
    translation's name (`FILES` of `harness/reads.py` and of
    `harness/ported.py`). `arith.c` is gone with the test
    `portable_c-arith`: `harness/arithcheck.py` is of the C
    translation.
  - The compiler is `wcc386`, in the flat model: one segment of code
    and one of data, a pointer an offset of 32 bits, every function
    near, and the arguments in EAX, EDX, EBX and ECX. What a note says
    of the medium model, of far functions and far pointers, of the two
    code segments with their limit, of a relocation that a call costs
    and of what the compiler did with a function's registers is of
    `wcc`, the 16-bit one. The words `__far`, `STACK`, `FAR`, `CODE`
    and `TEXT_FAR` are gone from the source, and `#ifdef __I86__` with
    them: each word of the addresses in `vc.h` has one body, the flat
    one, which the notes call the second.
  - `build/portable_c/VC.EXE` is the program that was
    `build/portable_c32/VC.EXE`, with `debug/` in it, and the tree
    `portable_c` of the harness is that program. There is no tree
    `portable_c32`, no `SOURCE` in `harness/deviations.py`, no
    directory of patches but `harness/goldens/portable_c/`, and no
    record: `harness/pass32.txt` and `ported.py --record` are gone, and
    the test `portable_c-scenarios` holds the program to every scenario
    that it is taken through. The tests of the layout and of the data
    are `portable_c-layout`, `portable_c-debug-layout` and
    `portable_c-data`; `harness/notice.py` has no `--build`; and
    `harness/trial32/both.py` is gone, `asked.py` there running the
    toy with `dos32.c` alone.
  - Where a note has "the same bytes of `build/portable_c/VC.EXE`" for
    a check, the file meant was the 16-bit one. The check itself holds
    of the program that there is, by `sha256sum` before and after, for
    a change that is to be no code. What is no longer to be had is the
    16-bit file as the witness of a change that is code in 32 bits: a
    cast that was "no code on DOS", a word of the addresses written
    where a cast stood. The scenarios judge those now.
  - The build compiles a file again when the compiler's command has
    changed, by a file that the configuration writes into the build's
    directory, `compiled.txt`, and every object follows: without it an
    object that another compiler had left under the same name was
    linked as it was. A custom command of the build does not notice
    by itself that it has changed.
  - A piece that takes a name of the harness away, a tree, a
    directory, a test, looks for it in `harness/trial32/` too, whose
    scripts and toys have the commands that run them in their
    comments.

- Since the piece 5a of step 5, on 14 October 2026, `dos32.c` is
  `sys_dos.c` and `stack.c` is `stack_dos.c`: a file whose name ends in
  `_dos.c` is of the implementation of `sys.h` for DOS, and the build
  for DOS takes those beside the common code, every file that ends in no
  implementation's name (`add_portable_c` of `CMakeLists.txt`). The
  notes from before name the two files by their old names, and are read
  so; those that say how a thing is done, where a function of `sys.h` is
  and how one is added, name them as they are. The map names an object
  by its file, so an image of `harness/dataimage.py` saved before the
  rename has the variables of `sys_dos.c` under `dos32`: save the
  "before" anew. `TODO.md`, "5a, the split", has the rest.
- The C translation is built by `add_c_tree` of `CMakeLists.txt`,
  which takes the tree's name, its model and the files of its second
  code segment: the notes above say `port_c_second` and `c_second` for
  that list, which is the end of the call now. The portable C is built
  by `add_portable_c`, which takes a target, the directory of the build
  and the compiler's options that are that build's own, once for the
  program and once for the one of `debug/`: it has one code segment and
  no such list.
  The harness takes the tree where it had `src/c` written in:
  `datacheck.py` and `reads.py` go by the name of the build's directory,
  `notice.py` by the file it is given, `play.py` has `--tree`. The
  tables of `datacheck.py` and `reads.py` are one for both trees: an
  entry that only one tree wants will need them told apart.
- The portable C was compiled in the medium model, `-mm`, while it was
  a program of 16 bits, and the notes from here to the one on the
  figures are of that compiler where they speak of one: a function is
  far unless it is static, by the compiler's rule and with no
  `#pragma aux`, and a pointer to a function is four bytes. A static
  function is called near, as before. The compiler calls `__U4M` and
  `__U4D` far then, with a relocation for each call from the second
  segment; a `__near` in their `#pragma aux` is taken without a word and
  the program links, and returns near to a far call. `_asm` takes the
  name of a function for a far place: `jmp Exit05` in `Exit01` of
  `dos.c` is a far jump there. (`Int2E` had one to `Exec2E`, while it
  did not return.)
- What the compiler does with a function that has no `#pragma aux`,
  read in the listings: the arguments come in AX, DX, BX and CX in
  their order, a far pointer in two of them, and what does not fit
  there on the stack; a value in AX or in DX:AX; a structure of four
  bytes or fewer in DX:AX as well, and a larger one through a pointer
  in SI to room in the caller's frame, which the function fills through
  SS, so it works with the stack outside the data. The function keeps
  every register that carries neither, but ES.
- The registers went from a function in every file at once. A file
  that calls a function of another often declares it for itself, with
  a line of its own, and the name went with the registers, `Name_`
  where it was `Name_c`: a declaration left behind is an undefined
  symbol at the linker. A line that kept the name and lost the
  registers in one file only would link, and call wrongly.
  `harness/ported.py` finds a function under either name.
- A pointer to a function has the convention of its type, and nothing
  holds that against the function at the linker: the type and every
  function called through it change together. `FncFn` of `dirfunc.c`
  and its five functions did; the casts in their table went, since the
  functions are of that type now.
- Before the registers of a function go, two things are worth knowing,
  and were looked at for all of them by a script: that every
  declaration of it is its definition word for word, and whether a
  call has arguments whose order of working out could matter, one that
  calls something next to one that calls or reads memory. There were
  20 such calls, and none where it matters.
- A local variable that is read before it is set holds what a register
  or the stack happened to, and by the compiler's convention that is
  another thing than by the registers of the assembly: no build says
  so, since Open Watcom does not warn of it, and a scenario only by
  chance. A compiler of this computer does warn. The tree was put
  through `clang -m32 -Wsometimes-uninitialized
  -Wconditional-uninitialized`, in a copy with the words of Open Watcom
  defined away, `__far`, `__based(x)` and the like, and the `LAID_OUT`
  lines taken out, all files but `resident.c` with its `_asm`: it named
  two variables, in `edit.c` and `read.c`, and both are set on every
  path that reads them. `gcc` at `-O2` named none, and none either in a
  function written to have one.
- A comment in front of the lines often said what they were for, "How
  the assembly passes arguments to these", "saves every register", and
  is stale when they go: look at what stood in front of each line
  taken out.
- What still has registers in a `#pragma aux`, and stays so until the
  thing itself changes: the inline assembly of `dos.c`; `__U4M` and
  `__U4D`, whose registers are the compiler's; and `CaseMapFn`, DOS's
  routine. `Int2E`, whose command was in DX:SI, in `dos.c` since the
  piece S3, went with Quick execute commands.
- An object has the compiler's name in the map, `_Name`, where the C
  translation takes the underscore off with `#pragma aux Name "*"`:
  `harness/datacheck.py` knew an object by either, and no name of the
  portable C comes from a `#pragma aux` any more. A text that code reads
  as a far object and a file of constants names by a near word is
  declared `TEXT_FAR` in `vc.h`, which is `__far` but in a file that
  has `tables.h`.
- A function that returned two things as one number of 32 bits returns
  what it means: all of them do now, and the README has what became of
  the 114. Whatever is retyped next is changed so that a call which was
  not changed does not compile: a structure returned by value does
  that, since no shift or cast takes one apart, and costs nothing
  where it is two words. A value with a
  pointer for the second thing does too, by the count of arguments,
  but the pointer is to a local variable and wants `STACK`, which is
  this compiler's. What does not: the same function returning one of
  the two as a `u16`, which every old caller still compiles against,
  and reads as the low word.
- How a deviation goes through the tests, as the first one did. It
  is declared in `src/portable_c/DEVIATIONS.md` and then, for the
  harness, in three tables that each fail the test for an entry that
  no longer applies:
  - `TREES` of `harness/deviations.py`: the scenarios that are not run
    for the tree, each of which an entry of `DEVIATIONS.md` has to name,
    as `` `NAME` ``; a step that is left out of every scenario for it,
    where the keys choose something that is gone, or steps of one
    scenario; a step that the tree is taken through in place of
    another, in every scenario or in one; and a rule for what is
    otherwise on many screens. A program knows its tree by the
    directory of the build that its file is in.
  - `DECLARED` of `harness/datacheck.py`: a table of the program that
    differs, as the places where it departs from the original's, each
    with the bytes that the tree has there or none for bytes left out.
    The object is held against the original's with that done to it,
    so the rest of it is still held. `GONE` of `harness/reads.py` has
    a constant that the tree no longer has and that `REVIEWED` lists.
  - `GONE` of `harness/ported.py`: a function of a batch that the tree
    no longer has.
  Then the scenarios: build, and `python3 harness/compare.py
  build/portable_c/VC.EXE` says which differ from the original's
  goldens, which is the list to declare, each looked at. A scenario
  whose steps choose what is gone needs the step left out, not a patch
  of the keys going astray; one that then shows every screen as the
  original does after the same step needs nothing more. What a
  scenario shows otherwise is its patch, the next note.
- How a patch is made and what it is, since a deviation is held by one
  (`harness/goldens/portable_c/NAME.patch`; the top of
  `harness/goldens.py` has the form and why it is no unified diff):
  - `python3 harness/goldens.py patch --tree portable_c NAME` takes the
    build through the scenario and writes every row that differs from
    the original's golden, the original's row behind a `-` and the
    build's behind a `+`, and prints them with the cells marked. That
    is what the build did. It is a patch when whoever declared the
    deviation has read every row against the entry of `DEVIATIONS.md`,
    has written the entry's number and a sentence of why into the
    header, and commits it; a header that is not filled in fails the
    test, and so does one that names an entry which does not name the
    scenario. A row that the declaration does not explain is a change
    that was not meant.
  - The test reads every patch before it runs anything, and holds it
    to the golden row for row. What it then says is one of two things,
    and they are told apart by their first lines. "does not fit ...
    Nothing was run": the original's golden is otherwise than the
    patch has it, which says nothing of the program; the message has
    both rows with the cells marked, and whether the golden moved in
    cells that the patch changes or only around them. "differs from
    the original with NAME.patch", with rows called "the patch": the
    patch fits and the program shows otherwise than it says, which is
    the program's to mend.
  - When goldens have been derived again on purpose, as for a change
    of the bench or of a scenario's steps, every patch on a row that
    came out otherwise fails in the first way, and is made again by
    the same command. It keeps the header and says how the new patch
    stands to the one that was there: the rows that expect the same
    in the same cells on a row of the original that is otherwise now,
    which need no second reading, and by name those that say something
    else, are new or are gone, which are read like a new patch. With
    all of them made again, `git diff` of the directory is the reading
    list.
  - A scenario where only steps are left out has no patch: it is held
    against the original's golden without the screens of those steps,
    `left_out` and `left_out_of` of `TREES`, and `python3
    harness/goldens.py changes --tree portable_c` lists them next to
    what each patch says. Four of the sixteen goldens of the tree's
    own that there were needed nothing else.
  - A patch is of rows, and a row that differs in one cell is in it
    whole, with its colours: 293 rows where the whole files had 9,050.
    The cell in the middle of the screen whose colours are exchanged is
    the mouse's pointer, which a program that has ended does not show:
    `lost` has it as a row of its own.
  - A rule of one tree is `rules` of its entry in `TREES`, made to
    every screen of a golden when it is read for that tree and before
    its patch: the rows of a patch are then the golden's as the rules
    leave them. The program's name is said by four (`DEVIATIONS.md`,
    10), each a `Part` of `harness/deviations.py`, the kind of rule
    for a text that is of another length in the tree, or stands
    elsewhere on its row, or for rows that the tree has other rows in,
    and that a menu, a box or a panel may cover
    in part: it says the text as the original shows it and as the tree
    does, and on a screen where enough of the original's shows, each
    cell that the golden has as the original's text has it, character
    and colour, is expected as the tree's, and every other cell as the
    golden has it. What making them met:
    - How much is enough. Eight letters in their places were too few:
      a line of text in the editor, in the colour of a panel, had nine
      of the Info's title, and the rule wrote the name into it. It is
      twelve, and the two screens that show fewer, of a title under
      a menu and of the banner beside a panel, have the rows in their
      scenarios' patches. A rule for one screen is a patch.
    - Such a rule is wrong of a cell in two ways, and both fail the
      row, which then goes into the scenario's patch. Something that
      covers the text has a space in the text's colour where the
      tree's text is longer: the setup's sample of the colours, made
      grey on black, over the end of the banner. Or the text shows in
      another colour, under the mouse's pointer: the rule takes that
      for a cover. It is not to be taught the pointer's colour, which
      was tried: the pointer turns grey on black into black on grey,
      which is a box, and the rule wrote the banner into the box of
      F10. The shadow of a box, which makes `(colour & 8Fh) ^ 8` of a
      colour as the program is distributed, a rule knows only where it
      is told to, `shaded`, which the rule for the top of the Info
      panel is: see below. No screen has the banner in one.
    - On a row that a rule changes, the line with `-` of a patch is
      what the rule made, not what the original shows: read it so. Of
      the two patches that there were on such rows, `second-copy` and
      `setup-lines`, the tool said "does not fit" and made them again.
    - A box whose width follows a text, as that of F10 follows the
      name, cannot be a rule: where it is narrower, the screen shows
      what was behind it, which the golden of that screen has not.
      Those are patches, the same seven rows in eight scenarios.
    - The test `portable_c-scenarios` says how many screens each rule
      expects otherwise, and fails for a rule that finds none.
    - `deviations.expected` makes the rules of every build too, when a
      golden is derived. After a change to it, derive every golden
      again: `git status` of `harness/goldens/` is then clean, as it
      was after this one.
    - In the data, a text that the tree has longer than the original
      reached into the room that is reserved for what only the shipped
      binary has, and was "not compared" for that, declared or not:
      what counts now is how much of the original's object it is held
      against, `held` of `harness/datacheck.py`. A text that only one
      tree has is in `OWN_OF` there.
    - The top of the Info panel is one `Part` of four rows of 40 cells
      (`DEVIATIONS.md`, 10 and 11): the title, the line and the two
      lines on the memory in the original; the name, what it is based
      on, the line and an empty row in the portable C. A part of rows
      took what a text did not. Colours cell by cell, since the two
      figures are yellow: `colour` and `now_colour` are a number or,
      for each row, its runs as a golden writes them. The frame and
      the line across left out of what the part is known by, `frame`:
      the foot of any panel has such a line between such ends, and
      forty cells of it are more than twelve. And the shadow.
    - `shaded` has a part known in the shadow of a box as well: a cell
      that has the part's character in what the shadow makes of the
      part's colour is expected with the tree's character in what the
      shadow makes of the tree's colour, and counts among the twelve.
      It was needed: the Right menu over the Info leaves ten cells of
      the title and five letters of the two lines below, four of them
      in the menu's shadow. It is not what teaching the pointer's
      colours was: the shadow makes cyan on black of the panel's
      colour, and nothing that the program draws over a panel as it is
      distributed has that. It is switched on for that one rule.
    - A part may stand with its first rows above the screen: an error
      beeps, the BIOS moves the screen up a row for the beep, and the
      panel is a row further up without its title (`move`). And no
      place of a part lies over another: what the program puts back
      after such a beep leaves a row of the panel twice, the part was
      known at two places that overlapped, and each wrote into the
      other, so that the row which the rule made was one that neither
      program shows. The place where more of the part shows is taken;
      the rows that are left twice are `move`'s patch.
    - A figure that the comparison leaves out is in a part as the
      golden has it, `#######`, since a golden is blanked when it is
      derived and a rule is made to the golden as it is read. What
      the machine has of memory, 653,296, is in the rule as a text: a
      bench with another figure fails every screen of the Info, which
      is the notice to change the rule.
    - `compared` takes the tree: a build's screen is blanked of what
      every tree leaves out, `IGNORED`, and of what its own tree does,
      `IGNORED_OF`, and a golden's of all of it. The count of free
      memory in the Info is of the C translation alone there, so a
      portable C that showed such a figure again would fail.
    - When a rule changes, a patch on a row that it now changes says
      "does not fit" before anything is run: make it again with the
      tool, which says whether it states what it stated. `mouse-menu`
      was the one here, and `menu` lost the row that the rule now
      knows.
- The portable C is one program, and the notes above on the part that
  stays in the memory, on `tools/mkexe.c` and on string literals and
  static variables are of the C translation alone. In the portable C DS
  is on the compiler's group, `DGROUP`, in every function, an interrupt
  function and a `__loadds` one included, since that is what the
  compiler loads; a file may have variables and strings, which are of
  that group; and `md` is a variable of `main.c`. The README has the
  layout, under "Staying in the memory whole". What making it so met:
  - `#pragma data_seg("NAME", "CLASS")` makes a segment of that name and
    class and puts it into the group that the file's data is of:
    `DGROUP`, or with `-nd=S` the group `S_GROUP`, and then under the
    name `SNAME`. An object written behind the line is of that segment.
    Nothing gives a segment an alignment: it is that of a word.
  - The linker takes a segment of the class `STACK` for the stack and
    has the file's header name its end, with SS on the paragraph that
    the segment begins in, which is the stack's group if it is one.
    Without such a segment the header has SS and SP 0, and the program
    would start on a stack laid over its own first bytes.
  - A group is as long as from its first segment to its last, whatever
    lies between, and the linker stops at 64K: the stack and the tables
    cannot be of `DGROUP` beside the main data, each is a group.
  - The file holds the program up to the last byte that has a value,
    with zeros for what has none in front of that: the main data, which
    has to be first, is 62.6 KB of zeros in the file. The linker's
    `segaddr`, which starts a class on a paragraph that the command
    names, breaks that: it wrote all of what followed into the file and
    a header that asked DOS for a megabyte.
  - A variable declared `CODE` and `static` has the compiler's name for
    data, and the data's test takes it for a constant of the original's
    name: `VARIABLES` of `harness/reads.py` is for such a one, and had
    `Vect21` until that went with the entry of INT 21h.
  - A text of the original's name may be a function's in another file:
    `Command`, "COMMAND.COM", and `Command` of `main.c`. It is declared
    in the file that reads it, not in `vc.h`.
  - DOS loads a program into the largest block that it has, not into
    the first that fits: COMMAND.COM, 73 KB on the bench, goes behind a
    program that has stayed, and the gap that an earlier one left in
    front of it is not used again. And FreeDOS's COMMAND.COM keeps its
    73 KB while a program runs from it. The bench has 500,128 bytes
    from where the program is loaded.
  - The program reads its panels in a block that it has just given back
    to DOS, to move them to the largest block that DOS then has
    (`TakeMem` of `start.c`): DOS writes nothing into a block that it
    is given back, and has it as the start of the largest if it lies in
    front of it.
  - What the tests know of a program's file they get from
    `harness/coverage.py`: `made_by_mkexe` told the C translation's
    file from one that the linker wrote, by its header, and `origin` is
    what to add to an address of the linker's map for the place in the
    file. The limits that `tools/mkexe.c` holds the C translation's
    build to, for QEMU, `check_layout` of `harness/ported.py` held the
    16-bit portable C to, at the start of the scenarios' test. The
    file of the program that there is, a flat one, is read by
    `harness/le.py`, `le.linear` tells it from one of 16 bits, and
    `check_layout` asks `harness/layoutcheck.py` of it.

- What retyping the first 22 of those functions met, each of which
  the compiler took without a word:
  - A variable that several calls share, `pair` as a rule, is split
    when one of the calls returns a structure. Every call but one took
    its number apart on the next lines; `Dialog` did not: four branches
    set `pair` and one line behind them all read it. The branch whose
    call got a variable of its own no longer set `pair`, and the line
    read what was there before. Look at every read of the old variable
    that is left, not only at those behind the call that changed.
  - A call may take apart what the call before returned, in its own
    arguments: `pair = F34_40(HIGH(pair), 1)`. With two variables that
    is the first one's member, and the second one's compiles as well.
  - `clang` finds the first of the two as a variable that may be read
    before it is set, by the audit of the note above, when nothing set
    the variable earlier in the function; neither it nor the compiler
    finds the second. Both were found by reading every changed line.
  - A file may have a structure of the name already: `struct Mouse` of
    `menu.c`, which is `struct MenuMouse` now. That one the compiler
    says.
  - `Function(arguments).member` is taken, and reads well where only
    one of the two things is wanted. A call that ignores what is
    returned needs no change, and compiles.
  - A structure of two words comes back as the number did, in DX:AX:
    the program is a few bytes longer for the functions that make one,
    `Changed` and `Unchanged` of `input.c` in place of two macros.
  - Open Watcom does say "has been referenced but never assigned a
    value", W200, for a variable that nothing sets at all.
- What the nine of `edit.c` added to that:
  - Where every call that sets a shared variable returns the same
    structure, the variable keeps its name and takes the structure's
    type, and neither of the two mistakes can be made: a read that was
    not changed does not compile. `answer` of `T80_00` is set by three
    functions and read behind them all, and the three return
    `struct Answer`. Divide a variable only where its calls return
    different things.
  - A macro may make the number that such a function returns, and be
    taken apart in more places than the function: `ADDRESS_OF` made
    the address that `T62_00` returned, and 22 more. It went with the
    function, and is `AddressOf` with `struct Address`. Called where
    the macro was expanded, it made the program 304 bytes shorter.
  - Two functions that make a structure in the same way are one piece
    of code under two names in the listing: `EndsAt` and `Done`,
    `NoEnd` and `Failed` of `edit.c`.
- What the ten met that a file kept to itself:
  - Not every such function has two things to return: read every
    `return` of one before it gets a structure. `F36_04` of `copy.c`
    returned the same place of the loop from each, with nothing above
    it: it is the end of a command in the assembly, a jump. It returns
    nothing now, its caller says where the command goes on, and
    `return F36_04();` left as it was is refused. `MenuItem` of
    `usermenu.c` put into a number what `FndItem` returns as a
    structure: it is gone.
  - Where the calls that set a shared variable return structures of
    their own, each with the thing that a line behind them read, the
    variable goes, and that line: each branch of `Dialog` sets the key
    itself. Nothing but reading holds that every branch does, since
    the key is set earlier in the function and `clang` has nothing to
    say.
  - That an old call is refused can be tried: put it back in a copy of
    the file and compile the copy. Define `LOW` and `HIGH` in the copy
    first. A file loses them with its last number, and then the
    compiler refuses the macro it does not know, whatever the function
    returns.
- What the commands met, and what else comes back to the main loop,
  49 functions, while those of `enter.c` were still to do:
  - What a command of the keys returns, `struct Onward` of `vc.h`, is
    two bytes and a word, four bytes, and comes back in DX:AX as the
    number did, the bytes where they were: `To` of `keys.c`, which
    makes one of a place, is three instructions with its return, and
    `ToMenu` two. A structure of more than four bytes comes back
    through a pointer to room in the caller's frame; this one was kept
    to four for that.
  - A member that is a byte holds a byte, where the number took
    whatever was shifted into it: `table << 8 | held` of the menu bar
    has the bits of `held` above the eighth in `table`. The two are
    the same only where every value fits its byte, and that is read
    from every assignment to it, as for `F39_00` of `menu.c`.
  - A flag next to a place may be a place of its own: the editor was
    the viewer's place with 1 in the high word, and is `NEXT_EDIT`.
    The list of the places is an `enum`, so a place put in moves the
    numbers of those behind it, in every file that is compiled.
  - Where one place of a family takes a second thing, it has a
    function of its own that makes it, and the others one that takes
    the place alone: `Shows` and `To` of `find.c`, `Masked` and `Go` of
    `copy.c`. One function with an argument that is 0 at every call
    but one says less.
  - Each file makes its structures by static functions of its own, as
    `To` is in `keys.c` and in `view.c`: a macro cannot make one, and
    a function of another file is a far call, with a relocation from
    the other code segment.
  - While one file still returns the number, what takes the structure
    has a function that makes it of the number, as `OfEnter` of
    `main.c` was, and the calls of that file go through it. An old call
    is refused
    both ways then: a number where the structure is taken, and later a
    structure where `OfEnter` takes a number. Twelve lines put back as
    they were, one of each kind, were all refused, with the trial of
    the note above.
  - The variable that two calls of `Command` shared, `pair`, went, and
    each call has a variable of its structure: every read stood right
    behind its call there.
  - A number of 16 bits may have two things in it as well, a thing in
    each byte or a flag above something, and its type does not say so.
    `grep -n '^\(static \)\?u32 \w*(' src/portable_c/*.c` lists the
    functions that return 32 bits, which was all that `tools/pairs.py`
    did, gone since: three, each a number, `Timer`, `__U4M` and `Dos35`.
- What the last of them met, the commands of `enter.c` and three
  numbers of 16 bits, `T00_00` of `edit.c` and `Command` and `Func21` of
  `main.c`:
  - A function that `vc.h` has among the commands may be none.
    `F24_02` of `enter.c`, which Enter and the changes of directory
    end in and the main loop calls for the file search, returned one
    place from every return: it returns nothing, and `main.c` says the
    place of the loop itself there, without `CmdEnd`.
  - A flag above something is one thing or another, not two things.
    Where the two are of a kind they are one enumeration: the places of
    the loop and, behind them, what the loop is left for, the `END_` of
    `main.c`, where `LEAVE` was. The numbers of the second kind are
    others then, so look at whatever compares with them. Where they are
    of two kinds, a command of `vc.h` or a place of the loop, it is a
    structure with a member that says which: `struct Asked` there. An
    old line is refused either way, the name of the flag being gone.
  - A variable that the changed call shared with a call that stays a
    number, `what` of `V00_03`, is there still, and a read of it that
    was overlooked compiles: its three reads behind the editor's call
    were each changed by hand, and read again.
  - The trial of an old line, of the note above, is one command for a
    copy of the file next to copies of the two headers, the compiler
    being in `build/_deps/open_watcom-src`, which `WATCOM` is set to:
    `binl64/wcc -q -0 -mm -os -s -zu -zl -zls -wx -we -fo=x.obj` and
    the copy. Try the copy as it is first: it has to be taken. That
    was the command of the 16-bit build. For the program as it is, the
    copy wants `tables.h` beside it as well where the file has tables,
    and the command is `binl64/wcc386 -q -3r -mf -s -zl -zls -wx -we
    -os -i=$WATCOM/h -fr -fo=x.obj` and the copy;
    `build/portable_c/compiled.txt` has the command that the build
    uses.
- A build can be tested while the tree goes on changing: copy
  `build/portable_c/VC.EXE`, `VC.MAP` and the object files elsewhere
  in `build/`, and run `harness/datacheck.py` (at once: it reads the
  source), `harness/layoutcheck.py` and `harness/ported.py` on the copy,
  each as its test in `CMakeLists.txt` runs it.
  The commit has to be made when the copy is. The copy is in a
  directory called `portable_c`, `build/copy/portable_c`: the harness
  knows the tree by that name, and holds a build of any other name to
  the original's goldens with nothing declared for it.
- For the audit with `clang`, of the note on local variables above:
  the `LAID_OUT` lines go from `tables.h` as well as from `vc.h`, and
  `-fsyntax-only -ffreestanding -Wno-everything` with the three
  warnings wanted, `-Wuninitialized` the third, leaves the two
  variables that are known and nothing else; `_asm` is defined away
  like the other words.
- What taking out four things met that the program did only for the PC
  and for the programs around it, and that show on no screen: TopView,
  DESQview, the turn given to Windows, and the wait for a display that
  shows snow (`src/portable_c/DEVIATIONS.md`, 2 to 5).
  - Such a removal wants nothing in the three tables of the harness, as
    long as no function that a batch lists in `functions`, no constant
    and no scenario goes or changes. What `harness/ported.py` lists as
    out of reach are instructions of the original, which stay listed;
    that list is also where to read that no scenario shows a thing: an
    instruction of it that the bench cannot get to is there, with the
    reason.
  - A feature is found by the byte that the main data keeps of it, in
    the assembly, `grep -a` over `src/asm` with all of its output, and
    not by its routine: `DesqVie` was read in three places, of which
    the pause in `LeaveTm` was one. The others were in `Input0` and in
    the editor, where the program does something otherwise under
    DESQview, and what it does without is what stays, unconditionally.
    The byte keeps its place in `struct MainDat`, with "of the
    original. Not set or read here" in its comment; a byte of the
    setup, Snow mode, is "kept as it is found", and its default stays
    in the table of `start.c`, so the setup file is the original's.
  - An instruction that stands with the feature may do more than the
    feature needs. `Window` and `MovScr` let the machine interrupt
    again, `STI`, behind what they copy with interrupts off for a
    display with snow, and do so whatever the display is; and DOS calls
    `FatErr` with interrupts off, whose box is put on the screen
    through `Window`. No scenario would have said whether the box
    needs that. The `IntOn` stayed, once behind each copy, with a
    comment that says why. Before one of a pair like that goes, look
    for who comes there in the other state.
  - Where a call of DOS or the BIOS goes, the registers that it left
    for the next call go too: read what the next call takes. `IniData`
    hands a structure of registers from call to call.
  - `Exit05`, of `resident.c` then and of `dos.c` since the piece S3,
    is no routine for INT 5: it is where DOS
    goes on when it ends the program. The program does not take that
    interrupt. What it does for Print Screen is one call of the BIOS in
    EGA lines, `F69_00` of `commands.c`, which was taken out and put
    back: it is a part of that command, and goes or stays with it
    (`TODO.md`).
  - A thing that is half of a command which shows is not of this kind,
    however little of it shows: the command stays whole until it is
    decided.
  - The figures that follow the size of the file were set again by
    hand with every one of these, in three documents, and were a
    conflict in every commit at the rebase. They are in one place now:
    see the note on the figures below.
  - The builds were each tested as a copy while the next was made, by
    the note above, two or three at a time: the scenarios' test took
    five to six minutes each that way.
- A name that is declared in `vc.h` can change the program where
  nothing uses it. What has no value the compiler lays out in an order
  of its own, and of `SavePtr` and `Vect27` in `resident.c` that order
  went by how many names were declared in front of them: three to six
  typedefs more in `vc.h`, or as many prototypes, and the two changed
  places; one, two, seven or eight, and they did not. The tag of a
  structure does not count, nor a macro. So a change that is to leave
  the program as it was byte for byte, and declares something in a
  header, may not: hold the listings against those of the build
  before, `build/portable_c/*.lst`, and if `resident.lst` alone
  differs, in the order of two variables of `_BSS` and nothing else,
  it is this, and the program is as right as it was. `HOLDS` of `vc.h`
  is a tag for that reason. (The two variables went with the watch on
  DOS, and the compiler was `wcc`. `wcc386` does it too: the 26 tags
  of the boxes and the dialogs that the piece 5b put into `tables.h`
  moved the variables without a value of `view.c` and `edit.c`, and
  their listings differed in the list of `_BSS` and nowhere else.)
- The test `portable_c-host` has gcc read the tree for a machine of 64
  bits and holds every file to its counts in `harness/hostcheck.txt`,
  by kind: the top of `harness/hostcheck.py` says what is counted, and
  `TODO.md` what each kind waits for. Using it and keeping it up:
  - It fails for a file that has another count than the record, fewer
    as well as more. Read what it names, `python3 harness/hostcheck.py
    --show FILE` for the lines, and if the change was to do that, write
    the record anew with `--record` and commit it with the change. The
    record's lines in the commit then say what the change did: a kind
    that went, a file that became clean. The test does not forbid a
    count that goes up, or a file that is clean no more: it has the
    change show in the commit.
  - After a rebase the record is written anew, not merged: the others
    have changed the files.
  - The kind `other` is what gcc says that `SAYS` of the script does
    not know. None is in the record. A file with a place of that kind
    usually does not parse: something of Open Watcom's that nothing
    takes out of gcc's sight, and gcc then says much about the lines
    that follow. Find the first of them with `--show other`.
  - The counts are gcc's, of any version: 13 and 15 agree place for
    place. clang counts otherwise, 480 numbers made pointers where gcc
    has 436 in the same source, and nothing for the address of a
    function kept as that of data, so the test does not ask it.
  - A cast that a macro makes is counted where the macro is used, so a
    file is not clean while it uses a macro that casts, as `DOS_AT` of
    `vc.h` was, or one of its own. A macro that is made right makes every file
    right that uses it, and `--macros` says which macros there are and
    how many places each makes. The words of the addresses in `vc.h`
    are made so: gcc reads their second bodies, which cast nothing.
  - What gcc cannot say, since it only reads, is in `TODO.md`. Of
    that, arithmetic that counts on an int of 16 bits has a test of
    its own, the next note.
- The test `portable_c-int16` has clang read the tree for a machine
  whose int has 32 bits and finds where a number that the two builds
  may have otherwise is looked at before it is back in 16 bits: the
  top of `harness/int16.py` says how it knows, and what it cannot
  see. `harness/int16.txt` has every such place with the reason why
  it is the same in both builds. The two builds are, since the program
  of 16 bits went, the code as it was written and judged, with an int
  of 16 bits, and the code as it is compiled now. Using it and keeping
  it up:
  - It fails for a line that is new or changed and is such a place:
    "not in the record". Read the place, `--show FILE` for the lines.
    Where the number can leave 16 bits or go below 0, or stays within
    only by something that is not to be seen at the place, write the
    cast that says what DOS does, `(u16)(a - b) < c`, and the place is
    gone. That was no code on DOS, so the 16-bit `VC.EXE` was the same
    bytes, which was held against the file of before; in the program
    that there is the cast is code, and the scenarios judge it. Where
    it cannot, run
    `--record` and write the reason into `harness/int16.txt` by hand,
    under the place's line, in the words of the others: what at the
    place or just above it keeps the number in.
  - It fails for a place of the record that is gone, also where a
    line was only written otherwise: `--record` takes it out, and
    keeps the reason of every place whose line is as it was. A place
    is its file, its function, its kind and the text of its line; the
    number of the line is not in the record.
  - A cast is the answer where the number is a place or an address of
    16 bits, a difference of two such, or a sum that the code itself
    cuts elsewhere. A reason is the answer where the line above asks
    that it fits, or a loop counts it. A cast where nothing can wrap
    says to the reader of step 2 that something does.
  - Added to a pointer, a cut number is what DOS has only where the
    pointer is the beginning of a segment: a panel, `ENTRY` of `vc.h`,
    and the viewer's text, `WORD_AT` of `view.c`, which cut what they
    are handed. Of any other pointer the two builds step back alike
    for a number below 0, kind `back`, and nothing written at the
    place makes a flat memory wrap round at 64K.
  - A place of the kind `minus` is no arithmetic: a number written
    below 0 and kept in 16 bits without a sign, `(u16)-160`, where it
    is cast, handed to a function, returned or given to a variable.
    It is the same number in both builds. Follow it to where it is
    used: added to a number of 16 bits it is the way back in both,
    added to a pointer it is that on DOS alone. The reason in the
    record says where it goes and that the sum is made in a number.
    One that is worked out and has wrapped round, a difference kept
    in a variable, is found nowhere.
  - A variable is taken to hold any number of its type, but one that
    a loop counts with, inside that loop: `for (i = 0; i < 13; i++)`
    with `i` the function's own, changed nowhere else in the loop and
    no label in it. A loop of another shape, `i += 3` or a second
    condition, is not known, and its places want a reason.
  - What is written in a word of the addresses of `vc.h` is no place,
    and the finder knows those words by their names, `WORDS` of the
    script: a word that is added there, with a shift or a difference
    of two addresses in it, is a place at every use until its name is
    in that list. And `dos32.c` is read like any file: it was written
    for an int of 32 bits, so the reason of a place there says that
    the number is meant as it is.
  - It needs clang, any that gives the account of a file that the
    finder was written for: it holds itself against
    `harness/int16check.c` first, lines with the kinds that are to be
    found in each, and says so where they are not. A line added there
    is worked out by hand first. Five of those written for it were
    wrong and the finder right: 65,530 written in decimal is a long on
    DOS, and what is added to it is added in 32 bits there too.
- With one build of the portable C, the bytes of the program say
  nothing of a change that is to leave it doing what it did. What says
  it is the data's image: what the program has in its main data, in the
  memory that it was handed and in its variables at the end of every
  scenario, which `harness/dataimage.py` keeps and holds against what
  another build has there. `TODO.md`, "F0, the net and the record", has
  what was tried and what is left out. Using it:
  - Before the first change of a piece, `python3 harness/dataimage.py
    save build/portable_c/VC.EXE build/before`. The scenarios' test
    writes the images of the build that it judges into
    `build/portable_c/images`, anew at every run; then `python3
    harness/dataimage.py compare build/before build/portable_c/images`,
    and its last lines go into the commit.
  - Where the scenarios' test has just been run on the build before the
    change, its images are the "before" already, and `save` is a second
    run of every scenario for the same bytes, some two and a half
    minutes: `cp -r build/portable_c/images build/before` before the
    change is built does the same. The test runs with the stack looked
    at, as `save` does, and the piece T2 found the two alike in all 159
    scenarios (`TODO.md`, "T2, the time of the suite").
  - It names what differs: a field of the main data, by the original's
    listing, a field of a panel or an entry of its list, a place behind
    the panels, a variable by its file and its name. Read each. What a
    piece means to change, it names with `--changed`; a word that it
    takes out of the main data for a pointer gets the pointer's symbol
    in `ADDRESSES` there, and is then held by what it names.
  - Two builds differ with no fault in what follows the time, since
    they get to a step by different counts of instructions: the random
    numbers, the blanker's stars, the time of a file that the scenario
    made. Those are in `UNSTEADY` with their reasons, and are counted
    as "may differ". Before another is put there, `save --early` of
    the same build says whether it follows the time: the driver then
    writes less, the program gets to every step a little earlier, and
    what differs from the build's own images follows the time.
  - The same build gives the same image in every run, under load too.
    One that does not has met a flaw of the bench, to report.
  - The machine is not left to end for it: QEMU pauses it where the
    driver resets it, `linger` of `harness/dos.py`, and the harness
    waits for QEMU to say so. Asking QEMU for its state a hundred times
    a second cost a quarter of a run.
- The test `portable_c-words` holds every file of the tree to how often
  it writes each word of the addresses, in `harness/words.txt`, fewer
  as well as more, as `portable_c-host` holds it to its record:
  `python3 harness/words.py --record` in the commit that moves a count,
  and after a rebase the record is written anew, not merged. A run
  prints which piece of step 2 is to have each word at nothing. A word
  is counted where it is written, in the body of another word's
  definition too, and a segment under another name than `panel` or
  `segment` is not found.
- The figures of the portable C that change with every step, the
  length of its file and what follows from that, what it has of the
  memory, how far its tables may grow, and the counts of what only
  Open Watcom takes, are in one place: the table of the README under
  "The figures", which `python3 tools/figures.py --write` writes of the
  build and the source, between two marks. Run it when a piece of work
  lands, after the build and the rebase, and commit the README with the
  work; `--check` says whether the table is the build's. Write no such
  figure anywhere else, in the README, `TODO.md`, `DEVIATIONS.md` or
  these notes: say the thing without the number, or point to the
  table. A figure that is history, what a step cost when it was made,
  stays where it tells that, and a commit message keeps its own. A new
  figure is a row of the table, added in the script. What the test
  `portable_c-host` counts is in its own table in `TODO.md`, which its
  `--record` flow goes with, and is not copied.
- What taking out the first feature that a user sees met, the map of
  the memory with the watch on DOS that it rested on
  (`src/portable_c/DEVIATIONS.md`, 7):
  - A line taken out of a menu moves every item below it, and an item
    is known by its number in more places than the tables that the
    data's test holds: the items that a menu shows dark are numbers in
    the code, `F39_58` of `menu.c`, and a command may be handed the
    number of its item. The data's test said nothing of those. `python3
    harness/goldens.py changes` did, as a minus in front of the wrong
    line, in the one scenario that shows that menu with the panels off:
    read every row of every screen that shows the menu, and `grep` the
    code for the menu's number. The tables that change with a line are
    in `DECLARED` of `harness/datacheck.py`, the eight of this one:
    among them where the lines and the pages of the next menu begin,
    two words of `MnuTab` that the test holds against a distance in the
    original.
  - A step that goes to the thing, with the steps that leave it again,
    is left out of one scenario by `left_out_of` of `TREES`: steps that
    follow one another there, since Esc is another key at the panels. A
    scenario whose whole point is the thing is not run.
  - What such a scenario had besides, which the tree still does and no
    other scenario shows, becomes a scenario of its own, for the
    original and both C trees: `asked`, which was a part of `stay`.
    `python3 harness/goldens.py derive NAME` derives the golden of a new
    scenario from the original in seconds, and both C trees have to
    pass it. A function of a batch that no scenario of a tree runs any
    more fails that tree's test, "not executed in the translation":
    either it is gone, and listed in `GONE` of `harness/ported.py`, or
    a scenario has to reach what is left of it.
  - A scenario that shows the Info panel on D: shows how much the file
    of the program under test leaves free there, which is another
    number for every tree.
  - What a program of a scenario prints of the registers may be an
    address in the program, once the program leaves another thing
    there than the original: it follows the layout and changes with
    every step of the work. `ended` prints DS:DX as the program leaves
    them, and `IGNORED` of `harness/deviations.py` had the two words
    left out for the line that only the portable C prints, while they
    were where INT 21h went before; it has one of them left out since
    (the note on the program's own call, below). Look for such a
    figure in every patch before it is committed: it passes today and
    fails for the next agent.
  - What leaned on the watch and stayed: the entry of INT 21h for the
    program's own call, with nothing of the watch in it, until that
    call went too; the prompt between two commands, which the original
    writes where the watch notes the interrupts, `PromptLine` of
    `resident.c`; and `SetStat` with 0 where the program begins. Read
    what a buffer or a routine of the thing is used for elsewhere
    before it goes: `grep` its name in the whole tree, the routine's
    and the buffer's.
  - A handler that no longer goes on in another handler needs no room
    on the stack for a second return address: `Int21` returned to
    whoever called, and `New21` jumped to it without the three
    instructions that made the room. `Exit01` still has them.
- What taking three settings out of two dialogs of the setup met,
  Memory allocation, TSR manager and Screen output
  (`src/portable_c/DEVIATIONS.md`, 1, 7 and 5; `TODO.md`, "Piece E2"):
  - A field of a dialog names the four fields that the keys go to from
    it, by their numbers, and a group by a flag whether on its first
    line or its last (`LINK_` of `dialog.c`). With a field gone every
    field behind it has another number, and its neighbours may be
    others: write down where each field stands now, and each of its
    four links from that, by the rule that the original's follow, not
    by taking one from every number. The data's test holds what is
    declared, which is what was written, and says nothing of a link
    that leads to the wrong field: only a scenario that goes that way
    does, as a cursor on another row.
  - A box with two groups side by side has one text for each row, both
    halves in it. A line taken out of the right half makes every text
    below it another, of the left half that it had and the right half
    of a line further down. Such a text keeps its name, and is declared
    in `DECLARED` of `harness/datacheck.py` without the bytes from the
    end of its left half to where that right half begins in the
    original, a stretch that runs on through the objects that follow:
    the test then holds the right half against the original's too, and
    the entry is two numbers. `_lines` there makes the entries, and
    `_field` those of a dialog's fields.
  - Steps that get to a line of such a dialog are others for the tree,
    and the same keys mean what they did elsewhere in the scenario:
    `in_place_in` of `deviations.TREES` has, for a scenario, steps of
    the original that follow one another and the tree's step for each,
    or None. Give a step with enough of those around it that the
    scenario has them once; a step that is alone is put in wherever it
    is, which is right for a click. `python3 harness/goldens.py
    changes --tree portable_c NAME` says of each screen which step the
    tree was taken through.
  - The mouse pointer exchanges the colours of the cell that it is on,
    and the setup puts it in the middle of the screen: lines that move
    under it are in the patch for that cell too, on the row that it is
    on and on the row that the text went to.
  - A patch of a dialog whose lines have moved is the same eleven rows
    on every screen that shows it, hundreds of lines. They were read
    by a script as well as by eye: it made of each screen of the
    original's golden what the declaration says, the rows of the right
    half moved and the cursor with them, held that against the golden
    with its patch (`goldens.read(name)` and `goldens.read(name,
    tree)`, screen for screen by `places`), and printed what was left,
    which is then little enough to read: the pointer's cell, the
    cursors that are on another field, the disks.
  - While a test runs, the next build can be tried beside it without
    touching `build/portable_c`: copy its object files into a
    directory of `build/` that is called `portable_c`, compile the
    files that have changed into it by their lines of
    `build/CMakeFiles/portable_c.dir/build.make`, and link by the line
    of the linker there with the paths changed. `harness/show.py
    --program` and `harness/datacheck.py` with `--source` take it.
- What taking EGA lines out met, the second command to go
  (`src/portable_c/DEVIATIONS.md`, 12; `TODO.md`, "The portable C: what
  EGA lines has left", which has all of it):
  - The screen of more than 25 rows is not tested in the portable C
    for now, on the user's word of 13 October 2026, until a system
    gives the program its rows: EGA lines was the one way there. The
    scenarios that were about such a screen are not run for the tree,
    those that passed through it have the steps left out, and `TODO.md`
    lists each with what of the program it tested, under "The tall
    screen: untested in the portable C until a system gives it rows".
    The code that goes by the rows stays as it is.
  - A second line out of a menu moves a click onto another item, where
    the first had moved it onto nothing: `mouse-menu` clicked a dark
    item by its place, and the place became Menu file edit. Look at
    every step that clicks or counts in a menu that changes, also
    those that an earlier removal looked at.
  - A scenario written twice in a table of `harness/deviations.py` is
    the second entry alone, and Python says nothing: add to the entry
    that the scenario has.
  - A command that changes what DOS has shown, as the original's EGA
    lines does when the screen gets shorter, changes screens many
    steps on, wherever a panel is off: leaving its steps out wants
    those rows in a patch.
- What taking Quick execute commands out met, the first setting to go
  from a dialog of the main program (`src/portable_c/DEVIATIONS.md`,
  14; `TODO.md`, "The portable C: without Quick execute commands"):
  - A dialog of one column that loses a line is a row shorter, and the
    row that it no longer covers shows what is behind it, another thing
    in every scenario: a patch for each scenario that shows the dialog,
    and no rule. Some thirty open the Advanced options only to untick
    Clear keyboard buffer for the editor, which stood below the line
    that went: each has a patch for that and a step in place of
    another. One entry of `in_place_of` for the same keys everywhere
    would have been wrong: the same keys tick the Clock in the setup's
    Configuration, in `setup-clock`. They are `in_place_in`, by the
    scenario's name, behind the step that opens the dialog.
  - A patch that is made again for a second deviation is read against
    the one before, not against the original: the patch of before,
    `git show HEAD:harness/goldens/portable_c/NAME.patch` in a file of
    `build/`, is made to the golden by `goldens.read(name, tree,
    patched=goldens.read_patch(file, tree))`, what the new declaration
    says is made to that, and what the golden with the new patch has
    otherwise is the reading list. That works where the steps are as
    they were; where steps are left out now, the old patch does not
    fit, and the screens are held against the original with the whole
    declaration made.
  - A function of `sys.h` that goes is in more places than those which
    the notes on the system's functions list: the toy that asks every
    function of `sys.h`, `harness/trial32/sys.c`; a table
    of `sys_dos.c` alone in `OWN_OF` of `harness/datacheck.py` and
    in `REVIEWED` of `harness/reads.py`; the count at the top of
    `sys_dos.c` of those that are made in real mode; the count of the
    functions in the README, in its text and in its table by group;
    and the layout of the block below the megabyte, where what stands
    behind the bytes that go moves down.
  - No test builds that toy, and a change to what a function of
    `sys.h` takes leaves it broken without a word: the piece F9 did, and
    S found it so. After such a change, `harness/trial32/build.sh sys
    32`, run from `build/` so that the compiler's `.err` lands there,
    and `harness/trial32/asked.py` for its pages.
  - The piece waited while two others landed, and what bringing it
    onto them met is in its section of `TODO.md`, "What landing it
    met": above all a count that both sides had each taken one from,
    which git merged without a word and which was wrong.
  - A call of DOS that was made for the feature alone goes with it and
    is said in the entry: DOS was asked for the current drive in two
    places for the command that went there first. No test counts the
    calls that the portable C makes for itself.
- The setup program, step 0 of the road, has its plan in `TODO.md`,
  "The setup program in the portable C: the plan": what the program is,
  what the portable C has of it already, how it becomes a part of the
  one program, the pieces of the work with the files of each, and how
  its scenarios are held against the original. Read it before any
  piece of that work. `harness/vcsetup/` has three scripts that hold
  the setup program's assembly against the main program's, with a
  README of their own: which routine is the main program's word for
  word, and how another differs.
- The harness judges the setup program, since the first piece of that
  plan. What making it so met, and what the pieces go by:
  - The original is two programs to the harness. A scenario is of one
    of them, `setup=True` for the setup program's, and the top of
    `harness/scenarios.py` says how such a scenario is written, started,
    tried, derived, held and declared: read "The scenarios of the setup
    program" there before writing one.
  - Two programs of the original in one machine have routines of the
    same bytes, and a trace cannot tell whose a block is that both
    have: `coverage.executed` places a program only by blocks that the
    other's file has not, `others`. Whatever runs a second program next
    to the one under test has to say so there.
  - A `.ran` next to a golden is of `VC.COM`, or of `VCSETUP.COM` for a
    scenario of the setup program, under the same name. `goldens.ran`
    keeps them apart; a tool that reads the lists of all scenarios by
    itself mixes offsets of two files.
  - The C translation is taken through no scenario of the setup
    program, by their kind, `no_setup` of `deviations.TREES`: a piece
    that adds one lists it for the portable C alone, which is the one
    tree whose tests it runs.
  - The test of the data has two originals, and knows a constant of the
    setup program by `St` in front of its name. Its top says what a
    word may hold that names a thing of the setup program: the tree's
    `StX`, the main program's `X` where that is the same bytes, and for
    a variable the place of the field of that name in the main data.
    Where it says "the original names X there, which the translation
    has not by that name", one of the three is missing, or the thing
    is kept under another name and wants a line in
    `SETUP_NAMED_OTHERWISE`. That table, not `NAMED_OTHERWISE`, is
    where a word of the setup program's object goes: an entry of
    `NAMED_OTHERWISE` for an object that a tree has not fails that
    tree's test, and the C translation has no object of the setup
    program. And a word that names a text which has no zero byte and is
    read by a count, as a picture of the colours is through `PagTab`,
    has its count in `SETUP_COUNTED`: without that the test takes a
    text that a word names to be read as far as a zero.
    `harness/reads.py` is the same in its way: an entry of `REVIEWED`
    for a file of the setup program, one of `SETUP_FILES` there, is
    not held against a tree that has no setup program, where an entry
    for a file that does not name its constant fails the test.
  - The test of the data can be tried before any code of a piece is
    there: a copy of the tree with one file of constants more, built as
    `harness/notice.py` builds its copy, `cmake -S` the copy with the
    two `FETCHCONTENT_SOURCE_DIR_` of `build/_deps`, and
    `harness/datacheck.py --source` the copy's `src/portable_c`. The
    copy's build is in a directory called `portable_c`, as always.
  - A screen of the setup program that is a dialog can be tried before
    the skeleton of setup is there, since `Dialog` is the main
    program's: in a change that is not kept, a command of the main
    program calls the piece's function, and `python3 harness/show.py
    --program build/portable_c/VC.EXE` with the keys of that command
    and then the steps of the scenario shows the box, to hold against
    the golden's rows inside its frame. Piece A did that for its four
    dialogs, and when the skeleton was there its scenarios passed as
    they were written (`TODO.md`, "Piece A, the option dialogs: done,
    and what a later piece should know").
  - Where the disks of a scenario differ in a file, the test says the
    two sums of its contents and no more. To see the bytes, a script
    of a dozen lines, kept in `build/`, runs the scenario by
    `scenarios.run` on the original, `setups.original_vc()`, and on
    the build, with `fat.hashlib` replaced by an object whose `sha256`
    keeps what it is given; `scenarios.of(name, "portable_c")` has the
    steps as that tree is taken through them. That is how the setup
    file of `setup-missing-saved` was found to differ in the 64 bytes
    behind the path of each panel and nowhere else, which is declared.
  - A line of a box that the setup program writes into before it shows
    the box, as Country info does, is an object without `const` in the
    file of tables, with the bytes that the assembly has there; the
    data's test holds it like a constant, and the code writes it
    through a far pointer made of `AsmSeg` and the object's offset.
  - A text or a button of the setup program that two pieces name is
    defined once, not static, in the file of the piece whose part of
    the assembly has it, and the other declares it `extern`: `StOk_S`
    and `StOkMn_S` of `stopts_t.c`, which the help of setup has too.
  - `python3 harness/goldens.py derive` derives all goldens again side
    by side, in a minute and a half on four processors, where it took
    them one behind the other. `git status` of `harness/goldens/` is
    clean afterwards, and that is the proof that a change to the
    harness has left the judging alone: it is cheap enough to do for
    every such change.
- The setup is entered and left, since the second piece of that plan:
  `stmain.c`, with a section of `vc.h` for all of setup and the plan
  itself for what the next pieces go by. What making a program of its
  own a part of another met:
  - Two routines of one name are held against each other before one
    is made to do for both: a diff of the two procedures, each cut out
    of its file after `iconv -f cp866`, says every line, where
    `harness/vcsetup/procs.py` only says which differ and by how many
    lines. What differs may be more than the flags that the other
    program lacks: the setup program's `Dialog` loads the place of its
    box whatever a flag says, and keeps a typed line elsewhere. Then
    each difference is one of three kinds, on the user's word that the
    setup is to use the main program's routines: what a scenario of
    setup shows and is no bug is an `if` on `StOpen`; what nothing of
    setup can show is left out, the main program's holding for both,
    and said in the comment at the routine; and a bug of the copy is
    not carried by a switch but asked about, as the 81st cell of the
    key bar was, which is fixed. The setup program's instructions that
    are left out so are still of the batch, as out of reach.
  - What a program does when it starts is not done twice, but look at
    what of it shows. The setup program sets the mouse up anew, which
    puts its pointer in the middle of the screen; it notes the row of
    the cursor, which is where it leaves it. And what the main program
    does around a command is done around setup too, where the next
    thing rests on it: the two interrupts are given back before
    `IniData` takes them, or it notes the program's own handlers as
    those to give back.
  - The screen behind setup is that of a command run: the main
    program writes prompt and command into the row of its command
    line, types the end of the line, and after the command the screen
    goes up by two rows where there is a key bar, one in `IniData` and
    one in `Init40`. With an
    empty row in place of that line and the same end of the line
    typed, every later screen is the original's too, so a scenario
    may start setup several times. A line of such a command that a box
    covers in part is not known to the harness for what it is, and
    stays in the golden: the top of `harness/scenarios.py` says where
    the lines get to.
  - A copy that runs on behind its table, as the notes above have it
    for the main program's defaults, may get into a file: `Default`
    copies 88 bytes of a panel from a table of 24, and the 64 behind
    it are in `VC.INI` when that is saved. Hold a file that the
    original writes against what the published tables can make before
    the scenario is written: `fat.read_tree` hashes every file of a
    disk, and a shim around its `hashlib` keeps the bytes. Here 17 of
    the 64 are of the binary alone, so no build can write that file,
    and the scenario has the main program read it and save its own.
  - What setup has of its own is in the main data, in buffers that
    the main program makes anew after a command: the original has
    nothing behind `DosBuf` left then, so nothing there is counted on
    across a command, here either. `struct StData` is laid over
    `MenuBuf`, with a `HOLDS` line that it fits. The file's image in
    it is an array of bytes with the panels and the sum by their
    places, not members: members would want `#pragma pack`.
  - An object of the setup program has `St` in front of its name, and
    the program has objects whose names begin so already: its
    `StErr_S`, the line for a file that could not be written, is
    `StStErr_S`, and `StErr_S` would be its `Err_S`. That one is not
    written: where the main program has an object of the same name
    and the same bytes that a file of tables can name, the setup's
    tables name it, `Err_S`, `Inc_S`, `YesNo_S`. And once one piece
    names the main program's, no piece may write the `St` one: the
    test of the data takes a word for the setup's own object if the
    tree has one, and fails the piece that named the other.
  - A screen that is still to come can be a function that waits for a
    key and returns: the item and Esc in one step then leave the menu
    as the real screen will, and the scenario of the menu runs the
    code that leads to every item before any of them is there.
  - The critical error of DOS is the main program's box whoever has
    asked for a file. `FtErr1` of `main.c` clears `StOpen` around its
    dialog, as it does the clock and the blanker, and has the next key
    bar drawn anew: the numbers of the two programs' bars mean other
    rows, and `BarPrev` would take one for the other.
  - The calls of DOS and the BIOS that setup makes have rows in entry
    6 of `DEVIATIONS.md` like any others, read from the setup
    program's assembly: a piece adds the rows of its calls.
  - A routine that the setup program alone has by its name may be the
    main program's by its text: `Main50`, the mouse at its menu, is
    `F32_70` word for word. `harness/vcsetup/procs.py` goes by names.
    Before translating such a routine, hold it against the main
    program's with the labels taken out of both. One that is the same
    and names nothing but routines that are the same too is called,
    not written again, and is listed in the setup's batch all the
    same, so that all of it has to have run in the setup program. One
    that names data is another matter: `HlpLine` is `Help77` by its
    text, and the table of colours that it names is the setup
    program's own.
  - What the original's setup program prints through DOS is sent
    nowhere in a scenario, all of it: the banner, and with it the line
    for a setup file that could not be written. A tree writes that
    line on the screen, so what DOS has shown differs from then on,
    and a scenario does not look at it after a save that failed.
  - A golden that would hold the original's edition, because a bug
    has changed the line that `harness/deviations.py` knows the banner
    by, is not mended there: the scenario first has that line gone
    from the screen. `setup-lines` does, for the corner that the key
    bar's 81st cell blanks.
- What taking out the program's own call met, with the entry of INT
  21h that was left for it (`src/portable_c/DEVIATIONS.md`, 8; "Piece
  E1" of the plan in `TODO.md`):
  - What a thing leaves behind when it goes may be a path that the
    original has too, by a setting: with no interrupt to give back,
    the registers that the program leaves when DOS ends it are those
    that the original leaves with TSR manager off. Such a path can be
    seen on the bench before it is written down: a scenario that is
    not kept, the steps of another behind the steps that set the
    original up so, derived from the original by `python3
    harness/goldens.py derive NAME`, read, and taken out again with
    its golden.
  - A figure that is left out of the comparison is looked at again
    when what made it follow the program's size goes: of the two words
    of `ended` one is the same on every build now and is compared, and
    the other, a segment as it came, is that of a program that lies
    behind the program under test and still follows its size. Leaving
    less out needs no asking; the patch is made again and both rows
    read.
  - A scenario that showed the thing through a second scenario's eyes
    says so where it is declared not run: `passed-on` named
    `handed-on`, and `stay` named `asked`, in `harness/deviations.py`
    and in two entries of `DEVIATIONS.md`. `grep` the names of the
    scenarios that go, not only of the functions.
  - An entry of `VARIABLES` of `harness/reads.py`, or of any table of
    the harness that is by name, fails the data's test when the object
    has gone: take it out with the object. A function of a batch that
    has gone is in `GONE` of `harness/ported.py`, and a batch may have
    every function there.
  - `harness/hostcheck.txt` has fewer places for such a change, and
    the test fails for fewer as for more: `--record`, in the same
    commit.
- What the help of the setup program met, the first of its pieces to
  have code, before the setup could be entered:
  - Every C file of `src/portable_c/` is in the next build, by its
    being there: a file that is written and not finished stops the
    build for whoever builds next, and a test that is running reads
    the tree in its first seconds. Compile a new file by itself first,
    with the command of the note on the trial of an old line above.
  - Code that nothing calls yet can still be tried: in a copy of the
    tree, made and built as `harness/notice.py` makes its own, with a
    few lines that call it from somewhere the main program gets to, F1
    of `Input` for the help, and `harness/show.py --program` on the
    copy's `VC.EXE`. What it draws is held by eye, or by a few lines of
    Python, against what `harness/show.py --setup` prints of the
    original for the same steps. The copy is not kept.
  - A routine of the setup program that walks through memory with
    addresses of 16 bits may go by where its buffer is in the
    original's one segment, not only by how long it is: the carry of
    an addition is a test there. The translation then counts in
    numbers of 16 bits from the original's address, and has that
    address as a number with where it comes from: `StHelp` of
    `sthelp.c`.
  - Where the original reads memory that it has put nothing into, a
    buffer behind a read that failed, what it shows on the bench is by
    what was there. The translation has zeros there, the scenario says
    in a comment that the original's screen is of that kind, and the
    plan has it among what was found.

- The words of the addresses, `AT`, `OFFSET`, `PANEL`, `MAIN_SEG`,
  `NEAR`, `SEG_OF`, `OFFSET_IN_SEG`, `MD_PLACE`, `MD_POINTER`, are in
  `vc.h` once, each with a body for DOS in 16 bits and one for a flat
  memory while there were two builds, and with the flat one alone
  since, and no file defines one for itself: `TODO.md`, "The portable
  C in 32 bits: the plan", has what each takes and gives, with a line
  as it was and as it becomes for every kind of place. What putting
  them there met:
  - A cast that is made a word is the same bytes of `VC.EXE`, and that
    is the check: `sha256sum build/portable_c/VC.EXE` before and after,
    which takes the build's second. Every form that the plan's table
    has was the same bytes at the first try, with the files' own
    definitions differing among themselves in a `const`, in a cast of
    the offset and in a macro between. One thing was not: the
    difference of two far pointers written out where `(u16)(u32)` took
    the offset, which is why `OFFSET_IN_SEG` is a word.
  - What a far pointer says beyond its address is lost where the
    memory is flat, and no second body gets it back: `WCB_OF` took the
    panel from the segment of a pointer to one of its files. Such a
    thing is handed on as an argument, which is other code in 16 bits
    too, and the scenarios judge it, not the bytes.
  - A second body is read by nothing but the compilers until the
    program runs in 32 bits. To try one, a file of a few lines that
    includes `vc.h`, has gcc take it for this computer, defines `md`
    after the include as a field of a structure that has 64K in front
    of it and a megabyte behind, and holds each word against the
    address that the plan's account gives: `#define md (arena.main)`
    behind the `#include` is enough, since a macro is expanded where it
    is used.
  - `harness/reads.py` goes by the text of a file of code. A constant
    that a file named and now names through a word of `vc.h` is one
    that the file "does not name": `AsmSeg` hands `Title_S` to
    `TABLES_BY` for that, where a word without an argument would have
    done for the compilers.
  - `wcc386` writes a file `NAME.err` into the directory it is run in
    when a file has a warning: run a trial of it from `build/`.
  - A name that `vc.h` takes is taken in every file: `dos.c` had an
    `OFFSET` of its own, which is `OFFSET_OF` there now, and `view.c`
    and `usermenu.c` each have a `WORD_AT` that is not the one that
    `dialog.c` had, which is why that word is in no header.
  - The host check does not count a pointer that is made of a number
    which gcc can work out. `MD_PLACE` of the address of a field is
    such a number in its second body, so `(struct DIRENT *)TRELIST`
    went uncounted in ten places of `treebox.c` and `tree.c`, and a
    file that is clean in the record may have one. To see them, have
    gcc read a copy of the tree, in `build/`, whose `vc.h` has
    `MD_PLACE` as a function, `extern u16 md_place(const void *);`,
    with the options of `harness/hostcheck.py`, and hold what it says
    against what it says of the tree.
  - A pointer to nothing has no place: `MD_PLACE` of a pointer that is
    0 is 0 on DOS and another number in a flat memory, and
    `MD_POINTER(0)` is no pointer to nothing there. Before a cast is
    made `MD_PLACE`, look whether the pointer can be 0, and before one
    is made `MD_POINTER`, whether the pointer is held against 0 behind
    it. The user menu cleared a pointer and handed it on as a number:
    it is set to `MD_POINTER(0)` now, which is the same bytes.
  - `OFFSET_IN_SEG` wants the segment that the pointer was made with,
    and a routine that is handed a far pointer alone has none to name:
    `NearFl` of `files.c` names the one that its only caller has the
    list in. The first body does not read the segment, so naming it
    is the same bytes, and nothing in 16 bits says whether it is the
    right one.
  - A cast with a step in it, `*(const u8 *)entry--`, is the same
    bytes with the step as a statement of its own behind it.
  - `NEAR` did something in 16 bits, and what it did was not always
    meant: it dropped the segment of a far pointer, and where the
    pointer was not into the main data, the original read at that
    offset in the main data. In a flat memory the cast does nothing
    and the pointer is read where it points, which is the original's
    bug gone with no word said and no scenario failing: `ChgDisk` in
    Go to of the file search was one, for all of the 32 bits until the
    piece F4 met it, and the user then decided that it stays fixed
    (`src/portable_c/DEVIATIONS.md`, 15). Before a `NEAR` is taken
    out, read whether its pointer is into the main data; where it is
    not, that is a deviation to hand back.
  - `SEG_OF` of a pointer with `AT(segment, 0)` behind it is the
    paragraph that the pointer is in, not the pointer: up to 15 bytes
    in front of it. Where the original goes by a segment so, the
    pointer that takes its place has that taken from it, written out:
    where the file search reads a file to, `FindFil` of `find.c`.
  - The capital of a letter costs a call into real mode and back in
    the program of 32 bits, some 320 instructions, `CaseMap` of
    `dos32.c`: a loop that asks `UpCase` for every byte of a text
    takes three and a half times the original's clock for that.
    `TODO.md`, "F4, the viewer and the file search", has what was
    counted; it stays so on the user's word.
- A panel is a pointer, `struct WCBdef *`, since the piece F2
  (`TODO.md`, "F2, a panel is a pointer"), and `PANEL` of the words
  above is gone: the four panels are `CurrentPanel`, `OtherPanel`,
  `LeftPanel` and `RightPanel` of `main.c`, beside the main data. What
  a sweep of that kind met:
  - Compile every file by itself and read all that is refused, before
    anything is mended: the build stops at the first file, and what one
    file refuses says how another is to be written.
  - The compiler refuses a pointer where a number is taken, the
    reverse, and the two held against each other. It does not refuse a
    pointer handed to a word of `vc.h` that casts what it is handed,
    `AT(panel, 0)`, `OFFSET_IN_SEG(panel, x)`: look for those by `grep`.
    Nor a list of names behind a type that a script has changed: `u16
    panel, size;` made `struct WCBdef *panel, size;` declares a
    structure.
  - A structure that a function returns and that grows over four bytes
    comes back through a pointer to room in the caller's frame, and
    `wcc386` gives each call a room of its own: `Command` of `main.c`,
    which calls some fifty commands that return `struct Onward`, has a
    frame of 396 bytes where it had 12. The frame is the `enter` at a
    function's first line in the listing.
  - A function that had a second name for what a word made, `wcb =
    PANEL(panel)`, has two names for one pointer when the word is gone.
    Taking one out by a script changes the bytes of the program, by
    what the compiler keeps in which register, so it is judged by the
    scenarios and the images like the sweep.
  - The images of two builds whose code differs differ in three things
    with no fault in either, which `harness/dataimage.py` knows since:
    where the 640 KB begin, if a variable was added in front of them;
    the stack pointer at the last call of DOS; and one byte behind the
    name in the record of a search, `DTA+41`, which by what was seen
    of the two files is a byte of the program's own file left in DOS's
    memory by the extender, of the table of its relocations: that is
    inferred, the memory was not looked at. The program copies a name
    as 13 bytes, so
    that byte is in `TmpBuf` and `TmpErr` after a copy, where the tool
    does not know it: a difference there that is the pair of bytes of
    `DTA+41` is this.
- What a dialog's fields are in is a pointer, since the first half of
  the piece F3 (`TODO.md`, "F3a, what a dialog's fields are in"):
  `Dialog` and `DialBox` are handed `&md` or a panel, `data`, and read
  a field at the offset that the table names; `ClsLine` is handed the
  main data, and in one place a panel by `PANEL_AS_MAIN` of `vc.h`,
  which is `MAIN_AS_PANEL` the other way round. What that met:
  - A sweep can be all of one form: every one of the 72 calls had
    `MAIN_SEG` or `SEG_OF(panel)` written out at the place of the
    segment, and the compiler refused nothing. A script that reads a
    call from its name to the bracket that closes it, over as many
    lines as it has, and changes one argument by its place, is surer
    there than a pattern for a line.
  - A parameter that is a `void *` takes any pointer, and a 0 for a
    pointer to nothing: the compiler holds only that no number is
    handed over. What each call hands was tallied by the same reading
    of a call, and that list is the check. Where one kind of thing is
    meant, the parameter has its type, as `ClsLine`'s has, and what is
    handed otherwise goes through a word that says so.
  - `AT` cut the offset that it was handed to 16 bits. Where a pointer
    and a number added to it take its place, look at the type of the
    number: a `u16` is cut as it was, and wants no cast; anything
    reckoned in an int wants its cut written.
  - An argument may be two things by a flag of the table: `include` of
    `Dialog` is the text that ^I stands for, or the size of a box that
    is on the screen already. A number carries both and a pointer does
    not: read what a parameter is assigned to before it is retyped.
- A place of the main data that a word of it kept is a pointer beside
  it, since the piece F7: the tree's cursor, its first line and the
  current directory in it, where the history is, the command to run
  next (`TODO.md`, "F7, the tree, the history, the commands"). What
  making them so met:
  - A number that is a place may be read as a number somewhere, and a
    pointer in its stead is then another thing there. The cursor's keys
    leave the tree's two addresses in BX and DX, which go round the
    main loop to Rename, and that goes by whether the two are 0: an
    address never is, and an offset in the list is at the root. Before
    a place becomes anything else, read every return of the routines
    that hand it on, to where the number ends: such a place keeps
    `MD_PLACE`, with its reason. So do the addresses that the original
    hands the BIOS with a register unset, `WAS` of `exec.c`.
  - Where one routine goes through a panel's files or the tree alike,
    by numbers of 16 bits, the tree's entry is an offset in the tree's
    list as the file's is one in the panel, `TREE_OFFSET` of `vc.h`:
    the arithmetic is differences and comparisons, and stays what it
    was. Arithmetic that only one list has is written in pointers, and
    there a test that was for wrapping at 16 bits is read and said, as
    in `TrCr20` of `tree.c`.
  - A place of 0 that meant none is a pointer of 0, and a word that
    was 0 because nothing had set it is one too: the image's tool
    takes both for nothing (`PLACE` of `harness/dataimage.py`).
- A word of a table that names something, a line of a box, the box
  and the buttons of a dialog, is a pointer in `tables.h`, as long as
  a pointer is, and code never reads through it: it asks `NAMED` of
  `vc.h` what the word names, as an offset in the tables' segment, and
  `NUMBER_OF` where the word holds a number. `TODO.md`, "P3, the
  tables read as what they are", has the two. What reading the tables
  so met:
  - Whether the bytes of the 16-bit program stay is a matter of how
    the read is written. A structure laid over what was read by its
    offsets, `head->lines` where `head[4]` stood, is the same bytes. A
    far pointer that is stepped where an offset was added to is not:
    `DialBox` is 6 bytes longer. And a far pointer to a table kept in
    a variable of its own was 10 bytes more in `F39_01` of `main.c`
    and moved every local variable of the function, where the same
    read in one expression, `NAMED(*(type __far *)AT(code,
    OFFSET(table) + index * sizeof *table))`, is the bytes that the
    function had.
  - A word may be a name or a number by the kind of the thing that it
    is a part of, as `mCtrl` of a dialog's field is, and code may read
    it before it has looked at the kind: the mouse's loop of `Dialog`
    did. Such a read is made once for each kind, behind the test.
  - The objects of two builds differ in every file, by what the
    compiler writes into them besides the code. The listings do not:
    `build/portable_c/*.lst` against a copy of the build before says
    which files changed, and within one, the bytes of each routine,
    without their addresses, say which routines.
- A table is handed on by a pointer to it, since the second half of
  the piece F3 (`TODO.md`, "F3b, what a table is handed as"), and the
  two notes above are of before: `OFFSET`, `AsmSeg`, `TABLES_BY` and
  `NAMED` are gone, and a word of a table that names something is read
  as the pointer that it is. What making it so met:
  - A file of code declares a box as what every box begins with,
    `extern const struct BOXTOP Name;`, and a dialog as its header,
    `extern const struct BOXHD Name;`, and hands on `&Name`; a text,
    buttons and colours stay `extern const u8 Name[];`. The compiler
    then holds the three apart at a call, which the names do not: a
    `_B` is a dialog in one file and a box in another. It found two
    variables that had held now a box and now a dialog.
  - What it does not refuse is a 0 where a pointer is asked for, and
    any pointer for a word of a table that is `const void *`. A script
    that reads every call and the files of tables says what each call
    hands, and whether a box whose lines have `^I` is handed a text.
  - `harness/reads.py` goes by the text and knows both forms, the C
    translation's and this one. A variable that keeps a box for a
    routine of `READERS` is known by its name, one of four there:
    under another name the place is "not known" and the data's test
    fails.
  - An offset that the assembly left in a register for a text which
    nothing reads, the size of a window, a count of files, 76, was
    handed on as an offset among the tables. A pointer cannot be that:
    it is 0 there, with the comment that says what the assembly has.
    Where the register is on a text of the main data, it is that text.
  - A script that takes `OFFSET(Name)` for `Name` writes wrong C where
    the offset was reckoned with inside the brackets, `OFFSET(Table) +
    n * sizeof *Table`: read every place where a table's name is
    followed by `+`.
  - `harness/layoutcheck.py` still held the tables to the 64K round
    `Title_S`, which nothing rested on any more: that was a limit of a
    test, left for the session, and went with the megabyte in F10.

- The program of 32 bits is linked by the build since the piece B2
  (`TODO.md`, "B2, the program"), `build/portable_c/VC.EXE` and
  `debug/VC.EXE` there, and the tests `portable_c-layout` and
  `portable_c-debug-layout` hold where the linker has put things. What
  linking it met:
  - The order of the segments is the linker's command's, by classes,
    and a class is linked whole: the constants and what has a value
    are both `DATA`, `CONST2` and `_DATA`, and nothing can be put
    between them. The stack and the main data have classes of their
    own by the two `#pragma data_seg`, `STACK` and `MAIN`.
  - What has no value is not in the file only where nothing with a
    value stands behind it, and of the class `BSS`. The main data has
    none and is in the file all the same, as zeros, also with nothing
    but that class behind it: the segment that its `#pragma data_seg`
    names is one with a value to the linker.
  - Within a segment the objects are in the order in which the linker
    is given them, so which file's variables stand behind which is the
    order of the link's files: `dos32.obj` is last for that.
  - `option stack=` makes a segment of the class `STACK` as long as it
    says, also one that the source has shorter. The build does not
    say it: `LenStk` is the stack's length.
  - The name of the file is in the file: two links of the same
    objects under two names differ in one byte. Under one name they
    are the same bytes, so the program of 32 bits can be held against
    itself as the one of 16 was.
  - `wcc386 -os` gives a function no instructions of its own where
    the function behind it in the file has the same: one that does
    nothing, or that is the same call. The map has several names at
    one place then, nineteen of `dos32.c` at three. With `-od` each has
    its own.
  - It puts pieces of a function in front of its label, behind the
    function before: an address of QEMU's log is not always of the
    function whose name precedes it in the map. The listing says.
  - `wdis -s=FILE` puts the lines of the source into a listing of an
    object that was compiled with `-d2`; without the linker's `debug`
    directive nothing of that is in the program.
  - A check that is to fail for a wrong link is tried with wrong
    links, by hand, from `build/`: the objects of
    `build/portable_c/` with another `order`, or with a toy object
    of constants or of memory without a value among them. The table
    in the plan has which link broke what.

- The tables of the program of 32 bits are held by `portable_c-data`,
  which was a test of their own, `portable_c32-data`, beside that of
  the 16-bit program: `harness/datacheck.py` with `build/portable_c`
  and `--source src/portable_c` (`TODO.md`, "H4, the test of the
  data"). What "Moving the data of a file to C" says
  of `c-data` holds for it, its messages and its tables, with this
  besides:
  - A word of a table that names something is a pointer of four bytes
    there, so a box, a dialog and a table of pointers are longer than
    the original's and are held field by field. Which fields are
    pointers the test reads in the declaration: `BOX`, `BOX1`, `BOX2`,
    `DIALOG` with a count, at the start of a line, an array of
    pointers, or an array of a structure of nothing but pointers. An
    object with pointers that is declared in another way is taken for
    bytes and fails, "has an address at +N ..., where its declaration
    has no pointer": `declared_shapes` and `SHAPES` there want it.
  - The place in a message is counted as the original lays the object
    out, which is what `DECLARED`, `NAMED_OTHERWISE` and `COUNTED`
    count in, and the message says where that is in the object itself:
    "StDfCol_P+10, which is +12 of it as it is here".
  - Where the original names something, the pointer has to be an
    address that the extender sets: "the translation has the number
    1234h, in a word that could name something, and no address that
    the extender sets" is a `NUMBER` or a 0 where a name belongs. Where
    the original has a number, it has to be none: "the translation has
    the address of X, ..., where the original has a number and names
    nothing". The first of the two passes in no program; the second
    is a mistake that only this test names for what it is.
  - A change to a file of tables was held by both tests while there
    were two, and the 16-bit one said the same in fewer words. There
    is the one now.
  - Data that only this program has is that of `sys_dos.c`. A constant
    of that file wants a line in `OWN_OF` there, and one in
    `REVIEWED` of `reads.py` for where code reads it; `reads.py` leaves
    out the file of the system's functions that a program is not built
    of, `SYSTEM_FILE`. A variable of that file wants nothing, with a
    value or without: those are left out as what they are. In any other
    file a variable with a value is held like a constant.
  - `wcc386` puts nothing between the fields of a box or a dialog, nor
    between two objects of a file: every object is as long as the
    original's but for its pointers. It does round what a file has of
    a segment up to four bytes, so the linker's map does not say how
    long the last object of a file is; the object file does.
  - A box cannot be declared before it is defined: each `BOX(n)` is a
    structure of its own, and a second declaration is "Type does not
    agree with previous definition". Buttons can, being arrays.

- The program of 32 bits went through the scenarios by a test of its
  own, `portable_c32-scenarios`, from the piece H3 (`TODO.md`, "Piece
  H3, the scenarios' test") until it became the portable C, and goes
  through them by `portable_c-scenarios` since. What that met:
  - While it failed scenarios it was held to a record,
    `harness/pass32.txt`, and not to passing everything, and was a tree
    of its own to the harness, `portable_c32`, with the source of
    another and with what was declared for it alone in an entry of its
    own. All of that went with the switch: the program is held to
    every scenario that the portable C is taken through, a fix wants
    no record written, and what was declared for that build alone is
    declared for the portable C, `editor-big`, which is not run, entry
    13 of `DEVIATIONS.md`. A scenario that is not run has to be named
    by an entry there, or the test fails. Nothing holds that the
    program still does otherwise in it.
  - That program has 640 KB of its own where the program of 16 bits
    has what DOS leaves it, and a scenario that takes the original to
    the end of its memory may pass without taking this program there:
    a count of free memory is left out of the comparison, and such
    steps were written to end on the same screen whatever room a build
    has. `editor-full` and `copy-big` are so (`TODO.md`, "The editor's
    room: untested in the program of 32 bits"). Before a scenario that
    passes is taken for a test of a limit of memory, look at what the
    program shows as free in it, `harness/show.py --program`.
  - A scenario of that test was ended at its first screen that
    differs, while the record was; `harness/compare.py` does so with
    `--first`.
    What the keys of a scenario do in a program that has left, or that
    stands in another box than the scenario takes it to, is not worth
    reading, and such a program waits for keys that do not come: DOS's
    prompt takes no click of the mouse, and a step with one waits
    until its time runs out.
  - When the program leaves by itself, DOS goes on in the bench's batch
    file to the command that switches the machine off, and QEMU ends
    with the driver's script half played. A check that looks at what
    the driver has written must look once more after the machine has
    ended, and a pause asked of a machine that has just ended fails.
  - What is looked for in a program's image for every block of QEMU's
    log is looked for in its code alone. With the 640K that the
    program hands out in the image, a scenario took 14 seconds where
    it takes 3: Python's threads take turns at such work, so four
    scenarios side by side waited on one another.
  - A second copy of the program, started from the command line of
    the first, is loaded behind it, where `copies` of `harness/le.py`
    works it out from the program's length: the place moves with every
    change of the program, and is written nowhere. The driver of the
    tests is told the places of three copies and tells which of them
    idles by the programs of that name in DOS's memory in front of it
    (`TODO.md`, "C2"): not by who ran whom, since FreeCOM names itself
    as the one that ran it, at 16h of its PSP, also when a program has
    run it for one command.
  - The time that a step has before the program counts as stuck is
    of the host's clock and of the machine's own, both
    (`Machine.wait_for_end` of `harness/dos.py`): a step that is given
    up on a busy host has also had that many seconds of the machine's
    clock, counted in instructions, and is no slow machine. A step
    that has the driver wait has the seconds of its wait besides.
  - While the program of 32 bits idles, a second of the machine costs
    the host four times what it costs for the 16-bit one. QEMU pays
    some ten microseconds for every change of the processor's mode,
    thirteen at a round, and as much again for a block of the
    extender's that it translates anew at every interrupt, since
    DOS/32A writes the interrupt's number into an `INT` of its own.
    `TODO.md`, "T, the time of a wait", has the measurements and why
    nothing in the program mends it. Such a thing is measured on a
    host by itself or counted, in instructions, flushes and blocks:
    the processor time of a QEMU follows the load as well.
  - The extender takes all of the memory above the megabyte while the
    program runs, and a command of the program is told that there is
    none: the scenario `command` shows that in what `mem` prints, and
    is held by a patch of that build's own, the difference being
    declared as it is on the user's word (entry 1 of `DEVIATIONS.md`;
    `TODO.md`, the plan, 9 of what was open).
  - A number of 16 bits that stands for one below 0, FF60h for 160
    back, and is added to a pointer goes that far on in a flat
    memory, and nothing at the place says so: `T71_00` of `edit.c`,
    which moves the rows of a window in the screen buffer, was nine
    of the fourteen scenarios that the program failed at first. The
    sum is made in the number and the pointer of that: for a pointer
    into the main data `MD_POINTER(MD_PLACE(to) + step)`, for one
    into a segment `AT` with the sum as its offset. On DOS that is
    the code it was, and the file the same bytes.
  - Such a write shows by where it lands, or not at all. That one
    made three kinds of screen, by how many rows the editor's window
    had, and none in the viewer, which made it at every line that it
    went up. Scenarios that fail alike by the look of their first
    screen are not thereby of one cause, nor those that fail
    otherwise of several: mend the first and run them all, `python3
    harness/compare.py build/portable_c/VC.EXE NAME --first`, a few
    seconds each and as many side by side as there are.
  - To see what a mend changed in the memory, keep the program of
    before beside the build in a directory named as its tree is,
    `build/NAME/portable_c/VC.EXE` with its `VC.MAP`: the harness
    takes it for the same tree, and `harness/show.py --program` shows
    the two side by side. The editor's F4 is a view of the memory at
    `ViewSeg` in hex.
  - Which functions of the batches ran is asked of that program, as
    of the C translation. A function that the program has under
    another name may be static in its file, as the entry of the
    critical error is, `DeviceEntry` of `dos32.c` for `FatErr`:
    `RENAMED` of `harness/ported.py`, which finds a static name in the
    linker's map (`TODO.md`, "W, the switch").
  - What that program does otherwise than the original, declared or
    not, is in one list with what the user decided of each: `TODO.md`,
    "The program of 32 bits: what differs from the original, for the
    user". A difference of it that a piece meets goes there, and to
    the user, before it is declared.
  - What a program asks of DOS, call for call, with the segments that
    DOS gets in real mode, `harness/trial32/calls.asm` writes to a
    file: it takes INT 21h and runs the program under the program's
    name, so the driver drives it. That is how the registers and the
    calls of entry 6 of `DEVIATIONS.md` were seen for the program of
    32 bits. Under the extender, DS of real mode holds what AX holds
    at a call that takes no address, and is not on the program's data
    at any.
  - `harness/notice.py` tries a change on the program, through its
    scenarios, as on the C translation: the file that it is given says
    which tree.

- The numbers of 8 and 16 bits are held by a test of their own,
  `portable_c-width`, since the piece N0 of step 2 (`TODO.md`, "N0, the
  finder, the record, the types", which has what a cluster does, in
  order). `harness/width.py` finds every such number that a file
  declares, and `harness/width.txt` has them, with a reason for each in
  what has been sorted and the counts of the rest. A cut, where a
  number that may not fit by the types is put into so few bits, is no
  entry there since the piece N0d (`TODO.md`, "N0d, a cut is said by a
  cast"), and is not counted: where clang names it, in a file that is
  sorted, it is a cast in the code, which the test holds by clang's
  warnings; where clang names nothing it stays as it is written, on
  the user's word of 13 October 2026, and the finder prints it for
  whoever asks. Using it and keeping it up:
  - It fails for a count that has moved, fewer as well as more, in a
    file that is not sorted, and prints the lines of the record that
    differ: `python3 harness/width.py --record`, and read what moved.
    A number that is new in something sorted, or a place of the kind
    `sign`, is written without a reason, and the test fails until the
    reason is under it in the record, or the number is widened.
  - It fails for a file that is sorted where clang says something
    under `-Wconversion`, `-Wsign-compare` and the two of the
    booleans. Each place gets the type that holds its number, first,
    or a cast. Nothing is asked beyond what clang names: it leaves out
    on purpose arithmetic on numbers of 8 or 16 bits that is kept in
    as many, `word = word + 1`, `byte = a - b`, and every step, `++`,
    `+=`, `|=`, and such a place on a variable that stays narrow for a
    reason stays as it is written, with no cast. `python3
    harness/width.py --warned FILE` prints what clang says of a file
    that is not sorted yet, and `--show FILE` the finder's cuts, which
    are read when it is decided what is widened and held by nothing.
  - A cast that is written is the same instructions as the cut that
    was not, for every form that the first five files have: `VC.EXE`
    was the same bytes with 12 of them written. So the check of a
    cluster's casts is the listings of its files against those of the
    build before, by `wdis`: what differs there is of a type that
    changed, not of a cast.
  - A cast in a macro is a cast, and clang reads it where the macro is
    used; but the place does not say it then. `blank.c` had `LOW` for
    `(u16)`, used once, and has the cast.
  - A function that is widened moves the cuts of other files: one
    that returns 32 bits where it returned 16 is a cut more in every
    file that keeps its answer in 16 bits, and one that takes 32 bits
    is a cut fewer where a caller counted on it to cut. Those are for
    the cluster of that file. The record does not count them, since
    N0d; in a file that is sorted clang names such a caller, and the
    test fails until it has its cast.
  - Before a parameter is widened, `--show NAME` lists the calls of the
    tree that hand the function more than it takes. Afterwards nothing
    lists them.
  - A file may declare another file's function for itself, and that
    line changes with the function: `grep` the name in the whole tree.
    The compiler stops for one that was left.
  - What the finder does not name, and a widening can get wrong: a mask
    that was written for 16 bits and now cuts, `length & 0xFF00` for
    all but the low byte, which is `length & ~(u32)0xFF` of 32 bits; a
    `~` or a `-` of the variable; a line that holds it against 0FFFFh,
    for a number that got there by stepping 0 back; and that a number
    is only ever 0 or 1. Each is read, at every line that names the
    variable.
  - A number that the program counts on wrapping at one place is
    widened, and the cut is written there, with the word `wraps` in a
    comment at the place that says what is counted on:
    `more = (u16)((second << 8 | second & 0x7F) - 1);` in `LnStr` of
    `line.c`, where `more` was a `u16` and the line had no cast. A
    cast without such a comment says that the number fits, by the
    lines just above; where it fits by something further off, a
    comment says by what.
  - A character, a colour, a cell of the screen, a place of a row and
    a column, and what the system answers in stay as wide as the
    translation has them, 16 bits as a rule: half of the numbers of the
    first five files did. Each has one word for its reason, of a fixed
    list, `WORDS` of `harness/width.py`, which the record prints at its
    top with what each word says; what is not plain goes behind a
    colon, and the table counts them in the column `by word`. The code
    has no type for any of them, by the rule of "The portable C". A
    single word that is not of the list fails the test. A word is added
    to the list for a thing that recurs and has none.
  - A reason is written into `harness/width.txt` by hand, or for many
    at once by a script of a dozen lines that is kept in `build/`: the
    entries are divided by empty lines, the first line of one is its
    key, a place of a sign has its line behind `    | `, and what
    follows is the reason. `--record` puts the file into its form
    again and keeps them.
  - `harness/int16.py` knows `bool` and `i32`. Its test stays, and a
    widening moves its places too: one that makes 32 bits of a
    difference of two numbers of 16 is a new place, `widen`, with its
    reason.

- The panels and the room are three blocks since the piece F1 of step
  2 (`TODO.md`, "F1, three blocks where there was one"), and nothing
  is moved: a panel has 64K from the start. What that met:
  - A piece that changes where things lie in the memory cannot be held
    to the images of the build before: the room differed in 142 of 155
    scenarios, by where the old list of a directory lies. What held it
    was a third build, not kept: the source of before with three lines
    changed so that it lays the memory out as the piece does and is
    otherwise the old code. Against that the images were equal. Make
    the build that separates the layout from the code before reading
    17,000 differences.
  - What is declared as changed is not looked at. `--changed "behind
    the panels"` or `--changed TmpBuf` passes anything there: declare
    a word or a count, not a buffer.
  - A piece that changes the image's tool saves its "before" with the
    tool as it was.
  - The clock's separator is a colon in the second half of a second:
    it is in the program's copy of the screen, and a build that is
    much slower or faster to a step has the other. `save --early` is
    too fine a trial for it.
  - The 16 bytes of a pattern round each block are looked at when a
    scenario ends, and a scenario that has written into them fails
    `portable_c-scenarios` with the block and the byte. The line says
    "in front of block 1" of a write behind the end of block 0: they
    are the same bytes.

- A number of the portable C is given its width and its sign by the
  rule of `TODO.md`, "The rule for a number", and held to it by the
  test `portable_c-width`, whose top, `harness/width.py`, says what it
  finds. What turning the first five files to a sign met:
  - Made an `i32`, a number of 16 bits leaves every expression as it
    was, since C reckoned it as an `int` already; the listings of
    `wdis` for a file as it was and as it is differ only in jumps that
    ask with a sign and in shifts that keep it, and each of those is
    the reading list. What does change is a variable that cut what it
    was given: `digit = *text - '0'` with `if (digit > 9)` behind it
    took a character below '0' for no digit because 65,520 is above 9,
    and -16 is not. The finder names such a store as a cut; the cast
    that is written there is not noise.
  - `wcc386 -os` loses a cut to 16 bits between a signed byte and a
    number of 32: of `u16 line = (u16)(signed char)c;` it hands
    `line`, to a parameter of 32 bits or into `line + first` and `line
    < first`, as the byte with its sign in all 32 bits, FFFFFFFEh for
    FEh where C says FFFEh. `-od` and clang do what C says. The
    original's `CBW` is not written as the register: the byte is
    taken as it is, into an `i32` where it is reckoned with, or with
    its sign into an int where that is meant, and no word of 16 bits
    without a sign stands between. With a `short` between,
    `(u16)(short)(signed char)c`, the compiler keeps the cut by how it
    works, and the tree had that on 33 lines before; a mask or
    arithmetic in its place comes out right by chance. `vc.h` says so
    at "A byte with a sign made a word"; `TODO.md`, "N0c", has which
    forms are lost, in which uses, and that every other cast that
    narrows is kept, and "CBW, the byte made a word written as what it
    is", how each of the 33 was read. The test
    `portable_c-cut` fails for the form anywhere in the tree,
    and holds the compiler to the instructions of
    `harness/cutcheck.c`: before a cast is relied on in a use that
    the file has not, a function is added there, and its
    instructions read. Where the test fails for a function called
    Lost, the compiler has changed and the fault with it: pin it
    down again before anything is rewritten.
  - A fault of the compiler is pinned down fastest by running its
    code on this computer: the functions compiled with the build's
    options, a driver compiled with `-bt=linux`, the two linked by
    `wlink system linux` with the C library of the same download, and
    what the program prints held against gcc's of the same source. A
    matrix of forms by uses is a script's work then, 1,900 functions
    in a minute. A function may be declared to the driver with wider
    parameters than it has, to hand it rubbish above its arguments.
  - The objects of two builds in two directories differ in length by
    the paths that are in them: hold listings against each other, or
    the places of the functions in the two maps, not the lengths.
- A word of a buffer of the main data that leaves it for a pointer, as
  two words of `TmpBuf` did in `copy.c` (`TODO.md`, "F6"), is "not set
  or read here", and its bytes may still be read: what is copied by a
  count and not by a text's zero carries them along. `DialBox` copies
  81 bytes of a box's text whatever its length, to `TmpErr+1` or
  `TmpErr+101`, so every text that a table names within 81 bytes in
  front of the word takes it with it, and the image shows the bytes
  there, `TmpErr+129` for `TmpBuf+128`. Before such a word goes, list
  those texts from the files of tables, `{ MD_AT(TmpBuf[n]), how,
  length }` and the names that a file defines for one, by a script, and
  read whether the word's bytes can lie in front of the text's zero.
  Reading the copies of the code alone, 68 bytes of a path, 13 of a
  name, found none, and the image found this one.

- A routine that reckons with segments can be held against its flat
  form on this computer before any scenario runs: the old function and
  the new cut out of the two files by a script, compiled by gcc with a
  few lines in place of `vc.h` that give `AT` and `SEG_OF` an arena of a
  megabyte or so, and called alike on data made for the edges, lines of
  64K less a few bytes, of 3,900 and of nothing. F5 did so for the
  editor's lines (`TODO.md`, "F5, the editor"), and the first things
  that it found were its own: a field that a function reads and the
  test had not set, after which the old code and the new looped for
  ever alike, and a room smaller than the program ever has, where both
  copied four gigabytes. Set every field that either reads, and give
  the program's least of what it has.

- What the last piece of the addresses in the common files met (F8):
  - A register of the main loop that the original sets to a place and
    nothing reads as it is left is not set, with a comment that says
    what the assembly has there: a pointer cannot go into `struct
    Regs`. Which commands read SI and DI was read from every `r->si`
    and `r->di` of the tree, and from where the loop asks for them.
  - A place that the original leaves in a register which a later
    command reads as a number stays the number, `MD_PLACE`, with its
    reason: the leak of `F00_50` of `keys.c` into BX and DX.

- What taking the megabyte down met (F10; `TODO.md`, "F10, the
  megabyte goes"):
  - A function that does not return is `NO_RETURN` of `compiler.h` (of
    `vc.h` until 5a) in front of its declaration, Open Watcom's
    `__declspec(aborts)`, which is
    the same instructions as `#pragma aux name aborts` behind it; on a
    static function it goes on the definition, `static NO_RETURN void
    Round(void)`. `_Noreturn` and `__declspec(noreturn)` are taken by
    `wcc386` without a word and are not the same: the caller goes on
    behind the call.
  - A macro that nothing uses is no code, and neither is a
    `__declspec(aborts)` where the `#pragma aux` stood: both programs
    of the build were the same bytes after the piece as before it,
    which `sha256sum` of `build/portable_c/VC.EXE` and of
    `debug/VC.EXE` says.
  - A rule of the test of the layout that goes takes its row of the
    figures with it: `tools/figures.py` read `room` of
    `harness/layoutcheck.py`, which is gone.
- The tree is held to clang-tidy since the piece 5b, by the test
  `portable_c-tidy` (`portable_c-stub` until the stub went, in L7) and
  `.clang-tidy` at the top (`TODO.md`, "5b, the stub: done, and what
  step 6 should know"). What that asks of code that is written:
  - A function that only its file calls is `static`, and one that other
    files call is declared in `vc.h`, once: a file no longer declares
    another's function for itself, which clang-tidy names as a function
    that no header declares. A function of a batch may be static:
    `harness/ported.py` finds it among the static symbols of the map.
  - A macro that assigns to an argument has it in brackets, `(i) = 0`.
  - What the analyser names where the main data is read across its
    fields is listed in `EXPECTED` of `harness/tidycheck.py` until the
    piece that reads it as bytes at an offset has mended it; any other
    place fails the test, and so does one of those that is gone.
  - The test takes a minute and a half; `python3 harness/tidycheck.py
    --compiled build/portable_c/posix/compiled.txt --posix-compiled
    build/portable_c/posix-unit/compiled.txt` runs it by itself.
- What making a word of a buffer of its bytes met (AL; `TODO.md`, "AL,
  words made of their bytes"):
  - A cast of a pointer is not the only way to an unaligned read. The
    structures of `vc.h` and of the tables are packed and lie at any
    place, and a member read as a member is right; a pointer taken of
    one, or of a buffer of the main data, and read through its own
    type, is not: `MenuBuf` and `TmpBuf` begin at odd places, and a box
    may name `MenuBuf+500` for what it covers. The lines of a box,
    pointers behind its texts, were such a read too, eight bytes on
    Linux at a place that eight does not divide. The sanitized build
    for Linux finds them only where a run gets to them: give it a run
    with files, dialogs and the editor, not only the panels.
  - A word read through `GET16` is its bytes the low one first; where
    a buffer only keeps what it is handed and gives it back, as `SavWin`
    and `ResWin` do with cells, `PUT16` and `GET16` keep it so on any
    machine, and on DOS they are the bytes that were kept before.
  - Four functions that put their instructions in place, some two
    hundred times, made the code of the program for DOS 4K longer:
    every image differs in the room, and the padded build of `main` is
    what holds them.

### Moving a file to the system's functions

What the program asks of the system it asks through
`src/portable_c/sys.h`, functions named for what is asked, with
`sys_dos.c` as their one file for DOS: read the top of `sys.h` before
anything else. Every file has gone over. None fills a `struct DosRegs`
or a `struct BiosRegs` and hands it to an interrupt, as each did and as
the C translation does: `Intr21`, `Int21d`, `Int10` and their like are
gone from the tree, the two structures are `sys_dos.c`'s own, and `python3
tools/calls.py` counts no such call. These notes are what the move met.
They are kept for whoever writes a call of the system anew, brings code
over from the C translation, or adds a function to the interface: the
steps and the table say how one call goes over, and the last notes how
a function is added.

Since the 16-bit build went, on 13 October 2026: `dos.c` in the notes
below is the file that the calls moved into, which went with that
build, and what it held is `sys_dos.c`'s, written for a flat memory
under the extender (`TODO.md`, the pieces D1 and D2 of the plan for the
32 bits; what D2 met is further down); `resident.c` is `exec.c`. What a note says of `sys.h`, of what a call hands back and
of the traps holds as it stands. What it says a function of `dos.c`
does, with registers, segments, a stack or `_asm`, is of that file and
is history. The table was held against `sys.h` by a script when that
build went: every function that it names as what a call is, is
declared there, but for those that it says went with a command, and
the rows that named something of `dos.c` say what `sys_dos.c` has.

For one function, in this order:

1. Find every call in it, and for each the service: the value last put
   in AX. The table below has the function for it.
2. For each call, read on from it to where each register is next
   written, and note every register that is read on the way, the
   function's own arguments to the next call included. That is what the
   call has to hand back, or what the next call has to be handed.
3. Write the calls, each with a variable for what it answers; `regs`
   goes when its last call has. Then read every line that still named
   `regs`.
4. Build, and run the tests: `portable_c-data` first, then
   `portable_c-scenarios`. No golden changes.

The calls of DOS, by the value in AH or AX. "Asked" is a call through
`Intr21` or the `CALL` that `copy.c` had, "bare" one through `Int21d` or
`Int21e`: the function has to be of the same kind, and where a service
is called both ways the bare function says so in its name.

| AH, AX | was | is |
|---|---|---|
| 06h, 09h | bare | `ConsoleChar(character)`, `ConsoleText(text)` |
| 0Eh | bare, `SelDrv` | `DriveSelect(drive)` |
| 13h | `Intr21f` | `FcbDelete(fcb, flagged)` |
| 19h | bare | `DriveCurrent()`, which is the low byte already |
| 1Ah | `SetDTA` | stays `SetDTA()`, which is `FindRecord(&md.DTA)` |
| 29h | bare | `FcbParse(name, fcb, how)` |
| 2Ch | `DosTime` | `TimeOfDay()` |
| 30h, 3306h | bare | `SystemVersion()`, `SystemTrueVersion()` |
| 3300h, 3301h | bare | `BreakCheck()`, `SetBreakCheck(on)` |
| 36h | `Intr21`, `Intr21f` | `DriveSpace(drive, flagged)` |
| 38h | bare | `CountryInfo(to)` |
| 39h, 3Ah, 3Bh | asked | `DirMake(name)`, `DirRemove(name)`, `DirChange(name)` |
| 3Ch | asked | `FileCreate(name, attributes)` |
| 3Dh | asked, bare | `FileOpen(name, mode)`, `FileOpenBare`; `OpenFil` stays what it is |
| 3Eh | asked, bare | `FileClose(handle)`, `FileCloseBare` |
| 3Fh | asked, bare | `FileRead(handle, to, count)`, `FileReadBare` |
| 40h | asked | `FileWrite(handle, from, count)` |
| 41h | asked | `FileDelete(name)` |
| 4200h to 4202h | asked | `FileSeek(handle, place)` for 4200h, and `FileStep(handle, from, step)`, with `from` 1 or 2, a step that may be back: two since the piece S, a place having no sign and a step one |
| 4300h, 4301h | asked | `FileAttributes(name)`, `FileSetAttributes(name, attributes)` |
| 4400h | asked | `FileDevice(handle)` |
| 4408h, 4409h | bare | `DriveFixed(drive)`, `DriveRemote(drive)` |
| 440Eh, 440Fh | bare | `DriveLetter(drive)`, `DriveSetLetter(drive)` |
| 47h | asked, bare | `DirCurrent(drive, to)`, `DirCurrentBare` |
| 48h, 49h, 4Ah | bare | `MemTake(paragraphs)`, `MemGive(block)`, `MemResize(block, paragraphs)` |
| 4Ah, of the program's own block | bare | `MemOwn()`: the block that DOS loaded the program into made as long as the program is. Which block that is and how long the program is, `dos.c` knew; in `sys_dos.c` there is nothing to make shorter, the program's memory being the extender's, and it answers that nothing failed |
| 5801h, 5803h | bare | `MemSetSearch(how)`, `MemSetUpper(on)` |
| 4B00h | asked, bare | `ProgramRun(name, tail)`, `ProgramRunBare`: the block that DOS takes with it is the system's file's to make, `Run` of `sys_dos.c` |
| INT 2Eh, COMMAND.COM's own | | Was `ShellRun(command)`, which came back, the stack kept and put back around the interrupt in `dos.c`: it went with Quick execute commands (`src/portable_c/DEVIATIONS.md`, 14) |
| 4Ch | bare | `ProgramEnd(code)` |
| INT 20h, entered at the start of the PSP | | `ProgramEndOld()`, for a DOS before 3 |
| The PSP at 0Ah, where DOS goes on when the program has ended; no call of DOS | | `ProgramEndedTake()` and `ProgramEndedGive()`: the PSP names a routine of the system's file between the two, `Exit01` of `dos.c` then and the routine of real mode `Ended_T` of `sys_dos.c` now, which has DOS told what the program has said beforehand, `ProgramEndedBreakCheck(on)`, when DOS has ended the program. It called a function of the program's, `told`, until the program of 32 bits had to do the same |
| 51h | bare | No function of `sys.h`, and no call any more: the entry of the 16-bit program, `Alloc0` of `dos.c`, asked where the PSP is, as the first thing that DOS was asked, by `ProgramPrefix`, static there; `Start` of `sys_dos.c` has it from the extender, by the selector that the program is entered with. What is read in the PSP is `ProgramTail()` and `ProgramEnvironment()`, which ask DOS nothing |
| 4Eh, 4Fh | asked | `FindFirst(pattern, attributes)`, `FindNext()` |
| 56h | asked | `FileRename(from, to)` |
| 5700h, 5701h | asked | `FileTime(handle)`, `FileSetTime(handle, time, date)` |
| 60h | asked | `FileTrueName(name, to)` |
| 25h, 35h | bare | The seven by which the program takes what DOS and the BIOS enter of it: `DeviceErrorNote()`, `DeviceErrorTake(ask)` and `DeviceErrorGive()` for INT 24h, `CtrlBreakNote()`, `CtrlBreakTake()` and `CtrlBreakGive()` for INT 1Bh, `CtrlCTake()` for INT 23h. `SetVector(number, to)` and `Vector(number)` were static in `dos.c` behind them; `PutVector(number, to)` beside them wrote the machine's table without DOS, for INT 21h, and went with that. In `sys_dos.c` the extender is asked, not DOS: `Note`, `Go` and `Give`, static there |

The BIOS for the screen, `Int10`, by AH: 0Fh is `ScreenState()`, 0
`ScreenSetMode(mode)`, 1 `CursorShape(shape)`, 2 `CursorTo(page,
place)`, 3 `CursorWhere(page)`, 6 and 7 `ScreenScroll(down, lines,
color, upper, lower)`, 8 `ScreenCell(page)`, 9 `ScreenFill(page,
character, color, count)`, 0Eh `ScreenType(character, page, color)`, 12h
with 10h in BL `ScreenEga()`, 1130h `ScreenFont()`; 12h with 20h in BL
was `ScreenPrintAll()` and 1112h `ScreenSmallFont()`, which went with
EGA lines (`src/portable_c/DEVIATIONS.md`, 12). For the keyboard, `Int16`: 0 is `KeyRead()`, 100h
`KeyWaiting()`, 200h `KeysHeld()`, which is the low byte already. The
mouse is done. `Int2F` with 150Bh is `DriveCd(drive)`, `Int28` is
`Idle()`. `Timer`, `Beep`, `SetDTA`, `OpenFil` and the others of
`system.c` and `window.c` are the program's routines as before, and
are called as before; `Int10m`, which scrolled a window by a line with
the mouse hidden, is `Scroll` there.

Nothing stays on the old calls. What the table lacked while the files
went over got its functions with the last of them: 51h and 58h, which
are of DOS alone, for the PSP that `Alloc0` asks for and for `SetStat`
of `resident.c`; and the BIOS's 12h with 20h in BL, by which EGA lines
has the BIOS print a screen of more lines than 25, which is of the PC
alone and goes with that command if the command goes. The map of the
memory and the watch on DOS, with 52h, 62h and 4452h, are gone.

What a call hands back, and where the traps are:

- AX is `value`, also where the call did not fail: `DirFunc` hands the
  high byte of it on with a file's attributes, `F38_40` reports it,
  `DelTr0` returns it. Whether DOS set the carry flag is `failed`. A
  line that read `regs.ax` behind `if (Intr21(...))` reads `.value` of
  the same answer.
- A register that a call answers in is a member with a name: the
  attributes, the time and the date, what a handle is, the clusters.
  If the call failed, the member is what `dos.c` put in the register,
  0. The original went on with what its caller had there, since DOS
  leaves a register alone that it has nothing to say in. Where a line
  reads such a register without having looked at the flag, write down
  what was in it: `M30_50` of `dirfunc.c` does, for the time and the
  date of a file that DOS would not tell.
- A register that a call does not answer in holds after it what it
  held before, also behind an error that `Intr21` asked DOS about. So
  what was read out of `regs` three calls later is a variable of the
  caller's now, and the call in between does not touch it: CX from
  4300h to 4301h is `attributes`, BX is the handle.
- A register that the service does not take was whatever the code
  before left there: AL of most calls, `0x3E00 | regs.ax & 0xFF`, the
  half of BX that a call does not read, CX and DX behind a call that
  set them, DS where no address goes. The functions hand over what is
  asked and nothing else, 0 in AL and the main data in DS. That is the
  program doing otherwise than the original, though no screen shows
  it, and it is declared: `src/portable_c/DEVIATIONS.md` has one entry
  for all of it, "DOS and the BIOS are handed nothing that they are not
  asked", with a table of the places. Every leftover that a file drops
  as it goes over gets a row there, in the same commit: the routine,
  the call, what the original has in the register and what the
  portable C has. Find them by step 2: a register that is written, or
  left as a call answered, and that the next call does not take.
- Before a leftover is dropped, be sure that the service does not read
  it, from what is written of DOS and of the BIOS, not from the bench
  passing. Four are known to be read by something, and the entry says
  what the functions do about each: AL of 4Eh, which DOS's APPEND goes
  by, a leftover at every search of the original; AL of the BIOS's
  call 1, where some BIOSes want the video mode, in `IniData` and in
  `Blank`; CX and DL of 1130h, which a BIOS that does not know the call
  leaves as they came, in `F69_00` and in `IniData`; and BL of 1112h in
  `F69_00`, which the call takes and the original does not set. One
  that you are not sure of is not dropped: hand the work back with it,
  as "Work by several agents" has it for a deviation, and the session
  asks.
- What the BIOS is asked in comes by name. The page that is shown goes
  from `ScreenState` to the calls that take one, where the assembly
  kept BH from the call before. `ScreenType` takes the page and the
  colour that the BIOS is handed, and a caller that handed it what a
  register happened to hold, as Enter does with `COMNINI_WAS`, hands
  it the two bytes of that.
- The carry flag as it was before the call: `FcbDelete` and
  `DriveSpace` take it as `flagged`, since DOS leaves it alone at
  those two and `Intr21` takes a set flag for an error. `F38_70` has it
  from comparing two characters, `C20_00` of `copy.c` from a
  subtraction.
- A call that took something at ES takes a pointer: the new name of
  `FileRename`, where `FileTrueName` writes, the block of the memory of
  `MemGive` and `MemResize`. A near pointer of the main data goes in as
  it is. The block of `ProgramRun` was one such, and is `dos.c`'s own
  now: see below, what the PSP and a program run met.
- `DirCurrent` takes where the path goes as one pointer, where the
  call had the segment with DX and the offset in SI: `regs.name = wcb +
  drive` with `regs.si` on `WinPath + 3` is `DirCurrent(drive,
  wcb->WinPath + 3)`.
- `FileSeek` hands back the place as one number of 32 bits, and with
  an error the two words that DOS left: a caller that held AX and DX
  against the place it asked for, without the flag, compares `place`.
- A loop that began with 4E00h in AX and went on with 4F00h has a
  variable for the first time round: `FncMs1` of `dirfunc.c`.
- A helper that took the caller's `regs` to go on with its registers,
  `SetDate` of `copy.c`, `GetVol0` of `read.c`, `CurNl` of `start.c`,
  takes what it reads of them, by name.
- `BlocEr` is the program's as before. `ErrDat` is cleared by every
  function that is asked with the program's errors, as `Intr21` cleared
  it, and by no bare one.
- The test of the data knows a constant that is handed to a function
  of `sys.h` which takes a name, and to `FileWrite`: `NAMED` of
  `harness/reads.py`. A function added to `sys.h` that reads a name
  goes there.
- A variable that is left without a use, and a static function, stops
  the build; a macro does not, `DX_OF`, `NUMBER`, `DOS_CHDIR`: take
  those out by hand.
- A structure of more than four bytes comes back through a pointer to
  room in the caller's frame. Where one member is wanted,
  `CursorWhere(0).shape` costs no variable.

Three callers, before and after, the casts of `&regs` left out.
`OpenFil` of `system.c`, where the second call took its mode from what
the first had left in BX:

```c
regs.ax = 0x3D00 | mode & 0xFF;
regs.bx = regs.ax;
regs.name = name;
failed = Intr21(&regs);
if (failed != 0 && regs.ax == 0x20) {
    regs.ax = regs.bx & 0xFF0F;
    failed = Intr21(&regs);
}
opened.value = regs.ax;
opened.failed = failed;
```

```c
opened = FileOpen(name, mode);
if (opened.failed != 0 && opened.value == ERR_SHARED)
    opened = FileOpen(name, mode & 0x0F);
```

`Cursor` of `window.c`, where the page went from one call of the BIOS
to the next in BH, and AL with it, which the second call does not
read:

```c
regs.ax = 0x0F00 | place & 0xFF;
regs.bx = 0;
regs.cx = 0;
regs.dx = place;
Int10(&regs);
regs.ax = 0x0200 | regs.ax & 0xFF;
Int10(&regs);
```

```c
CursorTo(ScreenState().page, place);
```

`M30_50` of `dirfunc.c`, where the original does not look whether DOS
told the time, and sets what CX and DX hold:

```c
regs.bx = opened.value;
regs.ax = 0x5700;
Intr21(&regs);
if (AttrDat != 0xFFFF)
    DX_OF(regs) = AttrDat;
if (AttrTim != 0xFFFF)
    regs.cx = AttrTim;
regs.ax = 0x5701;
Intr21(&regs);
```

```c
stamp = FileTime(opened.value);
if (stamp.failed) {
    /* The assembly goes on with CX and DX as they are, which DOS
       has not set then: the attributes that it has just set, and
       the address of the name. */
    stamp.time = old & 0xFFFE;
    stamp.date = (u16)md.TmpBuf;
}
if (AttrDat != 0xFFFF)
    stamp.date = AttrDat;
if (AttrTim != 0xFFFF)
    stamp.time = AttrTim;
FileSetTime(opened.value, stamp.time, stamp.date);
```

What making the first files so met:

- A function of `dos.c` that goes by something which another call has
  to have set fails where the other call is still the old one:
  `CaseMap` goes by what `CountryInfo` notes, and the program ended in
  "Invalid Opcode" at its first capital until `start.c` asked for the
  country through `CountryInfo`. Nothing else of `sys.h` is like that
  but `ScreenPut` and `ScreenGet`, which go by what `ScreenMemory` has
  noted: `ScrOffs` of `window.c` calls that, which `IniData` calls.
- `dos.c` grew by 2.6 KB for the functions, of which each is some 15 to
  70 bytes, and a caller is shorter for every call that it moves: the
  four files and the callers beside them gave back 0.8 KB for 76
  calls.
- A file that goes over makes fewer pointers of numbers, and the test
  `portable_c-host` fails for a count that has changed, fewer as well
  as more: write its record anew, `python3 harness/hostcheck.py
  --record`, in every commit that moves calls, and look at what the
  record's diff says. The README's table of figures has how many calls
  are left: `python3 tools/figures.py --write`, after the build, in the
  last commit of a piece of work.
- A removal that lands beside this work takes out what a function
  here had wrapped or moved, and the rebase then stops in the same
  lines: the removal stands, and the function takes its simpler form.
  `Window` and `MovScr` lost their waits for a CGA that way, and are a
  call of `ScreenPut` or `ScreenGet` each.
- The functions that nobody calls yet need nothing to keep them: a
  function that is not static is in the program whoever calls it.
- What a row of the deviation's table says of the original is read in
  the assembly, not in the structure of registers: that has what the C
  translation handed over, which is the original's only where it kept a
  register from one call to the next. Where it set one by itself, 0 in
  AL behind a `MOV AH`, zeros in SI and DI, the address of `TmpBuf` to
  have DS on the main data, the original has something else, and the
  row says both. `iconv -f cp866` of the assembly's file into `build/`
  and `grep -n 'Intr21\|INT'` there finds the calls. AL of 4Eh, which
  APPEND goes by, is such a register in `FindFil` and in `F34_11`: 0 in
  the C translation, a leftover in the original.
- A call that takes no address was still made with DS where
  `regs.name` had it: on the text of the viewer or the editor, on the
  list of the file search. The functions have DS on the main data
  there, and that is a row too.
- The scenarios take long with several agents at work. A commit's build
  can be copied and tested as the note on copies above has it, while
  the next file is moved: `harness/ported.py` with the copy's `VC.EXE`
  and `VC.MAP` and the original's listing.
- Where the C kept the registers of the BIOS and those of DOS in two
  structures, as `IniData` of `start.c` did, what it handed on in a
  register that a call does not take was not always the original's: BL
  of the carriage returns that `IniData` types was what the BIOS had
  answered about an EGA, and in the assembly it is what DOS answered
  about its version. The row of the deviation says what the original
  has there, read from the assembly, not from the C. And a leftover may
  be something that no function hands back, so that it cannot be handed
  on even where the function takes it, as `ScreenType` takes a colour:
  it is dropped like any other, with its row, if the service does not
  read it, and the BIOS reads no colour there on a screen of text.

What moving `blank.c`, `main.c`, `enter.c`, `commands.c`, `keys.c`,
`bars.c` and `menu.c` met:

- A file says at its top what it calls of the rest of the program,
  "Int10 calls the BIOS for the screen", "SelDrv is the call of DOS
  that makes a drive the current one", and has macros for the services,
  `DOS_CHDIR`. Both are stale when the calls go, and nothing stops the
  build for either: look for the old names in the file once more when
  it compiles.
- `SelDrv` and `DosTime` were `DriveSelect` and `TimeOfDay` under an
  older name already: their callers change the name and drop nothing.
- An argument may be there for a leftover alone. `F69_00` of
  `commands.c`, EGA lines, took what the main loop has in CX and handed
  the BIOS its high byte in a call that takes nothing there. When that
  went the argument was not read, which stops the build, and went too:
  from `vc.h`, from the call in `Command` of `main.c`, and from the two
  comments of `main.c` that said which registers the commands read.
- Many runs of the scenarios at once filled `/tmp`, which is in the
  memory and has a limit: with six checkouts testing, and one of them
  three copies at a time, the scenarios' tests of all of them failed in
  the same minute, the C translation's too, with "the machine stopped:
  it was reset, or QEMU ended" or "Disk quota exceeded". That was the
  machine and not the program. The harness has the disks of its machines
  in the checkout's `build/tmp` since, by itself, and nothing is to be
  set for it: `workdir` of `harness/dos.py`, which every tool of the
  harness comes to for such a directory. A run removes what it made
  there, and what a run that was killed has left, the next one removes.
  A `TMPDIR` that is set is used as it is.

What moving `dirfunc.c`, `read.c`, `usermenu.c`, `treebox.c`, `drive.c`
and `setup.c` met:

- What a DOS leaves in AX where a call has nothing to answer there
  may be what it was handed. The bench's FreeDOS does that when it has
  closed a file: a program that closes one with 3E53h in AX gets 3E53h
  back. So a caller that reads the low byte of AX behind such a call,
  where the call did not fail, reads the leftover that the original
  handed over, and the function of `sys.h` hands over 0: the leftover
  is behaviour there, and the call does not go over until that is
  decided. `ReDr00` of `read.c` is the one that was met, with the file
  that it closes after a read which failed, and the user decided it:
  the program says what the original does on such a DOS, in its own
  words, by the answer of the read, and entry 6 of
  `src/portable_c/DEVIATIONS.md` says what that is on a DOS that changes
  AX there. At every call, look for a read of `value` where `failed` is
  0. One that takes the high byte alone, as `DirFunc` and `GetVol` do,
  is not of this kind: the function hands the service over there as the
  original does.
- The directory of the machines' disks, of the note above, wants a
  short path: QEMU's socket is in it, and a path of more than 107
  characters is refused, "AF_UNIX path too long". `build/tmp` of a
  checkout does; where it would not, for a checkout that lies deep, the
  harness says so and uses the system's directory. A `TMPDIR` set by
  hand to a directory two levels further down did not: the harness
  names the path that is too long then, and starts no machine.

What `copy.c` added to that, which went over a family of calls at a
time, a commit each:

- What a register held that a line reads without having looked at the
  flag is read from the assembly, not from the structure of registers
  that the C had. The structure may never have been given that member,
  and then the C went on with what its stack held: CX where `C20_00`
  asks for the time of a source that is a device. Or the C worked a
  value out beside the structure where the assembly changes the
  register: the attributes of a target that is appended to, `AND` and
  `OR` on CX there. The place then says what the assembly has, and the
  commit says that the portable C had something else.
- The other answers are like the time and the date in that: `kind` of
  `FileDevice` is 0 if DOS would not say, where the original reads DX
  as it was, the address of the name; `free` and `sector` of
  `DriveSpace` are 0 where it reads BX and CX as they were, a handle
  and a count. `place` of `FileSeek` needs nothing, being the two words
  that DOS left.
- While some calls of a function still have the structure, a call that
  has gone no longer sets it for them. What a call that stays takes,
  the name and the handle, is set in front of it, where it was set
  once for a row of calls; and what a line behind it reads of a
  register that the call does not answer in, too. The structure goes
  with the last of them, and the build stops for it if it is left.
- A register that went through a call which does not take it to one
  that does, the attributes of a source directory from 4300h through
  39h to 4301h, is the answer of the first call, read at the third.
- A file gives back fewer bytes for a call that answers with more than
  four bytes, which come back through a pointer to room in the
  caller's frame: `FileAttributes`, `FileDevice`, `FileSeek`,
  `FileTime`, `DriveSpace`.
- Each family was built, held to the data and to the host check, and
  copied as a build, by the note on testing a build that is a copy;
  the scenarios then ran on the copies side by side while the next
  family was made.

What the last of it met, `resident.c`, the drive B:, EGA lines, the
extension file of a directory and the old way taken out, and how a
function is added:

- A function that the interface lacks is declared in `sys.h`, in the
  group of its kind, with a comment that says what is asked, what it
  answers and what it answers when it fails, in DOS's values and with
  no register in it; and written in `sys_dos.c`, as it was in `dos.c`,
  where the comment at the head of its group says the service. If it
  takes a name, it goes into `NAMED` of `harness/reads.py`. The README
  counts the functions by group, under "The system in one place", and
  the table above gets its row. What is of DOS alone and no part of
  the interface, where an interrupt goes, was in the last part of
  `dos.c` and declared in `vc.h`; of `sys_dos.c` nothing is declared
  there, and `vc.h` declares for it what it calls, `Main`. A function
  of `sys.h` is in more places besides, which the note on one that
  goes has, in "Notes on the portable C".
- A register that a service does take may be set by nothing in the
  original: BL of the BIOS's 1112h in `F69_00` is 0 from a line of the
  screen buffer cleared at the start of the command. Step 2 finds the
  registers that a call does not take; read also, for each that it
  takes, where the assembly sets it. A function sets what it is asked,
  and such a place is declared among what is known to be read.
- What a file handed over may be neither the original's nor nothing,
  but the portable C's own: `regs.name = 0`, to have a number in DX,
  put DS on 0 at calls that take no address, in `SetBrk` and `Exit02`;
  and a `name` that was never set handed DOS DS and DX as the stack had
  them, in `SetStat` and at the entry. The row says what the original
  has, and the commit what the portable C had.
- A function of `dos.c` may be entered on a stack that is not the
  program's: `SetBreakCheck` is, from `Exit05`, when DOS has ended the
  program, as `Exit02` of `resident.c` was while `Exit05` called that.
  All of `dos.c` reaches its
  registers through `STACK` pointers, so that works; it takes a few
  words more of the other program's stack than a structure in the
  caller did.
- The table of entry 6 of `DEVIATIONS.md` is in the order of the files,
  and in a file in the order of its routines: a row is put where it
  belongs, not at the end. What the C translation handed over has a
  column of its own, filled where it was something else than the
  original's.
- No file declares the names of the old way, so a call of one does not
  compile. What `tools/calls.py` still watches in the portable C is its
  last kind, a fixed address of the machine written outside `sys_dos.c`,
  and not where the segment comes from a variable, as it has to for
  the table of interrupts: `PutVector` of `dos.c` was where that was,
  and nothing writes that table now.
- The whole suite takes four to eight minutes with `ctest --test-dir
  build -j8`, where it took three while the portable C was a program of
  16 bits: the tests that read the source are done within the first ten
  seconds, and from then on the source may be edited while the
  scenarios of both C trees run. The build may not change until they
  are done.

What the PSP and a program run met, when what was 16 bits in kind left
the header (`TODO.md`, the piece S1 of the plan for the 32 bits):

- A shape of the header that only DOS in 16 bits has, a block of far
  addresses, a segment to read at, becomes what is asked: the name and
  the tail for a program run, the tail and the environment that this
  one was started with, where the memory ends. What DOS takes besides
  is made in `dos.c`, where the original has it and as it has it: the
  block is `EPB` of the main data, which keeps its 14 bytes there since
  the layout of the main data is held, filled in at every run, where
  the original fills in all but the tail once, when it starts. DOS is
  handed the registers and the bytes that it was handed.
- Before a field of the main data is given to `dos.c` like that, look
  for every line that names it: nothing else reads `EPB`. `EPB_WAS` of
  `resident.c` is its address in the original, for a register that the
  BIOS is handed, and reads no field.
- A function of the header that answers with a pointer answers with a
  `FAR` one, and what is read behind what it points at is said at its
  declaration: the environment is read on behind its end, where DOS
  has the path of the program's own file, as 68 bytes whatever the
  path, by `MnPth0` of `start.c`. Another system has to have that many
  bytes there.
- A place of DOS's own that is kept as a number stays one: `ComSpec` of
  the main data is where the name of the shell is in the environment,
  and is added to what `ProgramEnvironment` answers.
- What `dos.c` goes by that something else sets was said in `vc.h`,
  with what is of DOS alone: `PspSeg`, which the entry of the program
  set by `ProgramPrefix`, and the toy of `harness/trial32/sys.c` in an
  entry of its own. The entry is `dos.c`'s since the piece S3, and the
  toy is entered there as the program is. A function of `sys.h` is also a function
  of `dos32.c`, made or not, which the build compiles for 32 bits with
  the rest; and what is of DOS alone and outside `sys.h` is in no
  `dos32.c`, so the build, which links the tree for 32 bits too,
  stops at each such name as one that the linker does not find.
  `python3 harness/trial32/both.py` took seconds and said whether the
  two files still answered alike; since `dos.c` went,
  `harness/trial32/asked.py` runs the toy with `dos32.c`, and nothing
  judges what it writes.

What the critical error, Ctrl-Break and Ctrl-C met, when where they
arrive went into `dos.c` (`TODO.md`, the piece S2 of the plan for the
32 bits):

- What a system enters of the program is two things, and they are
  kept apart. Where it arrives, with DOS's registers, a segment
  register to load and an `IRET`, is the system's: an interrupt
  function of `dos.c`, `FatErr`. What the program does about it is a
  function of the program's, of a type that `sys.h` names,
  `DeviceErrorFn`, which the program hands over where it takes the
  thing, `DeviceErrorTake(FtErr1)`, and which is called through a
  pointer as any function is. It takes what it needs by what that is,
  in DOS's shapes as everything of `sys.h` does, and no address of
  real mode: of the header of the device that DOS names, whether it is
  a drive, and a pointer to the name if not.
- Each of the seven functions is one call of DOS, and the row is the
  program's: `HookVct` notes who has the critical error and who has
  Ctrl-Break and then takes both, as the original asks, and a function
  that noted and took one of them at once would have changed the
  order.
- What `dos.c` keeps for the program's sake stays where the original
  has it while the layout of the main data is held: `ErrVct` and
  `BrkVct`, as `EPB` before them. Look for every line that names such a
  field first; nothing else read these.
- The scenarios' test holds a function of a batch to have run by its
  name in the linker's map, among the symbols that are not static:
  `FatErr` is not static in `dos.c` for that, and keeps the name of the
  routine. A function that moves and becomes static fails the test
  with "the link map has no function", behind every scenario passed.
- The call through the pointer has its arguments in registers and
  costs the stack what the call of a static function cost: every
  scenario left to the byte what it had left.
- DOS enters the critical error with the machine kept from
  interrupting, and the compiler's interrupt function leaves it so:
  the box gets its keys because `Window`, `MovScr` and `MinBx3` call
  `IntOn`, as the original has `STI` there. Three of its four callers
  being of the common code, `IntOn` is a function of `sys.h`, beside
  the critical error, and of `dos32.c`. Having the entry let the
  machine interrupt before it asks the program would take those calls
  out of the common code, and would be the program doing otherwise
  than the original between the entry and its first `Window`: that is
  asked first.
- A decision of the program's that is still in `dos.c`: `AskEs` clears
  `md.ErrDat` in front of a call and makes the error 53h of a call
  behind which it is set. What the function answered does not say
  that: it answers `DEVICE_FAIL` with `BlocEr` set and no box shown,
  too, and such a call is asked about like any that failed. `TODO.md`
  says what moving it would take.
- A macro of `dos.c` may not have a name that `vc.h` has, which the
  file includes: W140 stops the build, as it did for `ENTRY`.

What the entry, the ends and Quick execute met, when the last of DOS
outside `dos.c` went into it (`TODO.md`, the piece S3 of the plan for
the 32 bits):

- What DOS enters is `dos.c`'s, and what the program does there is the
  program's, and the cut between them is where the order of DOS's calls
  is: the entry asks DOS one thing, where the PSP is, and calls `Main`;
  the version, the block made shorter and how DOS looks for memory stay
  a row of the program, `Alloc3` of `resident.c`, in the original's
  order, and each thing in that row that only `dos.c` can do is a
  function of `sys.h`. Moving a row into `dos.c` whole would have moved
  a call in front of a message of the program's.
- A register that the program that started this one is left with when
  DOS has ended the program is `dos.c`'s to leave, and it may be what a
  call of the program's left there: DL is what DOS was last told of
  Ctrl-C. `dos.c` has that from the program, which says it beforehand,
  `ProgramEndedBreakCheck`, and reads nothing of the program's for it.
- A function that is handed over and is static in its file is compiled
  far and called through the pointer far, as `FtErr1` is.
- What the system does when it has ended the program cannot be a
  function of the program's that is called then: under an extender the
  program is gone before DOS goes on, and nothing of it can run. So the
  program says beforehand what is to be put back, a value, each time
  that it has noted it anew (`SetBrk`), and each file of the system's
  functions puts it back in its own way: `Exit05` of `dos.c` by the
  function that tells DOS, `dos32.c` by eight instructions of real mode
  in its block below the megabyte, `Ended_T`. A thing that a system may
  have to do with the program gone is asked of the program as a value,
  not as a function.
- Instructions of real mode in `dos32.c` are bytes in a table, with the
  instruction beside each as a comment, and the places that they read
  behind them in the block, read through CS: the compiler writes for a
  segment of 32 bits. `ndisasm -b16` reads the bytes back; what holds
  them is the bench: `harness/trial32/ended.asm` run against the
  program, which prints the registers that it gets back and whether
  DOS looks for Ctrl-C again, the scenario `ended`, and the toy of
  `both.py`, `asked.py` since, which ends with its end left taken so
  that DOS goes on in the routine.
- `_asm` in a function that is static and naked: the function is near,
  `ret` is a near return, and the call of it is near. `seg` of a
  variable of the program's data is the segment of its group, `DGROUP`,
  the one that DS is on: `mov ax,seg ShellSp` in `Int2E`. A far pointer
  in `parm [si dx]` has its offset in SI and its segment in DX. BP in a
  `modify` list is not saved, so the instructions push and pop it; ES
  may be listed there, DS may not.
- What does not return cannot be a function of `sys.h` that another
  system makes by a call which does. Before taking such a thing as it
  is, read what the original does behind it: it set its stack anew
  behind INT 2Eh because it never returns from running commands at
  all, and the C had a second way into its loop for that alone,
  `Again`. With the stack kept and put back, the loop of the commands
  goes on behind the call, and what DOS is asked is the same row.
- The compiler for 32 bits takes `_asm` with registers of 16 bits,
  `seg` and `offset` of a segment of 16 bits, and `__loadds`, with no
  word said. So that a file compiles for 32 bits does not say that it
  has none: `resident.c` compiled the moment its interrupt function had
  left it, with `Int2E` still in it. The host check's record is what
  says that a file has none.
- `dos.c` had a `FLAG_ZERO` of its own, for the keyboard, when the
  flags of `resident.c` came over under the same names: look for a
  name in the file before bringing a block of definitions into it.
- A function is in more places than the six that the piece S1 has: a
  row of entry 6 of `DEVIATIONS.md`, if a register that the call does
  not take goes with it; and the README's paragraphs under "The system
  in one place", "What rests on Open Watcom" and "Inline assembly",
  which say where each thing of the compiler's is, by file.
- The toy of `harness/trial32/sys.c` is entered where the program is,
  at `Start_` of `dos32.c`, as it was at `Alloc0_` of `dos.obj` in 16
  bits, and defines `Main`, the one name of the program that the file
  of the system's functions calls. The build links the objects for 32
  bits and nothing else, `build/portable_c/VC.EXE`: a name of DOS that
  the tree asks for outside `sys.h` shows there as one that the linker
  does not find.
- The stack figures of the scenarios follow the layout of the file in
  two or three scenarios, `command`, `mouse-select` and `mouse-find`,
  by 2 to 16 bytes, at every change of the code: hold a change to the
  figures of the others.

What the rest of `dos32.c` met, the functions that the extender does
not carry through as they are (`TODO.md`, the piece D2 of the plan for
the 32 bits):

- The extender puts a selector into the PSP at 2Ch, where DOS has the
  segment of the environment, and DOS reads that word itself: a program
  run by 4Bh with no environment named gets a copy of whatever lies at
  that selector's number taken for a segment. `Run` of `dos32.c` names
  the environment's segment. Run through the shell, a program has an
  environment all the same, FreeCOM's: try such a thing with a program
  that is run by itself.
- What a toy of the trial did once and came back from says little: the
  trial's command ran with no environment named, through the shell.
- Where DOS writes into a buffer of the program's and the call is made
  in real mode, the program's bytes go down to the block first and
  come up afterwards, as many as DOS may write: then the program has
  what DOS wrote and, of the rest, what it had, as when DOS writes
  there itself. FreeDOS writes 24 of the country's 34 bytes and leaves
  a path alone where 60h fails, and the toy of both builds, which
  fills each buffer first, shows either.
- The extender is entered on the program's stack at every interrupt
  that the program makes, and keeps there what it needs of the call
  until that comes back: 190 bytes below the stack pointer at a call
  of DOS that it translates, 130 at one made by DPMI 0300h, 74 at any
  other. Whatever goes on on the program's stack while such a call is
  under way, as the routine of the critical error does, starts below
  that.
- A routine that the extender enters from real mode, for an interrupt
  taken by DPMI 0205h, has the registers of real mode in their low
  words, DS and ES anywhere, and a stack of the extender's. `wcc386`
  takes it as a `__declspec(naked)` function with `_asm`: `mov
  bx,word ptr cs:Variable` reads a variable through CS, `mov
  esp,Variable` and `call Function` are taken as written, and a static
  function that only `_asm` calls is kept. Give the function that it
  calls a `#pragma aux` with its registers, so that the call does not
  rest on the compiler's options.
- Nothing promises that what has no value is zeros when a program of
  32 bits is entered. A variable of `dos32.c` has a value, `= 0`, but
  for the 640 KB.
- DOS on the bench has free blocks besides its largest, and how many
  and how long differs between the two countries: a toy that is to be
  given the same blocks in both builds takes all of them first.
- Asked for the capital of a letter before the country has been asked
  for, the toy of 16 bits ends in "Invalid Opcode": `CaseMap` of
  `dos.c` calls what its four bytes hold. A toy of both builds asks
  only what the program may ask.
- `BREAK` off and Ctrl-Break at DOS's prompt end no step of a script,
  whatever ran before: it looks like a machine that has stopped and
  is none.
- When DOS ends a program of 32 bits by another's hand, the extender
  sees the call, since it has INT 21h of real mode while the program
  runs, and takes itself out, with the interrupts that the program
  took through it. `harness/trial32/ended.asm` shows it.

What a program that a command has left in the memory met, when the
program of 32 bits is left: a mend that was made for it and dropped on
the user's word of 13 October 2026, and what stays of it (`TODO.md`,
the piece L of the plan for the 32 bits, which has the mend in words):

- A setting of the extender that was "seen" to mend something on a
  copy of the file is looked at on every screen of the run, the first
  above all. A program that the extender does not load leaves DOS's
  prompt, which takes the steps of a script as the program would: one
  bit of the settings cleared was taken for a mend so, the user was
  told that the mend was one bit, and what the steps had shown was
  DOS. What it took then was two other bits, more of `dos32.c` and a
  routine of real mode, and the user, told that, had it dropped.
- What the extender does is read in its file where nothing describes
  it. `binw/dos32a.exe` is not packed: `ndisasm -b16` of it, from where
  its header has its two segments of code, kept in `build/` and not
  committed, and nothing of the listing written down but what the
  extender does, in words. The names of the options of its environment
  variable, `DOS32A`, are in the file, and the code that takes each
  sets a bit of the settings: that names the bits. The piece L has
  what was read.
- As it leaves, the extender puts all 256 interrupts of real mode back
  as they were when the program started, with a line of warning for
  each that was otherwise, by two bits of its settings; and whatever
  its settings say, it has INT 1Bh, 1Ch, 21h, 23h, 24h and 2Fh go where
  they went when it started, and INT 15h where it has taken that,
  whoever has taken them since. And told to give INT 1Bh, 1Ch, 23h or
  24h back in protected mode, it has the interrupt of real mode go
  there too.
- INT 21h of real mode is the extender's while the program runs. A
  program that takes it meanwhile hands on to the extender, so nothing
  in the program can leave it that interrupt when the extender has
  gone.
- The scenario `stay-left` shows all of it, and the program of 32 bits
  is held to what it shows there by a patch of its own, the difference
  being declared as it is on the user's word (entry 7 of
  `DEVIATIONS.md`; `TODO.md`, the plan, 10 of what was open). Its
  second program finds the first by INT 2Fh, so where that one is lost
  it prints dashes for all five, whatever else still goes to the
  first: such a row says less than it seems to, and the entry says
  what is known by something else.
- A patch that a build has of its own may lack a row that the tree's
  patch has, where the build shows the original's row there:
  `stay-left` at DOS's prompt. `goldens.py patch` says "gone" for it,
  and the patch's reason says why.
- A scenario that goes on at DOS's prompt when the program has been
  left: with `"cls" Enter` as its last command, or the prompt that the
  program leaves is written into a row of what the command before
  printed.

What a trial of the extender's memory met, DPMI 0500h to 0503h under
DOS/32A, before the memory of the panels and the texts was taken from
it (`TODO.md`, "X, the extender's memory on the bench", which has the
facts row for row and what the piece F9 goes by):

- The extender keeps 16 bytes of its own in front of every block, in
  the memory itself, and looks at them all at every call for memory,
  0500h too. A write behind the end of a block ends the program at the
  next such call, "DOS/32A run-time (6004): extended memory blocks
  have been corrupted", with the name of the block, which is its
  address less 16. A scenario that ends so has written behind a block.
- A block is given with what the memory held, and a block made shorter
  has the extender's 16 bytes written behind its new end.
- A toy that asks what is in memory must not have taken and given back
  blocks there first: the search by halving of `blocks.c` left the
  extender's 16 bytes at two dozen places, and the first reading of
  "what a machine that has just started has in its memory" was of
  those.
- The memory is the extender's since the piece F9 (`TODO.md`, "F9, the
  memory from the extender"). What that met: a bench's state is loaded
  only into a machine of the memory that it was saved of, so a scenario
  that asks for less memory, `memory` of `harness/scenarios.py`, gets a
  bench of its own, booted for it; the room of all that there is holds
  what a second copy, or the program before, or the machine's start,
  left there, and a routine that copies from behind the end of a text,
  the editor's line, shows that in the main data; and a tool that is
  run while the build changes reads the linker's map again at every
  scenario, so "before" is saved from a copy of the build in a
  directory of its own, `build/NAME/portable_c`.
- `harness/trial32/run.py` starts a machine for every file that it is
  given. Two programs on one machine are one file and steps that type
  the second at DOS's prompt, `q '"d:\vc\vc" Enter'`, with `--extra`
  for another file. A capital letter is `Shift+s` in a step.
- A program that is to be had with little memory is run from
  `harness/trial32/second.c`, which keeps all of the extender's memory
  but as much as its keys say.

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
