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
  attribute, the cursor, and the files it leaves on disk. A candidate and the
  original are run under DOS in an emulator, fed the same keystrokes, and
  compared. This is the test that will carry over to the port.

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
the program builds as `VC.EXE`, in the form in which compiled code can be
linked to it, and behaves as before. Nothing has been translated yet.

## Layout

| Path | Contents |
|---|---|
| `src/` | The Volkov Commander 4.05 source, adapted to assemble with JWasm |
| `port/` | The program on its way to C. So far it is `port/asm/`, the assembly, arranged to take compiled code next to it |
| `tools/` | `mkexe.c`, which makes the port's `VC.EXE` out of what the linker produces |
| `CMakeLists.txt` | Builds the assembler and the programs, and registers the tests |
| `harness/` | Compares a build with the original: byte for byte, and by running both under FreeDOS in QEMU |
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

It then builds the port, `build/port/VC.EXE`, for which it fetches a pinned
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

`port/asm/` starts as a copy of the assembly in `src/`, and routines will
leave it as they are translated to C. For that the program has to take
compiled code next to it, which a `.COM` file with under 300 bytes to spare
cannot. It is therefore linked as `VC.EXE`, though not the way an `.EXE`
usually is: it still loads and starts like the `.COM` file it was, and
still moves and reloads most of itself at run time, which will include the
compiled code. `tools/mkexe.c` describes how the file is made, and
`git diff --no-index src port/asm` shows what had to change in the program:
the code that starts it and that reloads it after a command, which now
measures in paragraphs and relocates what it has loaded.

## Testing

Needs `python3`, `qemu-system-i386` and `mtools`, in addition to the above.

```sh
ctest --test-dir build --output-on-failure
```

| Test | What it checks |
|---|---|
| `bytes-match-original` | `VC.COM` and `VCSETUP.COM` from `build/shareware-layout/` are as long as the originals, hold only filler in the reserved room, and are identical to the originals everywhere else |
| `start-screen` | `build/VC.COM` and the original `VC.COM` are started on identical machines; their screens, cursors and video modes must be the same |
| `start-screen-shareware-layout` | The same, for the copy with the reserved room |
| `port` | `build/port/VC.EXE` and the original are taken through every scenario and must show the same screen after every step and leave the same files on their disks |
| `harness-self-test` | The comparison itself: two runs of the original must match, and a copy with one letter changed must be reported as differing in exactly that cell |

On these machines the program under test sits on its own drive, `D:`, with
the original's help and configuration files, and is started in `C:\WORK`,
a directory with a small fixed set of files. Keeping it off the drive it
displays stops its file size from appearing on screen as free space.

Beyond the start screen the programs are compared by scenarios: lists of
key presses, written down in `harness/scenarios.py`. After the start and
after every step the screens must be the same, and at the end so must the
files on the disks. `python3 harness/compare.py FILE [SCENARIO...]` runs
scenarios for any `VC.COM` or `VC.EXE`, or all of them with `all`; without
a scenario it compares the start screens.

Where a build is meant to differ from the original, `harness/deviations.py`
says so and the comparison overlooks that and nothing else. There is one
such place: the banner names the edition, and the published source is of
another edition than the binaries shipped in 2000.

A run can also record which of the program's code was executed, from
QEMU's log of the code it translates (`harness/coverage.py`). No test uses
that yet.

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
