# Vibe Commander

<p align="center">
  <img src="docs/volkov-commander-4.05.png" width="720"
       alt="Volkov Commander 4.05 on a blue 80x25 text screen: a file panel listing C:\VC on the left, the Info panel on the right, a DOS prompt and the function key bar along the bottom">
</p>
<p align="center">
  <em>Volkov Commander 4.05, as it starts up on the day of its release, 16 June 2000.<br>
  The original binary running under FreeDOS, captured by the harness in this repository.</em>
</p>

A faithful recreation of
[Volkov Commander](https://en.wikipedia.org/wiki/Volkov_Commander) 4.05, the
DOS file manager written by Vsevolod V. Volkov, carried from its original
assembly source to a port for modern operating systems.

The project is also an experiment in how far an AI coding agent can take such
a recreation on its own. The engineering work is done by Claude; a human sets
the direction and reviews the results.

## Approach

Fidelity is judged against the original program, at two levels:

- **Bytes.** The binaries Volkov shipped in 2000 are the reference: what is
  built here must match them byte for byte. His own toolchain, Borland's
  TASM and TLINK, is commercial software that this project does not fetch
  or run; matching his output with a free assembler shows that nothing was
  lost by replacing it.
- **Behaviour.** Volkov Commander draws everything on an 80x25 text screen,
  so what it shows can be compared exactly: every character and colour
  attribute, the cursor, and the files it leaves on disk. The original is
  run under DOS in an emulator and fed keystrokes, and what it does is
  recorded; a candidate fed the same keystrokes has to do the same. This is
  the test that will carry over to the port.

The byte comparison has one complication. The binaries Volkov shipped were
shareware, and contain registration code, tables and strings that the source
he later published leaves out. That material is his and is not reproduced
here. Instead, the source can be assembled with room reserved for it: the
same number of bytes in the same places, filled with NOPs and zeros. Every
other byte then sits at the address it has in the original, and must be
identical to it. The reserved room is 109 of 65,090 bytes in `VC.COM` and
691 of 24,278 in `VCSETUP.COM`, in five places each.

The planned stages:

1. Build the original source with free tools, byte for byte.
2. Translate the program from assembly to C, still for DOS, a few routines
   at a time. The translated routines are linked into the assembly program
   in place of the ones they replace, so that at every step there is one
   working program to hold against the original. Bytes no longer match from
   here on; behaviour has to, bugs included, with every intended deviation
   listed explicitly.
3. Give the C program a layer for what it asks of the operating system, and
   put modern operating systems under it next to DOS.

Where it stands: the first stage is done. Outside the reserved room, both
programs come out identical to the originals. The second stage has begun:
the program builds as `VC.EXE` from assembly and C together, and the first
routines, those that write numbers, sizes, dates and times, run as C.

## Layout

| Path | Contents |
|---|---|
| `src/` | The Volkov Commander 4.05 source, adapted to assemble with JWasm |
| `port/` | The program on its way to C: `port/*.c` is what has been translated, `port/asm/` what is still assembly |
| `tools/` | `mkexe.c` makes the port's `VC.EXE` out of what the linker produces; `flagscan.py` examines the callers of a routine before it is translated |
| `CMakeLists.txt` | Builds the assembler and the programs, and registers the tests |
| `harness/` | Compares a build with the original: byte for byte, and by running it under FreeDOS in QEMU against what the original did there, recorded in `harness/goldens/` |
| `docs/` | Pictures for this README |

## Building

Needs CMake 3.24 or later, a C compiler and `git`.

```sh
cmake -B build
cmake --build build
```

This fetches [JWasm](https://github.com/Baron-von-Riedesel/JWasm), a free
MASM-compatible assembler, at a pinned commit, builds it, and assembles
`src/` into `build/VC.COM` and `build/VCSETUP.COM`. It also assembles a
second copy of both into `build/shareware-layout/`, with the room reserved
that the byte comparison needs; only the tests use that copy.

It then builds the port, `build/port/VC.EXE`, and next to it in
`build/port/signalling/` the variant that the tests run. For this it fetches a pinned
snapshot of [Open Watcom](https://github.com/open-watcom/open-watcom-v2), a
download of 150 MB. Open Watcom provides tools for Linux and macOS on x86-64
and ARM and for Windows on x86-64; the build has been run on Linux x86-64
only.

### Changes to the source

Everything changed since the source was imported can be listed with
`git diff $(git rev-list --max-parents=0 HEAD) -- src`. There are three
kinds of change.

**Syntax.** The original source is written for TASM; these changes make
JWasm accept it:

- The `JUMPS` directive is gone: JWasm lengthens out-of-range conditional
  jumps by itself. One `LOOP` that is out of range is spelled out as a jump
  over a jump.
- Structure fields are global names and labels inside procedures are global,
  as in TASM (`OPTION OLDSTRUCTS`, `OPTION NOSCOPED`).
- `LABEL` inside a `STRUC` became a zero-length field or, where the structure
  has initialised instances, an alias defined after it.
- Structure instances are written `<>` instead of `?`.
- Procedure headers use MASM's spelling for the language, `USES` and `LOCAL`.
- Code and code labels that sit in a data segment are preceded by
  `ASSUME CS:DGROUP`, an indirect `JMP` states its operand size, and
  `VcVers` is a text equate.
- `VCSETUP.ASM` declares its segments up front so that uninitialised data
  comes last and takes no space in the file.

**Encodings.** Where JWasm and TASM encode the same instruction
differently, the source now says which encoding to use, so that the output
matches the shipped binaries:

- Arithmetic on `AX` with a small constant carries `WORD PTR`, which selects
  the accumulator form of the opcode.
- A handful of jumps that TASM left in their long form are written that way
  explicitly, as `JMP NEAR PTR` or as a conditional jump over a jump.
- Two `LEA reg,address` are written as the `MOV` TASM turned them into, and
  one displacement is written so that it fits a byte.
- The byte-sized locals of `Input0` are declared two bytes long, because
  TASM gives each a whole word of stack.

**Layout.** `VCLAYOUT.INC` defines two macros, `ShwCode` and `ShwData`,
which are placed wherever the shipped binaries have something this source
does not. Each names a number of bytes. Normally they generate nothing.
When `SHAREWARE_LAYOUT` is defined they generate that many NOPs or zeros, so
that what follows lands where it does in the shipped binaries. The filler is
harmless: the program laid out this way still runs.

## The port

`port/` starts as a copy of the assembly in `src/` and ends as a C program.
In between it is both. A routine that has been translated is deleted from
`port/asm/` and leaves a few instructions behind that pass its callers on
to the C function; `git diff --no-index src port/asm` shows what is gone.
The C code keeps the names the routines and their data have in the
assembly, so that either can be found from the other.

The halves are linked into one program, `VC.EXE`, which is not built the
way a C program usually is. It still loads and starts like the `.COM` file
it was, keeps its data where the assembly put it, and moves and reloads
most of itself at run time, compiled code included. `port/vc.h` describes
what that asks of the C code, and `tools/mkexe.c` how the file is made.
The code that starts the program and reloads it after a command had to
change for this: it now measures in paragraphs and relocates what it has
loaded.

Every step is checked by the `port` test below, and nothing else decides
whether a translation is right. The routines translated so far are listed
in `harness/ported.py`.

The tests do not run the program that is shipped but a variant of it,
assembled with `HARNESS` defined. It writes a letter to a port of the PC
whenever it takes a key, waits for one, or ends, and so tells the tests
exactly when it has finished what a key started. The two differ by a few
dozen instructions, all in macros at the top of `port/asm/VC.ASM`.

## Testing

Needs `python3`, `qemu-system-i386` and `mtools`, in addition to the above.

```sh
ctest --test-dir build --output-on-failure
```

| Test | What it checks |
|---|---|
| `bytes-match-original` | `VC.COM` and `VCSETUP.COM` from `build/shareware-layout/` are as long as the originals, hold only filler in the reserved room, and are identical to the originals everywhere else |
| `port` | The port does what the original did in every scenario: the same screen after every step, the same files on its disks. Every instruction of the assembly that has been translated ran in the original on the way, and every C function that replaces it ran in the port. The port is not more than three times the size of the original. And the comparison itself: with one cell of a screen changed, it must report a difference |

### Scenarios and goldens

A scenario is a list of key presses, written down in `harness/scenarios.py`.
The original was taken through each one once, and what it showed after the
start and after every step, and what it left on its disks, is kept in
`harness/goldens/` as text. A build is right if it does the same. The
goldens are derived, never edited:

```sh
python3 harness/goldens.py derive [SCENARIO...]
```

Where a build is meant to differ from the original, `harness/deviations.py`
says how, and the golden is derived with that difference in it, so that it
holds exactly what a build should show. There is one such place: the banner
names the edition, and the published source is of another edition than the
binaries shipped in 2000.

`python3 harness/compare.py [--signals] FILE [SCENARIO...]` takes any
`VC.COM` or `VC.EXE` through scenarios and reports where it departs from
the goldens. No test runs the programs assembled from `src/`, which the
byte comparison answers for; this is how to run them.

`python3 harness/show.py STEP...` prints the screens that the original
shows after each of some steps, or with `--program` those of a build. It
is how to find out what keys do before writing them into a scenario.

### The machines

DOS is booted once for each country that scenarios need, the United States
and Russia, and the machine is saved as it is about to start the program.
Every scenario then gets a machine of its own in that state, which takes a
tenth of a second: the program under test on drive `D:` with the original's
help and configuration files, started in `C:\WORK`, on disks that no other
scenario has touched. Keeping the program off the drive it displays stops
its file size from appearing on screen as free space. A scenario that wants
the program set up differently does that itself, through the menus.

The program is set up as its distribution sets it up, with one exception:
the clock in the corner of the screen is off. It would put the time of day
on every screen, and the original takes longer to get through a scenario
when a golden is derived than a build does when it is tested. No setup file
is prepared for this. Whenever the harness sets up its machines, the
original is started once, made to switch the clock off in its Options menu
and to save its setup, and the `VC.INI` it writes is what every scenario
starts from. One scenario, `clock`, switches the clock on again.

### When a step is finished

The screen has to be read when the program has done all that a key makes
it do, and not before. The variant of the port that the tests run says so
itself (see [The port](#the-port)), and a scenario of twenty steps takes a
second or two. The original cannot say, so when its goldens are derived
the harness waits for the screen to stand still after every step. That is
slow, and needed only when a scenario is new or has changed.

Nothing waits without a limit. A program that hangs or resets its machine
fails its scenario within seconds, with the screen it got stuck on.

### What ran

Which code ran is taken from QEMU's log of the code it translates, which
`harness/coverage.py` matches against the program file. For the original
it is recorded with the goldens, and mapped to source lines through the
listing of the build in `build/shareware-layout/`, which has every
instruction at the address it has in the original.

The reserved room is listed in `harness/bytecheck.py`, with the offset and
length of each piece and a note of what the original has there.

## Running the original

Needs `python3`, `qemu-system-i386` and `mtools`.

```sh
python3 harness/screenshot.py
```

This downloads the Volkov Commander 4.05 shareware distribution and FreeDOS,
builds a boot floppy and a hard disk with the distribution unpacked in
`C:\VC`, boots the machine in QEMU, starts `VC` and waits for the screen to
stand still. It then writes to `out/`:

| File | Contents |
|---|---|
| `screen.png` | The display as the video card renders it |
| `screen.bin` | The text screen from video memory: 4000 bytes, a character and a colour attribute for each of the 80x25 cells |
| `screen.txt` | The characters of `screen.bin` as UTF-8 text |
| `screen.json` | Video mode, cursor position and shape, and which cells blink |

The run is repeatable: the PC's clock starts at noon on 16 June 2000 on every
boot and every file on its disks has a fixed date, so two runs give identical
files. The picture at the top is `screen.png` from
`python3 harness/screenshot.py --scale 3x4`, which enlarges the 720x400
display to roughly the 4:3 shape it had on a monitor.

## Origins

### The source

`src/` holds the contents of the 4.05 source archive published by the author:

- URL: <https://vc.vvv.kyiv.ua/download/src/vc405.zip>
- SHA-256: `c4ede94b833ea81da24483231de157bd64b67cf1ec66a77be383490046889c3e`

The first commit of this repository contains the 19 files of that archive
exactly as extracted. They are CP866 text with CRLF line endings, and git is
configured not to convert them.

The source builds two programs, `VC.COM` and `VCSETUP.COM`. It is a freeware
variant of 4.05: unlike the shareware binaries distributed in 2000, it has no
registration code.

### What the harness downloads

These are fetched into `downloads/` when first needed, checked against the
SHA-256 sums pinned in `harness/deps.py`, and never stored in the repository:

| What | From |
|---|---|
| Volkov Commander 4.05 shareware distribution | The author's site, <https://vc.vvv.kyiv.ua/download/vc405sw.zip> |
| FreeDOS 1.3 Floppy Edition, for its boot floppy | <https://www.ibiblio.org/pub/micro/pc-stuff/freedos/files/distributions/1.3/official/FD13-FloppyEdition.zip> |

### What the build downloads

CMake fetches these into the build directory, as pinned in
`CMakeLists.txt`:

| What | From |
|---|---|
| JWasm 2.20, as source at a pinned commit | <https://github.com/Baron-von-Riedesel/JWasm> |
| Open Watcom 2.0, the snapshot of 1 October 2026 with tools ready to run, pinned by SHA-256 | <https://github.com/open-watcom/open-watcom-v2/releases/tag/2026-10-01-Build> |

Both are open source under the Sybase Open Watcom Public License, which is
about the tools' own code. None of that code ends up in the programs built
here: the port is linked without Open Watcom's libraries.

## Licence

This project is released under the 3-clause BSD licence in `LICENSE`. That
covers everything written for it, including changes made to the files in
`src/`.

Volkov Commander is Copyright 1991-2000 Vsevolod V. Volkov and was released
by its author under the 2-clause BSD licence in `src/LICENSE.TXT`, which
continues to apply to his code.

The shareware distribution of Volkov Commander is proprietary and is not
part of this repository. The picture at the top shows it running.
