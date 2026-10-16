# Testing

How the builds are judged: the tests that `ctest` runs, the scenarios
and their goldens, the machines of the bench, how the harness knows that
a step is finished, and how it knows what ran. The README has the
commands; this page has how it works.

## The tests

Every test has a label for what it judges: `portable_c` for those of
the portable C, `frozen` for those of the assembly and the C
translation, and `harness` for `harness-le`. Those that start a machine,
or take a program through every scenario, have the label `machine` too:

```sh
ctest --test-dir build -LE machine            # every test that starts no machine
ctest --test-dir build -L portable_c -LE machine
ctest --test-dir build -R portable_c -j4      # what a change to the portable C needs
```

The tests that run no machine take seconds, the terminal's some forty;
each that takes a program through every scenario takes several
minutes. The tests of DOS and of the two C trees are below; those of
the program for Linux are in [linux.md](linux.md#how-it-is-judged),
those of the web page in [web.md](web.md#how-it-is-judged). Two more
are of the app for iOS, which is under way: `portable_c-ios-keys`, the
table of a hardware keyboard's keys on iOS run on Linux and held to the
BIOS's words that the program reads (`harness/ioskeys.py`), and
`portable_c-ios-host-scenarios`, the program of the app built on Linux
against a stand-in for its shell, `ios-host/vc`, through the scenarios.

| Test | What it checks |
|---|---|
| `asm-bytes` | `VC.COM` and `VCSETUP.COM` from `build/shareware-layout/` are as long as the originals, hold only filler in the reserved room, and are identical to the originals everywhere else |
| `c-data` | Every text, box, dialog and table of the translation that has the name of one of the original holds the original's bytes; a word that holds an address has that of what the original names there. And none is shorter than what the code reads of it, which `harness/reads.py` finds: a name that is copied as 13 bytes is compared as 13. No machine is run for it |
| `c-arith` | The two routines of `src/c/arith.c`, which the compiled code calls to multiply and to divide numbers of 32 bits, give what Python gives for some 4,700 pairs of numbers, those at the edges among them: the product, the quotient and the remainder. They are run as the bytes that the compiler made of them, called near or far as each returns, on a model of the processor that knows their instructions, so it also shows a register that is not kept and a write to the caller's stack. No scenario divides by more than 16 bits, and none would show a result that is wrong for some numbers only. No machine is run for it |
| `portable_c-data` | The check of `c-data` for the portable C, `build/portable_c/VC.EXE`: every text, box, dialog and table that has the name of one of the original, against the original's bytes, but where `src/portable_c/DEVIATIONS.md` declares otherwise. A table that is declared to differ is held against the original's with that difference made to it. A text, box, dialog or table of the setup program, which has `St` in front of its name in this tree, is held against the bytes of the original's `VCSETUP.COM` in the same way. A text or a table of bytes is the original's bytes. A box, a dialog and a table of pointers are not: a word that names something is a pointer in C, four bytes in a program of 32 bits with whatever stands behind it that much further on, so the object is longer than the original's and is held against it field by field. Which of its fields are pointers, the test reads in the declaration, `BOX`, `BOX1`, `BOX2` or `DIALOG` of `src/portable_c/tables.h` with its count, or an array of pointers; it fails for an object that is not as long as its declaration makes it with pointers of four bytes, which is how it would show a gap that the compiler had put in, and there is none. A pointer that the original has a name for has to be an address that the extender sets when it loads the program, a relocation of the file, of the thing that the tree has by that name, with the same distance from it; one that the original has a number for, the length of a line that is typed into, or nothing, a line across a box, has to be no such address, with the number in its low half and 0 in its high one; and an address anywhere else in an object fails. A place of the main data is the number of 16 bits that the original has. None of these is shorter than what the code reads of it, as in the C translation. The bytes and the addresses are read in the file, with the harness's reader, where each object is in the linker's map, and how long it is in its object file, since the compiler rounds what a file has of a segment up to four bytes. No machine is run for it |
| `portable_c-host` | The source of the portable C as another compiler reads it: `harness/hostcheck.py` has gcc read every file of `src/portable_c/` for a machine of 64 bits, as strict C of 1999, and counts by kind what gcc refuses, a pointer cast to a number of 16 or 32 bits, a number made a pointer, a structure that is not as long as on DOS, and what only Open Watcom reads, an interrupt function, inline assembly, a `#pragma` of its own. `harness/hostcheck.txt` has what was counted last, of every file, and the test fails where a file has another count of any kind: more, and a clean file that is clean no more, and fewer too, so that the record is always what is so, and what a change did to it shows in the commit. `python3 harness/hostcheck.py --record` writes the record anew, `--show` lists the places of a file or of a kind, and `--macros` says which macros make the casts. Nothing is compiled or run for it. It is skipped where there is no gcc |
| `portable_c-int16` | The arithmetic of the portable C that counts on an int of 16 bits. C makes an int of two numbers of 16 bits before it adds them or takes one from the other: without a sign and wrapped round where an int has 16 bits, with a sign and not wrapped where it has 32. `harness/int16.py` has clang read every file, for a machine whose int has 32 bits, and works out by the rules of C where a number that an int of 16 bits and one of 32 may have otherwise is looked at before it is put back into 16 bits: held against another, divided, shifted right, made a number of 32 bits, asked whether it is 0, added to a pointer; and where a number that is written below 0 is kept in 16 bits without a sign, `(u16)-160`, which added to a pointer is the way back only where a pointer wraps round at 16 bits. `harness/int16.txt` has every such place, with the reason why it is the same with either int all the same, written by whoever read it: the code was written and judged with an int of 16 bits and is compiled with one of 32, so a place is where the program can do otherwise than what it was translated from, and where step 2 of the road widens a variable; a place that can differ has no reason and gets the cast that says what an int of 16 bits does, `(u16)(a - b) < c`. The test fails for a place that the record has not, for one of the record that is gone, and for one without a reason. The finder is held against lines that were written for it, `harness/int16check.c`, each time it runs. `--show` lists the places of a file or of a kind, and `--record` writes the record anew and keeps the reasons. Nothing is compiled or run for it. It is skipped where there is no clang |
| `portable_c-words` | The words by which the portable C says where a thing is in the memory, `MD_PLACE`, `ENTRY` and their like, which stood for what a segment and an offset were on DOS and went as a segment became a pointer, `AT` and `SEG_OF` the last of them; those that are left are an offset or a place of 16 bits that stays. `harness/words.py` counts how often each file of `src/portable_c/` writes each, `harness/words.txt` has what was counted last, and the test fails where a file has another count, fewer as well as more, so that what a change did shows in the commit. `--record` writes the record anew and `--show` lists the places of a word or of a file. Nothing is compiled or run for it |
| `portable_c-width` | The numbers without a sign of the portable C, of 8, 16 and 32 bits, which step 2 of the road makes as wide as what they count and gives a sign unless there is a reason for none. `harness/width.py` has clang read every file, as the test of the int does, and finds every such number that a file declares, what a function returns, a parameter, a variable, a field of a structure; where a number with a sign that may be below 0 by its type is shifted to the right or divided; and, to print for whoever is to widen a number and held by nothing, every cut, where a number that may not fit, by the types alone, is put into 8 or 16 bits with no cast, by being kept, stepped, handed to a function or returned. `harness/width.txt` has the numbers and the places of a sign. Of a function or a structure that has been sorted by the rule of that step, every number that is still without a sign and every such place is an entry with its reason, written by whoever read it: for a number that has its width, or no sign, by what it is, a character, a cell of the screen, a row and a column, bits, the size of a file, one word of a list that the record has at its top with what each says, since the code gives no type a name for what a number means, and for anything else a sentence; what was given a sign or made a boolean is counted in the table at its top, by file and by the clusters of the plan; a structure whose layout is not the program's to change is there once, with whose it is; and of what nobody has read yet it has the counts. The test fails where the tree is not as the record has it, a count that moved, fewer as well as more, a number that is new in something sorted, an entry without a reason, so that a number added later is sorted and a commit shows what it widened. And a file that is sorted is held to the casts: where a number meets one of another sign or is put into fewer bits, the code says so with a cast, and no list beside the code has such a place. The test fails for what clang says of such a file under `-Wconversion`, all of it, which has `-Wimplicit-int-conversion` and `-Wsign-conversion`, under `-Wsign-compare`, and under two warnings of a boolean that is none, none of them switched off for a line, which it also looks for. What clang names is what is held: arithmetic on numbers of 8 or 16 bits that is kept in as many, `word = word + 1`, and a step, `++` and `+=`, it leaves out on purpose, and such a place stays as it is written. The finder is held against lines that were written for it, `harness/widthcheck.c`, each time it runs. `--show` lists the numbers and the cuts of a file, of a function, or what is cut in being handed to a function, in every file; `--sort` makes entries of what is not sorted; `--record` writes the record anew and keeps the reasons. Nothing is compiled or run for it. It is skipped where there is no clang |
| `portable_c-cut` | A cut that Open Watcom's compiler loses, and those that it keeps. It extends a number with a sign that is converted to a wider type without one, and from there to a still wider type, with its sign both times: a byte with a sign made a number of 16 bits without one, `(u16)(signed char)byte`, has its sign in all 32 bits wherever it goes on into 32 bits, FFFFFFFEh for the byte FEh where C says 0000FFFEh. With a `short` between it is right; the portable C writes neither, but takes the byte that the original's `CBW` makes a word as it is, or with its sign into an int. `harness/cut.py` has clang read every file of the tree and fails for the form that is lost, anywhere, also where a mask or arithmetic with a number that is written makes it come out right today, and for a number of 64 bits or a floating type, where the same fault is with another third type; it fails for a build whose options expand a function in place, `-oe` or `-ox`, with which the fault is across a call as well; and it compiles `harness/cutcheck.c` with the compiler and the options that the build compiles the tree with and holds each of its functions to the instructions that are written in front of it: those that have the fault to the wrong ones, so that a compiler which is mended is noticed, and the form with a `short` between, with every other cast that narrows, each handed on, added, compared and made an index, to the right ones. It runs no machine; without clang the tree is not read, and the test counts as skipped if the compiler's half has passed. |
| `portable_c-layout`, `portable_c-debug-layout` | [The two programs of the portable C](portable-c.md), each as the linker wrote it, against the layout that the tests rest on: one that is linked otherwise has its stack where the driver of the tests does not look, or is many times slower on the machine of the tests, with no word said. `harness/layoutcheck.py` reads the file and the linker's map of it, with the harness's reader, and holds: that the stack is the segment which ends where the file's header has the stack's top, as the driver of the tests looks for it, and is in the file as zeros; that the file holds of the data everything but what has no value; and that nothing which a scenario writes shares a page of the memory with code. And of the file itself: two objects, each as long as the map has it, the entry at `Start`, every relocation an address with the offset of what it names in the file, and a stub that is DOS/32A's with the banner switched off and nothing else changed. Until the piece F10 of step 2 it held the tables, `IniSave` and the end of the program to where the megabyte that the program kept inside its own let a segment and an offset name them: those rules went with that megabyte. No machine is run for them, and neither program is run; the debug program is read by no other test |
| `harness-le` | It judges the harness and no tree, and is named for that. The harness's own reader of a 32-bit program for DOS, `harness/le.py`, which the tests of the portable C go by. `harness/lecheck.py` builds a toy, `harness/lecheck.c`, as a flat program with DOS/32A for its stub, with the compiler and the linker that the build fetches, and holds what the reader finds in the file and in the linker's map against what the toy's source says must be there: where the stub ends, the objects and how much of each the file has, the entry and the stack, where a symbol is, a static one too, and every address that the extender puts in when it loads the program, with those that lie across the end of a page and those that name something outside their object. The toy is not run; what it shows of itself on the bench, and how that is held against the reader by hand, is at the top of `harness/lecheck.py`. No machine is run for it |
| `portable_c-scenarios` | The check of `c-scenarios` for the portable C, by the same `harness/ported.py` and against the same goldens: what is said of the translation in the row below holds for it, but where `src/portable_c/DEVIATIONS.md` declares otherwise. A scenario that is declared to change is held against the original's golden with that difference made to it, the steps that are left out, the rules and the rows of its patch, and one that only shows what the portable C has no more is not run; the test says how many scenarios were held against the original's golden as it is, and names the others. A patch that does not fit its golden, or that names no entry of `DEVIATIONS.md` which names its scenario, fails the test. Before anything is run it asks `harness/layoutcheck.py` for the layout, as the test `portable_c-layout` does; and from QEMU's log of the code that ran, it holds that the program was in the memory where the driver of the tests reads its variables and its stack, and so was a copy of it that a scenario starts from its command line. Half of the stack is to be left in every scenario. Every C function of a batch has to have run, under the name that this tree has it by: the entry of the critical error is `DeviceEntry` of `sys_dos.c`, and the entry of the program `Start`. Its file is held to no multiple of the original's size: the test says how long it is |
| `c-scenarios` | The translation does what the original did in every scenario: the same screen after every step, the same files on its disks. Every instruction of the assembly that has been translated ran in the original on the way, and every C function that replaces it ran in the translation. The translation is not more than three times the size of the original, and leaves at least half of its stack untouched in every scenario. And the comparison itself: with one cell of a screen changed, or one entry gone from a disk, it must report a difference |


## Scenarios and goldens

A scenario is a list of key presses, written down in `harness/scenarios.py`.
The original was taken through each one once, and what it showed after the
start and after every step, and what it left on its disks, is kept in
`harness/goldens/` as text. A build is right if it does the same. The
goldens are derived, never edited:

```sh
python3 harness/goldens.py derive [SCENARIO...]
```

A few differences hold for every build, of every tree, and are made to
a golden when it is derived, by the rules of `DEVIATIONS` in
`harness/deviations.py`, so that the golden holds what any build should
show. The banner names the edition, and the published source is of
another edition than the binaries shipped in 2000: that holds for as
much of the name as shows, its beginning next to a panel on the right or
its end next to one on the left. The program names its own file, when
it cannot load it again after a command and in the map of the memory:
the original is `VC.COM`, a C tree `VC.EXE`. And the command that
starts the original's setup program, of which the next paragraph says
more, is no line of what a build shows. What one tree is declared to do
otherwise is not derived into the goldens: it is applied when the
golden is read for that tree, as below.

The original is two programs, `VC.COM` and its setup program,
`VCSETUP.COM`, and the portable C is one, with the setup an item of
its Options menu. A scenario may be of the setup program: it says so in
`harness/scenarios.py`, and the original then has `VCSETUP.COM` of its
distribution next to it on the bench, which a step starts from the
original's own command line, with the banner that it prints sent
nowhere. Its golden is what the setup program showed, driven and
recorded as the main program is, and the list of what ran that is kept
with it is of `VCSETUP.COM`, apart from those of `VC.COM`. A tree that
has the setup in itself is taken through the same steps, but for the
keys of its menu's item in place of the command, and has to show the
same, screen for screen: behind the setup program is what DOS has shown,
and the one line there that a build has not is the command's own. The C
translation has no setup program and is taken through none of these
scenarios. The top of `harness/scenarios.py` says how such a scenario is
written, derived and declared; `setup` is the first.

Some numbers on the screen are not compared at all: the bytes that the
editor says it has free, the memory that the Info panel says is free, and
what `mem`, run as a command, says is in use and free. All follow the
size of the program, the last two the size of the part that stays in
memory while a command runs, or the Info's that of all of it when Memory
allocation is switched off. The translation is larger than the original and
has less, by more with every routine translated, and the numbers are not
worth an expectation that follows the size of the build. Both screens
have `#` in their place when a build is held against a golden, so a
golden that was derived with a number still in it need not be derived
again. The editor's and the Info's numbers are left out for the C
translation alone: the portable C shows no figure of free memory
(`src/portable_c/DEVIATIONS.md`, 11 and 24), and where the original has
one it is expected to show what the declaration says, every cell of
it. What `mem` says is left out for every tree.

What the portable C is declared to do otherwise than the original is
another matter, since the original cannot be made to show it. The
portable C is held against the original's goldens there too, with the
declared difference made to a golden each time it is read for that
tree, and written nowhere: the steps that the tree is not taken through
are left out with their screens; what is otherwise on many screens is a
rule of the comparison, for that tree alone; and the rows that one
scenario shows otherwise are its patch, `NAME.patch` in
`harness/goldens/portable_c/`. The rules are of the program's name,
which the original has on a thousand screens, in its banner behind the
panels, in the Info panel and over the help, often with a menu, a box
or a panel over a part of it. Such a rule is a part of the screen,
`Part` of `harness/deviations.py`: the text as the original shows it
and as the portable C does, cell for cell. Wherever enough of the
original's text shows on a screen of the golden to know it by, each
cell that the golden has as that text has it is expected as the
portable C's text has it, and every other cell, which is what covers
the text, as the golden has it. So a rule leaves no cell out of the
comparison, and a screen on which it is wrong of a cell fails and gets
a patch. The rule for the top of the Info panel is four rows of the
panel with their colours, since the portable C has other rows there,
without the original's two lines on the memory: it knows its cells in
the shadow of a menu too, and the panel with its first rows gone off
the top of the screen. A patch says of each such row what the
original's golden has and what the portable C is expected to show, one
above the other, by the screen's line in the golden and the number of
the row:

```
Deviation: 7 of src/portable_c/DEVIATIONS.md
Why: the key bar with Alt held has no name for F5, where the original
has Memory

== 13: Alt+F10
row 23
-|1Left   2Right  3View.. 4Edit.. 5Memory 6DirSiz ...| 07x1 30x6 ...
+|1Left   2Right  3View.. 4Edit.. 5       6DirSiz ...| 07x1 30x6 ...
```

It is exact: it is made to the golden only where every row with `-` is
what the golden has, and otherwise the test fails before anything is
run, with the patch, the screen, the row and both lines. That happens
when the original's golden has been derived again and come out
otherwise, or when the scenario has other steps, and is a failure to
look at: a patch is not made again to make it go away. The top of
`harness/goldens.py` has the whole form, with the cursor, the cells
that blink and the disks, and says why it is not a unified diff.

```sh
python3 harness/goldens.py patch --tree portable_c SCENARIO...
python3 harness/goldens.py changes --tree portable_c [SCENARIO...]
```

The first is how the rows of a patch are got: it takes the build
through the scenario and writes every row that differs from the
original's golden, under a header to be filled in. What it writes is
what the build did. It becomes a patch when its author has read every
row against the entry of `src/portable_c/DEVIATIONS.md` that declares
it, has named that entry and the reason in the header, and has
committed it: a statement of what the program is to show, which the
tests then hold the program to, not another run. A row that the
declaration does not explain is a change that was not meant. The
second prints what each patch says, the rows with the cells that differ
marked, and the steps that are left out. Which steps are left out of
every scenario for the tree, which of one scenario, where they pass
through something that the tree has no more, which rules it has and
which scenarios are not run for it at all is in
`harness/deviations.py`, under `TREES`, as
`src/portable_c/DEVIATIONS.md` declares it; a patch names the entries
of that file which it is of, and the test fails for an entry that does
not name the scenario, and for a scenario that is not run and that no
entry names.

Two more deviations show on no screen, and are listed in the same file.
When the original tries again to load itself after a command, it stores a
word through a segment register that is left on the DOS environment, and
the word lands in other data of its own. The translation stores it where it
belongs. And of two codes in the markup of its texts, one for a colour and
one for a character, the original's routine that writes a line has each
for the other, unlike the routine that measures it and unlike Norton
Guides, where the markup comes from. The translation has them agree. No text
uses either code.

`python3 harness/compare.py FILE [SCENARIO...]` takes any
`VC.COM` or `VC.EXE` through scenarios and reports where it departs from
the goldens. For a `VC.COM` it needs the listing that the build writes
next to the file, and for the translation's `VC.EXE` the linker's map there.
No test runs the programs assembled from `src/asm/`, which the
byte comparison answers for; this is how to run them. The copy with the
reserved room has no edition name in its banner, and differs from the
goldens wherever the banner shows.

`python3 harness/show.py STEP...` prints the screens that the original
shows after each of some steps, or with `--program` those of a build. It
is how to find out what keys do before writing them into a scenario. With
`--changes` it prints of each screen only the rows that differ from the
one before, which is most of what there is to see; rows that are again as
they were after an earlier step, as when a dialog closes, it names in a
line. A step that the program does not get through ends the run after
five minutes, as in a test, or after `--limit SECONDS`, with the screen
it stopped on. With `--setup` the bench has the original's setup program
for a step to start.

`python3 harness/reach.py ROUTINE...` says of each routine of the original
how many of its instructions some scenario has run and which are left, by
their addresses, in a line; with `--list` it prints those that are left as
the listing has them. With `--steps` it says which of them some steps
would reach, and takes `--limit` like `show.py`. With `--setup` the
routines are those of the original's setup program, and what has run is
what the scenarios of the setup program have run of it.

`python3 harness/notice.py FILE OLD NEW` makes one change to a copy of a
C tree, the one that `FILE` is of, builds it, and says which scenario
notices, or that none does.
That an instruction ran in the original does not mean that a screen shows
what it is for; this is how to find out, for a branch whose effect cannot be
pointed to in a golden.

`python3 harness/arithcheck.py build/c` is the test `c-arith` by
itself; with `--pairs N` it takes that many random pairs of numbers
where the test takes 4,000. The two routines were held to 200,000 when
they were written, which takes two and a half minutes.

## The machines

DOS is booted once for each country that scenarios need, the United States
and Russia, and the machine is saved as it is about to start the program.
Every scenario then gets a machine of its own in that state, which takes a
tenth of a second: the program under test on drive `D:` with the original's
help and configuration files, started in `C:\WORK`, on disks that no other
scenario has touched, and on a third disk, one without a file system, the
script of the scenario. Keeping the program off the drive it displays stops
its file size from appearing on screen as free space. A scenario that wants
the program set up differently does that itself, through the menus. When a
build is tested, up to four scenarios run side by side.

What the machines need while they run, the images of their disks above
all, 60 to 130 MB for a scenario, is kept in `build/tmp` of the checkout
and removed when a run ends; what a run has left there by being killed,
the next run removes. A directory that the environment names for
temporary files, by `TMPDIR`, is used in its place. So is the system's,
with a line that says so, where the checkout lies so deep that the
socket by which QEMU is spoken to would have too long a path there.
`workdir` of `harness/dos.py` is the one place that makes such a
directory.

The hard disks are SD cards, each in a controller of its own, which the
BIOS serves like any hard disk; DOS and the program know a disk only
through the BIOS. QEMU reads a sector of an SD card, as of a diskette,
while the machine executes the instruction that asks for it. A disk on
its IDE controller it reads in the background, and the BIOS asks in a
loop whether the sector is there yet: how often, depends on how busy the
computer is, and that made a machine's time of day differ from one run
to the next (see below). QEMU does not keep all of such a controller in
a saved state, so the harness notes where the BIOS has put each one and
tells the machine that goes on from the state, through the ports that
the BIOS used.

Where DOS loads the program is chosen: the driver of the tests keeps some
memory more than it needs, so that the original's stack ends where a page
of the machine's memory ends. QEMU has to look at every write to a page
with code in it that it has run, and the program has its code right
behind its stack: a push is many times slower there than elsewhere, which
is a third of the time of a scenario with dialogs or more. The translation
has its texts behind its stack, more than a page of them, and does not
depend on where it is loaded. The portable C has its stack
among its data above the megabyte, where the linker put it, and the
test of its layout holds that nothing which a scenario writes shares a
page with code.
`python3 harness/place.py` says where the stacks of the original and of
the translation end on each bench.

Both machines have a mouse: the PS/2 mouse of QEMU's PC, and FreeDOS's
driver for it, CuteMouse, which the boot floppy lacks and the harness adds.
The program finds the driver and shows the pointer, a cell of the screen
with its colours exchanged, in the middle of every screen until a
scenario moves it. A step moves the mouse and presses its buttons by the
cell it goes to: "Click@3,5", "RightClick@3,6", "MiddleClick@3,7",
"Move@40,12"; a button held while the pointer moves is "Click@3,5>3,2".
CuteMouse takes a PS/2 mouse for one of two buttons until it is told of
the third, so the bench loads it with its option for that, /3.

The Russian machine also has a Russian keyboard: the keyboard driver of
FreeDOS with its Russian layout, which the boot floppy lacks and the
harness adds. It types Latin letters until Alt and the right Shift select
its Cyrillic ones, and a scenario can then type Russian text. The driver
takes 40 KB, so this machine shows less memory free than the other. The
other machine has the keyboard a PC has by itself, and no way to type a
letter beyond ASCII: Alt with the numeric keypad types nothing there.

The program is set up as its distribution sets it up, with three
exceptions.
The clock in the corner of the screen is off. It would put the time of day
on every screen, and the machine's time is counted in the instructions it
executes (see below), of which the translation takes more than the original
to get through a scenario. And the panel on the right lists files, as the
one on the left does, where the distribution has the Info panel there: that
shows the size of the memory, the label of the disk and what is free on it,
things of the machine and of the system, and it would be on nearly every
screen of nearly every scenario. No setup file
is prepared for either. Whenever the harness sets up its machines, the
original is started once, made to switch the clock off in its Options menu,
to make the right panel one of files in its Right menu, and to save its
setup, and the `VC.INI` it writes is what every scenario
starts from. One scenario, `clock`, switches the clock on again, and a
scenario that is about the Info panel switches that on.

The third is that the panels of the bench list their files sorted by
name, where the distribution has them unsorted:
the same setup file is saved with Name chosen in the Left and the Right
menu. The order of a directory is that of its entries on the disk,
which the portable C no longer shows (entry 29 of
`src/portable_c/DEVIATIONS.md`), so the scenarios move the cursor to
the files that they mean in a sorted list. The tree of directories and
the file search have no order of their own and list a directory as it
stands on the disk, so the bench makes
its directories and its files in the order of their names, so that
those lists are alphabetical on DOS too, as they are on Linux. A
directory still stands in front of the files of its directory, as the
bench makes them.

A scenario that wants files to work on that are not the bench's own has
them in `C:\FILES`, on its bench alone, since the bench's disks are on
every golden: the files of FreeDOS's boot diskette's `FREEDOS\BIN` in
`BIN`, and three files in front of it, two of the diskette's and one of
the distribution's, with their times (`files_layout` of
`harness/setups.py`). Such a scenario went to `A:`, the diskette, or to
`D:\VC`, the original's directory, for them before; the program for
Linux has the same files in `H:\FILES`, so that the scenario runs alike
on every platform, and its golden was derived again from the original.

## When a step is finished

The screen has to be read when the program has done all that a key makes
it do, and not before; and a key has to come when the program is ready for
it. Neither is decided from outside the machine. DOS loads a small
resident program before Volkov Commander, the driver of the tests
(`harness/DRIVER.ASM`, with `harness/driver.py` for the host's side), and
that takes the program through the steps of the scenario's script.

The driver stands in for nothing. A key is the scan codes that the
keyboard would send, handed to the keyboard controller with the command
it has for that, so the BIOS, and on the Russian machine the keyboard
driver of DOS, make of them what they make of a keyboard's: the key in the
BIOS's buffer, and Shift, Ctrl and Alt in its flags. Every request the
program makes for a key is answered by the BIOS as always. The mouse is
fed the same way: its packets, the buttons and how far it has moved, go
to the controller's auxiliary port, the BIOS hands them to the mouse
driver, and the mouse driver moves the pointer and answers the program.
A mouse does not say where it is, so a step takes the pointer to the top
left corner first and from there to its cell, in movements too small for
the driver to speed them up.

The driver only watches three interrupts on their way to their owners:
the one by which a program says that it is idle, which Volkov Commander
calls every time it has gone round waiting for a key; the BIOS's
keyboard interrupt, for a program that waits inside the BIOS; and the
mouse driver's, where the program asks which buttons are down. A button
of the mouse comes up when the program has asked that so many times: a
program that waits for it to come up is not idle, and asks nothing else.

When the program is idle and the BIOS's buffer is empty, it has taken
every key so far, and the driver hands over the next: so no key is ever
found waiting by a program that looks for keys while it works. Shift, Ctrl
and Alt go down and come up one at a time, with a round of the program
between. Where a scenario wants the program to find a key while it works,
as `find` does of a search, the key is written as one name with the key
that starts the work, "Enter+Home": the two go into the buffer together.
A step may also let time pass with nothing pressed, "Wait*18" for 18
ticks of the machine's timer, a second: the driver hands the program
nothing more until the BIOS's count of ticks has gone up by so many, and
the program does meanwhile what it does by itself, as after a minute
without a key, when it blanks the screen.
A step is finished when the program has gone round twice with no key left
and nothing that it will do by itself after a while, which the driver
reads in the program's own data: a time of day, which it believes for two
seconds, since the program's data holds no time that means anything
until the main loop has run. The driver then writes the screen, with
the cursor, to a port that QEMU keeps in a file, and goes on with the
next step; after the last it resets the machine, which ends QEMU. The
host starts the machine and reads the file and the disks afterwards.

The original and the translation are driven alike, and neither is changed
for it. What the driver has to know of a program, its name and where two of
its variables are, it is told in the script, which takes that from the
listing of the original's assembly: the translation keeps those variables
where the original does. It knows the program's call of the
idle interrupt from DOS's by where the call comes from: the memory of the
program that DOS has as the current one, and that has that name. None of this is measured
in time, and a busy computer makes a run slower, not different.

The portable C is driven by the same driver, and is not changed for it
either. It is a program of 32 bits under [the extender](portable-c.md#the-extender),
and that is another thing to the driver in two ways. Its idle call comes
through
the extender, from the extender's part of real mode, which lies in the
block of the program that DOS has as the current one: the driver knows
the call as it knows any. But the program's data is above the megabyte,
where a program of real mode cannot read. So the script tells the
driver three addresses of the machine's memory, of the two variables
and of the stack, which `harness/driver.py` works out from the
program's file, the linker's map and the place where the extender puts
the program on the bench (`BENCH` of `harness/le.py`); and the driver
reads there with the BIOS's service for moving a block of the memory,
INT 15h with 87h, every time the program comes round. That is a part of
the driver that only such a script switches on, by an event in front of
its first key, so the original goes through the instructions that it
went through before the driver had it: every golden was derived again
to see that, and none changed. A scenario may start the program a
second time from the command line of the first. The extender loads
that copy right behind the first, the script has the addresses of
three copies, each started by the one before, and the driver finds out
which of them idles by counting the programs of that name along DOS's
blocks of memory. From QEMU's log of the code that ran, the scenarios'
test holds that the program, and every copy of it, was where the
driver was told. A machine's second takes more of the computer's time
while this program idles than while the original or the C translation
does, for the way through the extender and the block moves at every
round; nothing that a scenario shows differs for it.

Where time is what the program goes by, it is the machine's, and the
machine's clock is counted in the instructions that the machine executes,
256 nanoseconds for each (QEMU's `-icount`): the timer's ticks, the time
of day and the waits of the program come when the machine has done so
much, not when the computer has had so long. Nothing of the computer's
time gets into it. Where the machine is halted, as the BIOS halts it
while a program waits there for a key, its clock goes straight on to the
next tick of the timer (`sleep=off`). Nothing in the machine waits for
the computer: its disks answer within the instruction that asks. And the
state that every scenario starts from is saved where the machine stops
itself, which the driver has it do, by the port QEMU has for that, when
it is loaded and finds no script. So every scenario begins at the same
instruction and the same time of day, and gets to each of its steps at
the same hundredth of a second, in every run and on a computer however
busy. The program's first screen comes some ten seconds past noon. A
scenario that shows the time of day still has to do so early, within the
machine's first minute: the translation and the original do not execute the
same number of instructions, and their clocks drift apart by that.

With the clock on, when a golden is derived, the driver looks at the
screen every time the program comes round, for more than a second of the
BIOS's timer ticks, and writes out both states of what blinks.

Nothing waits without a limit. A program that hangs fails its scenario
when the driver has written nothing for five minutes, with the screen it
got stuck on, and one that resets its machine fails at once. The five
minutes are of the computer's clock and of the machine's own, both: when
the computer's are over, the harness asks QEMU how many instructions the
machine has executed since the driver last wrote, and gives the program
up only if those are five minutes of the machine's clock too, or if it
executes nothing at all. So a machine that the computer has given little
time is waited for, and a step takes as long as it takes there; a step
that has the driver wait has the seconds of its wait besides. Nothing
keeps runs of the harness apart: several checkouts may run their machines
on one computer at once, and each only gets slower.

## What ran

Which code ran is taken from QEMU's log of the code it translates, which
`harness/coverage.py` matches against the program file. For the original
it is recorded with the goldens, and mapped to source lines through the
listing of the build in `build/shareware-layout/`, which has every
instruction at the address it has in the original. The translation's code is
not in the memory quite as it is in its file: the program adds the segment
it is loaded at to the words that its relocation table names, and the file
is held against the log with those words as the program has them where
it is. The portable C is a linear executable behind its stub, and the
extender puts an address into every place of it that names something
when it loads it: `harness/le.py` makes of the file what the memory then
holds, with the first object where it is on the bench, and the log is
held against that, as far as it is code.

The reserved room is listed in `harness/bytecheck.py`, with the offset and
length of each piece and a note of what the original has there.
