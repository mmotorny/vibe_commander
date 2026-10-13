# Deviations of the portable C

The portable C is judged as the C translation was: by the scenarios, with
goldens recorded from the original, Volkov Commander 4.05. Where the
portable C is to behave otherwise than the original, that is a deviation,
and every deviation is declared here, before anything else says so:

- what the original does,
- what the portable C does,
- why,
- and which scenarios show it.

What a scenario is expected to show where a deviation changes it is the
original's golden with the declared difference made to it, and nothing that
was recorded from the portable C. The difference is made in one of three
ways. Steps that choose what is gone are left out, with their screens. What
is otherwise on many screens is a rule of the comparison. And the rows that
one scenario shows otherwise are its patch: a file of those rows, each as
the original's golden has it and as the portable C is expected to show it,
with the entry here that it is of and why. A patch is written by whoever
declares the deviation, is kept apart from the goldens of the original, and
is made to the golden each time that is read, so the golden derived again
gives the expectation again. A patch that no longer fits its golden fails
the tests: that is looked at, and the patch is not made again to fit.

A feature that only made sense on DOS is removed, not kept as a setting
that does nothing, and such a removal is declared here and made without
asking. Any other deviation, the fix of a bug of the original included, is
asked first. A bug of the original stays as it is until its fix has been
decided.

What the C translation, `src/c/`, already does otherwise than the original
is no deviation of this tree: it is in `harness/deviations.py`, and the
portable C began with it.

What the harness goes by of this, the steps that are left out, the rules
and the scenarios that are not run, is in `harness/deviations.py`, under
`TREES`. The patches are in `harness/goldens/portable_c/`, `NAME.patch` for
the scenario `NAME`, and the top of `harness/goldens.py` says how one is
written; the program of 32 bits has those, and in
`harness/goldens/portable_c32/` its own for the scenarios that it is to
show otherwise than the program of 16 bits, each in the place of the
other. `python3 harness/goldens.py changes --tree portable_c` prints what
each says, with the cells that differ marked. `python3 harness/goldens.py
patch --tree portable_c NAME` writes one from a run of the build, which is
where its rows come from and not what makes them right: its author reads
every row against the entry here, fills in the header and commits it, and
the tests then hold the program to the patch, not to another run. A patch
names the entries here that it is of, and each of those entries has to name
the scenario: the test `portable_c-scenarios` fails otherwise. It fails too
for a scenario that is not run and that no entry here names.

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
  it, stands a row further up for that. The letter M chooses another
  thing there, the setup, for which the menu has two lines at its
  bottom that the original has not: see 8. Shift with Enter is Enter.
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
- The Info panel, which said how much memory a command has while the
  portable C had the line, all that is behind the panels, says nothing
  of the memory since: see 11.
- The map of the memory, which listed what stays as the program's, is
  gone from the portable C since: see 7.
- The program's own call, which handed out the main data and the
  panels themselves while the portable C had it, is gone from the
  portable C since, with the setup program made a part of the program:
  see 8.
- The byte of the setup file that held the switch is kept as it is found
  and written back as it was; the program does not go by it. A setup file
  of the original is one of the portable C and the other way round.
- The setup has no box for it either. The original's setup program,
  `VCSETUP`, has the switch in its Configuration as well, the first of
  three boxes of Execute options, and writes the byte. In the setup of
  the portable C, which is that program (see 8), the box is gone: Quick
  execute commands stands in its place, a row further up, and with TSR
  manager gone from below that (see 7) it is the one line of Execute
  options. The Keyboard options stand two rows further up for the two,
  and the box of the dialog is as tall as it was, since its left half
  is: the last two rows of its right half are empty. The boxes go to
  each other as they stand now: Down from Show logical drive B: is Quick
  execute commands, Right from a box of the left half is the box on its
  row or else the one above that row, and Left from one of the right
  half the box on its row. The setup writes the byte into the setup file
  as it read it, and Set default configuration makes it 1, as the
  original's does.
- The original's help file, which the portable C reads, still has its
  page on Memory allocation; no item of the menu leads there. And the
  page of the Configuration that the setup shows for F1 still has its
  paragraph on the box, between those on Show logical drive B: and on
  Quick execute commands: the help file is the distribution's.
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

**The program of 32 bits.** The portable C is built a second way, as a
program of 32 bits that an extender, DOS/32A, starts above the megabyte,
and this entry is of both. What it says above holds for both but for
these, where it is of the program of 16 bits alone and the program of
32 bits is as said here:

- Where the program is. The code, the main data, the stack and the
  panels of the program of 32 bits are above the megabyte, all of them
  while a command runs; below it stays the extender, some 52 KB. So
  the count of what the program costs in memory, above, is the 16-bit
  program's: a command run from the program of 32 bits has of DOS's
  memory all but the extender's part. Of the memory above the megabyte
  it has none, the extender having taken all of that: this is not
  declared here, and is the user's to decide (`TODO.md`, the plan, 9 of
  what is open); the scenario `command` shows it and fails for it.
- The block of the panels. The program of 32 bits has it of 640 KB that
  are its own, above the megabyte, which it hands out to itself as DOS
  hands out its memory; it makes the block shorter for a command and
  longer again afterwards as the program of 16 bits does, since the code
  is the same, and DOS gains nothing by it.
- A second copy. One of the program of 32 bits has the memory to run a
  command, as the original's has: the last three sentences of the
  count, "It is enough for every command" and on, are not of it.
- "Program too big to fit in memory" after a command. The program of 32
  bits never says it there: the memory that the viewer needs is of
  those 640 KB, which no command takes. It says it when it starts and
  DOS has not the 512 bytes below the megabyte that the system's
  functions keep there; with less memory above the megabyte than the
  program wants, the extender does not start it, and says so in words
  of its own.
- The file. That of 32 bits is the extender, a program of DOS, with the
  program behind it as a linear executable, which the extender loads
  and relocates.

Everything else is of both: the menu without Memory allocation, no
check sum and no "Bad or missing", a panel left with 256 files for a
command, the byte of the setup file, the setup without the box, the
help file.

**Why.** The user's decision, on 12 October 2026: giving up the code and
the data and making them again saves memory that only DOS is short of,
at the price of reading the program's file and its directories after
every command, and of a program in two parts with copies between them.
It "only matters on DOS"; "it's OK for our port to be less memory- and
disk-efficient, as long as it's functionally equivalent"; and the option
"in the new tree goes away. It's going to be a pattern for a lot of
features that only made sense on DOS." And on 13 October 2026: where the
program of 32 bits does again what the original does and the program of
16 bits could not, a second copy that has the memory to run a command,
it is held to the original's golden, and this entry says only what still
differs.

**Which scenarios show it.** Each of these is held against the original's
golden with what its row says made to it: by its patch, or, where the row
says that every screen is as the original's, by the steps left out alone.
A row is of both programs unless it says of which; where the program of
32 bits is to show other rows than the program of 16 bits, it has a
patch of its own, in `harness/goldens/portable_c32/`, with those rows
and no others:

