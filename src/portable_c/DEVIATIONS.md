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
written. `python3 harness/goldens.py changes --tree portable_c` prints what
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
it. It is a program of 32 bits, which an extender, DOS/32A, starts
above the megabyte: all of it is there, also while a command runs, and
below the megabyte stays the extender.

- The Options menu has no line Memory allocation: Save setup, below
  it, stands a row further up for that. The letter M chooses another
  thing there, the setup, for which the menu has two lines at its
  bottom that the original has not: see 8. Shift with Enter is Enter.
- The program takes no check sum of its file and does not know a second
  copy of itself: it never says "Bad or missing", and goes on if its
  file is deleted while a command runs.
- The main data is of the program, not a block that it asks DOS for. The
  panels and the memory of the viewer, the room, are not of DOS either:
  they are the extender's, above the megabyte, which the program asks
  for as the original asks DOS for its block, and of which it gets as
  much as the machine has. They are three blocks where the original has
  one that it cuts up: one for each panel, 64K, which is as long as a
  panel's list of files can get, asked for when the program starts and
  kept for as long as it runs; and the room, the largest block that
  there is then, which is all that the extender has behind the panels:
  the README's figures have how much that is on the bench of the tests,
  some 15 MB, and 13 says what follows for the editor. The original has
  each panel as long as its list and the room behind them, and moves
  the second panel and the room when a list grows or shrinks; here
  nothing is moved. For a command the room is given back, and
  afterwards the program asks for the largest block that there is, as
  the original does. DOS gains nothing by that; a program that the same
  extender starts, a second copy of the program among them, gets its
  memory there. And for a viewer or an editor of another's, which the
  original runs with its block made as short as its panels and the
  least that its own viewer needs and then makes as long as DOS lets
  it, the room is made that short and is then given back and taken
  again, since the extender says nothing of how long a block could be;
  should there then be less than the least, the program says "Program
  too big to fit in memory" and ends, as it does after a command, where
  the original cannot meet that. A block is given with what the memory
  held there, as DOS gives the original its block: what the program
  had there before it gave the room back, what a program run meanwhile
  left. While the memory was 640 KB of the program's own, it was zeros
  until the program wrote there.
- A panel keeps all of its files for a command, where the original
  keeps 256, and is read again afterwards: what was selected stays
  selected. That is entry 20.
- What the main data has behind the command line, which the original
  has as whatever the memory holds after a command until it is set
  again, is as it was before the command.
- "Program too big to fit in memory" is said, as by the original, when
  the program starts or comes back from a command and has too little
  memory for its two panels and the least that the viewer needs: on a
  machine with too little memory above the megabyte, or behind a
  command that has kept some of it, as a program of the same extender
  that stays in the memory would. No scenario shows that: a command of
  DOS takes no memory of the extender's. The program says it too when
  it starts and DOS has not the 512 bytes below the megabyte that the
  system's functions keep there; and with less memory above the
  megabyte than its objects need, the extender does not start it, and
  says so in words of its own.
- The Info panel, which says in the original how much memory a command
  gets, says nothing of the memory: see 11.
- The map of the memory, which lists in the original what a command
  will find free, is gone from the portable C: see 7.
- The program's own call, by which another program is handed the
  copies, is gone from the portable C, with the setup program made a
  part of the program: see 8.
- The byte of the setup file that held the switch is kept as it is found
  and written back as it was; the program does not go by it. A setup file
  of the original is one of the portable C and the other way round.
- The setup has no box for it either. The original's setup program,
  `VCSETUP`, has the switch in its Configuration as well, the first of
  three boxes of Execute options, and writes the byte. In the setup of
  the portable C, which is that program (see 8), the box is gone, and so
  are the two below it, Quick execute commands (see 14) and TSR manager
  (see 7): the dialog has no Execute options, and 14 says how it stands
  without the group. The setup writes the byte into the setup file as it
  read it, and Set default configuration makes it 1, as the original's
  does.
- The original's help file, which the portable C reads, still has its
  page on Memory allocation; no item of the menu leads there. And the
  page of the Configuration that the setup shows for F1 still has its
  paragraph on the box, behind that on Show logical drive B:: the help
  file is the distribution's.
- The file is an `.EXE` file: the extender, a program of DOS, with the
  program behind it as a linear executable, the main data in it, which
  the extender loads and relocates.

What follows from the program's being where it is, with its extender
below it. The last four points are declared as they are on the user's
word of 13 October 2026:

- What it costs of DOS's memory. While a command runs, the original
  keeps 12,704 bytes of it and the C translation 19,728: on the bench of
  the tests DOS has 412,848 bytes for a command that is run from the
  original as it is distributed, and 405,824 for one run from the C
  translation. A command run from the portable C has of DOS's memory
  all but the extender's part, some 52 KB.
- A second copy of the program has the memory to run a command, as the
  original's has.
- The memory above the megabyte, while a command runs. The original
  takes none of that memory: a command of it has all of it, 15,232K on
  the bench by the BIOS, and with an XMS driver loaded 14,978K of them
  free. The extender of the portable C takes all of it when it starts,
  as it comes, and holds it while a command runs. Where no XMS driver
  and no other memory manager is loaded, as on the bench, it has INT
  15h for that and answers a command in the BIOS's place: by function
  88h that there are 0K, and E801h and E820h it refuses. With an XMS
  driver loaded, 0K are free to a command. So a command that wants such
  memory for itself finds none: a program with another extender, a
  cache, a RAM disk, which is from what a command is told and was tried
  with none of them. A program that DOS/32A starts gets memory all the
  same, since its extender asks the one that is in the memory, and a
  second copy of the program is such a one. On the bench the scenario
  `command` shows it in what FreeDOS's `mem` prints, run as a command:
  told by the BIOS that there is no such memory, it goes by the two
  bytes of the clock's memory, which say 15,360K, 128K more than the
  BIOS tells a command of the original, since the BIOS keeps that much
  at the top for itself. The extended memory in all and what is used of
  it are that much more there, and the total memory; the total that is
  used follows the same and is left out of the comparison for following
  the size of the program. The extender can be told how much to take,
  by four bytes of its settings, which was seen on a copy of the file
  and is not done: `TODO.md`, "C2, a second copy in the driver, and
  `command`", has what a command is told, call by call, and what that
  setting would cost.
- What it wants of the machine, where it cannot start. It is compiled
  for the 386 and started by an extender that needs one, and it wants
  memory above the megabyte for all of itself, for its two panels and
  for the least of its room: the README's figures have how much, some
  360 KB. The original runs on any PC that DOS runs on, and wants no
  memory there. With too little for its objects the extender does not
  start it and says so in its own words, not in the program's:
  "DOS/32A fatal (4003): not enough extended memory to load application
  exec" is what a second copy was told on the bench when the first had
  been left a megabyte for the two of them, and the program ends with
  FFh. With enough for its objects and too little for the rest, the
  program says "Program too big to fit in memory", in the original's
  words. What a machine without a 386 is told is
  the extender's as well, and was not seen: the bench has no such
  machine. And where the program itself says "Program too big to fit in
  memory", for want of the 512 bytes, the words are the original's,
  written through DOS from a copy that `dos32.c` has of them, and the
  program ends with 8, DOS's number for too little memory; the original
  ends with 1 behind those words. No scenario shows any of this.
- What the extender is in the machine while the program runs, a
  command of the program's included. Its part of real mode lies below
  the megabyte in the program's own block, 51,696 bytes on the bench,
  with the 512 bytes of the system's functions in a block of DOS's
  behind it. INT 21h of real mode goes to the extender for as long as
  the program is in the memory, so a call of DOS that another program
  makes then passes the extender on its way; so does INT 15h where no
  memory manager is loaded, by which a command is told of the memory
  above the megabyte: the point on that memory, above. When it starts,
  the extender sets the counter of the machine's timer anew, in its
  mode 3 where the BIOS has it in mode 2, and it keeps the line A20 on.
  The program runs in protected mode at the highest privilege, with no
  memory protected from it, as nothing is on DOS. And the call by which
  the program says that it is idle, INT 28h, comes to whoever has taken
  that interrupt from the extender's part of real mode, on a stack of
  the extender's, 512 bytes, with DS and ES 0, where the original's
  comes from the program on the program's stack with DS on the main
  data. No screen shows any of this, and no scenario.

