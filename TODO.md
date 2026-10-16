# To do

What is known and not done: plans, open questions, pieces not done, and
what is untested. Standing instructions are in `CLAUDE.md`; what was
done, and what the work met on the way, is in `DIARY.md`. A piece that
is done moves its account there. A section that the code or a document
cites in `TODO.md` and that is not here was moved there: `DIARY.md`
names the section that each entry came from.

## The setup program: the bugs of the original that stay

Each as it is, until its fix has been decided; the first two were seen
on the bench.

- The pages 6 and 7 of the help, Country info and Exit, are "Help
  contents is not available", with a beep. `RdHelp` reads the seven
  pages into what is left of the program's 64 KB, 8,155 bytes behind
  `HelpBuf`, which is at DF03h in the original, and the pages of the
  help file that was shipped are longer than that together: five fit.
  To show the same, the portable C has to go by that number.
- `Err2`, behind Ok in S0, is to fill `SrcIni` with FFh, so that the
  defaults count as a change and are offered for saving. It does so by
  `REP STOSB` with CX as it happens to be, which is 0 on every way
  there: nothing is stored, and defaults that are left as they are are
  not saved.
- Cancel in S10 leaves, as Save does, and does not go back to S1.
- A file that could not be written is said in a line on what DOS has
  shown, which the panels of a running copy cover at once, and the
  running copy takes the settings all the same. `setup-save-fails`
  holds that the program takes them and that the file stays; the line
  itself no scenario can hold, see "Piece D".
- A second click on a line of the menu enters the item under the
  cursor, not the item that was clicked: `Main50` says only that it
  was a second click, and `Main32` goes by the cursor. They differ
  when a key has moved the cursor between the two clicks. Seen on the
  bench, and `setup-mouse` has it.

## The portable C: one program

The portable C stays in the memory whole while a command runs
(`src/portable_c/DEVIATIONS.md`, the first entry), and is one program:
the linker writes its file, DOS loads and relocates it, its main data is
a variable, and it has no part that stays, no copy of anything and no
tool that lays it out (`docs/portable-c.md`, "What rests on Open
Watcom", has what is left of that, and `DIARY.md`
what went). What that has
left open:

- No scenario has the program without memory for the viewer's 8,000
  bytes after a command, where it says "Program too big to fit in
  memory" and leaves (`TakeMem` of `src/portable_c/start.c`, `Round` of
  `main.c`), nor a DOS before 3.20, which the bench is not.
- Names that say what no longer is: `FreMem2`, which is how many paragraphs it is from the main
  data to the end of the block of the panels and the viewer, so that
  the eight places that add it to the main data's segment for where the
  memory ends still do; `Init11`, `Exec10`, `F21_40` and `Alloc0`, which
  do less than in the original. They were left for the tables of the
  harness, which name the functions of both trees by one list, and for
  the files that other work has in hand.

## The portable C: what the map of the memory has left

The map of the memory, Alt+F5, and the watch on the programs that stay
in the memory are gone from the portable C
(`src/portable_c/DEVIATIONS.md`, 7). What that has left open:

- The original's help file, which the portable C reads, has its pages
  on the map of the memory, on Memory allocation and on EGA lines
  still. The file is the original's; a help of the portable C's own is
  not in hand.

## The portable C: what EGA lines has left

EGA lines, Alt+F9, is gone from the portable C, on the user's word of 13
October 2026 (`src/portable_c/DEVIATIONS.md`, 12): `F69_00` of
`commands.c` with its key, its line of the Commands menu and its name in
the key bar, and `ScreenSmallFont` and `ScreenPrintAll` of `sys.h`.
`src/c/` and `src/asm/` keep it, and every scenario and golden of the
original is as it was. What that has left open:

- The rows of the screen are to come from the system, later. Today the
  program finds them as the original does, in `IniData` of `start.c`,
  when it starts and behind every command: `ScreenEga`, and of such a
  display `ScreenFont().last`, 50 at the most. That is where a system's
  answer comes in, and those two functions are of the PC and go then.
  The 50 is `MaxLins` of `vc.h`, which is how long the screen buffer
  and what is kept of the screen are (`ScrBuf`, `UserScr`, `WinBuf`,
  `ErrBuf`, `InfBuf`): more rows than that want more than a number, and
  so does a screen that is not 80 columns, which is written in
  everywhere.
- Print Screen on a tall screen prints what whoever made it tall told
  the BIOS: the program says nothing of it. Nothing was tried.
- `MouseRows` of `sys.h` is asked only where the screen has other rows
  than 25, by `IniMous` of `mouse.c` and by the setup's start in
  `stmain.c`: no scenario of the portable C has it asked now. The toy
  of `harness/trial32` still asks it.

### The tall screen: untested in the portable C until a system gives it rows

On the user's word of 13 October 2026: "We don't need to test 50 row
screen for now, we can solve this problem when we have our first modern
os implementation with variable terminal width and height." EGA lines
was the one way that a scenario had to a screen of more than 25 rows,
so for both builds of the portable C no scenario is on one now. The
program's own handling of the rows that it finds is in the code,
untouched, and is not run by any test.

What was dropped, to be brought back when a system gives the screen its
rows. Every scenario is still run for the original and the C
translation, and its golden has the screens:

| Scenario | For the portable C | What of the program it was the test of |
|---|---|---|
| `setup-lines` | Not run | The setup on 50 rows over what DOS has shown: the 50th row of that, which the setup keeps apart (`StUserScr` of `stmain.c`); the mouse given the rows where the setup starts; and that the key bar does not write an 81st cell (`DEVIATIONS.md`, 9), which shows on 50 rows alone. Its patch for that one cell went with it, and is in the history: `harness/goldens/portable_c/setup-lines.patch` |
| `setup-colors-lines` | Not run | The setup's list of colours with room for all 39: no second page, a click below the last item, and the rows below the picture, which is of 25 rows (`md.Lines > 25` in `stcolor.c`) |
| `setup-help-lines` | Not run | The setup's help in a window of 38 lines: its last lines, its thirds for the mouse, a page up |
| `help-keys` | The last five steps left out, of 40 | The help's index of 38 lines, End there, and F10 out of it |
| `mouse-help` | The last nine steps left out, of 29 | The mouse on 50 rows: F1 of the key bar clicked in row 49, a line below the 25th row, a button held below the lines and below the text, the last line shown clicked twice, the middle button, and the pointer left on row 40, which 25 rows have not: `MouseRows`, by `IniMous` of `mouse.c` |
| `scroll` | Two steps left out, of 23 | The panels drawn on 50 rows and on 25 again, with the menu bar always visible (`ROWS` of `place.c`, a row more on a tall screen) |
| `user-menu` | Four steps left out, of 83 | The box of the user menu in the middle of 50 rows: `SftEGA` of `window.c`, by `usermenu.c` |
| `history-drives` | Four steps left out, of 70 | The box of the history in the middle of 50 rows: `SftEGA`, by `cmdline.c` |
| `menu-panel` | The last four steps left out, of 18 | The command line left high up when 50 rows become 25 with the panels off. That was EGA lines' own doing and is gone; what a system would bring back is the same question behind a command that leaves the screen shorter, which `IniData` answers |
| `menu-items` | Two steps left out, of 43 | EGA lines chosen from the menu, which is gone. Nothing to bring back |

The branches that only a screen of more than 25 rows takes, none of
which a scenario of the portable C runs now: `SftEGA` of `window.c`,
which puts a box in the middle of such a screen, for the history, the
user menu and the box of the drives, and the same in `DialBox` of
`dialog.c` for every box and dialog; `ROWS` of `place.c`; `MouseRows`
in `IniMous` and in the setup's start; the 50 that `IniData` takes at
the most; the 50th row and the rows below the picture in the setup.
The box of the drives and a dialog on a tall screen had no scenario
before either.

How they come back. A scenario needs a step that gives the screen its
rows in the tree, in place of the original's Alt+F9 (`in_place_in` of
`harness/deviations.py`, one step for one step), and the program takes
the rows that it finds behind a command. On DOS that would be a command
that sets them. The bench has none: its floppy is FreeDOS's boot
floppy, which has no `MODE.COM`, and of the programs that it has only
`VFONT` loads a font, from a file, of which the floppy has those of 16
lines. FreeDOS's `MODE` is in the Lite image that the harness downloads
already, `packages/base/mode.zip`, beside the keyboard driver and the
mouse driver that `harness/setups.py` takes from there. It was not put
on the bench: a file more on A: changes what the original shows of
that disk, so goldens of the original would be derived again, which is
asked first; and whether the original, given that command in place of
Alt+F9, then shows what its golden has was not tried, since the
user's word came first. It is known that it would not everywhere: such
a command sets the mode and clears what DOS has shown, where EGA lines
keeps it, so a scenario that shows that afterwards, `setup-lines`
above all, would differ there.

## The editor's room: untested in the program of 32 bits

The editor of the portable C has room for a file that the original's has
none for (`src/portable_c/DEVIATIONS.md`, 13), and the scenario that has
the editor refuse such a file, `editor-big`, is not run for it, on the
user's word of 13 October 2026 (`CLAUDE.md`, "Deviations"):
`TREES["portable_c"]` of `harness/deviations.py`, and so on Linux and on
the web, whose trees are built on that one. The C translation is taken
through it as it was. This is what the scenario alone tested, and so
what no test holds of the portable C; what two scenarios that it passes
no longer get it to; and what would test all of it again. It stands
behind the tall screen's section, which is of the same kind: what a
scenario that is no longer run was alone in testing.

**The room.** Since the piece F9 of step 2 the program for DOS takes the
memory for its panels and its text from the extender, as much as the
machine has, and its editor has all of it but its own objects and its
two panels: on the bench of the tests, a machine of 16 MB, the figures
of `docs/portable-c.md` have how much, many times the original's room there (345,840
bytes) and more than any file of a scenario. On a machine of 2 MB, which
a scenario may ask for (`memory` of `harness/scenarios.py`; `view-window`
does), it is some 550 KB. The program for Linux and the web page have
what the host gives them, at most 256 MB on the page since the cap
("The cap of the memory: done"; W10 of `DEVIATIONS.md`). Until F9 the program
of 32 bits handed out 640 KB of its own, and what was written here was
reckoned from that; `DIARY.md` has the figures as they were measured
then.

**So this is untested in the portable C**, each of it compiled from
statements that the scenario holds of the C translation:

| | What | What still holds a part of it |
|---|---|---|
| 1 | That the editor refuses a file a little larger than its room, and one that is far larger, where the window is the whole screen: the box over the panels | `view-window`: the box for a megabyte, in the window of the Info panel, on a machine of 2 MB |
| 2 | View from that box: the viewer with its status line on the file that was refused | `view-window`: View, in a window without a status line |
| 3 | The box left by Esc and by Cancel: `closed:` of `T00_00`, which nothing else runs | Nothing |
| 4 | A file that gives fewer bytes than its directory entry says, opened as long as it gave: `T00_11` | Nothing |
| 5 | A text found more than 32K into the editor's text, and found again | `editor-search`, in a short text |
| 6 | The editor on a text of 87K | `help-broken`, `setup-help-file`: 69K, a line of that length |

**Two scenarios that pass and do not get there.** The same memory is
what the editor fills and what a copy reads into, and two scenarios
that are run for the portable C were made to reach its end. They pass,
since a count of free memory is left out of the comparison (`IGNORED`
of `harness/deviations.py`) and their steps were written to end on the
same screen however much room a build has, for the C translation, which
is full a few keys before the original. Nothing says that they no
longer test what their titles say.