| Scenario | What changes in it |
|---|---|
| `lost` | The program's file is deleted by a command. The original asks for it, twice, and ends; the portable C is back at its panels, and the keys that answered the original are keys at the panels: a space into the command line, Enter, which runs it, and Esc, which switches the panels off |
| `clock`, `clock-russia` | The Options menu where it shows: without Memory allocation, so with Save setup a row further up. And by 8: with two rows more at its bottom, a line across the menu and More setup, so a row taller than the original's |
| `menu` | The same. By 8: Up from the top of the menu, and PgDn, go to More setup, the last item, where the original has Save setup, and Up from there goes to Save setup, where the original goes on from that to Memory allocation. And by 7: the Commands menu where it shows, five times, a line shorter, without Memory info, with the items that are dark there a row further up |
| `quick` | The step that switches Memory allocation off before the last command is left out; every screen is as the original's after the same step |
| `second-copy` | In 16 bits: the second copy is started with a command to run and has no memory for it: "There is not enough memory to run this command" where the original shows what the command printed. The rest is as the original's: the copy's panels, the copy left, the first copy at work. In 32 bits: every screen is as the original's, the copy having run its command; its patch is of 10 alone |
| `start-memory` | In 16 bits: the second copy has no memory to run the command that was to leave it without memory for its data, and says so. It is still there afterwards, where the original has said "Program too big to fit in memory" and ended: the panels of the last three screens are the copy's, with the mini status that the first copy's have not. In 32 bits: the copy runs the command, as the original's does, which stays in the memory and leaves 75 KB of DOS's; the copy needs none of that to come back and says nothing, so the command line shows a row less of it, and it is still there afterwards, with the same three screens |
| `setup-config` | The Configuration of the setup where it shows: without Memory allocation, and by 7 without TSR manager. In the right half of its box, Quick execute commands is a row further up, the frame of Execute options ends below it, the Keyboard options are two rows further up, and the last two rows are empty. The two steps that untick Memory allocation and TSR manager are left out; Quick execute commands is ticked where the cursor is when it comes down from the left half, by Space where the original goes down a line first. Left from Clear keyboard buffer is Error sound, the box of its row, where the original has Show logical drive B: there. And the setup file that is saved has the two bytes as it had them, 1 and 1, where the original's has 0 and 0, with another sum for that: three bytes of the file |
| `setup-config-mouse` | The Configuration so. The click on Quick execute commands is a row further up and those on Alt alone selects menu two rows, and the mouse pointer with them. Every box is ticked as in the original, and the setup file that is saved is the original's |
| `setup`, `setup-missing-saved`, `setup-clock`, `setup-left-handed`, `setup-help-screens` | The Configuration so, where each shows it; under the help of `setup-help-screens` nothing of its right half shows. In the Configuration the steps are the original's, since none goes into its right half. All but `setup-missing-saved` show Screen and Mouse too, which is otherwise by 5 |
| `ended` | In 16 bits the program that the second copy was to leave in the memory before DOS ends it does not run, for want of memory. In 32 bits the copy has the memory and runs it, and it stays in the memory, 448 bytes, also when DOS has ended the copy, which by 7 keeps no list of such programs to release: no screen of the scenario shows that, since the program that started the copy clears the screen, and both programs are held to the one patch. And by 7, for both: the copy would have none to release in any case, and the registers that the program which started it prints are as the portable C leaves them, both times: CX and SI as they went, 2222h and 1111h, where the original has 0 and, the second time, 0002h; and DS:DX no longer where INT 27h goes, which the original has as 0 and 0 from there. And by 8, with no interrupt of DOS's taken: DX has in its low byte what DOS was last told of Ctrl-C and is as it came above that, which is what the original leaves there where its setup has TSR manager off; and DS is as it came, on the program that started the copy, a segment that follows the size of the program and is left out of the comparison, in `harness/deviations.py`. The look at the map of the memory that ends the scenario, two steps, is left out |

A step that switches Memory allocation, "F9 o m", is left out of every
scenario for the portable C: there it would enter the setup (see 8).

