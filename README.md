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
assembly source to C, and from DOS to modern operating systems.

The program is here in three trees. `src/asm/` is the assembly: Volkov
Commander 4.05 as its author published it. `src/c/` is the C translation:
that program translated to C for DOS, behaving as the original does, bugs
included; it is finished, and is kept as it is. `src/portable_c/` is the
portable C: the line that leaves DOS behind. It began as a copy of `src/c/`,
is built and tested beside it, against the same recordings of the original,
and is where work goes on; where it is to behave otherwise than the
original, `src/portable_c/DEVIATIONS.md` says so, and has no entry yet.

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
  the test that will carry over to the C translation and the portable C.

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
programs come out identical to the originals. The second stage is nearly
done: the program builds as `VC.EXE`, and every routine of it runs as
C. In the order in which they were translated: those that write
numbers, sizes, dates and times, those
that write lines of text with their markup, those that copy, trim and
shorten texts, paths and file names, those that look up colours and put
characters and colours into the copy of the screen that the program draws
in, those that read and write hex digits and numbers and hold a file name
against a mask, those that filter and sort the list of files of a panel and
find a file in it, those that say where the panels are and where in them
the cursor is, those that draw a panel of files: its frame, its path, its
files, its status line and the cursor, those that draw what a panel
shows instead of files: the tree of directories, with the cursor on the
current directory, and the Info, and those that read from the disk what a
panel shows: the files of a directory with its extension file and the
sizes of its directories, and for the Info the sizes of the disk, its
label and the text of dirinfo. The viewer is C: opening a file, for the
editor too, finding the lines, keeping a part of a large file in the
memory, writing the screen, its keys and the mouse, looking for a text,
and the way back to the panels; its texts, boxes and dialogs are
constants of C, in `src/c/view_t.c`. The editor is C as a whole: the text in
the memory with the buffer of the line that the cursor is in, the screen
brought up to where the cursor is, as text or in hex, with its status
line, what every key does, the search, and saving the file, with its
texts, boxes and dialogs constants of C in `src/c/edit_t.c`. The commands
that start the two are C as well, by themselves or by a program that an
extension file names, which is run with the viewer's memory given up
for it. The keys of the
panels are C: those that move the cursor by lines, columns and pages,
select files one by one and by a mask, switch the panels off and on,
swap them, make them of another kind and read them again, Tab,
Enter, which runs a file or a command, the keys that change the
directory, those that go to the command before and after, and F10. The
loop that takes a key
and goes to its command is C, with what the mouse does at the panels,
and so are what it does on the way: drawing the two panels and reading
what they show when that has changed, the box that asks for another
drive when a disk cannot be read, the box of DOS's critical error, which
names the drive or the device and asks whether to try again, and saving
the setup when it has
changed; and leaving the program. The texts and boxes of those two, the
program's title and the button Ok are constants of C, in
`src/c/main_t.c`. What the program does once it has
been started, and again whenever a command has run, is C: reading the
setup file and the main extension file, or taking the defaults, the
banner, asking DOS for the memory of its data and making the data of
what was kept, and finding out what the display, DOS and the
environment are like. Where DOS enters the program is C too: the check
sum of the file, the second copy that brings the program back after a
command, the block of memory made of the part that goes while a command
runs, and the relocations of that part. What runs a
command with most of the program out of the memory and loads it again,
to which Enter hands over, is C, and is the outermost loop of the
program now: it calls the rest of the program, which comes back to it
with commands to run or to leave. So is the
program's watch on DOS: the interrupts of DOS that it takes to note every program that is
run and every one that stays in the memory, the records it keeps of
those, by which the map of the memory lists them and releases them, and
its answer to a program that asks for its data, as its setup program
does. The help is C as a
whole: reading its file, showing the index
and the pages in their window, and taking the keys and the mouse there;
its file of the assembly is the first that is gone, with its texts, its
box and its buttons constants of C.
So is the tree of directories away from its panel: reading it from the
disk, keeping it in its file there, and the box in which a directory is
chosen from it; its title, the name of its file and the boxes shown
while it is read and saved are constants of C, in `src/c/treebox_t.c`.
The file search is C as a whole: the command with its
dialog, the drives it searches and Go to, and the search itself, by name
and by contents through a directory and those below it, with its list
and the keys taken while it runs; its dialog, its buttons and their
colours are constants of C, in `src/c/find_t.c`. Deleting a file or a
directory with what is in it, copying, renaming and moving one, and setting
attributes, a date and a time, are C, and so is what goes through the files
of a directory for these, the sizes of directories and the file search, and
through the files that a mask names. The commands of these are C from
the key on: Copy and Rename with what they offer as the target, their
dialogs, the tree to choose the target from, and the renaming of a
directory of a tree panel, and those that make a directory, delete, set
attributes and change the volume label; the texts, boxes and dialogs of
copying and renaming are constants of C, in `src/c/copy_t.c`, and those of
making a directory, deleting, the attributes and the volume label in
`src/c/dirfunc_t.c`. The map of the memory is C as
well: the blocks of DOS, whose they are and which vectors point into
them, its window, and programs that have stayed in the memory released;
the buttons of the window are constants of C, in `src/c/memory_t.c`.
Going through the selected files
of a panel is C too. At the panels the user menu is C, with what makes
the commands of a line of a menu or of an extension file and writes the
list of the selected files for them, and so are the command line, with
Esc and the names of the selected files and the paths of the panels put
into it, the history of commands in its box, the speed search, and the
box that asks for the drive of a panel; what these have of texts, boxes
and dialogs, the mask to select by, the question before leaving and the
user menu's, are constants of C, in `src/c/keys_t.c`. The dialogs are C: a
box with its lines of text, the fields of a dialog, a line to type
into, a group to choose from, a box to tick and a line of buttons, with
the keys and the mouse that each takes, and what the fields hold handed
back at the end. Waiting for a key is C, with the clock that it keeps on
the screen, the tap on Alt and the keys that the mouse clicks on the key
bar, and so are what a key does to a line that is typed into, the speed
search, the key bar, the menu bar, the command line with its prompt, and
finding what an extension file says for a file. The screen blanker is
C, with the word to DOS that the program is idle; what the blanker draws
is on no screen of a scenario (see `TODO.md`). What all of these rest on
is C too: putting a window of
the program's copy of the screen on the display, keeping what a box
covers and putting it back, the frame of a box with its shadow and the
box growing, the cursor, the mouse with its pointer and the second click,
the keys that wait, the bell, capitals, every call of DOS with the error
that it may end in, opening a file, asking DOS for the current directory,
and asking the user for the disk of a drive that shares the floppy drive
with another. Saving the setup is C, and so is the jump from a key to its
command, which was a table in the assembly. The names of the menus in
the menu bar are constants of C, in `src/c/bars_t.c`, and the names of
the files that the program reads and the dialog that asks for a disk in
`src/c/system_t.c`.
The pull-down menus of the menu bar are C as well: the five menus with
their ticks and the items that cannot be chosen as things are, and the
keys and the mouse that the bar takes until an item is chosen. So are
the commands that the items stand for, in the same file of the source:
the kind of a panel, its order and its filter, a panel switched off and
on, the tree in a box to go to a directory, the sizes of directories,
the screen with as many lines as an EGA or a VGA has, two directories
compared, the configuration and the advanced options, the menu file and
the extension files opened in the editor, the switches of the Options
menu, and the setup saved; their texts, boxes and dialogs are constants
of C, in `src/c/commands_t.c`.
The main loop calls the commands. In the original a command is no
routine: the loop jumps to it by a table of the keys and the menu bar by
tables of the menus, and it jumps back into the loop at one of several
places, or on into another command. Each is a function now, which
returns where the program goes on, and the loop calls it from a `switch`
(`Command` of `src/c/main.c`); the tables of the menus hold the commands
by their numbers. What a command finds in the registers, as the command
before left them, is kept where one reads it. With that no instruction
of the program is assembly but those that are written into the C, and
no text or table. And nothing of it is assembled: the translation has no
assembly file, and its build runs no assembler for it. What the last
such file said, in what order the program's segments lie and which of
them count from the PSP, is said by options of the compiler and by the
linker's command (see [The C translation](#the-c-translation)).

