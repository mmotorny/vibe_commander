# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## The last of the assembly

Every routine of the program is C. What `port/asm/` still holds is data
and glue, and the port ends without it. This is what is there, how it is
held against the original while it moves, and in what order it can go.
How one file is done is in `CLAUDE.md`, "Moving the data of a file to C";
`VCHELP.INC` went that way, into `port/help_t.c`.

### What is left

Objects and bytes as they are linked, after the help's file had gone. An
object is what stands from one label to the next. "Code" is all that is
no data: stubs, entries and what jumps between them. A file with nothing
but code in its row has its data in C. `VC.ASM` had 14 texts of 696
bytes: the fifteenth that was counted here was `TrnBeg`, a label where
the first of them stood.

| File | texts | boxes | dialogs | buttons, colours | tables | code | goes with |
|---|---|---|---|---|---|---|---|
| `VC.ASM` | | | | | 2 / 160 | 219 | `main.c`, `start.c` |
| `VCFUNC.INC` | | | | | 2 / 76 | 980 | `keys.c`, `usermenu.c` |
| `VCMENU.INC` | | | | | 8 / 174 | 495 | `commands.c`, `menu.c` |
| `VCF1.INC` | | | | | | 217 | `dirfunc.c` |
| `VCCOPY.INC` | | | | | 1 / 10 | 82 | `copy.c` |
| `VCVIEW.INC` | | | | | | 180 | `view.c` |
| `VCEDIT.INC` | | | | | | 26 | `edit.c` |
| `VCMEM.INC` | | | | | | 40 | `memory.c` |
| `VCFIND.INC` | | | | | | 22 | `find.c` |
| `VCSUB1.INC` | | | | | | 50 | `dialog.c` |
| `VCSUB2.INC` | 2 / 49 | | | | | | `bars.c`, `panel.c` |
| `VCSUB3.INC` | 13 / 182 | 3 / 52 | 1 / 22 | 1 / 5 | | 109 | `treebox.c`, `system.c`, `read.c` |
| all | 15 / 231 | 3 / 52 | 1 / 22 | 1 / 5 | 13 / 420 | 2,420 | |

Besides these, all in `VC.ASM`:

- The part that stays in the memory: 213 bytes with contents (the seven
  words at 100h that `tools/mkexe.c` fills in, `RunPar`, `ChkFile`, the
  name `COMMAND.COM`, the command tail of the second copy, five texts),
  29 bytes of code (`Exec`, `AlocEr`, `Exit00` and the stub of
  `Release`), and room without contents: the 14,751 bytes of `RDATA`,
  which are the table of vectors, the list of programs, the copies of
  the panels and of the setup and the stack of 4,096, and the 16 of
  `RGAP`.
- The layout: the `STRUC` of the main data, 62,603 bytes that the
  program asks DOS for, that of a panel, and a dozen small ones; in the
  files those of the viewer, the editor and the file search, of which C
  has its own; and the `EQU` of the colours and the lengths.
- The compiler's helpers `__U4M` and `__U4D`, 92 bytes, in `CTEXT`.
- The macros `CallC`, `MainC`, `CmdC`, `MnuC` and `PanC`; `FromC`,
  `CallLine` and `TakeLine` have no use left.

The tables are `Main_T`, the 79 places that the main loop goes on at, and
`SecDat`, the word with the segment of the main data, in `VC.ASM`;
`CmdNx_T` and `AltSr_T` in `VCFUNC.INC`; the seven tables of the menus'
commands and `MnuNx_T` in `VCMENU.INC`; `F35_T` in `VCCOPY.INC`.

The code is, by what it is there for:

- The stubs of the commands, about 150 labels: a command is a label that
  the main loop or a menu jumps to by a table, with the registers of its
  family (`CmdC` with `CmdEnd` and `CmdNx_T` in `VCFUNC.INC`, `MnuC` and
  `PanC` with `MnuEnd` and `MnuNx_T` in `VCMENU.INC`, `F1Next` in
  `VCF1.INC`, `F35_T` in `VCCOPY.INC`), calls the C function and jumps on
  to where that says, which may be another command.
- The glue of the main loop in `VC.ASM`: the stubs of `Init00` and
  `Init11`, the eight entries made with `MainC`, `MainGo`, `Exit`.
- Stubs of routines that other stubs call: `AdrNum`, `NumAdr`, `ChgAdr`
  and `F20_02` in `VCFUNC.INC`, `Dialog` in `VCSUB1.INC`, `ReDir` and
  `ReadTre` in `VCSUB3.INC`, `DelTree` in `VCF1.INC`, and those that a
  dialog names for Enter and F10: `F38_10`, `M30_60`, `F35_60`.
- Entries for C: `ReDir_a` in `VC.ASM`; `MemRun_a`, `ResSeg_a` and
  `Release_a` in `VCMEM.INC`; `AsmSeg_a` and `DialSub_a` in
  `VCSUB1.INC`, with `CallPSP`.

What names what:

- C code names a text, a box, a dialog or buttons of the assembly as
  `extern const u8 __far`, with `#pragma aux Name "*"`, and hands its
  offset to `DialBox`, `Dialog`, `MinMnu` or `OncZoom`, or reads it
  through the far pointer. The assembly has the name `PUBLIC`, in the
  file that defines it.
- An object names another by a word that holds its address: a box its
  lines, a dialog its box, its buttons and the routine for Enter, a
  table its labels. Sixteen are named from more than one file: `Ok_S`,
  `OkMn_D`, `Var_S`, `Inc_S`, `Err_S` of `port/main_t.c`, which were of
  `VC.ASM`; `Empty_S`, `Kov_S`,
  `YesNo_S`, `YesNo_D` of `port/keys_t.c`, which were of
  `VCFUNC.INC`; `OkCan_S`, `OkCan_D` of
  `VCSUB3.INC`; `Var2_S`, `NoFnd_S` of `port/dirfunc_t.c`, which were of
  `VCF1.INC`; `EdFl_S` of `port/view_t.c`, which was of `VCVIEW.INC`;
  `Tree_S`, `PtMnu_S` of `VCSUB2.INC`. `VC.ASM` declares
  those `EXTERNDEF`, so that either side can name one wherever it is.
- Code of the assembly names data in one place: the stub of the menu bar
  loads the offset of `PtMnu_S`.
- The harness and the tools:
  - `harness/driver.py` reads where `AutoTim` and `Clock` are in the
    main data from the assembler's listing, `build/port/VC.LST`, and
    where the stack is, `DataSav` and `VcStack`, from the linker's map;
    `harness/ported.py` takes the stack from the map too. The two
    offsets are those of the original, which the listing of
    `build/shareware-layout/` has, as long as the main data is laid out
    as the original's; the stack wants two names in the map.
  - `harness/datacheck.py` reads `VC.LST` and `VC.OBJ` of the port for
    what is still assembly, and does without them when they are gone.
  - `harness/coverage.py`, `harness/reach.py` and `harness/notice.py`
    read the listing of the original only, and need nothing.
  - `tools/routines.py` and `tools/flagscan.py` read `port/asm/`: they
    are for routines that are still assembly, and go with it.
  - `tools/mkexe.c` reads the seven words at 100h, which the assembly
    lays out, and the map.

### What holds the data while it moves

The test `data`, `harness/datacheck.py`, holds every object of the port
that has the name of an object of the original against the original's
bytes, without a machine: 96 constants of the C code, 2,976 bytes, and
373 objects of the assembly, 8,545 bytes, when it was made. It names
each. A word that holds an address is held by what it names: the
original's source is assembled once more as an object file, whose
relocations say which words those are, and the port has to have at such
a word the address of what has that name in the port. So nothing
describes an object to the tool; an object is written with the names of
the assembly and the linker does the rest.

It found five constants of the C code that were not as the original, all
texts with a zero byte behind them that the original has not. Four made
no difference, and are exact now: `SubDir`, `Ctrl_S`, `Byts_S`, `MenuLn`.
The fifth did: the end of the banner, `CrLf_S`, is copied as far as a
zero byte, and the original copies the text behind it along,
`Press ENTER to return`, into the buffer that DOS writes from as far as
the dollar sign. The C has the texts in one array now and copies as the
original does. No screen shows it.

What it does not hold, each listed in the tool with the reason:

- Constants that the original has not as data: `PowersOfTen`, `Panel_T`,
  `Scan_P`, `Fnc_T`.
- `Suff_S`, the edition in the banner: it lies in room that is reserved
  for what only the shipped binary has.
- Data that became code: the tables of keys are `switch` statements, and
  the scenarios answer for them.
- Tables of the assembly that the original has not: `Main_T`, `CmdNx_T`,
  `MnuNx_T`, `F35_T`.

### The order

**1. The data, file by file.** Several agents at once, one assembly file
each, by the recipe in `CLAUDE.md`. The texts, boxes, dialogs, buttons
and colours of the file go to a file of constants; the tables of
commands and the code stay. Which file of constants:

| Assembly | Constants |
|---|---|
| `VCFIND.INC` | `find_t.c` |
| `VCMEM.INC` | `memory_t.c` |
| `VCEDIT.INC` | `edit_t.c` |
| `VCVIEW.INC` | `view_t.c` |
| `VCCOPY.INC` | `copy_t.c` |
| `VCF1.INC` | `dirfunc_t.c` |
| `VCMENU.INC` | `commands_t.c` |
| `VCFUNC.INC` | `keys_t.c` |
| `VCSUB2.INC`, `VCSUB3.INC` | `bars_t.c`, `treebox_t.c`, `system_t.c` |
| `VC.ASM`, its texts and boxes only | `main_t.c` |

No two agents have the same assembly file, and `VCSUB2.INC` and
`VCSUB3.INC` are one agent's. None of them needs to change a C file of
code, a file of another agent or the lists of `VC.ASM`: a name stays what
it is when its object moves. What all of them may add to, a line at a
time, is the tables of `harness/datacheck.py` and the types of
`port/tables.h`. Done: `VCMEM.INC`; `VCFIND.INC`, whose two texts
that are copied with what follows them are written as long as what is
copied; `VCCOPY.INC`, where the titles `Copy0_S` and `Ren0_S` are such
texts too, 20 bytes each; `VCFUNC.INC`, where the names of the menu file
and of the list file are, 13 bytes each, and whose `AltSr_T` stays with
the tables; `VCF1.INC`, where `DlWr4_S` is one, 81 bytes; `VCMENU.INC`,
where the title `ExFl_S` is one, 80 bytes, and `Chk1_S`, which is read
as far as the end of `Chk2_S` behind it, is ten; `VCVIEW.INC`, where
the names of the two extension files are such texts, 13 bytes each, and
`Run2_S`, 80 bytes; `VCEDIT.INC`, which has none; the texts and boxes
of `VC.ASM`, which have none either.

**2. The words that name a routine.** One agent, after 1, since it
changes dialogs that are constants by then. A dialog names a routine for
Enter and F10 in its header, `SubFunc`: `F38_10`, `M30_60`, `F35_60` and
`F35_61`. They become a table of functions in `port/dialog.c`, and the
word the place in it, as `FncAddr` did (see `Fnc_T` of
`port/dirfunc.c`); `DialSub_a` and the stubs go, and the test `data` is
told of each such word in `NAMED_OTHERWISE`, as a number.

**3. The commands and the main loop.** One agent, or one after another,
never two at once: all of it is one web of jumps through `port/main.c`,
`port/menu.c` and the `.CODE` of every file. The loop of `port/main.c`
calls the functions of the commands, by a `switch` where they take
different things; the stubs of the commands go with `Main_T`, `CmdNx_T`,
`MnuNx_T`, `F35_T`, `F1Next` and the tables of the menus, then `MainC`
and `MainGo`: `Stay` of `port/resident.c` calls the loop, which wants DS
on the main data, the one instruction that `FatErr` has too. The stubs
that only stubs call go as their callers do, and `ReDir_a` when `ReDir`
is one function. The other entries, tried as far as compiling:
`ResSeg_a` is a function that returns the segment of a far pointer to a
variable of its own, which compiles to `mov ax,ss`; `AsmSeg_a` is the
segment of a far pointer to a constant of the group, `mov ax,seg Name`,
a number that is relocated; `MemRun_a` reads two words of the part that stays through `ResSeg`;
`Release_a` and `CallPSP` go when `Release` of `port/resident.c` is a
far function that loads its own DS, as `SetBrk` is. With the last
command the files of the assembly are empty and go, with their
`INCLUDE` and their names in `CMakeLists.txt`.

**4. What is not of one file.** After 3, but for the helpers, which can
go at any time, side by side with everything.

- The helpers `__U4M` and `__U4D` can be C. Tried: a function may have
  such a name, `#pragma aux U4M "__U4M" __near parm [dx ax] [cx bx]
  value [dx ax]`, and stays in `CTEXT` with its callers. The product is
  made of products of two bytes, which need no helper, where a word by a
  word as a number of 32 bits would call `__U4M` itself; the quotient by
  shifting and subtracting, which need none either. What C cannot say is
  that `__U4D` hands back the remainder in CX:BX next to the quotient:
  a `#pragma aux` that holds no instruction, `parm [cx] [bx] modify
  exact []`, has the compiler load the two as the last thing before the
  result. That is no assembly, and it rests on what the compiler makes
  of it: read the end of the function in the listing. Before it lands,
  the C wants holding against the machine's own arithmetic on the
  computer that builds, where the same source compiles, and `__U4D` for
  a divisor of more than 16 bits, which no scenario runs.
- The data and the stack of the part that stays: as its agent left it
  below ("How the data that the assembly still lays out could go to C"),
  one structure in `port/resident.c` in a group of its own that begins
  at the PSP. Two things to add. The constants of that part, its five
  texts, `Command` and the tail `OvlPrm`, are better arrays declared
  `CODE` in `port/resident.c` than members: the test `data` finds an
  object by its name, and a member has none. And the group of the
  constants, the compiler's, need not begin at the PSP any more once no
  assembly counts from there: a word in a table is held by what it
  names, not by its number. `tools/mkexe.c` lets the object file of
  `port/resident.c` have data then, and finds the seven words by a name
  in the map; `harness/driver.py` and `harness/ported.py` find the stack
  by two names there.
- The layout of the main data: the `STRUC` goes with the last line of
  assembly that names a field, which is a stub of 3 or the room of the
  part that stays, sized by `CmnSave` and `WCBsave`. `struct MainDat` of
  `port/vc.h` is then the only one, and `md` a type at offset 0 of DS.
  Its layout stays the original's: boxes and dialogs hold places in it
  as numbers, `MD_AT`, and the test `data` holds those against the
  original's.