On every screen that shows one of the two dialogs with the mouse pointer
where the setup put it, in the middle of the screen, the pointer is over
a line that has moved: the cell whose colours it exchanges holds another
character, and the cell that the original has there is two rows further
up, or five in Screen and Mouse, in the colours of its line. The patches
have both.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory` | It switches Memory allocation off, for the Info panel to show the other number, and shows the menu without the mark |
| `passed-on` | It has a program stay in the memory, starts a second copy behind it and runs commands from that, to see what the copy hands on of the calls of DOS that pass it. The bench has no memory for a second copy of the portable C of 16 bits behind a program that has stayed: COMMAND.COM says "Allocation of DOS memory failed." And the portable C does not take INT 21h any more, in 16 bits or in 32, so no call of DOS passes it and it has none to hand on: see 8, where `handed-on`, which sees the same with one copy, is not run for that |

**In the data.** The test `portable_c-data` holds five tables of the menus
against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages of
their help, the size of the Options menu, its commands, and the item that
Shift-F9 chooses in it. And of the setup's Configuration, for this and for
7 together, the dialog, `StConf_B`, which is held against the original's
without the two fields and with the fields that each of the others goes
to and the rows of those that have moved as they are now; and eleven
lines of its box, `StCnf02_S` to `StCnf12_S`. A line of that box is one
text for the two halves: each of the nine that have something on the
right is held against the original's line of that name as far as its left
half goes and from there against the right half of the original's line
that stands there now, and the two that have nothing there against their
left halves, with spaces declared behind. The box itself, `StConf_P`, is
the original's.

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
original is one of the portable C and the other way round.

The setup has no choice for it either. The original's setup program has
it in Screen and Mouse, the group Screen output at the top of the right
half of the box, and writes the byte. In the setup of the portable C,
which is that program (see 8), the group is gone: the Other options,
Zooming boxes and the line of the screen saver's characters, stand in its
place, five rows further up, and the Mouse options below them. The box
is as tall as it was, since its left half is, and the right half is
empty from there down. The fields go to each other as they stand now:
Right from Screen colors is Zooming boxes, and from Screen blank delay
Left-handed mouse, the last of the right half, which ends above that
group; Up from Zooming boxes, the first of the right half now, is the
last line of Screen blank delay, as it was from Screen output; and Left
from Zooming boxes and from the line of characters is Screen colors, on
its first line, where the original has Screen blank delay to the left of
both. The setup writes the byte into the setup file as it read it, Set
default configuration makes it 2, as the original's does, and nothing is
done about the display when the dialog is left, where the original's
setup program tests it anew. The page of the help that the setup shows
for Screen and Mouse still has its paragraph on Screen output, between
those on Screen blank delay and on Zooming boxes: the help file is the
distribution's.

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

**Which scenarios show it.** That the program does not wait, none. The
bench has a VGA. `start-setup`
has a setup with Snow mode 0, so the original tests the display there,
and finds nothing: QEMU's VGA turns the bit over every time it is read,
so the instruction that sets the program to waiting, and the 20 of
`Window` and `MovScr` that wait, ran in no scenario and are listed as
out of reach in `harness/ported.py`. In that scenario the portable C
does not test, shows the same screens and leaves the same setup file,
with the byte as it found it.

That the setup has no Screen output, these, each held against the
original's golden with what its row says made to it, by its patch and by
the steps that `harness/deviations.py` has for it:

| Scenario | What changes in it |
|---|---|
| `setup-screen` | Screen and Mouse of the setup where it shows: without Screen output. In the right half of its box the Other options and the Mouse options are five rows further up, and the rows below them are empty. The two steps that choose a line of Screen output, by Space and by a click, are left out. The others get to the same fields by the keys that lead there now: Right from Screen colors is Zooming boxes at once, where the original goes down through Screen output; Up from Zooming boxes is the last line of Screen blank delay, where the cursor then is, and from there Right and Up is the line of characters; Left from that line is Screen colors, where the original's cursor is on Screen blank delay, and 20 minutes is three lines further down from there; the line is three times Tab from the first field, and Zooming boxes twice Up from Left-handed mouse. The clicks on the right half are five rows further up, and the mouse pointer with them. And the setup file that is saved has the byte as it had it, 2, where the original's has 1, Asynchronized access, with another sum for that: two bytes of the file |
| `setup-options-russia`, `setup-help-screens` | Screen and Mouse so, where each shows it; under the help of `setup-help-screens` nothing of its right half shows. The line of characters is three times Tab from the first field. The setup file that `setup-options-russia` saves is the original's |
| `setup-left-handed` | Screen and Mouse so. The click on Left-handed mouse is five rows further up, and the mouse pointer is there, on the dialog and on the menu behind it. The setup file that is saved is the original's |
| `setup`, `setup-delay-file`, `setup-clock` | Screen and Mouse so, where each shows it. The steps are the original's |

`setup`, `setup-clock`, `setup-left-handed` and `setup-help-screens` show
the Configuration too, which is otherwise by 1 and 7: the patch of each
names the three entries. Where the mouse pointer is in the middle of the
screen, it is over a line that has moved: see 1.

**In the data.** The test `portable_c-data` holds the dialog, `StScrn_B`,
against the original's with this declared, in `DECLARED` of
`harness/datacheck.py`: without the field of Screen output, and with the
fields that each of the others goes to and the rows of those that have
moved as they are now; and twelve lines of its box, `StScr01_S` to
`StScr12_S`, each against the original's line of that name as far as its
left half goes, and from there against the right half of the original's
line that stands there now, five further down, or with spaces declared
where there is none. The box itself, `StScrn_P`, is the original's.

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
it went. A call that the portable C makes no more has no row: DOS
asked where INT 21h goes, when the program starts, went with the
program's own call (see 8), and the five calls of the BIOS in `F69_00`
of `commands.c`, the font asked about, the mode set, the small font
loaded, Print Screen told of the rows and the font asked about again,
went with EGA lines (see 12).

| Routine | Call | The original | The C translation, where it differed | The portable C |
|---|---|---|---|---|
| `Blank`, `blank.c`, with its `Star` | BIOS 0Fh, 03h, 01h, 02h and 09h: the cursor noted, hidden and put back, and a star written | 0Fh: AL and BX as the code before left them, for a star that changes its look and its clock and kind. 03h, 01h and 02h: AL the mode, as 0Fh left it. 01h: BH the page, and DX where the cursor is or was put. 02h: BL and CX as the code before left them, for a star its clock and a count of the stars. 09h: DX the place of the star |  | AL 0 at all but 09h; BX, CX and DX 0 where the call takes nothing in them |
| `M02_10`, `commands.c` | DOS 3Bh, the directory of a panel made current | AL the drive, 1 for A: |  | AL 0 |
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
| `Exit00`, `resident.c`, and `Exit05`, `dos.c`, which is the original's `Exit02` where DOS has ended the program | DOS 3301h, Ctrl-C looked for as before, and 4Ch, the program ended | 3301h: DH as the code or DOS before left it. 4Ch: DS:DX on where an interrupt went before the program took it, of giving it back, where the watch on DOS is on | DS 0 at 3301h | DH 0 at 3301h; DS on the main data at both, and DX 0 at 4Ch |
| `Exec02`, `resident.c` | DOS 3301h, Ctrl-C looked for as before; 4B00h, COMMAND.COM run; 19h and 47h, the current drive and its directory for the prompt; BIOS 0Eh, the end of the line typed, twice | 3301h: DH as the code before left it. 4B00h: CX the length of the command's tail, with the slash above it; SI on the tail and DI on the name COMMAND.COM. 19h: AL 0Dh, of the start of the prompt, and DX as the command or DOS left it. 47h: AL a colon, of the prompt. 0Eh: CX and DX as the code before left them | CX and DX 0 at 0Eh | DH 0 at 3301h. CX, SI and DI as they come at 4B00h. AL and DX 0 at 19h, AL 0 at 47h. CX and DX 0 at 0Eh |
| `Exec02`, `resident.c`, with Quick execute commands | INT 2Eh, a command handed to the COMMAND.COM that is in the memory, which takes it at DS:SI | AX 3300h, of Ctrl-C just set and the zero that the end of the command was looked for by; CX the length of the command, DI on the carriage return at its end; DL what DOS was told of Ctrl-C; ES on the program's segment, as DS |  | DX the segment of the command, which is DS; AX, BX, CX, DI and ES as they come |
| `SetStat`, `resident.c` | DOS 5801h and 5803h, how DOS looks for memory | CX and DX as the code before left them |  | CX as it comes, DX 0 |
| `Alloc3`, `resident.c`, the original's `Alloc0` from that label on | DOS 30h, the version, and 4Ah, the block of the program made shorter | 30h: AL as taking the check sum of the file left it, which the portable C does not take. 4Ah: AL the minor number of the version; CX and DX how much of itself the original then moves, in bytes and in paragraphs |  | AL 0 at both, CX as it comes and DX 0 at 4Ah |
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
| `StRdHelp`, `sthelp.c` | DOS 3Dh, 3Fh and 3Eh, the pages of the setup's help read from the help file | 3Dh: BX, SI and DI as finding the directory and copying the name left them, CX 0, of that copy. 3Fh, five times: AL the low byte of the place in the file that was just gone to, or of what was worked out since, the room for the index or the bytes between two pages. 3Eh: AL as the call before left it, the low byte of what was read where all went well; CX and DS:DX the count and the buffer still, or what the look at the header left |  | BX, SI and DI as they come at 3Dh. AL 0 at 3Fh and 3Eh; CX and DX 0 at 3Eh |
| `StDefault`, `stmain.c` | DOS 19h, the current drive, for the panels that the setup makes without a setup file | AL as the code before left it: what DOS answered about the setup file, or 0, the answer of the box that asks |  | AL 0 |
| `StInit15`, `stmain.c` | DOS 3Dh, 3Fh and 3Eh, the setup file read | 3Dh: BX, CX, SI and DI as the help file read and the name copied left them. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  | BX, CX, SI and DI as they come at 3Dh. AL 0 at 3Fh and 3Eh; CX and DX 0 at 3Eh |
| `StExit`, `stmain.c` | BIOS 01h, the cursor in its shape again; DOS 4300h, 3Ch, 40h and 3Eh, the setup file written; 09h, the line that says it could not be | 01h: DX the rows of the screen and 80, of the screen put back. 4300h: BX the sum of the file. 3Ch: AL as 4300h left it, or 0 for a file that is not there; BX the sum still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still. 09h: AL, BX and CX as the calls before left them |  | DX 0 at 01h. AL 0 at 3Ch, 40h, 3Eh and 09h; BX as it comes at 4300h and 3Ch; CX and DX 0 at 3Eh; BX and CX as they come at 09h |
| `StSetup`, `stmain.c` | BIOS 0Fh and 03h, the page and the cursor, where the setup notes the row and the shape of the cursor | 0Fh: AL and BX as making the table of the small letters left them. 03h: AL the mode, as 0Fh left it; BL, CX and DX as the code before left them |  | AL 0 at both; BX 0 at 0Fh, and BL, CX and DX 0 at 03h |
| `StF07_10`, `stcolor.c`, of the setup program's `F07_16` | BIOS 06h and 07h, the lines of the list of colours moved by one | BL the column of the lines, 8 |  | BL 0 |
| `StF07_01`, `stcolor.c`, of the setup program's `F07_57` | BIOS 01h, the shape of the cursor put back when the list of colours is left | AL the low byte of the key that left the list, 1Bh for Esc and 0 for F10, or 0 behind the mouse; BX and DX where the box of the list is and its size |  | AL, BX and DX 0 |
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
  the mode there, left from the call that asked for it. The original's
  setup program has no mode there either where it puts the shape back
  behind its list of colours, but what the key or the mouse left:
  `StF07_01` of `stcolor.c`.
- CX and DL of BIOS 1130h, the font of an EGA, which a BIOS that does
  not know the call hands back as they came. `ScreenFont` puts 16 and 0
  there, so such a BIOS is taken to have a font of 16 lines and to say
  nothing of the rows. The original has 0 in DL where `IniData` asks
  for the rows, and a leftover in CX, which it does not read there.

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
  menu has no line "memory Info": it has a line less than the
  original's thirteen for that, Directory sizes stands a row further up
  with what is below it, and the box is a row shorter. (It is one more
  line and row shorter since, without EGA lines: see 12.) The letter I
  chooses nothing there, and Alt+F5 nothing in any open menu. The key
  bar has no name for F5 while Alt is held.
- The program does not take INT 27h, notes nothing of the programs that
  are run, and writes no record: a program that stays in the memory
  keeps the block that it asked for, as long as it asked. Any number of
  them may stay, and none is ended for being one too many.
- Nothing releases a program that has stayed. It is still in the memory
  when the program is left, as it is when it was run from DOS's own
  command line, and what gets it out again is whatever does there.
- The program does not take INT 21h either, since. When the watch went
  the entry of that interrupt was kept for the program's own call,
  which the same entry takes, and handed every other call of DOS on
  untouched; it went with that call when the setup program became a
  part of the program: see 8. A call of DOS that another program makes
  goes to DOS, and does not pass the program.
- When DOS ends the program, it goes on in the program that started it
  with the registers as the program leaves them, which are others: CX
  and SI are as they came, where the original has 0 in CX, its count of
  programs run down, and in SI twice that count as it was when the last
  was released; and DS:DX is not where INT 27h went before, which the
  original has there from giving that interrupt back. What the portable
  C has there is in 8.
- The byte of the setup that switched the watch, `ChkTSR`, is kept as it
  is found and written back as it was, 1 in a setup made of the
  defaults; the program does not go by it. A setup file of the original
  is one of the portable C and the other way round.
- The setup has no box for it. The original's setup program has TSR
  manager in its Configuration, the last of three boxes of Execute
  options, and writes the byte. In the setup of the portable C, which
  is that program (see 8), the box is gone, with that of Memory
  allocation: 1 says how the dialog stands without the two. The setup
  writes the byte into the setup file as it read it, and Set default
  configuration makes it 1, as the original's does. When the setup is
  left with Save, the program keeps the byte that it had, as the
  original's running program does.
- The original's help file, which the portable C reads, still has its
  pages on the map; no key and no item of a menu leads there. And the
  page of the Configuration that the setup shows for F1 still has its
  paragraph on TSR manager, behind that on Quick execute commands.

Gone are `memory.c` and `memory_t.c`, all of the map; and of
`resident.c` the list, `Counter` and `PrgList`, the records and what
made and read them, `ExeProg`, `AddProg`, `FreeBlk`, `ChkBlk` and
`Release`, the entry of INT 27h, `New27`, with the interrupt function
`Int27` behind it, the interrupts noted, `IntTab`, `SavePtr` and
`NameBuf`, and `GetStat`, which noted how DOS looks for memory before a
program was released. The entry of INT 21h, `New21`, was left then as
the three comparisons that know the program's own call and two jumps,
with `Int21`, the interrupt function that it went on in, which answered
that call and returned: both have gone since, with the call (see 8).
What is left for two things that are not the map: `SetStat`
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

**Which scenarios show it.** Each of these is held against the
original's golden with what its row says made to it: by its patch, or,
where the row says that every screen is as the original's, by the steps
left out alone:

| Scenario | What changes in it |
|---|---|
| `menu-items` | Memory info is not chosen from the Commands menu: the two steps that choose it and leave the map are left out. Every screen is as the original's after the same step |
| `mouse-menu`, `mouse-drag-menu` | The Commands menu where it shows, a line shorter, without Memory info. In `mouse-menu` the click that is on a dark item of the original's menu, Compare directories, was on the line across the menu below that item while this was all that the menu lacked. The menu is a second line shorter since, and the click is taken to the item where it is now: see 12 |
| `tree-choose` | The key bar with Alt held, twice, has no name for F5, where the original has Memory |
| `mouse-middle` | The middle button is not pressed in the map of the memory: two steps are left out. Every screen is as the original's after the same step |
| `mouse-drag-lists` | The last nine steps are left out: a program that stays in the memory, and a button of the mouse moved in the map. Every screen before is as the original's |

And `menu` and `ended`, which the first deviation changes too: their
rows there say all that differs in them, and the patch of each names
both entries. So it is with the scenarios that show the Configuration of
the setup, which is without TSR manager and without Memory allocation:
`setup-config`, where the step that unticks TSR manager is left out and
the byte is 1 in the setup file that is saved, `setup-config-mouse`,
`setup`, `setup-missing-saved`, `setup-clock`, `setup-left-handed` and
`setup-help-screens`. Their rows are in 1.

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
| `stay` | It has programs stay in the memory, by INT 27h and by DOS's call, to be listed in the map, released there, counted to the 18 that the list holds and beyond, and released when the program is left. What it has besides, a program that asks the program for its data by the program's own call, is a scenario of its own, `asked`, for the original and the C translation; the portable C answers that call no more, and is not taken through it: see 8 |
| `handed-on` | It shows what the program hands on of the calls of DOS that pass it, to a program that has put itself behind its entry of INT 21h. While the portable C had that entry without the watch, it was held there with a patch, for three calls that the original deals with and the portable C handed on like any other. The entry is gone: see 8 |

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

For that call the main program takes INT 21h, DOS's own interrupt, when
it starts, in the entry by which it also keeps its watch on the programs
that stay in the memory (see 7): every call of DOS that any program
makes while the main program is in the memory passes there, and all but
its own and three of the watch are handed on to DOS as they came. It
answers its own with the number of the interface, 42, in AH and an
address in ES:BX: of what it keeps of the first panel for 0 in AL, of
the second for 1, and of its setup for anything else (see 1). It gives
the interrupt back when it is left, and when DOS ends it. With TSR
manager off it does not take the interrupt, and nothing answers the
call.

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

- The Options menu has a line that the original has not, " More
  setup...", which the letter M chooses. It is the last, behind " Save
  setup" and a line across the menu of its own, so every item above it
  is on the row and has the number that it has without it, and the box
  is two rows taller. The item has no page of the help: F1 on it shows
  the index.
- Choosing it does what running a command does, without a command: the
  lists of the panels are cut down to 256 files, the screen is what DOS
  had shown, the row of the command line is emptied, where the original
  has its prompt and the command of the setup program there, and the
  cursor goes on to the next row, which moves the screen up if that row
  was the last. Nothing is noted in the history. The program keeps the
  two interrupts that it has taken.
- Then the setup is on the screen as the original's setup program is,
  but for three settings of what the program has no more, which its
  dialogs do not have: Memory allocation and TSR manager in the
  Configuration (see 1 and 7), and Screen output in Screen and Mouse
  (see 5).
  There is no word on the version of DOS or on the memory, no mode set
  and no banner: the program has started already. The directory is the
  one that the program has, which it makes by its own rule: in capitals,
  and with a backslash behind whatever `VC=` names, where the setup
  program adds none behind a colon. The pages of the help are read from
  the help file there, as the setup program reads them when it starts,
  and nothing is said if they cannot be: "Error reading data from the
  help file." is among what the setup program wrote through DOS at its
  start, and is not shown. F1 then says, as in the original, that the
  help is not available.
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
- The program's own call is gone, since nothing asks a running program
  for its data any more, and with it the last that the program did with
  INT 21h: it does not take that interrupt, has none to give back when
  it is left or DOS ends it, and a call of DOS that another program
  makes goes to DOS without passing the program. A program that makes
  the call all the same, as the original's setup program does when it
  is run from the portable C as a command, is answered by DOS, which
  has AH 2Bh for setting the date and refuses this one: the original's
  setup program has written the setup file by then and takes that for
  no main program in the memory, and the portable C goes on with the
  setup that it has.
- When DOS ends the program, the program that started it goes on with
  the registers as the portable C leaves them: ES on the PSP of the
  program that was ended, as in the original; in the low byte of DX
  what DOS was last told of Ctrl-C, and DS and the high byte of DX as
  they came, which is what the original leaves there where its setup
  has TSR manager off and it has no interrupt to give back; and CX and
  SI as they came (see 7). The flags are the original's.
- The setup that is made without a good file, and for Set default
  configuration, has zeros in the 64 bytes behind the path of each
  panel. A setup file that the setup saves of it differs from the
  original's in those bytes, twice 64 of the file's 511.

**Why.** The user's decision, on 12 October 2026: the setup program is
ported into the portable C "so that there is one program", reached by an
item of its Options menu and judged by scenarios whose goldens are
recorded from the original setup program. It is translated as it is
first: what is behind it on the screen is what DOS had shown, as when
the original is run from the program's command line, and it edits the
setup file, not the setup of the running program. Its start and its end
are those of a program of its own, and what they did that the one
program has done already is not done twice. Where the item is and what
it is called is the user's decision of 13 October 2026: it was put in
the day before as " seTup..." behind " Editors...", chosen by T, which
moved the seven switches below it a row down, and reads better at the
bottom of the menu, where it moves nothing. The critical error is the
program's, and the files are asked for as the rest of the program asks,
on the user's word of the same day. And the 64 bytes behind a path are
zeros because half of them cannot be anything else: 17 of each 64 are of
a dialog that the published source has not, which the project does not
reproduce, and the other 47 are a box with addresses of the original's
layout, which mean nothing in a file. The program's own call goes by
the decision of 12 October 2026 too, as the road of `CLAUDE.md` has
it: "The call by which the setup program asked a running copy for its
data goes then, with the last of the watch on DOS." Taking DOS's own
interrupt, so that every call of every program passes the program, was
for a second program that there is no more, and is a thing of DOS
alone.

**Which scenarios show it.** Every scenario of the setup program is held
against the golden that the original's setup program left, with two
things otherwise, which `harness/deviations.py` has. The portable C is
taken through the item of its menu, "F9 o m", where the original is
taken through the command, "d:\vc\vcsetup>nul" and Enter: the keys that
switch Memory allocation in the original, a step that is left out for
the portable C (see 1), and left out before this one is put in. And the row
that has the command in the original, behind the setup and afterwards
for as long as it is on the screen, is empty in the portable C: the
golden is derived with spaces there. The scenarios `setup-menu`,
`setup-defaults`, `setup-missing`, `setup-damaged` and `setup-cancel`
pass so, and those of the help, `setup-help`, `setup-help-pages`,
`setup-help-mouse`, `setup-help-file` and `setup-help-lines`; and so do
`setup` and those of the option dialogs, from `setup-config` to
`setup-missing-saved`, and `setup-delay-help`, which has the help over
one of them, and those of what crosses the screens, `setup-mouse`,
`setup-save-fails`, `setup-clock`, `setup-left-handed` and
`setup-help-screens`. Ten of them show the Configuration or Screen and
Mouse, which are without three settings here: each of those has a patch
of the original's golden besides, and six are taken through other steps
where a line has moved, by 1, 5 and 7, whose tables name them.
`setup-help-file`
has a help file that cannot be read, twice: the line that the original
prints then is not in its golden, since the step that starts the setup
program sends what it prints nowhere.

No scenario shows the 64 bytes. `setup-defaults` saves a setup that was
made by Set default configuration, starts the program anew with it and
ends with the setup file as that program saves it, where the bytes
behind a path are zeros in the original too: the file that the setup
program itself has written is not on the disk when the scenario ends.
`setup-missing-saved`, which saves the setup that is made without a
file, ends in the same way, with the file as the program that goes on
saves it.
No scenario has a drive that is not ready while the setup is open, a
`VC=` in the environment, or a look at the history.

The Options menu shows in three scenarios that have a patch for the
first deviation already, `clock`, `clock-russia` and `menu`: their rows
there say what differs, and their patches name this entry too.

What the program leaves when DOS ends it shows in `ended`, which has a
patch for the first and the seventh deviation already: its row under 1
says what differs, and its patch names this entry too. The program that
started the copy prints DX and DS as how far they are from where INT
27h goes, which is to DOS: DX is the same on every build and is
compared, and DS, the segment of the program that prints, follows the
size of the program and is left out of the comparison, in
`harness/deviations.py`, where both words were left out while DS:DX was
an address in the program.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `asked` | It is the program asked for its data by another program, by the program's own call: both panels and the main data, and a byte of each printed. The portable C does not answer |
| `handed-on` | It shows what the program hands on of the calls of DOS that pass it, and how: by a program that looks for the far jump in the program's entry of INT 21h and puts itself behind it. The portable C has no such entry, and no call passes it |

Both stay for the C translation, which has the call and the watch as
the original has them. `passed-on`, which shows the same as `handed-on`
with a second copy, was not run before, for want of memory: see 1.

**In the data.** The test `portable_c-data` holds four tables of the
menus against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages
of their help, the commands of the Options menu, where the item is the
number of a command that the original has not, and the size of the
Options menu, each with what the two lines add behind what the original
has. The constants
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

**Which scenarios show it.** None, for now. The cell shows on a screen
of 50 lines alone, and the one scenario that has it there is
`setup-lines`: the setup over what DOS has shown, with the panels off,
where the first row is the second line of the program's banner, and
its first cell, the C of Copyright, is a space in the original from
the way out of the colours on, and when the setup has been left. The
portable C was held to that scenario with a patch that said the one
cell otherwise, on its last two screens, and passed. It is not taken
through it now: the scenario gets its 50 lines by EGA lines, which the
portable C has not, and a screen of more than 25 rows is not tested
there until a system gives it one (see 12). The scenario stays for the
original and the C translation, and comes back here with the tall
screen; its patch is in the history, `setup-lines.patch`. Every other
scenario of the setup that the portable C is taken through is on a
screen of 25 lines, where the cell lands in a row of the buffer that
is not on the screen.

### 10. The program calls itself Vibe Commander

**What the original does.** It names itself in four ways:

- By its title, "The Volkov Commander, Version 4.05", one text in three
  places: in the second row of the Info panel, in the middle; over the
  list of the pages in the window of its help; and as the first line of
  its banner.
- By its banner, two lines that it has DOS write when it starts and that
  stay on the screen behind the panels: the title with the word of how
  it is distributed behind it, "The Volkov Commander, Version 4.05
  freeware", and "Copyright (C) 1991-2000 Vsevolod V. Volkov".
- By its name without a version, "Volkov Commander", in the box that
  asks before the program is left, by F10 or by Quit of the Files menu:
  in the title of the box, "The Volkov Commander", and in its question,
  "Do you want to quit the Volkov Commander?". The box is as wide as the
  question.
- In the two lines that ask for Enter when a command leaves the screen
  as one that the program cannot draw on, 40 columns or graphics: "Press
  ENTER to return" and "to the Volkov Commander", on two lines where the
  screen has fewer than 80 columns and on one otherwise.

**What the portable C does.** It is Vibe Commander 0.1, and says what it
is based on wherever the original named its version. No screen has a row
more for it than the original's.

- The banner is two lines still. The first is the name with the version
  and what it is based on, without the word "freeware", which said how
  the original was distributed: "Vibe Commander 0.1, based on Volkov
  Commander 4.05", 50 cells where the original's line is 43. The second
  keeps the copyright of the original's author and has that of this
  program behind it: "Copyright (C) 1991-2000 Vsevolod V. Volkov, (C)
  2026 Maksym Motorny", 67 cells where the original's is 42. So both
  reach further to the right behind the panels, and show in cells where
  the original has the blank screen.
- The title of the help's window is the first line of the banner: it
  has room there, 62 columns. It stands in the middle, as the original's
  does, six cells in where the original's is fourteen.
- The title of the Info panel is the name with the version alone, "Vibe
  Commander 0.1", in the middle of the row, 18 cells where the
  original's title is 34: the panel has 38 columns, which is no room for
  what the program is based on. That is a line of its own in the row
  below the title, "based on Volkov Commander 4.05", 30 cells in the
  middle of the row, where the original has the line across the panel;
  the line across is a row further down. The panel has no row more for
  it: its two lines on the memory are gone from below the title, by 11,
  which has the top of the panel row for row.
- The name without a version is "Vibe Commander". The box of F10 has
  "The Vibe Commander" in its title and asks "Do you want to quit the
  Vibe Commander?", and is two cells narrower for the two letters, a
  cell on either side, with its buttons where they were.
- The second line that asks for Enter is "to the Vibe Commander".
- Nothing else names the program otherwise. The files have the names
  they had, `VC.EXE`, `VC.INI`, `VC.HLP` and the others, and the
  variable of the environment is `VC`. The help file is the
  distribution's, which the portable C reads as it is: its pages speak
  of "the Commander" and of its author, and its header, which a viewer
  shows, has "VC 4.05 Help" and the author's copyright. The setup, which
  is a part of the program (see 8), does not name it: its menu is headed
  "Setup", and the pictures in which it shows the colours have a
  directory `VC` and a line of the help's list, "About the Commander",
  as the original's have.

**Why.** The user's decision, on 13 October 2026: the program in the
portable C no longer is what the original is, with what has gone from it
and with the setup in it, so it no longer carries the original's name or
its version. What it is based on is said wherever the original named its
version, and the copyright of the original's author stays where it was.
The C translation, which behaves as the original does, keeps the
original's name.

**Which scenarios show it.** Every scenario in which the banner shows
behind the panels or beside them, the Info panel is on, or the help is
at the list of its pages: a thousand screens, in some seventy
scenarios. Those are held against the original's goldens by rules of the
comparison, `rules` of `TREES` in `harness/deviations.py`, one for each
of the banner's two lines, one for the top of the Info panel, which is
of this entry and of 11 together and is described there, and one for
the title of the help's window. A rule says the text
as the original shows it and as the portable C does, cell for cell, and
is made to a screen of the golden wherever at least twelve characters of
the original's text show in their places: each cell that the golden has
as the original's text has it, the spaces behind a shorter text
included, is expected as the portable C's text has it, and every other
cell, where a menu, a box or a panel covers the text, as the golden has
it. Nothing is left out of the comparison by it.

What the rules cannot say is in the patches of these scenarios:

| Scenario | What changes in it |
|---|---|
| `quit`, `comspec`, `menu-items`, `setup-defaults`, `second-copy` | The box of F10, or of Quit of the Files menu: two cells narrower, with the other name in its title and its question. The cell that it no longer covers on either side shows what is behind it, and its shadow ends a cell further left |
| `palettes` | The same box in black and white and on a laptop's display, where it is in the colours of the panel and its shadow does not show: its frame and its two lines |
| `start-setup` | The same box, three times, once over what DOS had shown with no panel on the left: the cell that the box no longer covers shows the banner and the commands there |
| `start-screen` | The same box. And the second line that asks for Enter, on a screen of 40 columns, with the cursor two cells further left behind it; and on a screen of graphics, where the test sees the dots of the one line as cells: other dots, for another name |
| `setup-colors-own` | The sample of the colours that the setup shows at the right of the screen is grey on black on twelve screens, as what DOS has shown is, and covers the end of the banner's second line with four spaces. The rule takes a space in that colour for the screen behind the banner, and expects the line's letters in three of the four cells; the patch says that they are the sample's spaces |
| `mouse-list`, `mouse-help` | The mouse's pointer is on a cell of the banner, its first, and on a cell of the help's title. It turns the colours of its cell round, and a rule leaves a cell in another colour as the original has it: the patch says the portable C's character there, in the pointer's colours |
| `banner` | A panel on the left with none on the right covers the banner but for the ends of its lines, three letters and two, too few again: the patch has the ends of the longer lines there |

**In the data.** The test `portable_c-data` holds four texts against the
original's with this difference declared for each, in `DECLARED` of
`harness/datacheck.py`: the title, `Title_S`, which is the help's and the
banner's; the name without a version, `Name_S`; the second line of the
banner, `Copyr_S`, with what is put behind it; and the lines that ask
for Enter, `CrLf_S`. The title of the Info panel, `NameVer_S`, and the
line below it, `BasedOn_S`, are texts that the original has not, and are
listed as the tree's own, in `OWN_OF` there. The word behind the
original's title, `Suff_S`, is gone.

### 11. The Info panel says nothing of the memory

**What the original does.** The Info panel has its title in its second
row, a line across the panel under it, and from the fourth row on what
it knows, each line in the middle of its row, the figures in yellow. The
first two are of the memory:

- "653,296 Bytes Memory" on the bench of the tests: where DOS says that
  the memory ends which it has for programs, a word of the program's
  PSP, as bytes.
- "485,344 Bytes Free" there: how much of that a command gets, which
  follows how the program is set up to give itself up for a command, and
  how large the part of it is that stays (see 1).

Below them, in the sixth row, is the label of the drive, then the size
of the disk and what is free on it, the files of the other panel, a
second line across, and the text of `dirinfo`.

**What the portable C does.** It has neither line, and what it is based
on in the row that this leaves (see 10). The top of the panel, row for
row:

| Row of the panel | The original | The portable C |
|---|---|---|
| 2 | "The Volkov Commander, Version 4.05" | "Vibe Commander 0.1" |
| 3 | the line across | "based on Volkov Commander 4.05" |
| 4 | "653,296 Bytes Memory" | the line across |
| 5 | "485,344 Bytes Free" | empty |
| 6 | "Volume in drive C: is WORK" | the same |

Nothing from the sixth row down moves, the second line across is where
it was, and the panel is as tall as it was. The program does not work
the two figures out: it does not read where the memory ends, and the
word of the main data that held it, `Memorys`, keeps its place and is
neither set nor read. How much memory a command has is no longer to be
seen from the program; the README has it for the bench of the tests,
under "The figures".

**Why.** The user's decision, on 13 October 2026: the two lines "mean
nothing off DOS". How much of 640K there is and how much of it the next
command will find is what a user of DOS had to know before running
anything, and what no other system has a program count. So it is the
removal of something that only made sense on DOS, like those before it.
What stands in the rows that it leaves is of 10, which was decided with
it: what the program is based on, on a line of its own between the
title and the line under it, and no row added.

**Which scenarios show it.** Every scenario that has the Info panel on:
some 390 screens of 27 scenarios as they stand, on some fifty of which a
menu or a box covers a part of these rows. They are held against the
original's goldens by one
rule of the comparison, of `rules` of `TREES` in `harness/deviations.py`,
which is of 10 as well: the four rows from the title to the count of
free memory, with the frame on either side, 40 cells each, as the
original shows them and as the portable C does, each cell with its
colour. As the other rules of 10, it is made to a screen of the golden
wherever at least twelve characters of the original's rows show in their
places, letters and figures, not the frame and the line, which the foot
of any panel has too; each cell that the golden has as the original's
rows have it is expected as the portable C's rows have it, and every
other cell, which is what covers the panel, as the golden has it. Three
things it knows that the rules of a single text need not:

- The colours of the cells one by one: the two figures are yellow in
  rows of cyan, and nothing is yellow in the portable C's four rows.
- The shadow of a box: a menu or a box over the panel throws its shadow
  on the cells beside it and below it, which then have the row's
  character in what the shadow makes of the row's colour. The portable
  C has its own character there, in what the shadow makes of its
  colour: sixteen screens, most of them of `menu`.
- The rows gone off the top of the screen. An error beeps, the BIOS
  moves the screen up a row for the beep, and the panel stands a row
  or two further up, without its title: nine screens, all of `move`.
  And a row of the panel that is left twice then makes two places for
  the rule that lie over each other; it takes the one where more of the
  panel shows.

The count of free memory is a figure that the comparison leaves out for
the C translation, since it follows the size of the program
(`IGNORED_OF` of `harness/deviations.py`), and the original's goldens
have `#` for its digits. The rule has the row so, and expects an empty
row in its place, every cell of it: nothing of these rows is left out of
the comparison for the portable C, and the figure is no longer left out
for it anywhere.

