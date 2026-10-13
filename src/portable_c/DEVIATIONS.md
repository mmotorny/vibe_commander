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

- The Options menu has no line Memory allocation: Save setup, below
  it, stands a row further up for that, and the letter M chooses nothing
  there. The menu has sixteen lines all the same, as the original's,
  since it has a line that the original has not, the setup, further up:
  see 8. Shift with Enter is Enter.
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
- The map of the memory, which listed what stays as the program's, is
  gone from the portable C since: see 7.
- The program's own call hands out the main data and the panels
  themselves. They begin as the original's copies do: the setup, which
  is what the original's setup program reads and writes there, the
  command line, and of a panel its kind, its mask and its path. The
  files of a panel are where a panel has them, 24 bytes each, not 13
  behind the path.
- The byte of the setup file that held the switch is kept as it is found
  and written back as it was; the program does not go by it. A setup file
  of the original is one of the portable C and the other way round. The
  original's setup program, `VCSETUP`, still shows the switch and writes
  the byte, and so will the setup of the portable C, which is that
  program (see 8), until the setting is taken out there.
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
| `clock`, `clock-russia` | The Options menu where it shows: without Memory allocation. And by 8: with the setup above, so as long as the original's, the lines from Auto menus to the clock a row further down than there |
| `menu` | The same, and Up from Save setup goes to the clock. By 8: five times Down from the top is the setup, where the original has Auto menus, and F1 there is the index of the help. And by 7: the Commands menu where it shows, five times, a line shorter, without Memory info, with the items that are dark there a row further up |
| `quick` | The step that switches Memory allocation off before the last command is left out; every screen is as the original's after the same step |
| `second-copy` | The second copy is started with a command to run and has no memory for it: "There is not enough memory to run this command" where the original shows what the command printed. The rest is as the original's: the copy's panels, the copy left, the first copy at work |
| `start-memory` | The second copy has no memory to run the command that was to leave it without memory for its data, and says so. It is still there afterwards, where the original has said "Program too big to fit in memory" and ended: the panels of the last three screens are the copy's, with its Info, not the first copy's |
| `ended` | The program that the second copy was to leave in the memory before DOS ends it does not run, for want of memory. And by 7: the copy would have none to release in any case, and the registers that the program which started it prints are as the portable C leaves them, both times: CX and SI as they went, 2222h and 1111h, where the original has 0 and, the second time, 0002h; and DS:DX no longer where INT 27h goes, which the original has as 0 and 0 from there. The portable C has where INT 21h went before, the entry of the first copy: an address in the program, which changes with every step of the work and is left out of the comparison, in `harness/deviations.py`. The look at the map of the memory that ends the scenario, two steps, is left out |

