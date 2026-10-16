# Testing

Every build is judged against the original program. The original was
taken through a set of scenarios, lists of key presses and mouse moves,
on DOS in an emulator, and what it showed after each step and the files
it left were recorded as goldens. A build passes a scenario when it does
the same, cell for cell.

This page explains how that works: the tests that `ctest` runs, the
scenarios and their goldens, the bench of emulated machines, how the
harness knows that a step is finished, and how it knows which code ran.
The README has the commands to run the tests; the tests of the program
for Linux are in [linux.md](linux.md#how-it-is-judged), and those of the
web page in [web.md](web.md#how-it-is-judged).

## Running the tests

Each test has a label for what it judges:

| Label | Tests |
|---|---|
| `portable_c` | the portable C |
| `frozen` | the assembly and the C translation |
| `harness` | `harness-le`, which judges the harness itself |
| `machine` | in addition, every test that starts a machine or takes a program through every scenario |

```sh
ctest --test-dir build -LE machine            # every test that starts no machine
ctest --test-dir build -L portable_c -LE machine
ctest --test-dir build -R portable_c -j4      # what a change to the portable C needs
```

The tests that run no machine take seconds, the terminal test some
forty. Each test that takes a program through every scenario takes
several minutes.

## The tests

### Of the assembly and the C translation

| Test | What it checks |
|---|---|
| `asm-bytes` | `VC.COM` and `VCSETUP.COM` from `build/shareware-layout/` are as long as the originals, hold only filler in the reserved room, and are identical to the originals everywhere else |
| `c-data` | Every text, box, dialog and table of the translation that has the name of one of the original holds the original's bytes, and a word that holds an address names what the original names there. None is shorter than what the code reads of it, which `harness/reads.py` finds: a name copied as 13 bytes is compared as 13. No machine is run |
| `c-arith` | The two routines of `src/c/arith.c`, which compiled code calls to multiply and divide 32-bit numbers, against Python, for 729 pairs at the edges and 4,000 random ones: the product, the quotient and the remainder. They are run as the bytes the compiler made, on a model of the processor, so a register that is not kept or a write to the caller's stack also shows. No scenario divides by more than 16 bits, so this is the only check of the rest. No machine is run |
| `c-scenarios` | The translation does what the original did in every scenario: the same screen after every step and the same files on its disks. Every translated instruction of the assembly ran in the original on the way, and every C function that replaces it ran in the translation. The translation is at most three times the original's size and leaves at least half of its stack untouched in every scenario. And the comparison itself is tested: with one cell of a screen changed, or one entry gone from a disk, it must report a difference |

`python3 harness/arithcheck.py build/c` runs `c-arith` by itself;
`--pairs N` takes that many random pairs. The routines were held to
200,000 pairs when they were written, which takes two and a half
minutes.

### Of the portable C for DOS

| Test | What it checks |
|---|---|
| `portable_c-data` | `c-data` for `build/portable_c/VC.EXE`, with the differences that `src/portable_c/DEVIATIONS.md` declares. See [below](#how-portable_c-data-reads-the-tables) |
| `portable_c-scenarios` | `c-scenarios` for the portable C, by the same `harness/ported.py` and against the same goldens, with the declared differences. See [below](#how-portable_c-scenarios-differs) |
| `portable_c-layout`, `portable_c-debug-layout` | The two programs of the [portable C](portable-c.md) as the linker wrote them, against the layout that the tests rest on. A program linked otherwise could have its stack where the driver does not look, or run many times slower on the bench, with no error. `harness/layoutcheck.py` checks that the stack is the segment that ends at the stack top named in the file's header and is in the file as zeros; that the file holds all the data but what has no value; and that nothing a scenario writes shares a memory page with code. Of the file itself: two objects, each as long as the map says, the entry at `Start`, every relocation an address with the right offset, and a stub that is DOS/32A's with only its banner switched off. Neither program is run, and no other test reads the debug program |

The source checks below read the code and run nothing. Each keeps a
record of what it found, and fails when the tree differs from the record
in either direction, so that what a change did shows in its commit.
`--record` writes the record anew and `--show` lists the places of a
file or of a kind.

| Test | What it checks | Record | Needs |
|---|---|---|---|
| `portable_c-host` | The source as another compiler reads it. `harness/hostcheck.py` has gcc read every file for a 64-bit machine, as strict C99, and counts by kind what it refuses (a pointer cast to a 16- or 32-bit number, a number made a pointer, a structure whose size differs from DOS's) and what only Open Watcom reads (an interrupt function, inline assembly, its own `#pragma`). `--macros` says which macros make the casts | `harness/hostcheck.txt` | gcc |
| `portable_c-int16` | Arithmetic that counts on a 16-bit `int`. C widens two 16-bit numbers to `int` before adding or subtracting them: unsigned and wrapped where `int` has 16 bits, signed and not wrapped where it has 32. The code was written and judged with a 16-bit `int` and is compiled with a 32-bit one. `harness/int16.py` has clang find every place where such a result is used before it is back in 16 bits: compared, divided, shifted right, widened, tested for 0, or added to a pointer; and where a negative number is kept unsigned in 16 bits, `(u16)-160`, which added to a pointer goes back only where pointers wrap at 16 bits. Each place has a reason in the record why both widths give the same; a place that can differ gets the cast that says what 16 bits do, `(u16)(a - b) < c`. The finder is checked against `harness/int16check.c` at every run | `harness/int16.txt` | clang |
| `portable_c-words` | How often each file writes each word for a place in memory, `MD_PLACE`, `ENTRY` and the like: the 16-bit offsets and places that remain from DOS's segments | `harness/words.txt` | |
| `portable_c-width` | The unsigned numbers of 8, 16 and 32 bits, which are to be as wide as what they count and signed unless there is a reason not to be. See [below](#portable_c-width) | `harness/width.txt` | clang |
| `portable_c-cut` | A narrowing cast that Open Watcom loses. See [below](#portable_c-cut) | | clang, for the half that reads the tree |

`harness-le` checks the harness's own reader of 32-bit DOS programs,
`harness/le.py`, which the tests of the portable C rely on.
`harness/lecheck.py` builds a toy, `harness/lecheck.c`, as a flat program
with DOS/32A for its stub, and checks what the reader finds against what
the toy's source says must be there: where the stub ends, the objects
and how much of each the file holds, the entry and the stack, where a
symbol is (a static one too), and every address the extender sets at
load time, including those that cross a page boundary and those that
point outside their object. The toy is not run; the top of
`harness/lecheck.py` says how it was checked on the bench by hand.

### How portable_c-data reads the tables

A text or a table of bytes is held to the original's bytes. A table that
is declared to differ is held against the original's with the
difference applied. The setup program's texts and tables, which have
`St` in front of their names in this tree, are held against
`VCSETUP.COM` in the same way.

A box, a dialog or a table of pointers is longer than the original's: a
word that names something is a C pointer, four bytes in a 32-bit
program. So these are compared field by field. The test reads which
fields are pointers from the declaration (`BOX`, `BOX1`, `BOX2` or
`DIALOG` of `src/portable_c/tables.h` with its count, or an array of
pointers), and fails for an object of another length, which is how a
gap put in by the compiler would show. There is none.

- A pointer where the original has a name must be an address that the
  extender sets at load time, a relocation, of the object of that name
  in the tree, at the same distance from it.
- A pointer where the original has a number (the length of an input
  line) or nothing (a line across a box) must be no such address, with
  the number in its low half and 0 in its high half.
- An address anywhere else in an object fails.
- A place in the main data is the original's 16-bit number.

As in `c-data`, nothing is shorter than what the code reads of it. The
bytes and addresses are read from the file with the harness's reader,
where the linker's map puts each object, and each object's length from
its object file, since the compiler rounds what a file has of a segment
up to four bytes.

### How portable_c-scenarios differs

- A scenario that is declared to change is held against the original's
  golden with the difference applied: steps left out, rules and its
  patch (see [Declared differences](#declared-differences)). One that
  only shows what the portable C no longer has is not run. The test
  says how many scenarios were held to the golden as it is, and names
  the others.
- A patch that does not fit its golden, or does not name an entry of
  `DEVIATIONS.md` that names its scenario, fails the test.
- Before anything runs, it checks the layout as `portable_c-layout`
  does. From QEMU's log of the code that ran, it checks that the program
  was in memory where the driver reads its variables and stack, and so
  was any copy that a scenario starts from its command line.
- Half of the stack must be left in every scenario.
- Every C function of a batch must have run, under the name it has in
  this tree: the entry of the critical error is `DeviceEntry` of
  `sys_dos.c`, and the entry of the program is `Start`.
- The file is held to no size limit; the test reports its length.

### portable_c-width

`harness/width.py` has clang read every file and find every unsigned
number that a file declares (a return value, a parameter, a variable, a
field of a structure), and every place where a signed number that may be
negative is shifted right or divided. It also prints, for whoever is
widening a number, every implicit cut: a number that may not fit by its
type, put into 8 or 16 bits with no cast. Those are not held.

In what has been sorted, every number that is still unsigned and every
such signed place has an entry in the record with its reason. A number
with its width or its lack of sign by what it is (a character, a cell of
the screen, a row and a column, bits, the size of a file) has one word
from a list at the top of the record; anything else has a sentence. What
has been made signed or boolean is counted in the table at the top, by
file. A structure whose layout is not the program's to change is listed
once, with whose it is. Of what is not sorted yet, the record has the
counts.

The test fails for a count that moved, a number new in something
sorted, or an entry without a reason. A sorted file is also held to
clang's `-Wconversion` (which includes `-Wimplicit-int-conversion` and
`-Wsign-conversion`), `-Wsign-compare` and two warnings about booleans,
none switched off for a line: where a number meets one of another sign
or goes into fewer bits, the code says so with a cast. Arithmetic that
stays in its own width, `word = word + 1`, and steps, `++` and `+=`,
are left out on purpose, as clang leaves them out. The finder is
checked against `harness/widthcheck.c` at every run. `--sort` makes
entries for what is not sorted.

### portable_c-cut

Open Watcom's compiler extends a signed number that is converted to a
wider unsigned type, and from there to a still wider type, with its sign
both times. A signed byte made a 16-bit unsigned number,
`(u16)(signed char)byte`, has its sign in all 32 bits wherever it goes
on into 32 bits: FFFFFFFEh for the byte FEh, where C says 0000FFFEh.
With a `short` in between it is right. The portable C writes neither:
where the original has `CBW`, it takes the byte as it is, or with its
sign into an `int`.

`harness/cut.py` has clang read every file and fails:

- for the lost form anywhere, even where a mask or arithmetic makes it
  come out right today;
- for the same fault with a 64-bit or floating type as the third type;
- for a build whose options expand functions in place, `-oe` or `-ox`,
  with which the fault also crosses calls.

It also compiles `harness/cutcheck.c` with the build's compiler and
options, and holds each function to the instructions written in front
of it: the lost form to the wrong ones, so that a mended compiler is
noticed, and the form with a `short`, with every other narrowing cast
(passed on, added, compared and used as an index), to the right ones.
Without clang the tree is not read, and the test counts as skipped if
the compiler's half has passed.

## Scenarios and goldens

A scenario is a list of key presses and mouse actions in
`harness/scenarios.py`. The original was taken through each one once,
and what it showed after starting and after every step, and what it
left on its disks, is kept as text in `harness/goldens/`. The goldens
are derived, never edited:

```sh
python3 harness/goldens.py derive [SCENARIO...]
```

### Differences in every build

A few differences hold for every build of every tree. They are applied
to a golden when it is derived, by the rules of `DEVIATIONS` in
`harness/deviations.py`, so that the golden holds what any build should
show:

- The banner names the edition, and the published source is of another
  edition than the binaries shipped in 2000. This holds for as much of
  the name as shows: its start next to a panel on the right, or its end
  next to one on the left.
- The program names its own file when it cannot load itself again after
  a command, and in the memory map: the original is `VC.COM`, a C tree
  `VC.EXE`.
- The command that starts the original's setup program is no line of
  what a build shows (see the next section).

What one tree is declared to do otherwise is not derived into the
goldens. It is applied when a golden is read for that tree (see
[Declared differences](#declared-differences)).

### Scenarios of the setup program

The original is two programs, `VC.COM` and its setup program,
`VCSETUP.COM`. The portable C is one program, with the setup as an item
of its Options menu. A scenario may be of the setup program, which it
says in `harness/scenarios.py`:

- The original's bench then has `VCSETUP.COM` next to `VC.COM`. A step
  starts it from the original's command line, with the banner it prints
  sent nowhere.
- The golden is what the setup program showed, driven and recorded as
  the main program is. The record of which code ran is of `VCSETUP.COM`,
  kept apart from those of `VC.COM`.
- A tree with the setup built in takes the same steps, but with the keys
  of its menu item in place of the command, and must show the same,
  screen for screen. Behind the setup is what DOS has shown, and the one
  line there that a build lacks is the command itself.
- The C translation has no setup program and is taken through none of
  these scenarios.

The top of `harness/scenarios.py` says how such a scenario is written,
derived and declared. `setup` is the first of them.

### Figures left out of the comparison

Some numbers on the screen are not compared, because they follow the
size of the program:

- the bytes free that the editor shows;
- the memory free that the Info panel shows;
- what `mem`, run as a command, says is in use and free.

The last two follow the size of the part that stays in memory while a
command runs, or for the Info panel, with Memory allocation switched
off, the size of the whole program. The C trees are larger than the
original, and an expectation that followed each build's size would not
be worth having. These figures show as `#` when a build is held against
a golden, so a golden derived with a figure in it need not be derived
again.

The editor's and the Info panel's figures are left out for the C
translation only. The portable C shows no figure of free memory
(`src/portable_c/DEVIATIONS.md`, entries 11 and 24): where the original
has one, the portable C is expected to show what the declaration says,
every cell. What `mem` says is left out for every tree.

### Declared differences

The original cannot be made to show what the portable C is declared to
do otherwise. So the portable C is held against the original's goldens
with the declared difference applied each time a golden is read for that
tree, and written nowhere. There are three forms:

| Form | For | Where |
|---|---|---|
| Steps left out | steps that pass through something the tree no longer has; they are left out with their screens | `TREES` of `harness/deviations.py` |
| A rule | what is otherwise on many screens, for that tree alone | `TREES` of `harness/deviations.py` |
| A patch | the rows that one scenario shows otherwise | `harness/goldens/portable_c/NAME.patch` |

`TREES` also lists the scenarios that are not run for the tree at all.
Each entry there must be named by an entry of
`src/portable_c/DEVIATIONS.md`, and a patch names the entries it is of:
the test fails for an entry that does not name the scenario, and for a
scenario that is not run and that no entry names.

**Rules.** The rules are for the program's name, which the original
shows on a thousand screens: in its banner behind the panels, in the
Info panel and over the help, often partly covered by a menu, a box or
a panel. A rule is a part of the screen, a `Part` of
`harness/deviations.py`: the text as the original shows it and as the
portable C does, cell for cell. Wherever enough of the original's text
shows on a screen of the golden to recognise it, each cell that matches
the text is expected as the portable C's text, and every other cell,
which is what covers the text, as the golden has it. So a rule leaves
no cell out of the comparison, and a screen where it is wrong about a
cell fails and needs a patch. The rule for the top of the Info panel is
four rows of the panel with their colours, since the portable C has
other rows there, without the original's two lines on memory. It
recognises its cells in the shadow of a menu too, and with the panel's
first rows scrolled off the top of the screen.

**Patches.** A patch says, of each row that differs, what the
original's golden has and what the portable C is expected to show, one
above the other, by the screen's place in the golden and the row
number:

```
Deviation: 7, 12 of src/portable_c/DEVIATIONS.md
Why: the key bar with Alt held has no name for F5 and none for F9, where the
original has Memory and EGA Ln

== 13: Alt+F10
row 23
-|1Left   2Right  3View.. 4Edit.. 5Memory 6DirSiz 7Find   8Histry 9EGA Ln 10Tree  | 07x1 30x6 ...
+|1Left   2Right  3View.. 4Edit.. 5       6DirSiz 7Find   8Histry 9       10Tree  | 07x1 30x6 ...
```

A patch is exact. It applies only where every row with `-` is what the
golden has; otherwise the test fails before anything runs, showing the
patch, the screen, the row and both lines. That happens when the
original's golden has been derived again and come out otherwise, or when
the scenario's steps have changed. It is a failure to look at: a patch
is not regenerated to make it go away. The top of `harness/goldens.py`
has the whole format, with the cursor, blinking cells and disks, and
says why it is not a unified diff.

```sh
python3 harness/goldens.py patch --tree portable_c SCENARIO...
python3 harness/goldens.py changes --tree portable_c [SCENARIO...]
```

`patch` takes the build through the scenario and writes every row that
differs from the original's golden, under a header to be filled in. What
it writes is what the build did. It becomes a patch when its author has
read every row against the entry of `DEVIATIONS.md` that declares it,
has named that entry and the reason in the header, and has committed
it: a statement of what the program is to show, which the tests then
hold it to. A row that the declaration does not explain is a change that
was not meant. `changes` prints what each patch says, with the differing
cells marked, and the steps that are left out.

### Differences of the C translation that show on no screen

Two differences between the C translation and the original change no
golden. They are listed in `harness/deviations.py` so that the list is
complete:

- When the original tries again to load itself after a command, it
  stores a word through a segment register that is left on the DOS
  environment, and the word lands in other data of its own. The
  translation stores it where it belongs.
- Of two codes in the markup of its texts, one for a colour and one for
  a character, the original's routine that writes a line takes each for
  the other, unlike the routine that measures the line and unlike Norton
  Guides, where the markup comes from. The translation has them agree.
  No text uses either code.

## Tools

| Command | What it does |
|---|---|
| `python3 harness/compare.py FILE [SCENARIO...]` | Takes any `VC.COM` or `VC.EXE` through scenarios and reports where it departs from the goldens. A `VC.COM` needs the listing that the build writes next to it, and the translation's `VC.EXE` the linker's map. This is how to run the programs assembled from `src/asm/`, which no test runs; the copy with the reserved room has no edition name in its banner, and differs wherever the banner shows |
| `python3 harness/show.py STEP...` | Prints the screens that the original shows after each step, or with `--program` those of a build: how to find out what keys do before writing them into a scenario. `--changes` prints only the rows that differ from the screen before, and names in a line the rows that are back as they were after an earlier step. A step that does not finish ends the run after five minutes, or after `--limit SECONDS`. `--setup` gives the bench the original's setup program |
| `python3 harness/reach.py ROUTINE...` | Says how many instructions of each routine of the original some scenario has run, and which are left. `--list` prints those left as the listing has them, `--steps` says which some steps would reach, and `--setup` is for the setup program's routines |
| `python3 harness/notice.py FILE OLD NEW` | Makes one change to a copy of the C tree that `FILE` is of, builds it, and says which scenario notices, or that none does. That an instruction ran in the original does not mean that a screen shows what it is for; this finds out, for a branch whose effect cannot be pointed to in a golden |
| `python3 harness/place.py` | Says where the stacks of the original and of the translation end on each bench |

## The bench

### The machines

DOS is FreeDOS in QEMU. It is booted once for each country that the
scenarios need, the United States and Russia, and the machine is saved
just before it starts the program. Each scenario then gets a machine of
its own in that state, which takes a tenth of a second:

- the program under test on drive `D:`, with the original's help and
  configuration files, started in `C:\WORK`;
- disks that no other scenario has touched;
- a third disk, with no file system, holding the scenario's script.

The program is kept off the drive it displays, so that its file size
does not appear on the screen as free space. A scenario that wants the
program set up differently does that itself, through the menus. Up to
four scenarios run side by side.

What the machines need while they run, mainly disk images of 60 to 130
MB per scenario, is kept in `build/tmp` of the checkout and removed when
the run ends; whatever a killed run leaves there, the next run removes.
A `TMPDIR` that is set is used instead. So is the system's temporary
directory, with a message, where the checkout lies so deep that the
path of QEMU's socket would be too long. `workdir` of `harness/dos.py`
is the one place that makes this directory.

The hard disks are SD cards, each on a controller of its own, which the
BIOS serves like any hard disk; DOS and the program know a disk only
through the BIOS. QEMU reads a sector of an SD card, as of a diskette,
while the machine executes the instruction that asks for it. A disk on
the IDE controller it reads in the background, and the BIOS polls for
the sector: how often depends on how busy the computer is, which made
the machine's time of day differ between runs. QEMU does not keep all of
the SD controller in a saved state, so the harness notes where the BIOS
put each one and tells the restored machine through the ports the BIOS
used.

Where DOS loads the program is chosen. QEMU checks every write to a
memory page holding code that it has run, and the original has its
code right behind its stack, so a push is many times slower there: a
third of the time of a scenario with dialogs. The driver of the tests
keeps a little more memory than it needs, so that the original's stack
ends at a page boundary. The C translation has more than a page of texts
behind its stack and does not depend on where it is loaded. The portable
C has its stack among its data above the megabyte, and the layout test
checks that nothing a scenario writes shares a page with code.

### The mouse and the keyboard

Both machines have a mouse: QEMU's PS/2 mouse and FreeDOS's driver for
it, CuteMouse, which the boot floppy lacks and the harness adds. The
bench loads it with `/3`, since it takes a PS/2 mouse for one with two
buttons until told of the third. The program shows the pointer, a cell
with its colours exchanged, in the middle of every screen until a
scenario moves it. A step moves the mouse and presses its buttons by
cell:

| Step | Action |
|---|---|
| `Click@3,5`, `RightClick@3,6`, `MiddleClick@3,7` | press and release a button at a cell |
| `Move@40,12` | move the pointer |
| `Click@3,5>3,2` | hold the button while the pointer moves |

The Russian machine also has a Russian keyboard: FreeDOS's keyboard
driver with its Russian layout, which the harness adds. It types Latin
letters until Alt and the right Shift select the Cyrillic ones. The
driver takes 40 KB, so this machine shows less memory free. The other
machine has only the PC's own keyboard support, and cannot type a letter
beyond ASCII: Alt with the numeric keypad types nothing there.

### How the program is set up

The program is set up as its distribution sets it up, with three
changes:

- **The clock is off.** It would put the time of day on every screen,
  and the machine's time is counted in instructions, of which the C
  trees take more than the original. The scenario `clock` switches it
  on again.
- **The right panel lists files**, where the distribution has the Info
  panel. That panel shows the memory size, the disk's label and its
  free space, things of the machine and the system, and would be on
  nearly every screen. A scenario about the Info panel switches it on.
- **The panels are sorted by name**, where the distribution leaves them
  unsorted. The portable C has no unsorted order (entry 29 of
  `src/portable_c/DEVIATIONS.md`), so the scenarios move the cursor to
  the files they mean in a sorted list. The directory tree and the file
  search have no order of their own and list a directory as it stands on
  the disk, so the bench creates its directories and files in name
  order, which makes those lists alphabetical on DOS as on Linux.
  Within a directory, the bench still creates the subdirectories before
  the files.

No setup file is prepared by hand. Whenever the harness sets up its
machines, it starts the original once, has it make these changes in its
own menus and save its setup, and the `VC.INI` it writes is what every
scenario starts from.

### Files to work on

A scenario that needs files that are not the bench's own finds them in
`C:\FILES`, on its own bench only, since the bench's disks are on every
golden: the files of FreeDOS's boot diskette's `FREEDOS\BIN` in `BIN`,
and three files beside it, two from the diskette and one from the
distribution, with their times (`files_layout` of `harness/setups.py`).
The program for Linux has the same files in `H:\FILES`, so such a
scenario runs alike on every platform.

## When a step is finished

The screen has to be read when the program has done everything a key
makes it do, and not before; and a key has to come when the program is
ready for it. Neither can be decided from outside the machine. So DOS
loads a small resident program before Volkov Commander, the driver of
the tests (`harness/DRIVER.ASM`, with `harness/driver.py` for the host's
side), which takes the program through the scenario's script.

### Feeding keys and the mouse

The driver stands in for nothing:

- **Keys** are the scan codes a keyboard would send, handed to the
  keyboard controller with its command for that. The BIOS, and on the
  Russian machine DOS's keyboard driver, make of them what they make of
  a real keyboard's: the key in the BIOS's buffer, Shift, Ctrl and Alt
  in its flags. The program's every request for a key is answered by the
  BIOS as always.
- **The mouse** is fed the same way: its packets go to the controller's
  auxiliary port, the BIOS hands them to the mouse driver, and the
  driver moves the pointer and answers the program. A mouse does not say
  where it is, so a step first takes the pointer to the top left corner,
  then to its cell, in movements too small for the driver to accelerate.

The driver only watches three interrupts on their way to their owners:

- the one by which a program says that it is idle, which Volkov
  Commander calls each time it has gone round waiting for a key;
- the BIOS's keyboard interrupt, for a program that waits inside the
  BIOS;
- the mouse driver's, where the program asks which buttons are down. A
  button comes up after the program has asked so many times: a program
  that waits for a button to come up is not idle and asks nothing else.

### Steps

When the program is idle and the BIOS's buffer is empty, it has taken
every key so far, and the driver hands over the next. So no key is ever
found waiting by a program that looks for keys while it works. Shift,
Ctrl and Alt go down and come up one at a time, with a round of the
program in between.

- A key written together with the key that starts some work, as
  `Enter+Home`, goes into the buffer with it, for a scenario that wants
  the program to find a key while it works, as `find` does in a search.
- `Wait*18` lets 18 ticks of the timer, one second, pass with nothing
  pressed: the driver hands over nothing until the BIOS's tick count has
  risen that much, and the program does meanwhile what it does by
  itself, such as blanking the screen after a minute.

A step is finished when the program has gone round twice with no key
left and nothing that it will do by itself after a while. The driver
reads that in the program's own data: a time of day, which it trusts
for two seconds, since the program's data holds no meaningful time
until the main loop has run. The driver then writes the screen, with
the cursor, to a port that QEMU keeps in a file, and goes on with the
next step. After the last, it resets the machine, which ends QEMU. The
host starts the machine and reads the file and the disks afterwards.

The original and the C translation are driven alike, and neither is
changed for it. What the driver must know of a program, its name and
where two of its variables are, is in the script, taken from the
listing of the original's assembly: the translation keeps those
variables where the original does. The driver tells the program's idle
calls from DOS's by where they come from: the memory of the program that
DOS has as current, with that name. None of this is measured in time,
so a busy computer makes a run slower, not different.

### The portable C under the driver

The portable C is driven by the same driver, unchanged. It is a 32-bit
program under [the extender](portable-c.md#the-extender), which makes
two differences:

- Its idle call comes through the extender's part in real mode, which
  lies in the memory block that DOS has as the current program's, so
  the driver recognises it as it does any other.
- Its data is above the megabyte, where a real-mode program cannot
  read. So the script gives the driver three addresses: of the two
  variables and of the stack. `harness/driver.py` works them out from
  the program's file, the linker's map and where the extender puts the
  program on the bench (`BENCH` of `harness/le.py`). The driver reads
  there through the BIOS's block move, INT 15h function 87h, each time
  the program comes round. Only such a script switches that part of the
  driver on, by an event in front of its first key, so the original
  runs through the same instructions as before: every golden was
  derived again to check that, and none changed.

A scenario may start the program a second time from the first copy's
command line. The extender loads that copy right behind the first; the
script has the addresses of three copies, each started by the one
before, and the driver finds which one is idle by counting the programs
of that name along DOS's memory blocks. From QEMU's log of the code that
ran, the scenarios' test checks that the program, and every copy, was
where the driver was told.

A second of the machine costs more of the computer's time while this
program idles than while the original or the C translation does,
because of the way through the extender and the block moves at every
round. Nothing that a scenario shows differs for it.

### Time

Where the program goes by time, it is the machine's time, and the
machine's clock is counted in the instructions it executes, 256
nanoseconds each (QEMU's `-icount`). The timer's ticks, the time of day
and the program's waits come when the machine has done so much, not
when the computer has taken so long:

- Where the machine is halted, as the BIOS halts it while a program
  waits there for a key, its clock goes straight on to the next timer
  tick (`sleep=off`).
- Nothing in the machine waits for the computer: its disks answer
  within the instruction that asks.
- The state that every scenario starts from is saved where the machine
  stops itself, which the driver does through QEMU's port for that when
  it is loaded and finds no script.

So every scenario begins at the same instruction and the same time of
day, and reaches each step at the same hundredth of a second, on every
run and however busy the computer. The original shows its first screen
some five seconds past noon. A scenario that shows the time of day must
still do so within the machine's first minute, since the trees do not
execute the same number of instructions and their clocks drift apart.

With the clock on, when a golden is derived, the driver looks at the
screen every time the program comes round, for more than a second of
timer ticks, and writes out both states of what blinks.

### Limits

Nothing waits without a limit. A program that hangs fails its scenario
when the driver has written nothing for five minutes, with the screen it
got stuck on, and one that resets its machine fails at once. The five
minutes are of both clocks: when the computer's are over, the harness
asks QEMU how many instructions the machine has executed since the
driver last wrote, and gives up only if those are five minutes of the
machine's clock too, or if it executes nothing at all. So a machine
that the computer has given little time is waited for. A step that has
the driver wait gets the seconds of its wait on top. Nothing keeps runs
of the harness apart: several checkouts may run machines on one
computer at once, and each only gets slower.

## What ran

Which code ran is taken from QEMU's log of the code it translates, which
`harness/coverage.py` matches against the program file:

- **The original**: the log is recorded with the goldens, and mapped to
  source lines through the listing of the build in
  `build/shareware-layout/`, which has every instruction at its
  original address.
- **The C translation** is not in memory quite as in its file: the
  program adds its load segment to the words its relocation table
  names. The file is matched with those words as the program has them.
- **The portable C** is a linear executable behind its stub, and the
  extender sets every address in it at load time. `harness/le.py` makes
  of the file what memory then holds, with the first object where it is
  on the bench, and the log is matched against its code.