What the rule cannot say is in the patch of one scenario:

| Scenario | What changes in it |
|---|---|
| `move` | An error has moved the screen up a row, and what the program puts back where its boxes were leaves the row of the count of free memory on two rows or three, one under the other, whole or beside what is left of a box: twelve screens. The rule knows the panel once on a screen, and not one row of it by itself. Each of those rows is empty in the portable C, with what the box left where it was |

`memory`, which is about the count of free memory, is not run for the
portable C: see 1.

**In the data.** The three texts of the two lines, `PtIn1_S`, `PtIn2_S`
and `PtIn3_S` of `info.c`, are gone. The test `portable_c-data` holds
the texts that the tree has against the original's, and has nothing to
say of one that the tree has not. `BasedOn_S` is listed there as the
tree's own: see 10.

### 12. No EGA lines: the screen has the rows that the program finds

**What the original does.** Alt+F9, EGA lines of the Commands menu, gives
the screen more rows than 25, and 25 again. It asks the BIOS how many
lines of dots a character has, sets the mode of the screen anew, which
makes it 25 rows, and, if the font was of 14 lines or more, has the BIOS
load its font of 8 lines, which makes 43 rows of an EGA's screen and 50
of a VGA's, and tells the Print Screen key to print every row of such a
screen. Then it asks the BIOS how many rows that has made, takes 50 at
the most, and fits what it shows to them: what DOS had shown moves up
when the screen gets shorter, so that its last lines stay, and the
command line with it, onto the second row if it would leave the screen;
a screen that gets longer has empty rows below. The mouse is given the
rows of the new screen for its pointer. The key bar has "EGA Ln" for F9
while Alt is held, and the help a page for the item of the menu.