The third stage has begun, in a tree of its own, `src/portable_c/`,
which started as a copy of that program. Its functions no longer take
their arguments in the registers of the assembly, and its constants have
the compiler's names. And it behaves otherwise than the original in one
thing, the first of those that only made sense on DOS: it stays in the
memory whole while a command runs, and has no option for that (see [The
portable C](#the-portable-c)).

## Layout

| Path | Contents |
|---|---|
| `src/asm/` | The assembly: the Volkov Commander 4.05 source, adapted to assemble with JWasm |
| `src/c/` | The C translation: the program in C, all of it, `src/c/*.c`, with the tables of a file in the one next to it whose name ends in `_t`, and two headers |
| `src/portable_c/` | The portable C: a copy of `src/c/` to begin with, laid out like it, with the original's setup program in the files whose names begin with `st`, and `DEVIATIONS.md`, which declares where it behaves otherwise than the original |
| `tools/` | `mkexe.c` makes the `VC.EXE` of the C translation out of what the linker produces; `figures.py` writes [the figures](#the-figures) of the portable C into this README |
| `CMakeLists.txt` | Builds the assembler and with it the original, compiles and links the two C trees, and registers the tests |
| `harness/` | Compares a build with the original: byte for byte, and by running it under FreeDOS in QEMU against what the original did there, recorded in `harness/goldens/`; where the portable C is declared to behave otherwise, against what it did itself, recorded in `harness/goldens/portable_c/` |
| `docs/` | Pictures for this README |

## Building

Needs CMake 3.24 or later, a C compiler and `git`.

```sh
cmake -B build
cmake --build build
```

The original is built by one set of tools and the two C trees by
another, and neither set touches the other's programs.

The original is assembled. The build fetches
[JWasm](https://github.com/Baron-von-Riedesel/JWasm), a free
MASM-compatible assembler, at a pinned commit, builds it, and assembles
`src/asm/`, the source as its author published it, into `build/VC.COM` and
`build/VCSETUP.COM`. It also assembles a second copy of both into
`build/shareware-layout/`, with the room reserved that the byte
comparison needs, and both there once more as object files, for their
relocations; only the tests use those. That is what the test
`asm-bytes` holds against the binaries that were shipped, and
what says where the original has its routines and its data. The
assembler also builds the driver of the tests, `harness/DRIVER.ASM`,
which is no part of either program.

The translation is compiled: `build/c/VC.EXE`, which is what the tests run
and hold against the original. It is C and nothing else, `src/c/*.c`,
and no assembler has a hand in it. What the compiler made of each C
file is written out as a listing, `build/c/*.lst`. For this the build
fetches a pinned
snapshot of [Open Watcom](https://github.com/open-watcom/open-watcom-v2), a
download of 150 MB. Open Watcom provides tools for Linux and macOS on x86-64
and ARM and for Windows on x86-64; the build has been run on Linux x86-64
only.

The portable C is built by the same compiler, from `src/portable_c/`
into `build/portable_c/`: `VC.EXE`, the linker's map and the listings.
The linker writes its `VC.EXE`; `tools/mkexe.c`, which lays out the C
translation's, has no hand in it (see [The portable
C](#the-portable-c)).

### Changes to the source

Everything changed since the source was imported can be listed with
`git diff $(git rev-list --max-parents=0 HEAD):src HEAD:src/asm`: the first
commit has the files in `src/` itself. There are three kinds of change.

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

## The C translation

`src/c/` started as a copy of the assembly in `src/asm/` and ends as a C
program. In between it was both. A routine that had been translated was
deleted from `src/c/asm/` and left a few instructions behind that passed
its callers on to the C function, and where C code needed a routine that
was still assembly, the assembly got an entry for it that C could call.
All of the code is C now, and those are gone, and `src/c/asm/` with them:
the last file there held no instruction and no data any more, only the
order of the program's segments. The C code keeps the names
the routines and their data have in the assembly, so that either can be
found from the other, and a function still takes its arguments in the
registers that the assembly called it with. The two routines that the
compiler calls to multiply and to divide numbers of 32 bits, which a C
library would bring, are C too, `src/c/arith.c`: the program has no C
library.

DOS, the BIOS and the mouse driver are called by software interrupts,
which C cannot express. The C code calls them in one file, `src/c/dos.c`,
and nowhere else: twelve small functions, each a line of C around the
instruction, which is inline assembly there, with the registers as its
arguments; one has three such instructions that may not be parted. Four more are there for a port of the machine and its
interrupt flag, for which the compiler has functions of its own, and for
DOS's routine that makes capitals. Everything else is plain C that calls
these, and reads the BIOS's data and the memory of the display through
pointers. That file is where the program will meet another operating
system; until the translation is done it hides nothing of DOS.

The part of the program that stays in the memory while a command runs is
C in `src/c/resident.c`, with its data. The
rest of the program is not in the memory then, `src/c/dos.c` with it, so the
build compiles that file a second time, into that part: one source, two
copies of its code. `src/c/resident.c` has the only other inline assembly of
the translation, four instructions where an interrupt arrives that the
program has taken, INT 21h and INT 27h: they make room on the stack for a
second return address, and an interrupt function of the compiler's takes
over, which saves the registers and finds the program's data. Such a
function has to go on in the handler that had the interrupt before, with the
stack as the interrupt left it, and C cannot change how deep the stack is
that it returns on: so it writes that handler's address into the room,
and returns there. The same four stand where DOS goes on when it has
ended the program by itself. In front of the four of INT 21h stand
thirteen more: every call of DOS that any program makes comes there
while the program is in the memory, and those that it does not deal
with, nearly all, are told from the others and passed on before
anything is pushed, by six comparisons, their jumps, and a far jump to
the handler before. So the program takes nothing of the stack of whoever
calls DOS, which may be short, and DOS is entered with the flags of
those comparisons, as in the original; compiled code saves registers
before it compares and promises nothing of the flags. And nine
instructions are the interrupt by which the COMMAND.COM in the memory
runs a command, INT 2Eh: it comes back with no stack and no data
segment, so both are set there, from the code segment, before any
compiled instruction runs, and the program goes on in a function of its
own on an empty stack. `src/c/main.c` has one instruction, which loads
DS with the segment of the main data, known only when the program runs:
where DOS calls the program for a critical error, and where the part
that stays in the memory calls the rest of the program, which is one
function to it, and gets DS back as it was.

The program is linked as `VC.EXE`, which is not built the way a C
program usually is. It still loads and starts like the `.COM` file it
was, keeps its data where the original has it, and gives up and reloads
most of itself at run time, compiled code included. `src/c/vc.h` describes
what that asks of the C code, and `tools/mkexe.c` how the file is made.
The code that begins the program and reloads it after a command had to
change for this: it measures in paragraphs and relocates what it has
loaded.

The file is the program as it lies in the memory. First comes the part
that stays there while a command runs: room for the PSP, which
`tools/mkexe.c` takes off, its compiled code, and its data
with the stack at the end, which are nothing but room in the file. That
data is of `src/c/resident.c`, the one C file
with data of its own: its texts, the numbers that `tools/mkexe.c`
writes, which it finds by their names in the linker's map, one
structure for what the part keeps, in the order of the original, and
the stack. Code and data of that part are one group to the linker,
which begins at the PSP, as the program's one segment did when it was a
`.COM` file: CS, DS and SS are on the PSP there, and every offset
counts from it. Then, a
paragraph further on, the part that goes while a command runs: its
constants first, which are a group of their own, then its code, in two
segments. The
original has that part where the data of the first will be and moves it
up as it starts, over itself; the translation is loaded with it in place, on
its own stack from the first instruction, and has only to make a block of
memory of it. That costs 6,384 bytes of the file, and it is what let
compiled code stand in the first part at all: nothing can lay
data over code that the compiler has yet to make. It also leaves the
code that begins the program in the memory, where the original lays its
data over that code.

None of that layout is in the C language. The order of the segments is
the linker's to be told, in its command; that the code and the data of
the first part are one group, and where each kind of data goes, are
options of the compiler; and `tools/mkexe.c` holds what comes out
against what the program needs, and stops the build where it differs.
[What rests on Open Watcom](#what-rests-on-open-watcom) has each such
place.

Every step was checked by the `c-scenarios` test below, and nothing else
decides whether a translation is right. The routines are listed there, batch
by batch, in `harness/ported.py`.

The program's texts and tables are checked by themselves, by the `c-data`
test: a table that no screen shows can be wrong in every scenario and
pass. The boxes and dialogs name their texts, their buttons and one
another by address, words that are offsets in the one segment the
assembly has them in. In C they are constants written with the names of
what they name, in files that hold nothing else (`src/c/help_t.c`, with
`src/c/tables.h` for how they are written): the compiler puts them into
a group of their own, where a word that names one is its offset, as it
is in the original's one segment. They are apart from the code because the code has its data
segment on memory that the program asks DOS for, and would read a
constant that it names there. The test holds each against the original
by its name: the bytes, and for a word that holds an address the thing
that it names, which the original's source, assembled once more as an
object file, tells by its relocations.

The tests run the program that is shipped, as they run the original: a
driver in the machine takes either through the scenarios (see [When a step
is finished](#when-a-step-is-finished)). The driver also writes the
program's stack out whenever a step ends, and the `c-scenarios` test reports
for every scenario how much of it the program never used.

The stack is where the translation departs from the original in what it
takes of the memory. The original has 256 bytes of it. Compiled code uses
more stack than the assembly it replaces, and a stack used beyond its bottom
would overwrite data without a trace on the screen, so the translation has
4,096 bytes, of which the scenarios use some 200. The stack is the end of
the part of the program that stays in memory while a command runs: that part
is 19,728 bytes where it was 12,704, and a command has 7,024 bytes less
than the original leaves it: 3,840 for the stack, and the rest because
compiled code is longer than the assembly it replaces, and because the
code that starts the program stays.

### What rests on Open Watcom

The translation is C, but it is not all C that any compiler would take or
would make the same program of. These are the places, so that whoever
carries the program to another compiler has them in one list. Each has
a comment where it is done.

First what the code is written in, which shows wherever it is used and
which a 16-bit program for DOS with the original's layouts cannot do
without:

| What | Where | How many |
|---|---|---|
| `__far` and `__near` pointers, and pointers based on a segment, `CODE` and `STACK` of `src/c/vc.h` | every file | some 1,200 `__far`, 475 `STACK`, 97 `CODE` |
| `#pragma aux`: the registers that a function takes and returns, kept from the assembly, and the name that the linker has for it | every file | 866 |
| `#pragma pack(1)` for the structures that are the original's | `src/c/vc.h`, `src/c/tables.h` and five files with structures of their own | 8 pairs |
| `__interrupt` functions, which save the registers and load DS | `FatErr` of `src/c/main.c`; `CtrlC`, `Int21`, `Int27`, `Exit05` of `src/c/resident.c` | 5 |
| `__loadds` functions, which load DS when they are called from the other part | `SetBrk`, `Release` of `src/c/resident.c` | 2 |
| `__declspec(naked)` functions, which are their inline assembly and nothing else | `New21`, `New27`, `Exit01`, `Int2E` of `src/c/resident.c` | 4 |
| Intrinsic functions: `inp`, `_disable`, `_enable` | `src/c/dos.c` | 3 |
| The two routines that the compiler calls for arithmetic on 32 bits, under its names and with its registers | `__U4M`, `__U4D` of `src/c/arith.c` | 2 |

Then what cannot be seen in the code: where the program is right only
because of what this compiler and its linker do with it. The language
promises none of these. `tools/mkexe.c` stops the build where one of
the first eleven does not hold; the others would show in the tests.

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The option `-nd=R` gives the data of a file a group of its own, `R_GROUP`, which its code reads through DS without loading a segment, in four segments of the class `FAR_DATA`: `RCONST`, `RCONST2`, `R_DATA`, `R_BSS` | `CMakeLists.txt`, for `src/c/resident.c` | `tools/mkexe.c` knows the four names, and refuses any other data |
| 2 | The option `-g=R_GROUP` puts the code segment into a group of that name, and the linker makes one group of two of the same name: the part that stays is one segment to the program, the PSP's | `CMakeLists.txt`, for `src/c/resident.c` and the second copy of `src/c/dos.c` | the group has to begin at 0 |
| 3 | The linker's `order` directive, which its help does not list for DOS: the order of the classes and of the segments within one | `CMakeLists.txt`, the linker's command | the checks of 2, 6, 8 and 11 |
| 4 | A segment holds what the object files have of it in their order: `resident.c` is linked first, `transit_t.c` first of the part that goes | `CMakeLists.txt` | the checks of 6 and 10 |
| 5 | The compiler writes the objects of the code segment that have a value in the order of the file, and those without in front of them: so `Vect21` is given a value, `= 0` | `src/c/resident.c` | the check of 6 |
| 6 | So the room of the PSP, `Psp`, is the first thing of the code segment and of the program | `src/c/resident.c`, its first lines | `Psp` has to be at 0 |
| 7 | What has a value is in `R_DATA` in the order of the file, so `Stays`, written last and with a value, zeros, is the last of it: the part is nothing but zeros from there on, which the check sum of the file leaves out | `src/c/resident.c` | from `Stays` to the part that goes the file has to be zeros, and the seven words that are written in have to stand in front |
| 8 | What has no value is in a segment of its own behind the others, `R_BSS`: the stack, `DataSav`, is the one object without, and so the end of the part that stays | `src/c/resident.c` | `R_BSS` may hold no other symbol, and `DataSav` has to be at its start |
| 9 | A far pointer to something has the linker's number of its segment in the high word, and a word of the part that stays that names the part that goes is left as that number: `TRN_SEG` | `src/c/resident.c` | a number that is not 0 has to be a segment behind the part that stays |
| 10 | The constants of the files without code are in `CONST2` of the compiler's group, `DGROUP`, and begin with `TrnBeg`, 32 bytes in which there is a whole paragraph | `src/c/transit_t.c`, `src/c/*_t.c` | `TrnBeg` has to stand where the stack ends; a file with code and such constants is refused |
| 11 | No code of the part that goes is within 4K of the stack, the constants being in front of it; and no segment of code reaches into the last 4K of its 64K. Both are for QEMU, not for the program | the `order` of 3; `c_second` in `CMakeLists.txt` | read in the linker's map |
| 12 | `md`, the main data, is declared as an object and named to the linker as `Psp`: the linker takes the offset of an object in the group that the object is of, 0 here | `src/c/vc.h` | every scenario |
| 13 | The segment of a far pointer made of a near one is DS, `MAIN_SEG`, and of one to a local variable it is SS, with the option `-zu`: `ResSeg` | `src/c/vc.h`, `src/c/misc.c` | every scenario |
| 14 | An `__interrupt` function saves the registers in a fixed order, which `struct Saved` is, and loads DS with its group, by a number that is relocated; `SecDat`, the one variable of `DGROUP`, is where `FatErr` finds the main data by that | `src/c/vc.h`, `src/c/main_t.c`, `src/c/main.c`, `src/c/resident.c` | the scenarios of the critical error and of the programs that stay |
| 15 | A variable with a value in a file without code is in `_DATA` of `DGROUP`, in the file | `src/c/main_t.c`, `SecDat` | `tools/mkexe.c` refuses one without a value |
| 16 | `_enable` is `STI` and `CLD`: the first call of the program, `IntOn`, clears the direction flag, which the compiled code needs clear and DOS does not promise | `src/c/resident.c`, `Alloc0` | every scenario |
| 17 | A far call to a function of the same segment is made `push cs` and a near call by the linker, and needs no relocation: so the part that stays can call its own far functions before its relocations are applied | `src/c/resident.c`, `Alloc0` | every scenario |

What else the notes of `CLAUDE.md` have under "C for the translation" is how
to write for this compiler, not what the program rests on: a register
that it ignores in a `#pragma aux`, a pointer that it truncates.

### Inline assembly

Every piece of it, 81 instructions in 14 pieces. The first nine are a
`#pragma aux` each in `src/c/dos.c`, put in place where the function of
that file calls them; no other file calls DOS, the BIOS or the mouse
driver.

| File | Piece | Instructions | Why it is not C |
|---|---|---|---|
| `src/c/dos.c` | `Dos` | 20: `NEG AX` for the carry flag, `PUSH` of BP and DS, nine that load the registers from a structure on the stack, `INT 21h`, five that store what DOS returns, `SBB AX,AX` for the carry flag, two `POP` | A software interrupt is no call that C has, DOS takes and returns more registers than a function can, DS among them, and its answer is the carry flag |
| `src/c/dos.c` | `Video` | 7: `PUSH BP`, `INT 10h`, `POP BP`, four that store AX to DX on the stack | The BIOS for the screen; some lose BP |
| `src/c/dos.c` | `VideoBuffer` | 3: `PUSH BP`, `INT 10h`, `POP BP` | The same, with an address in ES:DI |
| `src/c/dos.c` | `Keyboard` | 3: `INT 16h`, `PUSHF`, `POP DX` | The BIOS for the keyboard answers in the zero flag |
| `src/c/dos.c` | `Multiplex` | 1: `INT 2Fh` | The interrupt that the mouse driver and Windows are asked by |
| `src/c/dos.c` | `DosIdle` | 1: `INT 28h` | The program says that it is idle |
| `src/c/dos.c` | `DesqviewPause` | 6: `MOV AX` and `INT 15h`, three times | DESQview takes the program off its stack between the first and the third: no compiled instruction may stand between them |
| `src/c/dos.c` | `MouseDriver` | 1: `INT 33h` | The mouse driver |
| `src/c/dos.c` | `MouseState` | 4: `INT 33h`, three that store BX, CX and DX on the stack | The same, which answers in three registers |
| `src/c/main.c` | `DataSeg` | 1: `MOV DS,AX` | C has no word for a segment register. DS is put on the main data, whose segment is known only when the program runs: where DOS calls the program for a critical error, and around the main loop |
| `src/c/resident.c` | `New21` | 17: six `CMP` with their jumps, a far `JMP` through `Vect21`, and `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` to the interrupt function | Every call of DOS by any program comes here. One that the program does not deal with is passed on with no register changed, nothing taken of the caller's stack and the flags of the original's comparisons; compiled code saves registers first and promises nothing of flags |
| `src/c/resident.c` | `New27` | 4: `PUSHF`, `PUSH AX`, `PUSH AX`, `JMP` | Room on the stack for a second return address, so that the interrupt function can go on in the handler before by returning: C cannot change how deep the stack is that it returns on |
| `src/c/resident.c` | `Exit01` | 4: the same | Where DOS goes on when it has ended the program by itself, on the stack of the program that started this one, to which it goes on in the same way |
| `src/c/resident.c` | `Int2E` | 9: `INT 2Eh`, `CLI`, `MOV AX,CS`, `MOV SS,AX`, `MOV SP`, `STI`, `MOV DS,AX`, `CLD`, `JMP` | COMMAND.COM's interrupt for a command comes back with no stack and no data segment: both are set from CS before any compiled instruction can run, and the function that called cannot be returned into |

Three more instructions are the compiler's own, by its intrinsic
functions in `src/c/dos.c`: `IN` for a port of the machine, `CLI`, and
`STI` with `CLD`.

## The portable C

`src/portable_c/` began as a copy of the C translation. What changes
there first is how the program is written, toward C that any compiler
takes, and what follows says where it has left the C translation so far.
Where it behaves otherwise than the original, that is declared in
`src/portable_c/DEVIATIONS.md`. Its first entry is that [it stays in
the memory whole](#staying-in-the-memory-whole) while a command runs,
and so is one program, which the linker writes, where the C translation
is two parts laid out by a tool. Then come [what the program did for
the PC and for the programs around it](#what-else-has-gone), and what a
register happened to hold where DOS or the BIOS is called, which [the
functions of the system](#the-system-in-one-place) do not hand over; no
scenario shows either. Then [what a user of DOS saw of the
program](#what-a-user-sees-no-more), a command or an option that only
made sense there. And the original's second program, its setup program,
[is becoming a part of this one](#the-setup-program-in-it). In every
scenario that a deviation does not change the portable C is held
against the original's golden, as the C translation is.

It is compiled in the medium model, the option `-mm`, where the C
translation has the small one: a function that another file calls is far
because the compiler makes it so, and a static one near, and no
`#pragma aux` is needed to say which. The compiler then calls its two
routines for arithmetic on 32 bits far as well, so those of
`src/portable_c/arith.c` are far, and a file of either code segment may
multiply and divide such numbers.

The registers are gone from its functions. In the C translation a
function that another file calls takes its arguments and returns its
value in the registers that the assembly called it with, by a
`#pragma aux` that also says what it changes and gives it a name of its
own for the linker; there is no assembly left to call it, and in the
portable C such a function is called as the compiler calls any other,
under the name that the compiler gives it: 329 functions, and the type
of the five among them that are called through a table.

A function that hands back two things, where the assembly has two
registers or a register and the carry flag, returns them in the C
translation as the halves of one number of 32 bits, which the caller
takes apart. In the portable C such a function returns a structure
with members named for what they are: `GetMous` the buttons and the
place of the mouse, `OpenFil` what DOS answered and whether that is an
error, `EdLin` the cursor and whether anything changed, a command of
the keys the place of the loop where the program goes on, with the
panel and the item of the menu where that place takes them. A call
that still took such a number apart would not compile. Of the 114
functions that returned one, 104 return a structure. Five had one
thing to say, the same place of the loop from every return, which is
the end of a command in the assembly, a jump: they return nothing. Two
are gone. And three return a number that is one: `Timer` of
`system.c`, the BIOS's count of timer ticks, the compiler's `__U4M`,
and `Dos35` of `main.c`, an address as DOS has it, which is `Vector` of
`dos.c` since. Three numbers of 16 bits that had a second thing or a
flag above the first went the same way: how the editor was left, and
in the main loop where the program goes on after a command and what
the mouse asks for.

The lines that say something of registers still, and why:

| What | Where |
|---|---|
| Inline assembly: the instruction and the registers it takes and leaves | every piece of `src/portable_c/dos.c` |
| The compiler's own routines for arithmetic on 32 bits, which it calls with registers of its choosing | `__U4M`, `__U4D` of `arith.c` |
| DOS's routine for capitals, which is no function of the program: called through a pointer, with the letter in AL | `CaseMapFn` of `dos.c` |
| The command for COMMAND.COM's INT 2Eh, in SI as the interrupt takes it, for a function that is inline assembly | `Int2E` of `resident.c` |

The other lines of `#pragma aux` say that a function does not return,
`aborts`; [the figures](#the-figures) have how many lines there are in
the portable C, where the C translation has 863. None of them gives
anything a name but the two of
`arith.c`, whose names are the compiler's to choose. In the C
translation a constant that another file names has a `#pragma aux`
wherever it is defined or declared, which takes off the underscore that
the compiler puts in front: the assembly named it so.
`harness/datacheck.py` knows an object with the underscore or without,
so in the portable C those lines are gone, 361 of them, and with them
the three by which a file of constants had a second, near name for a
text that code names far: `src/portable_c/vc.h` declares those texts
near in such a file and far in any other. What was left beyond that
while the tree was still two parts, 80 lines in all, the names of what
the part that stays in the memory defined, of a second copy of `dos.c`
and of the main data, and the registers where one part called the
other, went with the two parts.

### The figures

What can be measured of the portable C, or counted in its source, and
changes with every step of the work, is written here and nowhere else:
how long its file is, how much of the memory it keeps, how near its
code segments are to where the scenarios' test lets one end, and how
much of it only Open Watcom takes. No test holds the length of its
file to anything: only the C translation's is held, to three times the
original's. `python3 tools/figures.py --write` puts the
table here, of the build in `build/` and of `src/portable_c/`, and is
run when a piece of work lands; `python3 tools/figures.py --check` says
whether the table is that of the build. The rest of this README,
`TODO.md` and `src/portable_c/DEVIATIONS.md` say these things without
the number. No test holds the table.

<!-- The figures: written by `python3 tools/figures.py --write`, not by hand. -->

| What | Figure |
|---|---|
| `VC.EXE` | 189,472 bytes |
| as a multiple of the original's 65,090 bytes | 2.91 |
| more than the C translation's 133,056 bytes | 56,416 bytes |
| The relocations in its header, which DOS applies | 960 |
| What it keeps of the memory while a command runs: the program and its panels | 199,392 bytes |
| What DOS has for a command then, on the bench of the tests | 226,144 bytes |
| The code segment `CTEXT` ends at | D3F1h, 7,183 bytes below the limit, F000h |
| The code segment `CTEXT2` ends at | B3C6h, 15,418 bytes below the limit, F000h |
| Inline assembly | 57 instructions in 10 pieces: 37 in 7 of `dos.c`, 20 in 3 of `resident.c` |
| Lines of `#pragma aux` | 19, of which 9 say that a function does not return |
| Intrinsic functions | 2: `_disable`, `_enable` |
| Pairs of `#pragma pack` | 7 |
| `__far`, `FAR`, `STACK`, `CODE`, as the test `portable_c-host` counts them: each place where the word stands in the code | 736, 62, 257, 96 |
| Calls outside `dos.c` that hand registers to an interrupt, or name a fixed address of the machine, where everything is to go through `sys.h`, as `tools/calls.py` counts them | none |

<!-- The end of the figures. -->

What the program keeps while a command runs is worked out, of the
length of its file and of two panels; what DOS has for a command is
that taken from what the bench of the tests has, which was seen once
on the Info panel: the top of `tools/figures.py` says how. The three
words in the last line are counted by the test `portable_c-host`,
which prints them in its summary, each place where the word stands in
the code; what else that test counts of what only Open Watcom reads,
by kinds, is in its own table, in `TODO.md`.

### Staying in the memory whole

The original is two parts. One stays in the memory while a command
runs, with the stack and a copy of the setup and of what the panels
show; the other, the code and the main data, is given back to DOS for
the command, and made again afterwards: the main data of the copy, and
the code, if the Options menu has Memory allocation switched on, by
loading it from the program's file. The C translation is laid out for
that, by `tools/mkexe.c`, and most of what [rests on Open
Watcom](#what-rests-on-open-watcom) there is the two parts held
together.

The portable C stays in the memory whole, and is one program: the first
of the features that only made sense on DOS to go from this tree, which
is to lose them one by one. `src/portable_c/DEVIATIONS.md` has what the
original does, what the portable C does, why, and the scenarios that
show it.

What went, and what each thing was for:

| What | What it was for |
|---|---|
| The line Memory allocation of the Options menu, its command `M73_00`, its page of the menu's help, and Shift with Enter | The switch, and the same turned round for one command |
| The block of the code given back to DOS before a command; `Exec10`, which asked for a block, ran the program's file as a second copy, held its check sum against the first copy's and said "Bad or missing"; in the entry, the check sum of the file, and the second copy, which knew itself by its command tail and moved its code into the first copy's block; `OvlPth`, where the environment has the program's file | Giving the code up and loading it again |
| `FreMem1`, and the call of DOS that found it | What the Info panel showed for free memory with the switch on |
| The main data given back to DOS in `F21_31`, and asked for again in `Init11`; `WCB1sav`, `WCB2sav` and `ComnIni`, 12.5 KB, with `F21_40` writing a panel there and `Expand` making one of it; `Init00` reading the setup to where that copy is, with no main data yet | Giving the data up and making it again of a copy |
| `struct Stays`, the group `R_GROUP` and the options `-nd=R -g=R_GROUP`; `Psp`, the room of the PSP at the start of the file; DS on the PSP in `resident.c`, and `md` named to the linker as `Psp` | The part that stays as one segment with the PSP, as a `.COM` file is |
| `dos.c` compiled a second time, with other names | That part could not call what was not in the memory |
| `TrnMain` called through a pointer, `Stay`, `Go`, `Stay11`, `TrnAdr`, `TRN_SEG`, `RunPar`; `SetBrk` and `Release` called through pointers and `__loadds`; `ResSeg` and `MemRun`; `STAYS` and the far pointers made of that part's segment in `start.c`, `enter.c`, `memory.c` and `main.c` | One part calling the other, wherever that was after a command |
| `SecDat` and `DataSeg`, the instruction `MOV DS,AX` | Finding the main data where DOS calls the program, and at the seam |
| `Reloc`, the relocations of the part that stays applied in the entry, `TrnBeg`, the block of the other part made by writing the header that DOS keeps in front of one, and the seven words that `tools/mkexe.c` wrote into the file | A file that the program loads and relocates itself, time after time |
| `tools/mkexe.c`, for this tree, with what it checked | Laying that file out, and holding it to what the code took for granted |

What stayed then, because a user or another program sees it, in
`resident.c` as before but as a file like any other: the commands run
through COMMAND.COM or its INT 2Eh, with their texts; the watch on DOS,
which notes the programs that stay in the memory and releases them,
with its entry of INT 21h that takes nothing of the caller's stack for
a call that it passes on; DOS's critical error and Ctrl-Break; the
interrupts taken and given back, also where DOS ends the program by
itself; and the program's own call, by which a setup program reads and
writes its setup. Where one of these could not stay as the original has
it, `DEVIATIONS.md` says what differs. Of these the watch has gone
since, with the map of the memory that it was for: see [What a user
sees no more](#what-a-user-sees-no-more).

How the program is laid out now. The linker writes `VC.EXE`, DOS loads
it and sets the words of it that hold a segment, and it starts at
`Alloc0` of `resident.c` on the stack that the file's header names. Its
data is three segments, because it is more than one holds, 62.6 KB of
main data, 8.9 KB of tables and 4 KB of stack:

- The compiler's own group, `DGROUP`, which DS is on in every function:
  the main data, `md`, a variable of `main.c` now, and the variables of
  the other files, which a file may have now. The main data is the
  first thing of that group, at offset 0, since much of the program
  takes it to be there: the tables name a place in it by its offset in
  the structure, and a routine that is handed the main data's segment
  for a panel's reads the main data as if it were one. That puts it at
  the start of the file, 62.6 KB of zeros that the C translation's file
  has not.
- The tables, the files whose names end in `_t`, which code reads
  through far pointers as before, in a group of their own.
- The stack, `stack.c`, in a group of its own and last, so the program
  ends where its stack does, which is how it knows its length.

The memory of the panels and the viewer is a block that the program
asks DOS for; for a command it gives back all of that but the panels.
So the main data, the tables and the stack are still reached in three
ways, `md` and near pointers, `__far`, and `STACK` for a pointer to a
local variable, and a constant of a code file is still `CODE`, in the
code segment: with 12 KB less of main data all four could be one, which
`TODO.md` has, with what stands in its way and what it waits on.

`VC.EXE` of the portable C is longer than the C translation's: the
main data is in the file, the header has the relocations that DOS
applies, and the part that stayed in the memory, with its copies and
its second `dos.c`, is gone. While a command runs it keeps the program
and its panels in the memory, where the C translation keeps 19,728
bytes and the original 12,704. [The figures](#the-figures) have how
long and how much.

How far the source is from what another compiler takes is counted, by
the test `portable_c-host` (see [Testing](#testing)): gcc reads every
file for a machine of 64 bits, where a pointer is no segment and offset
and does not fit a number of 32 bits, and each place that it refuses is
counted, with each that only Open Watcom reads, by kind and by file.
`harness/hostcheck.txt` has the counts as they stand, and `TODO.md`
what each kind waits for. The three words that say what segment a thing
is in are not among them: `src/portable_c/vc.h` takes `__far`, `STACK`
and `CODE` for nothing where the compiler is not Open Watcom's for the
8086, as a build for a machine without segments will.

What rests on Open Watcom, for the portable C. Of what the code is
written in, the first table above: `__far`, `STACK` and `CODE`,
`#pragma aux` and `#pragma pack(1)`, each as often as [the
figures](#the-figures) say; four of the five `__interrupt` functions,
`FatErr` of `main.c` and `CtrlC`, `Int21` and `Exit05` of `resident.c`,
`Int27` having gone with the watch on DOS; two `__loadds` functions,
`Alloc0`, where DOS enters the program, and `Exec2E`, where it goes on
behind INT 2Eh; three of the four `__declspec(naked)` functions,
`New21`, `Exit01` and `Int2E`, without `New27`; two of the three
intrinsic functions, `_disable` and `_enable`; the two routines for
arithmetic; and, new, `#pragma data_seg`, for the main data and for the
stack. The test `portable_c-host` counts these by
their kinds, and `TODO.md` has its table.
Of what cannot be seen in the code, the seventeen places of the second
table, seven are left, most in another form, and two are new, 3 and 7
here:

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The option `-nd=NAME` gives the data of a file a group of its own: `T_GROUP` for the tables, in which a word that names a text is its offset, and `S_GROUP` for the stack | `CMakeLists.txt`, for `src/portable_c/*_t.c` and `stack.c` | `portable_c-data` holds such a word against the offset in the group; every scenario |
| 2 | The linker's `order` directive, which its help does not list for DOS: the order of the classes | `CMakeLists.txt`, the linker's command | the checks of 3, 4 and 5 |
| 3 | `#pragma data_seg` makes a segment of the compiler's group, of the class that it names: the main data is of a class of its own, which the linker's command has first, and so is at offset 0 of `DGROUP` and of the program | `src/portable_c/main.c` | the tests refuse a program whose `md` is not at 0 (`facts` of `harness/driver.py`) |
| 4 | In a file compiled with `-nd=S` the same line makes a segment of the class `STACK` outside `DGROUP`; the linker takes a segment of that class for the stack and names its end in the file's header; given a value, zeros, it is in the file; and the linker's command has it last, so the program ends there | `src/portable_c/stack.c`, `Alloc0` of `resident.c` | `portable_c-scenarios` fails if the stack is not the last of the program, or not in the file |
| 5 | No code is within 4K of the stack, the tables being in front of it; and no segment of code reaches into the last 4K of its 64K. Both are for QEMU, not for the program | the `order` of 2; the files of the second code segment in `CMakeLists.txt` | `portable_c-scenarios` reads the linker's map, where `tools/mkexe.c` stopped the build |
| 6 | The segment of a far pointer made of a near one is DS, `MAIN_SEG` | `src/portable_c/vc.h` | every scenario |
| 7 | The segment of a far pointer to an object of a group is the group's, and `seg` and `offset` in `_asm` are those of the group too: so the stack that `Int2E` sets is the one that the file's header names, and a table is read at its offset in `AsmSeg` | `src/portable_c/resident.c`, `misc.c` | the scenarios of INT 2Eh, `quick`; every scenario |
| 8 | An `__interrupt` function saves the registers in a fixed order, which `struct Saved` is, and loads DS with `DGROUP`, as a `__loadds` function does | `src/portable_c/vc.h`, `main.c`, `resident.c` | the scenarios of the critical error, of the program's own call, `asked`, and of the program ended by DOS, `ended` |
| 9 | `_enable` is `STI` and `CLD`: the first call of the program, `IntOn`, clears the direction flag, which the compiled code needs clear and DOS does not promise | `src/portable_c/resident.c`, `Alloc0` | every scenario |

Gone are 2, 4 to 7, 9, 10, 12, 15 and 17 of that table, and with 15 the
rule that a file of code may have no variable and no string: nothing
checks the object files of the portable C, and nothing has to.

Inline assembly, for the portable C, is as much as [the
figures](#the-figures) say. Of the nine pieces of `dos.c` in the table
above, `VideoBuffer` and `DesqviewPause` are
[gone](#what-else-has-gone), and the others are as they were. `DataSeg`
of `main.c` is gone. Of `resident.c`, `New27` is
[gone](#what-a-user-sees-no-more), and `New21` is eight instructions
where it was seventeen: three `CMP` with their jumps, for the one call
that the program still deals with, its own, the jump to the interrupt
function that answers it, and the far `JMP` through `Vect21` for every
other call; the three that made room on the stack are gone, since that
function returns to whoever called and goes on in no other handler.
`Exit01` is as it was. `Int2E` is an instruction shorter: `INT 2Eh`, `CLI`,
`MOV AX` with the segment of the stack, `MOV SS,AX`, `MOV SP`, `STI`,
`CLD` and the jump; it no longer loads DS, which the function that it
jumps to does for itself. Of the three instructions that are the
compiler's own, by its intrinsic functions, `IN` is gone too: the
program reads no port.

### What else has gone

After the two parts, what the program did only because it ran on a PC
under DOS, next to other programs of that time, and that no screen of
a scenario shows. Each is an entry of `src/portable_c/DEVIATIONS.md`,
by its number here, and is gone outright: the code, the calls, and
what the main data kept of it, whose bytes stay where they are and are
neither set nor read, since the layout of the main data is the
original's.

| | What | What it was for |
|---|---|---|
| 2 | `TopView` and `TopScr` of `dos.c`, the inline assembly `VideoBuffer`, their calls in `IniData` of `start.c` and in `Window` | Drawing into the buffer that TopView or DESQview shows in place of the display's memory, and telling them what has changed there |
| 3 | `DvPause` of `dos.c`, the inline assembly `DesqviewPause`, its call in `LeaveTm` of `blank.c`, the call of DOS in `IniData` that asked for DESQview, and the two places that went by the answer, in `Input0` and in the editor's loop of keys | Giving the rest of the program's turn to the other programs of DESQview, and leaving Alt and the keyboard's buffer to it |
| 4 | The call of INT 2Fh with AX 1680h in `LeaveTm` | Giving the rest of the program's turn to Windows, or to whatever else ran programs of DOS as guests. The call of INT 28h, DOS's own word that the program is idle, stays |
| 5 | The test of the display in `IniData`, the waiting in `Window` and `MovScr` of `window.c`, `InPort` of `dos.c` and the intrinsic function `inp` | Writing to the memory of a CGA only while it does not draw, which otherwise shows as snow. Snow mode stays a byte of the setup, kept as it is found |

### The system in one place

What the program asks of the system that it runs on, it is to ask in one
place, and the rest of it is not to know which system that is. The C
translation has one place for the instruction only: every file that
wants something of DOS fills a structure of registers, with the number
of the service among them, and hands it to `Intr21` or `Int21d`, and so
for the BIOS: 348 calls in 25 files when this began, each of which knows
DOS.

In the portable C that place is `src/portable_c/sys.h`: 84 functions
named for what is asked, with no register, no number of a service and no
interrupt in them, and no word of Open Watcom: the test `portable_c-host`
has the file clean. `dos.c` is those functions for DOS on a PC, and the
one file that knows how DOS is asked: no other file of the portable C
has a structure of registers, the number of a service, or an interrupt
that it calls. The same functions for another system will be another
file beside it.

| What | Functions |
|---|---|
| Files: made, opened, closed, read, written, a place in one, deleted, renamed, their attributes and their date and time, what a handle is, a full path | 17 |
| Files by a file control block: a mask deleted at one call, a label | 2 |
| Directories: what is in one, made, removed, the current one | 8 |
| Drives: the current one, the room of one, a drive that shares its unit, a network, a disk that stays in, a CD-ROM | 8 |
| Memory: a block taken, given back, made another size, and where the system looks for one | 5 |
| The time, the country and its capitals, the version, Ctrl-C, idling | 8 |
| Other programs: one run, this one ended, and what DOS keeps of this one | 4 |
| The console: a character and a text where the cursor is | 2 |
| The keyboard: a key that waits, a key taken, Shift and the like, Ins | 4 |
| The screen: its mode and page, the cursor, a window scrolled, a cell read and written, a character typed, the font of an EGA and what Print Screen prints of its rows, where the cells are and the cells themselves | 17 |
| The mouse: whether there is one, its pointer shown, placed and shaped, where it is | 8 |
| The timer | 1 |

What they take and answer with is still as DOS and the BIOS have it: a
drive by its letter, a path with backslashes, DOS's attributes, dates,
times and numbers of errors, a key with its scan code, a cell with its
colour, memory by the segment. The program has those shapes everywhere
and the screens show them; another system has to answer in them.

One function is one thing asked. Where the program asks several things
in a row, the row is still in the program, in the original's order,
since what DOS is asked and in which order is behaviour that the
scenarios see. Three things of the original's are in the functions by
name, where they were in what a register happened to hold:

- How a call is asked. The original calls DOS through its routine
  `Intr21` where a drive may not be ready, which lets the program's box
  for that come up, makes the error 53h of a call that the user gave up
  there, and asks DOS for its extended error behind a call that failed;
  and it calls DOS itself where it does none of that. The functions of
  files, directories and drives are of the first kind, and the five
  that the original also calls the second way are functions of their
  own, with Bare in their names.
- The carry flag before a call. DOS leaves it as it finds it at two of
  its calls, and `Intr21` takes a flag that is set for an error: the two
  functions take the flag as an argument, `flagged`.
- What a call leaves. Every answer has what DOS left in AX, also where
  nothing failed, since some callers go on with that; what else a call
  answers in is a member with a name.

What the functions do not take is what a register happened to hold
where the call does not read it, the low byte of AX at most calls of
DOS: the original hands DOS and the BIOS whatever the code before left
there, and the portable C hands them 0. No screen shows that, and
`src/portable_c/DEVIATIONS.md` declares it, with every place.

Not in the header, and in `dos.c` all the same, is what only DOS on a
PC has: where an interrupt goes, by DOS or written into the machine's
table, which the program takes for DOS's critical error, for Ctrl-Break
and Ctrl-C and for its own call of DOS; and the machine let interrupt
again. `vc.h` declares those four. What DOS enters of the program is
where it was, and is all that still knows an interrupt outside `dos.c`:
`FatErr` of `main.c` for the critical error, and in `resident.c` the
entry of the program, the entries of INT 21h and of Ctrl-C, where DOS
goes on when it ends the program, and the call of COMMAND.COM's own
interrupt for Quick execute, each with what assembly it has. `TODO.md`
says what is to become of them. The map of the memory and the watch on
DOS, which were of DOS alone, [are gone](#what-a-user-sees-no-more).

Every file asks the system through the header: the 348 calls went over
a file at a time, and the functions by which a structure of registers
was handed to an interrupt are gone with the last of them. `CLAUDE.md`,
"Moving a file to the system's functions", has what that met and how a
function is added. `python3 tools/calls.py` counts such calls, none in
the portable C and all of them in the C translation, and [the
figures](#the-figures) say so.

### What a user sees no more

Then the features that a user of the program sees and that only made
sense on DOS. Each is an entry of `src/portable_c/DEVIATIONS.md`, by
its number here, and is gone outright, not kept as a setting that does
nothing: its command with its key, its line of a menu and its name in
the key bar, its code and its data. What the main data kept of it keeps
its bytes, and a byte of the setup that switched it is kept as it is
found, so the setup file is the original's. The scenarios that showed
such a thing are not run for the portable C, or are run without the
steps that went there, against a golden of the portable C's own:
`DEVIATIONS.md` has each.

| | What | What it was for |
|---|---|---|
| 7 | The map of the memory, Alt+F5, Memory info of the Commands menu: `memory.c` and `memory_t.c`. And the watch on DOS that it rested on, in `resident.c`: the entry of INT 27h, `New27` with `Int27`; in the entry of INT 21h the three calls by which a program is run, stays in the memory or gives a block back; the list of the programs that have stayed and their records, `ExeProg`, `AddProg`, `FreeBlk`, `ChkBlk`, `Release`; `GetStat`; and the programs released when the program is left | Listing what DOS has given to whom, and getting a program out of the memory again that was run from here and has stayed, which DOS had no way to do |

What is left of the watch is what was not for the map: the program
still takes INT 21h, for its own call, by which a setup program reads
and writes its setup in the memory, and hands every other call on
untouched.

### The setup program in it

The original is two programs, and the second, `VCSETUP.COM`, is where
most of the setup is set: every colour, and the options that the main
program's menus have not. It exists only as assembly,
`src/asm/VCSETUP.ASM` with two files of routines that are for the most
part copies of the main program's. In the portable C it is becoming a
part of the one program, entry 8 of `src/portable_c/DEVIATIONS.md`: an
item of the Options menu, " seTup...", behind which the setup program
is on the screen as the original's is when it is run from the
program's command line, over what DOS had shown. It is translated as it
is: it reads the setup file and edits that, and the program takes what
was saved when the setup is left, as the original's running copy is
handed it by a call of DOS.

Its files have names that begin with `st`, and its routines and tables
have `St` in front of the names that they have in the assembly, where
many are the main program's names too. `stmain.c` has the setup
entered and left, its file, its menu and the setup that the program
has without a file; `stmain_t.c` its tables. `sthelp.c` has its help,
with `sthelp_t.c`: the seven pages that the help file has for the
setup, read when the setup is entered, and the page of where the user
is in a window, for F1. What both programs have is there once: the boxes, the dialogs, the bars and the mouse are the main
program's, and the three routines that the setup program has otherwise
go by a variable that says the setup is open, `StOpen`: `PutKey0`,
which has the setup's key bars then, `Dialog`, and `Input`, where F1 is
the setup's own help. What the setup program has in a segment of its
own is the main data here, in buffers that the main program makes anew
after a command anyway.

Of the menu's nine items two do something so far, Set default
configuration and Exit, and F1 is the help: the Configuration, Screen
and Mouse, the Panel options, the three palettes and the Country info
are to come, each in a file of its own, and so is the mouse at the
menu. `TODO.md`
has the plan, "The setup program in the portable C", with what each
piece brings and what is declared for it.

## Testing

Needs `python3`, `qemu-system-i386` and `mtools`, in addition to the above.
One test, `portable_c-host`, asks `gcc`, and is skipped where there is
none.

```sh
ctest --test-dir build --output-on-failure
```

There are eight tests: one of the assembly, three of each C tree,
named by the tree that they judge, `c-` for the C translation and
`portable_c-` for the portable C, and one more of the portable C, of
its source. The two trees are held to the same checks, against the same
goldens. Six of the tests run no machine and take a second or two;
`c-scenarios` and `portable_c-scenarios` each run every scenario, in a
little under two minutes on a machine that does nothing else, so all
eight take three minutes and forty seconds.

| Test | What it checks |
|---|---|
| `asm-bytes` | `VC.COM` and `VCSETUP.COM` from `build/shareware-layout/` are as long as the originals, hold only filler in the reserved room, and are identical to the originals everywhere else |
| `c-data` | Every text, box, dialog and table of the translation that has the name of one of the original holds the original's bytes; a word that holds an address has that of what the original names there. And none is shorter than what the code reads of it, which `harness/reads.py` finds: a name that is copied as 13 bytes is compared as 13. No machine is run for it |
| `c-arith` | The two routines of `src/c/arith.c`, which the compiled code calls to multiply and to divide numbers of 32 bits, give what Python gives for some 4,700 pairs of numbers, those at the edges among them: the product, the quotient and the remainder. They are run as the bytes that the compiler made of them, called near or far as each returns, on a model of the processor that knows their instructions, so it also shows a register that is not kept and a write to the caller's stack. No scenario divides by more than 16 bits, and none would show a result that is wrong for some numbers only. No machine is run for it |
| `portable_c-data`, `portable_c-arith`, `portable_c-scenarios` | The same three checks of the portable C, `build/portable_c/`: what is said of the translation in the rows above and below holds for it, but where `src/portable_c/DEVIATIONS.md` declares otherwise. A table that is declared to differ is held against the original's with that difference made to it. A text, box, dialog or table of the setup program, which has `St` in front of its name in this tree, is held against the bytes of the original's `VCSETUP.COM` in the same way. A scenario that is declared to change is held against a golden of the portable C's own, and one that only shows what the portable C has no more is not run; the test says how many scenarios were held against the original, and names the others. Its file is held to no multiple of the original's size: the test says how long it is, and holds it to what the machine of the tests asks of its layout |
| `portable_c-host` | The source of the portable C as another compiler reads it: `harness/hostcheck.py` has gcc read every file of `src/portable_c/` for a machine of 64 bits, as strict C of 1999, and counts by kind what gcc refuses, a pointer cast to a number of 16 or 32 bits, a number made a pointer, a structure that is not as long as on DOS, and what only Open Watcom reads, an interrupt function, inline assembly, a `#pragma` of its own. `harness/hostcheck.txt` has what was counted last, of every file, and the test fails where a file has another count of any kind: more, and a clean file that is clean no more, and fewer too, so that the record is always what is so, and what a change did to it shows in the commit. `python3 harness/hostcheck.py --record` writes the record anew, `--show` lists the places of a file or of a kind, and `--macros` says which macros make the casts. Nothing is compiled or run for it. It is skipped where there is no gcc |
| `c-scenarios` | The translation does what the original did in every scenario: the same screen after every step, the same files on its disks. Every instruction of the assembly that has been translated ran in the original on the way, and every C function that replaces it ran in the translation. The translation is not more than three times the size of the original, and leaves at least half of its stack untouched in every scenario. And the comparison itself: with one cell of a screen changed, or one entry gone from a disk, it must report a difference |

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
holds exactly what a build should show. There are three such places. The
banner names the edition, and the published source is of another edition
than the binaries shipped in 2000: that holds for as much of the name as
shows, its beginning next to a panel on the right or its end next to one
on the left. The program names its own file when
it cannot load it again after a command: the original is `VC.COM`, the
translation `VC.EXE`. And the command that starts the original's setup
program, of which the next paragraph says more, is no line of what a
build shows.

The original is two programs, `VC.COM` and its setup program,
`VCSETUP.COM`, and the portable C is to be one, with the setup an item of
its Options menu. A scenario may be of the setup program: it says so in
`harness/scenarios.py`, and the original then has `VCSETUP.COM` of its
distribution next to it on the bench, which a step starts from the
original's own command line, with the banner that it prints sent
nowhere. Its golden is what the setup program showed, driven and
recorded as the main program is, and the list of what ran that is kept
with it is of `VCSETUP.COM`, apart from those of `VC.COM`. A tree that
has the setup in itself is taken through the same steps, but for the
keys of its menu's item in place of the command, and has to show the
same, screen for screen: behind the setup program is what DOS has shown,
and the one line there that a build has not is the command's own. The C
translation has no setup program and is taken through none of these
scenarios. The top of `harness/scenarios.py` says how such a scenario is
written, derived and declared; `setup` is the first.

Some numbers on the screen are not compared at all: the bytes that the
editor says it has free, the memory that the Info panel says is free, and
what `mem`, run as a command, says is in use and free. All follow the
size of the program, the last two the size of the part that stays in
memory while a command runs, or the Info's that of all of it when Memory
allocation is switched off. The translation is larger than the original and
has less, by more with every routine translated, and the numbers are not
worth an expectation that follows the size of the build. Both screens
have `#` in their place when a build is held against a golden, so a
golden that was derived with a number still in it need not be derived
again.

What the portable C is declared to do otherwise than the original is
another matter, since the original cannot be made to show it. A scenario
that such a deviation changes has a golden of that tree's own, in
`harness/goldens/portable_c/`, recorded from the tree as it is built
and read by whoever records it:

```sh
python3 harness/goldens.py record --tree portable_c [SCENARIO...]
python3 harness/goldens.py changes --tree portable_c [SCENARIO...]
```

The second prints how each such golden differs from the original's, of
every screen the rows that do. A golden of that kind says in its first
line where it is from. It is no measure of the program: it shows that
nothing has changed since it was recorded. Which scenarios have one,
which are not run for the tree at all, which step is left out of every
scenario for it, and which steps of one scenario, where they pass
through something that the tree has no more, is in
`harness/deviations.py`, under `TREES`, as
`src/portable_c/DEVIATIONS.md` declares it; a scenario cannot be
recorded for a tree that is not declared to change it.

Two more deviations show on no screen, and are listed in the same file.
When the original tries again to load itself after a command, it stores a
word through a segment register that is left on the DOS environment, and
the word lands in other data of its own. The translation stores it where it
belongs. And of two codes in the markup of its texts, one for a colour and
one for a character, the original's routine that writes a line has each
for the other, unlike the routine that measures it and unlike Norton
Guides, where the markup comes from. The translation has them agree. No text
uses either code.

`python3 harness/compare.py FILE [SCENARIO...]` takes any
`VC.COM` or `VC.EXE` through scenarios and reports where it departs from
the goldens. For a `VC.COM` it needs the listing that the build writes
next to the file, and for the translation's `VC.EXE` the linker's map there.
No test runs the programs assembled from `src/asm/`, which the
byte comparison answers for; this is how to run them. The copy with the
reserved room has no edition name in its banner, and differs from the
goldens wherever the banner shows.

`python3 harness/show.py STEP...` prints the screens that the original
shows after each of some steps, or with `--program` those of a build. It
is how to find out what keys do before writing them into a scenario. With
`--changes` it prints of each screen only the rows that differ from the
one before, which is most of what there is to see; rows that are again as
they were after an earlier step, as when a dialog closes, it names in a
line. A step that the program does not get through ends the run after
five minutes, as in a test, or after `--limit SECONDS`, with the screen
it stopped on. With `--setup` the bench has the original's setup program
for a step to start.

`python3 harness/reach.py ROUTINE...` says of each routine of the original
how many of its instructions some scenario has run and which are left, by
their addresses, in a line; with `--list` it prints those that are left as
the listing has them. With `--steps` it says which of them some steps
would reach, and takes `--limit` like `show.py`. With `--setup` the
routines are those of the original's setup program, and what has run is
what the scenarios of the setup program have run of it.

`python3 harness/notice.py FILE OLD NEW` makes one change to a copy of a
C tree, the one that `FILE` is of, builds it, and says which scenario
notices, or that none does.
That an instruction ran in the original does not mean that a screen shows
what it is for; this is how to find out, for a branch whose effect cannot be
pointed to in a golden.

`python3 harness/arithcheck.py build/c` is the test `c-arith` by
itself, and with `build/portable_c` that of the portable C; with
`--pairs N` it takes that many random pairs of numbers
where the test takes 4,000. The two routines were held to 200,000 when
they were written, which takes two and a half minutes.

### The machines

DOS is booted once for each country that scenarios need, the United States
and Russia, and the machine is saved as it is about to start the program.
Every scenario then gets a machine of its own in that state, which takes a
tenth of a second: the program under test on drive `D:` with the original's
help and configuration files, started in `C:\WORK`, on disks that no other
scenario has touched, and on a third disk, one without a file system, the
script of the scenario. Keeping the program off the drive it displays stops
its file size from appearing on screen as free space. A scenario that wants
the program set up differently does that itself, through the menus. When a
build is tested, up to four scenarios run side by side.

What the machines need while they run, the images of their disks above
all, 60 to 130 MB for a scenario, is kept in `build/tmp` of the checkout
and removed when a run ends; what a run has left there by being killed,
the next run removes. A directory that the environment names for
temporary files, by `TMPDIR`, is used in its place. So is the system's,
with a line that says so, where the checkout lies so deep that the
socket by which QEMU is spoken to would have too long a path there.
`workdir` of `harness/dos.py` is the one place that makes such a
directory.

The hard disks are SD cards, each in a controller of its own, which the
BIOS serves like any hard disk; DOS and the program know a disk only
through the BIOS. QEMU reads a sector of an SD card, as of a diskette,
while the machine executes the instruction that asks for it. A disk on
its IDE controller it reads in the background, and the BIOS asks in a
loop whether the sector is there yet: how often, depends on how busy the
computer is, and that made a machine's time of day differ from one run
to the next (see below). QEMU does not keep all of such a controller in
a saved state, so the harness notes where the BIOS has put each one and
tells the machine that goes on from the state, through the ports that
the BIOS used.

Where DOS loads the program is chosen: the driver of the tests keeps some
memory more than it needs, so that the original's stack ends where a page
of the machine's memory ends. QEMU has to look at every write to a page
with code in it that it has run, and the program has its code right
behind its stack: a push is many times slower there than elsewhere, which
is a third of the time of a scenario with dialogs or more. The translation
has its texts behind its stack, more than a page of them, and does not
depend on where it is loaded.
`python3 harness/place.py` says where the stacks of the original and of
the translation end on each bench.

Both machines have a mouse: the PS/2 mouse of QEMU's PC, and FreeDOS's
driver for it, CuteMouse, which the boot floppy lacks and the harness adds.
The program finds the driver and shows the pointer, a cell of the screen
with its colours exchanged, in the middle of every screen until a
scenario moves it. A step moves the mouse and presses its buttons by the
cell it goes to: "Click@3,5", "RightClick@3,6", "MiddleClick@3,7",
"Move@40,12"; a button held while the pointer moves is "Click@3,5>3,2".
CuteMouse takes a PS/2 mouse for one of two buttons until it is told of
the third, so the bench loads it with its option for that, /3.

The Russian machine also has a Russian keyboard: the keyboard driver of
FreeDOS with its Russian layout, which the boot floppy lacks and the
harness adds. It types Latin letters until Alt and the right Shift select
its Cyrillic ones, and a scenario can then type Russian text. The driver
takes 40 KB, so this machine shows less memory free than the other. The
other machine has the keyboard a PC has by itself, and no way to type a
letter beyond ASCII: Alt with the numeric keypad types nothing there.

The program is set up as its distribution sets it up, with one exception:
the clock in the corner of the screen is off. It would put the time of day
on every screen, and the machine's time is counted in the instructions it
executes (see below), of which the translation takes more than the original
to get through a scenario. No setup file
is prepared for this. Whenever the harness sets up its machines, the
original is started once, made to switch the clock off in its Options menu
and to save its setup, and the `VC.INI` it writes is what every scenario
starts from. One scenario, `clock`, switches the clock on again.

### When a step is finished

The screen has to be read when the program has done all that a key makes
it do, and not before; and a key has to come when the program is ready for
it. Neither is decided from outside the machine. DOS loads a small
resident program before Volkov Commander, the driver of the tests
(`harness/DRIVER.ASM`, with `harness/driver.py` for the host's side), and
that takes the program through the steps of the scenario's script.

The driver stands in for nothing. A key is the scan codes that the
keyboard would send, handed to the keyboard controller with the command
it has for that, so the BIOS, and on the Russian machine the keyboard
driver of DOS, make of them what they make of a keyboard's: the key in the
BIOS's buffer, and Shift, Ctrl and Alt in its flags. Every request the
program makes for a key is answered by the BIOS as always. The mouse is
fed the same way: its packets, the buttons and how far it has moved, go
to the controller's auxiliary port, the BIOS hands them to the mouse
driver, and the mouse driver moves the pointer and answers the program.
A mouse does not say where it is, so a step takes the pointer to the top
left corner first and from there to its cell, in movements too small for
the driver to speed them up.

The driver only watches three interrupts on their way to their owners:
the one by which a program says that it is idle, which Volkov Commander
calls every time it has gone round waiting for a key; the BIOS's
keyboard interrupt, for a program that waits inside the BIOS; and the
mouse driver's, where the program asks which buttons are down. A button
of the mouse comes up when the program has asked that so many times: a
program that waits for it to come up is not idle, and asks nothing else.

When the program is idle and the BIOS's buffer is empty, it has taken
every key so far, and the driver hands over the next: so no key is ever
found waiting by a program that looks for keys while it works. Shift, Ctrl
and Alt go down and come up one at a time, with a round of the program
between. Where a scenario wants the program to find a key while it works,
as `find` does of a search, the key is written as one name with the key
that starts the work, "Enter+Home": the two go into the buffer together.
A step may also let time pass with nothing pressed, "Wait*18" for 18
ticks of the machine's timer, a second: the driver hands the program
nothing more until the BIOS's count of ticks has gone up by so many, and
the program does meanwhile what it does by itself, as after a minute
without a key, when it blanks the screen.
A step is finished when the program has gone round twice with no key left
and nothing that it will do by itself after a while, which the driver
reads in the program's own data: a time of day, which it believes for two
seconds, since the program's data holds no time that means anything
until the main loop has run. The driver then writes the screen, with
the cursor, to a port that QEMU keeps in a file, and goes on with the
next step; after the last it resets the machine, which ends QEMU. The
host starts the machine and reads the file and the disks afterwards.

The original and the translation are driven alike, and neither is changed
for it. What the driver has to know of a program, its name and where two of
its variables are, it is told in the script, which takes that from the
listing of the original's assembly: the translation keeps those variables
where the original does. It knows the program's call of the
idle interrupt from DOS's by where the call comes from: the memory of the
program that DOS has as the current one, and that has that name. None of this is measured
in time, and a busy computer makes a run slower, not different.

Where time is what the program goes by, it is the machine's, and the
machine's clock is counted in the instructions that the machine executes,
256 nanoseconds for each (QEMU's `-icount`): the timer's ticks, the time
of day and the waits of the program come when the machine has done so
much, not when the computer has had so long. Nothing of the computer's
time gets into it. Where the machine is halted, as the BIOS halts it
while a program waits there for a key, its clock goes straight on to the
next tick of the timer (`sleep=off`). Nothing in the machine waits for
the computer: its disks answer within the instruction that asks. And the
state that every scenario starts from is saved where the machine stops
itself, which the driver has it do, by the port QEMU has for that, when
it is loaded and finds no script. So every scenario begins at the same
instruction and the same time of day, and gets to each of its steps at
the same hundredth of a second, in every run and on a computer however
busy. The program's first screen comes some ten seconds past noon. A
scenario that shows the time of day still has to do so early, within the
machine's first minute: the translation and the original do not execute the
same number of instructions, and their clocks drift apart by that.

With the clock on, when a golden is derived, the driver looks at the
screen every time the program comes round, for more than a second of the
BIOS's timer ticks, and writes out both states of what blinks.

Nothing waits without a limit. A program that hangs fails its scenario
when the driver has written nothing for five minutes, with the screen it
got stuck on, and one that resets its machine fails at once. Nothing
keeps runs of the harness apart: several checkouts may run their machines
on one computer at once, and each only gets slower.

### What ran

Which code ran is taken from QEMU's log of the code it translates, which
`harness/coverage.py` matches against the program file. For the original
it is recorded with the goldens, and mapped to source lines through the
listing of the build in `build/shareware-layout/`, which has every
instruction at the address it has in the original. The translation's code is
not in the memory quite as it is in its file: the program adds the segment
it is loaded at to the words that its relocation table names, and the file
is held against the log with those words as the program has them where
it is. For the portable C it is DOS that adds it, by the table in the
header of the file, and the same holds.

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

## Using the C trees by hand

Needs the project built, and a terminal of 80 columns by 25 lines at least.

```sh
python3 harness/play.py
```

This starts the portable C, `build/portable_c/VC.EXE`, on the machine of
the tests, with its screen in the terminal and the terminal's keyboard:
FreeDOS from its floppy, the bench's files on `C:` and the program on
`D:`. `--tree c` runs the C translation, `build/c/VC.EXE`, `--original`
runs the original there, `--country ru` uses the Russian bench, and
`--display gtk` opens a window where there is a desktop. F10 leaves the
program, and the machine then switches itself off. The disks are made anew
every time, so nothing done there is kept.

## Origins

### The source

`src/asm/` holds the contents of the 4.05 source archive published by the
author:

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
| FreeDOS 1.4 Floppy Edition, for its boot floppy | The FreeDOS project's download host, <https://download.freedos.org/1.4/FD14-FloppyEdition.zip> |
| FreeDOS 1.4 Lite USB image, for the keyboard driver and its layouts, which are among its packages | <https://download.freedos.org/1.4/FD14-LiteUSB.zip> |

### What the build downloads

CMake fetches these into the build directory, as pinned in
`CMakeLists.txt`:

| What | From |
|---|---|
| JWasm 2.20, as source at a pinned commit | <https://github.com/Baron-von-Riedesel/JWasm> |
| Open Watcom 2.0, the snapshot of 1 October 2026 with tools ready to run, pinned by SHA-256 | <https://github.com/open-watcom/open-watcom-v2/releases/tag/2026-10-01-Build> |

Both are open source under the Sybase Open Watcom Public License, which is
about the tools' own code. None of that code ends up in the programs built
here: the translation is linked without Open Watcom's libraries.

## Licence

This project is released under the 3-clause BSD licence in `LICENSE`. That
covers everything written for it, including changes made to the files in
`src/asm/`.

Volkov Commander is Copyright 1991-2000 Vsevolod V. Volkov and was released
by its author under the 2-clause BSD licence in `src/asm/LICENSE.TXT`, which
continues to apply to his code.

The shareware distribution of Volkov Commander is proprietary and is not
part of this repository. The picture at the top shows it running.
