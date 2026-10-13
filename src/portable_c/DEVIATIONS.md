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

**What the original does.** Its Options menu has a switch, Memory
allocation, which is on as the program is distributed. With it on, the
program gives most of itself up while a command runs: all but a small part
of it, which stays in the memory, runs the command and then loads the rest
again from the program's own file, run as a second copy that moves its
code into place and ends. The part that stays knows the file by a check
sum that it took when it was loaded. If the file is not there any more, or
is another, it says "Bad or missing" with the name of the file and "Press
ENTER to try again or ESC to abort", and Esc ends the program. With the
switch off, the program keeps its code in the memory and gives only its
main data back to DOS. Shift held with Enter turns the switch round for
that one command. The Info panel says how much memory is free: with the
switch on, what a command gets when the code is given up; with it off,
what is left with all of the code loaded. And the map of the memory,
Alt+F5, lists the block of the code as free memory when the switch is on,
since a command will find it free.

**What the portable C does.** What the original does with the switch off,
always. There is no switch:

- The Options menu has no line Memory allocation. It has fifteen lines
  where the original has sixteen: Save setup stands a row further up, and
  the box is a row shorter. The letter M chooses nothing there.
- The program never loads itself again, takes no check sum of its file,
  and does not know a second copy of itself: it never says "Bad or
  missing", and goes on if its file is deleted while a command runs.
- Shift with Enter is Enter.
- The Info panel says how much memory is free with all of the code
  loaded.
- The map of the memory lists the code as the program's: the program has
  a block more there than in the original as it is distributed, and one
  interrupt more among those it has taken, 24h, DOS's critical error,
  whose routine is in that code.
- The byte of the setup file that held the switch is kept as it is found
  and written back as it was; the program does not go by it. A setup file
  of the original is one of the portable C and the other way round. The
  original's setup program, `VCSETUP`, which is not translated, still
  shows the switch and writes the byte.
- The original's help file, which the portable C reads, still has its
  page on Memory allocation; no item of the menu leads there.

What a command has less of the memory for that: the code, which stays.
On the bench of the tests DOS has 296,368 bytes for a command run from
the portable C, where it has 412,848 for one run from the original as it
is distributed and 405,824 for one run from the C translation. The main data is still given back to DOS for a command and
asked for again afterwards, as in the original.

**Why.** The user's decision, on 12 October 2026: giving up the code and
loading it again saves memory that only DOS is short of, at the price of
reading the program's file after every command. It "only matters on DOS";
"it's OK for our port to be less memory- and disk-efficient, as long as
it's functionally equivalent"; and the option "in the new tree goes away.
It's going to be a pattern for a lot of features that only made sense on
DOS."

**Which scenarios show it.** Each of these is held against a golden of the
portable C's own:

| Scenario | What changes in it |
|---|---|
| `lost` | The program's file is deleted by a command. The original asks for it, twice, and ends; the portable C is back at its panels, and the keys that answered the original are keys at the panels: a space into the command line, Enter, which runs it, and Esc, which switches the panels off |
| `menu`, `clock`, `clock-russia` | The Options menu where it shows: a line shorter, without Memory allocation. In `menu`, Up from Save setup goes to the clock |
| `memory-map`, `memory-map-russia`, `menu-items`, `mouse-memory`, `mouse-middle`, `mouse-drag-lists`, `stay`, `ended` | The line of the program in the map of the memory: three blocks where the original has two, and 24h among its interrupts |
| `memory-map` | Besides: the two steps that switch Memory allocation off and on again, around a second look at the map, are left out |
| `quick` | The step that switches Memory allocation off before the last command is left out; every screen is as the original's after the same step |
| `passed-on` | The step that switches Memory allocation off in the second copy is left out, which the scenario has only because the C translation, being larger, cannot load itself again there; every screen is as the original's after the same step |

A step that switches Memory allocation, "F9 o m", is left out of every
scenario for the portable C: there it would open the Options menu and
leave it open.

**Not run for the portable C:**

| Scenario | Why |
|---|---|
| `memory` | It switches Memory allocation off, for the Info panel to show the other number, and shows the menu without the mark |

**In the data.** The test `portable_c-data` holds five tables of the menus
against the original's with this difference declared for each, in
`DECLARED` of `harness/datacheck.py`: the lines of the menus, the pages of
their help, the size of the Options menu, its commands, and the item that
Shift-F9 chooses in it.
