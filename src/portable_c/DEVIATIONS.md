# Deviations of the portable C

The portable C is judged as the C translation was: by the scenarios, with
goldens recorded from the original, Volkov Commander 4.05. Where the
portable C is to behave otherwise than the original, that is a deviation,
and every deviation is declared here, before anything else says so:

- what the original does,
- what the portable C does,
- why,
- and which scenarios show it.

A scenario that a deviation changes gets a golden of its own, recorded from
the portable C and read by whoever records it. Such a golden is kept apart
from those of the original and says where it is from: it shows that nothing
has changed since it was recorded, not that anything is right.

A feature that only made sense on DOS is removed, not kept as a setting
that does nothing, and such a removal is declared here and made without
asking. Any other deviation, the fix of a bug of the original included, is
asked first. A bug of the original stays as it is until its fix has been
decided.

What the C translation, `src/c/`, already does otherwise than the original
is no deviation of this tree: it is in `harness/deviations.py`, and the
portable C began with it.

What the harness goes by of this, the scenarios that a deviation changes
and those that are not run, is in `harness/deviations.py`, under `TREES`;
the goldens of the portable C's own are in `harness/goldens/portable_c/`,
recorded by `python3 harness/goldens.py record --tree portable_c`, and
`python3 harness/goldens.py changes --tree portable_c` prints how each
differs from the original's.

## The deviations

### 1. The program stays in the memory whole while a command runs

**What the original does.** It is two parts. A small one stays in the
memory while a command runs, 12,704 bytes: it has the stack, runs the
command, watches DOS for programs that stay in the memory, and keeps a
copy of the setup, of the command line and of what the two panels show.
The rest is given up for the command:

- The main data and the panels, always. They are one block of memory,
  all that DOS has, which the program gives back before a command and
  asks for again afterwards: the largest block that there is then,
  wherever it is. The main data and the panels are made anew of the
  copy, in which a panel has room for 256 files, each with its name and
  its attributes; a longer list loses its end. Then the panels are read
  again, and the cursor and what was selected are found by name.
- The code, if the Options menu has Memory allocation switched on, as it
  has when the program is distributed. The part that stays then loads
  the code again from the program's own file, run as a second copy that
  moves its code into place and ends. It knows the file by a check sum
  taken when it was loaded. If the file is not there any more, or is
  another, it says "Bad or missing" with the name of the file and "Press
  ENTER to try again or ESC to abort", and Esc ends the program. Shift
  held with Enter turns the switch round for one command.

When it cannot get the memory back, for the code or for its data, the
main data, two panels and 8,000 bytes for the viewer, it says "Program
too big to fit in memory" and ends. The Info panel says how much memory
a command gets: the block of the data, and with the switch on that of
the code as well. The map of the memory, Alt+F5, lists what a command
will find free as free memory: the block of the data, and with the
switch on that of the code. Another program can ask the program for its
data while it runs from it, as the original's setup program does, by a
call of DOS of the program's own (AH 2Bh, CX 'VC', DX 'OM'): it is
handed the three copies, of a panel the start of its segment and behind
that 13 bytes of every file.

The original is a `.COM` file of 64K that DOS loads into any memory that
holds it.

**What the portable C does.** It is one program, and all of it stays in
the memory while a command runs: its code, its main data, its stack and
its two panels. It has no part that stays and none that goes, keeps no
copy of anything, never loads itself again and has no switch for any of
it.

- The Options menu has no line Memory allocation. It has fifteen lines
  where the original has sixteen: Save setup stands a row further up, and
  the box is a row shorter. The letter M chooses nothing there. Shift
  with Enter is Enter.
- The program takes no check sum of its file and does not know a second
  copy of itself: it never says "Bad or missing", and goes on if its
  file is deleted while a command runs.
