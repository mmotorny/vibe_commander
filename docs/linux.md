# The program for Linux

The same source is also a program for Linux of 64 bits, `vc`, built
with clang. `CLAUDE.md`, "The portable C", has the decisions that it
rests on, and `TODO.md` what is left of it. Nothing of the common code knows
that it is on Linux. It asks the system what it asked DOS and the BIOS,
through `src/portable_c/sys.h`, and for Linux the files
`src/portable_c/*_posix.c` answer as DOS would, at the level of DOS's
services, where DOSBox emulates the processor:

- Two drives, R:, the root of the file system, labelled ROOT, and H:,
  the home directory, labelled HOME. The program starts on H: where it
  is started under the home directory, on R: otherwise. Paths keep
  DOS's 67 characters; a name of the host that is not of DOS's form, 8.3
  in capitals, is shown under an alias of DOS's form, `LONGNA~1`, which
  leads back to it for the run.
- The code page 437, always, and the country with it: names cross
  between it and UTF-8, and a character that the code page lacks shows
  as `_`.
- The screen, 80 columns by 25 rows whatever the terminal's size, drawn
  in the terminal by the program's own escape sequences and termios,
  with no library: the VGA's sixteen colours exactly where `COLORTERM`
  says that the terminal has 24 bits, the theme's sixteen elsewhere.
- The keys, made the words that the BIOS of the machine of the tests
  gives, with the kitty protocol of the keyboard where the terminal has
  it, so that Alt by itself and the key bar's Shift, Ctrl and Alt work
  as on DOS; and the mouse from the terminal's reports, with its
  pointer drawn as DOS's mouse driver draws it.
- A command, run by `$SHELL -c` (`/bin/sh` where that is not set) on
  the terminal's own screen, with the paths of DOS's in it, and the
  names of DOS's of the current directory, made the host's; Ctrl-O shows
  that screen until a key. Enter on a file that the user may execute
  runs it, whatever its name, and on any other file that the extension
  file has no command for hands it to `xdg-open`.

What it does otherwise than the original for all that is declared in
`src/portable_c/DEVIATIONS.md`, under "The program for Linux".

## Building it

The one build, `cmake -B build` and `cmake --build
build`, makes it where it finds clang, beside the program for DOS, with
a list of clang's warnings written out, each an error:

| Program | What it is |
|---|---|
| `build/portable_c/posix/vc` | The program, with the usual hardening of a program for Linux |
| `build/portable_c/posix-asan/vc` | The same with the sanitizers of addresses, of undefined behaviour, of integers and of implicit conversions, each fatal: the one that the tests run |
| `build/portable_c/posix-msan/vc` | The same with the sanitizer of memory, which goes with no other: built only by its name, `cmake --build build --target portable_c-posix-msan`, run by no test, and kept for audits (`DIARY.md`, "Take the program for Linux through the scenarios, with its clock, masks and not-run list", says how) |

## Running it

In a terminal of 80 columns by 25 rows or more:

```sh
cd ~ && /path/to/the/checkout/build/portable_c/posix/vc
```

It takes the terminal's alternate screen and shows its banner, "Vibe
Commander 0.1, based on Volkov Commander 4.05", in the first two rows,
its panels on H:\, the prompt and the key bar, at the top left of a
larger terminal; a smaller one shows only the lines that ask for 80 by
25, until it is made larger. F10 leaves it, and gives the terminal back
as it was. Its own directory, where the original has the files that it
reads beside its own, is `~/.config/vc` (`$XDG_CONFIG_HOME/vc` where
that is set), made when it is not there: the setup is saved there as
`vc.ini`, and the help file and the files of the menus and the
extensions are read there. The program does not bring them, being the
original's: F1 shows no help and F2 no user menu until they are copied
there from the shareware distribution, which the harness keeps in
`downloads/vc405sw.zip` once it has fetched it (`python3
harness/deps.py` does). Of its files the program reads `VC.HLP`,
`VC.MNU`, `VC.EXT`, `VCVIEW.EXT`, `VCEDIT.EXT`, `ARCHIVES.MNU` and
`FORMAT.MNU`, their names in capitals or not. Its standard input and
output have to be a terminal. Ctrl-Z is the program's key for the tree
and does not stop it; `kill -TSTP` does, and `fg` draws it again. The
sanitized build writes a report on the alternate screen, which goes
with it: run that one with `2>file`.