- What stands between the stack and the code: the constants. A text
  that goes to C leaves `CONST` for `CONST2`, which `VC.ASM` declares in
  front of the code, and the assembly's tables, `_DATA`, are of the same
  class and stand there too: 8,700 bytes where 4,096 are needed.
  `tools/mkexe.c` stops the build when code comes within 4K of the
  stack. When `VC.ASM` is gone the order is the linker's to be told, by
  `order` in its command or by the first object file.
- `tools/mkexe.c` and `CMakeLists.txt` when there is nothing to
  assemble: the last step, by one agent. No `VC.OBJ`, no listing of the
  port; the order of the segments and the two groups by the linker's
  command; the relocations of the second group in `tools/mkexe.c` and
  `Reloc`; `harness/driver.py` with the original's listing for `AutoTim`
  and `Clock`; `tools/routines.py`, `tools/flagscan.py` and what
  `CLAUDE.md` and the README say of stubs and of the listing taken out.
  JWasm stays: it builds the original and the driver of the tests.

What must not be done by two at once: one assembly file; `VC.ASM` beyond
its texts and boxes; the loop of `port/main.c`; `port/resident.c` with
`tools/mkexe.c`; `CMakeLists.txt` beyond a name in a list.

## Known and not dealt with

- The number of bytes that the editor has free is not compared with the
  original's (see `harness/deviations.py`). What works it out is C now,
  in `T00_20` of `port/edit.c`, and the number still wants a check of
  its own: that it falls by one for every character typed, at least.
  It has been wrong once without a test saying so, when a stub that it
  called handed back another CX. Held against the original by hand when
  the editor's keys were translated, over some thirty steps of
  `editor-keys`, `editor-hex-keys` and `editor-buffer`, in text and in
  hex: 27,104 less in each, 331,648 for an empty text, with the port
  at 90,560 bytes.
- Saving in the editor to a disk that cannot be written, the floppy A:
  of the bench, with the box of DOS's critical error, is in no scenario:
  it went beyond the stack of 256 bytes that the port had when the
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
- `tools/flagscan.py` follows flags until an instruction sets them all,
  and so reports a jump that reads a flag which an instruction in between
  has set: `DEC AX` and then `JE`, `STC` and then `JNC`. It should follow
  each flag by itself.

- The screen blanker is C, `Blank` of `port/blank.c`, and what it draws
  is held against the original on no screen. Its stars come out where
  the program's random numbers say, and nothing sets the two words that
  those are made of, `Rand1` and `Rand2`, when the program starts: the
  sky follows from what the memory held where the main data is. On the
  bench that is 0 for the original, and something else for the port,
  whose main data is elsewhere, another thing with every build. So a
  step may not end in the blanker. The scenario `blank` brings it up,
  with the mouse in the top right corner, and ends it in the same step,
  by a key, by the mouse moving away, by Shift and by a button: what the
  goldens hold is that the screen and the cursor come back and that the
  key is lost. If the program set the two words when it starts, a step
  could end in the sky and a golden could hold it; that would be other
  behaviour than the original's, and is not decided.
  The C was held against the original by hand instead, once, outside
  the tests: with the port's two words set, in a build made for it, to
  what the original's are when the blanker comes up first, and with a
  step that lets the program go round without ending, both were taken
  through a sky of some hundreds of turns, and every screen of either
  was one that a model of the routine, written from the assembly, has
  after some turn. See what the batch lost time to, below.
- Of `LeaveTm`, six instructions of the original have run in no
  scenario and are listed as out of reach: they are for DESQview.
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

- The Info panel says how much memory is free: what the part of the
  program that stays during a command leaves, or with Memory allocation
  switched off, "F9 o m", what is left with all of the program loaded.
  Both follow the size of the port, and the number is left out of the
  comparison in every scenario (see `harness/deviations.py`). So nothing
  checks the number itself: that the port has less by what it is larger,
  at least. The same goes for what `mem` says in the scenario `command`,
  416K free under the original and some 7K less under the port, whose
  part that stays is 7,008 bytes longer, 3,840 of them its stack. With
  those numbers left out,
  no scenario shows any more that the program gives up its memory while
  a command runs: a check of its own would hold the port's figure
  against the original's less what the link map says the part has grown
  by.
- Every disk of the bench has a volume label, so no scenario has the
  program look for one and find none. The Info does say "has no label"
  in `reread`, where it does not look: a directory named DIRINFO stops
  it before. The program asks whether to delete a label and does not
  get it off under FreeDOS: a disk without one would have to be made for
  the bench.
- `tools/routines.py` lists procedures only. Code that stands between
  two is not in its list; `harness/reach.py` knows it by its first
  label.
- What reading a directory does when a drive is not ready, when a list
  has more files than there is room for, or when a drive has no current
  directory to fall back on, runs in no scenario: the bench has no such
  drive, and a step makes one file. The same goes for the label of a
  drive of a network. They are listed as out of reach in
  `harness/ported.py` and rest on reading.
- `ReDir` reads byte 23 of the main data, a colour of the first palette,
  where it means the fourth character of a panel's path, to decide
  whether ".." gets the time and the attributes that DOS has for it. It
  is ported like that. The colour is never 0, so it always does.
- `GetVol` answers for a label that it has found with the carry flag
  set and in AX what DOS left in AH and the eleventh character of the
  label; `RdInfo` takes 53h there for a drive that is not ready. A
  label of eleven characters that ends in S would do that, if DOS
  leaves 0 in AH. Ported like that; not tried.
- `VCVIEW.INC` is C but for its boxes and texts and the stubs of the
  commands, which the loop of the keys and the menus jump to. Every
  instruction of the commands that start the viewer or the editor has
  run in a scenario, those that tell a program run as the viewer of the
  mouse too, since the bench has one; what they tell it shows on no
  screen.
- What the commands that view and edit do in the original and the port
  does after them; whether to mend any of it is not decided:
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
    15: `Run2_S` of `port/view_t.c` is written 80 bytes long, with the
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
- `PrevLin` has three instructions for a place that is not the start of
  a line and one for a line of the table that is not its own. No caller
  hands it either, so they ran in no scenario and are translated by
  reading.
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
- What the viewer does and the port does after it, found while its
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
- Of `__U4D` in `port/asm/VC.ASM`, which divides 32-bit numbers for the
  compiled code, no scenario runs the half for a divisor of more than 16
  bits: nothing divides by one yet. It was held against a model of its
  steps, not run.

- Of the help, one instruction for the mouse is listed as out of reach
  in `harness/ported.py`: a click below the last line of an index that
  is shorter than the window. The file has one index, of 62 lines. The
  rest runs in `mouse-help`.
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
  into it has pages that are refused for their length.

- Of the file search's command, `F67_00`, three instructions are listed
  as out of reach: two for a drive that the CD-ROM extensions answer
  for, and Go to where the directory cannot be changed to.
- Go to asks for the disk of the wrong drive. `F67_00` hands `ChgDisk`
  the address of the line, which is in the memory of the list, and
  `ChgDisk` reads at that offset in the main data: two bytes of the
  first palette, since a line begins within 16 bytes of its segment. So
  the current drive is asked about, unless the second byte is 3Ah, a
  colon, and then the "drive" of the first. Ported like that. Go to
  also copies 69 bytes of the line into the path of the panel, which
  has 68; the last goes into the place of the cursor, which is set
  next.
- "N files found." is put together from two texts, each copied with
  the 15 bytes that begin with it: behind the full stop TmpBuf gets
  seven bytes of what follows `FnFl7_S` in the program, the title of
  an error's box without its zero byte. Nothing reads them. The two
  are constants of `port/find_t.c`, each 15 bytes long with what the
  original has there, so the copy no longer goes by what happens to
  stand behind them.
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
- A name to search for that has a full stop gets four bytes behind it
  that follow `\*.*` in the assembly: a zero and "VC.", the start of
  the name of the main extension file. Nothing reads the three; the C
  copies them from the assembly, `Mask_S`, as long as that has them.
- `FindFil` calls itself, through `DirFunc`, once for every level of
  the directories it searches. The original took 18 bytes of its stack
  of 256 for a level, by reading; what the port takes of its 4,096 has
  not been measured, and how deep a tree either can search has not been
  tried. The scenarios search three levels. DOS's critical error can come up with
  all of that on the stack: `FindFil` calls DOS for the files, and to
  open and read them.
- `VCMENU.INC` is C but for what the assembly needs of it: the tables by
  which the stub of the menu bar jumps to the commands, and the stubs of
  the commands, 28 of them with `M17_00`, which hands Drive... to the
  box of the drives. Its boxes, dialogs and texts are constants of
  `port/commands_t.c`. The stubs
  go when the loop that jumps to
  them is C, and with it `CmdM01`, `CmdM03` and `CmdM06` of
  `VCFUNC.INC`, by which the keys of the panels go on in three of the
  commands: `port/keys.c` can call the functions then.
- Of the commands of the menus, two instructions are listed as out of
  reach: one for a display that is neither EGA nor VGA, and one for a
  kind of panel that nothing asks for. The one of EGA lines that keeps
  the pointer of the mouse on the screen when it is below the 25th row
  and 50 lines become 25 runs at the end of `mouse-help`.
- What the commands of the menus do in the original and the port does
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
    " User Menu " the port has other bytes than the original from the
    42nd on: the texts of the box of the drives stood there, and are in
    `port/drive.c` now. Nothing reads them.
  - The boxes of Save setup and of Compare directories are shown with
    DS on the current panel, where a box looks for the texts it is to
    put into its lines. They have none.
  - The configuration loads CL with whether the menu bar is always
    visible, before its dialog, and nothing reads it.
- Drive... of a panel's menu, "F9 l d", showed the box for a panel that
  was none in the port, from the batch of the menu bar until that of
  the commands: the bar's table jumped to the stub of `F62_01` with the
  registers of the original, and the stub takes others. The box said
  "right" for the left panel and wrote to a segment made of the item's
  number. No scenario chose the item; `menu-panel` does now.
- Of the menu bar, four instructions run in no scenario and are listed
  as out of reach in `harness/ported.py`: EGA lines dark, for a display
  that is neither EGA nor VGA, and an open menu whose items are dark
  down to the last. What it does for the mouse runs in `mouse-bars` and
  `mouse-menu`, which also has Alt by itself in an open menu and an item
  of the Files menu chosen with Ctrl and with Shift held down: with the
  keys pressed inside the machine, each is still down when the program
  asks the BIOS for it. `mouse-middle` has the middle button on an item
  of the Files menu, which is the item with Ctrl, and `mouse-drag-menu`
  a button that is held and moved along the bar and down a menu.
- With the mouse the menu bar can come to have no menu at all, the
  number 0, while it goes on as if it had one: a button pressed on the
  bar next to the names and moved onto one leaves 0 for a round, and
  the page of the help is then read by a word in front of the table of
  the menus; a button let go at that moment, or the mouse leaving the
  bar between two looks at it after a menu was closed that way, has the
  keys work on menu 0, and a menu opened then is taken from 2,040 bytes
  behind the table. Ported like that as far as it can be: the C has the
  tables in its own place, so what it reads there is something else.
  `mouse-drag-menu` has the bar left like that, "Click@70,0*5>25,0*1",
  and Right there, which goes to the first menu in the original and in
  the port. It does not press F1: the page of the help is a byte that
  the original takes from its data and the port from its code, the
  word behind the table of the pages read as a place in that table.
  Tried by hand, both showed the same box, "There was error reading
  data from the help file", for a page that is none; another build of
  the port may have another byte there. Enter, Down and Left there
  were not tried, and F1 stays out of the scenario: they open a menu
  from behind the table, or show a page by that byte, so the original
  goes by stray memory there and the port cannot be held to it.
- In an open menu the key of an item that is dark, Ctrl+a in the Files
  menu of a tree, is not dropped: the number of the item is looked for
  among the letters of the items, where it is not. And the table of
  those keys has F1 for the Help item of the Files menu, which `Input`
  never hands on: it shows the help of the item under the cursor
  itself. Both ported as they are.
- Of the dialogs, all that they do for the mouse runs: in
  `mouse-dialog`, `mouse-fields`, `mouse-line` and `mouse-filter`, the
  middle button, which leaves a dialog as Esc does, in `mouse-middle`,
  and a button that is moved while it is down in `mouse-drag-dialog`.
  Two buttons at once leave a dialog too, by the same instruction; no
  step presses two.
- A dialog trusts its tables. One without a field takes no key and does
  not end, a box without a line has 65536, and a line to type into of
  length 0 is shown as one of 65536. Ported like that; the program has
  none of them.
- A click of the mouse into a line that is typed into puts the cursor
  where the mouse is, and no further than the text goes; `InpStr` looks
  for the end of the text in as many bytes as the line has, so in a
  line that is full it finds none and stops one short of the end. That
  shows nowhere: a click is in the line only on one of its characters,
  and the last of them is one short of the end. `mouse-filter` has such
  a line.
- The routine that a dialog has for Enter and F10 got DX as `Dialog`
  happened to have it, the size of the box or of its buttons, and two of
  them, `M30_60` and `F38_10`, showed a box of their own with that as
  the text for ^I. From the C they get the offset of the dialog there,
  and those two, which are C as well, name no text. None of the lines of
  their boxes has ^I, so nothing read it.
- What the box in which a directory is chosen from the tree does for
  the mouse, and the main loop for the mouse on a tree panel, runs in
  `mouse-tree`. Listed as out of reach are 17 instructions
  of `ReadTre`: for a drive that is not ready, at first or while its
  directories are read, for a tree of 767 entries, which has no room
  for another, and for hidden directories left out, which only a setup
  file made elsewhere asks for: `HidDirs` is in no menu.
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
- `ReadTre` takes a tree file for good if its two sums are right and its
  length is that of its number of directories, worked out in 16 bits: a
  file with the entries of ten directories that says 4106 passes, and
  the tree is then drawn from what lies behind `TreeBuf`. A file that
  says 0 or 4096 and has no entries has its sum taken over 65536 bytes,
  all of the main data's segment. Ported like that; it takes a file made
  by hand.
- `VCCOPY.INC` is C but for the stubs: of the commands and of what
  their dialogs call for the tree. Its boxes and texts are constants of
  `port/copy_t.c`. `C00_00`, which `FncSel` and
  `FncMas` call, has none any more: they call the function.
- What the commands of Copy and Rename do in the original and the port
  does after them; whether to mend any of it is not decided:
  - When a directory of a tree panel has been renamed, the current
    directory of DOS is taken along if its path begins with that of the
    directory, whether a name ends there or not: with DOCS renamed to X
    while DOCS2 is the current directory, the program goes to X2 if
    there is one, and stays where it is if there is not. In `copy-tree`.
  - Shift-F6, and F6 with the panels off or the Info active, offer the
    name of the file under the cursor or the last target according to
    BX and DX, which hold the numbers of selected files only where F5
    and F6 have counted them: there they are what the key left. The
    stub hands them on as they are. In `copy-ask`, where either way
    shows the same.