- The main data is of the program, not a block that it asks DOS for. The
  panels, with the memory of the viewer behind them, are a block that it
  asks for when it starts. For a command that block is made as long as
  two panels of 256 files, and the rest is DOS's. Afterwards the program
  asks for the largest block that there is, as the original does, and
  moves its panels there if that is another: so its memory lies where
  the original's does, in front of a program that a command has left in
  the memory or behind it.
- A panel is left with 256 files for a command, as the original leaves
  it, the cursor's file and the first one shown put last if they are
  beyond, and is read again afterwards: what stays selected is what the
  original keeps. The limit has no reason any more; lifting it would be
  a deviation of its own.
- What the main data has behind the command line, which the original
  has as whatever the memory holds after a command until it is set
  again, is as it was before the command.
- "Program too big to fit in memory" after a command comes only when
  less is free than the viewer needs at the least, 8,000 bytes: the main
  data and the panels are there already. When the program starts, it
  comes when there is less than two panels and that. With less memory
  than the program itself, DOS does not load it, and says so in its own
  words.
- The Info panel says how much memory a command has: all that is behind
  the panels.
- The map of the memory lists as the program's what stays, the program
  and its panels: three blocks with the environment, where the original
  as it is distributed has two, and one interrupt more among those it
  has taken, 24h, DOS's critical error, whose routine the original gives
  up with its code. What is behind the panels is listed as free. When a
  command has left a program in the memory, the panels have moved behind
  it, and the memory in front of it, where they were, is free: a stretch
  of free memory that the original's map has longer or not at all,
  according to where the program stayed.
- The program's own call hands out the main data and the panels
  themselves. They begin as the original's copies do: the setup, which
  is what the original's setup program reads and writes there, the
  command line, and of a panel its kind, its mask and its path. The
  files of a panel are where a panel has them, 24 bytes each, not 13
  behind the path.
- The byte of the setup file that held the switch is kept as it is found
  and written back as it was; the program does not go by it. A setup file
  of the original is one of the portable C and the other way round. The
  original's setup program, `VCSETUP`, which is not translated, still
  shows the switch and writes the byte.
- The original's help file, which the portable C reads, still has its
  page on Memory allocation; no item of the menu leads there.
- It is an `.EXE` file, with the main data in it, which DOS loads and
  relocates. A DOS before 3 is left through the INT 20h of the
  PSP, as the original leaves it, by a call instead of a return; the
  call by which the program asks DOS for its PSP is one of DOS 2 and
  later.

What it costs in memory. While a command runs the portable C keeps the
program and its panels, where the original keeps 12,704 bytes and the C
translation 19,728; and on the bench of the tests DOS has that much less
for a command than the 412,848 bytes that it has for one run from the
original as it is distributed, or the 405,824 for one run from the C
translation. How much the portable C keeps, and what DOS has for a
command then, changes with every step of the work: the README has both
as they stand, under "The figures". It is enough for every command of
the scenarios but those of a second copy of the program: the bench has
500,128 bytes from where the program is loaded, two copies with the
COMMAND.COM that runs the second take most of it, and what is left is
less than the 73K of the COMMAND.COM that a command of the second copy
wants. So a second copy starts there and shows its panels, and has no
memory to run anything; and behind a program that has stayed in the
memory, with the gap that COMMAND.COM leaves in front of it, a second
copy does not start at all.

**Why.** The user's decision, on 12 October 2026: giving up the code and
the data and making them again saves memory that only DOS is short of,
at the price of reading the program's file and its directories after
every command, and of a program in two parts with copies between them.
It "only matters on DOS"; "it's OK for our port to be less memory- and
disk-efficient, as long as it's functionally equivalent"; and the option
"in the new tree goes away. It's going to be a pattern for a lot of
features that only made sense on DOS."

**Which scenarios show it.** Each of these is held against a golden of the
portable C's own:

