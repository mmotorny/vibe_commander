# The portable C

`src/portable_c/` began as a copy of the C translation and is where the
work goes on. This page covers the program for DOS: how it is built and
laid out, how it asks the system for what it needs through `sys.h`, the
setup program inside it, and the extender that starts it. The program
for Linux and the web page are built from the same source; see
[linux.md](linux.md) and [web.md](web.md).

In short:

- It is a 32-bit DOS program, compiled by Open Watcom's `wcc386` in the
  flat model (one address space, 32-bit pointers) and started by the
  DOS/32A [extender](#the-extender).
- It [stays in memory whole](#staying-in-the-memory-whole) while a
  command runs, so it is one program that the linker writes, where the
  C translation is two parts laid out by a tool.
- Everything it asks of the system goes through
  [one header](#the-system-in-one-place), `sys.h`, answered for DOS by
  `sys_dos.c`.
- The original's setup program [is part of it](#the-setup-program-in-it),
  as an item of the Options menu.
- Every place where it behaves otherwise than the original is declared
  in `src/portable_c/DEVIATIONS.md`. In every scenario that no deviation
  changes, it is held against the original's golden, as the C
  translation is.

## Its name

The program no longer is what the original is, so it does not carry its
name (entry 10 of `DEVIATIONS.md`):

| Where | What it says |
|---|---|
| The banner | "Vibe Commander 0.1, based on Volkov Commander 4.05" over "Copyright (C) 1991-2000 Vsevolod V. Volkov, (C) 2026 Maksym Motorny" |
| The title over the help | the banner's first line |
| The Info panel | "Vibe Commander 0.1", with "based on Volkov Commander 4.05" on a line of its own below |
| The box that asks before leaving | "the Vibe Commander" |

Its files keep their names, `VC.EXE`, `VC.INI` and the rest, and the
help file is the distribution's.

## Building it

`cmake --build build` makes two programs:

| Program | What it is |
|---|---|
| `build/portable_c/VC.EXE` | The program: compiled for the 386, for small code (`-os`), with nothing for a debugger. It is the one to ship, and the one the tests judge |
| `build/portable_c/debug/VC.EXE` | The same source, for finding faults by hand: compiled as written, with nothing moved or left out (`-od`), with source lines in its listings (`-d2`), and with `CHECKED` defined, by which the words that make a pointer of a segment and an offset are to check what they are given; nothing reads `CHECKED` yet. No test runs it, since unoptimised code takes more instructions and the bench's clock counts instructions. It is built so that it does not rot, and `portable_c-debug-layout` reads it |

Each is a linear executable written by Open Watcom's linker, with the
extender in front of it, and with its map, `VC.MAP`, and a listing of
every file next to it.

## The figures

What can be measured of the portable C, or counted in its source, and
changes with the work is written here and nowhere else: the length of
its file, what it has of memory, and how much of it only Open Watcom
accepts. Other documents point here rather than repeat a number.

`python3 tools/figures.py --write` writes the table from the build in
`build/` and from `src/portable_c/`, and is run when a piece of work
lands. `python3 tools/figures.py --check` says whether the table is
that of the build. No test holds the table, or the length of the file:
only the C translation's length is held, to three times the original's.

<!-- The figures: written by `python3 tools/figures.py --write`, not by hand. -->

| What | Figure |
|---|---|
| `VC.EXE` | 294,339 bytes, of which the extender's stub is 27,536 |
| as a multiple of the original's 65,090 bytes | 4.52 |
| more than the C translation's 133,056 bytes | 161,283 bytes |
| Its code, and what the file has of its data | 124,812 and 104,499 bytes |
| What it has of the memory above the megabyte when it is loaded: its code and its data, with what has no value | 230,060 bytes |
| The addresses in it that the extender sets when it loads it | 4,647 |
| What it has for its room on the bench of the tests, a machine of 16 MB, where the extender's memory ends at FE0000h: all that is behind its objects and its two panels, as long as each is when it holds the original's 64K, `PANEL_LEAST`, each block with the extender's 16 bytes in front | 15,236,288 bytes |
| The least of that memory with which it gets to its panels: its objects, its two panels and the least of the room, `LEAST_ROOM`, 8,000 bytes, each block with the extender's 16 bytes, worked out of the same; with less, the extender or the program says that there is not enough | 369,280 bytes |
| The words of its boxes, dialogs and tables that are pointers, four bytes where the original has a word of two; and how many of them are among those addresses, the others holding a number: as the test `portable_c-data` holds them | 640 and 554 |
| Its stack | 16,384 bytes |
| The same program compiled for finding a fault, `build/portable_c/debug/VC.EXE` | 396,319 bytes, 229,111 of them code |
| Inline assembly | 100 instructions in 13 pieces: 100 in 13 of `sys_dos.c` |
| Lines of `#pragma aux` | 12 |
| Intrinsic functions | 2: `_disable`, `_enable` |
| Pairs of `#pragma pack` | 8 |
| Calls outside the file of the system's functions that hand registers to an interrupt, or name a fixed address of the machine, where everything is to go through `sys.h`, as `tools/calls.py` counts them | none |

<!-- The end of the figures. -->

The rows of the file are what the harness's reader of 32-bit programs,
`harness/le.py`, finds in it, and what the test `portable_c-layout`
works out of its layout. What the test `portable_c-host` counts of what
only Open Watcom reads, by kind, is in `harness/hostcheck.txt`, and
what each kind waited for in `DIARY.md`, "Hold the source of the
portable C against gcc: the test portable_c-host".

## Staying in the memory whole

The original is in two parts. One stays in memory while a command runs,
with the stack and a copy of the setup and of what the panels show. The
other, the code and the main data, is given back to DOS for the command
and made again afterwards: the main data from the copy, and the code,
if Memory allocation is switched on in the Options menu, by loading it
from the program's file. The C translation is laid out for that by
`tools/mkexe.c`, and most of what [rests on Open
Watcom](translation.md#what-rests-on-open-watcom) there is holding the
two parts together.

The portable C stays in memory whole, as one program (entry 1 of
`DEVIATIONS.md`). What stayed because a user or another program sees
it is in `exec.c`: commands run through COMMAND.COM, with their texts,
what the program does first, and what it does when it leaves. DOS's
critical error, Ctrl-Break and Ctrl-C, the interrupts taken and given
back, and where DOS goes on when it ends the program by another's hand
are in `sys_dos.c` (see [The system in one place](#the-system-in-one-place)).

### The layout

The linker writes `VC.EXE`, a linear executable with the extender in
front. The extender loads it above the first megabyte, sets every word
that holds an address, and enters it at `Start` of `sys_dos.c`, on the
stack that the file's header names. It is two objects, code and data.
The linker's command (`add_portable_c` of `CMakeLists.txt`, with the
reason for each) puts the data in this order:

1. **The stack**, `stack_dos.c`. The extender packs the data right
   behind the code, and the bench's machine is many times slower at
   writing to a memory page with code that it has run. What shares that
   page is the low end of the stack, which a program reaches only after
   using three quarters of it.
2. **The constants** of the code and the tables (the files whose names
   end in `_t`), and behind them what has a value, which is little more
   than the tables that the program writes to.
3. **The main data**, `md`, a variable of `main.c`, which the file holds
   as zeros.
4. **What has no value**, which the file does not hold: the files'
   variables, those of `sys_dos.c` last.

### Pointers where there were segments

A segment is no longer a number in the program. A panel, the text of
the viewer or the editor, a memory block and a table are pointers, and
a box, a dialog, a text or a row of buttons is passed by a pointer.

What stays a 16-bit number is an offset that is the original's by its
layout: a file's place in a panel's block, and a place in the main data
where the original leaves it in a register for something else to read.
The words of `src/portable_c/vc.h` that make pointers of these ("The
words of the addresses" there) count from where their block or
structure begins, and assume nothing about where the linker has put it.

A word of a table that names something (a line of a box, the box of a
dialog, the buttons of a field) is a pointer already, since C has it as
one. In the original it is a two-byte offset. Here it is four bytes, an
address that the extender sets at load time, so a box or a dialog is
two bytes longer for each. The test `portable_c-data` therefore compares
the tables field by field; [the figures](#the-figures) have how many
such words there are.

### Memory from the extender

The memory for the panels, the viewer and the editor is the extender's,
above the megabyte, as much as the machine has. The program asks for it
as the original asks DOS, for all there is, and `sys_dos.c` asks the
extender (DPMI 0500h to 0503h) for:

- **a block for each panel**, at least the original's 64K, kept for as
  long as the program runs. It grows and shrinks as its directory needs
  while the directory is read, and may move then.
- **one block for the room** that the viewer, the editor, a copy and the
  file search use: all that the extender has behind the panels. It gives
  way while a panel grows, and is taken again behind it.

The original instead has one block that it cuts up and moves its panels
in. [The figures](#the-figures) have how large the room is on the bench.

The extender keeps 16 bytes of its own in front of each block, and ends
the program at its next memory call if something has written into them.
The tests look at them when a scenario ends, so a write just outside a
block fails the scenario.

What follows from this (entries 1 and 13 of `DEVIATIONS.md` have the
details):

- A command that the program runs has DOS's memory below the megabyte,
  all but the extender's part, and finds none above it, since the
  extender holds it.
- A program of the same extender that it runs, including a second copy
  of itself, gets the room that it has given back.
- The editor has room for a larger file than the original's has.

`VC.EXE` is longer than the C translation's: it is compiled for 32
bits, the main data is in the file, every word that names something has
an entry in the extender's relocation table, and the extender is in
front. The part that stayed in memory, with its copies and its second
copy of the file that calls DOS, is gone. [The figures](#the-figures)
have the length.

## The system in one place

What the program asks of the system it asks in one place, and the rest
of it does not know which system that is. In the C translation the
interrupt instruction is in one file, but every file that wants
something of DOS fills a structure of registers, with the service
number, and hands it to `Intr21` or `Int21d`, and the same for the BIOS:
348 calls in 25 files, each of which knows DOS.

In the portable C that place is `src/portable_c/sys.h`: 99 functions
named for what is asked, with no register, no service number, no
interrupt and no word of Open Watcom in them (`portable_c-host` has the
file clean). Each implementation is a set of files beside the common
code, picked by the end of their names (the lists of
`portable_c_sources` in `CMakeLists.txt`):

| Implementation | Files |
|---|---|
| DOS | `sys_dos.c`, the functions for a 32-bit program under the extender, and the one file that knows how DOS is asked; `stack_dos.c`, the stack |
| Linux | `sys_posix.c`, with the files that emulate DOS beside it, each ending in `_posix.c`, and their header `emulate_posix.h`: `machine_posix.c` is the part with nothing of a platform in it, and `keyboard_posix.c` turns keys into the BIOS's words. See [linux.md](linux.md) |
| Web | `sys_web.c`, sharing the emulation of Linux. See [web.md](web.md) |

The common code is every file whose name ends in no implementation's
name. No file but `sys_dos.c` has a structure of registers, a service
number, an interrupt that it calls, a line of assembly or a register in
a `#pragma aux`, which `portable_c-host` holds. `python3 tools/calls.py`
counts calls that hand registers to an interrupt: none in the portable
C and all of them in the C translation, which [the
figures](#the-figures) say too.

### The functions by group

| What | Functions |
|---|---|
| Files: made, opened, closed, read, written, a place in one and a step from where one is, deleted, renamed, their attributes and their date and time, what a handle is, a full path | 18 |
| Files by a file control block: a mask deleted at one call, a label | 2 |
| Directories: what is in one, made, removed, the current one | 8 |
| Drives: the current one, the room of one, a drive that shares its unit, a network, a disk that stays in, a CD-ROM | 8 |
| Memory: a block taken, given back, made another size, where the system looks for one, and the program's own block made as long as the program | 6 |
| The time, the country and its capitals, the version, Ctrl-C, idling | 8 |
| Other programs: one run, with what it is told behind its name; whether a file is one, and a file handed to what opens its kind; this one ended, on an old DOS too; what this one was told when it was started, and its environment | 8 |
| What the system asks the program, and the keys that stop one: who is asked about a device that fails, and who has Ctrl-Break, noted, taken and given back; Ctrl-C taken; the machine let interrupt the program again; what the system is to put back when it has ended the program, said, taken and given back | 11 |
| The console: a character and a text where the cursor is | 2 |
| The keyboard: a key that waits, a key taken, Shift and the like, Ins | 4 |
| The screen: its mode and page, the cursor, a window scrolled, a cell read and written, a character typed, whether the display is an EGA and how many rows its font gives it, where the cells are and the cells themselves | 15 |
| The mouse: whether there is one, its pointer shown, placed and shaped, where it is | 8 |
| The timer | 1 |

`CLAUDE.md`, "Moving a file to the system's functions", says how a
function is added.

### DOS's shapes

The functions take and answer in DOS's and the BIOS's terms: a drive by
its letter, a path with backslashes, DOS's attributes, dates, times and
error numbers, a key with its scan code, a cell with its colour, memory
by the segment. The program uses those shapes everywhere and the screens
show them; another system has to answer in them.

One function is one thing asked. Where the program asks several things
in a row, the row is still in the program, in the original's order,
since what DOS is asked and in which order is behaviour that the
scenarios see. Three things that the original kept in registers are in
the functions by name:

- **How a call is asked.** The original calls DOS through its routine
  `Intr21` where a drive may not be ready. That lets the program's box
  for it come up, makes the error 53h of a call the user gave up on,
  and asks DOS for its extended error after a failure. Elsewhere it
  calls DOS directly. The functions for files, directories and drives
  are of the first kind, and the five that the original also calls
  directly have functions of their own, with `Bare` in their names.
- **The carry flag before a call.** DOS leaves it as it finds it at two
  of its calls, and `Intr21` takes a set flag for an error. Those two
  functions take the flag as an argument, `flagged`.
- **What a call leaves.** Every answer has what DOS left in AX, also
  where nothing failed, since some callers go on with it; anything else
  a call answers in is a named member.

What the functions do not take is whatever a register happened to hold
where the call does not read it, such as the low byte of AX at most DOS
calls. The original hands DOS and the BIOS whatever the code before left
there; the portable C hands them 0. No screen shows that, and entry 6 of
`DEVIATIONS.md` declares it, with every place.

### What the system enters

DOS's critical error, Ctrl-Break and Ctrl-C are taken by functions of
`sys.h`, and the first two given back by one, in the original's order.
They are real-mode interrupts, which the program takes through the
extender: the extender comes over from real mode and enters a routine of
`sys_dos.c`, on a stack of its own.

- **The critical error.** The box with Retry and Abort is the program's
  own function, `FtErr1` of `main.c`, which it hands over when it takes
  the critical error. The entry, `DeviceEntry`, switches to the
  program's stack; what it calls there reads DOS's registers and the
  device's header, calls the program's function in the usual way with
  what DOS says is wrong and the device's name where it is not a drive,
  and hands DOS the program's answer.
- **Ctrl-Break and Ctrl-C** arrive at a routine that does nothing.

### When DOS ends the program

The fourth thing DOS enters is where it goes on after ending the
program by another program's hand. Nothing of a 32-bit program can run
then: the extender sees the call in real mode and takes itself out
first, with the memory above the megabyte that the program is in. So
the program is not asked; `sys.h` has this as a value, not as a function
to call.

The program says beforehand what it has changed of DOS that DOS does
not put back by itself. That is one thing, whether DOS checks for Ctrl-C
at every call: `ProgramEndedBreakCheck`, which `SetBrk` of `exec.c`
calls whenever it has noted it. While the program has taken its end,
the PSP has DOS go on at eight real-mode instructions that `sys_dos.c`
keeps as bytes, `Ended_T`, in a block below the megabyte. They set the
Ctrl-C check as the program said, leave the registers that the original
leaves for the program that started this one, and go on where the PSP
said before. The scenario `ended` shows those registers. One thing is
left otherwise, by the extender: Ctrl-Break goes where it went before
the program started, where the original leaves it pointing into free
memory.

### Starting, and calls in real mode

The extender enters the program at `Start` of `sys_dos.c`, which is not
in `sys.h`. It settles what compiled code takes for granted, asks the
extender where the PSP and the environment are and for a block of 512
bytes below the megabyte, prepares the memory the program is given, and
calls `Main` of `main.c`, which is the program. What the program does
first it asks through `sys.h` like everything else: the DOS version, and
that it keep no more memory than it is. The PSP is read and written in
`sys_dos.c` alone: the command tail and environment, for the functions
that answer with them, and the address where DOS goes on when the
program has ended. `sys_dos.c` also makes the parameter block that DOS
is given for a program to run.

DOS, the BIOS and the mouse driver are of real mode. The program calls
them by the same software interrupts, and the extender carries each call
there and back, translating a DOS call that takes the address of a name
or a buffer where it knows the call. Thirteen functions it does not
carry through as `sys.h` has them; `sys_dos.c` makes those in real mode,
through the extender's service for that, with what DOS reads and writes
in the block below the megabyte. The top of the file says which they
are and how each was found.

The command loop of `exec.c` runs a command one way, through a
COMMAND.COM of its own. Quick execute commands, which handed a command
to the COMMAND.COM that the program was started from, the memory map
and the watch on DOS are gone (`DEVIATIONS.md`, entries 7 and 14).

## The setup program in it

The original is two programs. The second, `VCSETUP.COM`, sets most of
the setup: every colour, and the options that the main program's menus
lack. It exists only as assembly, `src/asm/VCSETUP.ASM` with two files of
routines that are mostly copies of the main program's.

In the portable C it is part of the one program (entry 8 of
`DEVIATIONS.md`): the last item of the Options menu, " More setup...",
behind which the setup program is on the screen as the original's is
when run from the command line, over what DOS had shown. It is
translated as it is: it reads the setup file and edits that, and when
the setup is left the program takes what was saved, as the original's
running copy is handed it by a DOS call. All nine items of its menu do
what the original's do, and F1 is the help.

Its files begin with `st`, and its routines and tables have `St` in
front of their names in the assembly, many of which are the main
program's names too:

| Files | What they have |
|---|---|
| `stmain.c`, `stmain_t.c` | entering and leaving the setup, its file, its menu, and the setup the program has without a file |
| `sthelp.c`, `sthelp_t.c` | its help: the seven pages the help file has for the setup, read when the setup is entered, and the page for where the user is in a window, for F1 |
| `stcolor.c`, `stcolor_t.c` | the three palettes, Black & White, Color and Laptop: the list of a palette's 39 colours with the grid of 256 beside it, and the picture in which a colour is chosen, one of four screens of the program painted in the palette |
| `stopts.c`, `stopts_t.c` | the four option dialogs: Configuration; Screen and Mouse; Panel options, with the tree's delay typed there and the box for an invalid one; and Country info, whose lines are written as the country has them before the box is shown |

What both programs have is there once. The boxes, dialogs, bars and the
mouse are the main program's. The mouse at the menu is `F32_70` of
`usermenu.c`, the main program's routine for a list in a box, which the
setup program has word for word under another name. What a colour
becomes in a shadow, which a picture asks for a shadow other than the
one set, is `ShadCol` of `screen.c`, for both. Three routines that the
setup program has otherwise go by a variable that says the setup is
open, `StOpen`: `PutKey0`, which then has the setup's key bars;
`Dialog`; and `Input`, where F1 is then the setup's own help. What the
setup program keeps in a segment of its own is in the main data here,
in buffers that the main program makes anew after a command anyway.

The first two dialogs lack four settings that the original's setup
program has, Memory allocation, Quick execute commands, TSR manager and
Screen output, which are of what the portable C no longer has. What
stood below them moves up, and their bytes of the setup file are kept
as they are found (`DEVIATIONS.md`, entries 1, 5, 7 and 14).

`DIARY.md`, "Write down the plan for the setup program, with the
scripts of its trial", has the plan, with what each step brought and
what is declared for it.

## The extender

A 32-bit program does not run on DOS by itself. An extender puts the
processor into protected mode, loads the program above the first
megabyte, and passes what the program asks of DOS and the BIOS down to
them. The portable C is started by DOS/32A:

**This product uses DOS/32 Advanced DOS Extender technology.**

The extender is the first part of the program's file, its stub (the
figures have its length), which the linker copies from
`binw/dos32a.exe` of the Open Watcom download. It is in no file of this
repository; its licence is, [`LICENSE.DOS32A`](../LICENSE.DOS32A).

One bit of it is changed. As it comes, DOS/32A prints two lines with
its name and copyright when it starts, onto what is the program's first
screen. `tools/stub.c` clears the bit of its settings that does that,
in the file that the linker wrote, and says where the bit is and how it
was found.

Open Watcom has other extenders. CauseWay, which is free too, does not
run on the bench because of a fault of QEMU's (`DIARY.md`, "Write down
what a trial of a 32-bit program on the bench showed"). DOS/4GW and
PMODE/W are not free, and are neither linked nor run here.

## What rests on Open Watcom

These are the places where the portable C is right only because of Open
Watcom, which the work is to shrink. The C translation's list is in
[translation.md](translation.md#what-rests-on-open-watcom); of its
seventeen places that cannot be seen in the code, none is left here,
since they are of a 16-bit program in two parts. How far the source is
from what another compiler accepts is counted by `portable_c-host`:
gcc reads every file for a 64-bit machine, where a pointer does not fit
a 32-bit number, and each place it refuses, and each that only Open
Watcom reads, is counted by kind and by file in
`harness/hostcheck.txt`. The C translation's words for which segment a
thing is in, `__far`, `STACK` and `CODE`, are in no file of the
portable C.

### What the code is written in

| What | Where |
|---|---|
| `#pragma aux` (the figures have how many lines) | inline assembly in `sys_dos.c`, and `Arrived` of `sys_dos.c`, a function that assembly calls, where the extender enters the program for DOS's critical error, with its arguments in the registers the assembly has them in. None of them names anything for the linker |
| `#pragma pack(1)` (the figures have how many pairs) | the structures that are the original's |
| `__declspec(naked)` functions, which are their `_asm` and nothing else | `DeviceEntry` and `NothingEntry` of `sys_dos.c`, which the extender enters where DOS's critical error, Ctrl-Break and Ctrl-C arrive |
| Intrinsic functions, `_disable` and `_enable` | `sys_dos.c` |
| `#pragma data_seg`, which the C translation does not have, for the main data and the stack | written by `SEGMENT_BEGIN` and `SEGMENT_END` of `compiler.h`, and nothing to another compiler |
| `__declspec(aborts)`, for a function that does not return | written as `NO_RETURN` of `compiler.h`, and nothing to another compiler |

`src/portable_c/compiler.h` has the words that each compiler says in
its own way, and `vc.h` includes it. There is no `__interrupt` and no
`__loadds` function, and no routine for 32-bit arithmetic, which the
32-bit compiler does by itself. The C translation's rule that a file of
code has no variables and no strings went with its two parts, and
nothing checks the object files of the portable C.

### What cannot be seen in the code

What this program rests on is where the linker puts things:

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The linker's `order` directive: the classes of the segments in the order its command names, each class linked whole | `CMakeLists.txt`, the linker's command | `portable_c-layout`, in each of the rows below |
| 2 | `#pragma data_seg` makes a segment of the name and class it names: the main data is of a class of its own, `MAIN`, and the stack of the class `STACK`, which the linker takes for the stack, naming its end in the file's header | `SEGMENT_BEGIN` of `src/portable_c/compiler.h`, in `main.c` and `stack_dos.c` | `portable_c-layout`: the stack is the segment that ends at the header's stack top, and is in the file as zeros, by which the scenarios' test tells how much of it a scenario has left |
| 3 | What has no value is of the class `BSS`, the last, and is not in the file; everything in front of it is | `CMakeLists.txt` | `portable_c-layout` |
| 4 | The compiler puts nothing between the fields of a box or a dialog, nor between two objects of a file | `src/portable_c/tables.h`, by `#pragma pack(1)` | `portable_c-data`: an object is as long as its declaration makes it |
| 5 | Compiled code takes ES to be on the data and the direction flag to be clear, and the extender enters the program with ES on the PSP: the first thing the program does settles both | `Entered`, in `Start` of `src/portable_c/sys_dos.c` | every scenario |
| 6 | A `__declspec(naked)` function is its `_asm` and nothing else, and `_asm` reads a variable through CS: where the extender enters the program from real mode, with DS and ES anywhere | `DeviceEntry` of `src/portable_c/sys_dos.c` | the scenarios of the critical error |
| 7 | The compiler keeps a cut that a cast says, but for one: a signed byte made a 16-bit unsigned number, `(u16)(signed char)byte`, which it extends with its sign to all 32 bits wherever the number goes on into 32 bits. The tree writes neither that nor the form with a `short` between, which the compiler keeps: where the original has `CBW`, the byte is taken as it is, or with its sign into an `int` | `src/portable_c/vc.h`, "A byte with a sign made a word" | `portable_c-cut`: the form is nowhere in the tree, and the compiler makes of `harness/cutcheck.c` the instructions written there, the wrong ones for the lost form and the right ones for every other narrowing cast |

### Inline assembly

All of it is in `sys_dos.c`, in thirteen pieces: eleven `#pragma aux`
with their instructions, put in place where they are called, and the
two naked functions. [The figures](#the-figures) have the total number
of instructions.

| Piece | Instructions | Why it is not C |
|---|---|---|
| `Dos` | 17: the 32-bit registers loaded from a structure, `INT 21h`, four of them stored back, and the carry flag | A software interrupt is not a function call. The extender reads all 32 bits of a register where it translates a call |
| `Dpmi` | 19: the same around `INT 31h`, storing six registers back | The extender's own services, by which a call is made in real mode, an interrupt is taken and a block below the megabyte is asked for |
| `Video` | 7 | The BIOS for the screen, `INT 10h`; some BIOSes lose EBP |
| `Keyboard` | 5 | The BIOS for the keyboard, `INT 16h`, answers in the zero flag |
| `Multiplex` | 3 | `INT 2Fh`, by which the CD-ROM driver is asked, answers in two registers |
| `DosIdle` | 1: `INT 28h` | The program says that it is idle |
| `MouseDriver` | 1: `INT 33h` | The mouse driver |
| `MouseState` | 4 | The same, answering in three registers |
| `StackNow` | 1 | The stack pointer, for where the critical error may use the program's stack |
| `Selectors` | 3 | CS and DS, by which the routine entered from real mode loads its segments |
| `Entered` | 5 | ES put on the data and the direction flag cleared, before any compiled instruction relies on either |
| `DeviceEntry` | 33 | Where the extender enters the program for DOS's critical error, with the registers of real mode, DS and ES anywhere and a stack of the extender's: no compiled instruction can run before the segments and the stack are the program's |
| `NothingEntry` | 1: `IRETD` | Where it enters the program for Ctrl-Break and Ctrl-C: an interrupt's return is not a function's |

Of the C translation's pieces, those of `src/c/dos.c` have their
counterparts here, with 32-bit registers, except `VideoBuffer` and
`DesqviewPause`, which are gone (`DEVIATIONS.md`, entries 2 and 3). Those
of `src/c/resident.c` are gone with what they were for, the entries of
INT 21h and INT 27h and the way through INT 2Eh, or are the extender's
to do: where DOS goes on when it has ended the program is no code of
the program's here (see [When DOS ends the
program](#when-dos-ends-the-program)). Of the three instructions that
the C translation's intrinsic functions make, `IN` is gone too: the
program reads no port.

## What is left of the assembly

A function that returns two things, where the assembly has two
registers or a register and the carry flag, returns a structure with
named members, where the C translation returns the halves of one 32-bit
number: `GetMous` the buttons and the place of the mouse, `OpenFil` what
DOS answered and whether that is an error, a command of the keys the
place in the loop where the program goes on. A call that took such a
number apart would not compile.

The `goto` statements that the translation kept from the assembly are
gone where that was easy: in a function that only jumps forward, or
whose backward jumps make a loop with one way in, they became `if`,
loops, `break`, `continue` and `return`. A label that went leaves the
assembly's name in a comment where it stood, so the code can still be
held against `src/c/` and `src/asm/`. Nine functions keep theirs, where
loops are entered in the middle or paths cross, until one is rewritten
for another reason:

| File | Functions |
|---|---|
| `copy.c` | `C20_00` |
| `dirfunc.c` | `FncSel` |
| `main.c` | `Loop` |
| `menu.c` | `F39_00` |
| `usermenu.c` | `F32_00`, `FileLst`, `Run_00` |
| `view.c` | `V00_10`, `V82_00` |