- The map of the memory, Alt+F5, names the program by its file: VC.COM
  in the original and VC.EXE in the port. `harness/deviations.py` has
  the golden expect VC.EXE there, as it does where the program cannot
  be loaded again. That is no figure that follows the size of the
  program, and no scenario can show the map without that line: the
  batch of the map waited, committed and not pushed, until the user
  had said that the deviation may stand in this second place.
- Of the map, the sizes and the places that follow the size of the
  program are left out of the comparison: the size of the program, where
  a program begins that it has started and that stays, and of every line
  of free memory where it begins, its blocks and its size. So nothing
  checks that what the program gives up while a command runs is listed
  as free, but the kind of the line.
- Of the map, 19 instructions of 913 have run in no scenario and are
  translated by reading: a DOS before version 4 or with a version of
  two digits, 15; and a block of an unknown owner, 4. A map of more
  lines than its window has, and a second program that stays in the
  memory, are in `stay`, which makes such programs in the editor, in
  hex, and runs one nineteen times. All that the map does for the
  mouse runs: in `mouse-memory` what a button does that goes down and
  comes up in one place, in `mouse-middle` the middle button, which
  leaves the map, and in `mouse-drag-lists` a button that is moved
  while it is down, from a line to above the lines or below them, where
  the cursor goes on, and from beside the lines onto the buttons.
- What the map does in the original and the port does after it:
  - A line with exactly eight vectors shows seven and three full stops,
    as one with more does: `MemItem` counts the vectors down in AL and
    tests all of AX, whose other half holds the colour of the line. By
    reading: no line of the bench has eight.
  - Grey + and Grey - have all lines drawn again by making the first
    line shown out to be another, two further on, and the cursor is
    taken off at the top of the window with the colour and the mark of
    the line two below the first. All lines are written afterwards, so
    nothing of that shows.
  - A vector is counted for the line whose blocks it points into, up to
    255 of them in a byte: the 256th would make it 0 and be noted as the
    count itself. No line has so many.
- A new name for a directory of the tree that makes its path longer
  than 64 characters is reported as "Can't create the directory", as a
  name with a path in it is. The first rests on reading: it takes a
  tree some ten levels deep.
- Copying and moving in C rest on reading where DOS would have to
  refuse what it has just allowed, or a drive not be ready: 32
  instructions, listed in `harness/ported.py`. So does going on with
  another disk when one is full, where the directories of the target
  are made on the new disk, 31 instructions: the same disk is still
  there on the bench.
- What the original does and the port does after it, found while
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
    port left a file of the same size and other contents, because where
    the memory ends depends on how much of it the program takes. It
    copies to new files now, so no scenario has it.
  - Of a device that gives its bytes in several calls of DOS to a
    piece, the room left in the memory is counted down by the last
    call only. By reading: the bench has no such device.
- The functions of `port/copy.c` are on the stack when DOS brings up
  its box for a critical error during a copy, a copy to the floppy A:
  for one. No scenario has that.
- `copy-full` fills C: by copies, 32 MB, in half a minute. So a full
  disk can be made on the bench after all, and what the editor does when
  it saves to one, listed as out of reach for `T80_00`, could be shown.
- The `port` test has five scenarios more for copying, 304 steps, which
  take two and a half minutes of machine time between them: `copy-full`
  alone 33 seconds on an idle machine and a minute next to the tests of
  other batches, the longest of all. It moves some 80 MB. Its first
  form did that in steps of up to 25 MB, and a step ran into the 20
  seconds that a step may take, twice, with other `port` tests and
  runs of goldens under way; no step moves more than 6 MB now. If the
  test is to be faster, this is the scenario to drop: 74 instructions
  of `C00_00` would go to the list of what is out of reach.
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
- Of the editor, five instructions have run in no scenario and are
  translated by reading, listed in `harness/ported.py`: two for a file
  that DOS will not read, and three where the window would move up by
  fewer lines than it does, or begin above line 0.
- `editor-big` has a file of 381,338 bytes that is too large for the
  editor, of the original and of the port alike. One that fits, with a
  size in the same 64K as the room, has no place in a scenario: the
  port's room was 27,104 bytes less than the original's then and
  shrinks with every batch. So of the two instructions that compare the low words,
  the scenarios take the jump one way only.
- `T54_00`, which goes to the line before, looks backwards for where a
  line begins and then goes forwards again with `T53_00` until it is back
  where it came from, without asking whether `T53_00` found a line. If
  none were found before that place it would not end. That takes a text
  whose lines do not begin where going forwards has them, which hex mode
  might make; not tried. Ported like that.
- Three instructions of `T53_00` are listed as out of reach: in hex, for
  16 bytes that go beyond the end of their segment. The address it gets
  there has an offset below 16, or is a window of lines further on.
- A search can be given a key only where it begins: with the Enter
  that starts it, written as one name, or behind the help that such a
  key brings up. So `find` no longer has a key that stops a search
  between two parts of a file it reads, and the five instructions of
  `FindFil` that close the file then, `FnFl48`, are listed as out of
  reach. To have that again, the driver of the tests would have to hand
  a key over when the search comes to a named place, in the original and
  in the port alike: it would have to be told where that is in each, as
  it is told where `LeaveTm` is.
- The bench has a mouse now, and scenarios use it: `mouse-panel`,
  `mouse-bars` and `mouse-dialog`, with which 349 of the instructions
  that `harness/ported.py` listed as out of reach for the mouse came off
  the lists, and the scenarios that followed with nearly all the
  others. What is still listed, by the file of the port that has it:
  - `help.c`, 1: a click below the last line of an index shorter than
    the window; the file has none.
  - `menu.c`, none: `mouse-menu` has the rest of the menu bar.
  - `view.c`, `edit.c`, `find.c` and `treebox.c`, none: `mouse-view`,
    `mouse-edit`, `mouse-find` and `mouse-tree`.
  - `dialog.c`, none: `mouse-fields`, `mouse-line` and `mouse-filter`
    have the fields of a dialog.
  - `usermenu.c`, `cmdline.c` and `drive.c`, none: `mouse-user-menu`,
    `mouse-history` and `mouse-drive` have the user menu, the history
    and the box of the drives.
  - `memory.c`, none: `mouse-memory` has the map of the memory.
  - In all of these `mouse-middle` has the middle button, and
    `mouse-drag-dialog`, `mouse-drag-menu` and `mouse-drag-lists` a
    button that is moved while it is down.
  - `input.c`, none: `mouse-keys` has the right button on the key bar, a
    key of the bar that has no name, and the corners of the screen.
  - `keys.c`, none: `mouse-select` has the right button held below a
    list and above it, which selects the files that come under it.
  A second click is two clicks in one step; a button held is a click
  with a count behind it, "Click@3,5*200"; one that goes down in one
  place and comes up in another has both, "Click@3,5>3,2"; the middle
  button is "MiddleClick@3,5"; and a row may be any of 50. See the top
  of `harness/scenarios.py`.
- `cmake --build build -j4` failed once in a new checkout, "The object is
  zero length" for one C file. Building without `-j` works.
- A run of the harness that is killed outright leaves its QEMU running,
  at full load.

- `^R` in a tree panel marks the tree's file as out of date and takes
  what `DelTr0` leaves in AL for the kind of the panel: 1, the one byte
  that DOS has written. With the file read-only the byte is not
  written, the routine beeps and leaves DOS's error there, 5: the panel
  is then one of a kind that there is not. The port hands back the same
  5. No scenario does it: what the C code of the panels makes of a
  fourth kind rests on reading, and whether it is what the original
  makes of it has not been tried. `delete` has `^R` with the file
  writable.
- A mask with a drive and no directory, "c:*.zzz" for Shift-F8, never
  says "Can't find files": `F38_70` calls DOS for an FCB with the carry
  flag set by a comparison before it, DOS leaves the flag, and `Intr21`
  takes it for an error. Ported like that, through `Intr21f`; `delete`
  has it. It rests on FreeDOS leaving the flag: of another DOS that
  clears it the original would say "Can't find files", and so would the
  port.
- When DOS deletes the files of a directory at one call, it stops at
  the first that is read-only; the others are deleted one by one
  afterwards. Cancel in the box about a directory that is not empty
  leaves that directory and goes on with the next one selected, where
  Cancel for a read-only file stops the command. `M30_50` does not ask
  whether DOS could read or set the date and the time of a file. All
  ported as they are.
- `VCF1.INC` is C but for the stubs; its boxes and texts are constants
  of `port/dirfunc_t.c`. `FncSel`, which goes through the selected files
  of a panel, and `FncExt`, where a command ends that may have changed
  the disk, are functions of `port/dirfunc.c`, and their stubs are what
  `F1Next` and the table of Copy and Rename jump to. They were code in
  the program's data, behind `DirFunc`; `harness/coverage.py` names such
  code by its label now.
- A command that is C returns where it goes on, and its stub jumps
  there: `F1Next` in `VCF1.INC`, with the `TO_` of `port/vc.h`. The
  commands of the other files can use the same; one that goes on
  somewhere else needs a number of its own there.
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
- The keys of the panels, `VCFUNC.INC`, are C: what is left there is
  the stubs, the table by which they jump on, `CmdNx_T`, and the scan
  codes of the speed search, `AltSr_T`, which the main loop reads there;
  the boxes and texts are constants of `port/keys_t.c`. Enter,
  `F21_00`, saves what the program needs to start over,
  gives the main data back to DOS and returns `NEXT_EXEC` or
  `NEXT_INIT11` (`F21_31` of `port/enter.c`), and the stub goes on to
  the part of the program that stays in the memory, at `Exec`, or to
  `Init11`: `CmdExe` and `CmdIni`. Those two jumps and that part are
  what is left of running a command, with `VC.ASM`. The program also
  comes through `F21_31` when it has started: the loop goes there from
  `Init40`. The loop that
  takes a key is C, `MainLoop` of `port/main.c`, and its labels in
  `VC.ASM` are stubs that enter it: the commands that are C still
  return to it through a table of those labels, and it goes to a
  command through a table of labels too, `Main_T`, because the commands
  are labels of the assembly; which command a key has is a `switch`,
  `Func12`. When the commands are functions, the loop calls them, and
  the table and the structure of registers go.
- `harness/reach.py` and `harness/ported.py` name code from a label to
  the next procedure, so `F22_00`, the keys that change the directory,
  names all that stands up to the procedure `F42_10`, with `F80_00`
  and what follows it, which an earlier batch lists under that label:
  the instructions from there on are counted in both batches.
- With "Quick execute commands" switched on, "F9 o o", a command is run
  by `INT 2Eh` of the shell, and the shell of the bench, FreeCOM, runs
  nothing then: the command is echoed and that is all. So no scenario
  shows what the part that stays does with `DosChg`, the drive that
  Enter tells it to make current first when the program was started
  on another: it reads that for `INT 2Eh` only. The C sets it by
  reading.
- What Enter and the keys next to it do in the original and the port
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
- Of the keys of the panels, what Tab does when DOS will not go to the
  directory of the other panel, or has not told the current one, has
  run in no scenario and is translated by reading, 18 instructions of
  `F20_02`: whatever removes or renames a directory has both panels
  read again, and the drives of the bench are ready. What they do for
  the mouse runs in `mouse-panel`, `mouse-list` and `mouse-select`.
- What the keys of the panels do in the original, and in the port after
  it; whether to mend any of it is not decided:
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
- What `F38_70` does when DOS says that a drive is not ready, or will
  not go back to the directory it came from, what `M30_50` does
  when DOS refuses the attributes of a file that it has just listed,
  `M30_60` where the year is written first, and `DirFunc` for a path
  of 68 characters or more ran in no scenario and rest on reading.
- A directory deleted with what is below it takes about 90 bytes of
  the stack for every level in the port, where the assembly took 22.
  The 32 levels that a path of DOS can have fit the port's stack; they
  did not fit the original's 256 bytes.

- Of the user menu, the history and the box of the drives, these have
  run in no scenario and are listed as out of reach in
  `harness/ported.py`: a box of the drives too wide for its title, 15
  letters or more; a drive that is not ready; and a list file that DOS
  will not give attributes to or write. What they do for the mouse
  runs, in `mouse-user-menu`, `mouse-history` and `mouse-drive`, with
  the middle button, which leaves each of the three, in `mouse-middle`,
  and a button that is moved while it is down in `mouse-drag-dialog`
  and `mouse-drag-lists`. A click on F1 or F10 of the key bar is the help or
  leaves the user menu, where the key itself is a line's if the menu
  has one for it: `Input0` says which of the two a key was.
- What the original does and the port does after it, found while the
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
    The port clears it in that colour too; the word on the stack it does
    not leave. No scenario has commands that long.
  - The speed search hands `Input` the column of its cursor for the key
    bar to show. The key bar is off while it runs.
- What a command leaves in DX and BP reaches the next command through
  the main loop, and a command that is C leaves other values there than
  the assembly did. One reader is known, Shift-F6, which goes by BX and
  DX together; BX is an address that `Func05` loads, not 0, whenever a
  command has run since the program started or came back from DOS, so
  DX decides nothing then. The commands of the table of keys were
  scanned for another, up to their first call or jump, and none was
  found; what the routines they call read was not followed. After the
  speed search, which goes on without `Func05`, the stub hands on BX,
  DX, SI and CX as the assembly left them.
- What the program does by the hundredths of the time of day is the
  same every time a scenario is run with the same program, but need not
  be the same in the port, which gets to a step some instructions later
  than the original, and so some hundredths. `DoublMo` adds 0.4 seconds
  to the time of day of a click, and three instructions of it, `INC
  DH`, `CMP DH,60` and `JB DblMo2` at 9506h, run when the hundredths
  are 60 or more; `SUB DL,100` in `Func07` is such a place too. Which
  way they went in the original is in the golden; a scenario that has
  both ways is what tests them. The double click of `mouse-list`, two
  steps within those 0.4 seconds, has room: the machine's clock is
  counted in what the machine executes, and little of that is between
  two clicks, in one step or in two. The time a second click has is 0.4
  seconds or up to 1.4, by the hundredths of the first click (see
  `DoublMo` below). `mouse-user-menu`
  has one that is late, after "Wait*30", 30 ticks of the timer.