On a display that is neither an EGA nor a VGA the key does nothing, the
item of the menu is dark, and the key bar of the panels is another, the
same without that name.

The program also finds the rows that the screen has without the
command: when it starts and behind every command that it has run, it
asks whether the display is an EGA or a VGA, and such a display how many
rows it shows, and takes that, 50 at the most. So it draws on a screen
that something else has given 43 or 50 rows, with its panels, its boxes,
its help, the viewer and the editor as tall as the screen is.

**What the portable C does.** It has no such command, and does not
change the rows of the screen.

- Alt+F9 is a key that has no command, and does nothing. The Commands
  menu has no line "EGA lines": with Memory info gone too (see 7) it has
  eleven lines where the original has thirteen, Swap panels stands two
  rows further up than in the original with what is below it, and the
  box is two rows shorter. The letter E chooses nothing there, and
  Alt+F9 nothing in any open menu. The key bar has no name for F9 while
  Alt is held.
- The key bar of the panels is the one, whatever the display is: the
  original's second one, for a display without such lines, was the
  first without the name, which is what the first is now.
- It finds the rows that the screen has as the original does, when it
  starts and behind every command, and draws on as many as it finds, 50
  at the most. That is untouched: `IniData` of `start.c`, and every
  routine that goes by the rows. What a user has for a tall screen, for
  now, is a screen that has been given more rows before the program is
  started, or by a command run from it.