A step that switches Memory allocation, "F9 o m", is left out of every
scenario for the portable C: there it would open the Options menu and
leave it open.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory` | It switches Memory allocation off, for the Info panel to show the other number, and shows the menu without the mark |
| `passed-on` | It has a program stay in the memory, starts a second copy behind it and runs commands from that, to see what the copy hands on of the calls of DOS that pass it. The bench has no memory for a second copy of the portable C behind a program that has stayed: COMMAND.COM says "Allocation of DOS memory failed." What the program hands on is held by `handed-on` instead, which sees the same with one copy, by a program that puts itself behind it. The C translation passes that against the original's golden; the portable C, which hands on more, has a golden of its own there: see 7 |

**In the data.** The test `portable_c-data` holds three tables of the
menus against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages
of their help, and the commands of the Options menu. Two more were
declared for it until the setup was put into that menu (see 8), and are
the original's again with a line less and a line more: the size of the
Options menu, and the item that Shift-F9 chooses in it.

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
original's setup program still shows the choice and writes the byte, as
the setup of the portable C will, which is that program (see 8), until
the setting is taken out there.

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

The places, file by file and in each file as its routines stand, each
with what the original hands over there in a register that the call
does not take, read from the assembly; what the C translation handed
over, where that was noted to be something else; and what the portable C
hands over. Where that column is empty, nothing was noted: as a rule
the C translation handed over what the original does. Every file of
the program has gone over to `sys.h`, and each added its places here as
it went.

| Routine | Call | The original | The C translation, where it differed | The portable C |
|---|---|---|---|---|
| `Blank`, `blank.c`, with its `Star` | BIOS 0Fh, 03h, 01h, 02h and 09h: the cursor noted, hidden and put back, and a star written | 0Fh: AL and BX as the code before left them, for a star that changes its look and its clock and kind. 03h, 01h and 02h: AL the mode, as 0Fh left it. 01h: BH the page, and DX where the cursor is or was put. 02h: BL and CX as the code before left them, for a star its clock and a count of the stars. 09h: DX the place of the star |  | AL 0 at all but 09h; BX, CX and DX 0 where the call takes nothing in them |
| `M02_10`, `commands.c` | DOS 3Bh, the directory of a panel made current | AL the drive, 1 for A: |  | AL 0 |
| `F69_00`, `commands.c` | EGA lines: BIOS 1130h, the font asked about; 00h, the mode set; 1112h, the small font loaded; 12h with 20h in BL, Print Screen told of the rows; and 1130h again | 1130h the first time: CH what the main loop has there, the number of the item where the command is chosen from the menu, and DX the last row of the screen and 80, of the screen put back as DOS had it. 00h, 1112h and 12h: CX and DX as the BIOS left them at the call before, the height of the font and its last row from 1130h. 12h: AL as 1112h left it. 1130h the second time: CX and DH as the calls before left them (see below) |  | CH and DX 0 the first time. CX and DX 0 at 00h, 1112h and 12h, and AL 0 at 12h. CX 16 and DH 0 the second time |
| `F35_61`, `copy.c` | DOS 0Eh, a drive made the current one, twice | AL the drive, as in DL, the first time, and as choosing from the tree left it the second; DH as the code before left it |  | AL and DH 0 |
| `C00_01`, `copy.c` | DOS 4400h, what a handle is, and 3Eh, the file closed | 4400h: CX what was left of comparing the target with the directory of the other panel, DX the address of the name. 3Eh: AL as 4400h left it, CX that still, DX what the handle is |  | CX as it comes and DX 0 at 4400h. AL, CX and DX 0 at 3Eh |
| `C00_01`, `C10_00`, `C20_00` and `C30_00`, `copy.c` | DOS 4300h, the attributes of a file or a directory, at seven of the eight places | CX as the code before left it: what was left of comparing two names or of a piece, or the attributes that the call before answered with |  | CX 0 |
| `C10_00` and `C20_00`, `copy.c` | DOS 41h, a file deleted, at nine places | AL as the call or the dialog before left it; CX the attributes of the file, as they were asked for or the 20h just set, or what else the code before left |  | AL and CX 0 |
| `C10_00` and `C30_00`, `copy.c` | DOS 56h, a file or a directory renamed, at three places | CX what was left of comparing the two names, or at the second try of `C10_00` the attributes of the target, with AL as 41h left it |  | CX as it comes; AL 0 |
| `C20_00`, `copy.c` | DOS 4400h, what a handle is, for the source and for the target; 3Ch, the target made; and 3Eh, a file closed, at five places | 4400h: DX the address of the name, CX what the code before left, 20h behind a file made. 3Ch: AL as the call before left it, the error of 4300h or what 41h answered. 3Eh: AL as the call before left it at three of the five; CX and DX what that call took or answered, which is what was left of a piece and where it was to go in the memory, the time and the date just set, the length of the file, or the address of the name; and DS on the memory that the file is read into, at three |  | CX as it comes and DX 0 at 4400h. AL 0 at 3Ch. AL, CX and DX 0 and DS on the main data at 3Eh |
| `C20_00`, `copy.c` | DOS 3Fh, the last byte read of a file that is appended to; 36h, the room of a drive; and 40h, what still fits of a piece written | 3Fh: AL the low byte of the place just gone to. 36h: BX the handle and CX the size of the piece, DH the second character of the target's name, DS on the memory that the file is read into. 40h: AL the low byte of the count, as in CL |  | AL 0 at 3Fh and 40h. BX, CX and DH 0 and DS on the main data at 36h |
| `C20_00`, `copy.c` | DOS 5700h, the date and the time of a file: of a source that is a device, and of a target that is appended to | CX and DX: what was left of comparing the two names, or the attributes just kept or set, and the address of the name |  | CX and DX 0 |
| `C20_00`, `C30_00` and `F36_04`, `copy.c` | DOS 39h, a directory made; 3Ah, one removed; and 3Bh, one made the current one | AL as the call before left it, but at 39h of `C20_00`; CX the attributes of the source or of the target at 39h and 3Ah of `C30_00`, what was left of looking for the end of the path from the second directory that `C20_00` makes, or what else the code before left |  | AL and CX 0 |
| `F38_40`, `dirfunc.c` | DOS 41h, a file deleted that was read-only | CX 20h, of the call that set its attributes |  | CX 0 |
| `M30_50`, `dirfunc.c` | DOS 5700h, the date and the time of a file, and 3Eh, the file closed | 5700h: CX the attributes and DX the address of the name. 3Eh: CX the time just set and DX the address of the name |  | CX and DX 0 at both |
| `F38_60`, `dirfunc.c` | DOS 60h, 3Bh and 3Ah, the current directory left and removed | 60h: DX the address of the path, as SI. 3Bh and 3Ah: SI on the path and DI behind the two full stops, of 60h and of what was written since |  | DX 0 at 60h; SI and DI as they come at 3Bh and 3Ah |
| `FncMs1`, `dirfunc.c` | DOS 4Fh, the next file | CX 37h and DS:DX on the mask, of the call for the first file |  | CX and DX 0 |
| `DirFunc`, `dirfunc.c` | DOS 4Eh and 4Fh, the first file and the next, in both of its searches | 4Eh: AL the last character of the directory's path. 4Fh: DS:DX on the mask, of the call for the first file, and CX 37h of that call or what a count has come down to since |  | AL 0 at 4Eh, CX and DX 0 at 4Fh |
| `DelTr0`, `dirfunc.c` | DOS 3Eh, a file closed | CX 1 and DX the address of the byte, of the call that wrote it |  | CX and DX 0 |
| `F54_00`, `dirfunc.c` | DOS 13h, the label deleted for a new one, and 3Eh, the new one's file closed | 13h: AL a space, of filling the name with them. 3Eh: CX 8 and DX the address of the name, of the call that made the file |  | AL 0 at 13h, CX and DX 0 at 3Eh |
| `F62_01`, `drive.c` | DOS 440Eh, the letter that a floppy drive goes by, for A: and for B:, and 440Fh, the letter that was chosen made the drive's | 440Eh: BH the other of the two drives, CX the item of the menu bar, DX as looking for the drives left it. 440Fh: BH the letter, CX the item, DX the drive with the height of the box above it. DS on the panel at both | 440Eh: DX the address of `TmpBuf`, DS on the main data | BH and DX 0, CX as it comes, DS on the main data |
| `F62_01`, `drive.c` | DOS 47h, 3Bh and 0Eh, a panel put on the drive that was chosen | 47h: AL as 440Fh answered, the drive that the unit goes by; BX the letter and the drive, of that call; CX the item of the menu bar; DH the height of the box. 3Bh: AL the low byte of what 47h left, BX and CX still. 0Eh: AL the low byte of the panel's segment, DH still, and DS on a panel |  | AL 0 at the three; BX and CX as they come at 47h, and CX 0 at 3Bh; DH 0; DS on the main data at 0Eh |
| `T80_02`, `edit.c` | DOS 4400h, what the handle of the file is that was made, and 3Eh, the file closed | 4400h: CX the attributes and DS:DX on the name of the file. 3Eh: CX the attributes, and DX as the call before left it | 3Eh: CX the count of the last write, DS on the piece last written | CX as it comes and DX 0 at 4400h. CX and DX 0 and DS on the main data at 3Eh |
| `T00_00`, `edit.c` | DOS 3Eh, the file closed that was read | CX the count of the last read. DS on the segment of the piece last read |  | CX 0, DS on the main data |
| `F21_10`, `enter.c` | DOS 3Bh, the directory of the tree made current | CX 37h, of the call that looked for the directory, where it did |  | CX 0 |
| `FindFil`, `find.c` | DOS 4Fh, the next file | CX as the code before left it, and DS:DX on the path |  | CX and DX 0 |
| `FindFil`, `find.c` | DOS 4201h, 80 bytes gone back in a file that is searched | DS on the segment that the file is read to |  | DS on the main data |
| `FindFil`, `find.c` | DOS 3Eh, a file that was searched closed | CX and DX as the calls before left them | DX the address of `TmpBuf` | CX and DX 0 |
| `F67_00`, `find.c` | DOS 4409h and 4408h, a drive asked whether it is of a network and whether its disk can be taken out | CX the drive, 0 for A:. DX the address of the path at 4409h, and at 4408h what 4409h left there |  | CX as it comes, DX 0 |
| `F67_00`, `find.c` | DOS 0Eh and 19h behind Go to, a drive made the current one and the current one asked for | DS on the segment of the list at 0Eh and of the panel at 19h. DL the drive still at 19h | DS on the segment of the list at both | DS on the main data. DL 0 at 19h |
| `HelpFil`, `help.c` | DOS 3Eh, the help file closed | DX as the code before left it | DX the address of the file's name | DX 0 |
| `ErrRdy`, `main.c` | DOS 440Fh, the drive of the box made the one that its unit goes by | BH the row of the box, CX the letter of the drive, DX the size of the box |  | BH and DX 0, CX as it comes |
| `Init40`, `main.c` | BIOS 03h, where the cursor is, and 0Eh, a line feed typed | 03h: AL the mode, as 0Fh left it. 0Eh: CX the shape of the cursor and DX its place, as 03h left them, with the row that the program goes on with |  | AL 0 at 03h, CX and DX 0 at 0Eh |
| `Init50`, `main.c` | DOS 47h, the current directory of the drive that was chosen | AL the drive, 1 for A:. DH the rows of the screen. CL the letter of the drive |  | AL and DH 0, CX as it comes |
| `F39_00`, `menu.c` | BIOS 02h of the keyboard, the keys that are held | AL 2, the menu that the command is of |  | AL 0 |
| `ReDr00`, `read.c` | DOS 4Eh, 47h and 4Fh, the directory of a panel read | 4Eh: AL the drive, 1 for A:. 47h: AL the drive too, and CX 37h of the search. 4Fh: DS:DX on the mask, of the call for the first file, and CX 37h of that call or 0, where a copy has counted it down |  | AL 0 at 4Eh and 47h, CX as it comes at 47h, CX and DX 0 at 4Fh |
| `ReDr00`, `read.c` | DOS 3Fh and 3Eh, the extension file of the directory read and closed | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what reading answered, CX and DS:DX the count and the buffer still. And the original reads AL back behind 3Eh (see below) |  | AL 0 at both, CX and DX 0 at 3Eh |
| `RdInfo`, `read.c` | DOS 36h, the room of the drive, and 3Fh and 3Eh, dirinfo read | 36h: AL the drive, as DL. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | AL 0 at the three, CX and DX 0 at 3Eh |
| `GetVol`, `read.c` | DOS 60h, the full name of the drive, and 4Eh, its label | 60h: BX, CX and DX as the caller has them, for the Info the handle of dirinfo, the count and the buffer. 4Eh: AL 5Ch, of the two backslashes that the name was held against |  | BX and CX as they come and DX 0 at 60h. AL 0 at 4Eh |
| `OutStr`, with `OutStr0`, `resident.c` | BIOS 0Eh, a character typed, for every character of what the code of the commands writes | CX and DX as the code before left them | CX and DX 0 | CX and DX 0 |
| `NotFind`, `resident.c` | BIOS 00h of the keyboard, a key waited for | AL as the call before left it | AL 0 | AL 0 |
| `SetBrk`, `resident.c` | DOS 3300h and 3301h, whether DOS looks for Ctrl-C at every call, asked and set | 3300h: DX as whoever calls has it. 3301h: DH as 3300h left it | DS 0 at both, DX 0 at 3300h | DX 0 at 3300h and DH 0 at 3301h, DS on the main data |
| `Exit02`, `resident.c` | DOS 3301h, Ctrl-C looked for as before, and 4Ch, the program ended | 3301h: DH as the code or DOS before left it. 4Ch: DS:DX on where an interrupt went before the program took it, of giving it back, where the watch on DOS is on | DS 0 at 3301h | DH 0 at 3301h; DS on the main data at both, and DX 0 at 4Ch |
| `TakeDos`, `resident.c` | DOS 3521h, where INT 21h goes | BX the address of what the original keeps of the main data while a command runs. CX, DX, SI and DI as the code before left them | CX, SI and DI 0 | BX, CX, SI and DI as they come, DX 0 |
| `Exec02`, `resident.c` | DOS 3301h, Ctrl-C looked for as before; 4B00h, COMMAND.COM run; 19h and 47h, the current drive and its directory for the prompt; BIOS 0Eh, the end of the line typed, twice | 3301h: DH as the code before left it. 4B00h: CX the length of the command's tail, with the slash above it; SI on the tail and DI on the name COMMAND.COM. 19h: AL 0Dh, of the start of the prompt, and DX as the command or DOS left it. 47h: AL a colon, of the prompt. 0Eh: CX and DX as the code before left them | CX and DX 0 at 0Eh | DH 0 at 3301h. CX, SI and DI as they come at 4B00h. AL and DX 0 at 19h, AL 0 at 47h. CX and DX 0 at 0Eh |
| `SetStat`, `resident.c` | DOS 5801h and 5803h, how DOS looks for memory | CX and DX as the code before left them |  | CX as it comes, DX 0 |
| `Alloc0`, `resident.c` | DOS 30h, the version, and 4Ah, the block of the program made shorter | 30h: AL as taking the check sum of the file left it, which the portable C does not take. 4Ah: AL the minor number of the version; CX and DX how much of itself the original then moves, in bytes and in paragraphs |  | AL 0 at both, CX as it comes and DX 0 at 4Ah |
| `SaveSet`, `setup.c` | DOS 4300h, 3Ch, 40h and 3Eh, the setup file written | 4300h: BX the length of the file. 3Ch: AL the low byte of what 4300h answered, BX the length still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still |  | AL 0 at the three; BX as it comes at 4300h and 3Ch; CX and DX 0 at 3Eh |
| `ReadOne`, `start.c` | DOS 3Dh, 3Fh and 3Eh, a file read | 3Dh: BX, CX, SI and DI 0. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | BX, CX, SI and DI as they come at 3Dh. AL 0 at 3Fh and 3Eh; CX as it comes and DX 0 at 3Eh |
| `Init00`, `start.c` | DOS 19h and 47h, the current drive and its directory; BIOS 00h, the mode set; DOS 09h, the banner written | BX 1 if the setup file was not one of the program's and 0 if it was or there is none, and from the BIOS's call on the page that is shown above that. CX 0 at 19h and 47h |  | BX and CX as they come at the calls of DOS, BX 0 at the BIOS's |
| `Init11`, and `TakeMem`, which asks as it does, `start.c` | DOS 48h, a block of the memory, the second time | AL the low byte of the error of the first time, 8 for too little memory |  | AL 0 |
| `IniData`, `start.c` | DOS 38h, the country, and DOS's routine for capitals, 128 times | 38h: BX and CX as the BIOS left them at the call before, the page and the shape of the cursor. The routine: AH as 38h left it |  | BX and CX as they come at 38h. AH 0 at the routine |
| `IniData`, `start.c`, where a command has left a screen that is not text of 80 columns | BIOS 01h, the cursor hidden; 00h, the mode set; 0Fh, the mode | 01h: AL the mode, as 0Fh left it (see below), and BX the page and the columns. 00h: BX the same, and CX 2000h, the shape that hides the cursor. 0Fh: AL as 00h left it, BX and CX the same |  | AL 0 at 01h and 0Fh; BX and CX 0 where the call takes none |
| `IniData`, `start.c` | BIOS 03h and 01h, the shape of the cursor asked for and set | 03h: AL the mode; BL, CX and DX as the calls before left them. 01h: AL as 03h left it, the mode from a BIOS that leaves AX alone (see below); BH the page; DX where the cursor is, as 03h answered |  | AL 0 at both; BL, CX and DX 0 at 03h, BX and DX 0 at 01h |
| `IniData`, `start.c` | DOS 19h, the current drive; BIOS 12h and 1130h, an EGA and its font | 19h: BX and CX as the calls before left them. 12h: AL the drive, as DOS answered; CX and DX as left. 1130h: BL and CX as 12h answered, the memory of the EGA and its switches (see below); DH as left |  | BX and CX as they come at 19h. AL, CX and DX 0 at 12h. BL and DH 0 and CX 16 at 1130h |
| `IniData`, `start.c` | DOS 3306h and 30h, the version | 30h: BX what 3306h left in AX. CX and DX of both as the calls before left them |  | BX and CX as they come, DX 0 |
| `IniData` with `CurNl`, `start.c` | The colour that DOS writes in: BIOS 0Fh and 03h, the page and the cursor, twice; 0Eh, a carriage return or a line feed typed, three times; 08h, the cell at the cursor, twice; DOS 06h, a space written; BIOS 09h, the cell written back | 0Eh: BL what DOS left when it was asked for its version, the low byte of AX from 3306h or BL from 30h; the BIOS takes a colour there only where the screen is graphics. 06h: AL the character at the cursor, as 08h answered; DH the row of the cursor; BX and CX as the BIOS left them. AL of 0Fh, 03h and 08h as the call before left it. CX and DX of the BIOS's calls the shape of the cursor and where it is, as 03h answered, with the space in DL from 06h on |  | BL 0 at 0Eh. AL and DH 0 at 06h, BX and CX as they come. AL 0 at 0Fh, 03h and 08h, and 0 in whichever of BL, CX and DX a call of the BIOS does not take |
| `StDefault`, `stmain.c` | DOS 19h, the current drive, for the panels that the setup makes without a setup file | AL as the code before left it: what DOS answered about the setup file, or 0, the answer of the box that asks |  | AL 0 |
| `StInit15`, `stmain.c` | DOS 3Dh, 3Fh and 3Eh, the setup file read | 3Dh: BX, CX, SI and DI as the help file read and the name copied left them. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | BX, CX, SI and DI as they come at 3Dh. AL 0 at 3Fh and 3Eh; CX and DX 0 at 3Eh |
| `StExit`, `stmain.c` | BIOS 01h, the cursor in its shape again; DOS 4300h, 3Ch, 40h and 3Eh, the setup file written; 09h, the line that says it could not be | 01h: DX the rows of the screen and 80, of the screen put back. 4300h: BX the sum of the file. 3Ch: AL as 4300h left it, or 0 for a file that is not there; BX the sum still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still. 09h: AL, BX and CX as the calls before left them |  | DX 0 at 01h. AL 0 at 3Ch, 40h, 3Eh and 09h; BX as it comes at 4300h and 3Ch; CX and DX 0 at 3Eh; BX and CX as they come at 09h |
| `StSetup`, `stmain.c` | BIOS 0Fh and 03h, the page and the cursor, where the setup notes the row and the shape of the cursor | 0Fh: AL and BX as making the table of the small letters left them. 03h: AL the mode, as 0Fh left it; BL, CX and DX as the code before left them |  | AL 0 at both; BX 0 at 0Fh, and BL, CX and DX 0 at 03h |
| `OpenFil`, `system.c` | DOS 3Dh, a file opened, both times | BX the service and the mode, as in AX |  | BX as it comes |
| `SetPth`, `system.c` | DOS 47h, the current directory | AL the drive, 1 for A: |  | AL 0 |
| `Fantom`, `system.c` | DOS 440Fh, the drive that a unit goes by, three times | BH the other of the two drives |  | BH 0 |
| `ReadTre`, `treebox.c` | DOS 3Fh and 3Eh, the tree's file read | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | AL 0 at both, CX and DX 0 at 3Eh |
| `ReadTre`, `treebox.c` | DOS 4Eh and 4Fh, the directories of the disk read | AL 2 at both, which is put there for how the reading ends if a key stops it. 4Fh: CX and DX as the code between left them, 37h and the address of the mask, of the call for the first file, or a count and the size of the box that counts |  | AL 0 at both, CX and DX 0 at 4Fh |
| `ReadTre`, `treebox.c` | DOS 41h, the tree's file deleted, and 4300h, 3Ch, 40h and 3Eh, the file written | 41h: CX as the reading of the disk left it. 4300h: BX and CX the length of the file and of its list. 3Ch: AL the low byte of what 4300h answered, BX the length still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still |  | AL 0 at all of them; CX 0 at 41h, 4300h and 3Eh; BX as it comes at 4300h and 3Ch; DX 0 at 3Eh |
| `FileLst`, `usermenu.c` | DOS 41h, 3Ch, 40h and 3Eh, the list of the selected files written | 41h: AL the low byte of what setting the attributes answered, and CX 20h of that call. 3Ch: AL as the call before left it. 40h, for the mask of a directory and for the end of a line: AL the low byte of what the write before answered. 3Eh: AL as the last write left it, with CX and DS:DX the count and the text of that write |  | AL 0 at all of them, CX 0 at 41h, CX and DX 0 at 3Eh |
| `F32_00`, `usermenu.c` | DOS 3Fh and 3Eh, the menu file read | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | AL 0 at both, CX and DX 0 at 3Eh |
| `Load`, and Home and the search of `V00_10`, `view.c` | DOS 4200h, a place of the file gone to | DS on the segment of the text |  | DS on the main data |
| `V13_00`, `view.c` | DOS 3Eh, the file closed | DS on the segment of the text. DX as the key before left it | DX the address of `TmpBuf` | DS on the main data, DX 0 |
| `Aexec0`, `view.c` | BIOS 03h, the shape of the cursor | BL the column that the box begins in |  | BL 0 |
| `Aexec0`, `view.c` | DOS 4Ah, the block of the memory made shorter, 4B00h, the viewer run, and 4Ah twice, the block made longer | 4Ah: DX as the code or DOS before left it; the third time AL the low byte of the error of the second, 8. 4Ah the first time and 4B00h: CX, SI and DI as the code before left them | 4Ah: DX the address of the viewer's name. 4Ah the first time and 4B00h: CX, SI and DI 0 | AL and DX 0 at 4Ah. CX, SI and DI as they come |
| `F34_11`, `view.c` | DOS 3Fh and 3Eh, the extension file read and closed, and 4Eh, the viewer looked for | 3Eh: CX and DS:DX the count and the buffer still. 3Fh and 4Eh: SI and DI, and at 4Eh BX, as the code before left them | 3Fh and 4Eh: SI and DI 0, and BX 0 at 4Eh | CX and DX 0 at 3Eh. SI, DI and BX as they come |
| `ChkMode`, `window.c` | BIOS 0Fh, the mode | BX as the caller has it |  | BX 0 |
| `Cursor`, `window.c` | BIOS 0Fh and 02h, the cursor placed | 0Fh: AL the column, DX the place. 02h: AL the mode, as 0Fh left it |  | AL 0 at both, DX 0 at 0Fh |
| `ChgCur`, `window.c` | BIOS 01h, the shape of the cursor | DX where the cursor is, as 03h left it |  | DX 0 |
| `GetCur`, `window.c` | BIOS 03h, where the cursor is | AL the mode, as 0Fh left it |  | AL 0 |
| `Scroll`, `window.c`, for its seven callers | BIOS 06h and 07h, a window scrolled | BL the column of a place that the caller has |  | BL 0 |

What is known to read such a register, and what the portable C hands
it:

- AL of DOS 4Eh, the first file of a search. DOS's APPEND, where it is
  installed, goes by it. The original sets AH alone for the call, at
  every one of its ten searches, and AL is what the code before left
  there: the number of the drive in `ReDr00` of `read.c` and in
  `F21_10` of `enter.c`, 5Ch in `GetVol` of `read.c`, 2 in `ReadTre` of
  `treebox.c`, the last character of the path in `DirFunc` of
  `dirfunc.c`, what the keys left in `FindFil` of `find.c`, a character
  of the extension file in `F34_11` of `view.c`, and 0, as it happens,
  in `C30_00` of `copy.c` and in `FncMs1` of `dirfunc.c`, where the
  code before clears it for something else, or in `FncMs1` what
  deleting by a mask has just answered, 0 or 1. The C translation handed
  over the original's leftover at three of them, the two of `read.c`
  and the one of `treebox.c`, and 0 at the others, of its own making.
  `FindFirst` hands over 0 at all of them.
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
- BL of BIOS 1112h, the small font loaded, which says where the font
  goes among those that the display keeps. The original does not set it
  for the call: it has 0 there from clearing the command line, at the
  start of the command, if the mouse driver and the BIOS leave BX as
  they find it at their calls between, as they are to.
  `ScreenSmallFont` hands over 0. The C translation set BX to 0 at the
  first of the BIOS's calls and kept it as the BIOS left it.

Nothing else that the functions leave out is known to be read by a DOS
or a BIOS.

At one place the original reads such a register back itself. `ReDr00`
of `read.c` reads the extension file of a directory and closes it, and
where either failed it looks in AL, behind the close, for the error of
a drive that is not ready, 53h, to have the panel read again. Where
reading failed and closing did not, DOS has answered nothing in AX.
FreeDOS leaves AX as it finds it when it has closed a file, tried on
the bench with a program that closes a file with 3E53h in AX and writes
what comes back, 3E53h; so there the original finds in AL what it
handed over, the low byte of the read's error, and marks the panel
exactly when the read failed for a drive that was not ready. The
portable C says that: it marks the panel if the read failed with 53h,
by the read's own answer, whatever closing leaves in AX. On a DOS that
does change AX when it closes a file the original goes by what that DOS
leaves, and does not mark the panel unless that happens to be 53h; the
portable C marks it there too.

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

### 7. No map of the memory, and no watch on the programs that stay in it

**What the original does.** Alt+F5, Memory info of the Commands menu,
shows the map of the memory in a window: a line for DOS, for every
program and for every stretch of free memory that DOS's chain of blocks
has, with the segment that it begins at, the number of its blocks, its
size, its name and the interrupts that point into it. The key bar has
"Memory" for F5 while Alt is held, and the help a page for the map and
one for the item of the menu.

For the map the original keeps a watch on DOS, if its setup says so,
which is "TSR manager" of the original's setup program and is on in the
program as it is distributed. It takes INT 21h and INT 27h when it
starts. Of every program that is run while it is in the memory (AH 4Bh)
it notes the name and all the interrupts as they are then; and when a
program stays in the memory, by DOS's call for that (AH 31h) or by INT
27h, it makes the program's block longer and writes a record behind the
program there: its PSP, its name, and every interrupt that has changed
since the program was run, with where it went before. It keeps a list
of up to 18 such programs. One more does not stay: the bell sounds, the
interrupts are put back as they were, and DOS is told to end the
program instead. A block of memory given back (AH 49h) takes its
program off the list.

In the map a program of that list has a mark. It can be selected, by
Ins, Grey + and Grey - or by the right button of the mouse, and
released, by Enter, by the button Release or by two clicks on its line:
its interrupts go back to who had them before and its blocks are given
back to DOS, with DOS told to look in the upper memory too for that
moment. A program is released only if no program that came after it and
stays has taken an interrupt that it took. When the program is left, or
DOS ends it, it releases every program of its list first, and gives INT
21h and INT 27h back.

The same entry of INT 21h takes the program's own call (AH 2Bh, CX
'VC', DX 'OM'), by which its setup program reads and writes its setup
in the memory. With the watch switched off the original does not take
INT 21h, and nothing answers that call.

**What the portable C does.** It has no map of the memory and keeps no
watch.

- Alt+F5 is a key that has no command, and does nothing. The Commands
  menu has no line "memory Info": it has twelve lines where the original
  has thirteen, Directory sizes stands a row further up with what is
  below it, and the box is a row shorter. The letter I chooses nothing
  there, and Alt+F5 nothing in any open menu. The key bar has no name
  for F5 while Alt is held.
- The program does not take INT 27h, notes nothing of the programs that
  are run, and writes no record: a program that stays in the memory
  keeps the block that it asked for, as long as it asked. Any number of
  them may stay, and none is ended for being one too many.
- Nothing releases a program that has stayed. It is still in the memory
  when the program is left, as it is when it was run from DOS's own
  command line, and what gets it out again is whatever does there.
- The program takes INT 21h all the same, for its own call, which
  answers as before (see 1), and takes it whatever the setup has for
  the watch. Every other call of DOS is handed on untouched, with no
  register changed, nothing of the caller's stack used and the flags of
  the comparison that told it from the program's own. That now holds
  for a program run, one that stays and a block given back as well,
  which the original hands on from its watch with other flags, having
  used the caller's stack.
- When DOS ends the program, it goes on in the program that started it
  with the registers as the program leaves them, which are others: DS:DX
  is where INT 21h went before, where the original has where INT 27h
  went, and CX and SI are as they came, where the original has 0 in CX,
  its count of programs run down, and in SI twice that count as it was
  when the last was released.
- The byte of the setup that switched the watch, `ChkTSR`, is kept as it
  is found and written back as it was, 1 in a setup made of the
  defaults; the program does not go by it. A setup file of the original
  is one of the portable C and the other way round. The original's
  setup program still shows TSR manager and writes the byte, and so will
  the setup of the portable C, which is that program (see 8), until the
  setting is taken out there.
- The original's help file, which the portable C reads, still has its
  pages on the map; no key and no item of a menu leads there.

Gone are `memory.c` and `memory_t.c`, all of the map; and of
`resident.c` the list, `Counter` and `PrgList`, the records and what
made and read them, `ExeProg`, `AddProg`, `FreeBlk`, `ChkBlk` and
`Release`, the entry of INT 27h, `New27`, with the interrupt function
`Int27` behind it, the interrupts noted, `IntTab`, `SavePtr` and
`NameBuf`, and `GetStat`, which noted how DOS looks for memory before a
program was released. The entry of INT 21h, `New21`, is the three
comparisons that know the program's own call and two jumps, and
`Int21`, the interrupt function that it goes on in, answers that call
and returns. What is left for two things that are not the map: `SetStat`
with 0, by which the program has DOS look for memory in the lowest block
first when it starts, as the original does; and the prompt that is
written between two commands, which the original makes where its watch
notes the interrupts and the portable C in a buffer of its own.

**Why.** It only made sense on DOS. The map is of DOS's chain of blocks
and of the table of interrupts of a PC, and a program that stays in the
memory by taking an interrupt is a thing of DOS, which had no way of its
own to see such a program or to get it out again. A system that the
program is carried to has both, and gives a program's memory back when
it ends.

**Which scenarios show it.** Each of these is held against a golden of
the portable C's own:

| Scenario | What changes in it |
|---|---|
| `menu-items` | Memory info is not chosen from the Commands menu: the two steps that choose it and leave the map are left out. Every screen is as the original's after the same step |
| `mouse-menu`, `mouse-drag-menu` | The Commands menu where it shows, a line shorter, without Memory info. In `mouse-menu` the click that is on a dark item of the original's menu, Compare directories, is on the line across the menu below that item: nothing is chosen either way, and the screen is the same |
| `tree-choose` | The key bar with Alt held, twice, has no name for F5, where the original has Memory |
| `handed-on` | The flags that three calls of DOS come on with from the program, to a program that has put itself behind it: a program run, 0002h for 0246h, one that stays, 0016h for 0202h, and a block given back, 0016h for 0002h and for 0006h. They are those of the call compared with the program's own, 2Bh, as for every other call, and with interrupts as the call found them, off. The other calls come on as from the original, and the stack as it was |
| `mouse-middle` | The middle button is not pressed in the map of the memory: two steps are left out. Every screen is as the original's after the same step |
| `mouse-drag-lists` | The last nine steps are left out: a program that stays in the memory, and a button of the mouse moved in the map. Every screen before is as the original's |

And `menu` and `ended`, which the first deviation changes too: their
rows there say all that differs in them.

A step that passes through the map on the way to something else is
left out of its scenario for the portable C, with the steps that leave
the map again: there Alt+F5 would do nothing, and Esc or a click behind
it would be a key at the panels. `harness/deviations.py` has them,
scenario by scenario.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory-map`, `memory-map-russia` | They show the map of the memory: its lines, its cursor and its numbers, and a program that stays selected and released there |
| `mouse-memory` | It is the mouse in the map of the memory |
| `stay` | It has programs stay in the memory, by INT 27h and by DOS's call, to be listed in the map, released there, counted to the 18 that the list holds and beyond, and released when the program is left. What it has besides, a program that asks the program for its data by the program's own call, is a scenario of its own now, `asked`, which the original, the C translation and the portable C are all taken through; the portable C passes it against the original's golden |