| Scenario | What changes in it |
|---|---|
| `lost` | The program's file is deleted by a command. The original asks for it, twice, and ends; the portable C is back at its panels, and the keys that answered the original are keys at the panels: a space into the command line, Enter, which runs it, and Esc, which switches the panels off |
| `menu`, `clock`, `clock-russia` | The Options menu where it shows: a line shorter, without Memory allocation. In `menu`, Up from Save setup goes to the clock |
| `memory-map`, `memory-map-russia`, `menu-items`, `mouse-memory`, `mouse-middle`, `mouse-drag-lists`, `stay`, `ended` | The line of the program in the map of the memory: three blocks where the original has two, and 24h among its interrupts |
| `memory-map`, `mouse-memory`, `stay` | Besides: a line of free memory more where a program has stayed. In `memory-map` and `mouse-memory` the program is released again, and the memory in front of the panels and that behind them are two lines where the original has one; in `stay` two lines of free memory stand behind the program where the original has one: where the panels were, and where COMMAND.COM was, with the environment of the program that stayed between the two |
| `memory-map` | Besides: the two steps that switch Memory allocation off and on again, around a second look at the map, are left out |
| `quick` | The step that switches Memory allocation off before the last command is left out; every screen is as the original's after the same step |
| `second-copy` | The second copy is started with a command to run and has no memory for it: "There is not enough memory to run this command" where the original shows what the command printed. The rest is as the original's: the copy's panels, the copy left, the first copy at work |
| `start-memory` | The second copy has no memory to run the command that was to leave it without memory for its data, and says so. It is still there afterwards, where the original has said "Program too big to fit in memory" and ended: the panels of the last three screens are the copy's, with its Info, not the first copy's |
| `ended` | The program that the second copy was to leave in the memory before DOS ends it does not run, for want of memory. So the copy has none to release when it is ended, and the register that says how many it had comes back as it went, 1111h for 0002h |