- It does not tell the Print Screen key anything. On a screen that
  something else has made tall, the key prints what that something has
  told the BIOS to print.
- The original's help file, which the portable C reads, still has its
  page on EGA lines; no key and no item of a menu leads there.

Gone are `F69_00` of `commands.c`, with its number among the commands,
its key in the main loop, its line of the Commands menu and what goes
with a line there, and its name in the key bar; the key bar of the
panels for a display without EGA lines, with its row of names, and the
main loop's choice between the two; the test that made the item of the
menu dark; and two functions of `sys.h` that only the command asked,
`ScreenSmallFont` and `ScreenPrintAll`, with their bodies in `dos.c` and
in `dos32.c`. `ScreenEga` and `ScreenFont` stay, for the rows that the
program finds; so does the byte of the main data that says whether the
display is an EGA or a VGA, which the start sets and reads.

**Why.** The user's decision, of 13 October 2026: EGA lines goes, and
the rows of the screen are to come from the system that the program
runs on. The command is of the PC alone: a font of 8 lines that the
BIOS of an EGA or a VGA loads, and the BIOS's own routine for the Print
Screen key. On a system that the program is carried to, how tall the
screen is belongs to the terminal or the window that the program is
shown in, and the program is told; that is what finding the rows is
for, and it comes with the first such system.