**In the data.** The test `portable_c-data` holds eight tables against
the original's with this difference declared for each, in `DECLARED` of
`harness/datacheck.py`: of the menus their lines, the pages of their
help, the size of the Commands menu and where the lines and the pages of
the Options menu begin, the commands of the Commands menu, and the keys
that choose an item in an open menu, which have Alt+F5 no more and the
items of the Commands menu below Memory info a place further up; and of
the key bar the names of the keys, without "Memory", and the two rows
that had it for F5 with Alt.

### 8. The setup program is a part of the program

**What the original does.** It is two programs. `VCSETUP.COM` is the
setup program, of 24 KB, with a copy of every routine that it shares
with the main program: boxes, dialogs, the key bar, the mouse. A user
runs it from DOS, or from the command line of the main program like any
command: the main program writes the prompt and the command into the
row of its command line, notes the command in its history, gives the
screen back as DOS had it, and has COMMAND.COM run it. When the setup
program starts,

- it looks at the version of DOS, "This program requires DOS 3.20 or
  later.", and at the memory, "Program too big to fit in memory", sets
  the screen to text of 80 columns if it is not, and writes a banner
  through DOS: its name with "Setup", the version, the copyright;
- it finds its directory, from `VC=` of the environment or else from the
  path of its own file, and reads its seven pages of the help file there,
  "Error reading data from the help file." if it cannot;
