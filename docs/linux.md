# The program for Linux

The portable C is also a 64-bit program for Linux, `vc`, built with
clang, that runs in a terminal. None of the common code knows that it is
on Linux. It still asks the system what it asked DOS and the BIOS,
through `src/portable_c/sys.h`, and for Linux the files
`src/portable_c/*_posix.c` answer as DOS would. Where DOSBox emulates
the processor, this emulates DOS at the level of its services.

`CLAUDE.md`, "The program for Linux", has the decisions it rests on, and
`TODO.md` what is left to do. What it does otherwise than the original
is declared in `src/portable_c/DEVIATIONS.md`, under "The program for
Linux".

## What it emulates

| Area | How |
|---|---|
| Drives | Two: R:, the root of the file system, labelled ROOT, and H:, the home directory, labelled HOME. The program starts on H: when started under the home directory, and on R: otherwise |
| Names and paths | Paths keep DOS's limit of 67 characters. A host name that is not of DOS's form, 8.3 in capitals, is shown under an alias of DOS's form, `LONGNA~1`, which leads back to it for the run |
| Code page | Always 437, with its country. Names are converted between it and UTF-8; a character the code page lacks shows as `_` |
| Screen | 80 columns by 25 rows whatever the terminal's size, drawn with the program's own escape sequences and termios, with no library. The VGA's sixteen colours exactly where `COLORTERM` says that the terminal has 24 bits, the terminal theme's sixteen elsewhere |
| Keys | Made into the words that the BIOS of the test bench gives. The kitty keyboard protocol is used where the terminal has it, so that Alt by itself, and Shift, Ctrl and Alt for the key bar, work as on DOS |
| Mouse | From the terminal's reports, with the pointer drawn as DOS's mouse driver draws it. The wheel does nothing |
| Commands | Run by `$SHELL -c` (`/bin/sh` where `SHELL` is not set) on the terminal's own screen, which Ctrl-O shows until a key is pressed. DOS paths in a command, and DOS names of files in the current directory, are turned into the host's |
| Enter on a file | A file that the user may execute runs, whatever its name. Any other file for which the extension file has no command is handed to `xdg-open` |

## Building it

The normal build, `cmake -B build` and `cmake --build build`, makes the
program wherever it finds clang, beside the program for DOS. Every
warning of a written-out list of clang's warnings is an error.

| Program | What it is |
|---|---|
| `build/portable_c/posix/vc` | The program, with the usual hardening |
| `build/portable_c/posix-asan/vc` | The same with the address, undefined-behaviour, integer and implicit-conversion sanitizers, each fatal. The tests run this one |
| `build/portable_c/posix-msan/vc` | The same with the memory sanitizer, which cannot be combined with the others. It is built only on request, `cmake --build build --target portable_c-posix-msan`, run by no test, and kept for audits. `DIARY.md`, "Take the program for Linux through the scenarios, with its clock, masks and not-run list", says how to run one |

## Running it

In a terminal of at least 80 columns by 25 rows:

```sh
cd ~ && /path/to/the/checkout/build/portable_c/posix/vc
```

- It takes the terminal's alternate screen and shows its banner, "Vibe
  Commander 0.1, based on Volkov Commander 4.05", in the first two rows,
  with its panels on H:\, the prompt and the key bar. In a larger
  terminal it sits at the top left. A smaller one shows only a message
  asking for 80 by 25, until it is made larger.
- F10 leaves, and gives the terminal back as it was.
- Its standard input and output must be a terminal.
- Ctrl-Z is the program's key for the directory tree and does not stop
  it. `kill -TSTP` does, and `fg` redraws it.
- The sanitized build writes its reports to the alternate screen, which
  disappears with it: run that build with `2>file`.

### Its directory

Where the original keeps its files beside its own program file, this
program uses `~/.config/vc` (`$XDG_CONFIG_HOME/vc` where that is set),
and creates it if needed. The setup is saved there as `vc.ini`.

The help file and the files of the menus and extensions are read from
there too, with names in capitals or not: `VC.HLP`, `VC.MNU`, `VC.EXT`,
`VCVIEW.EXT`, `VCEDIT.EXT`, `ARCHIVES.MNU` and `FORMAT.MNU`. They are the
original's and are not shipped, so F1 shows no help and F2 no user menu
until they are copied there from the shareware distribution.
`python3 harness/deps.py` downloads it to `downloads/vc405sw.zip`.