**Which scenarios show it.** Each of these is held against the
original's golden with what its row says made to it: by its patch, or,
where the row says that every screen is as the original's, by the steps
left out alone. The scenarios that press the key do so to get to a
screen of 50 rows and to leave it again, and the steps on that screen
are left out with the key, from the one press to the other:

| Scenario | What changes in it |
|---|---|
| `menu`, `mouse-menu`, `mouse-drag-menu` | The Commands menu where it shows, a second line shorter, without EGA lines: their patches, which are of 7 too, have the rows. In `mouse-menu` the click on the item that is dark beside the Info, Compare directories, is taken to where that item is in the portable C, two rows further up: on the original's row the portable C has Menu file edit, which the click would choose. The pointer of the mouse is on that row behind the click, which the patch says |
| `tree-choose` | The key bar with Alt held, twice, has no name for F9, where the original has EGA Ln, and none for F5 (see 7) |
| `menu-items` | EGA lines is not chosen from the Commands menu: the two steps that choose it, for 50 rows and for 25 again, are left out. Going back to 25 rows the original moves what DOS had shown up by 25 rows, which leaves none of it. So where the scenario later has the panel on the right off, beside the Info, the original shows nothing there and the portable C the ends of the two lines of its banner: two rows of one screen, in the patch, which is of 10 too |
| `help-keys` | The help on 50 rows is not shown: the last five steps are left out. Every screen before is as the original's |
| `mouse-help` | The mouse in the help on 50 rows: the last nine steps are left out. Every screen before is as the original's, with the patch that the scenario has for 10 |
| `scroll` | The two steps that go to 50 rows and back, with the panels on, are left out. Every other screen is as the original's after the same step |
| `user-menu` | The user menu in its box on 50 rows: four steps are left out. Every other screen is as the original's after the same step |
| `history-drives` | The history in its box on 50 rows: four steps are left out. Every other screen is as the original's after the same step |
| `menu-panel` | The command line left high up by 50 rows, with the panels off: the last four steps are left out, the panels switched off and on again around the two presses. Every screen before is as the original's |

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `setup-lines` | It is the setup on a screen of 50 rows, over what DOS has shown there, and the corner of that where the original's setup program writes its key bar a cell too far (see 9) |
| `setup-colors-lines` | It is the setup's list of colours on a screen of 50 rows, which has room for all 39 |
| `setup-help-lines` | It is the setup's help on a screen of 50 rows, in a window of 38 lines |

