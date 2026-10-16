# The portable C

`src/portable_c/` began as a copy of the C translation, and is where
the work goes on. This page says how it is built and laid out for DOS,
how it asks the system for what it needs, and where it has left the C
translation. The program for Linux and the web page are built from the
same source: [linux.md](linux.md) and [web.md](web.md).

Where it behaves otherwise than the original, that is declared in
`src/portable_c/DEVIATIONS.md`. Its first entry is that [it stays in
the memory whole](#staying-in-the-memory-whole) while a command runs,
and so is one program, which the linker writes, where the C translation
is two parts laid out by a tool. Then come what the program did for
the PC and for the programs around it, and what a
register happened to hold where DOS or the BIOS is called, which [the
functions of the system](#the-system-in-one-place) do not hand over; no
scenario shows either. Then what a user of DOS saw of the
program, a command or an option that only
made sense there. And the original's second program, its setup program,
[is a part of this one](#the-setup-program-in-it). In every
scenario that a deviation does not change the portable C is held
against the original's golden, as the C translation is.

With that much gone from it and the setup in it, the program no longer
is what the original is, and does not carry its name: it calls itself
Vibe Commander 0.1, and says what it is based on wherever the original
named its version. Its banner is "Vibe Commander 0.1, based on Volkov
Commander 4.05" over "Copyright (C) 1991-2000 Vsevolod V. Volkov, (C)
2026 Maksym Motorny", two lines as the original's is; the first of them
is the title over its help; the title of its Info panel, which has no
room for all of that, is "Vibe Commander 0.1", with "based on Volkov
Commander 4.05" on a line of its own below it; and the box that asks
before the program is left calls it the Vibe Commander. That is entry
10 of `DEVIATIONS.md`. Its files keep their names, `VC.EXE` and
`VC.INI` and the rest, and the help file is the distribution's.

It is a program of 32 bits, still for DOS, where the original and the C
translation are programs of 16: that was the first step of the road
that takes it off DOS, which `CLAUDE.md` has under "The portable C".
Open Watcom's compiler for 32 bits, `wcc386`, compiles it in the flat
model, in which the memory is one address after another and a pointer
is an offset of 32 bits, and [the extender](#the-extender) DOS/32A
starts it. `cmake --build build` makes two programs of it:

| Program | What it is |
|---|---|
| `build/portable_c/VC.EXE` | The program: compiled for the 386, for small code (`-os`), with nothing for a debugger. It is the one that is to be shipped, and the one that the tests judge |
| `build/portable_c/debug/VC.EXE` | The same source for finding a fault, to be run by hand: compiled as it is written, with nothing moved or left out (`-od`), with the lines of the source between the instructions of its listings (`-d2`), and with `CHECKED` defined, by which the words that make a pointer of a segment and an offset are to look at what they are handed. Nothing reads `CHECKED` yet. No test runs this program, since unoptimised code takes more instructions and the machine's clock is counted in those; it is built so that it does not rot, and the test `portable_c-debug-layout` reads it |

Each is a linear executable that Open Watcom's linker writes, with its
map, `VC.MAP`, and a listing of every file next to it, and with the
extender in front of it. [Staying in the memory
whole](#staying-in-the-memory-whole) says how it is laid out.

A function that hands back two things, where the assembly has two
registers or a register and the carry flag, returns a structure with
members named for what they are, where the C translation returns the
halves of one number of 32 bits: `GetMous` the buttons and the place of
the mouse, `OpenFil` what DOS answered and whether that is an error,
a command of the keys the place of the loop where the program goes on.
A call that took such a number apart would not compile.

The lines that say something of registers still, and why:

| What | Where |
|---|---|
| Inline assembly: the instruction and the registers it takes and leaves | eleven pieces of `src/portable_c/sys_dos.c` |
| A function that assembly calls, where the extender enters the program for DOS's critical error, with its arguments in the registers that the assembly has them in | `Arrived` of `sys_dos.c` |

That a function does not return is said by a word in front of it,
`NO_RETURN`, which is Open Watcom's `__declspec(aborts)` and nothing to
another compiler: `src/portable_c/compiler.h` has the words that each
compiler says in its own way, and `vc.h` includes it. [The
figures](#the-figures) have how many lines of `#pragma aux` there are
in the portable C, where the C translation has 863; none of them gives
anything a name.

The jumps that the translation kept from the assembly, `goto`, are gone
where that was easy: in a function that only jumps forward, or whose
jumps back make a loop with one way in, they became `if`, loops,
`break`, `continue` and `return`. A label that went leaves the
assembly's name in a comment where it stood, so that the code can still
be held against `src/c/` and `src/asm/`. Nine functions keep theirs,
where loops are entered in the middle or paths cross, until one is
rewritten for another reason: `C20_00` of `copy.c`, `FncSel` of
`dirfunc.c`, `Loop` of `main.c`, `F39_00` of `menu.c`, `F32_00`,
`FileLst` and `Run_00` of `usermenu.c`, and `V00_10` and `V82_00` of
`view.c`.

## The figures

What can be measured of the portable C, or counted in its source, and
changes with every step of the work, is written here and nowhere else:
how long its file is, what it has of the memory, how much room the
layout that it rests on has left, and how much of it only Open Watcom
takes. No test holds the length of its
file to anything: only the C translation's is held, to three times the
original's. `python3 tools/figures.py --write` puts the
table here, of the build in `build/` and of `src/portable_c/`, and is
run when a piece of work lands; `python3 tools/figures.py --check` says
whether the table is that of the build. Other documents point here
rather than repeat a number. No test holds the table.

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

The rows of the file are what the harness's reader of a flat program
of 32 bits finds in it, `harness/le.py`, and how much room the layout
that it rests on has left, as the test `portable_c-layout` works it
out. What the test `portable_c-host` counts of what only Open Watcom
reads, by kinds, is in its own table, in `DIARY.md`, "Hold the source of
the portable C against gcc: the test portable_c-host".

## Staying in the memory whole

The original is two parts. One stays in the memory while a command
runs, with the stack and a copy of the setup and of what the panels
show; the other, the code and the main data, is given back to DOS for
the command, and made again afterwards: the main data of the copy, and
the code, if the Options menu has Memory allocation switched on, by
loading it from the program's file. The C translation is laid out for
that, by `tools/mkexe.c`, and most of what [rests on Open
Watcom](translation.md#what-rests-on-open-watcom) there is the two parts held
together.

The portable C stays in the memory whole, and is one program: that was
the first of the features that only made sense on DOS to go from this
tree. `src/portable_c/DEVIATIONS.md` has what the
original does, what the portable C does, why, and the scenarios that
show it.

What stayed, because a user or another program sees it, is in `exec.c`:
the commands run through COMMAND.COM, with
their texts, what the program does first, and what it does when it
leaves. DOS's critical error, Ctrl-Break and Ctrl-C, the interrupts
taken and given back, and where DOS goes on when it ends the program by
another's hand, are in the file of [the system's
functions](#the-system-in-one-place). Where one of these could not stay
as the original has it, `DEVIATIONS.md` says what differs.

How the program is laid out. The linker writes `VC.EXE`, a linear
executable with [the extender](#the-extender) in front of it. The
extender loads it above the first megabyte, sets every word of it that
holds an address, and enters it at `Start` of `sys_dos.c`, on the stack
that the file's header names. It is two objects, the code and the data,
and the linker's command says in which order the data lies
(`add_portable_c` of `CMakeLists.txt`, with the reason for each thing's
place):

- The stack, `stack_dos.c`, first. The extender packs the data right behind
  the code, and the machine of the tests is many times slower at a
  write to a page of the memory that has code which it has run: what
  shares that page is the low end of the stack, which a program writes
  to only when it has used three quarters of its stack.
- The constants of the code and the tables, the files whose names end
  in `_t`, and behind them what has a value, which is little more than
  the tables that the program writes to.
- The main data, `md`, a variable of `main.c`, which the file has as
  zeros.
- What has no value, which the file does not hold: the variables of the
  files, which a file may have here, those of `sys_dos.c` last.

A segment is no number in the program any more: a panel, the text of
the viewer or of the editor, a block of the memory and a table are
pointers, and a box, a dialog, a text or a line of buttons is handed on
by a pointer to it. What stays a number of 16 bits is an offset that
is the original's by its layout, a file's in a panel's block, and a
place of the main data where the original leaves it in a register for
something else to read: the words of `src/portable_c/vc.h` that make
pointers of those ("The words of the addresses" there) count from where
their block or structure begins, and take nothing for granted of where
the linker has put it.

One kind of word is a pointer already, since C has it as one: a word of
a table that names something, a line of a box, the box of a dialog, the
buttons of a field. In the original that is a word of two bytes, the
offset of what it names. Here it is four bytes, an address that the
extender sets when it loads the program, and a box or a dialog is
longer than the original's by two bytes for each; the code reads such
a word as the pointer that it is. So the tables are held against the original's field by field,
by the test `portable_c-data`: [the figures](#the-figures) have how
many such words there are.

The memory of the panels, the viewer and the editor is not DOS's but
the extender's, above the megabyte, as much as the machine has. The
program asks for it as the original asks DOS, all that there is, and
`sys_dos.c` asks the extender, DPMI 0500h to 0503h: a block for each
panel, the original's 64K at the least, which is kept for as long as the
program runs and grows and shrinks as its directory needs while the
directory is read, and may be moved then, and one for the room that the
viewer, the editor, a copy and the file search take, all that the
extender has behind the panels, where the original has one block that
it cuts up and moves its panels in, and which gives way while a panel
grows and is taken again behind it. [The figures](#the-figures) have how long the room is on the bench
of the tests. The extender keeps 16 bytes of its own in front of each
block and ends the program at its next call for memory where something
has written into them; the tests look at them when a scenario ends,
and a write just outside a block fails the scenario. So a command that
the program runs has DOS's memory below the megabyte, all but the
extender's part, and finds none above it, which the extender holds; a
program of the same extender that it runs, a second copy of it among
them, gets the room that it has given back; and the editor has room for
a larger file than the original's has. Entries 1 and 13 of
`DEVIATIONS.md` say what follows from each.

`VC.EXE` of the portable C is longer than the C translation's: it is
compiled for 32 bits, the main data is in the file, every word that
names something has an entry in the table that the extender goes by,
and the extender is in front of it; the part that stayed in the memory,
with its copies and its second copy of the file that calls DOS, is
gone. [The figures](#the-figures) have how long it is.

How far the source is from what another compiler takes is counted, by
the test `portable_c-host` (see [testing.md](testing.md)): gcc reads every
file for a machine of 64 bits, where a pointer does not fit a number of
32 bits, and each place that it refuses is counted, with each that only
Open Watcom reads, by kind and by file. `harness/hostcheck.txt` has the
counts as they stand, and `DIARY.md`, "Hold the source of the portable C
against gcc: the test portable_c-host", what each kind waited for. The
words by which the C translation says what segment a thing is in,
`__far`, `STACK` and `CODE`, are in no file of the portable C.

What rests on Open Watcom, for the portable C. Of what the code is
written in, the first table of [What rests on Open
Watcom](translation.md#what-rests-on-open-watcom): `#pragma aux` and `#pragma
pack(1)`, each as often as [the figures](#the-figures) say; two
`__declspec(naked)` functions, `DeviceEntry` and `NothingEntry` of
`sys_dos.c`, which the extender enters where DOS's critical error,
Ctrl-Break and Ctrl-C arrive; two intrinsic functions, `_disable` and
`_enable`; and, which the C translation has not, `#pragma data_seg`,
for the main data and for the stack, which `SEGMENT_BEGIN` and
`SEGMENT_END` of `compiler.h` write, and which are nothing to another
compiler. It has no `__interrupt` and no
`__loadds` function, and no routine for arithmetic on 32 bits, which
the compiler for 32 bits does by itself. The test `portable_c-host`
counts these by their kinds, and `DIARY.md`, "Hold the source of the
portable C against gcc: the test portable_c-host", has its table. Of
what cannot be seen in the code, none of the seventeen places of the
second table there is left: they are of a program of 16 bits in two
parts. What this program rests on is where the linker puts things:

| | What is relied on | Where it is done | What holds it |
|---|---|---|---|
| 1 | The linker's `order` directive: the classes of the segments in the order that its command names, each class linked whole | `CMakeLists.txt`, the linker's command | `portable_c-layout`, in each of the rows below |
| 2 | `#pragma data_seg` makes a segment of the name and of the class that it names: the main data is of a class of its own, `MAIN`, and the stack of the class `STACK`, which the linker takes for the stack, naming its end in the file's header | `SEGMENT_BEGIN` of `src/portable_c/compiler.h`, in `main.c` and `stack_dos.c` | `portable_c-layout`: the stack is the segment that ends where the header has the stack's top, and is in the file as zeros, by which the scenarios' test tells how much of it a scenario has left |
| 3 | What has no value is of the class `BSS`, the last, and is not in the file; everything in front of it is | `CMakeLists.txt` | `portable_c-layout` |
| 4 | The compiler puts nothing between the fields of a box or a dialog, nor between two objects of a file | `src/portable_c/tables.h`, by `#pragma pack(1)` | `portable_c-data`: an object is as long as its declaration makes it |
| 5 | Compiled code takes ES to be on the data and the direction flag to be clear, and the extender enters the program with ES on the PSP: the first thing that the program does settles both | `Entered`, in `Start` of `src/portable_c/sys_dos.c` | every scenario |
| 6 | A `__declspec(naked)` function is its `_asm` and nothing else, and `_asm` reads a variable through CS: where the extender enters the program from real mode, with DS and ES anywhere | `DeviceEntry` of `src/portable_c/sys_dos.c` | the scenarios of the critical error |
| 7 | The compiler keeps a cut that a cast says. It loses one: of a byte with a sign made a number of 16 bits without one, `(u16)(signed char)byte`, which it extends with its sign to all 32 bits wherever the number goes on into 32 bits. The tree writes neither that nor the form with a `short` between, which it keeps: where the original has `CBW`, the byte is taken as it is, or with its sign into an int | `src/portable_c/vc.h`, "A byte with a sign made a word" | `portable_c-cut`: the tree has the form nowhere, and the compiler makes of `harness/cutcheck.c` the instructions that are written there, the wrong ones of the form that it loses and the right ones of every cast that narrows |

The rule of the C translation that a file of code may have no variable
and no string is gone with the two parts: nothing checks the object
files of the portable C, and nothing has to.

Inline assembly, for the portable C, is all in `sys_dos.c`, thirteen
pieces: eleven that are a `#pragma aux` with their instructions, put in
place where they are called, and the two functions that are their
`_asm` and nothing else. [The figures](#the-figures) have how many
instructions that is.

| Piece | Instructions | Why it is not C |
|---|---|---|
| `Dos` | 17: the registers of 32 bits loaded from a structure, `INT 21h`, four of them stored back, and the carry flag | A software interrupt is no function call. The extender reads all 32 bits of a register where it translates a call |
| `Dpmi` | 17: the same around `INT 31h` | The extender's own services, by which a call is made in real mode, an interrupt is taken and a block below the megabyte is asked for |
| `Video` | 7 | The BIOS for the screen, `INT 10h`; some lose EBP |
| `Keyboard` | 5 | The BIOS for the keyboard, `INT 16h`, answers in the zero flag |
| `Multiplex` | 3 | `INT 2Fh`, by which the driver of a CD-ROM is asked, answers in two registers |
| `DosIdle` | 1: `INT 28h` | The program says that it is idle |
| `MouseDriver` | 1: `INT 33h` | The mouse driver |
| `MouseState` | 4 | The same, which answers in three registers |
| `StackNow` | 1 | The stack pointer, for where the critical error may use the program's stack |
| `Selectors` | 3 | CS and DS, which the routine entered from real mode loads its segments by |
| `Entered` | 5 | ES put on the data and the direction flag cleared, before any compiled instruction counts on either |
| `DeviceEntry` | 33 | Where the extender enters the program for DOS's critical error, with the registers of real mode, DS and ES anywhere and a stack of the extender's: no compiled instruction can run before the segments and the stack are the program's |
| `NothingEntry` | 1: `IRETD` | Where it enters the program for Ctrl-Break and Ctrl-C: an interrupt's return is no function's |

Of the pieces that the C translation has, those of `src/c/dos.c` have
their like here, with registers of 32 bits, but for `VideoBuffer` and
`DesqviewPause`, which are gone (`DEVIATIONS.md`, 2 and 3). Those of
`src/c/resident.c` are gone with what they were for, the entries of INT
21h and INT 27h and the way through INT 2Eh, or are the extender's to
do: where DOS goes on when it has ended the program is no code of the
program's here (see [The system in one place](#the-system-in-one-place)).
Of the three instructions that are the compiler's own in the C
translation, by its intrinsic functions, `IN` is gone too: the program
reads no port.

## The system in one place

What the program asks of the system that it runs on, it is to ask in one
place, and the rest of it is not to know which system that is. The C
translation has one place for the instruction only: every file that
wants something of DOS fills a structure of registers, with the number
of the service among them, and hands it to `Intr21` or `Int21d`, and so
for the BIOS: 348 calls in 25 files, each of which knows
DOS.

In the portable C that place is `src/portable_c/sys.h`: 99 functions
named for what is asked, with no register, no number of a service and no
interrupt in them, and no word of Open Watcom: the test
`portable_c-host` has the file clean. `sys_dos.c` is those functions for
DOS on a PC, for a program of 32 bits under [the
extender](#the-extender), and the one file that knows how DOS is asked:
no other file of the portable C has a structure of registers, the number
of a service, or an interrupt that it calls. The same functions for
another system are other files beside it, and the build picks the files
of an implementation by the end of their names, beside the common code,
every file that ends in no implementation's name, by the lists of
`portable_c_sources` in `CMakeLists.txt`. Those of DOS end in
`_dos.c`: `sys_dos.c` and the stack, `stack_dos.c`. And `sys_posix.c`
is the program for Linux, with the files that emulate DOS there beside
it, each ending in `_posix.c`, and their own header, `emulate_posix.h`:
`machine_posix.c` the part of the functions that has nothing of a
platform in it, and `keyboard_posix.c` the keys made the BIOS's words,
which the web page shares with it, beside `sys_web.c`; see
[linux.md](linux.md) and [web.md](web.md).

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

What DOS and the BIOS enter of the program while it runs is in the
header too, and where they arrive is in `sys_dos.c`: DOS's critical error,
Ctrl-Break and Ctrl-C, each of which the program takes by a function and
the first two of which it gives back by one, in the original's order.
They are interrupts of real mode, which the program takes through the
extender: that comes over from real mode and enters a routine of
`sys_dos.c`, on a stack of its own. What the program does when DOS asks it
about a device, its box with Retry and Abort, is the program's own
function, `FtErr1` of `main.c`, which it hands over when it takes the
critical error; the entry, `DeviceEntry`, goes over to the program's
stack, and what it calls there reads DOS's registers and the header of
the device, calls the program's function as any function is called, with
what DOS says is wrong and with the device's name where it is no drive,
and hands DOS what the program answers. Ctrl-Break and Ctrl-C arrive at
a routine that does nothing.

The fourth thing that DOS enters is where it goes on when it has ended
the program, by another program's hand. Nothing of a program of 32 bits
can run there: the extender sees the call in real mode and takes itself
out first, with the memory above the megabyte that the program is in. So
the program is not asked, and the header has it as a value and not as a
function of the program's to be called. The program says beforehand what
it has changed of DOS that DOS does not put back by itself, which is one
thing, whether DOS looks for Ctrl-C at every call
(`ProgramEndedBreakCheck`, which `SetBrk` of `exec.c` says whenever it
has noted that). And where the PSP has DOS go on, while the program has
taken its end, is eight instructions of real mode that `sys_dos.c` keeps
in a block below the megabyte, written there as their bytes, `Ended_T`:
they have DOS look for Ctrl-C as the program has said, leave the
registers that the original leaves for the program that started this
one, and go on where the PSP said before. The scenario `ended` shows
those registers. One thing is left otherwise, by the extender:
Ctrl-Break goes where it went before the program was started, where the
original leaves it going into memory that is free.

Not in the header, and in `sys_dos.c` all the same, is where the extender
enters the program, `Start`: it settles what compiled code takes for
granted, asks the extender where the PSP and the environment are and for
a block of 512 bytes below the megabyte, makes ready the memory that the
program is handed, and calls `Main` of `main.c`, which is the program.
What the program does first it asks through the header like everything
else: the version of DOS, and that it keep no more of the memory than it
is. The PSP is read and written in `sys_dos.c` alone: the command tail and
the environment, for the functions that answer with them, and the
address at which DOS goes on when the program has ended; and `sys_dos.c`
makes the block that DOS is told of a program to run.

How a call gets to DOS is that file's matter too. DOS, the BIOS and the
mouse driver are of real mode: the program calls them by the same
software interrupts, and the extender takes each call there and back. A
call of DOS that takes the address of a name or of a buffer, and that
the extender knows, the extender translates. Thirteen functions it does
not carry through as `sys.h` has them, and those `sys_dos.c` makes in real
mode, by the extender's service for that, with what DOS reads and writes
in the block below the megabyte. The top of the file says which they are
and how each was found.

Nothing of DOS is left outside `sys_dos.c`: no other file has a line of
assembly or a register in a `#pragma aux`, which the test
`portable_c-host` holds. The commands' loop of `exec.c` has one way of
running a command, by a COMMAND.COM of its own. Quick execute commands,
which handed a command to the COMMAND.COM that the program was started
from, the map of the memory and the watch on DOS are
gone (`DEVIATIONS.md`, 7 and 14).

`python3 tools/calls.py` counts calls that hand a structure of
registers to an interrupt: none in the portable C and all of them in
the C translation, and [the figures](#the-figures) say so. `CLAUDE.md`,
"Moving a file to the system's functions", has how a function is
added to `sys.h`.

## The setup program in it

The original is two programs, and the second, `VCSETUP.COM`, is where
most of the setup is set: every colour, and the options that the main
program's menus have not. It exists only as assembly,
`src/asm/VCSETUP.ASM` with two files of routines that are for the most
part copies of the main program's. In the portable C it is a
part of the one program, entry 8 of `src/portable_c/DEVIATIONS.md`: an
item of the Options menu, " More setup...", its last, behind which the
setup program is on the screen as the original's is when it is run from
the program's command line, over what DOS had shown. It is translated as it
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
is in a window, for F1. What both programs have is there once: the
boxes, the dialogs, the bars and the mouse are the main program's, and
the three routines that the setup program has otherwise
go by a variable that says the setup is open, `StOpen`: `PutKey0`,
which has the setup's key bars then, `Dialog`, and `Input`, where F1 is
the setup's own help. What the setup program has in a segment of its
own is the main data here, in buffers that the main program makes anew
after a command anyway.

`stcolor.c` has the three palettes, Set Black & White palette, Set
Color palette and Set Laptop palette: the list of the 39 colours of
one, with the grid of the 256 colours beside it, and the picture in
which a colour of the list is chosen, one of four screens of the
program painted in the palette. `stcolor_t.c` has its tables, the
four screens among them. What a colour becomes in a shadow, which a
picture asks for a shadow that is not the one set, is `ShadCol` of
`screen.c`, for both programs.

`stopts.c` has the four dialogs of the options: the Configuration,
Screen and Mouse, the Panel options, with the delay of the tree that
is typed there and the box for one that is none, and the Country info,
whose lines are written as the country has them before the box is
shown. `stopts_t.c` has their tables. The first two are without four
settings that the original's setup program has there, Memory
allocation, Quick execute commands, TSR manager and Screen output,
which are of what the portable C has no more: what stood below them in
a box stands further up, and their bytes of the setup file are kept as
they are found (`src/portable_c/DEVIATIONS.md`, 1, 5, 7 and 14).

All nine items of the menu do what the original's do, and F1 is the
help. The mouse at the menu is the main program's routine for a list in
a box, `F32_70` of `usermenu.c`, which the setup program has word for
word under a name of its own. `DIARY.md` has the plan, "Write down the
plan for the setup program, with the scripts of its trial", with what
each piece brought and what is declared for it.

## The extender

A program of 32 bits does not run on DOS by itself: an extender puts the
processor into protected mode, loads the program above the first
megabyte, and hands what the program asks of DOS and the BIOS down to
them. The portable C is started by DOS/32A:

**This product uses DOS/32 Advanced DOS Extender technology.**

The extender is the first 27,536 bytes of the program's file, its
stub, which the linker copies there from `binw/dos32a.exe` of the Open
Watcom that the build fetches. It is in no file of this repository;
its licence is, [`LICENSE.DOS32A`](../LICENSE.DOS32A). One bit of it is changed:
as it comes, DOS/32A writes two lines of its name and its copyright
when it starts, onto what is the program's first screen, and
`tools/stub.c` clears the bit of its settings that makes it, in the
file that the linker wrote. That tool says where the bit is and how it
was found. Open Watcom has other extenders too. CauseWay, which is
free as well, does not run on the machine of the tests, for a fault of
QEMU's (`DIARY.md`, "Write down what a trial of a 32-bit program on the
bench showed"); DOS/4GW and PMODE/W are not free, and are neither linked
nor run here.