## How it is judged

By the same scenarios and goldens as the program for DOS, and by tests
of its own. Each is skipped where there is no clang.

| Test | What it checks |
|---|---|
| `portable_c-posix-unit` | What the program emulates of DOS without input or output of its own, the files `*_posix.c` but `sys_posix.c` and `machine_posix.c`: names and aliases, the code page and UTF-8, drives and paths, dates, DOS's error numbers, and the services on the host's files and directories. `harness/posixcheck.c`, built with the sanitizers |
| `portable_c-terminal` | What `posix-asan/vc` draws in a pseudo-terminal, replayed through pyte, a terminal emulator in Python that the harness fetches pinned, and compared cell by cell with the grid that the program writes through its test channel for the same keys, in 24 bits and in 16 colours. Also a terminal that is too small, SIGTSTP and SIGCONT, the kitty protocol, the keys held to the BIOS's words, the mouse, and commands with Ctrl-O. `harness/terminal.py` |
| `portable_c-posix-scenarios` | `posix-asan/vc` through the scenarios. See below. `harness/posixrun.py` |
| `portable_c-cap` | The cap on the memory that the program's blocks hold together, which the machine of `*_posix.c` takes from the implementation (none on Linux, 256 MB on the web page) or from `VC_MEMORY` in a test. It runs `posix-asan/vc`, and with `-DVC_WEB=ON` `web-node/vc.js`, and asks the editor for a file of 1 MB. With `VC_MEMORY` at 512K the box that refuses it is the original's, cell for cell, as the golden of `editor-big` has it where the original refuses the same file; without it the editor opens the file; and with a `VC_MEMORY` that is not a number of bytes the program does not start. `harness/capcheck.py` |
| `portable_c-tidy` | The common code, the Linux files and their test against clang's warnings, with the commands the build compiles them with, and against clang-tidy by `.clang-tidy`. `harness/tidycheck.py` |

### The scenarios on Linux

`portable_c-posix-scenarios` runs each scenario on a bench of its own,
a directory in `/tmp` whose home directory is laid out as the DOS
machine's C:. It drives the program through its test channel, `VC_TEST`,
which takes the scenario's keys as the BIOS's words and writes the grid
whenever the program waits for a key, on the clock of the DOS bench,
`VC_TEST_CLOCK`. It starts a DOS machine once, for the bench's setup
file, and keeps that in `build/posix-bench`.

The screens and files are held to the original's goldens with what is
declared for the portable C and, on top of that, for Linux: `ON_LINUX`
of `harness/deviations.py`, each part with its reason.

- **Masks.** What the comparison leaves out because it is the host's
  and no golden can expect it, found on the screen by a rule rather than
  listed as cells: the drive's letter and label, the drive menu's
  letters, the drive's free space in the Info panel, the time of a
  directory that the run wrote into, the place of the program's own
  files, and what DOS showed behind the panels after a command. Every
  other cell is compared.
- **Scenarios not run.** Those that rest on what only DOS has: Russia,
  the diskette, a DOS program or command. Each has its reason.
- **Scenarios cut.** Others are taken only as far as a step, where what
  follows is DOS's alone, a diskette or the original's directory, or is
  otherwise on Linux for a reason written there.
- **Steps left out.** DOS's device names, NUL, CON and the like, are
  ordinary file names on Linux. The steps that use them as devices are
  left out, or the scenario is cut where the original beeps for one.
- **Patches.** Where a mask cannot do, a patch of its own, in
  `harness/goldens/posix/`, says what the program for Linux is expected
  to show.

`LINUX_NOT_RUN`, `LINUX_CUT` and `LINUX_LEFT_OUT_OF` of
`harness/deviations.py` have every cut with its step, and each is named
by an entry of "The program for Linux" in `DEVIATIONS.md`.

Two things keep most scenarios whole on Linux. The bench creates its
directories and files in name order, so the tree and the file search,
which list a directory as it stands on the disk, list it alphabetically
on DOS as on Linux. And the files that scenarios work on are in
`H:\FILES` on Linux, where the DOS bench has them in `C:\FILES` (see
[testing.md](testing.md#files-to-work-on)).