With these, no scenario has the portable C on a screen of more than 25
rows: EGA lines was the one way that the bench has to such a screen. On
the user's word of 13 October 2026 that is not tested for now, "we can
solve this problem when we have our first modern os implementation with
variable terminal width and height". The scenarios stay as they are for
the original and the C translation, and their goldens with them; what
is left out here and what of the program it was the test of is listed
in `TODO.md`, "The tall screen: untested in the portable C until a
system gives it rows", to be brought back then.

**In the data.** The test `portable_c-data` holds nine tables against
the original's with this difference declared for each, in `DECLARED` of
`harness/datacheck.py`. Seven of them 7 names too, and the entry of
each says both: of the menus their lines, the pages of their help, the size of the
Commands menu and where the lines and the pages of the Options menu
begin, the commands of the Commands menu, and the keys that choose an
item in an open menu, which have Alt+F9 no more and the three items of
the Commands menu below EGA lines two places further up than in the
original; and of the key bar the names of the keys, without "EGA Ln",
and the rows of the names, where the row that had the name has none
there, the row for a display without EGA lines is left out, and every
name behind "EGA Ln" has a number less. The eighth is the table of the
key bars, whose ninth, the bar for such a display, is spaces, and which
names the rows behind the one left out by a number less. The ninth is
the commands of the Options menu, where the setup, the last of the
commands, has a number less (see 8).

### 13. The editor of the program of 32 bits has room for a larger file

This is of one build of the portable C alone: the program of 32 bits,
`build/portable_c32/VC.EXE`. The program of 16 bits that is built of
the same source does here what the original does.

**What the original does.** The editor has the whole of a text in the
memory. Its room is what lies behind the lists of the two panels, as
far as the block of memory goes that the program got from DOS when it
started: `EdSpace` of the editor, in paragraphs. That is what DOS had
free then, less the program and its panels, so it follows the machine,
what else is loaded, and the size of the program. A file that is not
shorter than the room is not opened: a box of the editor says "The file
NAME is too large for Edit." and offers View, which shows the file in
the viewer, and Cancel.

Two more things are kept in the same memory. A copy reads a file into
it, as much as there is room for, and writes that before it reads on.
And the file search keeps its list of what it found there, which is
full when the memory is.

**What the portable C does.** The same, by the same statements, with
the room that each of its builds has. In the program of 16 bits that is
what DOS has left, as in the original, and less than the original's by
what the program is larger. The program of 32 bits has its panels and
its text in memory of its own, 640 KB that are a part of the program
and that it hands out to itself where the program of 16 bits asks DOS
(`TODO.md`, "The portable C in 32 bits: the plan", "The memory"): its
editor has all of that but the panels' lists, whatever DOS has free and
however large the program is. On the bench of the tests that is nearly
twice the original's room, and more than twice that of the program of
16 bits; `TODO.md`, "C1, the editor", has the four figures as they were
measured.

So the program of 32 bits opens a file in the editor that the original
refuses with its box, if the file is larger than the original's room
and smaller than its own. A file that is larger than its own room it
refuses as the original does, with the same box. A copy reads a file
of that size in one piece where the original reads and writes it in
two, and the file search has room for a longer list.

**Why.** The user's decision, on 13 October 2026: the editor of the
program of 32 bits has room for a file that the original's has none
for, that is declared, and the memory is not made short for it. How
much room there is was never the program's to say: the original has
another room on every machine, and a file that it refuses on the bench
it opens where less is loaded. A length chosen so that this build
refuses what the original refuses on the bench would be a number made
for a test, and would go with step 2 of the road, when the memory is
the extender's and no longer a stretch of the program's own. It is no
removal of something that only made sense on DOS, and was asked first.

**Which scenarios show it.** `editor-big`, which makes a file of
381,338 bytes, above the room of the original's editor on the bench
and in the same 64K, and has the editor refuse it, view it from the
box, and refuse it once more. The program of 32 bits opens the file at
the first of those steps. The scenario is not run for that program:
`TREES["portable_c32"]` of `harness/deviations.py`, which is what the
portable C is declared to do with this added. It is run for the program
of 16 bits and for the C translation, and held against the original's
golden as it is. No patch and no rule is of this entry.

What the scenario has behind that file is not run for the program of 32
bits either, and some of it no other scenario has: a file that is
shorter than its directory entry says, and the editor's box left
without View. `TODO.md`, "The editor's room: untested in the program of
32 bits", lists what the scenario alone tested, what that program was
seen to do with its other steps, and what would test it again.

Two scenarios are run for the program of 32 bits and pass, and do not
get it where they get the original, for the same memory. Nothing says
so when they are run, since a count of free memory is left out of the
comparison and their steps were made to end on the same screen however
much room a build has:

| Scenario | What it is for | In the program of 32 bits |
|---|---|---|
| `editor-full` | The editor with all of its memory full: a line put back until there is no room, and two characters typed that have none | The editor is never full: it has some 250,000 bytes free where the original has none. Seen, in the count of its status line |
| `copy-big` | A file that the memory does not hold, 526,632 bytes, read and written in two parts, and skipped when not all of it has been read | The memory holds it: one part. By the numbers, not seen |
