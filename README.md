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

Fidelity is judged against the original program's behaviour. Volkov
Commander draws everything on an 80x25 text screen, so what it shows can be
compared exactly: every character and colour attribute, the cursor, and the
files it leaves on disk. A candidate and the original are run under DOS in
an emulator, fed the same keystrokes, and compared.

A byte-for-byte comparison with what the author's own toolchain produces
would be a stronger test, but that toolchain, Borland's TASM and TLINK, is
commercial software that this project does not fetch or run. The binaries
he shipped in 2000 can stand in for it. Where JWasm and TASM encode the same
instruction differently, the source now selects the encoding found in them.
Comparing whole files with them is the next step.

The planned stages:

1. Build the original source with free tools.
2. Build the harness that runs the original and a candidate side by side.
3. Port the program to modern operating systems, held to the same
   comparison, with every intended deviation listed explicitly.

Where it stands: the source builds with a free assembler, and the resulting
`VC.COM` shows the same start screen as the original. That is one screen;
the harness cannot yet press keys, so everything behind the start screen is
still unchecked.

## Layout

| Path | Contents |
|---|---|
| `src/` | The Volkov Commander 4.05 source, adapted to assemble with JWasm |
| `CMakeLists.txt` | Builds the assembler and the two programs, and registers the tests |
| `harness/` | Runs Volkov Commander under FreeDOS in QEMU, captures its screen, and compares a build with the original |
| `docs/` | Pictures for this README |

## Building

Needs CMake 3.24 or later, a C compiler and `git`.

```sh
cmake -B build
cmake --build build
```

This fetches [JWasm](https://github.com/Baron-von-Riedesel/JWasm), a free
MASM-compatible assembler, at a pinned commit, builds it, and assembles
`src/` into `build/VC.COM` and `build/VCSETUP.COM`.

### Changes to the source

Everything changed since the source was imported can be listed with
`git diff $(git rev-list --max-parents=0 HEAD) -- src`. There are two
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

## Testing

Needs `python3`, `qemu-system-i386` and `mtools`, in addition to the above.

```sh
ctest --test-dir build --output-on-failure
```

| Test | What it checks |
|---|---|
| `start-screen` | `build/VC.COM` and the original `VC.COM` are started on identical machines; their screens, cursors and video modes must be the same |
| `harness-self-test` | The comparison itself: two runs of the original must match, and a copy with one letter changed must be reported as differing in exactly that cell |

On these machines the program under test sits on its own drive, `D:`, with
the original's help and configuration files, and is started in `C:\WORK`,
a directory with a small fixed set of files. Keeping it off the drive it
displays stops its file size from appearing on screen as free space.
`python3 harness/compare.py FILE` runs the comparison for any `VC.COM`.

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

CMake fetches the JWasm source from
<https://github.com/Baron-von-Riedesel/JWasm> at the commit pinned in
`CMakeLists.txt` (release 2.20) into the build directory. JWasm is open
source under the Sybase Open Watcom Public License.

## Licence

This project is released under the 3-clause BSD licence in `LICENSE`. That
covers everything written for it, including changes made to the files in
`src/`.

Volkov Commander is Copyright 1991-2000 Vsevolod V. Volkov and was released
by its author under the 2-clause BSD licence in `src/LICENSE.TXT`, which
continues to apply to his code.

The shareware distribution of Volkov Commander is proprietary and is not
part of this repository. The picture at the top shows it running.