- `editor-full`, "with all of its memory full". The original has some
  268K free with the long line made, and none after the steps that put
  a line back as often as they have keys for and `"ab"`; the C
  translation the same, a little less to begin with. The portable C is
  never near full there, with the bench's room as with the 640 KB that
  it had when this was first seen (it had some 248K left after
  `"ab"`). What that leaves untested: `T51_00` where a line has grown
  and there is no room for it, with its beep and the line cut ("Known
  and not dealt with" has what the original does then), and a byte
  typed in hex behind the last one with no room.
- `copy-big`, "one that the memory does not hold". The file is 526,632
  bytes, six times `command.com`, and a copy reads into what the editor
  has for a text, cut to whole pieces of 512 bytes (`copy.c`, `left`).
  The original and the C translation read it in two parts; the
  portable C, with the bench's room, in one, and on a machine of 2 MB it
  would still have room for all of it. Untested then: a file read and
  written in more than one part, and the question whether to overwrite
  asked with only a part of the file read, which the scenario answers
  with Skip, by the comments at its steps. (For a day, from the piece
  F1 to F9, the panels in blocks of their own left the 640 KB a room
  short enough that the file went in two parts again, and both were
  tested.)
- The list of the file search is in the same memory. No scenario fills
  it in any build.

Both are of entry 13 too, which names them. Neither is declared in the
harness, which has no word for a scenario that is run and does not
reach its point.

**What would test it again.** None of it is for the piece that wrote
this. What the portable C does with a text that the original has no room
for, which `editor-big` does not run, `editor-huge` holds since the
piece H, to an expectation of the portable C's own: the editor opens
such a text and has all of it.

| Way | What it tests again | Against |
|---|---|---|
| For the portable C the five steps 28 to 32 are left out of `editor-big`, `left_out_of`, where the scenario is not run for it now | 3 to 6 of the table: View and Esc and Cancel from the box, and the file shorter than its entry, but for what the room now changes | The user's word is that the scenario is not run for the portable C: this is another thing than was decided, and is asked first. And with the room what the machine has, `noon.dat`, which says a megabyte, is no longer too large on the bench: steps 39 and 40 would open it where the original refuses, and have to go the way of 28 or be declared. Tried while the room was 640 KB, when it passed; not tried since |
| A scenario of its own on a machine of 2 MB, `memory`, with a file larger than the portable C's room there and so than the original's: eight times `command.com`, 702,176 bytes | 1 and 2 for the program for DOS, with nothing declared | A new golden of the original, which has to come out on that machine as on the bench's; drive C: of the bench has to hold the file. The program for Linux and the web page have the host's room, larger than any such file: there it is cut before the box, or not run, declared |
| `editor-full` and `copy-big` on a machine of 2 MB, `memory`, with as many steps more as fill some 550 KB, and a file of more than that for `copy-big` | What those two no longer reach, for the program for DOS | Both scenarios change, so both goldens are derived anew, on that machine, and every tree is taken through the longer steps; on Linux and the web they again do not get there |

## The portable C: without Quick execute commands

Quick execute commands is gone from the portable C, the setting and
the way through INT 2Eh, on the user's word of 13 October 2026
(`src/portable_c/DEVIATIONS.md`, 14). Every command is run by a
COMMAND.COM of its own, with `/C`. `src/c/` and `src/asm/` keep the
feature, and `quick` is run for the original and the C translation.
What is open:

- The layout of the two dialogs, as "Piece E2" has it for the setup's:
  the Advanced options have a group of one line now, Other options
  with Clear keyboard buffer alone, and the right half of the
  Configuration is five rows shorter than the left. Arranging either
  anew is the user's to decide.
- The original's help file still explains Quick execute commands, in
  the page of the Advanced options and in the setup's page of the
  Configuration. The file is the distribution's; a help of the
  portable C's own is not in hand.

## The extender when the program is left: the user's

The piece L (`DIARY.md`) found what DOS/32A does when the program is
left, and the user had the difference declared as it is
(`src/portable_c/DEVIATIONS.md`, 7). Two things it left to the user,
not decided:

1. Whether the variable `DOS32A` is to be read at all: bit 0 of the
   byte 15h of the extender's settings. Nothing was changed there; with
   that bit clear the program would go by its file alone.
2. The mouse driver told to call nobody when the program ends itself,
   by the extender. Entry 7 says it among what was read, and that it is
   not among what was decided.

## The program for DOS: what was never looked at

What the program of 32 bits does otherwise than the original is
declared, each difference as the user decided it (`DEVIATIONS.md`, 1
and 7; the list that the piece W made of them is in `DIARY.md`). What
no piece looked at or tried:

- What DOS leaves in AX where a call did not fail, on a DOS that changes
  AX there: at the calls that the extender translates, 39h, 3Ah, 3Bh,
  41h, 4300h and 56h, it may be the AX that the call was made with.
  FreeDOS leaves AX alone there, so the bench cannot tell. Not known to
  differ on any DOS; nothing to decide today.
- Hosts that the bench has not, EMM386 and a DOS box of Windows, where
  the extender goes into protected mode another way.
- What a DOS before 3 and a machine without a 386 are told, by the
  extender; and DS and ES of real mode at the calls of the BIOS and of
  the mouse driver.

## The flat memory: what is left

Step 2 is done; its plan and pieces are in `DIARY.md`. What is left:

- The viewer's column of a text that is found beyond 65,535 is fixed
  (`src/portable_c/DEVIATIONS.md`, 18), and no scenario shows it: one
  that types 8,192 tabs into the editor is long, and whether a scenario
  can make such a line with the program's own commands is to be seen.
- Whether the layout of the main data is ever freed: `vc.h` still holds
  `struct MainDat` to the original's, `LAID_OUT(MainDat, ...)`, which
  the tables of the harness and the data's image go by.
- The proper test of a limit, with the system's answer made for the
  test: "An implementation of `sys.h` for tests", below.

## An implementation of `sys.h` for tests: future work

The road of the portable C (`CLAUDE.md`, "The portable C") has DOS as
the layer at its step 3, `sys.h` as it is, the implementation for DOS
behind it at step 4, and at step 5 a stub, which has gone since. An
implementation of `sys.h` made for
tests would be another: one that does run, on this
computer, built by clang, with the common code of the program above it,
and that answers the program where a system would. It is future work,
on the user's word of 13 October 2026, with everything that it would
give written down here. It is not started, and is not to be started
without the user. Making it in place of the stub of step 5 was
proposed, and not taken.

What is here was found on that day by reading the tree as it was at
7fc7a51 and by counting, with nothing of the program run. Of every
figure it is said whether it was counted, and by what, or is an
estimate:

- "IR" is counted in what clang wrote. clang 23 was given every file of
  `src/portable_c/` for 32 bits, `clang -m32 -ffreestanding -std=gnu99
  -O0 -S -emit-llvm`, with `__interrupt`, `__loadds` and
  `__declspec(x)` defined away, and `dialog.c` and the files of tables
  in a copy with the `LAID_OUT` lines taken out. It took 54 of the 55
  files, all but `resident.c` with its `_asm`, and nothing counted so
  is of that file. In what it wrote a script counted the functions, of
  each its instructions, which is what a share of the program is a
  share of below, its conditional branches, `br i1`, the cases of its
  `switch` statements, and the functions that it calls by name; two
  calls go through a pointer and were not followed. That clang reads a
  file says nothing of whether it would run: a panel is a segment's
  number there still.
- "Regex" is a pattern counted in the source without its comments, or
  in the listing of the original.
- "Read" is sorted by reading, one thing at a time.
- "The test's" is what the scenarios' test printed when it last ran
  before that day.

### What it would give

1. **The sanitizers under the steps of the scenarios that there are.**
   clang's address sanitizer stops a program at a read or a write
   outside the object that a pointer is of, on every path that a step
   takes and whatever the form of the read. The project has had to find
   such reads by other means: `harness/reads.py` knows a read behind
   the end of a constant by its form, or by an entry of `REVIEWED`
   made by reading, and `CLAUDE.md` tells what it was made for, five
   constants with a zero byte too many, `CrLf_S` among them, and
   `User_S`, copied as 80 bytes from another file and found last. In a
   flat memory, step 2 of the road, what lies behind an object is no
   longer what the original has there, so the sanitizer's list is the
   list of the places where the program goes by the layout of the
   memory. The sanitizer of undefined behaviour stops at a shift by
   too much, at a sum with a sign that does not fit, and at a number
   read or written at an address that is not even, which
   `harness/hostcheck.py` says at its top that it cannot see. For
   arithmetic it has two groups besides, which are no undefined
   behaviour and are switched on by name: `-fsanitize=integer`, which
   stops where a number without a sign wraps round, and
   `-fsanitize=implicit-conversion`, which stops where a number loses
   bits or changes its sign as it is put into a narrower type. Under
   the steps of the scenarios those say every place where a number
   does wrap or is cut, which is the reading list for step 2 of the
   road, what is to stay 16 bits and what is to be widened: a place
   that the program counts on stops like one that is a fault, so it
   is a list to read and not a test. What none of them finds is
   wrapping that is missing: a number that wrapped round at 16 bits
   on DOS and, in an int of 32, no longer does. Nothing overflows
   there, the result is another, and only a comparison says so: the
   scenarios, and 8 below. The memory sanitizer stops at a value that
   is used and was
   never set: `SetStat` and the entry handed DOS a `name` that nothing
   had set, DS and DX as the stack had them (`CLAUDE.md`, "Moving a
   file to the system's functions"), and the audit with clang's
   warnings named two variables, both set. The first two sanitizers
   are there for a program of 32 bits and the third for one of 64 only:
   that is what is known of clang, and was not tried.
2. **The suite in seconds, on every build and in a CI.** An estimate:
   nothing was run. What it rests on was measured, in "Ways of testing
   that were tried": 77% of a scenario's time is the machine running
   while the harness waits, most of it the program waiting for the
   timer while a box grows. A clock that the implementation moves on
   when the program says that it is idle takes that time away, and no
   machine is started. The two tests of the scenarios take a little
   under two minutes each today.
3. **Mutation testing that can be paid for, which measures what
   coverage does not.** That a branch ran does not mean that a screen
   shows what it is for. `harness/notice.py` asks that of one change at
   a time, with a build of a copy of the tree and a run of the
   scenarios for each, minutes; it found three branches that no
   scenario noticed in one batch of nine routines ("What batches have
   lost time to"), and the table of the blanker's stars was three bytes
   wrong with every scenario passing. One change to each conditional
   branch of the program is some 3,300 runs: at seconds a run that is
   an afternoon, an estimate, and at minutes it is not done. What
   comes of it is the list of the branches that the goldens do not
   watch.
4. **The first measurement of the C's own branches.** What is measured
   today is the instructions of the original that ran, 22,603 of the
   22,981 that the 50 batches translate, 98.4% (the test's), and of the
   C only that each function of a batch was entered
   (`check_translation` of `harness/ported.py`). What is not measured,
   of the original or of the C, is which way a jump went, and it cannot
   be had from what is recorded: QEMU logs a block once, when it
   translates it. The C has 3,297 conditional branches and 420 cases of
   `switch` statements in its 521 functions outside `dos.c`, and
   `dos.c` has 5 conditional branches in its 102 (IR); the listing of
   the original has some 3,000 conditional jumps and loops (regex,
   3,008, and a line that the pattern does not take is not counted).
   The source has 2,406 `if`, 153 `while`, 439 `for`, and 215 `&&` and
   132 `||` (regex, the files of code). clang's coverage by the source
   says of every branch how often it went each way, `llvm-cov show
   --show-branches=count`. Nobody knows the figure today, and none was
   guessed. MC/DC is not worth its cost: the conditions came from
   single conditional jumps, and few are of more than one part. What
   to report is three lists that are never added up: the branches that
   a scenario of the bench runs, where the original is the judge; those
   that only a test on this computer runs, where reading is; and those
   listed with a reason, as `unreached` is today. Only the first is
   fidelity.
5. **The paths that the bench cannot produce.** `harness/ported.py`
   lists 378 instructions of the original as out of reach, with 94
   reasons (counted there; the batches of the setup program had none
   yet). Sorted by the kind of the reason (read):

   | Why no scenario gets there | Instructions | With an implementation for tests |
   |---|---|---|
   | DOS refuses a call that it grants on the bench: a file that it will not delete, attributes that it will not set, a directory that it will not go to | 79 | reached: the test says the answer |
   | Another machine: a CGA with snow, a network, a CD-ROM, no mouse, DESQview or TopView, no EGA | 60 | reached: the test says the answer |
   | A disk that is full, a write that comes up short, the next disk | 43 | reached: the test says the answer |
   | Memory that runs out, or more than there is room for | 42 | reached in part: where the program asks for memory and is refused |
   | Another version of DOS | 30 | reached: the test says the answer |
   | A drive that is not ready, the critical error | 19 | reached: the critical error is a call back that the program hands over since S2 |
   | The time of day at the end of a minute, an hour, a day | 14 | reached: the clock is the implementation's |
   | A country that neither bench has | 11 | reached, also by calling the routine with the country's record |
   | Code that nothing calls, or input that no step makes: markup that no text has, a file with a line end in every two bytes, a path of 68 characters | 80 | some 50 by calling the routine itself or with a file made for it (an estimate); the rest nothing reaches |

   So 298 of the 378 are of the kinds that such an implementation
   reaches. Some 50 of the 378 are of code that the portable C has not,
   or not in that form: the 19 of the map of the memory, and most of
   the 30 of the program's entry, which are for the block of the part
   that goes (read, and not held against `GONE`). And the 378 are 1.6%
   of the instructions: this is the last of the coverage, not a change
   of its kind. What runs there has no golden behind it, see the
   limits; "rests on reading" becomes "rests on reading, runs without
   a fault, and is held from then on".
6. **Unit and property tests of what reaches nothing of the system.**
   The 521 functions outside `dos.c`, by what each reaches of
   `sys.h`'s 84 functions, itself or through what it calls (IR; the
   share is of the instructions clang wrote):

   | Reaches | Functions | Share | Conditional branches |
   |---|---|---|---|
   | Nothing of the system | 189 | 22.0% | 644 |
   | The screen only | 33 | 2.7% | 90 |
   | The screen, the time, memory or the rest, and neither keys nor files | 40 | 7.8% | 241 |
   | Files, and no keys | 22 | 3.3% | 109 |
   | Keys or the mouse, and no files | 103 | 18.8% | 560 |
   | Keys and files | 134 | 45.4% | 1,653 |

   "Ways of testing that were tried" has 27% for the first row, of the
   C translation and by another way. The program draws into its own
   copy of the screen, `ScrBuf`, so much of what draws is in the first
   row, and a test reads the cells there with no implementation of
   anything. Those functions need `sys.h` for nothing but to be
   built on this computer. The candidates, each wholly or mostly in
   the first row:

   - `format.c`, all of it: `Number`, `PutSize`, `PutDate`, `PutTime`,
     `BufDate`. Handed a country's record, they run 10 of the 11
     instructions of a country that neither bench has; the eleventh is
     of `M30_60`, the dialog of the attributes.
   - `files.c`: `Sort`, `CmpPar`, `NearFl`, `FnFile`, `TstExe`, the
     order and the filter of a panel; and `mask.c`.
   - `extfile.c`, all of it: `FindExt` and `Item`, the lines of an
     extension file.
   - `tree.c`, all of it: `PutTree`, `TreeCur`; and `place.c`:
     `CorPar`, `AdrNum`.
   - `view.c`: `PrevLin`, `NextLin`, `V20_00`, `V32_00`, the table of
     lines and a text looked for. `edit.c`: `T53_00`, `T54_00`,
     `T72_00`, `T50_00`, the buffer.
   - `string.c`, `digits.c`, and the markup of `line.c`.

   Not among them: `copy.c`, of whose instructions two thirds are in
   functions that call the system themselves, 2,549 of 3,896, so what
   it decides it decides between two answers of DOS; and
   `usermenu.c`, of which nothing is in the first row. Those want the
   implementation, told what to answer.
7. **Fuzzing.** First of the files that the program reads, each into
   room of a fixed length: the setup file `VC.INI`, the extension files
   `VC.EXT`, `VCVIEW.EXT` and `VCEDIT.EXT`, the menu file `VC.MNU`, the
   tree's `TREEINFO.NCD`, `DIRINFO` and the help file; then of what a
   directory lists and of sequences of keys. `unreached` has such
   input as what no step makes: more directories than the tree has
   room for, a list too long to keep, a setup file made elsewhere. It
   looks for a fault, with the sanitizers on, and not for fidelity:
   what the implementation shows there is held against nothing. Much
   of what it finds will be the original's own, as `ClsLine` reading a
   colour in a panel's list of files is, and each such is a bug of the
   original that stays until its fix is decided: a list for the user,
   not a list of things to mend.
8. **The build of this computer held against the build for DOS on the
   same steps.** `harness/hostcheck.py` says at its top what it cannot
   see since it only reads, above all arithmetic that counts on an int
   of 16 bits, and "The portable C: what another compiler says of it"
   has it. With the answers recorded from a scenario, see below, the
   build of this computer has to show the screens that the build for
   DOS showed: where it does not, it has found such a place.
9. **What it would have caught, of the bugs that this project had.**
   By reading how each was found:

   | The bug | What would have caught it |
   |---|---|
   | BX and DX going round the main loop from one command to the next: Rename behind Down offered a name that the original does not, and no scenario had the two keys one behind the other | Nothing on this computer. Sequences of keys made at random, on the bench, against the original: the first of the two things that need no implementation for tests, below |
   | Texts read behind their end: `CrLf_S` with a zero byte too many, `User_S` copied as 80 bytes | The address sanitizer, in the first scenario that reads them |
   | The table of the blanker's stars, three bytes wrong where no screen shows | Not this: `c-data` holds it now. Mutation testing would have said that no screen watches the blanker |
   | A `name` that was never set, so DOS was handed what the stack held, in `SetStat` and at the entry | The memory sanitizer, in a build of 64 bits |
   | CX as `T75_00` leaves it: the editor's count of free bytes wrong, a figure that is not compared | A property test of the editor's buffer: the count falls by one for every character typed, which "Known and not dealt with" asks for |

### What it has to be

The rule of "Scenarios" in `CLAUDE.md` is that a test puts no stand-in
where something is that the project does not own, and that what a test
knows of DOS, the BIOS and the machine it gets from the real ones.
`sys.h` is the project's. What a function of it promises is a statement
about a system all the same: that `FileWrite` answers with fewer bytes
on a disk that is full is so of DOS, whoever wrote the header. So an
implementation for tests is within the rule only as this:

- It knows nothing of its own of what a system does.
- It answers what a test tells it to, the test's premise in so many
  words, "the third write answers 5 of 512": scripted. It has a queue
  of answers and no more.
- Or it answers what the real system was recorded answering on the
  bench, call for call: replay. Such a record is derived, as a golden
  is, and never edited, and is taken only from a run whose screens were
  the original's golden's.
- Where a premise of a scripted test can be made to happen on the
  bench, a small program asks the real DOS and holds the premise
  against it, as the toys of `harness/trial32/` asked DOS/32A.

An implementation that models DOS, a file system in the memory that
behaves "as FreeDOS does", is what the rule forbids, behind our own
interface as much as in front of it. It holds what we believe of DOS:
the order of a directory, which error, what is left in AX. The code is
then made to pass on it, and the bench disagrees, or is not asked. The
one use that such a thing may have is under a fuzzer, where it only has
to agree with itself and nothing that it shows is compared with
anything.

A wording for the rule was proposed. It is **not adopted**: the rule is
the user's, and this is here as what was proposed, no more:

> `sys.h` is the project's, and an implementation of it made for
> tests is a part of the program replaced. It answers what a test tells
> it, or what the real system was recorded answering on the bench, and
> knows nothing of its own of what a system does. What runs on it shows
> what the program does with those answers; that a system gives them,
> and that the program does as the original, only the bench shows.

### Its limits

- The goldens stay the only witness of what the original does. Nothing
  here records the original, and nothing here says what DOS does.
- The bench stays the gate: a step lands when it has passed its
  scenarios on DOS, as now. What runs on this computer is what is run
  while working, and where the bench cannot go. The implementation for
  DOS is tested on the bench alone, whatever else there is.
- What a unit test expects comes from reading the assembly. It holds a
  behaviour in place and says where a failure is; it certifies no
  fidelity. An expected screen written by hand would be a golden
  edited by hand under another name: a test of a routine holds cells
  and numbers that the routine is said to make, not screens.
- A scripted failure shows what the program does with an answer. That
  a system gives that answer, and that the original does the same with
  it, it does not show: for the 298 instructions of the table there is
  still no golden.
- A recorded trace is valid only while the program asks the same
  things in the same order. This is the session's reservation, and it
  is right; what there is to add makes it sharper, not weaker. Replay
  serves work that leaves the questions alone: pointers for segments,
  numbers widened, a routine written anew above `sys.h`, what is
  drawn into the program's own buffer. Any change to what the program
  asks of the system, one call more or two in another order, ends the
  trace of every scenario that passes there, and those are recorded
  again on the bench, from a run that has passed against the
  original's golden. The same holds, for all of them at once, of a
  change to `sys.h` itself. Two things keep this from being a
  trap. A replay that meets another question than was recorded fails,
  it does not pass on a wrong answer, so a stale trace is noticed and
  says that the bench is wanted. And a trace is what a golden of the
  tree's own was before a deviation was held by a patch: it shows that
  nothing has changed since it was recorded, and what made it right was
  the bench.
- The sanitizers and the coverage are of the build of this computer,
  another compiler's of the same source, in a memory laid out
  otherwise. A fault that only the layout of DOS makes they do not
  see, and one that they see may be the original's.

### What it needs of `sys.h`

This section asked a design of the layer, on 13 October 2026, to leave
room for it. Step 3 settled the design the next day as DOS's, `sys.h`
as it is, and the critical error has been a call back that the program
hands over since S2. What still stands in the way of an implementation
for tests, from what was read then:

- A decision of the program's under the seam: `AskEs` of the system's
  file makes the error 53h of a call that the user gave up, asks DOS for
  its extended error, and reads `md.ErrDat`. Every implementation has
  to do the same again.
- What DOS happens to do, in what the functions take and answer:
  `value` as DOS left AX where nothing failed, `flagged`, the functions
  with Bare in their names, the page and the colour of `ScreenType`,
  `FcbParse` and `FcbDelete`. Each is one thing more that every
  implementation and every trace has to do as DOS does.
- What the system keeps between two calls and no call names: the
  current drive and directory, the record of a search, how often the
  pointer was hidden.
- Nothing that a test can call to start the program, and `ProgramEnd`
  does not return.

### Open, for the user

- Whether a build of the program for DOS that writes out what it asks
  and what it is answered may exist, to derive traces from, where
  "Scenarios" has that no build is made for the tests. It would judge
  nothing: the build that is shipped is the one that is held against
  the goldens.
- Whether the rule gets the wording above, or another, before any of
  this is built.
- Two things need no implementation for tests and could be done now.
  Neither is started:
  - Sequences of keys made at random, on the bench, against the
    original and a tree, with one that differs kept as a scenario. It
    is the one thing here that looks for a mistake of the translation
    where nobody thought to write a scenario. One to two days, an
    estimate.
  - A first figure of the C's own code: which of its blocks never
    ran, from the log of QEMU that the scenarios' test reads already,
    where it looks today only at where each function begins. Half a
    day, an estimate. It says which blocks ran, not which way a branch
    went.

## The program saves its setup and its session by itself: decided, not started

On the user's word of 14 October 2026 (`CLAUDE.md`, step 3 of the
road), given while a layer was being designed and kept when that design
was set aside: the program saves its preferences and its session, the
panels' directories and cursors, the sort modes, the command line and
its history, by itself, when they change, after a short wait so that a
run of changes is one write, and when it is ended, on Linux by a signal
or by its terminal going; on every platform, DOS included. "Save setup"
and the setting "Auto save setup" go. On DOS that is a deviation, to be
declared when it is built. The rule of the harness for it: the
comparison of a scenario's disks leaves the setup file out, but in the
scenarios that are about the setup itself.

To settle when it is built: one file or two, and which field is in
which; where each lives, on DOS beside the program under names of 8.3;
how long a change waits; a write that an end in the middle of it leaves
whole; what two copies of the program running at once do with one file;
and what the scenarios about the setup hold, now that the file is
written when the setup changes and not by a command. The setup file
keeps the original's format, on the user's word of 14 October 2026
(`CLAUDE.md`, step 3), which works wherever DOS is emulated: the
design that was set aside had made it a format of the program's own
everywhere, and that is dropped.

## Step 6, the program for Linux: what is left

Step 6 is done (`CLAUDE.md`, "The program for Linux"); its pieces, L1 to
L7, are in `DIARY.md`. What they left:

- **For the user.** That the panels switched off otherwise than by
  Ctrl-O show a screen without what the commands wrote is how it
  stands (`src/portable_c/DEVIATIONS.md`, L9). The mouse's wheel does
  nothing, as the original's driver has none; a turn could be Up and
  Down, or scroll the list under the pointer (L11).
- **A renamed directory.** A directory renamed keeps its place in its
  directory on DOS and goes to the end in the emulation's table
  (`names_posix.c`, `Add`), and a directory renamed has its own entries
  listed anew by their names' bytes, the host's path being another.
  `copy-tree` is run whole on Linux since 15 October 2026, on the
  user's word: it renames only the last directory of a parent, D made
  X behind D2 and X2, and WORK before anything is made in it. Its new
  steps run every byte of the original that the old ones ran, and End
  in the tree besides; `F36_01`, the box for a path where a name is
  asked, and `F36_05`, DOS's current directory following a rename, are
  run by it alone. A directory renamed to the beginning of a sibling's
  name cannot be both the last on the disk and the last by name.
  DOS refuses to rename its current directory, and the original then
  moves it file by file, which fails; Linux renames it, declared on
  the user's word as L13 of `DEVIATIONS.md`, with no scenario.
  `stamps` is cut at 18, the file search, where the bench has SUB in
  front of STAMPS's files and Linux behind them.
- **Commands of DOS's.** `del`, `rd`, `cls`, `attrib` and `time` are
  none of the host's shell's, and the scenarios that use one stop there
  on Linux. A command of the host's in its place (`rm` for `del`), by
  `in_place_in`, would let them go on.
- **The BIOS's key words.** The table of them that the program for
  Linux makes of the terminal's keys is held by the scenarios and by
  `portable_c-terminal` alone; a toy on the DOS bench that holds the
  words that the BIOS gives was designed and not made.
- **A mount that waits.** A network or FUSE mount may wait without end
  on a call; nothing in the files of Linux deals with it, and there is
  no critical error on Linux to offer the user a way out.
- **The recording.** `mouse-history`'s `.ran` has a block of two or
  three bytes, 60C6 or 5E52 or none, that comes and goes with the load:
  a flaw of the recording, not looked into.

## Step 7, the web page

The plan, approved by the user on 15 October 2026.

CLAUDE.md, the road, step 7, has the user's decisions; this is the plan behind them. The piece W1 copies this file into `TODO.md` as the section "Step 7, the web page", and every piece keeps its own part current there.

### Architecture
- **Reuse of the Linux emulation.** Emscripten's POSIX layer covers what the emulation calls: opendir/readdir, lstat, statvfs, futimens, chmod, rename, unlink and realpath. So `files_posix.c`, `paths_posix.c`, `names_posix.c`, `codepage_posix.c` and `time_posix.c` build unchanged.
  - Under Emscripten, geteuid() is 0, and `DosAttributes` treats user 0 as having any w bit.
  - `term_posix.c` and the fork/termios part of `sys_posix.c` do not build for the web.
- **The split (W1).**
  - About 1,500 of the 2,212 lines of `sys_posix.c` are free of the platform: the BIOS screen grid, the console, the mouse driver, memory, time, the test channel, the environment, and the wrappers of DOS's files. They move into a shared `machine_posix.c`, declared in `emulate_posix.h`.
  - `sys_posix.c` keeps the terminal, the shell, xdg-open and main().
  - The key decoder of `term_posix.c` (Scan, Named, Hold, KittyKey, Pad…) moves into `keyboard_posix.c`.
  - `TermGlyph` moves into `codepage_posix.c` as `CodePageGlyph`.
  - The suffix filter of CMake becomes a list per implementation. For the web: the common code, plus `*_posix.c` but `sys_posix.c` and `term_posix.c`, plus `*_web.c`.
- **`sys_web.c` (W2)** has:
  - KeyRead, KeyWaiting, Idle, Ticks, KeysHeld and MouseWhere;
  - ProgramRun, ProgramEnd, ProgramOfFile and ProgramOpenFile;
  - main(), and the file system's mount.
- **Where the program yields (Asyncify).**
  - `KeyRead` waits by `emscripten_sleep`, and `Idle` yields.
  - `KeyWaiting` yields once 50 ms have passed since the last yield, so that TstBrk sees Esc in a copy or a search.
  - `MouseWhere` yields as `KeyWaiting` does and takes what the page has queued, since the program waits for a button to be let go by asking the mouse alone: without it a click on the key bar froze the page. A change of the buttons stays for 50 ms at the least, `BUTTON_HOLD` of `keyboard_posix.c`, so that a press and its release that were queued together are not seen as a release alone; the program for Linux has the same, and reads the terminal in `MouseWhere` for the same reason.
  - `Ticks` yields at its second read with no yield between, so that the zooming boxes and the blanker are drawn.
  - `ProgramEnd` awaits the sync of the store.
  - JS never calls into the wasm while it is suspended: keys, the mouse and uploads are queued in JS, and C drains the queues at each yield.
- **The cost of Asyncify.**
  - Most of the program is instrumented, since the tables call through function pointers. Expect the code to grow by 50–100% and to run 20–50% slower; loops that never yield stay fast.
  - `ASYNCIFY_STACK_SIZE` is 64 KB, for Input → Help → Input and Dialog → Input → Help.
  - Build at -O2 or -Os.
  - JSPI comes later, when every browser has it.
- **The JS (W3a and W3b).**
  - Pure modules `web/render.js`, `web/keys.js` and `web/mouse.js`, testable in Node.
  - A glue of `--js-library`/`--pre-js`, and a shell HTML.
- **Drawing the grid in the DOM.** A `<pre>` of 25 spans, one per row.
  - At each yield C calls `web_draw(cells, cursor, shape)`. JS holds each row's 80 u16 against a cached Uint16Array and rebuilds only the rows that changed, with one span per run of one attribute.
  - 256 generated CSS classes give the exact VGA colours (AA and 55, brown AA5500), and bit 7 blinks at the VGA's rate.
  - The cursor is an overlay, a block or a line by the BIOS's shape. The mouse's pointer is the emulation's cell with its colours exchanged.
- **The code page.** JS builds its table of 256 characters from the exported `CodePageGlyph`, so there is one source.
- **Keys.** keydown and keyup become tuples of kitty's shape for the shared C decoder:
  - the US key from `event.code`;
  - the text from `event.key`, with AltGraph taken as text;
  - what is held, from the modifiers and getModifierState;
  - press, repeat or release.
  - preventDefault is called on every key that is mapped.
  - Ctrl-W, Ctrl-T and Ctrl-N with their Shift forms, Ctrl-Tab, Ctrl-PgUp and PgDn, Ctrl-F4 and Alt-F4 cannot be taken in a normal tab, and F11 and F12 are unreliable. They are declared. A full-screen button calls `navigator.keyboard.lock()`, Chromium only, and holding Esc leaves the full screen. The list of the keys is checked by hand in a browser.
- **The mouse.** Pointer events on the `<pre>`.
  - The cell is worked out from the measured cell box and clamped to 80×25.
  - Buttons 1, 2 and 4. The context menu is suppressed, and the wheel does nothing (L11).
- **The clock.** Emscripten's clock_gettime uses the browser's Date and time zone. The test clock is shared from `machine_posix.c`.
- **The drive (the user's decision).** One, C: labelled VC, which holds C:\VC, the program's own files, and nothing else as it is shipped; the page starts there (CLAUDE.md, "The web page"; "One drive" below). Until the user's word of 15 October 2026 there were two, C: labelled VIBE and D: labelled DEMO, a small tree of sample files.
  - It is a directory of the page's server, a script for Node with Node's own modules: the page loads its tree from it, and every change, the setup file's too, is made there as it happens; what the server refuses or cannot do is DOS's error for that call. Nothing is kept in the browser then.
  - Served statically, from GitHub Pages with no server, the same page has its drive start from what is served with it, C: with `vc` alone, and keeps what the user changes in the browser (CLAUDE.md, "The web page").
  - The program takes the drives from the environment, `VC_DRIVE_C`, `VC_DRIVE_D` and the rest: W2's section below has them, and the page sets `VC_DRIVE_C` alone.
  - The tests give the drives as the bench has them, through the environment, C: the working disk labelled VIBE and D:\VC the program's files on D: labelled DEMO, so the masks of the drive's letter are not needed on the web, and no golden or mask moved with the one drive.
- **Files in and out.**
  - An upload button and drag-and-drop write into the host directory of DOS's current directory, which C publishes to JS at each yield. Overwriting asks confirm(). Then Ctrl-R (1312h) is queued, so that the panel reads the directory anew.
  - `ProgramOpenFile` opens the file in a NEW TAB, as a blob URL with a MIME type from the extension (the user's decision, not a download): the user views or saves it from there.
  - `ProgramOfFile` answers 0: nothing on the page is a program.
- **The setup file.** C:\VC\VC.INI, `vc/vc.ini` of C:'s directory, on the server like the rest of the drive.

### Deviations (W entries of DEVIATIONS.md, a part of their own, "The web page")
- **A command.** ProgramRun writes the prompt and the command, then "Commands cannot be run in a web page." on DOS's screen, and answers that it ran with code 1. This covers the user menu, VC.EXT, and the external viewer and editor.
- **Ctrl-O.** It is the program's own key, as on DOS: the panels go off and DOS's screen shows.
- **Enter on a file.** It opens a new tab.
- **The keys.** The browser's reserved keys, as above.
- **F10, leaving.** It shows the last screen and "Reload to start again", after the sync.
- **Memory.** Capped at 2 GB (`MAXIMUM_MEMORY`); since the cap ("The cap of the memory: done"), at 256 MB for the program's blocks and 320 MB for the page (W10).
- **The screen.** It is scaled by whole multiples of the font.
- **Help and menus.** The shareware's files are not shipped: F1 shows the original's message for help that is missing. A user may upload their own.

### The font
- VileR's Web437 IBM VGA 8x16, from the Ultimate Oldschool PC Font Pack (int10h.org), CC BY-SA 4.0, about 20 KB.
- Fetched pinned by SHA-256 and inlined as a base64 @font-face, with `font-display: block`, and credited on the page.
- 16px × n with line-height 1 gives a cell of exactly 8×16·n.

### Testing
- **The program under Node.** A second link, `web-node`: `-sNODERAWFS -sENVIRONMENT=node -sEXIT_RUNTIME`, with a pre-js that copies VC_TEST, VC_TEST_CLOCK, HOME, XDG_CONFIG_HOME, TZ and VC_TRACE into ENV.
  - The program writes its screens through VC_TEST as on Linux.
  - Reused: posixrun's bench, keys, `_screens`, read_tree, compare.py, goldens.py and the masks.
  - `setups.Program.load` takes `*.js` as the tree "web", and posixrun runs `[node, vc.js]`.
  - `TREES["web"] = _on_top(TREES["posix"], ON_WEB)`, and the patches are in `harness/goldens/web/`.
  - The test is `portable_c-web-scenarios`. Try ASan and UBSan with Asyncify; UBSan alone if they do not go together.
- **The limits of that.** NODERAWFS is not MEMFS/IDBFS: the sparse files of 4 GB in STAMPS cannot be held in MEMFS. So the page's file system is not what the scenarios exercise. That is declared.
- **The render test,** `portable_c-web-render`, needs no download. render.js is fed screens recorded by the channel, the HTML is decoded back and held to the cells. keys.js is held the same way, with synthetic KeyboardEvent-shaped objects against the BIOS words of terminal.py.
- **No new download for tests** (no jsdom, no Playwright). The browser is a manual pass by the user in Chrome and Firefox.

### Downloads and the build
- **Emscripten.** The release archive that emsdk fetches, `storage.googleapis.com/webassembly/emscripten-releases-builds/linux/<commit>/wasm-binaries.tar.xz`, the commit taken from emsdk's `emscripten-releases-tags.json`. It is pinned by CMake FetchContent with URL_HASH SHA256, without running emsdk's scripts.
  - A generated `.emscripten` points at its LLVM and Binaryen and at the system's Node. EM_CACHE is in build/.
  - The version is the newest release on the day W0 runs.
  - Licences: MIT/NCSA, Apache-2.0 with the LLVM exception, Apache-2.0, and MIT for musl.
- **When it is fetched (the user's decision).** Only with `-DVC_WEB=ON`; the web's tests exist only there.
- **Open Watcom** likewise behind an option, `VC_DOS`, default ON, since every test of DOS needs it. A checkout or a CI job for the web alone can switch it off. Make that so where it makes sense, and say what does not.

### Deployment
- `SINGLE_FILE` with a shell HTML gives one `build/portable_c/web/vc.html`, with the wasm, the JS, the CSS and the font inline: an estimated 1–3 MB, with no CDN.
- The demo tree is preloaded into it (`--preload-file` bundled) or embedded. (There is no demo since the one drive.)
- The targets are `portable_c-web` and `portable_c-web-node`.
- Publishing, by GitHub Pages or otherwise, was asked of the user in W4. The user had the static page published on GitHub Pages on 16 October 2026 (below, "Open, for the user").

### Pieces
- **W0, the toolchain.**
  - The `VC_WEB` option and the pinned Emscripten download, the `VC_DOS` option, the config, and the test `portable_c-web-probe`.
  - The probe compiles every common file with emcc, with the project's warnings and -Werror: new warnings of clang and wasm32 (ILP32) issues are found early. Mend what it finds in the common code (DOS and Linux scenarios still pass).
  - Heavy (the download). It runs beside W1 and W3a.
- **W1, the split: done.** `machine_posix.c`, `keyboard_posix.c`, `CodePageGlyph`, and the lists of CMake.
  - Linux behaves as it did, and VC.EXE is the same bytes.
  - The tests: posix-scenarios, terminal, posix-unit and tidy.
  - Heavy. It also copies this plan into `TODO.md`.
  - What it made, for W2:
    - `machine_posix.c` is all of what `sys_posix.c` was but KeyRead, KeyWaiting, KeysHeld, Idle, Ticks, MouseWhere, ProgramRun, ProgramOfFile, ProgramOpenFile, ProgramEnd and main(), which an implementation has of its own: `sys_posix.c` has them for Linux, some 470 lines, with the terminal, the shell and xdg-open. Their parts that are no platform's are functions of the machine, declared in `emulate_posix.h`, "The machine": `TestBegin`, `MachineBegin`, `MachineEnvironment(shell, count, arguments)` (COMSPEC is the implementation's to name, `ShellDos` on Linux), `MachineEnd`, `MachineTicks`, `ScreenShown`, `TypedForget`/`TypedText`, `MouseMoved`/`MouseAnswer`, `TestIdle`, `TestKeyWaiting`, `TestKeyRead`, `TestKeysHeld`, `TestMouseAsked`, `TestProgramRun`, `TraceKey`, `TraceHeld`, `TraceFlush`, and `HostFileOf`. `sys_posix.c`'s functions are a few lines of those each: `sys_web.c` is written the same way, with the yields of the plan around them.
    - The one DOS is `MachineDos`, and the host file of a name of DOS's `HostFileOf`: `harness/posixcheck.c` has a static `Emulated` and `HostFile` of its own, and the test of the emulation links `term_posix.c` and `keyboard_posix.c` with it, so a name of the machine must not be one of that file's.
    - `keyboard_posix.c` is the decoder of `term_posix.c`, from "The keys, and the mouse" on, with the key buffer, what is held and the mouse's reports: `KeyboardWaiting`, `KeyboardTake`, `KeyboardHeld` and `KeyboardMouse` (`struct ReportedMouse`) where those were `Term...`. Bytes are handed in by `KeyboardBefore(&wait)`, `KeyboardRoom(&to)` just before they are read, and `KeyboardAfter(count)`: a page can hand it kitty's `CSI ... u` as bytes and have the terminal's words. What it had the terminal do, push the kitty protocol's flags at the answer to the question, is `KeyboardKittyAnswered()`, which `term_posix.c` asks behind each decoding (`Answered`); a terminal too small to show the program is `KeyboardRefuse(true)`.
    - `CodePageGlyph` is in `codepage_posix.c`, declared beside `CodePageToUnicode`.
    - `CMakeLists.txt` has `portable_c_sources(<variable> <implementation>)` with the lists `portable_c_implementations` (dos posix web), `portable_c_takes_<it>` and `portable_c_leaves_<it>`: `portable_c_sources(files web)` is the common code, `*_posix.c` but `sys_posix.c` and `term_posix.c`, and `*_web.c`. `add_portable_c` and `add_portable_c_host` take their files by it. The test of the emulation leaves out `machine_posix.c` as it does `sys_posix.c` (it reads the main data), and `harness/tidycheck.py` reads it with the program's command (`PROGRAM_POSIX`).
    - `portable_c-web-probe` of W0 compiles both new files for the web with the rest, 68 files, and says nothing. It had its own list of what the web shares, `NOT_SHARED` of `harness/webprobe.py`, beside `portable_c_leaves_web` of `CMakeLists.txt`; since W2 it reads that one, which the configuration writes for it.
    - VC.EXE and debug/VC.EXE are the same bytes before and after (SHA-256 f6538da4... and cf1abd33...). The program for Linux is the same code moved: posix-scenarios, terminal, posix-unit and tidy pass. The records of `portable_c-host` and `portable_c-words` have the two new files, clean and with no word.
- **W3a, the pure JS.** render.js, keys.js, mouse.js and the test `portable_c-web-render`. Medium.
- **W2, the program under Node: done**, below. `sys_web.c`, the web-node link, the harness's tree "web", ON_WEB, and `portable_c-web-scenarios`.
- **W3b, the page and its server: done**, below. The server, the page's file system against it, the glue, the shell HTML, upload and new-tab open, the demo tree, the font pinned, the cursor, the full screen, the phone, and the targets `portable_c-web` and `portable_c-web-serve`. The user tries it in a browser.
- **W3c, the static page: done**, below. The same page with no server, its drives starting from what is served with it and the user's changes kept in the browser (CLAUDE.md, "The web page"), the static site, and the workflow of GitHub Pages, which runs only by hand and with a variable set.
- **W4, the documents: done**, below. DEVIATIONS "The web page" (W1 to W9, written by the pieces before it), the README's section "The web page", the licences of Emscripten, musl and the font, and the question of publishing, which is the user's, under "Open, for the user" below.
- **W5, the licences on the page: done**, below. The bar's credit of the font gone, and one "Licences" that shows the texts of every work that the page carries, put in from the repository's files as the page is made (the user's decision of 15 October 2026, CLAUDE.md, "The web page").
- **W6, the page in a real browser: done**, below. The machine's own Chromium, headless, driven over CDP from Node; both ways of serving, on a desktop and as an iPhone held both ways; the screenshots looked at; two faults of the drawing on a phone mended; the test `portable_c-web-browser`.
- **W7, the scenarios through the page: done**, below. Every scenario of the web played through `vc.html` in Chromium, its keys and mouse the browser's events, its screens read from the `<pre>` and from the pixels; `harness/webpagerun.py`.

### W0, the toolchain: done, and what a later piece should know

- **Emscripten 6.0.12**, the newest release on 15 October 2026 by
  emsdk's `emscripten-releases-tags.json` (commit
  `6e9f6885cb17a3d6c8fd488a5cff730f50aba61d`), the archive
  `wasm-binaries.tar.xz` for Linux on x86-64 that emsdk would fetch,
  pinned in `CMakeLists.txt` ("Emscripten") by FetchContent with its
  SHA-256, `7b073a10...a5e8fa7`. The archive's top directory, `install`,
  is taken off: `build/_deps/emscripten-src` has `bin` (LLVM),
  `emscripten` (emcc, with its `node_modules`) and `lib`. Its clang is
  `24.0.0git`, where the path's clang is 23.
- **The download:** 304 MB, 1.5 GB unpacked. On the bench's link the
  download took about a second and the whole configure with the
  unpacking 15 seconds. A second build directory can take the same
  download by `-DFETCHCONTENT_SOURCE_DIR_EMSCRIPTEN=<build>/_deps/
  emscripten-src`.
- **The configuration** that emsdk would write is written by CMake,
  `build/emscripten/config`: `LLVM_ROOT`, `BINARYEN_ROOT`, `NODE_JS` (the
  `node` that the path has, found by `find_program`; the configure
  stops without one) and `CACHE`, `build/emscripten/cache`. The command
  is `${emcc}` of CMake: `emscripten/emcc --em-config build/emscripten/
  config`. At its first use emcc fills the cache with the system's
  headers and prints `shared:INFO:` and `cache:INFO:` lines; a link
  will build musl and the rest there the first time, which takes
  longer. Emscripten runs on the system's `python3`.
- **`VC_DOS`**, on by default, guards Open Watcom: its download, the C
  translation, the program for DOS of the portable C and its debug
  build, and the tests that need any of them, `c-data`, `c-arith`,
  `c-scenarios`, `portable_c-data`, `portable_c-scenarios`,
  `portable_c-cut`, `harness-le` and the two of the layout. JWasm is
  not guarded: it is small, and the driver of the tests that it
  assembles is what `portable_c-posix-scenarios` runs once on a machine
  of DOS for the bench's setup file, with the original; `asm-bytes`
  stays with it. A configure with `-DVC_DOS=OFF -DVC_WEB=ON` was built,
  and `asm-bytes`, `portable_c-posix-unit` and `portable_c-web-probe`
  passed in it.
- **`VC_WEB`**, off by default, guards the download and the web's
  tests. Every target and test of the web goes inside `if(VC_WEB)`.
- **The probe**, `portable_c-web-probe`, `harness/webprobe.py`:
  compiles with `-c` (so wasm32's code is made, not only parsed), at
  `-O2`, with `clang_warnings`, `-fno-strict-aliasing` and
  `-D_XOPEN_SOURCE=700`, every file that the web is to have by the
  plan's list: the common code, every `*_posix.c` but `sys_posix.c` and
  `term_posix.c`, and `*_web.c` (`NOT_SHARED`, `leaves.txt` since W2, and `IMPLEMENTATIONS`
  there; W1's `machine_posix.c` and `keyboard_posix.c` come in by
  themselves). 66 files, four seconds. What emcc says of itself at its
  first use is not counted.
- **What it found: nothing.** All 66 files compiled with nothing said
  at the first try, so nothing in the common code was mended and
  `build/portable_c/VC.EXE` is the same bytes (SHA-256 `f6538da4...
  dc715abe` before and after; `debug/VC.EXE` the same too). A toy with
  a `long` put into a `u16` and a `size_t` into an `int` was refused,
  so the warnings are on. Of wasm32: `size_t` is `unsigned long` there,
  32 bits.

### W2, the program under Node: done, and what W3b should know

- **`sys_web.c`** is written as `sys_posix.c` is, over the machine of
  `machine_posix.c`: KeyRead, KeyWaiting, KeysHeld, Idle, Ticks,
  MouseWhere, ProgramRun, ProgramOfFile, ProgramOpenFile, ProgramEnd and
  main(). The yields are the plan's, `Yield` there: KeyRead waits by
  `emscripten_sleep` (50 ms at a time while nothing is queued), Idle
  yields, KeyWaiting and MouseWhere yield once 50 ms of the host's
  clock have passed since the last yield, and MouseWhere takes what the
  page has queued, Ticks at its second read with no yield between,
  and ProgramEnd draws and waits for `web_sync` before the machine's end.
  Under the test channel it yields at the same places, with 0 ms, so
  that the scenarios go through Asyncify's unwinding as the page does.
- **The page's side** is `web/library.js`, an Emscripten library
  (`--js-library`) of four functions that go by what the page puts on
  `Module`, and do nothing where it has put nothing, as under Node:
  `web_input(to, room)` takes from `Module.vcInput`, an array of strings
  of bytes, which W3b fills with what `keys.js` and `mouse.js` answer;
  `web_draw` hands `Module.vcDraw(cells, place, shape)` the 2,000 cells
  as a Uint16Array over the memory, good for the call alone, which is
  what `render.js`'s `draw` takes; `web_open_file` calls
  `Module.vcOpen(host, name)`, the file's path in the file system and
  DOS's name, for the new tab; `web_sync` awaits `Module.vcSync()`,
  by `Asyncify.handleAsync`. The bytes go to the decoder of
  `keyboard_posix.c` as a terminal's do (`PageInput`, TermInput's way:
  `KeyboardBefore`, `KeyboardRoom`, `KeyboardAfter`), mouse reports
  included; Ctrl-O is the program's own key.
- **The drives.** Two, C: and D:, each the directory of the host that
  a variable of the environment names, `VC_DRIVE_C` and `VC_DRIVE_D`,
  labelled as `VC_LABEL_C` and `VC_LABEL_D` say (11 characters at the
  most, DOS's form), VC and DEMO where they say nothing (VIBE and DEMO
  until the one drive); a drive whose
  variable is not set is not there, and main() ends with a message
  where neither is (`main` of `sys_web.c`, `DosBeginGiven` of
  `files_posix.c`, `DrivesBeginGiven` of `paths_posix.c`, which take a
  table of `struct DriveGiven`). The program's own files are in the
  directory `vc` of the drive that `VC_PROGRAM_DRIVE` names, C: where it
  names none, made where it is not there (`OwnDirectory` of
  `machine_posix.c`): C:\VC on the page, as the user decided on 15
  October 2026, and D:\VC in the tests, as on the bench. Both are
  ordinary drives, which the drive box and the Info panel see. The
  program starts in the host's current directory where a drive has it,
  else at the root of the first. `DosPathOfHost` takes the first drive
  in the table that a path is under, which is H: before R: on Linux as
  it was. W1 of DEVIATIONS.md. HOME and XDG_CONFIG_HOME mean nothing on
  the web. A first try had one drive C: and the program's files on a D:
  that only a path led to; the user decided the two drives instead.
- **What W3b must set** on the page: `VC_DRIVE_C` and `VC_DRIVE_D` to
  the two directories of the file system that mirror the server's
  drives, and nothing else, the labels and the program's drive having
  the page's values by default (`ENV` in a `preRun`, as `web/node.js`
  does). The drives are the server's (CLAUDE.md, "The web page"), a
  script for Node with Node's own modules: every file operation has to
  reach it as it happens, and what it refuses or cannot do is DOS's
  error for that call. One way: a MEMFS mirror loaded from the server's
  tree at the start, written through by fetches that Asyncify awaits,
  in the calls of `files_posix.c` that change something (create, write,
  close, delete, rename, make and remove a directory, set a time or the
  attributes), the error of a failed fetch made the call's.
  Served statically, with no server, the page keeps the changes in the
  browser instead, and the program is the same: what it takes is the two
  directories.
- **A command** writes "Commands cannot be run in a web page." through
  DOS's console behind the prompt and the command that the program has
  typed, and answers that it ran with the code 1; `TestProgramRun` says
  the step on standard error as on Linux, so what DOS shows behind the
  panels is masked from there on. **ProgramOfFile** answers 0.
  **ProgramOpenFile** writes "open NAME" into the trace where VC_TRACE
  asks, and calls `web_open_file`.
- **The link `web-node`**, target `portable_c-web-node`, is
  `build/portable_c/web-node/vc.js` with its `vc.wasm`, by
  `add_portable_c_host` with `COMPILER ${emcc}` and `OUTPUT vc.js`:
  `-O2`, the sanitizers of addresses and of undefined behaviour, fatal,
  `--profiling-funcs` for the names in a report (`-g` makes emcc warn
  that it optimizes less, which `-Werror` stops), `-sASYNCIFY
  -sASYNCIFY_STACK_SIZE=65536 -sNODERAWFS -sENVIRONMENT=node
  -sEXIT_RUNTIME -sALLOW_MEMORY_GROWTH -sMAXIMUM_MEMORY=512MB`, and
  `web/node.js` as the pre-js that copies VC_DRIVE_C, VC_DRIVE_D,
  VC_LABEL_C, VC_LABEL_D, VC_PROGRAM_DRIVE, VC_TEST, VC_TEST_CLOCK,
  VC_TRACE and TZ into `ENV`.
  **ASan and UBSan go together with Asyncify**, so both are on.
- **The memory.** The program asks for all that there is, and the
  machine said the host's memory less what is held, which Linux gives;
  a heap of wasm32 gives less, and the program's second ask, for what
  it was told, failed. So the machine finds how long a block the C
  library gives by taking it and giving it back, halving to a page,
  where it does not give the whole (`Obtainable` of `machine_posix.c`;
  Linux gives the whole at the first try). Two things met there: clang
  drops a `malloc` whose block is only given back, and takes it for
  given, so the block goes through a volatile pointer (`Tried`); and
  the sanitizer of addresses keeps a block that is given back from use
  for a while, so the next ask for the same length failed: under it the
  web turns that off, `quarantine_size_mb=0`, with
  `allocator_may_return_null=1` so that a refusal is NULL
  (`__asan_default_options` of `sys_web.c`). With 2 GB, under the
  sanitizer, the halving poisons and unpoisons a gigabyte's shadow at
  every ask, and `start` took 24 seconds; the Node build has 512 MB, and
  W3b's page, which has no sanitizer, the plan's 2 GB.
- **The sanitizer found one fault** of a `long` of 32 bits: the test's
  clock worked out the ticks of VC_TEST_CLOCK as `long`, which overflows
  on wasm32 (`ClockBegin`, made `int64_t`).
- **The harness.** `setups.Program.load` takes a `.js` for the tree
  "web"; `posixrun.HOSTED` is the trees that run on the host, and
  `posixrun.run` runs `[node, vc.js]` with `VC_DRIVE_C` the bench's
  home, `VC_DRIVE_D` the run's `d` and `VC_PROGRAM_DRIVE=D`, the
  program's files in `d/vc`, from the bench's setup file as it is
  (C:\WORK and C:\, so no setup file of its own as Linux has) and with
  the steps as the scenario has them, D:\VC being D:\VC. `TREES["web"]` is the portable C's with
  `ON_WEB`: Linux's scenarios not run, cut and left out, and Linux's
  masks but those of the drive's letter, `LINUX_LETTER_MASKS`, which the
  web does not need; `WEB_NOT_RUN` and `WEB_CUT` are empty. `BASE`
  goes down a chain, web to posix to portable_c, for a patch, and
  `harness/goldens/web/` has none. `ported.check_web_declared` holds a
  scenario of `WEB_NOT_RUN` or `WEB_CUT` to a W entry, as
  `check_linux_declared` does for Linux, and both are asked for the
  web. The label of the web had first been WEB: the box of Ctrl-F4 is as
  wide as the label, and three letters made it narrower than the
  golden's, which no mask can do; with four it is masked as on Linux.
- **The result.** The 119 scenarios that the program for Linux passes
  pass on the web, with nothing declared of the web's own; `find`,
  `find-drives`, `label` and `enter`, tried on the web, differ as they
  differ on Linux. Some 4 to 13 seconds a scenario, three side by side.
- **The test** `portable_c-web-scenarios`, with VC_WEB alone, labels
  portable_c and machine.
- **One list of what the web leaves.** The configuration writes
  `portable_c_leaves_web` into `build/portable_c/web-probe/leaves.txt`,
  which `harness/webprobe.py` reads: `NOT_SHARED` is gone.
- **The checks that read the tree** with the host's compilers leave
  `*_web.c` to the probe, since they have not Emscripten's headers:
  `OTHER_SYSTEMS` of `harness/int16.py` (and so `width.py` and `cut.py`)
  and `NOT_READ` of `harness/hostcheck.py`. `harness/tidycheck.py` reads
  no `*_web.c` either: clang-tidy does not read `sys_web.c`.
- **The size**, measured on 15 October 2026 for the plan's question of
  what Asyncify costs: the Node build as the test has it, sanitized,
  `vc.wasm` 1.81 MB and `vc.js` 98 KB. The same objects without the
  sanitizers, linked as the Node build is: at `-O2` 394 KB of wasm with
  Asyncify and 298 KB without it (+32%), at `-Os` 313 KB and 221 KB
  (+42%), the JS some 85 KB each. The plan's guess was +50 to 100%.
- **The Asyncify stack used**, measured by a pre-js of a trial, not
  kept, that wraps `_asyncify_stop_unwind` and reads how far the data
  of the unwinding has gone at each: at the most 1,668 bytes, in all
  119 scenarios that the web is taken through, of 26,166 unwindings,
  against the 64 KB that the link gives it (the help over a dialog over
  the panels among them). The sanitized build at `-O2`; another build
  keeps other locals, so W3b may measure its page's the same way.

### W3a, the pure JS: done

The piece W3a of the plan above: done.

- Where: `src/portable_c/web/`, `render.js`, `keys.js` and `mouse.js`.
  They are the program's platform for the web as `term_posix.c` is for a
  terminal, so they are of the portable C and nowhere else (CLAUDE.md,
  "The portable C"); a directory of their own, since the build takes
  every C file of `src/portable_c/` by its suffix and these are no C,
  and since the page's other files (W3b, the shell HTML and the glue)
  go beside them. Each is a plain ES module, with nothing of the
  browser's in it but `apply` of `render.js`, which writes into elements
  that it is handed.
- `render.js`: `new Screen(codePoints)` takes the table of the 256
  glyphs as code points, which W3b makes of the program's
  `CodePageGlyph`; `draw(cells, place, shape)` takes the 2000 words of
  the grid (character low, attribute high), the cursor's place (row
  above column) and the BIOS's shape (first line above last), and
  answers `{ rows: [[row, html]...], cursor }`, the rows that changed
  since the last draw (all at the first, and after `forget()`), each a
  span per run of one attribute of the class `vc-aXX`, with `vc-b` for
  bit 7; `cursor` is null or `{ row, column, block }` by the rule of
  `ShapeOf` of the terminal's code. `css()` is the style sheet of the 256
  classes, the VGA's colours exactly. The blink is not an animation per
  span, which would go out of step as rows are made anew: the page turns
  `BLINK_CLASS`, `vc-off`, on its `<pre>` every `BLINK_HALF` ms (16 of
  the VGA's 70.086 frames), and `.vc-off .vc-b` makes the character
  transparent. `CURSOR_HALF` is the cursor's, twice as fast; W3b blinks
  its overlay by it. `apply(drawn, rowElements, cursorElement, cell)`
  writes a draw into the 25 row spans and places the overlay, a line
  being the lowest 2 of the cell's 16 lines.
- `keys.js`: `keyEvent(event)` takes a KeyboardEvent, keydown or keyup,
  and answers `{ bytes, prevent }`: the escape code that kitty sends
  with the flags that the program pushes, as a string of bytes, or null
  for a key that the program has nothing to do with (then the browser
  keeps it). `kittyOf` gives the tuple, `{ code, shifted, base, mods,
  event, text, final }`, and `sequence` writes it. So the shared decoder
  is fed bytes, through its own reader of CSI, exactly as a terminal
  feeds it: W2 hands the bytes to the decoder (TermInput's path, which
  W1 moves into `keyboard_posix.c`) and needs nothing of kitty's tuples
  in C. `FOCUS_OUT` is what to feed on blur, which lets go what is
  held. AltGraph is a shift of the layout, not Ctrl and Alt: its text
  is typed. A dead key or an input method's character comes by no
  keydown and is dropped (W3b may take `compositionend`; it was not
  asked).
- `mouse.js`: `new Mouse()`, and `event(pointerEvent, rect)` answers
  the SGR reports, one per button that changed, from
  `PointerEvent.buttons`, which has the driver's bits; a motion where
  nothing changed. `cellOf` keeps the cell to 80 by 25. The wheel has
  no report, as in the terminal.
- The test `portable_c-web-render`, `harness/webrender.py` with
  `harness/webrender.mjs`: Node runs the modules (no download, no
  jsdom), skipped with 77 where there is no Node or no build for Linux.
  It draws the 43 grids of the test channel for terminal.py's KEYS and
  KITTY and eight of its own, reads the HTML back and holds it to the
  cells and to the rows that changed; it types the keys and the
  mouse's reports of keys.js and mouse.js into the program for Linux in
  a pseudo-terminal with the kitty protocol (terminal.Terminal) and
  holds what the program reads to the bench's BIOS's words, so the
  decoder that judges keys.js is the program's own and not a model of
  it. Each mutation tried of the three modules (escaping, the rows
  made, the base key, the cell, the cursor's shape, its hiding, the
  background's bits) failed it. Some 30 seconds.
- For W3b: the reserved keys of the browser (Ctrl-W and the others of
  the plan) never reach `keyEvent`; `prevent` is true for every key
  that the program takes, Alt alone included, which keeps Firefox's
  menu bar away.

### W3b, the page and its server: done, and what W3c should know

**To try it in a browser** (the user, by hand): in a checkout configured
with `-DVC_WEB=ON`,

    cmake --build build --target portable_c-web-serve

builds the page and runs its server in the foreground, on
`http://127.0.0.1:8437/`, with C: in `build/portable_c/web/drives/c`,
made with its `vc` the first time. Ctrl-C stops it. By hand, with
another directory or another port:

    node src/portable_c/web/server.mjs --port 8437 \
        --page build/portable_c/web/vc.html \
        --drive-c DIR

For a phone, an HTTPS reverse proxy in front of it, for example
`tailscale serve --bg 8437`. What to look at by hand: the panels drawn in the VGA font and
colours, the blink, the cursor in the editor and the command line; the
keys of W4 of DEVIATIONS.md, in Chrome and Firefox, and the full screen
with its keyboard lock in Chrome; the mouse, clicks and the right button;
Enter on a file, a new tab; Upload and dragging a file, with the
question before one is written over; on the phone, the fit when it
turns, pinch-zoom, a tap as a click, the row of keys with Ctrl and Alt
held for the next key, and the phone's keyboard by its button.

- **The server**, `src/portable_c/web/server.mjs`, Node's own modules: it
  serves the page and the drives' trees, `GET /api/tree?drive=c|d` (every
  file with its contents in base64, parents first), and makes the
  changes, `PUT /api/file` (a file written whole, with its time and
  read-only bit) and `POST /api/change` (`delete`, `rename`, `mkdir`,
  `rmdir`, `mode`); its top has the API. It refuses, with EACCES, any
  path with `.`, `..`, an empty name, a backslash or a NUL, an absolute
  path, a drive that there is not, and any name on the way that is a
  link of the host's, the last included; a tree lists no link. It
  listens on 127.0.0.1 alone. It makes C:'s `vc` where it is not there,
  since the program's `OwnDirectory` makes it in the page's copy alone.
  Its answers are no-store: nothing is cached.
- **The hooks.** `files_posix.c` tells `dos->change` of `struct Dos`
  (`emulate_posix.h`, `enum HostChange`) of every change before it makes
  it: delete (and the FCB's delete), rename, make and remove a
  directory, and the read-only bit, before the host is asked; a file's
  contents, time and mode at its close, of the handle that is still
  open, where it was made (`made`, new in `struct DosHandle`), written or
  stamped. An error answered is the call's, by `DosErrorOf`; Linux has
  no hook, `DosBegin` making it none, and does what it did. `sys_web.c`
  sets it to `Change`, which calls `web_change` of `library.js`, awaited
  by `Asyncify.handleAsync` where the page has put `Module.vcChange` and
  answered at once where not (Node): its numbers are `Errors` there,
  `CHANGE_ERRORS` of `drives.js`. `Draw` also tells the page DOS's
  current directory on the host at each yield, `web_where`, which the
  uploads go into. The program's mkdir of its own directory at the start
  (`OwnDirectory`) and the test channel's files are not told: the first
  is the server's to make, the second the tests'.
- **The page's file system**, `web/drives.js`, a plain module handed the
  FS and `fetch`: `load` (each drive's tree into MEMFS at `/drive/c` and
  `/drive/d`, `ROOTS`), `change` (the server first; a path in neither
  drive is no concern of the server's), `upload`. Whole-file writes at
  close, as the brief allowed. `VC_DRIVE_C` and `VC_DRIVE_D` are set in
  the pre-js's `preRun` (`web/pre.js` hands the page FS and ENV), and the
  program waits for the load by a run dependency.
- **The mode is chosen in one place**, `MODE` and `MODES` at the top of
  `web/page.js`: "server" makes the `Drives` of the server. W3c adds a
  mode with no server, whose object has the same three methods, `load`,
  `change` (answering 0 and keeping the copy, in IndexedDB or as it
  likes) and `upload`, and chooses it where the page has no server (a
  failed `/api/tree`, or the page's URL).
- **The page**, `web/page.js` with `web/shell.html`, made one file by
  `web/bundle.mjs` (each module a function of its own answering its
  exports, its one-line imports taken from those before it) and emcc's
  `SINGLE_FILE`, `build/portable_c/web/vc.html`, about 570 KB, no
  sanitizers, `-O2`, 2 GB at most. emcc minifies the shell's script.
  `ProgramOpenFile` opens a blob URL with a type by the extension in a new
  tab; a browser that keeps a tab from opening, as one may since the
  open does not come from the click itself, gets a notice with a link to
  click. Ctrl-R is typed after an upload. The cursor blinks at
  `CURSOR_HALF`, the characters at `BLINK_HALF`, as W3a's notes say. The
  phone: the grid fitted by `visualViewport` where the page is not zoomed
  (a zoom of the fingers is left alone), by whole multiples of the font
  where it has room for one, smaller where not; a tap is pointerdown and
  pointerup, `touch-action: manipulation`; the row of keys is shown on a
  coarse pointer and by its button; the phone's keyboard comes up for a
  hidden input (`font-size: 16px`, so that Safari does not zoom in on its
  focus) focused in the tap, its `input` events made keys, Backspace and
  Enter by their input types; `100dvh` with `100vh` before it; nothing
  rests on hover. None of the browser's side has been seen in a
  browser: it is the user's pass by hand.
- **The font**: the Ultimate Oldschool PC Font Pack 2.2's archive for the
  web, `oldschool_pc_font_pack_v2.2_web.zip` from int10h.org, 5 MB,
  pinned in `CMakeLists.txt` by its SHA-256 `f0e0a15e...a8d74855`, with
  VC_WEB alone; the page takes `Web437_IBM_VGA_8x16.woff`, 9.8 KB, as
  base64. It was credited in the page's bar until W5, which has its
  licence shown with the others' by the bar's "Licences".
- **The demo**, `src/portable_c/web/demo`: a readme, notes, three small
  programs' sources that cannot be run, made-up data, and a text with
  DOS's line ends, written for the project. Gone from the page with the
  one drive; the files are the README's animation's since,
  `harness/session_files`.
- **The tests.** `portable_c-web-server` (`harness/webserver.py` and
  `.mjs`, Node alone, no VC_WEB): the server's API and its refusals,
  links of the host's among them, the page's file system against it with
  a file system of Emscripten's shape, and the page's script bundled,
  compiled and run up to `Module`. `portable_c-web-page`
  (`harness/webpage.py` and `.mjs`, VC_WEB): `vc.html` itself, its
  scripts run in Node with a document of what the page asks of one,
  against the server: the drives loaded, a directory made, a file opened
  in a tab, a file written in the editor reaching the server at its
  close, a directory removed, a refusal of the server (the drive's
  directory made read-only on the host) shown as DOS's error, the setup
  saved on C:, and the page's end. Some five seconds. The editor clears
  what is typed ahead of it, so the test types one key at a time, and
  waits for each box before its keys.
- **A start on C:.** The page starts at the root of C:, as W1 of
  DEVIATIONS.md says (the root of the first drive): the demo is a drive
  away, by Alt-F1 or Alt-F2. Starting on D: is one line of `vcBegin`
  (`FS.chdir(ROOTS.d)` behind the load); it was tried and taken out, as
  a choice that is the user's, and the user decided on 15 October 2026
  that the page starts on C:, as it does. With the one drive it starts
  in C:\VC.
- **Left for W4**: the README, the licences, the list of keys checked in
  the browsers, and publishing.

### W3c, the static page: done, and what W4 should know

- **One page, two ways.** `vc.html` is the same file served both ways,
  and knows which by asking where it is served, `askMode` of
  `web/page.js` beside `MODES`, the one place: `GET api/server`, a new
  answer of `server.mjs`, `{ server: "vibe-commander", drives }`, is the
  server ("server", every change made on it, as in W3b); an answer of
  4xx, a static host's 404, is no server ("browser"); an answer of 5xx
  or none is a server that is away, and the page says so and does not
  start, so that a page of the server never keeps its changes in the
  browser because the server was down for a moment (a proxy in front of
  it answers 502). A build of its own for the static site was set aside:
  a second page to build and test, for a question that the host answers.
  Since a page was published on a host of pages in a sandbox, whose
  answer to a name that it has not is not known: an answer of 5xx or
  none is a static site all the same where `drives/c.json` is served
  beside the page as `static.mjs` writes it, `staticSite` of
  `web/page.js`, since `server.mjs` never serves that name and a proxy
  in front of a stopped server does not either; `portable_c-web-page`
  holds hosts that answer 200 with a page and 500, and one that is
  away, 502.
- **The static site**, `web/static.mjs`, the target
  `portable_c-web-static` (in ALL with VC_WEB):
  `build/portable_c/web/static/`, `index.html` (the page), `drives/c.json`
  (C:, its `vc` and nothing in it), `drives/d.json` (the demo, by
  `treeOf` of `server.mjs`, lifted out of `makeServer` for it, the
  server's tree as it was; gone with the one drive) and `.nojekyll`. The trees are of the shape of
  `/api/tree`. To try it by hand: `python3 -m http.server -d
  build/portable_c/web/static 8438` and `http://127.0.0.1:8438/`, which
  answers 404 for `api/server` as GitHub Pages does (a browser that
  opens it as `file://` cannot fetch the trees).
- **The store**, `web/kept.js`: `KeptDrives` is `Drives` with its `put`
  and `order` made in a store of records by key (`d:notes/todo.txt`, one
  per file or directory, the bytes with the time and the read-only bit)
  and a record `begun` that says the drives are the store's; `load` takes
  the store where `begun` is there, and else what is served, which it
  then keeps. A change that the copy is to refuse (there already, not
  empty, not there, into itself) is refused first, so that the store
  never has what the copy has not. `IdbStore` is IndexedDB, the database
  `vibe-commander` with its one object store `records`, a transaction a
  call; `MemoryStore` where `openStore` finds no IndexedDB, or it does
  not open in 5 seconds (blocked, refused). The bar says which (`#kept`),
  and in IndexedDB has the button Reset drives (`#reset`), which empties
  the store after a `confirm` and reloads the page. Whole files at close,
  as with the server.
- **The tests**, nothing downloaded: `portable_c-web-server` holds
  `kept.js` with `MemoryStore` and with `IdbStore` on
  `harness/fakeidb.mjs`, a stand-in of IndexedDB of what the store uses,
  in Node: every change and refusal, the next visit, the reset, a full
  store (ENOSPC), no IndexedDB and one that refuses; and that the server
  answers `api/server`. `portable_c-web-page` runs the server's part as
  before, and holds that it opens no IndexedDB; then makes the static
  site by `static.mjs`, serves it by a host of a few lines that answers
  404 for what it has not, and runs the page four times on one stand-in
  of IndexedDB: changes made and kept, the page loaded again with them,
  the reset, and a browser with no IndexedDB. Some ten seconds.
- **The workflow**, `.github/workflows/pages.yml`: `workflow_dispatch`
  alone, and both jobs `if: vars.VC_PAGES == 'publish'`, so it cannot run
  by itself and does nothing until the user sets the variable; Pages has
  to be switched on as well, with GitHub Actions as its source. It
  configures with `-DVC_DOS=OFF -DVC_WEB=ON`, builds the target
  `portable_c-web-static` and deploys its directory. Emscripten is kept
  between runs by actions/cache, keyed by the two lines of
  `CMakeLists.txt` that pin it, unpacked in `~/emscripten-src` and handed
  to the configure by `FETCHCONTENT_SOURCE_DIR_EMSCRIPTEN`, with emcc's
  cache of libraries beside it. Its actions are GitHub's own, checkout,
  cache, upload-pages-artifact and deploy-pages, each pinned by the
  commit of its tag (looked up by `git ls-remote`). The commands of its
  steps were run here in a build directory of their own, with the
  Emscripten of another checkout, and made the same `drives/d.json`;
  the workflow itself had not run then. It has since, on 16 October
  2026, when the user had the page published: see "Open, for the
  user".
- **What W4 should know.** The browser has not seen the static page:
  the user's pass by hand, in a browser, of the page from a static host,
  a reload keeping the changes, Reset drives, and a private window
  (Firefox's IndexedDB works in a private window and is cleared at its
  end; Safari's may refuse it, and then the page says it keeps the
  changes in memory). `navigator.storage.persist()`, which asks the
  browser not to clear the site's data when room is short, is not asked
  for: some browsers ask the visitor, and the user decided on 15 October
  2026 that the static page does not ask it (CLAUDE.md, "The web
  page").
  The drives that a visitor has kept are not changed by a new deploy
  until Reset drives. What a visitor kept of D: before the one drive is
  forgotten at the next visit (`load` of `kept.js`).

### W4, the documents: done, and the user's pass by hand

- **The README** has a section of its own, "The web page", beside "The
  program for Linux": the two drives, the two ways of serving, building
  it, the server and a reverse proxy in front of it, the phone, the keys that a browser
  keeps, what cannot work, the tests of the web and the downloads with
  their licences. The rows of the web's five tests are in its table of
  tests, and the count of the tests in "Testing" has them.
- **The licences.** `LICENSE.EMSCRIPTEN` is the `LICENSE` of Emscripten
  6.0.12, MIT and NCSA, and `LICENSE.MUSL` the `COPYRIGHT` of the musl
  that Emscripten builds its C library from, MIT, both as Emscripten's
  repository has them at the tag `6.0.12`: the page has Emscripten's
  runtime in its script and musl's code in its wasm. `LICENSE.FONT` is
  the `LICENSE.TXT` of the font pack's archive that the build fetches,
  CC BY-SA 4.0, copied as it is; the pack's `README.TXT` says "(c)
  2016-2020 VileR". Each is kept as `LICENSE.DOS32A` is, beside
  `LICENSE`, and the README's "Licence" names them. Clang's runtime
  that may be in the wasm is Apache-2.0 with the LLVM exception, which
  asks nothing of a program compiled with it.
- **The page's credit of the font**, the bar's: the font's name, linked
  to the pack's home page, "by VileR", and "CC BY-SA 4.0" linked to the
  licence. The pack asks for credit in that form, the handle VileR and a
  link to the site ("Legal Stuff" of its documentation); the font is put
  into the page as it comes, the WOFF unchanged, so the page is no
  adaptation of it and ShareAlike asks nothing of the page. The
  licence's other items, the copyright notice and the notice of no
  warranty, are on the pack's page that the credit links to, which the
  licence allows (3(a)(2)). A line with "(c) 2016-2020 VileR" in the
  bar would say it on the page itself: not made, since W4 changed no
  code. The user decided on 15 October 2026 that no work is credited
  apart from the others: W5 took the credit out of the bar, and the
  font's copyright line is in its licence's heading on the page, below.
- **The pass by hand**, the user's, which nothing else can make, in
  Chrome, in Firefox and in Safari on an iPhone. In a checkout
  configured with `cmake -B build -DVC_WEB=ON` (and
  `-DFETCHCONTENT_SOURCE_DIR_EMSCRIPTEN=<build>/_deps/emscripten-src`
  to take the download of another build):

      cmake --build build --target portable_c-web-serve

  serves the page with its server on `http://127.0.0.1:8437/`, and for
  the phone, beside it, an HTTPS reverse proxy in front of it, for
  example

      tailscale serve --bg 8437

  The static page:

      cmake --build build --target portable_c-web-static
      python3 -m http.server -d build/portable_c/web/static 8438

  and `http://127.0.0.1:8438/`; for the phone, the proxy on 8438 in
  place of the other. What to look at:
  - [ ] The panels in the VGA's font and colours, the blink, the cursor
    in the editor and on the command line.
  - [ ] The keys: F1 to F10 with Shift, Ctrl and Alt and the key bar
    that follows them, Alt alone for the menu, Ctrl-O, the arrows,
    Ins and Del, typing with Shift and AltGr; and that the keys of W4
    of `DEVIATIONS.md` are the browser's and every other key the
    program's. The full screen with its keyboard lock in Chrome, and
    holding Esc to leave it.
  - [ ] The mouse: a click, a double click, the right button, with no
    menu of the browser's.
  - [ ] Enter on a file: it opens in a new tab, or the notice with its
    link where the browser held the tab back.
  - [ ] Upload and a file dragged onto the page: written into the
    current directory, the question before one is written over, the
    panel read anew.
  - [ ] On an iPhone: the fit, again when it turns; pinch-zoom; a tap
    as a click; the row of keys, with Ctrl and Alt held for the next
    key; the phone's keyboard by its button, typing and Backspace and
    Enter.
  - [ ] F10: the last screen stays, and the page says to reload.
  - [ ] The page of the server: a change made on the page is in
    `build/portable_c/web/drives/` at once; a refusal is DOS's error
    (a directory of the drive made read-only on the host).
  - [ ] The static page: the bar says that changes are kept in this
    browser; a change, then a reload, and the change is there; Reset
    drives asks, forgets and loads the served drives again; a private
    window, where Firefox keeps the changes until the window closes and
    Safari may refuse IndexedDB, the bar then saying that they are kept
    in the page's memory.

### W5, the licences on the page: done

- **What the page shows.** The bar has "Vibe Commander 0.1, based on
  Volkov Commander 4.05." and one small "Licences", a button that looks
  like a link, beside it in the bar that wraps, so the phone's layout
  has it too. It opens a modal `<dialog>` of the page's with a heading
  and a text for each work that the page carries, all in one form: the
  work and what of it the page has, its copyright, its home where it
  has one, and the licence's text as the repository has it. Five:
  Vibe Commander's own, `LICENSE`; Volkov Commander's, whose code the
  program is, `src/asm/LICENSE.TXT` (its second condition asks that a
  binary carry the notice); Emscripten's, `LICENSE.EMSCRIPTEN`; musl's,
  `LICENSE.MUSL`; the font's, `LICENSE.FONT`, with "(c) 2016-2020
  VileR" in its heading, since the licence's text has no copyright of
  the font in it. DOS/32A's is in no page. A browser that has no
  `showModal` gets the five as text in a new tab, from a blob.
- **Closing it.** Esc, which the browser takes for the dialog, its
  Close button, and a click on the backdrop. No key reaches the program
  while it is up, nor the letting go of a key that went down then, Esc
  above all (`unseen` of `page.js`); the focus is taken off "Licences"
  when it closes, so that the next Enter is the program's. The modal
  dialog keeps the mouse from the screen.
- **Where the texts come from.** `LICENCES` of `web/bundle.mjs` names
  the files and the headings; the bundle reads the files as it makes
  the shell, line ends made LF, and puts them in as JSON in a `<script
  type="application/json">` with every `<` escaped, so no text can end
  the element. `CMakeLists.txt` has the five files among what the shell
  depends on, so the page is made again when one changes. Each was held
  against its source before: the font's against the `LICENSE.TXT` of
  the pinned archive, Emscripten's and musl's against the `LICENSE` and
  the `COPYRIGHT` of the pinned Emscripten 6.0.12, the same bytes.
- **Emscripten minifies the shell**: attributes lose their quotes,
  `id=licences-data`. A test that reads `vc.html` looks for a name with
  or without them.
- **The test**, `portable_c-web-page`: one button "Licences" in the
  page and no credit of the font apart; a click shows the five texts,
  each the file's, in order, and not DOS/32A's; keys while it is up, Esc
  with its letting go, reach nothing of the program; the button closes
  it, and keys are the program's again. It fails, tried, where the
  dialog does not hold the keys back. `VC.EXE` and `debug/VC.EXE` are
  the same bytes as before.
- **What is the user's to see by hand**: the dialog in Chrome, Firefox
  and Safari on an iPhone, scrolled, and closed by Esc and by its
  button.

### W6, the page in a real browser: done

- **The browser** is the machine's own Chromium, found on the path,
  `chromium` or `chromium-browser`, not fetched or pinned (CLAUDE.md,
  "The web page"); where it was made, that was the snap of Chromium 155.
  A snap reads no hidden directory and not the real /tmp: the profile
  and everything that it opens are in `build/`. The snapshots of
  Chromium's own storage were tried first: they need NSS and NSPR
  (`libnss3`, `libnssutil3`, `libsmime3`, `libnspr4`), which the
  machine lacked.
- **The driver**, `harness/browser.mjs`: Chromium started headless with
  `--remote-debugging-port=0` and its profile in a directory of the
  build, the port read from the profile's `DevToolsActivePort`, CDP over
  Node's own `WebSocket`, a tab a session of one connection (`flatten`).
  Keys by `Input.dispatchKeyEvent` with `code`, `key` and the virtual key
  of Windows, the modifiers going down before and up after as a keyboard
  has them; taps by `Input.dispatchTouchEvent`, clicks by
  `dispatchMouseEvent`; the phone by `Emulation.setDeviceMetricsOverride`
  with `mobile`, `setTouchEmulationEnabled` and Safari's agent, which
  gives `(pointer: coarse)` and so the row of keys; the console by
  `Runtime` and `Log`; a tab that the page opens by
  `Target.targetCreated` with an opener, its URL asked after, since it
  is empty when the tab is made.
- **The checks**, `harness/webbrowser.mjs`, and the test
  `portable_c-web-browser` (`harness/webbrowser.py`), which runs them
  without screenshots in some forty seconds: the table of tests in
  `docs/web.md` has them. With `--shots DIR` the same run saves its screenshots. The
  server is `server.mjs` as a process of its own on a free port; the
  static site is behind a host of Node's that answers 404, whose
  sockets are closed with it, or Chromium's kept-alive ones hold its
  `close` for ever.
- **What looking at it found, and what was mended:**
  - On the phone the grid had a black seam between every two rows,
    across every frame of a panel. An inline run of text has its
    background only as tall as the font's own ascent and descent, and
    at a scale that is no whole multiple of 8 by 16 that falls short of
    the row. Each run is an `inline-block` as tall as the cell now
    (`--cell`, `shell.html`).
  - Then the upright lines of the frames had a notch at every third
    row: Chromium puts a line of text on a whole pixel of the page's,
    not of the device's, so rows of 9.33 pixels each shifted their
    glyphs. The cell is a whole number of the page's pixels now,
    `Math.floor(16 * scale)`, which makes the grid on an iPhone held
    upright 360 pixels wide where it was 382. The desktop, at whole
    multiples, is as it was. Two other ways were tried and dropped: a
    cell of a whole number of the device's pixels left the notches
    sharper at every third row; and a `translate` that put the grid's
    corner on a pixel of the device's drew a faint line between every
    two rows, the compositor resampling the moved layer. In the
    device's own pixels a faint notch may still show in an upright line
    on the phone held upright, where a glyph of 16 lines is drawn in 27;
    at the size of the screen it is not seen, and turned, in 33, there
    is none.
  - The page asked for `/favicon.ico`, which neither the server nor a
    static host has: an error in the console at every load. It has an
    empty icon of its own now, and the `mobile-web-app-capable` that
    Chromium asks for beside Apple's.
  - The README said that F1 says there is no help. It shows nothing, as
    the original does without its help file.
- **What a test has to know of the program**, which it met as faults of
  its own: the editor clears what is typed ahead of it (Clear keyboard
  buffer, on by default), so a test types there a key at a time and
  waits for the screen; Shift-F4 on a name that is not there asks
  New-file first; the key bar of Shift stays up while a dialog is, as
  the original's does; without a setup file the program shows one
  panel, the right one, as the original does; and it saves its setup by
  itself, so a second tab of the same drives starts as the first left
  them.
- **What is left, for whoever goes on:**
  - The static page's question, `api/server`, is a 404 that Chromium
    writes into the console as an error. It is how the page knows that
    it has no server, so the test leaves it out. A file `api/server` in
    the static site, saying that it is no server, would end it, at the
    cost of `portable_c-web-page` no longer meeting the hosts that
    answer 200 or 5xx for what they have not.
  - A button of the row of keys keeps the focus after a tap, and
    Chromium draws its ring of focus around it; Safari may not.
  - On the phone held upright the grid is less than half the screen's
    height, centred, with the row of keys at the bottom: as fitting by
    the width makes it.

### One drive, C:\VC alone: done, and what is left of it

On the user's word of 15 October 2026 the page has one drive, C:,
labelled VC, which holds C:\VC, the program's own files, and nothing
else as it is shipped; there is no D: of a demo and no sample files, and
the page starts in C:\VC (CLAUDE.md, "The web page"; W1 of
DEVIATIONS.md). What it changed and met:

- **The program** keeps its two drives by the environment (`main` of
  `sys_web.c`): the page names `VC_DRIVE_C` alone, and C:'s label where
  none is given is VC, a label of two letters, which makes the box of
  Ctrl-F4 narrower than four do. The page starts the program in C:\VC
  by the current directory of Emscripten's FS, `FS.chdir` behind the
  load in `vcBegin` of `page.js`, which the program starts in where a
  drive has it.
- **The tests of the scenarios** give the bench's layout through the
  environment as before, and its labels besides, `VC_LABEL_C=VIBE` and
  `VC_LABEL_D=DEMO` (`harness/posixrun.py`), which were the program's
  defaults: so no golden, patch or mask moved.
- **The server** serves one directory, `--drive-c`, made with its `vc`;
  `--drive-d` and `--demo` are gone, and it refuses drive `d` as any
  drive that it has not. **The static site** ships `drives/c.json`
  alone, C: with `vc`. **The browser's store** keeps the one drive, and
  forgets at the next visit what it kept of D: before (`load` of
  `kept.js`). Reset drives is as it was.
- **The page's tests** make the files that they work on in their drive,
  by `harness/webfiles.mjs`, nothing of them shipped: the server's test
  lays them on C: beside `vc`, the page's test on the server's C: and in
  a tree of C: served in place of the static site's, and the browser's
  test both ways, its static site a copy in its work directory. The
  page's test holds that the drive box has C: alone and that the static
  site ships C: with `vc` alone; the server's that D: is refused and
  that a store forgets D:.
- **The demo's files** are the README's animation's now,
  `harness/session_files`, byte for byte, readme.txt's words on D:
  among them, so that `python3 harness/session.py` makes the same
  bytes of `docs/vibe-commander-session.svg`, which it does.
- **The README's animation, from the web page**, on the user's word of
  16 October 2026: `python3 harness/session.py` plays its session on
  the web page as its server serves it, C: labelled VC with C:\VC alone,
  in the machine's headless Chromium (`harness/websession.mjs`, by
  `harness/browser.mjs`), the keys as a keyboard sends them: two panels,
  a directory made with F7, hello.txt written in the editor and viewed
  with F3, the menu bar walked, the Configuration dialog, F10's question.
  A frame is the page's cells, read from the classes of its runs, and
  the cursor's element; the mouse's pointer is moved to the last cell of
  the key bar first, where the program had it in the middle of the
  screen. The page's clock is set to 16 June 2000, 12:00 UTC, so two
  runs give the same bytes, which they did. The SVG had a button below
  the screen, "Try it live in your browser", and the README links the
  image and a caption to the published page; on 16 October 2026 the user
  had the button and its band taken out, the caption being the one
  place for those words, and the SVG is the 80 by 25 screen alone. `--linux` makes it of the
  program for Linux and `harness/session_files`, as before.

### W7, the scenarios through the page

The user asked on 15 October 2026 that the whole suite of scenarios be
run at least once through the page itself, where `portable_c-web-scenarios`
runs the program under Node and passes the page by.

- **How.** `harness/webpagerun.py` with `harness/webpagerun.mjs`: the
  page, `vc.html`, served by its own server (`makeServer` of
  `server.mjs`, in the runner's process) with the bench's C: as its one
  drive, as posixrun.py lays it out for the web, in the machine's Chromium,
  headless, a Chromium and a profile in `build/webpagerun/` for each
  scenario, three side by side. `setups.Program.load` takes `vc.html`
  for the tree "web", and `posixrun.run` hands such a program to
  `webpagerun.drive`, so that the run's screens, disks and declarations
  are the web's, as under Node.
- **The test channel through the page.** The program's channel keeps
  the bench's clock and says when an event is due and when a step has
  ended, as under Node; with `VC_TEST_PAGE` besides (`sys_web.c`), it
  asks the page for each key, change of what is held and move of the
  mouse by its number among the events of `keys` (`TestByPage` of
  `machine_posix.c`, `web_test_event` of `library.js`), and waits; the
  runner gives it as the browser's key events and pointer events over
  CDP, which `keys.js` and `mouse.js` make the program's input as they
  make a user's, and the decoder of `keyboard_posix.c` the BIOS's words;
  the channel then holds the first key to the word that `keys` has, and
  says on standard error where it differs. Each screen that the channel
  records is told to the page, `web_test_screen`, and waited on while
  the runner reads it. The page's one addition is its test hook,
  `window.vcTestHook` (`page.js`), which a user's page does not have:
  it puts the channel's variables into the environment and its keys
  into the file system, and is the page's side of those two calls.
- **What is read.** Each screen twice: the `<pre>`, its characters by
  the page's own table of glyphs (`CodePageGlyph`) and its spans'
  classes, a glyph that several bytes share, a blank among them, read
  as the byte that the channel recorded; and its pixels, a screenshot
  of the grid at one device pixel to the CSS pixel, every 8 by 16 cell
  held to the glyph of the page's font, drawn by the page itself in its
  `<pre>` white on black at the first screen, in the VGA's colours of
  the cell (a blinking cell its background too, the cursor's cell with
  its overlay or without); a cell that is not so is decoded into what
  its pixels show. Both are held to the goldens with the web's
  declarations and masks; the cursor that the page draws is held to the
  one that the channel recorded. Chromium runs with `--disable-lcd-text`:
  it draws text in the colours of the subpixels or in grey by what it
  composites where, and the same glyph came out otherwise in the atlas
  and on the screen. On a phone's viewport (`--viewport portrait` or
  `landscape`), where a cell is no whole number of device pixels, the
  pixels well inside each cell are held to the cell's two colours and
  their blends, on the lattice of 80 by 25 cells: an exact atlas is not
  workable at a scale that is no whole multiple of the font.
- **D:.** The page has one drive, C:, since the user's word of 15
  October 2026, and makes C:\VC; the bench has the program's own files
  in D:\VC and no C:\VC. The hook takes the C:\VC that the page made
  away again, with the root's time, gives the page's file system the
  bench's D: from the job, and hands it back at the end, where the
  runner writes it into the bench's `d` before the disks are read
  (`tree_of` and `put_tree` of `webpagerun.py`). Its changes are no
  server's: the page asks its server only of what is on C:.
- **What the page's file system cannot hold**: the four files of
  C:\STAMPS of a megabyte or more are laid out empty on the server's
  drive and given their sizes in the page's file system once it has
  loaded, read as zeros; the disks after a run have them as the bench
  does where nothing changed them. And the program starts in C:\WORK
  by the file system's current directory, which the hook sets before
  it starts and puts back to the root at the first event: Emscripten's
  file system refuses to rename or remove its current directory
  (EBUSY), which a user's page, whose current directory is the root,
  never meets.
- **A scenario that presses a key that a browser keeps** (W4 of
  DEVIATIONS.md: Ctrl-PgUp, Ctrl-PgDn, Ctrl-T, Ctrl-W, Ctrl-F4, Alt-F4)
  is not taken through the page, `KEPT` of `webpagerun.py`: through
  the page those keys never reach the program, or close the tab. 19
  scenarios.
- **Found and mended in the page**: a directory's time, and the root's.
  `drives.js` (and `kept.js`) set each directory's time before what is
  in it was made, which moved it, so that every directory showed the
  time of the page's start; and the root's was never set, which `..`
  shows. Each directory's time is set again once the tree is made, and
  the server's tree has the root's time, `mtime` beside `entries`.
- **The F2 of the user's screenshot** (the box "Could not find the menu
  file" with Ok twice, a shadow to its right and below, and the key bar
  with Help and Quit alone) is the original's, cell for cell: the
  golden of `user-menu` has it, and the page passes that scenario both
  ways.
- **The mouse through the page.** The decoder of `keyboard_posix.c`
  keeps a change of what is held or of the buttons from what comes
  behind it until the program has looked, or for a while of the host's
  clock (CHANGE_WAIT, BUTTON_HOLD), as a hand's pace has it. Through the
  page the channel gives one event at a time when the program is to
  have it, as the bench's driver does, and so takes the place of the
  hand: `KeyboardUnpaced` has the decoder make whatever comes at once,
  where the host's clock would make a run of the page another in every
  run. And what the decoder holds back is taken where the program asks
  for keys, what is held or the mouse (`PageTake` of `sys_web.c`). The
  freeze of a click that the user met on the page (MouseWhere, which
  neither yielded nor took input, ef09ff6) was met here too, by the
  first run, as every scenario of the mouse stopping with the page
  given no answer.
- **The result** on 15 October 2026, at the desktop's viewport: of the
  119 scenarios that the tree "web" is taken through, 100 are played
  through the page and pass, by the `<pre>` and by the pixels alike, no
  cell's pixels other than its glyph's, no cursor other than the
  channel's; 19 press a key that the browser keeps and are not played.
  Some five to nine minutes, three at a time. As an iPhone held upright
  and turned, every scenario without the mouse passed by the lattice of
  cells, and the 31 with the mouse once it was mended. The whole run was
  before the page came to one drive; after it, eight scenarios, of the
  mouse, of the setup file on D:, of the tree's copy and the start, were
  run again and pass, and the machine was too loaded for another whole
  run.

### Open, for the user

- Publishing to GitHub Pages: done, on the user's word of 16 October
  2026, the repository being public. `VC_PAGES` is `publish`, Pages has
  GitHub Actions as its source, and `pages.yml`, run by hand, has
  published the static page at https://mmotorny.github.io/vibe_commander/,
  which the README's animation links to. What its first runs met: CMake's
  download of the font pack came back with another SHA-256 at each try on
  GitHub's runners, where curl gets the pinned bytes, so the workflow
  fetches the archive by curl, holds it to the pin of `CMakeLists.txt` and
  hands it to the build; and its actions warned of Node 20, which GitHub
  has deprecated, so each is at its release on Node 24 (checkout v7.0.1,
  cache v6.1.0, upload-pages-artifact v5.0.0 with upload-artifact v7.0.0,
  deploy-pages v5.0.1), still pinned by commit. A workflow runs by hand
  only (CLAUDE.md, "Third-party software").

Decided on 15 October 2026, by the user: the font's copyright line,
which is in its heading among the licences that the page shows, no work
being credited apart (W5); that the page starts on C:, as it does (W3b,
"A start on C:"); and that the static page does not ask the browser to
keep its data, `navigator.storage.persist()` (W3c).

### Left open when the session of 15 October 2026 ended

- **The page in a real browser**: done as W6, above, in Chromium. Firefox and Safari on an iPhone are still the pass by hand of W4.
- **A small fault in `web/drives.js`:** `upload` refuses names that match `/[/\\\0]/`. The minifier writes the `\0` as a raw NUL, which a browser reads as U+FFFD, so the check refuses U+FFFD instead of NUL. It is harmless, since no name has a NUL, but it should say what it means, for example `\u0000` written so that it survives the minifier.
- **`editor-big`** is run by no tree of the portable C. Running it on the web with a small `VC_MEMORY`, where the cap makes the original's refusal reachable, is a deviation to ask the user ("The cap of the memory: done").

## The cap of the memory: done

The web page's memory has a cap, as the user decided on 15 October 2026
(`CLAUDE.md`, "The web page"): the memory of WebAssembly, once grown, is
never given back while the page is open. The piece was called I1.

- `MemoryAll = min(physical, cap)`, the cap set by each implementation,
  and the variable `VC_MEMORY` for tests. The web page: 256 MB. Linux:
  none.
- `MachineBegin` of `machine_posix.c` takes the cap as its last
  argument, `int64_t cap`, 0 for none: `sys_posix.c` passes 0 and
  `sys_web.c` its `MEMORY_CAP`, 256 MB. The memory is then the
  machine's (`_SC_PHYS_PAGES`) or the cap, whichever is less, and the
  blocks that the program holds, the panels and the room, hold that
  much at most together (`Largest`).
- `VC_MEMORY` takes the place of the cap on every implementation that
  has the machine: a number of bytes, with K, M or G behind it for
  1,024, 1,024 squared or cubed, from 1 to 7FFFFFF0h; anything else ends
  the program before it starts, with a line on standard error that names
  the variable. Under Node it is copied into the program's environment
  by `web/node.js`; the page has no environment, and so no `VC_MEMORY`.
- The page's ceiling, `MAXIMUM_MEMORY`, is 320 MB, the cap and 64 MB
  for the runtime, where it was 2 GB; the program under Node keeps its
  512 MB, which the sanitizer wants room in. `Obtainable` stays: with
  the cap the heap gives the block at the first try, and the halving is
  there for a heap that gives less, which none of the builds now is.
- What the original does when its room is short is what the program
  does under a cap: the editor's box, "The file ... is too large for
  Edit.", with View and Cancel. No code of the program changed for it;
  the room is the largest block that there is (`start.c`).
- The test `portable_c-cap`, `harness/capcheck.py`, holds it on Linux and
  under Node: with `VC_MEMORY` at 512K the editor's box for
  `H:\STAMPS\noon.dat`, 1 MB, cell for cell against the original's in
  the golden of `editor-big`, step 39; without it, that the editor opens
  the file; and that a `VC_MEMORY` of `512Q` does not start. It runs in
  a second or two. `posixrun.run` takes `environment` for it.
- Declared as W10 of `DEVIATIONS.md`, with L7 saying that Linux has no
  cap. `editor-big` stays not run in every tree of the portable C;
  bringing it back on the web with a small `VC_MEMORY` is a deviation to
  ask.

## A patch with a line for a cell that blinks: untried

What no scenario has yet, and was tried by hand only: a patch with a
line for a cell that blinks. It was made to a golden and held against a
run in a trial; no committed patch has one, since no deviation so far
changes what blinks.

## The bench with a list of files on the right: what was put off

The bench makes the panel on the right one of files (`CLAUDE.md`,
"Scenarios"), and the scenarios that showed the Info were mended the
short way, with `Ctrl+l` in front.

### Put off: eight scenarios that could leave the Info off

Put off on purpose, on 13 October 2026. Eleven scenarios switch the Info
on as their first step and so have it on every screen, as before. For
three that is what they are about, `memory`, `dirinfo` and
`view-window`. The other eight could be written so that the Info is on
only where a step wants it, or not at all, which is what the change of
the bench was for. They were mended the short way all the same: with
`Ctrl+l` in front, every screen behind it is the old golden's cell for
cell, and that was the proof that the change of the bench lost nothing.
A scenario written anew has a golden to be read, and proves nothing of
the kind; it is worth doing once the proof is no longer needed, and not
before. What each would need, none of it tried but where it says so:

- `copy-big`: "F5", "Enter", "p", five times, repeats a copy by the
  last target, which F5 offers only next to the Info. With the name
  typed each time, `"huge" Enter`, it needs no Info. Tried: it leaves
  the same files.
- `move`: steps 4 and 5, F6 and Enter "onto itself", lean on F6
  offering the name of the file; next to files it offers their
  directory and the file is moved there. The name typed, or the Info on
  for those two steps.
- `rename-keys`: only its last three steps want the Info, as the
  current panel. `Ctrl+l` in front of them; the dialogs of the steps
  before then have `C:\` for a target, which is not what they are
  about.
- `copy-ask`: "offers its own name" and "offers the last target" are of
  a panel next to the Info, and three steps want the Info as the active
  panel. Next to files the same keys offer `C:\`, which is another
  thing to show and wants its own steps.
- `compare`: its first step, `Ctrl+c` that does nothing next to the
  Info. `Ctrl+l`, `Ctrl+c`, `Ctrl+l` in a row would have the Info on
  two screens where it is on nine.
- `reread`: its second step looks at what the Info says of a directory
  named dirinfo; the Info could go behind that step, where it goes at
  the twelfth.
- `menu`: one look, the Right menu of the Info, with the bar open from
  the second step to the twentieth. The look would move to a place
  where the Info can be switched on in front of it and off behind.
- `mouse-menu`: one click on an item that is dark, Compare directories
  next to the Info; the Commands menu has none dark next to files. The
  Info on around that click, which is at the fourth step with the menu
  open from the second.

## Leaving a part of the screen out of the comparison: what is left

The idea, recorded on the user's word of 13 October 2026, is built for
the program for Linux: its masks, `LINUX_MASKS` and `masked` of
`harness/deviations.py` (`CLAUDE.md`, "The scenarios of the program for
Linux"). One thing that it asked for is not built:

- Every part that is left out is still compared whole in at least one
  scenario that is about it, and the test says so if none is.

## Known and not dealt with

- Saving in the editor to a disk that cannot be written, the floppy A:
  of the bench, with the box of DOS's critical error, is in no scenario:
  it went beyond the stack of 256 bytes that the translation had when the
  editor's saving was translated. With the stack of 4,096 bytes it can go
  back into one.
- `T80_00` takes what the program knows of the main extension file from
  the editor's text after saving it, and does so whether the file was
  written or not: a save of VC.EXT that fails changes what the program
  goes by until it is started again. Ported like that. Saving to the
  console writes nothing and is no error; and after a save under another
  name that fails, the new name is in the status line only once that is
  drawn again for another reason. Ported like that too.
- A disk that is full cannot be made on the bench, so what `T80_00` does
  then, deleting what it has written, rests on reading; and so does
  what it does when DOS refuses the attributes of a file just written.
- The harness can type Russian on the Russian bench, and nothing beyond
  ASCII on the other: Alt with the numeric keypad types nothing there.

- The screen blanker is C, `Blank` of `src/c/blank.c`, and what it draws
  is held against the original on no screen. Its stars come out where
  the program's random numbers say, and nothing sets the two words that
  those are made of, `Rand1` and `Rand2`, when the program starts: the
  sky follows from what the memory held where the main data is. On the
  bench that is 0 for the original, and something else for the translation,
  whose main data is elsewhere, another thing with every build. So a
  step may not end in the blanker. The scenario `blank` brings it up,
  with the mouse in the top right corner, and ends it in the same step,
  by a key, by the mouse moving away, by Shift and by a button: what the
  goldens hold is that the screen and the cursor come back and that the
  key is lost. If the program set the two words when it starts, a step
  could end in the sky and a golden could hold it; that would be other
  behaviour than the original's, and is not decided.
  The C was held against the original by hand instead, once, outside
  the tests: with the translation's two words set, in a build made for it,
  to what the original's are when the blanker comes up first, and with a
  step that lets the program go round without ending, both were taken
  through a sky of some hundreds of turns, and every screen of either
  was one that a model of the routine, written from the assembly, has
  after some turn. See what the batch lost time to, below.
- `ClsLine` fills a row with the colour that DOS writes in, and reads
  that colour through DS, where every other routine of its kind goes by
  ES. One caller comes with DS on a panel: a command of a menu or of an
  extension file that begins with `@` and gets longer than the 998
  characters it may have while the name of the file under the cursor is
  put into it (`Run_22` to `Run_31`). The row is then filled in whatever colour
  the byte at that offset of the panel's segment makes, which is in the
  panel's list of files. Ported like that, by reading: `ClsLine` takes
  the segment as an argument, and `Run_00`, which is C too, passes the
  panel's there. No scenario has it.
- Backspace in a line that is typed into, with the cursor behind the end
  of the text, moves the cursor back and says that nothing has changed,
  so the caller does not draw it. Whether a caller can leave the cursor
  there has not been looked into. Ported like that.

- `ReDir` reads byte 23 of the main data, a colour of the first palette,
  where it means the fourth character of a panel's path, to decide
  whether ".." gets the time and the attributes that DOS has for it. It
  is ported like that. The colour is never 0, so it always does.
- What the commands that view and edit do in the original and the
  translation does after them; whether to mend any of it is not decided:
  - An extension file of the viewer, `VCVIEW.EXT` of the current
    directory, that DOS will not open for another reason than that it
    is not there, has F3 do nothing, without a word: a directory of
    that name, in `view-commands`.
  - A program that such a file names as the viewer and that is not
    there leaves the program's own viewer, without a word. One that DOS
    finds and will not run is "Not enough memory to load" for the
    errors 7 and 8 and "Can't run commands for some reason. Try
    rebooting." for every other: a name with a star in it, for which
    DOS finds a file and can run none, in `view-commands`. For that
    box 80 bytes are copied from the text "Try rebooting.", which has
    15: `Run2_S` of `src/c/view_t.c` is written 80 bytes long, with the
    first texts of the editor that the original has behind it.
  - Nothing is kept of what a program run as the viewer has shown: the
    screen is written as it was, and Ctrl+O shows what was there
    before.
  - A program is taken for one only if it is all that the line of the
    extension file has, with an extension of three characters; the
    character behind the name is made a zero byte before the name is
    found too long, 80 characters or more, and run as a command after
    all.
  - Behind the name of the file such a program gets nine bytes, 36
    zeros and the palette, of which nothing in the published source
    says who reads them. Ported as they are; no scenario shows them.
- Of the viewer's keys, 13 instructions have run in no scenario and are
  translated by reading, listed in `harness/ported.py`: Ctrl+Right at
  column 65455; Up to a line with none behind
  it, which cannot be; Down on the last line of a part that ends in the
  first half of the memory; and a text found by reading the file on
  less than 2000 bytes into it, which cannot be either. The fourth is
  within reach of a scenario that makes a file of more than 16376 line
  ends with nothing between them, by appending a file to itself some
  twenty times, and pages down a hundred times in hex: about a hundred
  steps for eight instructions. What they do then looks like a slip:
  the next part is read and shown from its start, half of the memory
  further on, and the lines between are skipped.
- What the viewer does and the translation does after it, found while its
  keys were translated; whether to mend any of it is not decided:
  - Esc, Ctrl-C or F10 typed while a file is being opened for the
    viewer or the editor stops it, and the file stays open: `V00_02`
    asks for the key when DOS has opened the file and does not close
    it.
  - Once a text has been found, the next search begins on the line
    behind the first line of the screen, so the same text is not found
    twice; in a file that is one line, or with the text in the last
    line, nothing more is found at all. And where fewer bytes are left
    in the memory than the text is long, the search says that it could
    not find the string without reading the file on. In `view-keys`.
  - A dirinfo that is too large for the editor is viewed in the window
    that the editor would have had in the Info panel. The box "Reading
    the file" stays on the screen above it, with its title changed to
    View; and in hex the viewer writes its lines, 77 columns, from the
    left edge of a window of 38: over the frame, into the row below,
    and into the other panel's half of it. In `view-window`.
  - A file whose length is below 64K but above what the viewer has
    room for gets that length for the bytes in the memory, until the
    table of lines is made, which cuts it down. Nothing reads it in
    between.
- A second click on a line of the help's index is Enter, and Enter goes
  by the button that the cursor of the keys is on: with that on Cancel,
  after Right, the second click leaves the help instead of showing the
  page of the line. Ported like that, and in `mouse-help`.
- The help trusts its file. `DeCode` takes a length of 0 for 65536, so an
  entry whose text has no length would have the whole main data changed,
  byte by byte; an index without lines has Enter take the place of a page
  from its text; and a line whose end is missing is looked for
  through 65535 bytes. Ported like that. The file of the distribution
  has none of these, and what a scenario makes of it by typing a letter
  into it has pages that are refused for their length. **The first is
  dealt with in the portable C**, on the user's word of 14 October
  2026: such a file is refused (`src/portable_c/DEVIATIONS.md`, 23).

- Go to asks for the disk of the wrong drive. `F67_00` hands `ChgDisk`
  the address of the line, which is in the memory of the list, and
  `ChgDisk` reads at that offset in the main data: two bytes of the
  first palette, since a line begins within 16 bytes of its segment. So
  the current drive is asked about, unless the second byte is 3Ah, a
  colon, and then the "drive" of the first. Ported like that in the C
  translation. **Dealt with in the portable C**, on the user's word of
  13 October 2026: it asks for the disk of the line's drive
  (`src/portable_c/DEVIATIONS.md`, 15). No scenario shows it yet. Go to
  also copies 69 bytes of the line into the path of the panel, which
  has 68; the last goes into the place of the cursor, which is set
  next.
- What `FndMove` does for the mouse runs in `mouse-find`, with the
  search over. While a search runs the mouse is taken too, by `FndBack`,
  and no scenario has it: a button would be pressed at no moment that
  the harness can name, as a key would. The two places where `FindFil`
  runs out of memory, for its list and for the file it reads, are
  translated by reading.
- `FndBack`, which takes the keys while a search runs, means to go back
  for the next key when the menu, "Stop, Go to, Quit", has taken one:
  Left, Right, Space, Backspace, a letter that is no answer. It tests
  the carry flag for that behind `MnMn40`, which draws the menu and ends
  in `Window`, and that leaves the carry clear. So it goes on to
  `FndMove` with what `MnMn40` left in AX, the colour of the chosen
  answer in both bytes, as if that were a key. `FndMove` knows no such
  key and does nothing, unless the colour is 0Dh or 1Bh, which it would
  take for Enter or Esc. On the bench the colour is 0Fh; the other
  palettes have not been looked at. Ported like that.
- `FindFil` calls itself, through `DirFunc`, once for every level of
  the directories it searches. The original took 18 bytes of its stack
  of 256 for a level, by reading; what the translation takes of its 4,096
  has not been measured, and how deep a tree either can search has not been
  tried. The scenarios search three levels. DOS's critical error can come up with
  all of that on the stack: `FindFil` calls DOS for the files, and to
  open and read them.
- What the commands of the menus do in the original and the translation does
  after them; whether to mend any of it is not decided:
  - The configuration and EGA lines end by writing the prompt and
    drawing the panels again "if the panel is on", with DS on the main
    data instead of a panel. The prompt takes of a panel only whether
    it is the left one, for a column that it does not use; the test
    reads the third byte of the main data, the colour of selected files
    in the first palette, 0Fh as the program comes, so the panels are
    always drawn again. A setup with that colour 0 would leave them
    undrawn after the configuration: by reading.
  - Brief, Full or an order chosen for a panel that shows the tree or
    the Info and is the current one has DOS go to the directory that
    the panel had as one of files. If that is gone, DOS stays where it
    is on that drive and the panel shows that directory, without a
    word. In `menu-panel`. If the diskette is refused, nothing changes
    and the panels are drawn again; for Info and Tree, which go to the
    directory of the other panel first, nothing is drawn. In `phantom`.
  - Menu file edit and the three for the extension files copy the
    title of their box as 80 bytes, with whatever follows it in the
    program, and the name of the file as 13; the 64 bytes that become
    the name of the file to edit have those behind the name's end. The
    C copies them from the assembly, as long as that has them. Behind
    " User Menu " the translation has other bytes than the original from the
    42nd on: the texts of the box of the drives stood there, and are in
    `src/c/drive.c` now. Nothing reads them.
  - The boxes of Save setup and of Compare directories are shown with
    DS on the current panel, where a box looks for the texts it is to
    put into its lines. They have none.
  - The configuration loads CL with whether the menu bar is always
    visible, before its dialog, and nothing reads it.
- In an open menu the key of an item that is dark, Ctrl+a in the Files
  menu of a tree, is not dropped: the number of the item is looked for
  among the letters of the items, where it is not. And the table of
  those keys has F1 for the Help item of the Files menu, which `Input`
  never hands on: it shows the help of the item under the cursor
  itself. Both ported as they are.
- F2 and Ctrl+R in that box have `DelTr0` spoil the tree file so that
  the disk is read again, and `DelTr0` is told the drive by a path at
  ES:SI. `ChTree` leaves in SI what `Case` had there, the address behind
  the key's entry in its table of keys, in the code segment; in the main
  data that is in `TreeBuf`, in the name of the 510th entry of the
  tree. If the third character of that name were a colon, or for Ctrl+R
  the seventh, the file of the drive named before it would be spoiled
  instead of the current one's. DOS gives no name with a colon, so it
  takes a tree file made by hand. Ported like that: the C hands `DelTr0`
  the two addresses of the original, E45Ah and E45Eh.
- What the commands of Copy and Rename do in the original and the
  translation does after them; whether to mend any of it is not decided:
  - When a directory of a tree panel has been renamed, the current
    directory of DOS is taken along if its path begins with that of the
    directory, whether a name ends there or not: with DOCS renamed to X
    while DOCS2 is the current directory, the program goes to X2 if
    there is one, and stays where it is if there is not. In `copy-tree`.
  - Shift-F6, and F6 with the panels off or the Info active, offer the
    name of the file under the cursor or the last target according to
    BX and DX, which hold the numbers of selected files only where F5
    and F6 have counted them: there they are what the command before
    left, and the name is offered if both are 0. The main loop hands
    them on as the original has them (see `src/c/main.c`). They are not
    both 0 behind any command that was found: the loop loads BX with
    an address where a command comes back to it, and the keys of the
    cursor, which come back behind that place, leave an address or a
    size in DX. So the name is never offered, which `rename-keys` and
    `copy-ask` show.
- A new name for a directory of the tree that makes its path longer
  than 64 characters is reported as "Can't create the directory", as a
  name with a path in it is. The first rests on reading: it takes a
  tree some ten levels deep.
- What the original does and the translation does after it, found while
  `C00_00` was translated; whether to mend any of it is not decided:
  - F6 on a directory offers its own name, and Enter on that moves the
    directory into itself: `DOCS\DOCS` is made, then found in `DOCS`
    and moved into itself in turn, until the path is too long, with a
    box for every level and the directories left behind. In no
    scenario.
  - A directory that is moved and cannot be removed afterwards, because
    a file in it was skipped, is searched once to see whether it is
    empty. That search takes the place of the one that walks the
    directory above, which then goes on with the entries of the
    directory below as if they were its own: `SRC\PAPERS\README.TXT`
    is tried once more as `SRC\README.TXT`, and cannot be opened. In
    `copy-dirs`.
  - Renaming a file onto one that is there, with aPpend for an answer,
    says that it cannot open the file if that file has no attribute at
    all: the copying takes the attributes for the room that the name
    has left. In `move`. By the same way a file that DOS will not
    rename, for another reason than one in the way, is not copied
    instead if the target's path is the start of the source's: by
    reading.
  - When copying stops, by Esc, an error or a full disk, the target is
    deleted, also one that was being appended to and had been there
    before. Seen while `copy-full` was written; as it is now it has no
    such step.
  - A directory of the tree that DOS will not rename, the current one
    under FreeDOS, is made under the new name and its contents moved.
    The first entry is then given the new directory itself for a
    target: an empty directory in it is removed, as if moved, and a
    file cannot be renamed. The entries after it get the name of the
    old directory behind that of the new one, which is nowhere. In
    `move`.
  - A source that turns out to be the console, or whose attributes DOS
    will not tell, is left open, and so is a target that is the
    console: `copy` does both.
  - After a write that is short, on a disk that is full, the next one
    begins where a whole number of paragraphs ends: up to 15 bytes are
    written twice, and as many are missing where what was in the memory
    ends. That happens when a file is appended to one whose size is no
    whole number of clusters, the disk gets full, and the user goes on
    and has the target overwritten. `copy-full` did that at first: the
    translation left a file of the same size and other contents, because
    where the memory ends depends on how much of it the program takes. It
    copies to new files now, so no scenario has it.
  - Of a device that gives its bytes in several calls of DOS to a
    piece, the room left in the memory is counted down by the last
    call only. By reading: the bench has no such device. The portable C
    counts it down by every call, on the user's word: entry 17 of
    `src/portable_c/DEVIATIONS.md`.
  - Where DOS will not tell the date and the time of a file that it has
    just opened, the copying goes on with CX and DX as they are, and
    the target is given that for a time and a date when it is closed.
    For a source that is a device they are what comparing the two names
    left and the address of the name; for a target that is appended to,
    its attributes as they are kept, or the 20h given to one that was
    read-only, and the address of the name. By reading: no scenario has
    DOS refuse that, and none could tell, since the one device that is
    a source, `nul` in `copy`, gives nothing to write. The translation
    had something else in CX at both places, what its stack held and
    the attributes as DOS answered them, until the assembly was read
    again for the portable C, call by call, and `C20_00` of
    `src/c/copy.c` mended.
- `copy-full` fills C: by copies, 32 MB, in half a minute. So a full
  disk can be made on the bench after all, and what the editor does when
  it saves to one, listed as out of reach for `T80_00`, could be shown.
- When the editor has no room left for a line that has grown, `T51_00`
  beeps and takes what room there is, but moves what follows the line up
  by as much as it had asked for: the end of the text is lost, by what
  did not fit, and not the end of the line. Ported like that. The
  scenario `editor-full` fills the memory at the end of the text, where
  nothing follows and the line itself is cut; with text behind the line
  it rests on reading.
- Del at the end of what the editor keeps of a line longer than 3900
  characters joins the next line to those 3900 and drops the rest of the
  line. Ported like that; `editor-long` and `editor-full` make their
  long lines that way.
- What the editor does with its keys, found while that was translated
  and ported as it is; whether to mend any of it is not decided:
  - Ctrl-Y on a line that the buffer holds a part of cuts the whole
    line and keeps only the part, 3900 characters, to be put back;
    Ctrl-K there cuts the rest off without keeping any of it. By
    reading.
  - Where Del joins two lines, and where Ctrl-K and Ctrl-Y look for the
    end of a line, `T53_00` is given a number of the line that is
    whatever the key before left in DI: the address of a cell of the
    status line as a rule. If it were 0FFFFh nothing would be joined or
    cut. The C hands it 0.
  - A new file is asked about, "New-file, Cancel", only in a window as
    high as the screen: not the menu file or an extension file, which
    have their help below, and not dirinfo in the Info panel.
  - PgUp and PgDn in a window of one row, and the clearing of a window
    of no rows, would go round 65,536 times: the Info panel's dirinfo
    with panels half the screen high might get there. Not tried.
  - A click of the mouse on the space behind the two digits of a byte
    puts the cursor on a third digit, which is taken for the second:
    the screen shows it on the second, and a digit typed there goes
    into it. In `mouse-edit`. A digit typed on the second digit of the
    byte behind the last one is cleared again: by reading.
  - The keys take the cursor no further than column 3900 of a line;
    the mouse does. A click in the last column of the window on
    another line than the cursor's puts the cursor there, and the
    window then begins a column further on, so the next such click is
    a column further: `mouse-edit` gets to column 3983 of a line of
    a:\kernel.sys, where the 3900 characters that the editor keeps of
    the line end, tabs counted as the columns they fill.
  - While a file is read, and before and after a search, the keyboard
    is looked at for Esc, Ctrl-C and F10. While it is read that cannot
    be seen: the viewer's code looks first, and takes the key.
- `T54_00`, which goes to the line before, looks backwards for where a
  line begins and then goes forwards again with `T53_00` until it is back
  where it came from, without asking whether `T53_00` found a line. If
  none were found before that place it would not end. That takes a text
  whose lines do not begin where going forwards has them, which hex mode
  might make; not tried. Ported like that.
- A search can be given a key only where it begins: with the Enter
  that starts it, written as one name, or behind the help that such a
  key brings up. So `find` no longer has a key that stops a search
  between two parts of a file it reads, and the five instructions of
  `FindFil` that close the file then, `FnFl48`, are listed as out of
  reach. To have that again, the driver of the tests would have to hand
  a key over when the search comes to a named place, in the original and
  in the translation alike: it would have to be told where that is in each,
  as it is told where `LeaveTm` is.
- `cmake --build build -j4` failed once in a new checkout, "The object is
  zero length" for one C file. Building without `-j` works.
- A run of the harness that is killed outright leaves its QEMU running,
  at full load.

- `^R` in a tree panel marks the tree's file as out of date and takes
  what `DelTr0` leaves in AL for the kind of the panel: 1, the one byte
  that DOS has written. With the file read-only the byte is not
  written, the routine beeps and leaves DOS's error there, 5: the panel
  is then one of a kind that there is not. The translation hands back the
  same 5. No scenario does it: what the C code of the panels makes of a
  fourth kind rests on reading, and whether it is what the original
  makes of it has not been tried. `delete` has `^R` with the file
  writable.
- A mask with a drive and no directory, "c:*.zzz" for Shift-F8, never
  says "Can't find files": `F38_70` calls DOS for an FCB with the carry
  flag set by a comparison before it, DOS leaves the flag, and `Intr21`
  takes it for an error. Ported like that, through `Intr21f`; `delete`
  has it. It rests on FreeDOS leaving the flag: of another DOS that
  clears it the original would say "Can't find files", and so would the
  translation.
- When DOS deletes the files of a directory at one call, it stops at
  the first that is read-only; the others are deleted one by one
  afterwards. Cancel in the box about a directory that is not empty
  leaves that directory and goes on with the next one selected, where
  Cancel for a read-only file stops the command. `M30_50` does not ask
  whether DOS could read or set the date and the time of a file. All
  ported as they are.
- F7 in a tree copies the path of the new directory into `CrnPath`
  from its third byte on, 66 bytes, and puts the zero behind them into
  the byte after `CrnPath`: the first of `ViewFil`, the name that
  Shift+F3, Shift+F4 and Shift+F8 offer again. Ported like that. And
  F7 with a name and no path adds the directory to the list of the
  current panel without asking whether that shows files, or has room.
- The volume label: the old one is deleted before the new one is made,
  so a label that cannot be made would leave none, on a DOS that
  deletes one. FreeDOS neither deletes the label nor shows the new
  one, and reports no error for either; what the program does where
  DOS does both rests on reading. Ported as it is.
- A drive that shares the floppy drive can be had on the bench after
  all: B:, typed. `delete-mask` has "Insert diskette for drive B:" and
  refuses, which is how `F38_70` came to stop there: its two
  instructions for that were listed as out of reach and are not any
  more. What copying lists as out of reach for "a disk the user will
  not put in" could be reached the same way, a path on B: and Esc;
  nobody has tried. `not-ready` goes further: with the diskette made
  B:'s, A: is a drive that is not ready for everything that reads it,
  which is what many lists in `harness/ported.py` call out of reach,
  "the drives of the bench are always ready". The top of
  `harness/scenarios.py` says how.
- What Enter and the keys next to it do in the original and the translation
  does after them; whether to mend any of it is not decided:
  - Enter on a file that is no program copies 3,999 bytes from where
    the panel keeps the extension file of its directory, which is 512:
    the rest of what the panel keeps and the start of its list of
    files come along into `MenuBuf`. The file ends within the 512, so
    nothing reads them.
  - Going up a directory takes the name of the one that is left from
    the path of the panel, 12 bytes whatever its length, to put the
    cursor on it: what follows the path in the panel's segment comes
    along behind the name's zero byte.
  - Before the command is echoed the BIOS is asked to write the end of
    a line with BX as it is, the address of the saved setup, where the
    BIOS takes a page of the screen and a colour: page 37 in the
    original. The C hands it the original's address, 25E5h; the BIOS of
    the bench goes by neither.
  - With more than 256 files in a panel only 256 are kept while a
    command runs, and the cursor's file is put into the last place if
    it is beyond; the list is read again afterwards and the cursor put
    on that name. In `enter-many`.
  - Shift held with Enter has the program, for that command, stay in
    the memory whole where the setup has it give most of it up, and
    the other way round. In `enter`.
- What the keys of the panels do in the original, and in the translation
  after it; whether to mend any of it is not decided:
  - Ctrl+R in a tree panel reads nothing if the disk has no tree file,
    as a disk with fewer than five directories has none: `DelTr0` cannot
    open the file to mark it, and the command takes that for a failure.
    `panel-keys` has it, on D:.
  - PgDn does not go a panel further in a list of less than two panels:
    it moves the list as far as that goes and leaves the cursor, unless
    it would be above the list. Thirteen directories in a tree of eleven
    lines: from the second to the third, and only then to the last.
  - Unselecting by a mask unselects directories too, which selecting and
    turning the selection round leave alone.
  - Ins tells the BIOS that the Ins key is up again, by clearing the
    bit in its byte of keys held down, when the cursor goes down a
    line.
  - Once a panel has shown both the tree and the Info, Ctrl+L and
    Ctrl+Z go from one to the other there and never back to the files.
- What the original does and the translation does after it, found while the
  user menu was translated; whether to mend any of it is not decided:
  - A line of a menu that has no command, at the end of the file, is
    run from the byte behind the end-of-file character: what MenuBuf has
    there from the menu or the setup that was in it before. If that
    began with a space or a tab it would be taken for a command. In
    `user-menu`, where it is none.
  - With Auto menus switched on, a line without a command brings the
    menu back, as if a command had run.
  - A comment among the commands of a line is one only at the left
    margin: behind a space or a tab it is run as a command, the
    apostrophe included. The editor's help for menu files shows it at
    the margin.
  - A key of a line comes before the keys of the menu: with a line
    "F1:" there is no help, with "F10:" no leaving by F10.
  - Of a command 123 characters are taken, but of what "!" stands for
    only the name is counted: the path of a file that is viewed, "!"
    with Shift+F3, is not.
  - When the commands of a line outgrow their 998 bytes while a name is
    put in, the routine leaves its loop without taking a word off the
    stack, and, for the name of a file of the panel, with DS on the
    panel: the line of a command that begins with "@" is then cleared in
    the colour that the byte of the panel at the place of `CrntCol` is.
    The translation clears it in that colour too; the word on the stack it
    does not leave. No scenario has commands that long.
  - The speed search hands `Input` the column of its cursor for the key
    bar to show. The key bar is off while it runs.
- What ran in the original is told by its bytes, wherever they are in
  the memory, so a program that runs off into what is left there of
  Volkov Commander counts as Volkov Commander running. A command of 33
  bytes, sixteen of them `a`, which is POPA, and a RET behind them that
  took its address from the PSP, went on at 7184h of the copy that DOS
  had loaded there to bring the program back, and `harness/reach.py`
  said that `CurNl` had been reached for a cursor below the screen. It
  had not. A step whose program does not come back, or comes back to a
  screen that it should not have left, is the sign; `start-screen` has
  its programs typed right.
- What running a command does in the original, and in the translation after
  it (`src/c/resident.c`); whether to mend any of it is not decided:
  - Whether DOS had a block of memory for the part of the program that
    is loaded again is taken from the block's address less a number of
    paragraphs, not from what DOS answered. DOS answers 8 or 7 for no
    block, which is less than that number, so it comes out right.
  - A command of more than 123 characters is cut at the 124th by the
    carriage return that ends it for COMMAND.COM, and the rest of it is
    skipped.
  - With Quick execute commands, COMMAND.COM's own interrupt runs the
    commands, INT 2Eh, and under FreeDOS's COMMAND.COM nothing is run
    at all and nothing is said: the prompt and the command are shown,
    and the program comes back. In `quick`.
  - The texts that the part that stays writes by itself, "Can't find
    the file" and the others, and the commands it shows, are written
    with whatever BH holds for the page of the screen: half of an
    address of its own, or of a number that DOS answered. The C hands
    the BIOS the same at each place, the address as the original has
    it: 2Bh where a file was to be run, the high byte of where the
    command lies for a prompt, 3 where there is no memory at the start.
    The BIOS of the bench, like most, writes to the page that shows
    whatever it is told, so no scenario can tell, and this rests on
    reading the original's listing.
  - When DOS ends the program by itself and goes on at the address
    that the program keeps in its PSP for that, the program that
    started this one gets the registers as the original leaves its
    own: ES on the program, which is free memory by then, CX 0, DS:DX
    the address that INT 27h went to before, and SI twice the length
    of its list when the last program on it was released. No program
    can rely on them. In `ended`, where a program made for it has DOS
    end a second copy.
- What the critical error does in the original, and in the translation after
  it (`FatErr` in `src/c/main.c`):
  - The number of the error is all of DI, of which DOS defines the low
    byte. A number of 21 or more is "Unknown error.", and so is any
    whose high byte is not 0: a copy to COM2, which the machine has
    not, says so under FreeDOS. In `device`.
  - Where DOS does not allow another try, the box has Ok alone and the
    call fails; where it does, anything but Retry fails it, Esc too.
    The program never has DOS end it.
- `CtrlC`, the routine that the program has for Ctrl-C of DOS and for
  Ctrl-Break of the BIOS, runs in no scenario: the BIOS of the bench
  makes no interrupt of Ctrl and ScrollLock, which was tried with a key
  added to `harness/driver.py` for it and taken out again. In the
  translation it is an interrupt function without a body, which saves the
  registers and puts them back where the original has one instruction.
- Ported as they are, in `src/c/main.c`:
  - When a panel cannot be read and another drive is chosen for it,
    the assembly notes the drive if none is noted as the one that
    failed. One always is: an instruction that never runs, `MOV CL,AL`
    in `Init60`.
  - After another drive has been chosen in the box, the window that is
    put on the screen has the number of that drive for a width.
    `Window` goes by the width for a single row only, so nothing shows.
  - The row below the panels is asked for with DS on the main data
    where a panel is meant, at `Init41`: `BegBX` answers for the right
    panel, and only the row is used, which both have.
  - A tree or an Info in the second panel is not read but given
    nothing to show when the current directory could not be had for
    the first panel, whatever it would show.
  - The box of a drive that is not ready tells DOS, as it closes, that
    the floppy drive's diskette is that of the letter it ends with
    (INT 21h, AX=440Fh), and given up, with a drive number that is no
    drive's, C0h. On the bench the diskette is then that of A: again.
    With the mouse, a button that is up again by the time the box asks
    gives up.
  - Leaving gives Ctrl-Break back and not the critical error, which
    DOS puts back by itself when a program ends.
  - When nothing is pending for the tree, the high word of `AutoTim`
    is whatever DX holds, and nothing reads it then.
  - `Mvz2lin` had no caller, and is gone.
- `AltSr_T`, the scan codes that begin the speed search, is a constant
  of `src/c/keys_t.c`, which the main loop reads, and the place behind
  the code found goes round in DI as in the assembly. `src/c/cmdline.c`
  has a copy of its own: the table wants one home.
- What the routines of this batch do in the original and the translation
  does after them; whether to mend any of it is not decided:
  - `SaveSet` makes the setup file anew before it writes it: when the
    writing fails, what the file held is gone. A file that could not be
    written whole stays as it is, and the error that comes back has a
    low byte of 0. By reading: it takes a full disk.
  - The random numbers are never seeded: see the blanker, above.
  - `Blank` goes away after its first turn when Shift, Ctrl or Alt is
    held as it comes up. By reading.
  - `Blank` steps a star's clock as a word, with the kind of the star
    in the high byte, and keeps the low byte: a clock of 0 would borrow
    from the kind. No star has one.
  - `LeaveTm` asks Windows for its turn to be passed on every time
    until Windows has once not answered: `WinLeav` is 1 when the call
    is not to be made, the other way round than its name says.
- One instruction of `SaveSet` is listed as out of reach, `MOV AL,0`
  where the diskette of the program's drive is refused or fewer bytes
  are written than the setup has. The program is on D:, a hard disk, so
  only a full disk gets there: with the setup file deleted first and D:
  filled as `copy-full` fills C:, saving would. That scenario is the
  longest by far, 136 seconds of 468 when four checkouts test at once,
  and this would be it once more for one instruction: say if it is
  wanted.
- What these routines do in the original and the translation does after
  them; whether to mend any of it is not decided:
  - `Window` tells TopView or DESQview which cells of their buffer have
    changed with ES on the main data, where the call takes the segment
    of the buffer. By reading: the bench runs neither. The portable C
    does not know of the two any more (`src/portable_c/DEVIATIONS.md`,
    2), and has no such call.
  - `Window` copies whole rows when a window has more than one, whatever
    its columns are, and of a window of one row and no columns 65536
    cells. `SavWin`, `ResWin` and `ResUser` take a window of no rows for
    one of 65536.
  - `DoublMo` adds the 40 hundredths that a second click may take to the
    time of the first byte by byte, and where the hundredths go beyond
    99 it carries a second and leaves them there: the click is in time
    until the next second ends, up to a second and 39 hundredths. The
    same goes for the seconds of a minute and the minutes of an hour,
    which are left at 60. And a click is the second one whenever the
    clock is not beyond the time noted: after midnight the next click on
    the line of the last one of the day before is a second click, however
    late. By reading.
  - `TstBrk` looks at the next key only. A key that does not stop what
    is going on stays where it is, and Esc typed behind it is not seen
    until that key has been taken, which is when the work is done. In
    `keys-waiting`.
  - A box that is smaller than the one it grows from, fewer than five
    rows or eleven columns, is drawn while it grows with a length worked
    out in a byte that has gone below 0: `MinBx5`. The program has no
    such box.
  - `ShadBox` takes ten columns off the width of a box in a byte, and
    writes the shadow below it over that many cells and eight, counted
    in a byte too: a box of two columns would have a shadow of 65536
    cells. The program has no such box.

## Ideas put off

- Keeping the booted bench from one run of a tool to the next. Booting DOS
  for the two countries and making the bench's setup file take about five
  seconds of every run. What is kept would have to be thrown away whenever
  the harness, a download or QEMU changes.
- A file or a directory that a scenario makes gets the time of day, to

## From CLAUDE.md

What `CLAUDE.md` had among its rules and notes that is still to do, moved
here when it was cut down to the rules as they stand (15 October 2026).

- The jumps that stay. The rule of `CLAUDE.md` ("One program, one place
  for the system") has the jumps of the assembly go where they are easy,
  and those of a function where loops are entered in the middle or paths
  cross stay until the function is rewritten for another reason. The
  sweeps G1 to G6 left `goto` in `view.c`, `usermenu.c`, `copy.c`,
  `menu.c`, `main.c` and `dirfunc.c` (`grep -c goto src/portable_c/*.c`
  counts them): `V00_10` of `view.c`, `C20_00` of `copy.c`, `F32_00` and
  `Run_00` of `usermenu.c` and `F39_00` of `menu.c` among them. Each goes
  when its function is rewritten.
- No test builds the toy that asks every function of `sys.h`,
  `harness/trial32/sys.c`, so a change to what a function takes leaves it
  broken without a word (the piece F9 did, and S found it so). After such
  a change, `harness/trial32/build.sh sys 32`, run from `build/`, and
  `harness/trial32/asked.py`; or a test that builds it.
- `IntOn` is a function of `sys.h` because the critical error's box gets
  its keys by the `STI` of `Window`, `MovScr` and `MinBx3`, which the
  original has there, three of them in the common code. Having the entry
  of the critical error let the machine interrupt before it asks the
  program would take those calls out of the common code, and would be the
  program doing otherwise than the original between the entry and its
  first `Window`: a deviation, to ask first. Not asked yet.