## How it is judged

By the same scenarios and goldens as the program
for DOS, and by tests of its own, each of which is skipped where there
is no clang:

| Test | What it checks |
|---|---|
| `portable_c-posix-unit` | What the program emulates of DOS without input or output of its own, the files `*_posix.c` but `sys_posix.c` and `machine_posix.c`: names and aliases, the code page and UTF-8, drives and paths, dates, DOS's errors, and the services on files and directories of the host's, by `harness/posixcheck.c`, built with the sanitizers |
| `portable_c-terminal` | What `posix-asan/vc` draws in a pseudo-terminal, replayed through pyte, a terminal emulator in Python that the harness fetches pinned, held cell by cell to the grid that the program writes for the same keys through its test channel, in 24 bits and in 16 colours; a terminal too small, SIGTSTP and SIGCONT, the kitty protocol and the keys that the program reads held to the BIOS's words, the mouse, and commands with Ctrl-O. `harness/terminal.py` |
| `portable_c-posix-scenarios` | `posix-asan/vc` through the scenarios, by `harness/posixrun.py`: on a bench of the host's in `/tmp`, its home directory laid out as the machine's C:, driven through the test channel, `VC_TEST`, which takes the scenarios' keys as the BIOS's words and writes the grid whenever the program waits for a key, on the clock of the machine of the tests, `VC_TEST_CLOCK`. Its screens and its files are held to the original's goldens with what is declared for the portable C and, on top of that, for Linux, each scenario that is not run on Linux or is cut there named by an entry of Linux of `DEVIATIONS.md`. It starts a machine of DOS once, for the bench's setup file, and keeps that in `build/posix-bench` |
| `portable_c-cap` | The cap of the memory that the program's blocks hold together, which the machine of `*_posix.c` takes from the implementation, none on Linux, 256 MB on the web page and at most that on iOS, and from `VC_MEMORY` in its place for a test: `posix-asan/vc`, the app for iOS on a host, `ios-host/vc`, and with `-DVC_WEB=ON` `web-node/vc.js`, on the bench of `harness/posixrun.py`, the editor asked for a file of 1 MB. With `VC_MEMORY` at 512K the box that refuses it is the original's, cell for cell, as the golden of `editor-big` has it where the original refuses the same file; without it the editor opens the file; and with a `VC_MEMORY` that is no number of bytes the program does not start. `harness/capcheck.py` |
| `portable_c-tidy` | The common code, the files of Linux and their test held to clang's warnings, with the commands that the build compiles them with, and to clang-tidy by `.clang-tidy`. `harness/tidycheck.py` |

What is declared for Linux is `ON_LINUX` of `harness/deviations.py`,
each part with its reason there. Masks: what the comparison leaves out
where the golden has the original's, because it is the host's and no
golden can expect it, found on the screen by a rule and not written as
cells: the drive's letter and its label, the drive menu's letters, the
room of the drive in the Info panel, the time of a directory that the
run wrote into, the place of the program's own files, and what DOS
showed behind the panels after a command. Every other cell is
compared. The scenarios that rest on what only DOS has, Russia, the
diskette, a program or a command of DOS's, are not run, each with its
reason; others are cut, taken only as far as a step, where what follows
is DOS's alone, a diskette or the original's directory, or is otherwise
on Linux for a reason that is written there. Two things keep most
scenarios whole. The bench makes
its directories and its files in the order of their names, so that the
tree and the file search, which list a directory as it stands on the
disk, list it alphabetically on DOS as on Linux. And DOS's names of
devices, NUL, CON and the like, are ordinary names of files on Linux:
the steps of a scenario that use them as devices are left out there,
or the scenario is cut where the original beeps for one. Where a mask cannot do, a patch
of its own, in `harness/goldens/posix/`, says what the program for
Linux is expected to show. `LINUX_NOT_RUN`,
`LINUX_CUT` and `LINUX_LEFT_OUT_OF` of `harness/deviations.py` have
every cut with its step, and each is named by an entry of "The program
for Linux" in `src/portable_c/DEVIATIONS.md`.