- The main loop hands the next command the registers as the assembly
  left them, BX, CX, DX, SI, DI and BP, but for two things. What
  `DoublMo` leaves in AX and CX after a click on an entry is not handed
  on: its entry for C answers in AX and does not return CX. Where the
  program goes on then, `F00_60`, `Func04` and `F21_10`, sets both
  before reading them; `mouse-list` goes to the first and the last of
  these so, and comes out as the original. And what the loop hands to the BIOS and to DOS in
  registers that the call does not read, BX to DX of the call for the
  video mode, BX and CX of those that set a vector, is 0 where the
  assembly had what was left there.
- What of `port/asm/VC.ASM` is still assembly, and what each piece will
  want. The port ends with no assembly, and the order of preference is
  the user's: portable C, then C tied to the compiler or the machine,
  then intrinsics, then inline assembly where nothing else works.
  - The stubs of the start-up, `Init00` and `Init11`: the start-up is C,
    `port/start.c`, and what is left of it in the assembly is what the
    part that stays calls, far, the segment of that part and the block
    of the code handed to the C, DS put on the main data that
    `Init11_c` has asked DOS for, and the far jump to `AlocEr` when
    there is none to have. `Stay` of `port/resident.c` could call the
    three functions itself, but for DS: `IniData_c` and the main loop
    behind it want it on the main data, which C cannot set; that is
    the instruction of inline assembly that `FatErr` wants too. What
    `port/start.c` names of the part that stays is in one list at its
    top: `ComnIni`, `WCB1sav`, `WCB2sav`, `Counter`, `Vect21`,
    `Vect22`, `Vect27`, `RelTab`, the functions `Exit01`, `New21` and
    `New27`, by their offsets, and `SetBrk`, which it calls through a
    pointer to a far function.
  - The part that stays in the memory is C, `port/resident.c`: the
    entry of the program, `Alloc0`, which DOS enters; `Reloc`, which
    that part applies to the other whenever it has loaded it; the
    outermost loop of the program, `Stay`, which calls the part that
    goes, far, at `Init00` and after every command at `Init11`; `Exec`,
    which runs the commands and loads the program again; `Exit00` to
    `Exit05`; `New21` and `New27` with what keeps the list of the
    programs that stay in the memory; `SetBrk`, `NotFind`, `OutStr`,
    `GetStat` and `SetStat`. What is left of its assembly, in `RHEAD`
    of `VC.ASM`, is its data, the block of numbers for `tools/mkexe.c`
    at 100h, the name and the command tail, the texts, with `RDATA`
    behind the compiled code, and two kinds of stub:
    - `Exec`, `AlocEr` and `Exit00`, three instructions each. The part
      that goes still comes back by a far jump to one of them, from
      `CmdExe` in `VCFUNC.INC`, from the stub of `Init11` and from
      `Exit`, with the
      stack as it was when `Stay` called it: each loads a number and
      returns far, into `Stay`. They go when those three places return
      by themselves, which they can as soon as they are of a function
      that `Stay` calls: `PUSH SS`, `POP DS`, the number in AX, `RETF`.
    - `Release`, a stub: for `Release_a` of `VCMEM.INC`, which the map
      of the memory calls through `CallPSP` with the record in DS.
      `SetBrk` and `Exit01` have none: `port/start.c` names the C
      functions themselves, by their offsets, as it named the labels,
      and calls `SetBrk` through a pointer to a far function.
    - `CallPSP` itself, in `VCSUB1.INC`, is of the part that goes.
  - `FatErr`, what DOS calls for a critical error, is C, an interrupt
    function in `port/main.c`. Its texts and its two boxes are still
    the assembly's, where `Dialog` reads them, and so is `SecDat`, the
    word that holds the segment of the main data: `port/start.c`
    writes it, the stub of `Intr21` reads it, and `FatErr` loads DS
    from it, by the one instruction of inline assembly there. When no
    assembly is left, that word wants a home that an interrupt function
    can read with nothing but its code segment or the compiler's group
    to go by.
- Ported as they are, in `port/start.c`: see the comments of `Init00`
  and `IniData` there. Three of them reach further:
  - The original takes the two panels of a program that starts without
    a setup file, or with one that is not correct, from 88 bytes of
    which it has 8: the other 80 are what follows them, the first
    twenty entries of its table of the keys, each a key and the address
    of its command. They are in the mask of both panels behind its zero
    byte, and behind their paths, in the memory only, until the main
    loop first holds the setup against the one that was saved: `MkIni1`
    fills both fields with zeros behind their texts, in the panel, before
    the first key is taken, and the setup file is made of that. The
    port has no such table any more, the keys are a `switch`, and
    `Init00` of `port/start.c` puts zeros where the original has those
    80 bytes. Nothing was found that reads them in between: the mask
    and the path are read as texts, by `SetMask`, `PutPath`, `Init50`
    and `ReDir`, and copied whole only as such. The file that a copy
    started on the defaults saves is the original's byte for byte:
    `start-setup` ends with it on its disk.
  - The registers that the start-up leaves go to the first command
    through the main loop, which sets all of them but BP before a
    command reads them. BP was the offset of the font that the BIOS
    answers with when it is asked for the rows of the screen; now it is
    what the part that stays, or the command that ended in `CmdIni`,
    left there. No command is known to read BP.
  - The test for snow counts readings of the display's status, seven
    running with the bit set, and compiled code reads more slowly than
    the three instructions of the assembly did. It could not be
    measured: QEMU's VGA turns the bit over at every reading, so the
    test never succeeds on the bench, and its `MOV AH,0` is listed as
    out of reach. On a CGA the bit stays set for the whole time that
    the beam returns to the top, more than a millisecond, which is long
    for seven readings either way.
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
- What the program's watch on DOS does in the original, and in the
  port after it (`port/resident.c`); whether to mend any of it is not
  decided:
  - The call by which another program asks for the program's data,
    INT 21h with AH 2Bh, CX 'VC' and DX 'OM', returns with the flags as
    the handler has them, not as the caller had them: the carry flag
    clear, which is the answer, the other flags those of AL compared
    with 1, and interrupts off, as they are when an interrupt begins.
    They stay off in the caller until it lets them in again. In `stay`.
  - When the list of programs that stay is full, 18 of them, the next
    one ends instead of staying, and every interrupt is put back as it
    was when that program was run, whoever has changed it since. In
    `stay`.
  - If DOS will not make the block of a program that stays longer by
    the 82 paragraphs of its record, the program stays without one, and
    the map of the memory does not know it for a program that can be
    released. By reading: DOS has given the program all the memory
    there is, so it can.
  - A program is taken off the list when the block of its PSP is given
    back to DOS, by whomever. Leaving the program releases "the last
    of the list" as many times as the list is long, and relies on
    that: a record that is not whole any more is released that many
    times, which does nothing.
  - The twelve bytes of a program's name are taken from where the name
    begins in what DOS was asked to run, whatever follows the name
    there.
- The entry of the program, in the original and in the port:
  - The program sets how DOS looks for memory to what it is when DOS
    starts, the lowest block that fits and no upper memory, whatever it
    was when the program was run. Ported as it is.
  - The original cannot always make its block of memory as long as the
    program needs it, its file being shorter than the program in the
    memory by the data of the part that stays, and then asks DOS for a
    block elsewhere for the part that goes: so it can be loaded into
    a block that holds its file and little more, the upper memory for
    one. The port's file is as long as the program in the memory, so
    DOS loads it where all of it fits or not at all, and that second
    way is not in the port: it could only be taken with DOS's chain of
    blocks destroyed, where it fails too. The port ends then as the
    original does when both ways fail, "Program too big to fit in
    memory".
  - The check sum by which the program knows its file again after a
    command leaves out the data of the part that stays, in the port:
    the stack is there and in use while the sum is taken.
  - A DOS before version 3 is left by its function 0 in the port,
    where the original returns to the INT 20h that a .COM file has at
    the bottom of its stack. Both end the program the old way.
- What running a command does in the original, and in the port after
  it (`port/resident.c`); whether to mend any of it is not decided:
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
- What the critical error does in the original, and in the port after
  it (`FatErr` in `port/main.c`):
  - The number of the error is all of DI, of which DOS defines the low
    byte. A number of 21 or more is "Unknown error.", and so is any
    whose high byte is not 0: a copy to COM2, which the machine has
    not, says so under FreeDOS. In `device`.
  - Where DOS does not allow another try, the box has Ok alone and the
    call fails; where it does, anything but Retry fails it, Esc too.
    The program never has DOS end it.
- Where the part that stays in the memory (`port/resident.c`) is not
  as the original. No scenario shows any of these; each is here with
  what it would take to make it as the original, for the user to
  decide:
  - The calls of DOS that the program deals with take more of the stack
    of whoever calls than the assembly did: some 50 bytes for a program
    run (AH 4Bh) where the assembly took 14, 60 for a block of memory
    given back (49h) where it took 22, and 120 for a program that stays
    (31h, and INT 27h) where it took 40, before what DOS takes for the
    calls made from there; and some 150 of the stack of the program
    that started this one, when DOS has ended it, where the assembly
    took 32. The 30 bytes of an interrupt function's saved registers
    and second return address are in each, the rest are the frames of
    the C behind it. A call that the program only passes on takes none,
    as in the original: the scenario `passed-on` shows that, and the
    flags that DOS is entered with. As the original: these handlers
    would have to be assembly again, or run on a stack of the
    program's own, which is assembly too and not what the original
    does.
  - Where the processor leaves the auxiliary flag undefined, the port
    has it clear in the flags that it enters DOS with: after the `XOR`
    of a list that is full, and after the `SHL` of INT 27h. The
    processor of the bench leaves it clear after `SHL` as well.
  - No scenario shows the flags where the list of programs is full, or
    where DOS will not make a program's block longer for its record.
    The first was tried by hand, with nineteen programs from the second
    copy of `passed-on`: a program that is ended for a full list, AH
    31h made 4Ch, came to the watcher with zero, parity, carry and
    interrupts let in, 0247h, under the original and under the port
    alike. It is not kept. The watcher is on the first copy's list, so
    that list is full a program sooner, and the first copy ends the
    eighteenth, of which the second has a record all the same; whether
    it still has it when the next one comes follows where the memory
    was taken, which follows the size of the program, and so the
    original's second copy was not full for INT 27h where the port's
    was. A watcher that runs the second copy itself, as its parent,
    would be on no list. The second rests on reading.
  - The second way of the entry is not there: the original, when DOS
    will not make its block of memory as long as the program needs,
    asks for a block elsewhere for the part that goes, so it can be
    loaded into a block that holds its file and little more. The
    port's file is as long as the program in the memory, so DOS loads
    it where all of it fits or not at all (see the entry, above). As
    the original: the file would have to be shorter than the program
    again, with the data of the part that stays laid over what
    follows it, which `tools/mkexe.c` could do by leaving that data
    out of the file; the entry would then run on a stack outside what
    it moves and set the program's own afterwards, four instructions
    of assembly (`CLI`, SS, SP, `STI`) and a jump as behind INT 2Eh,
    and the check sum and `harness/driver.py`, which knows what the
    stack holds when loaded, would follow the new layout.
- Where the size of the port becomes other behaviour, for the user to
  weigh. A second copy of the port that runs a program which stays in
  the memory may have no room to load itself again afterwards, and
  ends with "Program too big to fit in memory", where the original
  goes on. It follows from how large the port is in the memory and in
  its file, and from nothing else: the translation does there what the
  original does. A program that stays lies behind the COMMAND.COM that
  ran it, a gap of 71K when that has ended. The part that is loaded
  again is 111K in the port and 51K in the original: the original's
  fits the gap, the port's goes behind the program that stays. And it
  is loaded from the program's file, which wants its own length behind
  that once more, 130K in the port and 64K in the original. With
  another program staying in front of the copy, behind a gap of its
  own, the watcher of `passed-on`, that is more than the bench has
  left; without one in front the port has room, which was tried. The
  scenario switches Memory allocation off in the copy, which keeps all
  of it loaded. The port may use more memory than the original as long
  as it behaves the same: this is a place where it does not.
- How the data that the assembly still lays out could go to C, from
  having built the layout of the part that stays:
  - The data of the part that stays, `RDATA` and what `RHEAD` has, is
    best one structure in `port/resident.c`, with the stack its last
    member: `WCB2sav` has to stand right in front of `ComnIni` (the
    setup file is read across the two), `Counter` right in front of
    `PrgList`, `SavePtr` behind `IntTab`, and the compiler lays out
    separate variables in an order of its own. Compiled with `-nd=R`
    the file gets a group of its own for it, `R_GROUP`, and reads it
    through DS without loading a segment (tried). That group has to
    begin at the PSP, since SS, DS and the offsets that the rest of
    the program has for that part all count from there: the structure
    begins with 256 bytes for the PSP, and the group is linked first.
    Its code is then no longer of that group: `RTEXT` gets a segment
    of its own, and where the part that goes names a function of it,
    the relocation takes the PSP's segment and not `RunPar`.
    `tools/mkexe.c` can tell the two by the segment that the word
    names, as it tells the two parts by where a word lies, and `Reloc`
    gets a second count.
  - The block of numbers at 100h is then the first initialised member
    of that structure, and `tools/mkexe.c` finds it by its name in the
    linker's map, as it could find the end of the stack.
  - `Vect21`, where INT 21h went before the program took it, is read
    through CS by the entry of that interrupt, `New21` of
    `port/resident.c`, which passes a call on before it has saved a
    register: a word of the code's own segment is all that can be
    reached there with no register and no stack. Today the code and
    the data of the part that stays are one segment. If the code gets
    a segment of its own, that word has to go with it, a word of
    `RTEXT` that the start-up writes through a far pointer, as it
    writes it now.
  - The main data is no storage of the program's but a block that DOS
    gives it: `md` needs no label, only a type at offset 0 of DS,
    `(*(struct MainDat __near *)0)`.
  - `SecDat`, the word with the main data's segment, can be a variable
    of the compiled code's own group once the assembly is gone: an
    interrupt function has DS on that group when it begins, which is
    how `FatErr` reads the word now. `tools/mkexe.c` refuses static
    data in the part that goes, because nothing else there has DS on
    that group; that one word would be the exception, and the group
    is relocated like any segment of that part.
  - The texts behind the stack: when `CONST` is gone, a C file of
    constants only, linked first in the part that goes, keeps code
    off the page that the stack ends in.
- `CtrlC`, the routine that the program has for Ctrl-C of DOS and for
  Ctrl-Break of the BIOS, runs in no scenario: the BIOS of the bench
  makes no interrupt of Ctrl and ScrollLock, which was tried with a key
  added to `harness/driver.py` for it and taken out again. In the port
  it is an interrupt function without a body, which saves the registers
  and puts them back where the original has one instruction.