A step that switches Memory allocation, "F9 o m", is left out of every
scenario for the portable C: there it would open the Options menu and
leave it open.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory` | It switches Memory allocation off, for the Info panel to show the other number, and shows the menu without the mark |
| `passed-on` | It has a program stay in the memory, starts a second copy behind it and runs commands from that, to see what the copy hands on of the calls of DOS that pass it. The bench has no memory for a second copy of the portable C behind a program that has stayed: COMMAND.COM says "Allocation of DOS memory failed." What the program hands on is held by `handed-on` instead, which sees the same with one copy, by a program that puts itself behind it; both C trees pass that against the original's golden |

**In the data.** The test `portable_c-data` holds five tables of the menus
against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages of
their help, the size of the Options menu, its commands, and the item that
Shift-F9 chooses in it.

### 2. No buffer of TopView or DESQview in place of the screen

**What the original does.** When it starts, and again after every
command, it asks whether a program stands between it and the display that
shows a buffer of its own in place of the display's memory: TopView,
IBM's program for running several programs of DOS side by side, and
DESQview, which answers TopView's calls. The call is the BIOS's interrupt
for the screen, INT 10h, with AH 0FEh and the address of the display's
memory in ES:DI: such a program hands back the address of its buffer
there, and the BIOS by itself leaves the address as it is. If the segment
comes back as another, the program draws into that buffer and reads the
screen from it, takes the display for one that shows no snow, and tells
that program which cells have changed every time it has put a piece of
its screen there (INT 10h, AH 0FFh, with the number of cells in CX and
where they begin in DI). It tells it so with ES on its main data, where
the call takes the segment of the buffer.

**What the portable C does.** It makes neither call. Its screen is the
memory of the display, at the page that the BIOS shows, whatever runs
besides. `TopView` and `TopScr` of `dos.c` are gone, with the inline
assembly that they were made of, `VideoBuffer`, three instructions. The
byte of the main data that said whether such a buffer is drawn into,
`TopView`, keeps its place and is neither set nor read.

**Why.** It only made sense on DOS. TopView and DESQview shared a PC
among programs each of which took the machine for its own, and a program
that wrote to the display's memory, as this one does, was asked to write
elsewhere. A system that the program is carried to has a way of its own
to the screen, for which the program will have one place, and no such
program in between to ask.

**Which scenarios show it.** None. The bench runs neither TopView nor
DESQview: the BIOS leaves the address as it is, and of the original the
instructions that go by another address ran in no scenario, four that
`harness/ported.py` lists as out of reach, in `Init34` and in `Window`.
Every scenario is held against the golden that it was held against
before.

### 3. The program does not know of DESQview

**What the original does.** When it starts, and again after every
command, it asks whether it runs under DESQview, a program that shared a
PC among programs of DOS, each in a window: it has DOS set a date that
there is none of (INT 21h, AX 2B01h, with 'DE' in CX and 'SQ' in DX),
which DOS refuses with FFh in AL and DESQview answers for itself. Under
DESQview it does three things otherwise:

- Every time it goes round waiting for a key, also while the screen is
  blanked, it gives the rest of its turn to the other programs of
  DESQview: three calls of INT 15h, which take it onto a stack of
  DESQview's (AX 101Ah), pause it (1000h) and put it back on its own
  (1025h).
- Alt pressed and let go by itself does not bring up the menu bar,
  whatever the Configuration has for "Alt alone selects menu": Alt is
  the key by which DESQview's own menu is called.
- The editor does not empty the keyboard of the keys that wait after
  each key that it has taken, whatever the Configuration has for "Clear
  keyboard buffer".

**What the portable C does.** It does not ask, and does what the original
does without DESQview, also under it: it makes none of those calls, Alt
alone brings up the menu bar if the Configuration says so, and the editor
empties the keyboard if the Configuration says so. `DvPause` of `dos.c`
is gone, with the inline assembly that it was, `DesqviewPause`, six
instructions, and the call of DOS in `IniData` of `start.c`. The byte of
the main data that said whether DESQview runs, `DesqVie`, keeps its place
and is neither set nor read.

**Why.** It only made sense on DOS, and there only next to that one
program. A system that shares the machine among programs by itself is
not told by each of them that it may go on, and takes no key from them
on the way; what the program has to say of being idle it says in one
place, which is the call of INT 28h today and stays.

**Which scenarios show it.** None. The bench does not run DESQview: DOS
refuses the date, and of the original the six instructions that pause
ran in no scenario and are listed as out of reach in
`harness/ported.py`, in `LeaveTm`; the two tests of the byte, in `Input0`
and in the editor, always went the way that the portable C now goes
without testing. Every scenario is held against the golden that it was
held against before.

### 4. The program does not give its turn to Windows

**What the original does.** Every time it goes round waiting for a key,
also while the screen is blanked, it says that it is idle in two ways.
It calls INT 28h, DOS's own word for that, in which the programs that
stay in the memory do their work. And it calls the multiplex interrupt,
INT 2Fh, with AX 1680h, by which a program of DOS gives up the rest of
its turn to whatever shares the machine among programs: Windows from
3.0 on, in its 386 enhanced mode, and what answers the same call, OS/2
and others. Where nothing answers, the call comes back with 80h still
in AL, and the program does not make it again until it has run a
command.

**What the portable C does.** It calls INT 28h and no more. The byte of
the main data that said that nothing answers, `WinLeav`, keeps its place
and is neither set nor read. `Int2F` of `dos.c` stays, for the search of
files, which asks the CD-ROM extensions whether a drive is theirs.

In a window of such a system the portable C built for DOS thus takes
all of its turn while it waits, unless that system knows a program to
be idle by INT 28h or by how it asks for keys, as some do.

**Why.** It only made sense on DOS, next to a system that ran programs
of DOS as guests and could not tell by itself that one of them was
waiting. A system that the program is carried to is asked for the next
key and has the program wait for it; what the program says of being idle
it says in one place, which is the call of INT 28h today. That call
stays: DOS's own resident programs live on it, and the tests take it for
the program waiting for a key.

**Which scenarios show it.** None. On the bench nothing answers the
call, and the original makes it once when it starts and once after every
command, and goes on as if it had not; unless FDAPM answers it, a
program of FreeDOS that `memory-map`, `mouse-memory` and
`mouse-drag-lists` have stay in the memory, which halts the processor
until the next interrupt for a program that says it is idle, by INT 28h
as well. That was not looked into: no screen can show it, and every
scenario is held against the golden that it was held against before.

### 5. The program does not wait for a display that shows snow

**What the original does.** The first colour display of the PC, the CGA,
shows snow, white dots that come and go, when its memory is written or
read while it draws. So a program writes to it between two lines or two
screens, and the original does so for a display that it has found to
want it, when it starts and again after every command. The setup has a
byte for that, Snow mode, which no menu of the program sets: the
original's setup program has it as Screen output, with "Synchronized
access", 0, "Asynchronized access", 1, and "Auto detect", 2, which is
how the program is distributed and what its defaults have. With 1, and
on a monochrome display, the program does not wait. With 0 it tests the
display, and with 2 it tests one that is no EGA or VGA: it reads the
status of the display, port 3DAh, with the machine kept from
interrupting, and if in 32,767 tries the lowest bit is once set seven
times running, which says that the display is not drawing for long
enough to be seen at it, the program waits from then on:

- A window of one row is put on the screen cell by cell, each when the
  display has just finished a line.
- A window of more rows, and the screen read when the program starts,
  are copied seven rows at a time, each seven when the display has begun
  to return to the top of the screen: ten looks in a row have to find it
  not drawing.

Each cell and each seven rows are copied with the machine kept from
interrupting, which it may again afterwards. The original says so after
every cell and every seven rows whatever the display is.

**What the portable C does.** It does not test the display, and writes
to its memory and reads it at once. `InPort` of `dos.c` is gone, and of
the compiler's intrinsic functions `inp`, the instruction `IN`: the
program reads no port any more. The byte of the main data that said
whether to wait, `Snow`, keeps its place and is neither set nor read.
Snow mode is a byte of the setup that is kept as it is found and written
back as it was, 2 in a setup made of the defaults: a setup file of the
original is one of the portable C and the other way round, and the
original's setup program, which is not translated, still shows the
choice and writes the byte.

One thing is left of it, because more than the snow may rest on it: when
a window has been put on the screen, or the screen has been read, the
machine may interrupt the program, whether it might before or not. DOS
calls the program for a critical error with the machine kept from
interrupting, and the box that the program shows then is put on the
screen like any other.

**Why.** It only made sense on a PC with that display. No system that
the program is carried to has a display's memory that may be written
only while the beam is elsewhere, and a wait that is timed by reading a
port has nothing to read there.

**Which scenarios show it.** None. The bench has a VGA. `start-setup`
has a setup with Snow mode 0, so the original tests the display there,
and finds nothing: QEMU's VGA turns the bit over every time it is read,
so the instruction that sets the program to waiting, and the 20 of
`Window` and `MovScr` that wait, ran in no scenario and are listed as
out of reach in `harness/ported.py`. In that scenario the portable C
does not test, shows the same screens and leaves the same setup file,
with the byte as it found it. Every scenario is held against the golden
that it was held against before.

### 6. DOS and the BIOS are handed nothing that they are not asked

**What the original does.** It calls DOS and the BIOS with the registers
that a call takes set for it, and every other register as the code
before happens to have left it: what the call before answered, a
counter, an address. The low byte of AX is often like that, since the
high byte alone says which service is meant, and so are the half of BX
that a call does not take, CX and DX of a call that takes neither, and
DS where no address goes with the call. The C translation hands over
the same where it kept the registers from one call to the next in one
structure, `0x3E00 | regs.ax & 0xFF`, and where it did not, it already
hands over something else: a register that it never sets is what the
stack holds, and `TODO.md` says where.

**What the portable C does.** It asks through the functions of `sys.h`,
and a function is handed what is asked and nothing else. So a call gets
the registers that it takes from the function's arguments; AL is 0
where the service takes nothing there, and so is the half of BX that it
does not take; DS is on the main data where no address is handed over;
and a register that the service takes nothing in is 0 or what the stack
holds, as `dos.c` has it. The calls, their order and every register
that a call takes are the original's.

The places so far, each with what the original hands over there in a
register that the call does not take. The files that have not gone over
to `sys.h` still hand over what the C translation does, and their places
are added here as they go.

| Routine | Call | The original | The portable C |
|---|---|---|---|
| `OpenFil`, `system.c` | DOS 3Dh, a file opened, both times | BX: the service and the mode, as in AX | BX as it comes |
| `SetPth`, `system.c` | DOS 47h, the current directory | AL: the drive, 1 for A: | AL 0 |
| `Fantom`, `system.c` | DOS 440Fh, the drive that a unit goes by, three times | BH: the other of the two drives | BH 0 |
| `ChkMode`, `window.c` | BIOS 0Fh, the mode | BX as the caller has it | BX 0 |
| `Cursor`, `window.c` | BIOS 0Fh and 02h, the cursor placed | 0Fh: AL the column, DX the place. 02h: AL the mode, as 0Fh left it | AL 0 for both, DX 0 for 0Fh |
| `ChgCur`, `window.c` | BIOS 01h, the shape of the cursor | DX: where the cursor is, as 03h left it | DX 0 |
| `GetCur`, `window.c` | BIOS 03h, where the cursor is | AL: the mode, as 0Fh left it | AL 0 |
| `Scroll`, `window.c`, for its seven callers | BIOS 06h and 07h, a window scrolled | BL: the column of a place that the caller has | BL 0 |
| `M30_50`, `dirfunc.c` | DOS 5700h, the date and the time of a file, and 3Eh, the file closed | CX and DX: the attributes and the address of the name for the first, the time just set and the address of the name for the second | CX and DX 0 |
| `DelTr0`, `dirfunc.c` | DOS 3Eh, a file closed | CX 1 and DX the address of the byte, of the call that wrote it | CX and DX 0 |
| `FncMs1`, `dirfunc.c` | DOS 4Fh, the next file | CX 37h and DS:DX on the mask, of the call for the first file | CX and DX 0 |
| `ReadOne`, `start.c` | DOS 3Dh, 3Fh and 3Eh, a file read | 3Dh: BX, CX, SI and DI 0. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still | 3Dh: those four as they come. AL 0 for the other two, and for 3Eh CX as it comes and DX 0 |
| `IniData`, `start.c` | DOS 38h, the country, and DOS's routine for capitals, 128 times | 38h: BX and CX as the BIOS left them at the call before, the page and the shape of the cursor. The routine: AH as 38h left it | BX and CX as they come; AH 0 |
| `HelpFil`, `help.c` | DOS 3Eh, the help file closed | DX as the code before left it, the address of the file's name in the C translation | DX 0 |
| `Load`, and Home and the search of `V00_10`, `view.c` | DOS 4200h, a place of the file gone to | DS: the segment of the text | DS on the main data |
| `V13_00`, `view.c` | DOS 3Eh, the file closed | DS: the segment of the text. DX as the key before left it, the address of `TmpBuf` in the C translation | DS on the main data, DX 0 |
| `Aexec0`, `view.c` | BIOS 03h, the shape of the cursor | BL: the column that the box begins in | BL 0 |
| `Aexec0`, `view.c` | DOS 4Ah, the block of the memory made shorter, 4B00h, the viewer run, and 4Ah twice, the block made longer | 4Ah: DX as the code or DOS before left it, the address of the viewer's name in the C translation; the third time AL the low byte of the error of the second, 8. 4Ah the first time and 4B00h: CX, SI and DI as the code before left them, 0 in the C translation | 4Ah: AL and DX 0. CX, SI and DI as they come |
| `F34_11`, `view.c` | DOS 3Fh and 3Eh, the extension file read and closed, and 4Eh, the viewer looked for | 3Eh: CX and DS:DX the count and the buffer still. 3Fh and 4Eh: SI and DI, and for 4Eh BX, as the code before left them, 0 in the C translation | 3Eh: CX and DX 0. SI, DI and BX as they come |
| `FindFil`, `find.c` | DOS 4Fh, the next file | CX as the code before left it, and DS:DX on the path | CX and DX 0 |
| `FindFil`, `find.c` | DOS 4201h, 80 bytes gone back in a file that is searched | DS: the segment that the file is read to | DS on the main data |
| `FindFil`, `find.c` | DOS 3Eh, a file that was searched closed | CX and DX as the calls before left them, the address of `TmpBuf` in DX in the C translation | CX and DX 0 |
| `F67_00`, `find.c` | DOS 4409h and 4408h, a drive asked whether it is of a network and whether its disk can be taken out | CX: the drive, 0 for A:. DX: the address of the path for 4409h, and for 4408h what 4409h left there | CX as it comes, DX 0 |
| `F67_00`, `find.c` | DOS 0Eh and 19h behind Go to, a drive made the current one and the current one asked for | DS: the segment of the list for 0Eh and of the panel for 19h, of the list for both in the C translation. DL of 19h: the drive still | DS on the main data. DL 0 for 19h |
| `T80_02`, `edit.c` | DOS 4400h, what the handle of the file is that was made, and 3Eh, the file closed | 4400h: CX the attributes and DS:DX on the name of the file. 3Eh: CX the attributes, the count of the last write in the C translation, and DX as the call before left it; DS on the piece last written in the C translation | 4400h: CX as it comes, DX 0. 3Eh: CX and DX 0, DS on the main data |
| `T00_00`, `edit.c` | DOS 3Eh, the file closed that was read | CX: the count of the last read. DS: the segment of the piece last read | CX 0, DS on the main data |

What is known to read such a register, and where the portable C will
hand it something else when the file goes over:

- AL of DOS 4Eh, the first file of a search. DOS's APPEND, where it is
  installed, goes by it. `FindFirst` hands over 0, as the original does
  at most places; at three it has a leftover there, the number of the
  drive and 5Ch in `read.c` and 2 in `treebox.c`. At two more the
  original has one where the C translation already handed over 0, so
  nothing changed there when the file went over: what the keys left in
  `FindFil` of `find.c`, and a character of the extension file in
  `F34_11` of `view.c`.
- AL of BIOS 01h, the shape of the cursor. Some BIOSes of the time are
  said to want the video mode there. `CursorShape` hands over 0, as the
  original does where an input line sets the shape, by `ChgCur` and
  `CurShp`; in `IniData` of `start.c` and in `Blank` the original has
  the mode there, left from the call that asked for it.
- CX and DL of BIOS 1130h, the font of an EGA, which a BIOS that does
  not know the call hands back as they came. `ScreenFont` puts 16 and 0
  there, so such a BIOS is taken to have a font of 16 lines and to say
  nothing of the rows. The original has 16 in CL where `F69_00` of
  `commands.c` asks for the height and 0 in DL where it and `IniData`
  ask for the rows, and leftovers in the other of the two each time,
  which it does not read then.

Nothing else that the functions leave out is known to be read by a DOS
or a BIOS.

**Why.** A function of `sys.h` says what is asked, and what a register
happened to hold is not asked: it cannot be an argument without a name,
and a name for it would be the name of a register. What the original
goes on with behind a call is another matter and is kept: what DOS
leaves in AX where nothing failed, a register that a call did not set,
the carry flag before a call, and the page that the BIOS is handed,
which the functions answer with or take by name.

**Which scenarios show it.** None. FreeDOS and the bench's BIOS read none
of these registers, APPEND is not installed there, and every scenario
passes against the golden that it had.