**Why.** The user's decision, on 12 October 2026: giving up the code and
the data and making them again saves memory that only DOS is short of,
at the price of reading the program's file and its directories after
every command, and of a program in two parts with copies between them.
It "only matters on DOS"; "it's OK for our port to be less memory- and
disk-efficient, as long as it's functionally equivalent"; and the option
"in the new tree goes away. It's going to be a pattern for a lot of
features that only made sense on DOS." And on 13 October 2026: where the
program as one of 32 bits does again what the original does, and what it
could not do while it was built as a program of 16 bits and kept all of
itself in DOS's memory, a second copy that has the memory to run a
command, it is held to the original's golden, and this entry says only
what still differs. And the same day: what follows from its running
under an extender and shows on no screen is declared as it is, what it
wants of the machine and what the extender is in the machine while it
runs among it. The user's word, "declare all five", on a list of what
differs in the program for its 32 bits (`TODO.md`, "The program of 32
bits: what differs from the original, for the user", 6 to 10 there; the
others of the five are in 6 and in 8). And the same day, when the last
of that list had been put to them: that a command finds no memory above
the megabyte while the extender holds it is declared as it is, with no
change of the program (1 of that list, and `CLAUDE.md`, "The portable
C").

**Which scenarios show it.** Each of these is held against the original's
golden with what its row says made to it: by its patch, or, where the row
says that every screen is as the original's, by the steps left out alone:

| Scenario | What changes in it |
|---|---|
| `lost` | The program's file is deleted by a command. The original asks for it, twice, and ends; the portable C is back at its panels, and the keys that answered the original are keys at the panels: a space into the command line, Enter, which runs it, and Esc, which switches the panels off |
| `clock`, `clock-russia` | The Options menu where it shows: without Memory allocation, so with Save setup a row further up. And by 8: with two rows more at its bottom, a line across the menu and More setup, so a row taller than the original's |
| `menu` | The same. By 8: Up from the top of the menu, and PgDn, go to More setup, the last item, where the original has Save setup, and Up from there goes to Save setup, where the original goes on from that to Memory allocation. And by 7: the Commands menu where it shows, five times, a line shorter, without Memory info, with the items that are dark there a row further up |
| `second-copy` | Every screen is as the original's: the second copy is started with a command to run and runs it. Its patch is of 10 alone |
| `start-memory` | The second copy runs the command that was to leave it without memory for its data, as the original's does, which stays in the memory and leaves 75 KB of DOS's. The original's copy then says "Program too big to fit in memory" and ends. The portable C's needs none of DOS's memory to come back and says nothing, so the screen shows a row less there, and it is still there afterwards: the panels of the last three screens are the copy's, with the mini status that the first copy's have not |
| `command` | Where `mem` is run as a command, two rows of what it prints: the extended memory, in all and used, 15,360K where the original has 15,232K, and the total memory, 16,384K where it has 16,256K. A command is told that there is no memory above the megabyte, and `mem` goes by the clock's memory, which has 128K more |
| `setup-config` | The Configuration of the setup where it shows: without Memory allocation, by 7 without TSR manager and by 14 without Quick execute commands, so without Execute options. The row of the scenario in 14 says all that differs in it: of this entry, the step that unticks Memory allocation is left out, and the setup file that is saved has the byte as it had it, 1, where the original's has 0 |
| `setup-config-mouse`, `setup`, `setup-missing-saved`, `setup-clock`, `setup-left-handed`, `setup-help-screens` | The Configuration so, where each shows it: their rows are in 14. All but `setup-config-mouse` and `setup-missing-saved` show Screen and Mouse too, which is otherwise by 5 |
| `ended` | The second copy runs the program that it is to leave in the memory before DOS ends it, and that stays in the memory, 448 bytes, also when DOS has ended the copy, which by 7 keeps no list of such programs to release: no screen of the scenario shows that, since the program that started the copy clears the screen. And by 7: the registers that the program which started it prints are as the portable C leaves them, both times: CX and SI as they went, 2222h and 1111h, where the original has 0 and, the second time, 0002h; and DS:DX no longer where INT 27h goes, which the original has as 0 and 0 from there. And by 8, with no interrupt of DOS's taken: DX has in its low byte what DOS was last told of Ctrl-C and is as it came above that, which is what the original leaves there where its setup has TSR manager off; and DS is as it came, on the program that started the copy, a segment that follows the size of the program and is left out of the comparison, in `harness/deviations.py`. The look at the map of the memory that ends the scenario, two steps, is left out |

A step that switches Memory allocation, "F9 o m", is left out of every
scenario for the portable C: there it would enter the setup (see 8). Of
the scenarios that the portable C is taken through none has such a step
since `quick`, which had one before its last command, is not run for it:
see 14.

On every screen that shows one of the two dialogs with the mouse pointer
where the setup put it, in the middle of the screen, the pointer is over
a line that has moved: the cell whose colours it exchanges holds another
character, and the cell that the original has there is five rows further
up, in the colours of its line. The patches have both.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory` | It switches Memory allocation off, for the Info panel to show the other number, and shows the menu without the mark |
| `passed-on` | It has a program stay in the memory, starts a second copy behind it and runs commands from that, to see what the copy hands on of the calls of DOS that pass it. The portable C does not take INT 21h, so no call of DOS passes it and it has none to hand on: see 8, where `handed-on`, which sees the same with one copy, is not run for that |

**In the data.** The test `portable_c-data` holds five tables of the menus
against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages of
their help, the size of the Options menu, its commands, and the item that
Shift-F9 chooses in it. And of the setup's Configuration, for this and for
7 and 14 together, the dialog, `StConf_B`, and twelve lines of its box:
14 says how each is held.

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
besides. `TopView` and `TopScr`, which the C translation has in its
`dos.c`, are gone, with the inline assembly that they were made of,
`VideoBuffer`, three instructions. The byte of the main data that said
whether such a buffer is drawn into, `TopView`, keeps its place and is
neither set nor read.

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

**What the portable C does.** It does not ask, and does what the
original does without DESQview, also under it: it makes none of those
calls, Alt alone brings up the menu bar if the Configuration says so,
and the editor empties the keyboard if the Configuration says so.
`DvPause`, which the C translation has in its `dos.c`, is gone, with the
inline assembly that it was, `DesqviewPause`, six instructions, and the
call of DOS in `IniData` of `start.c`. The byte of the main data that
said whether DESQview runs, `DesqVie`, keeps its place and is neither
set nor read.

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
and is neither set nor read. The one call of INT 2Fh that stays is
`DriveCd` of `sys.h`, for the search of files, which asks the CD-ROM
extensions whether a drive is theirs.

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
to its memory and reads it at once. `InPort`, which the C translation
has in its `dos.c`, is gone, and of the compiler's intrinsic functions
`inp`, the instruction `IN`: the program reads no port any more. The
byte of the main data that said whether to wait, `Snow`, keeps its place
and is neither set nor read. Snow mode is a byte of the setup that is
kept as it is found and written back as it was, 2 in a setup made of the
defaults: a setup file of the original is one of the portable C and the
other way round.

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
and a function is handed what is asked and nothing else. `dos32.c` is
those functions, for a program of 32 bits whose extender takes each call
down to real mode, where DOS and the BIOS are. A call gets the registers
that it takes from the function's arguments, and 0 in every other of AX,
BX, CX, DX, SI and DI, and in the half of one that it takes nothing in:
at a call of DOS by `Set` of `dos32.c`, which fills all six, and at a
call of the BIOS or of the mouse driver by the function that makes it.
Nothing is what the code before left there, and nothing what the stack
holds. One call is handed something else than 0 where it takes nothing,
the BIOS asked about its font: "What is known to read such a register",
below. What is in DS, in ES and in the carry flag is the extender's
doing, and so is that some calls of the original's are not made: both
are below, behind the table. Every other call, its place in the order
and every register that it takes are the original's.

The places, file by file and in each file as its routines stand, each
with what the original hands over there in a register that the call does
not take, read from the assembly, and what the C translation handed
over, where that was noted to be something else: where that column is
empty, nothing was noted, and as a rule the C translation handed over
what the original does. What the portable C hands over has no column: it
is 0 in each of the six registers that a row names, and in DS what is
said below. Every file of the program has gone over to `sys.h`, and each
added its places here as it went. A call that the portable C makes no
more has no row: DOS asked where INT 21h goes, when the program starts,
went with the program's own call (see 8); the five calls of the BIOS in
`F69_00` of `commands.c`, the font asked about, the mode set, the small
font loaded, Print Screen told of the rows and the font asked about
again, went with EGA lines (see 12); and INT 2Eh, a command handed to
the COMMAND.COM that is in the memory, went with Quick execute commands,
as did DOS asked for the current drive in two places for that way alone
(see 14); and DOS asked for memory, 48h and 4Ah in `Init11`, `TakeMem`
and `Aexec0`, which are asked of the extender since the memory is its
own (see 1, and the table of the calls below).

| Routine | Call | The original | The C translation, where it differed |
|---|---|---|---|
| `Blank`, `blank.c`, with its `Star` | BIOS 0Fh, 03h, 01h, 02h and 09h: the cursor noted, hidden and put back, and a star written | 0Fh: AL and BX as the code before left them, for a star that changes its look and its clock and kind. 03h, 01h and 02h: AL the mode, as 0Fh left it. 01h: BH the page, and DX where the cursor is or was put. 02h: BL and CX as the code before left them, for a star its clock and a count of the stars. 09h: DX the place of the star |  |
| `M02_10`, `commands.c` | DOS 3Bh, the directory of a panel made current | AL the drive, 1 for A: |  |
| `F35_61`, `copy.c` | DOS 0Eh, a drive made the current one, twice | AL the drive, as in DL, the first time, and as choosing from the tree left it the second; DH as the code before left it |  |
| `C00_01`, `copy.c` | DOS 4400h, what a handle is, and 3Eh, the file closed | 4400h: CX what was left of comparing the target with the directory of the other panel, DX the address of the name. 3Eh: AL as 4400h left it, CX that still, DX what the handle is |  |
| `C00_01`, `C10_00`, `C20_00` and `C30_00`, `copy.c` | DOS 4300h, the attributes of a file or a directory, at seven of the eight places | CX as the code before left it: what was left of comparing two names or of a piece, or the attributes that the call before answered with |  |
| `C10_00` and `C20_00`, `copy.c` | DOS 41h, a file deleted, at nine places | AL as the call or the dialog before left it; CX the attributes of the file, as they were asked for or the 20h just set, or what else the code before left |  |
| `C10_00` and `C30_00`, `copy.c` | DOS 56h, a file or a directory renamed, at three places | CX what was left of comparing the two names, or at the second try of `C10_00` the attributes of the target, with AL as 41h left it |  |
| `C20_00`, `copy.c` | DOS 4400h, what a handle is, for the source and for the target; 3Ch, the target made; and 3Eh, a file closed, at five places | 4400h: DX the address of the name, CX what the code before left, 20h behind a file made. 3Ch: AL as the call before left it, the error of 4300h or what 41h answered. 3Eh: AL as the call before left it at three of the five; CX and DX what that call took or answered, which is what was left of a piece and where it was to go in the memory, the time and the date just set, the length of the file, or the address of the name; and DS on the memory that the file is read into, at three |  |
| `C20_00`, `copy.c` | DOS 3Fh, the last byte read of a file that is appended to; 36h, the room of a drive; and 40h, what still fits of a piece written | 3Fh: AL the low byte of the place just gone to. 36h: BX the handle and CX the size of the piece, DH the second character of the target's name, DS on the memory that the file is read into. 40h: AL the low byte of the count, as in CL |  |
| `C20_00`, `copy.c` | DOS 5700h, the date and the time of a file: of a source that is a device, and of a target that is appended to | CX and DX: what was left of comparing the two names, or the attributes just kept or set, and the address of the name |  |
| `C20_00`, `C30_00` and `F36_04`, `copy.c` | DOS 39h, a directory made; 3Ah, one removed; and 3Bh, one made the current one | AL as the call before left it, but at 39h of `C20_00`; CX the attributes of the source or of the target at 39h and 3Ah of `C30_00`, what was left of looking for the end of the path from the second directory that `C20_00` makes, or what else the code before left |  |
| `F38_40`, `dirfunc.c` | DOS 41h, a file deleted that was read-only | CX 20h, of the call that set its attributes |  |
| `M30_50`, `dirfunc.c` | DOS 5700h, the date and the time of a file, and 3Eh, the file closed | 5700h: CX the attributes and DX the address of the name. 3Eh: CX the time just set and DX the address of the name |  |
| `F38_60`, `dirfunc.c` | DOS 60h, 3Bh and 3Ah, the current directory left and removed | 60h: DX the address of the path, as SI. 3Bh and 3Ah: SI on the path and DI behind the two full stops, of 60h and of what was written since |  |
| `FncMs1`, `dirfunc.c` | DOS 4Fh, the next file | CX 37h and DS:DX on the mask, of the call for the first file |  |
| `DirFunc`, `dirfunc.c` | DOS 4Eh and 4Fh, the first file and the next, in both of its searches | 4Eh: AL the last character of the directory's path. 4Fh: DS:DX on the mask, of the call for the first file, and CX 37h of that call or what a count has come down to since |  |
| `DelTr0`, `dirfunc.c` | DOS 3Eh, a file closed | CX 1 and DX the address of the byte, of the call that wrote it |  |
| `F54_00`, `dirfunc.c` | DOS 13h, the label deleted for a new one, and 3Eh, the new one's file closed | 13h: AL a space, of filling the name with them. 3Eh: CX 8 and DX the address of the name, of the call that made the file |  |
| `F62_01`, `drive.c` | DOS 440Eh, the letter that a floppy drive goes by, for A: and for B:, and 440Fh, the letter that was chosen made the drive's | 440Eh: BH the other of the two drives, CX the item of the menu bar, DX as looking for the drives left it. 440Fh: BH the letter, CX the item, DX the drive with the height of the box above it. DS on the panel at both | 440Eh: DX the address of `TmpBuf`, DS on the main data |
| `F62_01`, `drive.c` | DOS 47h, 3Bh and 0Eh, a panel put on the drive that was chosen | 47h: AL as 440Fh answered, the drive that the unit goes by; BX the letter and the drive, of that call; CX the item of the menu bar; DH the height of the box. 3Bh: AL the low byte of what 47h left, BX and CX still. 0Eh: AL the low byte of the panel's segment, DH still, and DS on a panel |  |
| `T80_02`, `edit.c` | DOS 4400h, what the handle of the file is that was made, and 3Eh, the file closed | 4400h: CX the attributes and DS:DX on the name of the file. 3Eh: CX the attributes, and DX as the call before left it | 3Eh: CX the count of the last write, DS on the piece last written |
| `T00_00`, `edit.c` | DOS 3Eh, the file closed that was read | CX the count of the last read. DS on the segment of the piece last read |  |
| `F21_10`, `enter.c` | DOS 3Bh, the directory of the tree made current | CX 37h, of the call that looked for the directory, where it did |  |
| `FindFil`, `find.c` | DOS 4Fh, the next file | CX as the code before left it, and DS:DX on the path |  |
| `FindFil`, `find.c` | DOS 4201h, 80 bytes gone back in a file that is searched | DS on the segment that the file is read to |  |
| `FindFil`, `find.c` | DOS 3Eh, a file that was searched closed | CX and DX as the calls before left them | DX the address of `TmpBuf` |
| `F67_00`, `find.c` | DOS 4409h and 4408h, a drive asked whether it is of a network and whether its disk can be taken out | CX the drive, 0 for A:. DX the address of the path at 4409h, and at 4408h what 4409h left there |  |
| `F67_00`, `find.c` | DOS 0Eh and 19h behind Go to, a drive made the current one and the current one asked for | DS on the segment of the list at 0Eh and of the panel at 19h. DL the drive still at 19h | DS on the segment of the list at both |
| `HelpFil`, `help.c` | DOS 3Eh, the help file closed | DX as the code before left it | DX the address of the file's name |
| `ErrRdy`, `main.c` | DOS 440Fh, the drive of the box made the one that its unit goes by | BH the row of the box, CX the letter of the drive, DX the size of the box |  |
| `Init40`, `main.c` | BIOS 03h, where the cursor is, and 0Eh, a line feed typed | 03h: AL the mode, as 0Fh left it. 0Eh: CX the shape of the cursor and DX its place, as 03h left them, with the row that the program goes on with |  |
| `Init50`, `main.c` | DOS 47h, the current directory of the drive that was chosen | AL the drive, 1 for A:. DH the rows of the screen. CL the letter of the drive |  |
| `F39_00`, `menu.c` | BIOS 02h of the keyboard, the keys that are held | AL 2, the menu that the command is of |  |
| `ReDr00`, `read.c` | DOS 4Eh, 47h and 4Fh, the directory of a panel read | 4Eh: AL the drive, 1 for A:. 47h: AL the drive too, and CX 37h of the search. 4Fh: DS:DX on the mask, of the call for the first file, and CX 37h of that call or 0, where a copy has counted it down |  |
| `ReDr00`, `read.c` | DOS 3Fh and 3Eh, the extension file of the directory read and closed | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what reading answered, CX and DS:DX the count and the buffer still. And the original reads AL back behind 3Eh (see below) |  |
| `RdInfo`, `read.c` | DOS 36h, the room of the drive, and 3Fh and 3Eh, dirinfo read | 36h: AL the drive, as DL. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  |
| `GetVol`, `read.c` | DOS 60h, the full name of the drive, and 4Eh, its label | 60h: BX, CX and DX as the caller has them, for the Info the handle of dirinfo, the count and the buffer. 4Eh: AL 5Ch, of the two backslashes that the name was held against |  |
| `OutStr`, with `OutStr0`, `exec.c` | BIOS 0Eh, a character typed, for every character of what the code of the commands writes | CX and DX as the code before left them | CX and DX 0 |
| `NotFind`, `exec.c` | BIOS 00h of the keyboard, a key waited for | AL as the call before left it | AL 0 |
| `SetBrk`, `exec.c` | DOS 3300h and 3301h, whether DOS looks for Ctrl-C at every call, asked and set | 3300h: DX as whoever calls has it. 3301h: DH as 3300h left it | DS 0 at both, DX 0 at 3300h |
| `Exit00`, `exec.c` | DOS 3301h, Ctrl-C looked for as before, and 4Ch, the program ended | 3301h: DH as the code or DOS before left it. 4Ch: DS:DX on where an interrupt went before the program took it, of giving it back, where the watch on DOS is on | DS 0 at 3301h |
| `Exec02`, `exec.c` | DOS 3301h, Ctrl-C looked for as before; 4B00h, COMMAND.COM run; 19h and 47h, the current drive and its directory for the prompt; BIOS 0Eh, the end of the line typed, twice | 3301h: DH as the code before left it. 4B00h: CX the length of the command's tail, with the slash above it; SI on the tail and DI on the name COMMAND.COM. 19h: AL 0Dh, of the start of the prompt, and DX as the command or DOS left it. 47h: AL a colon, of the prompt. 0Eh: CX and DX as the code before left them | CX and DX 0 at 0Eh |
| `SetStat`, `exec.c` | DOS 5801h and 5803h, how DOS looks for memory | CX and DX as the code before left them |  |
| `Alloc3`, `exec.c`, the original's `Alloc0` from that label on | DOS 30h, the version, and 4Ah, the block of the program made shorter | 30h: AL as taking the check sum of the file left it, which the portable C does not take. 4Ah: AL the minor number of the version; CX and DX how much of itself the original then moves, in bytes and in paragraphs |  |
| `SaveSet`, `setup.c` | DOS 4300h, 3Ch, 40h and 3Eh, the setup file written | 4300h: BX the length of the file. 3Ch: AL the low byte of what 4300h answered, BX the length still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still |  |
| `ReadOne`, `start.c` | DOS 3Dh, 3Fh and 3Eh, a file read | 3Dh: BX, CX, SI and DI 0. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  |
| `Init00`, `start.c` | DOS 19h and 47h, the current drive and its directory; BIOS 00h, the mode set; DOS 09h, the banner written | BX 1 if the setup file was not one of the program's and 0 if it was or there is none, and from the BIOS's call on the page that is shown above that. CX 0 at 19h and 47h |  |
| `IniData`, `start.c` | DOS 38h, the country, and DOS's routine for capitals, 128 times | 38h: BX and CX as the BIOS left them at the call before, the page and the shape of the cursor. The routine: AH as 38h left it |  |
| `IniData`, `start.c`, where a command has left a screen that is not text of 80 columns | BIOS 01h, the cursor hidden; 00h, the mode set; 0Fh, the mode | 01h: AL the mode, as 0Fh left it (see below), and BX the page and the columns. 00h: BX the same, and CX 2000h, the shape that hides the cursor. 0Fh: AL as 00h left it, BX and CX the same |  |
| `IniData`, `start.c` | BIOS 03h and 01h, the shape of the cursor asked for and set | 03h: AL the mode; BL, CX and DX as the calls before left them. 01h: AL as 03h left it, the mode from a BIOS that leaves AX alone (see below); BH the page; DX where the cursor is, as 03h answered |  |
| `IniData`, `start.c` | BIOS 12h and 1130h, an EGA and its font | 12h: AL the current drive, as DOS answered the call before, AH 19h, which the portable C does not make (see 14); CX and DX as left. 1130h: BL and CX as 12h answered, the memory of the EGA and its switches (see below); DH as left |  |
| `IniData`, `start.c` | DOS 3306h and 30h, the version | 30h: BX what 3306h left in AX. CX and DX of both as the calls before left them |  |
| `IniData` with `CurNl`, `start.c` | The colour that DOS writes in: BIOS 0Fh and 03h, the page and the cursor, twice; 0Eh, a carriage return or a line feed typed, three times; 08h, the cell at the cursor, twice; DOS 06h, a space written; BIOS 09h, the cell written back | 0Eh: BL what DOS left when it was asked for its version, the low byte of AX from 3306h or BL from 30h; the BIOS takes a colour there only where the screen is graphics. 06h: AL the character at the cursor, as 08h answered; DH the row of the cursor; BX and CX as the BIOS left them. AL of 0Fh, 03h and 08h as the call before left it. CX and DX of the BIOS's calls the shape of the cursor and where it is, as 03h answered, with the space in DL from 06h on |  |
| `StRdHelp`, `sthelp.c` | DOS 3Dh, 3Fh and 3Eh, the pages of the setup's help read from the help file | 3Dh: BX, SI and DI as finding the directory and copying the name left them, CX 0, of that copy. 3Fh, five times: AL the low byte of the place in the file that was just gone to, or of what was worked out since, the room for the index or the bytes between two pages. 3Eh: AL as the call before left it, the low byte of what was read where all went well; CX and DS:DX the count and the buffer still, or what the look at the header left |  |
| `StDefault`, `stmain.c` | DOS 19h, the current drive, for the panels that the setup makes without a setup file | AL as the code before left it: what DOS answered about the setup file, or 0, the answer of the box that asks |  |
| `StInit15`, `stmain.c` | DOS 3Dh, 3Fh and 3Eh, the setup file read | 3Dh: BX, CX, SI and DI as the help file read and the name copied left them. 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  |
| `StExit`, `stmain.c` | BIOS 01h, the cursor in its shape again; DOS 4300h, 3Ch, 40h and 3Eh, the setup file written; 09h, the line that says it could not be | 01h: DX the rows of the screen and 80, of the screen put back. 4300h: BX the sum of the file. 3Ch: AL as 4300h left it, or 0 for a file that is not there; BX the sum still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still. 09h: AL, BX and CX as the calls before left them |  |
| `StSetup`, `stmain.c` | BIOS 0Fh and 03h, the page and the cursor, where the setup notes the row and the shape of the cursor | 0Fh: AL and BX as making the table of the small letters left them. 03h: AL the mode, as 0Fh left it; BL, CX and DX as the code before left them |  |
| `StF07_10`, `stcolor.c`, of the setup program's `F07_16` | BIOS 06h and 07h, the lines of the list of colours moved by one | BL the column of the lines, 8 |  |
| `StF07_01`, `stcolor.c`, of the setup program's `F07_57` | BIOS 01h, the shape of the cursor put back when the list of colours is left | AL the low byte of the key that left the list, 1Bh for Esc and 0 for F10, or 0 behind the mouse; BX and DX where the box of the list is and its size |  |
| `OpenFil`, `system.c` | DOS 3Dh, a file opened, both times | BX the service and the mode, as in AX |  |
| `SetPth`, `system.c` | DOS 47h, the current directory | AL the drive, 1 for A: |  |
| `Fantom`, `system.c` | DOS 440Fh, the drive that a unit goes by, three times | BH the other of the two drives |  |
| `ReadTre`, `treebox.c` | DOS 3Fh and 3Eh, the tree's file read | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  |
| `ReadTre`, `treebox.c` | DOS 4Eh and 4Fh, the directories of the disk read | AL 2 at both, which is put there for how the reading ends if a key stops it. 4Fh: CX and DX as the code between left them, 37h and the address of the mask, of the call for the first file, or a count and the size of the box that counts |  |
| `ReadTre`, `treebox.c` | DOS 41h, the tree's file deleted, and 4300h, 3Ch, 40h and 3Eh, the file written | 41h: CX as the reading of the disk left it. 4300h: BX and CX the length of the file and of its list. 3Ch: AL the low byte of what 4300h answered, BX the length still. 40h: AL the low byte of the handle. 3Eh: AL the low byte of what was written, CX and DS:DX the count and the buffer still |  |
| `FileLst`, `usermenu.c` | DOS 41h, 3Ch, 40h and 3Eh, the list of the selected files written | 41h: AL the low byte of what setting the attributes answered, and CX 20h of that call. 3Ch: AL as the call before left it. 40h, for the mask of a directory and for the end of a line: AL the low byte of what the write before answered. 3Eh: AL as the last write left it, with CX and DS:DX the count and the text of that write |  |
| `F32_00`, `usermenu.c` | DOS 3Fh and 3Eh, the menu file read | 3Fh: AL the low byte of the handle. 3Eh: AL the low byte of what was read, CX and DS:DX the count and the buffer still |  |
| `Load`, and Home and the search of `V00_10`, `view.c` | DOS 4200h, a place of the file gone to | DS on the segment of the text |  |
| `V13_00`, `view.c` | DOS 3Eh, the file closed | DS on the segment of the text. DX as the key before left it | DX the address of `TmpBuf` |
| `Aexec0`, `view.c` | BIOS 03h, the shape of the cursor | BL the column that the box begins in |  |
| `Aexec0`, `view.c` | DOS 4B00h, the viewer run | CX, SI and DI as the code before left them | CX, SI and DI 0 |
| `F34_11`, `view.c` | DOS 3Fh and 3Eh, the extension file read and closed, and 4Eh, the viewer looked for | 3Eh: CX and DS:DX the count and the buffer still. 3Fh and 4Eh: SI and DI, and at 4Eh BX, as the code before left them | 3Fh and 4Eh: SI and DI 0, and BX 0 at 4Eh |
| `ChkMode`, `window.c` | BIOS 0Fh, the mode | BX as the caller has it |  |
| `Cursor`, `window.c` | BIOS 0Fh and 02h, the cursor placed | 0Fh: AL the column, DX the place. 02h: AL the mode, as 0Fh left it |  |
| `ChgCur`, `window.c` | BIOS 01h, the shape of the cursor | DX where the cursor is, as 03h left it |  |
| `GetCur`, `window.c` | BIOS 03h, where the cursor is | AL the mode, as 0Fh left it |  |
| `Scroll`, `window.c`, for its seven callers | BIOS 06h and 07h, a window scrolled | BL the column of a place that the caller has |  |

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

DOS and the BIOS are of real mode, where the extender takes each call,
and what follows from that is declared as it is, on the user's word of
13 October 2026. It was seen on the bench, by a program of real mode
that takes INT 21h, runs the program and writes down every call as DOS
gets it, `harness/trial32/calls.asm`, with the program taken through its
start, a command and its end.

What DOS is handed besides the six registers:

- DS and ES are not on the main data, as the original's DS is, since
  that is above the megabyte: they are what the extender has in them
  when it goes over to real mode. At a call that takes no address, DS
  held the number that is in AX and ES 0, at every such call that was
  seen: 1900h in both for the current drive, 3301h, 2C00h, 4C00h. At a
  call that the extender translates, a name or a buffer handed over, DS
  was on the extender's own buffer below the megabyte, to which it
  copies the name, and ES as the extender left it. At the thirteen
  functions that `dos32.c` makes in real mode, both are on the program's
  512 bytes below the megabyte, where the name or the buffer then is.
- The carry flag is clear at the two calls that the original makes
  with it set and that DOS leaves it at, 36h and 13h: the extender does
  not carry the program's flag down. `dos32.c` counts it as set
  afterwards, so the program goes on as the original does behind a DOS
  that leaves the flag; a DOS that looked at it would see it clear.
- At 4B00h, a program run, DOS's block names the program's environment
  by its segment, where the original names none and DOS copies the
  program's own: it is the same environment, named because the word of
  the PSP through which DOS would find it is
  a selector of the extender's. CX, SI and DI are 0 there.
- When DOS has ended the program by another program's hand and is told
  of Ctrl-C by the routine of real mode that the program has left for
  that (see 8), the registers that 3301h does not take are as the
  program that started this one left them, which is what the original
  hands DOS there.

Which calls DOS is asked. The calls, their order and every register that
a call takes are the original's, but for these calls of the original's,
which the portable C does not make, and one that it makes besides:

| When | The original | The portable C |
|---|---|---|
| It starts | 4Ah, its own block made as long as the program is | Not that: the block is the extender's, which has asked DOS for it before the program is entered. Where the PSP is, the extender is asked, and not DOS. One call more, before the first of the program's: 48h for 20h paragraphs, the 512 bytes below the megabyte |
| It starts, and comes back from a command | 2523h, Ctrl-C taken; 48h twice, how much memory there is and all of it taken; 3524h and 351Bh, who has the critical error and Ctrl-Break; 2524h and 251Bh, both taken. Behind a command 49h first, the memory given back | None of them. The memory of the panels, the viewer and the editor is the extender's, which it is asked for in the same order, by DPMI 0500h, 0501h and 0502h, and the three interrupts are taken through the extender: DOS sees neither. So `ErrVct` and `BrkVct` of the main data, where the original keeps who had the two, are never written |
| It runs a command | 2524h and 251Bh, the two given back; 4Ah, the block of the panels made shorter | Neither, for the same two reasons: the room is given back to the extender, 0502h. The extender is asked where INT 22h of real mode goes before 4B00h and has it put back behind it, since DOS changes it for the program that is run and the extender would write a warning of that when the program ends |
| It runs a viewer or an editor of another's | 4Ah, the block made as short as the panels and the least of the viewer, before 4B00h; behind it 4Ah twice, how long the block could be and the block made that long | None of them: the room is made that short by the extender, 0503h, and behind 4B00h given back and taken anew, as after a command, by 0502h, 0500h and 0501h |
| It is left | 251Bh, Ctrl-Break given back | Not that: the extender gives back the three that the program took through it |

Every other call is made as the original makes it, in the same order,
with the registers that it takes: the version, how DOS looks for memory, the
files, the directories and the searches, the country, the drives, the
time, a program run and the end. What the extender asks DOS for itself,
while it loads the program and when it leaves, is not the program's
and is not listed.

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
  menu has no line "memory Info": it has a line less than the original's
  thirteen for that, Directory sizes stands a row further up with what
  is below it, and the box is a row shorter. (It is one more line and
  row shorter since, without EGA lines: see 12.) The letter I chooses
  nothing there, and Alt+F5 nothing in any open menu. The key bar has no
  name for F5 while Alt is held.
- The program does not take INT 27h, notes nothing of the programs that
  are run, and writes no record: a program that stays in the memory
  keeps the block that it asked for, as long as it asked. Any number of
  them may stay, and none is ended for being one too many.
- Nothing releases a program that has stayed. It is still in the memory
  when the program is left, as it is when it was run from DOS's own
  command line, and what gets it out again is whatever does there.
- When the program is left, such a program keeps its memory and is cut
  off from its interrupts, by the extender. Declared as it is on the
  user's word of 13 October 2026, with no change of the program. As the
  program is left, its extender holds the interrupts of real mode
  against what they were when the program was started, writes a line
  onto the screen that is left for each that is otherwise, "DOS/32A
  warning (9003): real mode interrupt vector has been modified: INT
  1Ch", and puts every one back. So the program that stayed has none of
  its interrupts, which is the original's, that gave each back as it
  released the program; and it keeps its memory, which the original gave
  back to DOS, with the extender's lines on the screen, which are of
  neither. And Ctrl-Break, INT 1Bh, it has lost before, while the
  program still runs: the program takes that interrupt through the
  extender behind every command and gives it back in front of the next
  and at its end, and the extender then has it go where it went when the
  extender started, not to who has taken it since. What is known of
  this, and how. Seen on the bench: in `stay-left`, a program that stays
  with INT 1Bh, 1Ch, 21h, 2Fh and 60h taken, and a second that asks by
  INT 2Fh who the first is and prints which of the five go to it. Run
  from the program with the first in the memory, it prints all but INT
  1Bh; the program left, the screen has four lines of the warning, for
  INT 1Ch, 21h, 2Fh and 60h; and at DOS's prompt it prints two dashes
  for each of the five. Since it finds the first by INT 2Fh, those
  dashes say only that INT 2Fh no longer goes there, and nothing of the
  other four: that those are put back too is by the four lines, and by
  the extender's code. And seen with FDAPM, which the bench has: run
  from the program as `fdapm apmdos`, it leaves four lines for INT 16h,
  28h, 2Ah and 2Fh when the program is left, and `fdapm stats` says then
  that none is resident, where it found it behind the portable C as that
  was built for 16 bits, with no extender. Read in the extender's code,
  and seen no further than that: that all 256 interrupts are put back
  there, with the masks of the two interrupt controllers, and INT 1Bh,
  1Ch, 21h, 23h, 24h and 2Fh whatever its settings say; that the mouse
  driver is told there to call nobody, so that a program which has
  stayed and has given the driver a routine of its own would lose that,
  which is not among what was decided and is the user's still; and that
  when DOS ends the program by another's hand that part of the extender
  is not reached, so that no line is written and those six alone are put
  back, with INT 15h where the extender has it. A mend was built and
  dropped on the user's word the same day: it left such a program at
  work but for INT 21h, which the extender has while the program runs
  and which nothing of the program can keep for another. `TODO.md`, "L,
  a program that has stayed in the memory, when the program is left",
  has the mend in words and what was read of the extender.
- The program does not take INT 21h either, since. When the watch went
  the entry of that interrupt was kept for the program's own call, which
  the same entry takes, and handed every other call of DOS on untouched;
  it went with that call when the setup program became a part of the
  program: see 8. A call of DOS that another program makes goes to DOS,
  and does not pass the program.
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
  options, and writes the byte. In the setup of the portable C, which is
  that program (see 8), the box is gone, with those of Memory allocation
  (see 1) and of Quick execute commands: 14 says how the dialog stands
  without the three, which were all of its Execute options. The setup
  writes the byte into the setup file as it read it, and Set default
  configuration makes it 1, as the original's does. When the setup is
  left with Save, the program keeps the byte that it had, as the
  original's running program does.
- The original's help file, which the portable C reads, still has its
  pages on the map; no key and no item of a menu leads there. And the
  page of the Configuration that the setup shows for F1 still has its
  paragraph on TSR manager, behind that on Quick execute commands.

Gone are `memory.c` and `memory_t.c`, all of the map; and of `exec.c`
the list, `Counter` and `PrgList`, the records and what made and read
them, `ExeProg`, `AddProg`, `FreeBlk`, `ChkBlk` and `Release`, the entry
of INT 27h, `New27`, with the interrupt function `Int27` behind it, the
interrupts noted, `IntTab`, `SavePtr` and `NameBuf`, and `GetStat`,
which noted how DOS looks for memory before a program was released. The
entry of INT 21h, `New21`, was left then as the three comparisons that
know the program's own call and two jumps, with `Int21`, the interrupt
function that it went on in, which answered that call and returned: both
have gone since, with the call (see 8). What is left for two things that
are not the map: `SetStat` with 0, by which the program has DOS look for
memory in the lowest block first when it starts, as the original does;
and the prompt that is written between two commands, which the original
makes where its watch notes the interrupts and the portable C in a
buffer of its own.

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
| `stay-left` | The program that has stayed in the memory with five interrupts, INT 1Bh, 1Ch, 21h, 2Fh and 60h, is not released when the program is left: at DOS's prompt the second program prints all five as still going to it, where the original has given every one back and it prints five times two dashes. The patch names 10 and 14 too, for the box of F10 and for the Advanced options. In 32 bits, by a patch of that program's own, which has those rows of 10 and 14 and three screens otherwise: with the first program in the memory and the program still running, the second prints two dashes for INT 1Bh and the numbers of the other four; when the program is left, four lines of the extender's warning stand above DOS's prompt, for INT 1Ch, 21h, 2Fh and 60h, with the prompt and the cursor two rows further down; and at DOS's prompt the second program prints five times two dashes, as behind the original, which says that INT 2Fh, by which it finds the first, goes there no more, with the four lines two rows further up |

And `menu` and `ended`, which the first deviation changes too: their
rows there say all that differs in them, and the patch of each names
both entries. So it is with the scenarios that show the Configuration of
the setup, which is without TSR manager, without Memory allocation and
without Quick execute commands: `setup-config`, where the step that
unticks TSR manager is left out and the byte is 1 in the setup file that
is saved, `setup-config-mouse`, `setup`, `setup-missing-saved`,
`setup-clock`, `setup-left-handed` and `setup-help-screens`. Their rows
are in 14, and the patch of each names that entry with this one and 1.

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
  screen is what DOS had shown, the row of the command line is emptied, where the original
  has its prompt and the command of the setup program there, and the
  cursor goes on to the next row, which moves the screen up if that row
  was the last. Nothing is noted in the history. The program keeps the
  two interrupts that it has taken. The lists of the panels are not
  cut down to 256 files, as the original's are for its setup program
  (see 20).
- Then the setup is on the screen as the original's setup program is,
  but for three settings of what the program has no more, which its
  dialogs do not have: Memory allocation and TSR manager in the
  Configuration (see 1 and 7), and Screen output in Screen and Mouse
  (see 5). There is no word on the version of DOS or on the memory, no
  mode set and no banner: the program has started already. The directory
  is the one that the program has, which it makes by its own rule: in
  capitals, and with a backslash behind whatever `VC=` names, where the
  setup program adds none behind a colon. The pages of the help are read
  from the help file there, as the setup program reads them when it
  starts, and nothing is said if they cannot be: "Error reading data
  from the help file." is among what the setup program wrote through DOS
  at its start, and is not shown. F1 then says, as in the original, that
  the help is not available.
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
  the call all the same, as the original's setup program does when it is
  run from the portable C as a command, is answered by DOS, which has AH
  2Bh for setting the date and refuses this one: the original's setup
  program has written the setup file by then and takes that for no main
  program in the memory, and the portable C goes on with the setup that
  it has.
- When DOS ends the program, the program that started it goes on with
  the registers as the portable C leaves them: ES on the PSP of the
  program that was ended, as in the original; in the low byte of DX what
  DOS was last told of Ctrl-C, and DS and the high byte of DX as they
  came, which is what the original leaves there where its setup has TSR
  manager off and it has no interrupt to give back; and CX and SI as
  they came (see 7). The flags are the original's.
- What the program leaves of the interrupts then is otherwise than the
  original, by its extender, and is declared on the user's word of 13
  October 2026 for what follows from running under an extender and shows
  on no screen (see 1). The original never gives Ctrl-Break, INT 1Bh,
  back when DOS ends it, and INT 22h, which DOS sets from the PSP, names
  the routine in which the original goes on then: both go into the ended
  program, memory that is free from then on. The extender sees the call
  that ends the program and, as it takes itself out, has Ctrl-Break go
  where it went when the program was started. And INT 22h names the few
  instructions of real mode that `dos32.c` keeps in its 512 bytes below
  the megabyte, in which DOS goes on then, since nothing of a program of
  32 bits can run there: memory that DOS has freed like the ended
  program, where the interrupt stays until DOS sets it anew for the next
  program that it runs. The registers are those above, left by those
  instructions; what DOS is handed there in the registers that its call
  does not read is in 6.
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
layout, which mean nothing in a file. That was made so before it was
asked, and was decided on the user's word of 13 October 2026: a setup
file made from the defaults has zeros there, and that stands. The
program's own call goes by the decision of 12 October 2026 too, as the
road of `CLAUDE.md` has it: "The call by which the setup program asked
a running copy for its data goes then, with the last of the watch on
DOS." Taking DOS's own interrupt, so that every call of every program
passes the program, was for a second program that there is no more, and
is a thing of DOS alone.

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
| `quit`, `comspec`, `menu-items`, `setup-defaults`, `second-copy`, `stay-left` | The box of F10, or of Quit of the Files menu: two cells narrower, with the other name in its title and its question. The cell that it no longer covers on either side shows what is behind it, and its shadow ends a cell further left |
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
`ScreenSmallFont` and `ScreenPrintAll`, with their bodies in `dos32.c`.
`ScreenEga` and `ScreenFont` stay, for the rows that the program finds;
so does the byte of the main data that says whether the display is an
EGA or a VGA, which the start sets and reads.

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
| `user-menu` | The user menu in its box on 50 rows: four steps are left out. Every other screen is as the original's after the same step, with the patch that the scenario has for 14 |
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

This is of the portable C as the program of 32 bits that it is,
`build/portable_c/VC.EXE`. As a program of 16 bits, which it was built
as until 13 October 2026, it did here what the original does.

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
the room that it has. The program of 16 bits that it was had what DOS
had left, as the original. The program of 32 bits has its panels and
its text in the extender's memory, above the megabyte, of which it asks
for as much as there is, as the original asks DOS (`TODO.md`, "The
portable C in a flat memory: the plan", "The memory from the
extender"): its editor has all of that but its own objects and its two
panels, 64K each, whatever DOS has free. The README's figures have how
much that is on the bench of the tests, some 15 MB: more than forty
times the original's room there, 345,840 bytes. From when it was
switched to 32 bits until the memory was the extender's, on 14 October
2026, the program handed out 640 KB of its own instead, and its room
was 641,824 bytes and then, with the panels in blocks of their own of
64K from the start, 524,240 bytes on any machine; `TODO.md`, "C1, the
editor", has the figures as they were measured then.

So the program of 32 bits opens a file in the editor that the original
refuses with its box, if the file is larger than the original's room
and smaller than its own. A file that is larger than its own room it
refuses as the original does, with the same box. A copy reads a file
of up to that size in one piece where the original reads and writes it
in parts, and the file search has room for a list as long as that room
holds. The editor's count of free bytes in its status line has seven
cells, and from a million on shows the last seven characters of the
number: that figure is left out of the comparison, and goes with every
figure of free memory, on the user's word of 14 October 2026, in a
piece of its own.

**Why.** The user's decision, on 13 October 2026: the editor of the
program of 32 bits has room for a file that the original's has none
for, that is declared, and the memory is not made short for it. How
much room there is was never the program's to say: the original has
another room on every machine, and a file that it refuses on the bench
it opens where less is loaded. A length chosen so that this build
refuses what the original refuses on the bench would be a number made
for a test. And the same day: from step 2 of the road on, the memory
for the panels and the texts comes from the extender, as much as the
machine has, and what a user then sees otherwise is declared, "an
editor that opens whatever fits". It is no removal of something that
only made sense on DOS, and was asked first.

**Which scenarios show it.** `editor-big`, which makes a file of
381,338 bytes, above the room of the original's editor on the bench
and in the same 64K, and has the editor refuse it, view it from the
box, and refuse it once more. The program of 32 bits opens the file at
the first of those steps. The scenario is not run for that program:
`not_run` of `TREES["portable_c"]` of `harness/deviations.py`. It is run
for the C translation, and held against the original's golden as it is.
No patch and no rule is of this entry.

What the scenario has behind that file is not run for the program of 32
bits either, and some of it no other scenario has: a file that is
shorter than its directory entry says, and the editor's box left
without View. `TODO.md`, "The editor's room: untested in the program of
32 bits", lists what the scenario alone tested, what that program was
seen to do with its other steps, and what would test it again.

`view-window` has the editor refuse a file whose entry says a megabyte,
which the room of the bench holds. It is run on a machine of 2 MB, for
the original and for both C trees, where the program of 32 bits has a
room of some 550 KB, and is refused the file as the original is: the
original looks at nothing above the megabyte, and its golden, derived
again on that machine, is what it was on the bench's (`memory` of a
scenario, `harness/scenarios.py`, "A machine with less memory").

Two scenarios are run for the program of 32 bits and pass, and do not
get it where they get the original, for the same memory. Nothing says
so when they are run, since a count of free memory is left out of the
comparison and their steps were made to end on the same screen however
much room a build has:

| Scenario | What it is for | In the program of 32 bits |
|---|---|---|
| `editor-full` | The editor with all of its memory full: a line put back until there is no room, and two characters typed that have none | The editor is never full: it has nearly all of its room free where the original has none. Seen, in the count of its status line. It was never full in the 640 KB either |
| `copy-big` | A file that the memory does not hold, 526,632 bytes, read and written in two parts, and skipped when not all of it has been read | The memory holds the file: one part. While the room was 524,240 bytes, it was two parts as in the original, the first of 523,776 bytes where the original's is of 345,600, and skipped with not all of it read. A machine of 2 MB does not get it there either: its room is larger than the file |

### 14. No Quick execute commands

**What the original does.** A command that is typed at the prompt, chosen
from the user menu or run by a line of the extension file is run by a
COMMAND.COM of its own: the program runs the file that `COMSPEC` names,
with `/C` and the command for its tail, once for every command. With
Quick execute commands switched on it does otherwise. The setting is the
first of the Other options in the Advanced options of the Options menu,
and the second of the Execute options in the Configuration of the setup
program; it is off in the program as it is distributed. With it, each
command is handed to the COMMAND.COM that the program was started from,
which is in the memory still, by that shell's own interrupt, INT 2Eh,
with the command at DS:SI. No second shell is loaded for a command, and
what a command sets in the shell stays set there. That shell is on the
drive that it was last left on, so the original notes the current drive
each time it starts, also when it has come back from a command, and
before the commands hands the shell one more, which goes to the current
drive, "D:", if that is another by then.
The interrupt comes back with every register lost, the stack among
them, and the original sets its stack anew behind it. A shell that does
not serve the interrupt runs nothing and says nothing, and the program
comes back as if the command had run: the COMMAND.COM of FreeDOS, which
the bench of the tests has, is such a shell.

**What the portable C does.** It has neither the setting nor that way:
every command is run by a COMMAND.COM of its own, with `/C`, as the
original runs one with the setting off.

- The Advanced options have no line for it. Other options has one line,
  Clear keyboard buffer, a row further up; the end of its frame, the two
  lines of text, the line across the box and the buttons are a row
  further up with it, and the box is a row shorter, seventeen rows of the
  screen with its margins where the original's has eighteen. The boxes go
  to each other as they stand now: Down from Alternative editor is Clear
  keyboard buffer, and Up from the buttons.
- The Configuration of the setup has no Execute options: the group had
  three boxes, of which Memory allocation went by 1 and TSR manager by
  7, and this was the one that was left. The Keyboard options stand at
  the top of the right half of the box, five rows further up than in the
  original's, beside the first five of the General options, and the box
  of the dialog is as tall as it was, since its left half is: the last
  five rows of its right half are empty. The boxes go to each other as
  they stand now, by the rule that the original's follow: Down from Show
  logical drive B: is Alt alone selects menu; Right from a box of the
  left half is the box on its row, or else the one above that row, which
  is Clear keyboard buffer from Clock down; and Left from a box of the
  right half is the box on its row.
- INT 2Eh is not called. And DOS is not asked for the current drive
  where the original asks for the sake of that way alone: when the
  program starts and each time it has come back from a command (AH 19h,
  in `IniData`, behind the environment searched), and before the
  commands are run (AH 19h, in `F21_31`). Nothing is left in the five
  bytes in front of the command line, `DosChg`, but what running a
  command by COMMAND.COM writes there, the count and `/C`.
- The byte of the setup that held the switch, `ExecTyp`, is kept as it
  is found and written back as it was, 0 in a setup made of the
  defaults; the program does not go by it. A setup file of the original
  is one of the portable C and the other way round: with a file that has
  the byte set, the portable C runs its commands by COMMAND.COM.
- The original's help file, which the portable C reads, still has its
  paragraph on Quick execute commands in the page of the Advanced
  options, and in the page of the Configuration that the setup shows
  for F1: the help file is the distribution's.

Gone are the way through the shell in `Exec02` of `exec.c`, with the
command kept in `BatPoin` of the main data meanwhile, a word that keeps
its place and is neither set nor read; what `F21_31` of `enter.c` and
`IniData` of `start.c` did for the command that goes to the drive; and
the function of `sys.h` by which the shell was handed a command,
`ShellRun`. With that, of `dos32.c`: the 26 bytes of instructions of
real mode, `Shell_T`, by which the interrupt was called below the
megabyte, with the stack kept by hand around it and put back, and the 48
bytes of the block there that they stood in. The program has one way of
running a command.

**Why.** The user's decision, on 13 October 2026, as `CLAUDE.md` has it
under "The portable C": Quick execute commands goes, the setting and the
way through INT 2Eh. It was asked and not taken out as a thing of DOS's,
since a feature is judged by what it gives a user and not by how DOS did
it: the interrupt is one that only DOS's COMMAND.COM has, but commands
run by the shell that the program was started from, sooner and with what
they set kept, are not DOS's alone. No bench of the project could show a
command run so: the shell there does not serve the interrupt.

**Which scenarios show it.** Each of these is held against the
original's golden with what its row says made to it, by its patch:

| Scenario | What changes in it |
|---|---|
| `command-edit`, `comspec`, `dirinfo`, `ended`, `sizes`, `view-search`, `editor-hex`, `editor-long`, `editor-save`, `editor-words`, `editor-full`, `editor-keys`, `editor-hex-keys`, `editor-buffer`, `editor-dirinfo`, `mouse-edit`, `start-setup`, `start-screen`, `start-memory`, `setup-delay-file`, `setup-help-file`, `stay-left` | The Advanced options, on two screens: opened, and with Clear keyboard buffer unticked, which is four times Down and Space where the original has five. From the row of Quick execute commands down the box has what the original's has a row further down, and the cursor on Clear keyboard buffer is a row further up with it; the row that the original's box ends on shows what is behind the box in the shadow of the box, and the row of that shadow what is behind it. The mouse pointer is in the middle of the screen, on the row that Quick execute commands was on: the cell whose colours it exchanges holds a letter of Clear keyboard buffer, the line that stands there now. `comspec`, `ended`, `start-setup`, `start-screen`, `start-memory` and `stay-left` have patches of other entries, which name this one too; `setup-delay-file` shows Screen and Mouse besides, which is otherwise by 5 |
| `user-menu`, `user-menu-run`, `view-commands` | The same, on one screen: the step that unticks Clear keyboard buffer also leaves the dialog |
| `command-line`, `setup-cancel`, `setup-defaults` | The Advanced options so, on one screen, with the original's steps: in `command-line` Escape toggles panels on/off is unticked, the second box, and in the other two the dialog is looked at for what the setup left |
| `setup-config` | The Configuration of the setup where it shows: without Execute options, and by 1 and 7 without the two other boxes of that group. The Keyboard options are five rows further up, and the last five rows of the right half are empty. Down from Show logical drive B: is Alt alone selects menu, which is unticked there by Space where the original goes down three lines first; the three steps on the way, which untick Memory allocation, tick Quick execute commands and untick TSR manager, are left out. Left from Clear keyboard buffer is Full screen, the box of its row, where the original has Show logical drive B: there. And the setup file that is saved has the three bytes as it had them, 1, 0 and 1, where the original's has 0, 1 and 0, with another sum for that: four bytes of the file |
| `setup-config-mouse` | The Configuration so. The click on Quick execute commands is left out, and those on Alt alone selects menu are five rows further up, and the mouse pointer with them. The setup file that is saved has the byte of Quick execute commands as it had it, 0, where the original's has 1, with another sum: two bytes of the file |
| `setup`, `setup-missing-saved`, `setup-clock`, `setup-left-handed`, `setup-help-screens` | The Configuration so, where each shows it; under the help of `setup-help-screens` nothing of its right half shows. The steps are the original's, since none goes into the right half |

On every screen that shows the Configuration with the mouse pointer
where the setup put it, in the middle of the screen, the pointer is over
a row of the right half that is empty now: the cell whose colours it
exchanges holds a space, and the cell that the original has there is
five rows further up, in the colours of its line. The patches have both.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `quick` | It switches Quick execute commands on and runs commands so, on the drive that the program began on and on another. The bench's shell runs nothing by that interrupt, so what the scenario holds is that the program comes back and goes on; the portable C runs the commands, by COMMAND.COM, as every other scenario that runs one has it do |

**In the data.** The test `portable_c-data` holds two tables of the
Advanced options against the original's with this difference declared
for each, in `DECLARED` of `harness/datacheck.py`: the box, `Sets_P`,
with fourteen lines and without the one that it named ninth, and the
dialog, `Sets_B`, without the field of the setting and with the fields
that the others go to and the rows of the two that have moved as they
are now. The line itself, `Set08_S`, is gone, and the test has nothing
to say of a text that the tree has not. Of the setup's Configuration,
for this and for 1 and 7 together, the dialog, `StConf_B`, is held
against the original's without the three fields of Execute options and
with the fields that each of the others goes to and the rows of those
that have moved as they are now; and twelve lines of its box,
`StCnf01_S` to `StCnf12_S`. A line of that box is one text for the two
halves: each of the seven that have something on the right is held
against the original's line of that name as far as its left half goes
and from there against the right half of the original's line five
further down, and the five that have nothing there against their left
halves, with spaces declared behind. The box itself, `StConf_P`, is the
original's. The instructions of real mode that the program had for the
shell, `Shell_T`, a table of its own that was not compared, are gone
from what that test lists.

### 15. Go to of the file search asks for the disk of the drive that its line is on

**What the original does.** Go to, in the list of what Alt-F7 has found,
makes the directory of the line under the cursor the current one. Before
it asks DOS to change there, it means to do what the program does before
every such call: if the drive of the path shares the floppy drive with
another, as B: does with A: on a machine with one floppy drive, and the
drive goes by the other letter just now, it shows the box "Insert
diskette for drive B:" and tells DOS of the answer, so that DOS does not
ask at its own prompt, over the program's screen. The routine for that,
`ChgDisk`, takes the path at ES:DX. Go to hands it the address of the
line with DS on the memory of the list, where the line is, and ES is on
the main data: the routine reads at the line's offset, which is below
16, in the main data, two of its first 16 bytes, which are colours of
the first palette. Those name no drive, their second byte being no
colon unless a colour of the setup is 3Ah, so the routine takes the
current drive, which needs no disk. The original then changes to the
line's directory and makes its drive the current one, and where that
drive wanted its disk asked for, DOS asks.

**What the portable C does.** It hands `ChgDisk` the line. DOS is asked
which letter the drive of the line goes by, and with Ok which it is to
go by (AX 440Eh and 440Fh, with that drive), where the original asks of
the current drive; the box is shown where the line's drive shares the
floppy drive and goes by another letter, and Cancel there leaves the
list for the panels with nothing changed, as it does wherever the
program shows that box.

**Why.** The user's decision, on 13 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The work on the
addresses met it: the program of 32 bits had done so since it was one,
by a cast that dropped a segment in 16 bits and drops nothing in a flat
memory, without anybody having decided it.

**Which scenarios show it.** None. `find`, `find-drives` and
`mouse-find` take Go to, each from the drive that the line is on or to
a drive that is no floppy's, and are held to the original's goldens as
they are. A scenario that showed it would take Go to on a line of the
drive that shares the floppy drive with another, B: while the drive
goes by A:, from a third drive: the original then has DOS's prompt for
the disk, and the portable C its box. It is to be written
(`TODO.md`, "F4, the viewer and the file search").

### 16. The file search reads no part of a file behind the end of its room

**What the original does.** Alt-F7 with a text to look for reads each
file that it finds by name in parts, into the memory behind its list of
what was found, and looks for the text in each part; a part begins 80
bytes before the end of the one before. A part is 8000h and 80 bytes
long, or as long as the whole room behind the panels where that is
shorter, and the search stops if that is less than 4,000 bytes. The
length is of the whole room, not of what is left of it behind the list,
which grows by a line of 65 bytes for every file found. So once the list
is within 8000h and 80 bytes of the room's end, its last 505 lines or
so, a part is read behind the end: DOS writes the file's bytes behind
the original's block of the memory, which is the last on the machine,
into what lies above DOS's memory, and the text is looked for in what
that memory then gives back. That is known by reading the assembly,
`FnFl44` to `FnFl46` of `VCFIND.INC`, and was not tried: it takes some
thousands of files found by one search.

**What the portable C does.** A part is as long as what is left of the
room behind the list, from where the part is read to, or 8000h and 80
bytes where that is shorter; with less than 4,000 bytes left, the search
stops, as it stops in both when the whole room is that short, and the
box says how many files were found and offers its menu, as for a list
that is full. Before the list is within 8000h and 80 bytes of its end,
nothing differs. `FindFil` of `find.c`.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The program of 32
bits wrote into its own memory behind the room there, and from the
memory of the extender on it would be ended by the extender for it. The
cluster N5 met it where it counted the room in bytes (`TODO.md`, "N5,
`find.c`", and question 10).

**Which scenarios show it.** None. `find`, `find-drives`, `mouse-find`,
`stamps` and `stamps-russia` search for a text with a list of a few
lines, and are held to the original's goldens as they are. A scenario
that showed it would have a search with a text find more files than the
room has lines for, less some 505: some 7,560 with the room that the
program of 32 bits has on the bench today, some 230,000 once the room is
the extender's, and in the original as many as its room on the bench
has, less 505. The bench has no such directory, and making one in the
scenario would take thousands of files written by a command of DOS; the
original's screen there would be by whatever lies behind its memory.

### 17. A copy from a device counts its room down by every read

**What the original does.** F5 and F6 read the source into the memory
behind the panels before they write, in pieces of 8000h bytes, as many
as the room holds, rounded down to 512 bytes. A piece is read by calls
of DOS until it is whole or DOS gives nothing; a file gives a piece in
one call, a device may give it in several. The room left is counted
down behind that loop, by what the last call gave, while the buffer
goes on by the whole piece: of a device that gives a piece in several
reads, the room is counted down by less than was read, and the reading
goes on behind the room's end for as long as the device gives, into
whatever lies behind the original's block of the memory. That is known
by reading the assembly, `C20_10` to `C20_13` of `VCCOPY.INC`, and was
not tried.

**What the portable C does.** Every read counts the room down, so a
piece that came in several reads takes as much of the room as it is
long, and the reading stops at the room's end; what was read is then
written and the copy goes on, as it does for a file that is larger than
the room. A file, and a device that gives each piece in one read, are
copied as before. `C20_00` of `copy.c`.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The program of 32
bits wrote into its own memory behind the room there, and from the
memory of the extender on it would be ended by the extender for it. The
cluster N9 met it where it counted the room in bytes (`TODO.md`, "N9,
`copy.c`").

**Which scenarios show it.** None, and none can on the bench. The one
device that a scenario copies from, `nul` in `copy`, gives nothing; the
console, which gives a line at a time, is refused as a source, by the
original and the portable C alike; and the bench has no device that
answers a read with less than it was asked for and more after it.
`copy`, `copy-big` and the other scenarios of the copy are held to the
original's goldens as they are.

### 18. The viewer shows a text that it finds beyond column 65,535

**What the original does.** The viewer counts the columns of a line in
16 bits, a tab going on to the next multiple of eight. Where F7 or
Shift-F7 finds a text, it counts the columns in front of it from where
the line begins, and moves the first column that is shown so that the
text is on the screen. A part of the file in the memory is 0FFE0h bytes
at the most, so a line passes 65,535 columns only with tabs: with more
than 8,191 of them in front of the text, the count wraps round, the
first column shown is set near the wrapped number, and the screen shows
tabs of the line where the text is not. Drawing a line, the viewer
counts its columns in 16 bits as well, which tells where the search has
put the first column within 80 of 65,535. That is known by reading the
assembly, `V15_10` to `V15_21` and `V32_00` of `VCVIEW.INC`, and was
not tried.

**What the portable C does.** Both counts are of 32 bits and do not
wrap: the text found is on the screen, and the status line says its
column, which has room in the seven cells that the original has for
it, some 524,000 at the most. The Right key still stops at column
65,455, 0FFAFh, a limit of the program's own; Left goes back from a
column beyond it. A line of fewer than 65,536 columns is shown as
before. `V00_10`, `V25_00` and `V32_00` of `view.c`.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The cluster N4
met it where it gave the viewer's numbers their widths (`TODO.md`,
"N4, `view.c`, its second part", and question 11).

**Which scenarios show it.** None. `view-search`, `view-keys` and the
other scenarios of the viewer have lines of a few columns, and are held
to the original's goldens as they are. A scenario that showed it would
view a file with a line of more than 8,191 tabs in front of a text, and
search for that text: some 8 KB of tabs, which the bench's disks do not
have, and which a scenario would have to type into the editor, or make
by doubling a line of tabs with the editor's blocks, as `editor-big`
makes its file. Its golden, from the original, would show the wrong
columns, and its patch the rows of the status line and the window.

### 19. The Info panel counts every file in its whole clusters

**What the original does.** The Info panel says how many files the
other panel has and how many bytes they take on the disk, each file
counted in whole clusters. It counts in 16 bits: the clusters that the
high word of a size fills by multiples of the cluster's size first,
then those of the rest, a quotient of 16 bits, and one more if there is
a remainder. Where that quotient is 65,535 and there is a remainder, the
one more wraps it round to 0: a file whose size lies inside the last
cluster below a multiple of 65,536 clusters, and not at its end, counts
65,536 clusters less. On a disk of 2K clusters a file of 134,217,000
bytes counts 0 bytes, where it takes 134,217,728; on one of 512 bytes, a
file of 33,554,000. That is known by reading the assembly, `PtIn02` to
`PtIn04` of `VCSUB3.INC`, and was not tried.

**What the portable C does.** It rounds the size of each file up to
whole clusters in 32 bits, so that such a file counts all its clusters.
Every other file counts as in the original, and the sum wraps at 4 GB
as the original's does. `PutInf` of `info.c`.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The cluster N12
met it where it sorted the numbers of `info.c` (`TODO.md`, "N12,
`panel.c`, `info.c`, `drive.c`", question B). Its other question there,
a cluster of 64K, which the original keeps as 0 bytes, is decided to be
fixed too, by the piece M, and is declared then: it rests on this
count, which divides in 32 bits.

**Which scenarios show it.** `enter` and `view-window`, by their
patches, which have the line of the files of `C:\STAMPS` on the Info
panel, under the viewer's window in `view-window`. Of its seven files,
whose sizes the bench writes into their entries, `LAST.DAT` has
4,294,967,295 bytes, which on the bench's clusters of 512 bytes lies
inside the last cluster below 65,536 times 128 clusters: the original
counts it 32 MB short, and the portable C counts its 4 GB. The sum of
the seven wraps at 4 GB in both, as a size does wherever the program
shows one, so the line says 2,001,050,624 bytes where the original says
1,967,496,192, and neither says the 6 GB that the files take. The
other scenarios that show the Info panel are held to the original's
goldens as they are.

### 20. A panel keeps all of its files while a command runs

**What the original does.** Before a command it gives its main data and
its panels back to DOS, and keeps of each panel, in the part of it that
stays in the memory, the start of its list and the names and attributes
of 256 entries, `..` among them. A longer list loses its end. If the
cursor is on an entry that is dropped, that entry is put into the last
place that is kept, and the first entry shown, if it is dropped too,
into the place before; both lose the marks of being selected. When the
command is done the panel is read again, and what was selected is
found by name among what was kept: what was selected beyond the 256th
entry, and the two that were moved, are no longer. `F21_40` of
`VCFUNC.INC`, which the setup, entered as a command is run, goes
through too. With 288 files selected and the cursor on the last, 253
are selected after a command.

**What the portable C does.** The panels stay where they are, whole,
while a command runs, and nothing is cut: a panel that is read again
behind the command finds every file that was selected and selects it
again, the cursor's and the first one shown among them. The same when
the setup is left. `F21_31` of `enter.c` and `StSetup` of `stmain.c`
no longer cut; `F21_40` is gone.

**Why.** The user's decision, on 13 October 2026, as `CLAUDE.md` has it
under "The portable C": the cut was there because the original gives
its memory up for a command, which the portable C does not (entry 1),
so it is the removal of what only made sense on DOS.

**Which scenarios show it.** `enter-many-selected`, which selects all
288 files of a directory, with the cursor on the last, and runs a
command: the original then shows 253 selected, with none of those on the
screen, and the portable C 288, the eighteen on the screen and the
cursor's file among them, until the end. Its patch has those rows.
`enter-many` runs a command from the same directory with nothing
selected, which shows the same in both, and no other scenario has a list
longer than 256 entries when a command runs or the setup is entered.

### 21. A tree's file that says it has no entry, or 4,096 or more, is refused

**What the original does.** The tree of directories is kept in
`TREEINFO.NCD` in the root of its disk, which says how many entries its
list has, with a sum of its head and one of the list. The program holds
the file's length against the bytes of that many entries, in 16 bits,
and takes it if they agree and the sums are right. Of a count of 4,096
entries or more the bytes wrap round, so such a file is held to the low
twelve bits of its count, and is taken if it is as long as that says:
the program then has the count that the file says, and shows that many
entries of `TreeBuf` and of what lies behind it in the main data. And a
file of no entry is as long as its head alone and is taken: the tree is
empty then, without even the root, and a click on a line of the tree's
box puts the cursor on the line under the mouse, which is no entry.
That is known by reading the assembly, `RdTr02` and `ChTr67` of
`VCSUB3.INC`, and was not tried. No program writes such a file: the
original's own has five entries or more.

**What the portable C does.** Such a file is taken for a broken one, as
one that is too long or whose sums are wrong is: the disk is read, and
the file written anew. Every other file is taken as before. `ReadTre`
of `treebox.c`, and the cursor of `ChTree`, which a list of no entry no
longer reaches.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed. The program of 32
bits counts the bytes of the list in 32 bits, which refuses a count of
4,096 or more by itself; the cluster N13 met it where it widened that
count, and had kept the cut of 16 bits until this was decided
(`TODO.md`, "N13, `tree.c`, `treebox.c`, `extfile.c`").

**Which scenarios show it.** None. A scenario sets up what it needs
through the program's own menus, and such a file has bytes that are no
characters, a count and two sums that are worked out, which the editor
would have to be typed into by their codes; nothing of the original's
writes one. `tree`, `tree-box`, `tree-choose`, `tree-deep`, `copy-tree`
and `mouse-tree`, which read a tree from the disk and from its file, are
held to the original's goldens as they are.

### 22. The Info panel goes by whether reading the label failed

**What the original does.** `RdInfo` of `VCSUB3.INC` reads what the Info
panel shows of the current drive, the label last, by `GetVol`, and takes
the panel for one that could not be read where `GetVol` answers with the
carry flag set and 53h in AX, the error of a drive that is not ready.
But `GetVol` sets the flag for every local drive, also where it has
found a label, and AX then holds what DOS left in AH at the call that
found it, 4Eh, with the eleventh character of the label in AL, as
`Convert` has made it a capital. So a label of eleven characters whose
last is an S, `GAMES_DISKS` say, on a DOS that leaves 0 in AH there, is
taken for a drive that is not ready. The panel still shows all that was
read, the label too; what differs is that it stays marked as changed,
so that it is read from the disk again every time the panels are drawn,
where it is otherwise read only when something has changed. That is
known by reading the assembly, and was not tried: what DOS leaves in AH
when a search succeeds is not said of the call, and the bench was not
asked.

**What the portable C does.** The panel counts as not read only where
looking for the label failed with that error. `GetVol` of `read.c`
answers whether it failed apart from the error, and `RdInfo` goes by
that; `GetVol` gives no error for a label that it has found.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed.

**Which scenarios show it.** None. Every disk of the bench has a label,
`VOLKOV`, `PROGRAM` and `WORK`, which the bench makes and a scenario does
not change, and none of them is eleven characters long. The program's
own way to a label, Ctrl+F4, makes none on FreeDOS, which keeps the
label that the disk has and reports no error (`label`). The scenarios
that show the Info panel are held to the original's goldens as they are.

### 23. A help file whose entry has a text of no bytes is refused

**What the original does.** An entry of the help file, the index or a
page, says how long its text is, and the program reads that many bytes
and takes the mark of 1Ah off each, by `DeCode`, which counts with
`LOOP`: a text that the entry says has no bytes is read as none, and
then decoded as one of 65,536 bytes, all of the segment from the text
round to it again. The main program takes such an entry, refusing only
a text too long for its room, and changes the whole of its main data so;
what it does from there is by what that makes of it. With the third
page of the help file made so, it shows the index again, a row higher,
and comes back to wait for a key. The setup program does the same with
the index's text, if its entry says that it has none, and with its own
pages when the page of the index behind them begins where they do, so
that there is nothing to read between: its one segment is its code as
well, and it changes its own code as it runs, and does not come back to
wait for a key on the bench.

**What the portable C does.** Such a file is refused as one that cannot
be read: the help says "There was error reading data from the help
file." as it does for a page that is not where the index says or too
long, `RdEntry` of `help.c`; and the setup goes on without its pages,
which F1 then says it has none of, as it does for any file that it
cannot read, `StRdHelp` of `sthelp.c`. In a flat memory the 64K that
the original goes round would be written behind the text, past the end
of the main data. What the help does with the numbers of the lines of a
file that is not true to its text, which wrap at 65,535 as the
original's do, is not changed: the user decided that it stays.

**Why.** The user's decision, on 14 October 2026, as `CLAUDE.md` has it
under "The portable C": a bug of the original, fixed.

**Which scenarios show it.** None. Such a file can be made through the
program: the editor types the bytes of the help file in hex, and a page
whose length is typed as 1A 1A, the mark on 0, is one. But what the
original does then rests on every byte of the 64K that it goes round,
and the C translation, which is taken through every scenario of the
main program, has other bytes there past its main data and shows
another screen: such a scenario was made, and is not in the tree for
that. The setup's two places have none either: the original's setup
program does not come back from such a file to wait for a key, so no
golden can be derived of it, as was tried with both on the bench.
`help-broken` and `setup-help-file`, which have the help refuse files
for other reasons, are held to the original's goldens as they are.