- Ported as they are, in `port/main.c`:
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
- `AltSr_T`, the scan codes that begin the speed search, is read by the
  main loop where it is, in `port/asm/VCFUNC.INC`, and the place behind
  the code found goes round in DI as in the assembly. `port/cmdline.c`
  has a copy of its own. When that file of the assembly goes, the table
  wants one home.
- The list file gets, behind its name at TmpBuf+100, the six bytes that
  follow `vc.lst` in the assembly, and the menu's name at TmpBuf the six
  that follow `VC.MNU`: both are copied from the assembly, `List_F` and
  `Menu_F`, as long as it has them. Nothing reads the six.

- `VCSUB1.INC`, `VCSUB2.INC` and `VCSUB3.INC`, the shared routines, hold
  no routine of the original any more but one, `CallPSP`. What is left
  in them, and goes when what needs it is C:
  - Stubs, three of them: the routine's name, the call of the C
    function, and the registers and flags that the assembly which still
    calls it goes on with. `VCSUB1.INC` has one: `Dialog`, which the
    routine for DOS's critical error calls. The eight that the code
    which starts the program called, `ChkMode`, `ScrOffs`, `MvzLin0`,
    `MvzLine`, `AddFile`, `CutPath`, `PathLin` and `Cursor`, went when
    that became C. `VCSUB2.INC` has
    none: it is the names of the menus and the title of the tree.
    `VCSUB3.INC` has two: `ReDir`, for the entry `ReDir_a` of the main
    loop, and `ReadTre`, which the stub of `ReDir` calls. That of
    `DiSi10`, whose address `DirSize` put into `FncAddr`, went when
    `FncAddr` came to hold a number. A stub that had lost its last caller went with this
    batch, 58 of them, behind the 38 that went with Enter, each with
    its line in the `EXTERN` list and its function in
    `harness/ported.py`. `tools/routines.py` counts calls, not jumps:
    `Cursor` has fewer callers there than it has. The assembler says
    "Symbol not defined" for a stub that is taken out too early.
  - Entries for C: `AsmSeg_a`, the assembly's code segment, where its
    boxes and texts are, and `DialSub_a`, which calls what a dialog
    names for Enter. Neither calls an interrupt. `FncCall_a`, which
    called what `FncAddr` names, is a function of `port/dirfunc.c`.
  - Texts and tables that the assembly reads, or C at its offset there:
    the names of the menus, `PtMnu_S`; `Tree_S`; `Mask_S` and `Ext_F`;
    the name of the tree's file and the two boxes shown while it is
    read and saved; `DrIn_F`; `Init_F`, the name of the setup file,
    which the code that starts the program reads too; and the dialog
    that asks for a diskette, `Fant_B`, with its box and texts.
  - `CallPSP`, 6 bytes: a far call, by way of the stack, of a routine in
    the part of the program that stays in the memory. It is no routine
    to translate: C code makes such a call through a far pointer, as
    `Init00` of `port/start.c` calls `SetBrk`. One place of the assembly
    uses it still: `Release_a` of `VCMEM.INC`, by which the map of the
    memory has a program released. It goes with that.
- No entry in the assembly calls an interrupt for C code any more. A
  drive made the current one is `SelDrv` of `port/system.c`; the FCB
  that deleting by a mask has DOS make is `Fcb29` of `port/dirfunc.c`;
  what the map of the memory asks of DOS is in `port/memory.c`,
  `Dos5802` to `Dos2523`. All go through `Int21d` of `port/dos.c`, as
  the assembly called DOS itself there, without `Intr21`. `LeaveTm`'s
  three interrupts are `Int28`, `Int2F` and `DvPause` there.
- The jump of the main loop to the command of a key is a `switch`,
  `Func12` of `port/main.c`, and `Case` is gone. The commands are still
  labels of the assembly, stubs of the C functions with one way of
  handing registers over for each file, so the function returns the
  number of a place and the stub of the loop jumps there by `Main_T` in
  `VC.ASM`, which has 79 places now. With Enter and the commands of the
  viewer and the editor in C too, the loop can call the functions: the
  table, the stubs of the commands and the structure of registers go
  then.
- What the routines of this batch do in the original and the port does
  after them; whether to mend any of it is not decided:
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
- What the C of this batch hands DOS and the BIOS where the assembly
  had whatever a register held, none of which they read: the BIOS gets
  AL 0 where `Blank` asks for the page, and the multiplexer BX and CX 0
  with the call 1680h. DOS gets the registers of `struct DosRegs` that
  the call takes, and the rest as they come.
- One instruction of `SaveSet` is listed as out of reach, `MOV AL,0`
  where the diskette of the program's drive is refused or fewer bytes
  are written than the setup has. The program is on D:, a hard disk, so
  only a full disk gets there: with the setup file deleted first and D:
  filled as `copy-full` fills C:, saving would. That scenario is the
  longest by far, 136 seconds of 468 when four checkouts test at once,
  and this would be it once more for one instruction: say if it is
  wanted.
- Of the mouse's routines, which are C, ten instructions of the
  original have run in no scenario and are translated by reading: six of
  `IniMous`, for a mouse driver that does not know the call 21h and for
  no mouse at all, which every machine of the bench has now, and four of
  `DoublMo`, where a first click falls into the last 40 hundredths of a
  minute. The rest ran when the bench got its mouse, and the C did what
  the original does in every scenario of the mouse the first time it
  was tried. Three more instructions of `DoublMo`, at 9506h, run where
  the hundredths of the first click are 60 or more. They ran or did not
  from one derivation of the goldens to the next while the machine's
  clock still took in time of the host's; they run in `mouse-list` and
  others, the same in every derivation now.
- Of the screen's routines 26 instructions are out of reach on the
  bench and translated by reading: a monochrome display in mode 7, two
  of `ChkMode` and `ChgCur`; a screen on another page than the first,
  two of `ScrOffs`; a display that shows snow, where `Window` and
  `MovScr` wait for it not to draw, 20; and TopView or DESQview, two of
  `Window`. Which of these the machine is, the program finds out when it
  starts, and no menu says otherwise.
- What these routines do in the original and the port does after them;
  whether to mend any of it is not decided:
  - `Window` tells TopView or DESQview which cells of their buffer have
    changed with ES on the main data, where the call takes the segment
    of the buffer. By reading: the bench runs neither.
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
- What the C of these routines hands the BIOS where the assembly had
  whatever a register held, none of which the BIOS reads: `Beep` writes
  the character 7 with page 0 and colour 0 where BX was as it came;
  `ChgCur` and `CurShp` set the shape of the cursor with AL 0, where
  some BIOSes of the time wanted the video mode there and the assembly
  had what the call before left; the mouse driver gets BX 0 with the
  calls that take none. DOS gets the registers of `struct DosRegs` and
  ES on the main data, whatever the caller of `Intr21` in the assembly
  has in ES: the one call that reads ES, which runs a program for
  Alt+F3, has it there. And `Fantom` shows its dialog with 0 for the
  include line and for the place of a box that is on the screen
  already, where the assembly had registers as they came: its box has
  no line that includes one, and is not on the screen.
- `OpenFil` asks DOS a second time, without what the mode says of
  sharing, where DOS says that another program has the file open. Three
  instructions, listed as out of reach: DOS says so only where files
  are shared, and nothing else has a file open on the bench.

## Ways of testing that were tried, for the user to decide on

The full `port` test took 105 seconds on an idle machine with 59 scenarios
and twelve minutes with eight checkouts busy, so four other ways were
tried on 11 and 12 October 2026, and a fifth, a driver that stays
resident in the guest, which is what the tests use now (see the README). What was measured first: 77% of a
scenario's time is the guest running while the harness waits, most of it
the program waiting for the timer while a box grows; pressing keys is
13%; reading the machine's memory for the stack 6.5%; starting a machine
1%. Recording what was executed costs nothing that shows. None of the
four is in use; each is on a local branch of the checkout named, not
pushed.

- A build of the port that tests itself (`proto-selftest`, checkout
  `.8`). The program's own routines that call the keyboard BIOS, 14
  places, and its mouse routines ask a driver in place of the BIOS; DOS,
  the disks, the screen and the clock are the real ones. 71 of 72
  scenarios equalled their goldens. It is 12% cheaper than today, no
  more. What it alone offers is the mouse: `Click@column,row` as a step,
  with four runs of clicks that came out equal to the goldens of the keys
  they stand for. 1,020 of the 1,289 instructions listed as out of reach
  are for the mouse, in 18 places. To decide: whether covering them is
  worth 30 to 50 hours; whether keys from a table of ours are acceptable
  where the original's come through the BIOS; whether a click may be held
  against the golden of a key in the meantime. A golden from the original
  would need a real mouse on the bench: FreeDOS's `CTMOUSE` is on the
  image already fetched, and QEMU can send mouse events; not tried.
- DOSBox-X as the machine (checkout `.9`, `build/dbx`, not kept in git).
  It starts no faster than QEMU from a saved state. With its turbo the
  guest's clock runs 16 to 37 times as fast and dialog-heavy scenarios
  take a fifth to a tenth; without it, as long as in QEMU. Its DOS is
  not FreeDOS: every screen differed in free memory and the form of
  dates, `move` and `attributes-directory` went another way, and some 20
  scenarios would have to be excused or recorded again. Not recommended.
- Routines held against their assembly originals on recorded inputs
  (checkout `.3`, `build/perf`). Capture through the debugger works, and
  so does replay in a paused machine, a millisecond a call, with the real
  DOS underneath. 27% of the translated code reaches neither DOS nor the
  BIOS and fits; 54% calls routines of ours that do; 18% calls them
  itself. 8 to 12 hours to build, for a quick check of a quarter of the
  code. Not recommended now.

## What batches have lost time to

Minutes that a batch actually lost, for deciding which improvement of the
tools pays (see "A run of batches" in `CLAUDE.md`).

- The files of a panel and the place of the cursor, 17 routines, about
  half an hour in all:
  - 3 minutes: three tries at the one instruction of `NearFl` that runs
    when no file of the old list is left. What worked is emptying C:\ of
    its two directories.
  - 2 minutes: an offset taken from a C structure that the assembly has
    otherwise (`TreList`), found by the `tree` scenario; and a label
    inside a procedure that others call (`NumAd1`), found by the
    assembler. Both are in `CLAUDE.md` now.

- A panel of files on the screen, 9 routines, about half an hour in all:
  - 4 minutes: goldens derived twice for three scenarios, 70 seconds the
    second time. `harness/notice.py` needs the C, so it ran after the
    translation, and found three branches that no scenario noticed: a
    selected directory counted once its size is known, the mini status
    off, and the clock that leaves the path of the left panel alone, or
    that of the right one under the menu bar. Asking before translating,
    which scenario takes a branch either way, would have had the
    scenarios right the first time; the coverage of the original records
    instructions, not which way a jump went.
  - 2 minutes: the one instruction of `SetCur` that is out of reach. A
    try of twelve steps on an emptied disk, and then reading the five
    places that lead to it.
  - 1 minute: a step of 23 keys failed, "the key 0 did not arrive". One of
    16 keys is taken. Noted at the top of `harness/scenarios.py`.

- The tree and the Info in a panel, 8 routines, about 50 minutes in all:
  - 5 minutes: code that stands between procedures, `PutTre` and
    `PutInf`, could not be named to `harness/reach.py` or listed in
    `harness/ported.py`. Both take it by its first label now, a commit of
    its own.
  - 4 minutes: a key lost in the editor. The port test failed before
    anything was translated, an Enter gone from a step of seven keys: the
    editor empties the keyboard buffer after every key. Found by reading
    its loop; the scenario switches the option off first. Noted at the
    top of `harness/scenarios.py`.
  - 4 minutes: two steps that could not stay, each a golden derived, 23
    seconds, and one of them a port test failed, 34: the volume label,
    which the program does not get off the disk, and the memory free
    with Memory allocation off, which depends on the size of the program.
  - 4 minutes: three tries until a scenario noticed `FilePan` taking a
    panel that is switched off for one of files. Ins does not get there
    and the menu refuses by itself; what shows it is the Info drawn anew
    next to such a panel. Each try was a golden derived and a run of
    `harness/notice.py`, 55 seconds together.
  - 2 minutes: the Info was 12 bytes deeper in the stack in every
    scenario, written from inside the function that chooses between it
    and the tree. The stub chooses now.

- Reading what a panel shows, 5 routines, 38 minutes in all:
  - 9 minutes: the stack. As one function `ReDir` left 12 bytes of it in
    every scenario, and nothing said where it was deepest: the stack
    dumped from the machine could not be read, and a log of the
    registers from QEMU could. It is deepest in the filter
    of a panel, where DOS makes capital letters on the caller's stack.
  - 6 minutes: `harness/notice.py`, six runs of it, and two scenarios
    made longer for three branches that none had noticed, with their
    goldens derived again, 50 seconds. No error in the translation was
    found by it.
  - 2 minutes: two tries of steps that failed after 12 and 17 seconds,
    for a text with a space in it. Noted at the top of
    `harness/scenarios.py`.
  - 1 minute: a far pointer in `[bx dx]` had its halves the other way
    round, so no file was opened; three scenarios showed it. In
    `CLAUDE.md` now.
  - 1 minute: `DrIn_F` went with `RdInfo` and the viewer uses it: found
    by the assembler.

- The viewer, 14 routines, 1,730 bytes, about 25 minutes in all, next
  to another batch in a checkout of its own:
  - 2 minutes: waiting for a turn at the computer, in three runs: 20
    seconds before a run of `harness/show.py`, 25 before the goldens
    were derived and 60 before the first `port` test. The rebase, the
    build and the test after it took a minute and met no conflict: the
    other batch was not on `main` yet.
  - 1 minute: a run of `harness/show.py` that pressed Enter on a file
    of A:\ instead of a directory and so ran `fdauto.bat`; the files
    there are not in the order of the bench's. Noted at the top of
    `harness/scenarios.py`.