- it reads the setup file, `VC.INI`, of that directory, calling DOS
  itself: with a drive that is not ready, DOS's own question comes up,
  whether to try again, since the setup program takes no critical error;
- it has DOS not look for Ctrl-C at every call, takes Ctrl-C and
  Ctrl-Break, asks for the country, and finds out about the screen, as
  the main program does when it starts.

What it edits is what it read from the file, or, without a good file,
the setup that it makes itself. The setup of a main program that is
running is not looked at. When it is left with Save it writes the file
and then asks a running main program for the address of its setup, by a
call of DOS that the main program answers (INT 21h with AX 2B02h, CX
'VC', DX 'OM'): the answer comes only from a main program whose setup
has TSR manager on, as it has when the program is distributed. Into that
setup it copies the 330 bytes that it has saved, but for four that the
running program keeps as it has them: which panel is the current one,
whether the other is switched off, TSR manager, and Memory allocation,
which the running program takes when the command has ended. Then it
ends, and the main program goes on as after any command: it looks at
the screen anew, reads its panels again, and has no setup that it knows
to be saved.

The setup that the setup program makes without a good file, and for
Set default configuration, has two panels as well as the colours and
the options. It copies the 88 bytes that the file has of a panel from a
table of 24, so the other 64 are what stands behind that table in the
program: the box that says the setup file is missing, with the addresses
that it names, and the start of a dialog that only the binary has which
was shipped, not the published source. They land behind the zero byte
that ends the path of each panel, where no program reads them, and are
in the setup file when it is saved.

