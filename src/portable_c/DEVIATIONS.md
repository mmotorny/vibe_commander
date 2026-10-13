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
  than the program itself, 181K, DOS does not load it, and says so in
  its own words.
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
- It is an `.EXE` file of 185K, with the main data in it, which DOS
  loads and relocates. A DOS before 3 is left through the INT 20h of the
  PSP, as the original leaves it, by a call instead of a return; the
  call by which the program asks DOS for its PSP is one of DOS 2 and
  later.

What it costs in memory. While a command runs the portable C keeps
194,800 bytes, the program and its panels, where the original keeps
12,704 and the C translation 19,728. On the bench of the tests DOS has
230,736 bytes for a command run from the portable C, 412,848 for one run
from the original as it is distributed, and 405,824 for one run from the
C translation. That is enough for every command of the scenarios but
those of a second copy of the program: the bench has 500,128 bytes from
where the program is loaded, two copies with the COMMAND.COM that runs
the second take 463K of it, and the COMMAND.COM that a command of the
second copy wants is 73K more. So a second copy starts there and shows its panels, and has no
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