- The viewer's keys, 2,290 bytes that are no procedure but for 20 of
  them, 40 minutes to a translation that passed its own scenarios the
  first time it was built, next to seven other batches; then the `port`
  test did not pass, for scenarios of copying, and the batch waited an
  hour and a quarter for its turn to land:
  - 7 minutes: the first full `port` test, 4 minutes 6 seconds at a load
    of 17 on two processors, and the two scenarios it failed in run
    again by themselves, a minute and a half and 44 seconds. `copy-full`
    stopped both times at step 36, '"c1" Enter', a copy of 3 MB that did
    not end within the 20 seconds a step had then. `move` differed at
    step 66, '"d:\" Enter', in the full run and not by itself: the
    port's screen was taken with the dialog of F6 off the screen and the
    box of the move not yet drawn, and the next two screens were a step
    behind. The end of a step of the port has been tied to the keys it
    has taken since. The other 56
    scenarios were as the original, and the checks that the test makes
    after the comparison were run for the viewer's scenarios by a
    script of thirty lines around the functions of `harness/ported.py`,
    49 seconds: every instruction ran or is listed, every function ran.
    A `port` test that could leave a scenario out, or that went on to
    its checks after a scenario has failed, would have said the same in
    one run.
  - 1 minute: a search expected to begin at the top of the file again,
    which begins at the place found last: one trial of 19 seconds on
    the port's build that signals. At the top of `harness/scenarios.py`
    now.
  - 1 minute: the golden of `view-keys` derived twice, 21 seconds the
    second time, for a file between what the viewer holds and 64K,
    which `harness/reach.py` showed to be missing when the rest was
    reached.
  - What saved time: trying every scenario on the port's build that
    signals before deriving its golden, 20 seconds for 58 steps, by the
    script that the batch before last describes; and a file of
    C:\STAMPS, which has a megabyte by its entry in the directory and
    no contents, for a dirinfo too large for the editor.
  - 12 minutes for getting onto `main`, behind thirteen commits, the
    editor's keys and the bars and the input among them. Three files
    in conflict, `README.md`, this one and the list of C functions in
    `VC.ASM`. What git did not show: the editor's batch had kept the
    stubs of `T70_00` and `T71_00` for the viewer's assembly and
    written `V80_00` out for itself, while this one had kept the stubs
    of `V80_00` and `V82_00` for the editor's; `PutPrm`, `PutMnu`,
    `PutKey` and `Input` were functions, and their entries gone, the
    one this batch had made for `PutPrm` with them; a scenario could no
    longer have a key pressed ahead, so the Esc that stops a search is
    "Enter+Esc" and that golden was derived again, 19 seconds; and the
    compiled code was 1830 bytes over the 64K of its segment, so
    `view.c` is in the second one. All four stubs went, the editor
    calls `V80_00`, and the build, the viewer's scenarios and the
    `port` test, 5 minutes 33 seconds, passed at once.

- The editor: addresses in its text, a line on the screen, saving, 13
  routines, 1079 bytes, 40 minutes in all, next to the viewer's batch:
  - 5 minutes: a scenario that could not stay as written, found by the
    `port` test before anything was translated. It showed D:\VC in a
    panel, where the program's file has another name and size; and it
    saved to the write-protected floppy, which left 4 bytes of the stack
    in the port as it was. Two goldens derived again, 33 seconds each,
    and the stack of every step read from the machine by a script of ten
    lines around `scenarios._stack_left`: `ported.py` says how much a
    scenario leaves, not after which step.
  - 4 minutes: `T75_00` left CX as it came, where the original leaves
    0, and its caller goes on with CL. The test failed on a status line
    drawn once too often, 45 seconds; the caller was read again.
  - 3 minutes: two tries at a full disk, 31 and 52 seconds, before it
    showed that the floppy is write-protected. In `harness/scenarios.py`
    now.
  - 1 minute: an edit of `harness/scenarios.py` by a script cut off what
    follows the scenarios; put back from the commit before.
  - 1 minute waiting for a turn at the computer, in three runs.
  - 3 minutes for the rebase, with the viewer's batch on `main` first:
    six files in conflict, each where both batches had added to the end
    of the same list. Git cut the two lists of batches and of scenarios
    at lines they share, a closing bracket, so putting one side behind
    the other left both open; mended by hand. The build and the test
    after it took a minute and passed at once.

- The editor: the line buffer, from line to line and from word to word,
  9 routines, 1032 bytes, 30 minutes in all, next to three other batches:
  - 4 minutes: a step that the harness cannot record, Up in the editor
    where only the number of the line changes, to a lower one. Two runs
    of `harness/reach.py` that failed after 34 seconds each, and reading
    `wait_for_screen` for why. At the top of `harness/scenarios.py` now.
  - 2 minutes waiting for a turn at the computer: 60 seconds before a
    run of `harness/reach.py`, 15 before the goldens were derived, and
    the `port` test with the routines still assembly took 138 seconds
    next to the tests of the others, where it takes 99 by itself.
  - 4 minutes for getting onto `main`, twice: the larger stack arrived
    while the test after the first rebase ran. One conflict, in the notes
    at the top of `harness/scenarios.py`, where both had changed the same
    paragraph; a build and a test of 70 seconds each time. None of the
    other batches was on `main` yet.

- The editor: opening a file, the screen and every key, code between
  procedures, 3,900 bytes, next to seven other batches. The translation
  passed the first time it was tested, but for two functions of earlier
  batches that nothing called any more.
  - 4 minutes: a file of 332,152 bytes for the editor, which fitted the
    original and the port as it was, and was 504 bytes too large for
    the port with the editor in C: the scenario failed on the first
    run of the translation. It has a file now that is too large for
    both. At the top of `harness/scenarios.py`.
  - 3 minutes and a half: a full `port` test that failed on nothing but
    `T60_00_c` and `T61_00_c`, listed for an earlier batch and called
    by stubs that had gone. `harness/compare.py` on the batch's own
    scenarios does not look at that.
  - 3 minutes: the Info in both panels, where the second "Bytes Free"
    of a row is compared. Two more layouts tried: with the right panel
    off, the screen behind it shows the end of the banner, "ware" and
    "are"; with files in the right panel it holds.
  - 2 minutes: a search through 245K in the original, which did not
    end within the 20 seconds that a step had then, while the goldens
    were derived next to other runs; and A:\SETUP.BAT, which the
    bench's floppy has not.
  - 14 minutes: a step of five keys in `editor-full` with two beeps in
    it and F4 before and after them, which the port passed twice and
    then failed, in a full `port` test of nine minutes: the key bar
    was missing. It is drawn only where the program waits and finds no
    key, and the port need not wait within a step. Found by reading
    `Input0` and `PutKey0`; at the top of `harness/scenarios.py` now.
  - What saved time: `harness/reach.py` with steps and `--changes`
    written to a file, 10 seconds for 30 steps, and the scenarios run
    on the port before anything was translated, with
    `harness/compare.py` and the build that signals: 30 seconds for
    all of the editor's.

- The help, 9 routines, 1,403 bytes, 36 minutes in all, next to three
  other batches. The translation passed the first time it was tested.
  - 5 minutes: planning the C and its stub around the 256 bytes of the
    stack, read from the callers and the routines below, before the
    stack was made larger. The test then failed on nothing but the
    stack, 6 bytes left with F1 in a dialog, and passed unchanged on
    the stack of 4,096 bytes; half a minute went on waiting for that
    to reach `main`.
  - 4 minutes and a half waiting for a turn at the computer before
    the rebases, in six runs: 57 and 26 seconds before two runs of `harness/reach.py`
    with steps, 97 before the goldens were derived, 46 and 19 before
    two `port` tests, and 25 before a derivation that then stopped at
    once, the scenarios not yet in the file: an edit had been refused
    and the refusal not read.
  - 1 minute: the C function named `Help_c`, which the assembler takes
    for `Help_C`. In `CLAUDE.md` now.
  - 1 minute: the byte 1Ah at the end of the files of `port/asm/`,
    which a script that split them by lines did not expect. In
    `CLAUDE.md` now.
  - 11 minutes for four rebases, the push refused once. The last was
    clean, and its test waited four minutes for a turn, behind a run
    that recorded the original. The second, on the larger stack, had
    one file in conflict, the notes at the top of `harness/scenarios.py`,
    where both sides had changed the end of the list. The third, on the
    editor's second batch, which reached `main` while the commit was
    being written, had six, each where both had added to the end of the
    same list; the list of batches was cut at its closing bracket again,
    and a script put the lines in between. Each time a build and a test,
    a minute and a half, which passed at once.

- Find file, 9 routines, 1,536 bytes, 42 minutes in all, next to three
  other batches. The translation passed the first time it was tested,
  but for the stack and for a stub that nothing called.
  - 8 minutes: finding out when a key reaches a search that runs, and
    what a step can rest on: three runs of `harness/reach.py` with
    steps, 17, 30 and 38 seconds, and reading `Input` and the harness
    for where the milliseconds go. A search by contents of the floppy
    stood still for longer than the original is waited for, so a step
    was recorded in the middle of it. At the top of
    `harness/scenarios.py` now.
  - 6 minutes waiting for a turn at the computer: 95 seconds before the
    third run of `harness/reach.py`, 50 and 40 before two derivations,
    and some three minutes of a `port` test that took 4:43 where it
    takes 1:16 by itself.
  - 1 minute and a half: a derivation that stopped, "the screen did not
    settle", at the step that ends in the help; the next one went
    through.
  - 1 minute: `FndAdr_c` not executed in the port. Its stub is called
    by Go to alone, which no scenario had: three steps more, and the
    golden derived again.
  - The `port` test failed on the stack alone, none of 256 bytes left
    in `stamps` and `find`, and passed on the stack of 4,096 bytes,
    which was on `main` by then.
  - 4 minutes for getting onto `main`, behind the larger stack, the
    editor's second batch and the help: three files in conflict, the
    lists of batches, of scenarios and of the C functions that the
    assembler is told of, the first two cut at their closing brackets
    again and put right by a script. Seven entries for C were there twice,
    made by the help's batch as well and under the same names, without
    a conflict; those of this batch went, and its C took the arguments
    and the answers of the others. A build and a test, 76 seconds,
    passed at once.

- The tree read from the disk and its box, 3 routines, 1,792 bytes,
  34 minutes in all, next to three other batches. The translation
  passed the first time it was tested, and nothing but waiting was lost.
  - 8 minutes waiting for a turn at the computer before the rebase, in
    four runs: 158 seconds before the goldens of the two scenarios were
    derived, which then took a minute, 143 and 134 before the two `port`
    tests, the one before translating and the one after, and 43 before a
    comparison of four scenarios. Two runs of `harness/reach.py` with
    steps, 20 and 36 seconds, did not wait.
  - 4 minutes for getting onto `main`, behind find file: two files in
    conflict, `README.md` and this one, each where both batches had
    written in the same place, and a script kept both. The lists of
    batches, of scenarios and of the C functions that the assembler is
    told of merged by themselves: this batch has its entries behind
    those of the tree in a panel, not at the end, and its entries for C
    next to its stubs. A build and a test, which waited 86 seconds for
    a turn and passed at once.

- Deleting, attributes and `DirFunc`, 8 routines, 1,607 bytes, 54
  minutes in all, next to three other batches:
  - 10 minutes waiting for a turn at the computer, in seven runs: 99
    seconds before a run of `harness/reach.py` with steps, 42, 157, 104
    and 77 before the goldens were derived, 27 before a `port` test and
    98 before a run of `harness/show.py` on the port. The others' tests
    follow one another, and a run that wants the computer alone waits
    for a gap.
  - 6 minutes: `EXTERN` for the C functions at the top of `VCF1.INC`,
    which is included inside the code. The assembler made near calls of
    the far ones without a word; deleting worked and the attributes
    ended in "Invalid Opcode". A `port` test failed, 77 seconds, three
    runs of `harness/show.py` on the port, and the bytes of a stub read
    in the program file. In `CLAUDE.md` now.
  - 6 minutes: the golden of `delete` derived four times, 71 seconds
    each. The tree's file is gone from the root after a directory has
    been deleted from the tree; a panel lists the new one only when it
    is read again; and `DelTr0`, a label inside `DelTree` that `^R` in a
    tree calls, was found by the assembler when the scenario was done,
    and wanted a step. At the top of `harness/scenarios.py` now. Reading
    `tools/routines.py` for labels that others call, before the
    scenarios are written, would have saved the last one: it does not
    show them.
  - 4 minutes: `harness/reach.py` and the `port` test did not know
    `DirFunc`, which is code in the program's data. `harness/coverage.py`
    reads such procedures now, a commit of its own.
  - 1 minute: "F8 Enter" in one step leaves the box of F8 open, which
    takes no key typed ahead; half a try of 49 steps went another way
    than meant. Noted at the top of `harness/scenarios.py`.
  - 4 minutes for getting onto `main`, behind the tree and its box: two
    files in conflict, `README.md` and this one, where both batches had
    written in the same place. No conflict, and the same name twice:
    the tree's batch had made entries for `TstBrk` and `OncZoom` next
    to its stubs, and this one next to the routines; those next to the
    routines stay. It had also made an entry for `DelTr0`, which is C
    now: the entry went, and the tree's box calls the function. A build
    and a test of 146 seconds, which passed at once.

- Waiting for a key, the line that is typed into, the bars and the
  command line, 16 routines, 1,849 bytes, 72 minutes in all, next to
  seven other batches. The translation passed the first time it was
  tested, and no golden was derived twice.
  - 1 minute: `PtMnu1`, a label inside `PutMnu` that the menu calls, and
    `PtMnu_S`, which the menu reads: the first found by the assembler,
    the second by looking for every table that was to go. In `CLAUDE.md`
    now.
  - 3 minutes: the `port` test took 268 seconds next to the tests and the
    recordings of seven other batches, where it takes 105 by itself. The
    steps were tried on the port's build that signals first, 6 to 10
    seconds a run, and the three goldens were derived once, in 19
    seconds.
  - 2 minutes: the commit had the message of another batch. The file
    it was written to was in the directory for scratch files that the
    session gives its agents, which all of them share, and another agent
    had written one of the same name meanwhile. Seen in the log before
    anything was pushed. Such files belong in the checkout's own
    `build/`.
  - 55 minutes for getting onto `main`, six times. The first rebase met no
    conflict and no entry twice, and its `port` test failed after 324
    seconds in `copy-full`, a scenario of the copying: step 42, '"c4"
    Enter', a copy of 3 MB, was still copying when the 20 seconds were
    over that a step might take, with a load of 14 on the two
    processors. The other 57 scenarios were as the original, and by
    itself, with the same build, `copy-full` passed, in 92 seconds where
    it takes 33 on an idle machine. The batch stopped there, committed
    and not pushed, as its instructions said for a test that comes out
    different from one run to the next; a step has 300 seconds since.
    The second rebase, twenty minutes later and behind the commands of
    `VCF1.INC` and the file search, met no conflict either; the file
    search called `PutMnu` by its entry, which was gone, and calls the
    function now. The test took 337 seconds and passed, and meanwhile
    the signals had been changed on `main`: W, S and T say where the
    keyboard buffer ended. That is all in the macros, which the entries
    of `Input0` hold, so the third rebase needed nothing; its test took
    247 seconds, and by then the scenario `find` had been changed on
    `main`. The fourth was clean too, and its test took 340 seconds;
    the user menu and the command line of the panels had landed by
    then. The fifth had four files in conflict: `harness/ported.py` and
    `VCSUB1.INC`, where both had written in the same place, and the two
    files of a golden, because both batches had named a scenario
    `command-line`: this one's is `command-edit` now, and its golden was
    derived again, 14 seconds. What git did not show: that batch had
    made entries for `DosCom`, `FndItem`, `Input0` and `ClsLine`, and
    its three C files called these and `PutMnu`, `PutKey`, `Input` and
    `EdLin` by entries. The entries went, the C calls the functions,
    with `FndItem`'s answer turned round in a function of the user
    menu's, and the stubs of `FndItem` and `Input0` went too, having no
    caller left. Ten minutes, and a test of 195 seconds, during which
    the editor landed. The sixth had `port/edit.c` and `VCEDIT.INC` in
    conflict; the editor's entries for `EdSub1` and `CmpSym` went, it
    calls the functions and `Input`, and the stub of `EdSub1` went. Its
    test took 162 seconds.