**What the portable C does.** It is one program, and the setup is a part
of it.

- The Options menu has a line that the original has not, " seTup...",
  behind " Editors...", which the letter T chooses. The lines from Auto
  menus to the clock stand a row further down for it, with their ticks,
  and Ctrl-B, the key bar, is the tenth item and not the ninth. The item
  has no page of the help: F1 on it shows the index.
- Choosing it does what running a command does, without a command: the
  lists of the panels are cut down to 256 files, the screen is what DOS
  had shown, the row of the command line is emptied, where the original
  has its prompt and the command of the setup program there, and the
  cursor goes on to the next row, which moves the screen up if that row
  was the last. Nothing is noted in the history. The program keeps the
  two interrupts that it has taken.
- Then the setup is on the screen as the original's setup program is.
  There is no word on the version of DOS or on the memory, no mode set
  and no banner: the program has started already. The directory is the
  one that the program has, which it makes by its own rule: in capitals,
  and with a backslash behind whatever `VC=` names, where the setup
  program adds none behind a colon. The help file is not read when the
  setup is entered.
- The setup file is asked for as the rest of the program asks for its
  files, with the program's errors (see `sys.h`): a drive that is not
  ready has the program's box, a call that was given up there is the
  error 53h, and DOS is asked what was wrong behind a call that failed.
  The box is the main program's, with its own key bar, also while the
  setup is open.