- The dialogs, 15 routines, 2,190 bytes, 50 minutes in all, next to
  three other batches. The translation passed the first time it was
  tested.
  - 8 minutes: an address that `harness/reach.py` listed, taken for
    another instruction. `Dialog` has `ADD BL,40` three times; the
    listing was searched for a label near the first of them, because
    `grep` takes the listing for a binary file and found nothing by the
    address: `grep -a`. The steps that reach the first did not reach the
    one listed, so the record of what ran looked wrong, and a trace of
    QEMU was kept and read to find that it was right. Three minutes of
    that were waiting for a turn.
  - 5 minutes waiting for a turn at the computer otherwise: 137 seconds
    before a run of `harness/reach.py` with 30 steps, 63 before the
    golden was derived, 100 before the `port` test with the routines
    still assembly; and the first test of the C took 156 seconds where
    it takes 78 by itself.
  - 3 minutes: "not executed in the port" for three functions that
    began by loading the segment of the assembly as `seg Title_S`, a
    number that is set when the program is loaded. A test of 139
    seconds, and an entry that hands the segment over instead. In
    `CLAUDE.md` now.
  - 15 minutes for getting onto `main`, three times, each time behind
    what had arrived while the test before ran: the tree, deleting, and
    a change to the harness. Behind the tree one file was in conflict,
    `README.md`, and three things git did not show: the tree had made
    entries for `Input`, which this batch had made too, and for
    `InSt10`, which is C now and takes its arguments in another order,
    and `struct Box` was in its C file and in `port/vc.h`. Behind
    deleting eight files were in conflict, the lists of batches and of
    scenarios cut at their closing brackets again and put right by a
    script; its C file called `Dialog` by the entry that is gone, in
    seven places, and had `struct Box` too. The third was clean. A build
    and a test each time, 78, 142 and 75 seconds.

- The keys of the panels, 29 commands and routines, 1,543 bytes, next
  to seven other batches. The translation passed the first time it was
  tested, and nothing in the harness had to be waited for:
  - 3 minutes: finding out what the tools take for code between
    procedures. A label names it as far as the next procedure, so
    `F22_00` to `F27_00` cannot be listed without `F80_00` to `F32_57`,
    which another batch has; they were left. In `CLAUDE.md` now.
  - 2 minutes: the golden of `panel-keys` derived four times, 12 to 17
    seconds each. With the right panel off the end of the program's
    banner shows, which the original and the port have differently:
    found by `harness/compare.py` on the port before anything was
    translated, 20 seconds. Then a step for the one instruction left,
    and one for the function that only the menu calls. At the top of
    `harness/scenarios.py` now.
  - What saved time: the scenarios were tried on the port's build that
    signals, six seconds for twenty steps, by a script of ten lines
    around `scenarios.run` and `show.print_screens`, and held against
    their goldens with `harness/compare.py` before the translation and
    after it, 20 seconds; the full `port` test ran once before the
    rebase, 193 seconds next to the others.
  - 3 minutes for the rebase, behind what Copy and Rename ask first: one
    file in conflict, the notes at the top of `harness/scenarios.py`,
    where both had added to the end of the list. Two entries were there
    twice without a conflict: `SetPth`, which the other batch had made
    next to the routine and with an answer, and the call of DOS that
    makes a drive current, `SelDrv_a` there and `SelDsk_a` here, the
    same instructions. Those of this batch went.
  - 7 minutes: the `port` test after the rebase, 356 seconds with a
    load of 13 on two processors, failed in `copy-full`, which is not
    of this batch: after step 52, '"c9" Enter', the ninth copy of 3 MB,
    the program had not come to wait for a key within the 20 seconds a
    step had then. The 56 scenarios before it were as the original, and
    `copy-full` by itself, on the same build a minute later, was too,
    in 55 seconds. The batch waited, committed, for fifty minutes, as
    its instructions said for a test that comes out different from one
    run to the next.
  - 7 minutes for the second rebase, behind eight commits, with the
    machine quiet: three files in conflict, `README.md`, this one and
    the notes at the top of `harness/scenarios.py`. `ClsLine`, `PutPrm`
    and `PutMnu` had become C, with a segment and a panel for
    arguments, and `Cursor1` had an entry next to the routine: the
    entries of this batch for them went. Then the program did not
    link, "size of segment CTEXT exceeds 64k by 1279 bytes": the
    compiled code had outgrown its one segment. A C file can be put
    into a second one now, a commit of its own, and `keys.c` is. The
    test took 182 seconds and passed.

- The menu bar, 1,498 bytes of code that is no procedure with its five
  routines, some 65 minutes in all, next to seven other batches. The
  translation passed the full test the first time that ran.
  - 3 minutes: the end of the banner. The scenario switched the right
    panel off next to the left one, and the port showed "are" where the
    golden has "ware": `harness/deviations.py` knows the banner by its
    beginning only (see above). It looked like the translation, and was
    in the port without it: the original and the port were run through
    "Ctrl+l" and "Ctrl+p" with `harness/show.py`, and the scenario
    switches the left panel off now. The `port` test with the batch
    listed without functions, before translating, would have said so
    at once; it was left out to save a run of three minutes.
  - 2 minutes: F1 on an item showed the index of the help. The table of
    the pages was a constant of two dimensions, which the compiler read
    in the main data. Found in the listing; in `CLAUDE.md` now.
  - What saved time: `harness/compare.py` with `--signals` and the name
    of the one scenario holds the port against its golden in ten
    seconds, and both of these showed there, not in the full test.
    `harness/show.py` on the port's build that signals tried the 49
    steps in nine seconds before the golden was derived, which took
    nine more.
  - 4 minutes: the registers at the jump to a command. The stub gave
    the commands CX and DS, after every command that the tables name
    had been read for what else it takes: nothing. The batch of Copy
    and Rename, which arrived with the rebase, had found that Rename
    goes by BX and DX as they come. The stub hands on every register
    but BP now, a step has Rename chosen from the menu, and the golden
    was derived a third time.
  - Getting onto `main`, three rebases. The first two took a minute
    each, with the same two files in conflict, `CLAUDE.md` and
    `harness/scenarios.py`, where both sides had added to the end of
    the same list of notes. The full `port` test, 218 seconds each time
    where it takes 105 on an idle machine, passed before the first
    rebase and failed after it, in `copy-full` alone, at the 20 seconds
    that a step could take then; the batch waited for the harness, an
    hour, committed and not pushed. The third rebase had five files in
    conflict, these two, `README.md`, this one and the list of `EXTERN`
    in `VC.ASM`, each where both sides had added. What git did not
    show: `PutKey`, `Input` and `PtMnu1` had become C, so their
    declarations here went and with them the entry made for `PtMnu1`;
    and the linker said that CTEXT exceeds 64K, so `menu.c` is in the
    second segment. The test then failed once more, for an instruction
    listed as out of reach that the scenario `cursor` of another batch
    runs: the one that puts the tick on Full, which had been taken for
    that of a panel of a kind that there is not, because the two ways
    to it share a jump. It is off the list, and the C was right. Twelve
    minutes for the third rebase, two tests among them, of five minutes
    and of three and a half.
  - A file of the commit message, kept in the directory for temporary
    files that the eight agents share, was overwritten by another
    agent's. It was written again, into `build/` of the checkout.

- Copying, renaming and moving, 9 routines, 3,018 bytes, 100 minutes in
  all, next to three other batches at a time, seven of which got onto
  `main` first:
  - 14 minutes: keys that did not arrive. 'F5 "readme.txt" Enter' as
    one step failed with "the key t did not arrive", in three runs of
    the original, 3 minutes of them, before a run of the port with the
    keyboard buffer printed at every key showed what it was: after a
    box with an error the dialog of F5 comes up slowly, and the keys
    behind F5 fill the buffer. It looked like one letter of one name,
    because the sixteenth key was a `t` each time. `reach.py`,
    `show.py` and `goldens.py` say which key and not which step, and
    print no screen of the steps before. Noted at the top of
    `harness/scenarios.py`.
  - 19 minutes waiting for a turn at the computer, in 43 runs, about
    half of them while other work went on: a derivation of goldens
    waited a minute or two each time, eight times, and the last `port`
    test took four minutes where it takes one and a half.
  - 12 minutes: goldens derived again. `move` three times, 2 minutes
    each with the waiting: once for steps added, once because a step of
    the editor in hex stopped with "the screen did not settle"; the
    file it was to make is now copied from the help file. `copy-full`
    four times, 2 or 3 minutes each: for a better ending; because that
    ending, going on after a full disk with a file that was appended
    to, left other contents in the port than in the original (see
    above), which the `port` test found after the rebase; and for
    smaller steps, after a `port` test of four minutes that stopped in
    it at the 20 seconds a step may take.
  - 1 minute: F6 and Enter on a directory, which moves it into itself.
  - 31 minutes from the first rebase to the push: four rebases, each
    with a build and a `port` test of two and a half to four minutes,
    and each but the last overtaken by another batch while its test
    ran. Six or seven files in conflict each time, `CLAUDE.md`,
    `README.md`, `TODO.md`, `ported.py`, `scenarios.py` and `VC.ASM`,
    wherever both sides had added at the same place: settled by a
    script that puts one side behind the other with the lines they
    share between them, a minute or two each time. What did not show
    as a conflict: `DirFunc_a`, `OncZoom_a` and `TstBrk_a` were there
    twice, made by other batches too, the same instruction for
    instruction; then `DirFunc`, `DelTree` and `Dialog` had become C,
    with other arguments, and the entries were gone. The C of this
    batch was changed to call what is there each time. Two of the
    tests failed: on the contents that `copy-full` left, and on its
    long step (see above).
  - What saved time: steps tried on the port's build that signals,
    which is the original's code until something is translated, needs
    no turn alone and takes a tenth of the time. A script of fifteen
    lines around `scenarios.run` and `show.print_screens` ran a whole
    scenario of `scenarios.py` that way; four scenarios of sixty steps
    were right when their goldens were first derived.

- What Copy and Rename ask first, the code of six commands and three
  routines, 903 bytes, 25 minutes to the commit, next to seven other
  batches. The translation passed the first time it was tested, and no
  run waited: nothing takes turns any more. Two goldens were derived in
  20 seconds, the first `port` test took 151 seconds.
  - 3 minutes: three drafts of the scenarios, tried on the port's build
    that signals, 10 seconds each. A step typed into the buttons of the
    dialog instead of its line, and after a directory of the tree is
    renamed the cursor is on another line. Both are at the top of
    `harness/scenarios.py` now.

- The commands of `VCF1.INC` and the file search's, 2,099 bytes of the
  original in five stretches and three small routines, next to seven
  other batches. The translation passed the first time it was tested,
  but for one number.
  - 6 minutes: `F37_00`, `F47_00` and `F38_00` could not be named to
    `harness/reach.py`: they stand in a stretch of code that begins in
    `VCMENU.INC`, with `M60_00`, and the tools took a stretch by its
    first label only. `harness/coverage.py` takes any label of one now,
    a commit of its own.
  - 2 minutes: the key bar of the box of the search, read from the
    wrong byte of the dialog's header, 32, which is its help page. Four
    scenarios showed it at once: 45 seconds, and the `STRUC` read
    again.
  - What saved time: all steps tried on the port's build that signals
    first, six runs of `harness/show.py --changes` of 5 to 15 seconds,
    before the goldens were derived, which then took 21 seconds for
    four scenarios and were right, but for Ctrl+F8, which was added.
    And `harness/compare.py` with only the scenarios of the batch, 20
    to 45 seconds, before the full test.

- The user menu, the command line, the history and the box of the
  drives, nine names in `harness/ported.py`, 3,429 bytes, 67 minutes
  in all, next to seven other batches. The translation passed its own
  four scenarios the first time it was run.
  - 14 minutes: the full `port` test failed once, 3 minutes 35, in
    `copy-full`, where a step that copies 3 MB took more than the 20
    seconds a step had then, with a load of 14 to 20 on two processors;
    every other scenario had passed, and nothing after the comparison
    was printed. So after the first rebase a script of twenty lines
    around the functions of `harness/ported.py` ran everything but
    `copy-full`, 5 minutes 36, and `copy-full` ran by itself, 3 minutes
    34 where it takes half a minute: both passed. Then the wait of 300
    seconds reached `main`, and the full test after a second rebase
    passed, 4 minutes 46. `harness/ported.py` cannot leave a scenario
    out, and says nothing of what ran when one scenario stops it.
  - 5 minutes for the first rebase, behind three commits: four files in
    conflict, `CLAUDE.md`, `README.md`, `harness/scenarios.py` and the
    list of functions in `VC.ASM`, settled by a script. What git did not
    show: the file search's batch had made `Int21d_a` for DOS called
    without `Intr21`, and the entry made here for the same went; and its
    note that Shift-F6 reads BX and DX as the key before left them, which
    a scan of the commands made here had not found: the main loop was
    read again for what gets round it.
  - 3 minutes: scenarios that could not stay as tried. A new file in the
    editor wants Enter once more, but not vc.mnu; a comment behind a Tab
    is run; the Info next to a panel on D: shows free bytes that follow
    the size of the program, found by comparing the port as it was with
    the new golden, which was derived again, 33 seconds.
  - 2 minutes: three builds for a variable named `far`, and for a far
    pointer kept in a near one. In `CLAUDE.md` now.
  - 1 minute: the scratchpad directory is one for all agents, and a
    directory made there for a second derivation held another agent's
    goldens under the same name. Nothing was lost; the files are in
    `build/` since.
  - What saved time: steps tried on the port's build that signals, six
    seconds for twenty steps; the four scenarios, 272 steps, were right
    on the original when first derived, but for the figure of D:. And
    `harness/coverage.py` came to name a later label of a stretch while
    this batch was at work, so `F80_00` could be listed after all.