- What is edited is what was read from the file, as in the original, in
  the place of the program's own setup, which is set aside meanwhile:
  the routines that both programs have go by what is edited, so the
  colours, the key bar and the clock of the setup are those that are
  edited, as in the original.
- Left with Save, the file is written, and the program keeps what was
  edited, but for the four bytes that the original's running program
  keeps: it takes the setup whatever that has for TSR manager, and no
  call of DOS is made for it. Left without, the program has its setup
  back as it was. Either way it goes on as after a command: the screen
  is looked at anew, which moves it up as after a command, the panels
  are read again, and what stood in the command line is gone.
- The setup that is made without a good file, and for Set default
  configuration, has zeros in the 64 bytes behind the path of each
  panel. A setup file that the setup saves of it differs from the
  original's in those bytes, twice 64 of the file's 511.
- While the setup program is being translated (`TODO.md`, "The setup
  program in the portable C: the plan"), what its menu leads to is not
  all there: an item whose screen is still to come waits for a key and
  goes back to the menu, the mouse does nothing at the menu, and F1 is
  no key.

**Why.** The user's decision, on 12 October 2026: the setup program is
ported into the portable C "so that there is one program", reached by an
item of its Options menu and judged by scenarios whose goldens are
recorded from the original setup program. It is translated as it is
first: what is behind it on the screen is what DOS had shown, as when
the original is run from the program's command line, and it edits the
setup file, not the setup of the running program. Its start and its end
are those of a program of its own, and what they did that the one
program has done already is not done twice. The critical error is the
program's, and the files are asked for as the rest of the program asks,
on the user's word of the same day. And the 64 bytes behind a path are
zeros because half of them cannot be anything else: 17 of each 64 are of
a dialog that the published source has not, which the project does not
reproduce, and the other 47 are a box with addresses of the original's
layout, which mean nothing in a file.

**Which scenarios show it.** Every scenario of the setup program is held
against the golden that the original's setup program left, with two
things otherwise, which `harness/deviations.py` has. The portable C is
taken through the item of its menu, "F9 o t", where the original is
taken through the command, "d:\vc\vcsetup>nul" and Enter. And the row
that has the command in the original, behind the setup and afterwards
for as long as it is on the screen, is empty in the portable C: the
golden is derived with spaces there. The scenarios `setup-menu`,
`setup-defaults`, `setup-missing`, `setup-damaged` and `setup-cancel`
pass so; `setup` is not run until the screens that it shows are there.

No scenario shows the 64 bytes. `setup-defaults` saves a setup that was
made by Set default configuration, starts the program anew with it and
ends with the setup file as that program saves it, where the bytes
behind a path are zeros in the original too: the file that the setup
program itself has written is not on the disk when the scenario ends.
No scenario has a drive that is not ready while the setup is open, a
`VC=` in the environment, or a look at the history.

The Options menu shows in three scenarios that have a golden of the
portable C's own for the first deviation already, `clock`, `clock-russia`
and `menu`: their rows there say what differs.

**In the data.** The test `portable_c-data` holds four tables of the
menus against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages
of their help, the commands of the Options menu, where the item is the
number of a command that the original has not, and the keys that choose
an item in an open menu, where Ctrl-B chooses the tenth. The constants
of the setup are held against those of the original's setup program,
`VCSETUP.COM`, each by its name with `St` in front; the table of a
panel's defaults is the 24 bytes that the published source has.

### 9. The key bar of the setup is 80 cells, not 81

**What the original does.** Its setup program has a copy of the routine
that draws the key bar, `PutKey0`, which differs from the main program's
in one place. Both write, for each of the ten keys, its number, the six
cells of its name, and a space before the next number. The main
program's looks whether the key was the tenth before it writes that
space; the setup program's writes the space first and looks then. So
the setup program's bar has a space behind the name of its tenth key
too, an 81st cell in a row of 80, in the colour of the bar's numbers.

The bar is drawn into the last row of the screen buffer, and that row
alone is put on the screen, so the cell does not show there. It is the
first cell of what stands behind the row: of the next row of the
buffer, on a screen of fewer lines than 50, which no key bar is put on
the screen from; and on a screen of 50 lines of what stands behind the
buffer, which is the setup program's copy of what DOS had on the
screen. That copy is put back on the screen when the setup program is
left, and where one of its screens is: the top left cell of what DOS
had shown is a space then, in the colour of the key bar's numbers.

**What the portable C does.** It has one routine for both, the main
program's, which writes 80 cells: `PutKey0` of `bars.c`. While the setup
is open it draws the setup's key bars, which are tables that it reads
then, and nothing else is otherwise. What DOS had shown is put back as
it was.

**Why.** The user's decision, on 13 October 2026: the cell is a bug of
the original's setup program, and is fixed, not kept, "taking into
account that we are going to merge vc and vcsetup code". The setup uses
the main program's routines, and a second copy of one is not kept for a
bug of it.

**Which scenarios show it.** None so far: the scenarios of the setup
are on a screen of 25 lines, where the cell lands in a row of the
buffer that is not on the screen. One that has the setup on a screen of
50 lines, which is still to come (`TODO.md`), shows it in the top left
corner of what DOS had shown, and is held against a golden of the
portable C's own for that.