- The map of the memory, one command and five routines, 2,036 bytes, 25
  minutes to the commit, next to seven other batches; it passed but for
  one thing the first time it was tested.
  - 3 minutes: the vectors came out wrong. A far pointer into segment 0,
    written with the constant, is compiled as a near one, and the map
    read the main data for the table of vectors. Two scenarios showed
    it, and the listing why. In `CLAUDE.md` now.
  - 4 minutes: goldens derived three times, 45 seconds each: for what
    is left out, where the number of blocks of free memory turned out
    to follow the size of the program too, which the `port` test of the
    scenarios showed before anything was translated; and for a step
    more.
  - 6 minutes: deciding what to do about the name of the program in the
    map, VC.COM or VC.EXE, which no scenario of the map can avoid
    showing.
  - 25 minutes to get onto `main` an hour later, behind fourteen
    commits: four files in conflict, each where both sides had added,
    the list of scenarios cut at the lines that two scenarios end with.
    What git did not show: `PutMnu` had become C, and the keys of the
    panels had made entries for `SavMous` and `CurMous` too, which went
    for those next to the routines. The first `port` test, 9 minutes,
    failed in `command-edit`, a scenario of another batch: "Invalid
    Opcode" after a command of an extension file. The compiled code of
    the first segment was 64,636 bytes with `memory.c` in it, 900 below
    the 64K, and the linker had said nothing. The same scenario passed
    on `main` as it was, 17 seconds, and with `memory.c` in the second
    segment, where the first has 61,177 bytes; the second test, 254
    seconds, passed. Why the program fails with a first segment that
    full was found out afterwards: see `CLAUDE.md`, "C for the port".

- The commands of the menus, 28 commands and three routines, 1,210
  bytes, 50 minutes in all, next to three other batches and the mouse of
  the bench:
  - 10 minutes to get onto `main`, behind the two commits that gave the
    bench its mouse: no conflict, the goldens of the four new scenarios
    derived again for the pointer, and one `port` test, 256 seconds next
    to the tests of others. Eight of the nine instructions that EGA
    lines has for the mouse ran at once, with a mouse there.
  - 5 minutes: one `port` test more, 100 seconds on a quiet machine and
    270 next to the tests of others, because the scenario of the floppy
    under its second letter reached four instructions of `ReDir` that
    its batch had listed as out of reach. `harness/reach.py` says so
    beforehand only for a routine that is asked about: after goldens are
    derived it could say for every batch which of its listed
    instructions have run.
  - 4 minutes: Drive... of a panel's menu, which the port got wrong
    before this batch (see above). It showed in reading what the menu's
    table jumps to, before any scenario chose the item.
  - 3 minutes: a scenario with the Info in both panels, which fails on
    the port by the second "Bytes Free" of a row. The top of
    `harness/scenarios.py` had said so.
  Nothing was lost to the compiler or to the stubs: both were written
  by the notes of `CLAUDE.md`, and the first build ran.


- The main loop, the panels drawn and read, a drive that is not ready,
  and leaving, 1,801 bytes, about three quarters of an hour in all, of
  which reading `VC.ASM` and what the loop hands on took most:
  - 12 minutes: three runs of `harness/show.py` that waited out the five
    minutes of `STEP_TIME` each, two of them side by side. `b:` typed
    as a command leaves the program at "Insert diskette for drive B:",
    drawn before the main loop has set `AutoTim` in the new main data,
    and the driver hands no key to a program whose `AutoTim` says that
    something is pending. The screen that the error shows after five
    minutes is the blanker's. A copy of `show.py` with `STEP_TIME` at
    12 seconds showed the box at once; a switch of `show.py` for that
    limit would have saved ten of the minutes. The top of
    `harness/scenarios.py` has both facts now.
  - 6 minutes: four tries at the scenario of a drive that is not ready,
    40 seconds each, until it was understood that the box makes the
    diskette that of its drive again as it closes, and that a panel
    which could not be read is read when the viewer is left.
  - 4 minutes: the first run of the `port` test, which failed on seven
    instructions that two earlier batches had listed as out of reach
    and that `not-ready` runs; `harness/reach.py` on the routines of
    the batch does not show those of other batches. The test took 171
    and 227 seconds next to those of three other batches, where it
    takes 80 alone.
  - 1 minute: a macro used before it was defined, which JWasm calls a
    syntax error. In `CLAUDE.md` now.
  - 15 minutes to get onto `main`, behind the mouse of the bench and the
    commands of the menus: three files in conflict, `CLAUDE.md`,
    `TODO.md` and the list of files of the second segment, each where
    both sides had added. What git did not show: the commands had made
    `Int10_a` and an entry for `SaveSet` as this batch had, under the
    same names; theirs stayed, and both are declared in `port/vc.h`
    now. The three scenarios of the mouse passed on this C at once, so
    nothing was lost to the mouse at the panels; a fourth was written
    for what they leave, and derived five times because one record of
    what ran differed (see above).
- The shared routines of `VCSUB1.INC` and four of `VCSUB3.INC`, 49 of
  them, 1,822 bytes, with the file for DOS and the BIOS, some 45 minutes
  to a translation that passed, next to three other batches and one
  change to the harness. No golden was derived twice, and the full
  `port` test passed both times it ran, 173 and 233 seconds.
  - 2 minutes: the halves of a number of 32 bits in `[dx cx]`, which are
    the other way round than in `[dx ax]`: the cursor came back to the
    wrong place after the help, in the third scenario compared. In
    `CLAUDE.md` now.
  - 5 minutes: the first comparison of all scenarios, 4 minutes 26 with
    the other checkouts at work, where the `port` test takes three.
  - Most of the time went into reading: the 125 places of the assembly
    that call these routines, for the registers they go on with. Three
    mattered: the start of the program takes AL, AH and BH from
    `ChkMode` and DI from `ScrOffs`, and Enter with Shift keeps a flag
    in AH across `InvVar`.
  - What saved time: `harness/compare.py` on three scenarios, four
    seconds, before all of them; and keeping the functions' names and
    registers as their entries had them, so that the sixteen C files
    that call them lost their declarations to a script and changed in a
    dozen places.
  - 15 minutes to get onto `main`, behind the mouse of the bench, the
    commands of the menus and the main loop: eight files in conflict,
    each where both sides had added. What git did not show: `main.c`
    and `commands.c` declared 19 of these routines by their entries;
    seven stubs had lost their last caller to the main loop; the two
    batches had made entries for the BIOS and for a vector of DOS,
    which went into `port/dos.c`, the first by making `Int10` take the
    structure that their code already filled; and the new scenario of
    the left-handed mouse ran 55 instructions that the dialogs had
    listed as out of reach, which the first `port` test said, 132
    seconds. The second passed, 348 seconds next to other runs. The
    mouse's routines, read and never run before, needed no change.
- Enter, the keys that change the directory, the commands that view
  and edit, and `FncSel`: 2,252 bytes, all but 281 of them no procedure,
  about 50 minutes to the commit, next to four other batches. The
  translation passed its scenarios and the `port` test the first time it
  was built.
  - 3 minutes: the files of the listing, written as text into `build/`
    to read them, were moved out of the way with `build/*.txt`, and
    `CMakeCache.txt` went with them: the next build said "not a CMake
    build directory" and took the list of the C files with it; the
    cache put back and `cmake -S . -B build` mended it. Keep such files
    in a directory of their own from the start.
  - 2 minutes: `F21_31`, in the middle of Enter, is where the start of
    the program jumps to; the assembler said so when the label was gone.
    Only the labels that looked like entries had been looked for. In
    `CLAUDE.md` now.
  - 3 minutes: the places a command goes on at were first made an
    `enum` of `port/vc.h` for all files, `M03_00` among them, which is
    the name of a routine that another batch has in hand. Undone before
    the commit; in `CLAUDE.md` now.
  - 2 minutes: the first scenarios copied the driver of the tests,
    which is on the floppy as driver.com, and showed the bench's
    fdauto.bat in the viewer: both change when the harness does, and
    another batch was changing it. Two goldens derived again, ten
    seconds. At the top of `harness/scenarios.py` now.
  - What saved time: `harness/reach.py --steps` with `--changes`, three
    seconds a try, said for every idea whether it reached what was
    left, a dozen tries in all; and the `port` test before anything was
    translated, which showed that the three scenarios hold.
  - 30 minutes for getting onto `main` twenty minutes later, behind
    five commits: the mouse on the bench, the commands of the menus,
    the main loop and the shared routines, with every entry for C of
    theirs gone. Ten files in conflict. What git did not show: the C
    of this batch called DOS, the BIOS and the cursor by entries that
    were functions of `port/vc.h` by then, and its own entry for the
    BIOS went for `Int10`; 38 stubs of shared routines had lost their
    last caller in the assembly with Enter and the viewer's commands,
    found by `tools/routines.py` on both trees and taken out by a
    script, with their lines of `EXTERN` and of `harness/ported.py`;
    the four instructions for the mouse ran, now that the bench has
    one, and left the list; the three goldens were derived again for
    the pointer. The build passed at once, and the `port` test, ten
    minutes with 96 scenarios, too.
- The start of the program, `Init00` to `Init40` with five routines,
  1,478 bytes, an hour and a half to the commit, next to four
  other agents. The translation ran when it was first built.
  - 12 minutes: a program of 17 bytes for a scenario, typed in hex
    behind Home, which goes to the start of the row and not of the
    file: 16 letters ran as code before it. Two derivations waited out
    the five minutes of `STEP_TIME` each, and before that
    `harness/reach.py` had said that the instructions looked for were
    reached, by code that had run astray (see above). At the top of
    `harness/scenarios.py` now.
  - 5 minutes: the table of stars read as two bytes where it is five,
    three of them no characters, and a bug of the original made of
    that, in the C and in three files of notes. The `port` test passed
    with it. It showed when the setup file of a copy started on the
    defaults was held against the original's, to see how they differ:
    by those three bytes and nothing else. In `CLAUDE.md` now, and the
    scenario ends with that file on its disk.
  - 3 minutes: the flag for a setup file that is not correct was set
    for every file that could be read, a mistake of the C. The two
    scenarios with a second copy said so at once, in the line of the
    banner.
  - 25 minutes: four runs of the `port` test, five to nine minutes
    each next to the tests of others, where it takes under two alone,
    most of it spent writing the notes meanwhile. The
    first failed on three instructions that two earlier batches had
    listed as out of reach for want of a monochrome screen, which
    `start-screen` has; `harness/reach.py` on the routines of the batch
    does not show those of others.
  - 5 minutes to get behind the shared routines, which landed while the
    scenarios were written: one conflict, in the list of scenarios, and
    the calls of DOS written against `port/dos.c` from the start. That
    file calls DOS with ES on the main data, and the start has a block
    of its own to make smaller: `Int21e`, and `TopScr` for an address
    that the BIOS hands back.
  - Most of the time went into finding steps for what the bench was
    said not to have: 40 columns, a monochrome screen, a cursor without
    a shape, too little memory, a setup file that no menu writes. A
    program of ten bytes typed in the program's own editor does each;
    `harness/reach.py --steps --changes` said in seconds whether it had.
  - What saved time: `harness/compare.py` on the three new scenarios
    and two old ones, ten seconds, before every run of the whole test.
  - 10 minutes to get onto `main` behind sixteen commits, the layout of
    the part that stays and the last shared routines among them: six
    files in conflict, each where both sides had added or taken out.
    `TrnPar` had become a word read through SS under the same name, so
    the stub of `Init11` assembled as it was. The table of the keys was
    gone, and with it the 80 bytes that the defaults of a panel took
    from behind their own 8: the panel went to the C file, with zeros
    there (see above). The three goldens came out the same, and the
    `port` test took 78 seconds, alone on the machine.

- What was left of the shared routines, seven of them with the screen
  blanker, 717 bytes, and the entries that called DOS for C code: some
  45 minutes to a translation that passed, with other checkouts at
  work. Every scenario compared as the original the first time it ran
  on the C, twelve of them in 20 seconds and then all.
  - 18 minutes: the `port` test twice, 468 seconds for the base before
    anything was changed and 612 for the batch, where it takes 100
    alone. The base need not have been tested: the batch before had
    just passed on it.
  - 8 minutes: why the port's sky is another than the original's. A
    model of the routine in Python, 40 lines, found the original's
    stars for a seed of 0 and none of the port's for any number of
    turns before: the numbers are never seeded. With that known, no
    step may end in the blanker, and the scenario was written so.
  - 3 minutes: a step of 1,300 movements of the mouse, to wait out the
    minute of the blanker: 530K of script, where the driver counts in
    16 bits. Nothing said so; the run ended with one screen too few.
  - What saved time: `harness/reach.py --steps`, four seconds, said
    which of the blanker's instructions a list of steps reaches before
    a golden was derived; a script took the stubs without a caller out
    of the three files, the `EXTERN` list and `harness/ported.py`, and
    can be run again after a rebase; and the table of keys was turned
    into the `switch` and into the table of labels by a script, so that
    the two agree.
  - The sky by hand, 5 minutes of machine time and 15 of writing: the
    model, a step that has the driver wait for the mouse to be asked
    255 times, made of the driver's event 05 by a wrapper around
    `driver.events` that is not in the repository, and a build of the
    port with `Rand1` and `Rand2` set as `Blank` begins, 5BA1h and
    6E84h, which is what the original has there when the blanker is
    the first thing a scenario does. 151 screens of the original over
    674 turns and 151 of that build over 776 were each the sky that
    the model has after some turn, in order: stars that come out,
    flare up four times or go, a sky with no room for another, and a
    star that would come out where one is.

  - 12 minutes to get onto `main`, behind Enter with the commands of
    the viewer and the editor, twelve commits of the mouse and the new
    layout of the part that stays: nine files in conflict. The five of
    the assembly were taken as `main` had them and the batch's changes
    made again by the scripts that had made them, which took three
    minutes; the script for the stubs then found 57 without a caller
    where it had found 51. What git did not show: the 14 instructions
    of `Input0` that `mouse-keys` had taken off its list were the ones
    this batch had taken off; `F34_30` was C, so the `LODSW` moved into
    it was gone with it; and the table of keys had to be held against
    the `switch` again, by a script, 72 entries the same. The two
    goldens came out as before on the new bench, and the `port` test
    passed the first time, 150 seconds.

## Ideas put off

- Keeping the booted bench from one run of a tool to the next. Booting DOS
  for the two countries and making the bench's setup file take about five
  seconds of every run. What is kept would have to be thrown away whenever
  the harness, a download or QEMU changes.
- A file or a directory that a scenario makes gets the time of day, to
  the second, and the golden lists it; deriving is slower than testing, so
  the two differ. No scenario makes one yet. Such a time stamp could be
  recorded only as made during the run.
