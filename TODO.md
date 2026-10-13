# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

## The setup program in the portable C: the plan

Step 0 of the road (`CLAUDE.md`, "The portable C"): the setup program,
`VCSETUP`, which exists only as assembly, `src/asm/VCSETUP.ASM` with
`VCSTSUB.INC` and `VCSTHLP.INC`, goes into `src/portable_c/` as a part
of the one program, reached by an item of its Options menu and judged
by scenarios whose goldens are recorded from the original
`VCSETUP.COM`. This is the plan for it, made before any of it was
translated, with the parts that could fail tried on the bench. Where
something below was seen on the bench it says so; the rest is from
reading the assembly.

Piece 0a, the harness, is done: the harness judges the setup program,
and `setup` is its first scenario. "What 0a made of the harness" says
what the other pieces can rely on, and the top of
`harness/scenarios.py` how a scenario of the setup program is written,
derived and declared. Of the trial's scripts the three that hold the
setup program's assembly against the main program's are kept, in
`harness/vcsetup/`; what the others showed the harness does itself.

Seven things were the user's to decide. They are decided, and stand
first.

### Decided, on the user's word of 12 and 13 October 2026

| | What | Decided |
|---|---|---|
| 1 | The item of the Options menu | ` More setup...`, the last, behind ` Save setup` and a line across the menu of its own, chosen by M, which is free since Memory allocation went; F1 on it shows the index of the help, which has no page for it. No item moves for it, and the box is two rows taller. This is the user's word of 13 October 2026: on the 12th it was ` seTup...` behind ` Editors...`, chosen by T, and was made so first, with every item below it a place further down |
| 2 | What is on the screen behind setup | What DOS has shown, with the panels off, as when the original was run from the program's command line. So whole screens are held against the original's goldens |
| 3 | What setup edits | The setup file, as the original: it reads `VC.INI` when it is entered and shows that, and the program takes what was saved when setup is left |
| 4 | The three settings of what the portable C has no more, Memory allocation, TSR manager and Screen output | Shown, doing nothing, while the dialogs were translated and held against the original's goldens, and taken out by piece E2, which is done. "A feature that only made sense on DOS is removed, not kept as a setting that does nothing" waited for that piece here: taken out from the start, the Configuration and Screen and Mouse would never have been held against the original |
| 5 | The limit of the file's size, three times the original `VC.COM`'s, which the scenarios' test holds a build to | Dropped for the portable C: the test no longer holds it to a multiple of the original's size, and piece 0a has taken the check out for that tree. It stays for the C translation, which is frozen. The compiler is asked for small code already, `-os`; builds for release and for debugging are a matter for the 32-bit program. The two other limits, of the program's data and of where a code segment ends, are the machine's and stay: see "What could go wrong" |
| 6 | `asked` and `handed-on`, once the program's own call of DOS is gone with piece E1 | Not run for the portable C, which has nothing left for them to show; they stay for the C translation |
| 7 | A critical error of DOS while setup is open, as when the setup file is on a drive that is not ready | The program's own box, since its handler of INT 24h is in place then; the original's setup program has none and leaves the error to DOS's own words. And setup asks for its files as the rest of the program does, through the functions of `sys.h` that ask with the program's errors: none with Bare in its name is added for it. No scenario shows either |

2, 3 and 4 were decided as one thing: the setup program is translated
as it is first, and what is to be otherwise comes afterwards, in steps
that are declared. The panels left behind setup, and setup editing what
the running program has in place of the file, may be such steps later,
and are asked then.

### What the setup program is

**When it starts.** In this order:

- A DOS before 3.20: "This program requires DOS 3.20 or later." Not 64
  KB of memory: "Program too big to fit in memory". Either ends it.
- A screen that is not text of 80 columns is set to mode 3. Then the
  banner, through DOS (AH 09h): the name with "Setup", the version, the
  copyright.
- Its directory: from `VC=` of the environment, or else the path of its
  own file.
- `RdHelp`: its seven pages of `VC.HLP`, read into the memory in one
  piece. If that fails: "Error reading data from the help file.",
  through DOS, and it goes on.
- `VC.INI`, read into its own data: `IniStat` is 0 for no file, 1 for
  one that is as long as it has to be, begins with `VVV` and has the
  right sum, 2 for any other. Without a good file, `Default`.
- `SrcIni`: a copy of the common part of the setup as it is then, to
  tell at the end whether anything has changed.
- BREAK off, INT 1Bh and 23h to an `IRET`, the country and the table of
  small letters, the mode and the page of the screen, the cursor's shape
  and row, EGA, the lines, up to 50, TopView, DESQview, `SetSnow`, the
  screen saved, `UserScr`, and the mouse.

The published source does nothing with the command line. The binary
that was shipped has 275 bytes there that the source has not (`ShwCode`
between `Err3` and `Main00`: a switch, a dialog for registering and its
check): a scenario gives the program no argument.

**What a user sees and does.** S1 is where everything comes back to,
with the cursor on the item that was chosen last (`MnItem`); S9 comes
up from anywhere that waits for a key; S10 ends the program.

| | Screen | Keys, and what follows |
|---|---|---|
| S0 | Only without a good setup file. Box " Error ": "The setup file" and its path, then "not found." or "contains data that is not correct."; Ok, Exit | Ok goes on to S1 with the defaults; Exit and Esc leave |
| S1 | The menu " Setup ": F2 Configuration, F3 Screen and Mouse, F4 Panel options, F5 Set Black & White palette, F6 Set Color palette, F7 Set Laptop palette, F8 Set default configuration, F9 Country info, F10 Exit. The key bar: Help, Config, Screen, Panels, B&W, Color, Laptop, Reset, Cntry, Quit | Up, Down, Home and PgUp, End and PgDn; Enter; F2 to F9 at once; Esc and F10 are Exit; F1 is the help of the item under the cursor. The mouse: a click on a line puts the cursor there and a second one is Enter, with either button; the middle button, and the left one let go outside the box, are Exit |
| S2 | F2, " Configuration ": 18 boxes to tick. General options: Auto menus, Path prompt, Key bar, Menu bar visible, Full screen, Clock, Confirmation on quit, Error sound, Auto save setup, Show logical drive B:. Execute options: Memory allocation, Quick execute commands, TSR manager. Keyboard options: Alt alone selects menu, Esc toggles screen on/off, Alternative viewer, Alternative editor, Clear keyboard buffer. Ok, Cancel | As any dialog of the program. With Ok, Key bar and Clock hold inside setup at once: the bar goes, and its row is put back from `UserScr`; the clock shows in the corner |
| S3 | F3, " Screen and Mouse ": Screen colors, one of Black & White, Color, Laptop; Screen blank delay, one of 40, 20, 5, 3, 1 minutes and Off; Screen output, one of Synchronized access, Asynchronized access, Auto detect; Zooming boxes; Screen saver chars, a line of 5 characters that is typed into; Left-handed mouse. Ok, Cancel | With Ok: `ColTyp`, `BlDelay` by `Delay_T`, `SnowMod`, and `SetSnow` whichever button it was. The colours of setup itself follow `ColTyp` from the next thing drawn |
| S4 | F4, " Panel options ": Mini status line, Auto directory size, Ins moves down, Unselect during group operation; Auto change directory, Show hidden subdirectories, Auto rescan tree, and Auto change dir time delay, typed, "0.60". Ok, Cancel | Ok has `F04_10` read the delay, seconds and hundredths with the country's decimal separator between: "Illegal format of" "auto change dir time delay." in a box with Ok if it is none, and back to the dialog |
| S5 | F5, F6, F7: " Color Setup " for one of the three palettes. On the right "Mode: Color" or "Monochrome", "Palette:" and its name, a grid of 16 by 16 x in every colour with the cursor on the colour of the item, and below it the colour's name, "White" "on Black"; for the last item "No shadow", "Inv brightness" or a colour. On the left a window with 39 items, Panels to Shadow | Up, Down, Home, End, PgUp, PgDn; Enter is S6 for the item; F8 asks "Do you wish to set default colors" "for current palette?", Yes, No; Esc and F10 go back to S1. The mouse: a click on a line, a second one is Enter, a button held above or below the lines moves them, the middle button and a click outside leave |
| S6 | The colour of one item: the left 58 columns show one of four pictures of the program, panels with boxes, the menus over the stars, the editor with boxes of errors, the help, in the palette that is edited, and are drawn again in the new colour as the cursor moves over the grid | Left, Right, Up, Down, Home, End, PgUp, PgDn move in the grid; F8 is the default colour; Enter takes the colour, Esc and F10 do not; both go back to S5. The mouse: the left button on the grid picks, the right one takes, the middle one leaves |
| S7 | F8: " Setup ", "Do you wish to reset all options" "to default value?", Yes, No | Yes is `Default`: every colour, every option and both panels |
| S8 | F9, " Country info ": how the country writes the time, the date and numbers, and the letters beyond ASCII with their capitals and their small letters. Ok | Any way out goes back to S1 |
| S9 | F1: a window " Help " with the title of a page, its text and [ Ok ]; or the box " Help ", "Help contents is not available", Ok, with a beep | Up, Down, PgUp, PgDn, Home, End; Enter, Esc and F10 close. The mouse: a button in the upper third of the text or the lower moves it, the middle one and a click outside close |
| S10 | F10, Esc or Exit, if anything has changed: " Setup ", "Do you wish to save" "the current setup?", Save, Cancel. Unchanged, it leaves without asking | Save writes the file. Cancel and Esc leave too, without saving: nothing goes back to S1 |

**What it reads and writes.** `VC.INI`, in its directory, 511 bytes:
`VVV`; what each of two panels keeps, 88 bytes; the common part, 330
bytes, which is 240 of colours, six palettes of 40, then 32 bytes of
options from `FulScr` to `DriveB`, `DelayTm`, the 5 characters of
`StarTab` and 51 reserved; and the sum of all that as a word. That is
the file of the main program byte for byte, `MakeIni` and `INI_LEN` of
`src/portable_c/setup.c`. The one thing that is otherwise is whose
panels are in it: the main program writes those that it shows, the
setup program those that it read from the file, or the defaults. It
writes with DOS called by itself, AH 43h for the attributes, which it
keeps, 3Ch, 40h, 3Eh.

What only the setup program sets, of all that the file holds: Error
sound, Show logical drive B:, TSR manager, Screen output, the
characters of the screen saver, Ins moves down, Unselect during group
operation, Show hidden subdirectories, Auto rescan tree, the delay of
the tree, every colour, and all of it back to the defaults. The other
options the main program has in its menu or in its two dialogs.

**What it does with a copy of the main program that is running.** One
call, when it has been left with Save, also if the file could not be
written: INT 21h with AX 2B02h, CX 'VC', DX 'OM', which a running copy
answers with 2A00h in AX and the address of its common setup in ES:BX
(`Int21` of `src/c/resident.c`, and of the portable C's until piece E1
took the call out; the original answers only with its TSR manager on,
which is how it is distributed). It then takes
`Active`, `Ctrl_O` and `ChkTSR` from the running copy into its own
data, puts the running copy's `LoadFl` into its own and its own into
the running copy's `LoadRes`, and copies its 330 bytes over the running
copy's. So the running copy has every colour and option as saved, but
which panel is the current one, whether the other is switched off, and
the two things of the memory, which are as they were until it is
started again. Its panels are not touched. With no copy running nothing
answers, and nothing more is done.

**When it leaves.** INT 1Bh back, `UserScr` back on the screen, the
mouse pointer off, the cursor in its shape on the row it was on, BREAK
as it was. Then the file, if Save was chosen; "There was an error while
saving the setup file", through DOS, if that failed; the call above;
and DOS's call to end, with 0.

### What the portable C already has

The routines were held against the main program's one by one, by the
text of the assembly (`harness/vcsetup/procs.py`, which also prints
how a routine differs), and counted in
the listing (`classes.py` there): 4,861 instructions, 10,423 bytes of
code without the room reserved for what the binary has besides.

| Class | Routines | Instructions | In the portable C |
|---|---|---|---|
| The main program's word for word | 68 | 1,745 | All of them, by the batches of `harness/ported.py`, in `bars.c`, `blank.c`, `box.c`, `dialog.c`, `digits.c`, `format.c`, `help.c`, `input.c`, `line.c`, `main.c`, `mouse.c`, `screen.c`, `string.c`, `system.c` and `window.c`; `Window` and `MovScr` without the wait for snow |
| Another text, the same behaviour | `Input0` and `Blank`, which test what `GetMous` has tested already; `ColAddr`, written otherwise; `Shadow`, with the mixing of the colour a routine of its own, `ShadCol`; `LeaveTm`, without the call for Windows, which the portable C has not either | 525 | There. `ShadCol` has to come out of `Shadow` of `screen.c`: S6 calls it by itself |
| Another text, and it matters | `Dialog`, `PutKey0`, `SeekDsk`, `ReadDsk`, `Help` | 775 | See below |
| The setup program's alone | 16 procedures, 454, and the code between procedures, which is most of `VCSETUP.ASM`, 1,362 | 1,816 | Not there |

The five that matter:

- `Dialog`, 394 instructions, 61 lines otherwise. The setup program's
  draws no menu bar: it never calls `PutMnu`, whatever `bMenu` of the
  box is, and five of its boxes have 4 there. It has no flag 4, which
  in the main program moves a box down by `SftErr`, though three of its
  boxes have the bit set; the flags 1, 2, 8 and 10h are the main
  program's. A line that is typed into is kept at `TmpErr`, 64
  bytes for each by the high byte of `mCtrl`, which is 0 in both of its
  lines, where the main program has it behind the command line, at
  `DosBuf+LenCmd+10`; `F04_10` reads the text at `TmpErr+3`. It has no
  `NormStr` for such a line. And F10 and Alt-F10 with flag 8 answer the
  other way round, which no box of the setup program has. One `Dialog`
  can do for both, told that setup is open.
- `PutKey0`, 65 instructions. Its own bars, `KeyB_T`, `KeyC_T` and
  `Key_S`: bar 0 is the menu's, with a name for every key; 1 a
  dialog's and S6's, Help and Quit; 2 that of S5 and of the help, with
  Reset for F8 as well; 3 Quit alone. A bar is the same with Shift,
  Ctrl or Alt held. And it stores
  the space behind a name before it looks whether that was the last, so
  it writes 81 cells where the main program's writes 80: see "The bugs
  of the original that stay".
- `SeekDsk` and `ReadDsk` call DOS themselves, where the main program's
  go through `Intr21`. So do all its calls for files. `sys.h` has a
  file opened, closed and read both ways, `FileOpenBare`,
  `FileCloseBare`, `FileReadBare`; a place in a file, a file's
  attributes, a file made and one written are there only as asked with
  the program's errors. Setup asks as the rest of the program does,
  with the program's errors, through the functions that there are (see
  7 above), and none is added for it: where a drive is not ready the
  program's box comes up, a call that was given up there is the error
  53h, and DOS is asked what was wrong behind one that failed. That is
  declared.
- `Help`, 297 instructions, is another viewer than the main program's:
  no index, no buttons but Ok, the pages all in the memory, read once by
  `RdHelp`. The pages are those from the 55th of `VC.HLP` on, seven of
  them (`BegHlp`, `NumHlp`): 1 Configuration, 2 Screen and Mouse, 3
  Panel options, 4 the palettes, 5 Set default configuration, 6 Country
  info, 7 Exit. `MainHlp` has the page of each item of S1, and each box
  its own in `bHelp`.

Of the 1,816 instructions that are the setup program's alone, some 280
are of its start and have no place in the one program, which has done
all that when it started, and `SetSnow`, 25, goes with the snow. What
is to be written is some 1,900 instructions, where the main program has
22,668:

| What | Instructions |
|---|---|
| Of the start: the setup file read and checked, `SrcIni`, the box S0 | about 70 |
| S1: `Main00`, `Main40`, and the mouse, `Main50` | 84, 20, 74 |
| S2, S3, S4 with `F04_10` | 22, 33, 23 and 70 |
| S5 with `F07_60` to `F07_85` | 401 and 114 |
| S6, `Col00` | 289 |
| S7 with `Default` | 5 and 19 |
| S8 with `F09_10`, `F09_20` | 64 and 12 |
| S10 and what follows, `Exit` | 103 |
| S9: `RdHelp`, `Help`, `HlpLine` | 86, 297, 25 |
| `PutKey0` with its bars, and what `Dialog` does otherwise | 65, about 30 |

**The data.** 13,164 bytes in the source (`harness/vcsetup/data.py`
has the names). 36 objects have the text of one of the main program's,
706 bytes, the default palettes among them, `Palets`. 34 have the name
of one of the main program's and other contents: `Conf_P`, `Conf_B`,
`Cnf01_S` to `Cnf14_S`, `Key_S`, `KeyB_T`, `KeyC_T`, `DefInit`,
`DefWCB`, `Menu_T`, `Help_T`, `HlpEr_P`, `HlpEr_B`, `Ok_S`, `SvSt_P`,
`SvSt_B` and others. 111 are the setup program's alone. About 12.2 KB
are new tables: the four pictures of S6, `Page1_S` to `Page4_S`, 1,450
bytes each, with characters that are no ASCII and some below 20h; their
colours, `Colr1_T` to `Colr4_T`, 990 bytes; the names of the 39 items,
`Colrs_S`, 829; the lines of five dialogs. Five tables of keys,
`Main_T`, `Menu_T`, `Color_T`, `ColSt_T` and `Help_T`, become `switch`
statements.
`NcParm` is data that no code of the source reads.

The variables that only the setup program has: `IniStat`, `MnItem`,
`CurType`, `SrcIni`, 330 bytes, `ColCur`, `ColFrst`, `ColNum`, `BackC`,
`HistC`, `HstCuC`. Its buffers against the main data's: `WinBuf` is
there; `BoxBuf` is not, and `ErrBuf` is as long; `UserScr` has 50 rows
and the main data's 49; `HelpBuf` is all that is left of its segment,
8,155 bytes of pages at the most, and the main data's is 3,000.

### How it fits into the one program

The two programs of the original become this, with no screen otherwise
than the original's but for what "The deviations to declare" lists:

- **Entered**, from the item of the menu. The screen is made what the
  program shows just before it runs a command: what DOS has shown, the
  panels and the bars gone, with the row of the command line empty,
  and the cursor on the next row, which is what Enter behind a command
  leaves. The common part of the setup as the program
  has it, 330 bytes of the main data from `ColTab` on, is set aside.
  `VC.INI` is read as the setup program reads it: its common part into
  the main data, its two panels into room of setup's own; without a
  good file the defaults, and S0. The main data is then what the setup
  program has in its own segment: every routine that both programs
  have reads the colours, the key bar, the clock and the hand of the
  mouse there, so what is edited holds inside setup at once, as in the
  original. `SrcIni` is the copy to hold the end against.
- **Left without Save**: what was set aside goes back, all of it.
- **Left with Save**: the file is written, with the two panels as the
  file had them or as `Default` made them. The main data keeps what was
  edited, but `Active`, `Ctrl_O` and `ChkTSR`, which are put back from
  what was set aside, and `LoadFl`, which is put back too, with what
  was edited in `LoadRes`. That is what the call of DOS did to a
  running copy, and it is done as the original does it: `LoadFl`
  becomes what `LoadRes` is when the setup has been left, as it does
  behind every command (`Exec10` of `resident.c`), so the byte that the
  program later saves is the one that was edited, as the original's.
  The portable C does not go by it.
- **Either way** it goes on as when a command has ended: the place
  `NEXT_INIT11` of the main loop, by which `Round` of `main.c` calls
  `IniData` again. That finds out about the screen and the mouse anew,
  reads the panels again with the cursor and what was selected found by
  name, and has no setup known as saved (`IniSave`), so that Auto save
  setup does not write the file again at once. The original does all
  that because setup was a command.

Told that setup is open, by a variable of setup's that three places
read, `StOpen`: `PutKey0` takes the setup's bars, `Dialog` leaves the
menu bar alone and keeps a typed line in `TmpErr`, and `Input` has F1
call setup's help. The boxes then hold the bytes that the original's
have, and the data's test holds them so. That is all that is switched,
on the user's word of 13 October 2026 that the aim is one program whose
setup uses the main program's routines: where the setup program's copy
of a routine differs in what a scenario of setup shows and that is no
bug, the switch stays; where it differs only by a bug, the bug is asked
about and not carried by a switch, as `PutKey0`'s 81st cell, which is
fixed (entry 9 of `DEVIATIONS.md`); and where it differs in what
nothing of setup can show, the main program's holds for both. Of that
last kind are four things of `Dialog`, which the comment at the routine
lists: the flag 4, the place of a box that is on the screen already,
`NormStr`, and the two answers F10 and Alt-F10. A
fourth place clears it for a moment: the box of DOS's critical error,
`FtErr1` of `main.c`, which is the main program's also while setup is
open.

Where setup keeps what it has of its own: in the main data, in
buffers that the main program does not need while setup is open,
since it makes them anew when a command has ended. `DGROUP` has seven
bytes more: the one of `StOpen`, and six of `stcolor.c`, the box and
the palette of the list of colours, which the assembly has in three
registers.

- `struct StData` of `vc.h`, in `MenuBuf`, which a file of setup has
  as `sd`: the setup file as it is read and written, `IniStat`,
  `MnItem`, `CurType`, `SrcIni`, the variables of the palettes, and two
  things that the original has not, what is set aside of the program's
  setup and the 50th row of what DOS had shown. It is a third of
  `MenuBuf`.
- `UserScr` is the main data's, 49 rows, and the 50th is in `StData`:
  `StUser(first, rows)` of `stmain.c` puts rows of it into the screen
  buffer, which is all that a piece needs of it.
- `BoxBuf` is `ErrBuf`; `WinBuf`, `TmpBuf`, `TmpErr`, `ScrBuf` and
  `Stars` are the main data's. `ErrBuf1` is left alone: the box of a
  critical error keeps there what it covers.
- `NcPath` is the directory of the program, as the main program has
  it, and has the name of the setup file behind it only while that
  file is read or written and while the box S0 shows it: whatever else
  looks for a file of the program while setup is open, the help file,
  finds the directory there (`MainFil` of `string.c`).
- The pages of the help, up to 8,155 bytes, are in `TreeBuf` of the
  main data (`StHelpBuf` of `sthelp.c`), which is 12.3 KB and free
  while setup is open, since a panel that shows
  the tree reads it anew when setup has been left, as after a command
  (`ReDir` of `read.c`, with `ChgFlg` set by `IniData`); and so is
  `InfBuf`, 1.4 KB. `HelpBuf` is the main program's help, 3,000 bytes,
  with what that has read in it: whatever uses it has to leave the
  main help knowing that it has nothing. The memory behind the panels,
  where the viewer has its text, is free too, and is reached by far
  pointers.

### Names and files

Every routine and every object of the setup program has `St` in front
of the name that it has in the assembly, `StMain00`, `StF02_00`,
`StConf_B`, `StHelp`: `F02_00` to `F09_00`, `Exit`, `Init20`, `CtrlC`
and the 34 objects above are the main program's already. The harness
then knows `StX` for `X` of `build/shareware-layout/VCSETUP.LST` by one
rule. A routine that both programs have word for word keeps its name
and is not written twice.

| File of `src/portable_c/` | Holds |
|---|---|
| `stmain.c`, `stmain_t.c` | Setup entered and left, the setup file read and written, S0, S1 with the mouse, S7 with `Default`, S10 |
| `stopts.c`, `stopts_t.c` | S2, S3, S4 with `F04_10`, S8 |
| `stcolor.c`, `stcolor_t.c` | S5 and S6 |
| `sthelp.c`, `sthelp_t.c` | S9 |

`setup.c` is the setup file of the main program, and stays that. New
files go into the second code segment, by their names in
`add_portable_c` of `CMakeLists.txt`.

### The pieces

Eight, of which the first two land alone, one behind the other. A, B
and C are made side by side, each in a clone of its own with the files
that the table gives it. Each piece writes its scenarios, derives their
goldens from the original, and lists its routines in the harness as a
batch, with what no scenario reaches.

| Piece | Owns | Size | Scenarios | Needs |
|---|---|---|---|---|
| 0a, the harness. Done | `harness/`: what the list below has; `tools/figures.py` and the README for the limit that goes. No file of the program | Some 800 lines with what is written for the pieces, where 200 were thought | One, `setup`: the setup program started, its menu, the Configuration with the key bar switched off, Screen and Mouse looked at, the setup saved and left, and the main program without its key bar | Nothing |
| 0b, setup entered and left. Done | `stmain.c`, `stmain_t.c`; the item of the menu in `menu.c`, `commands_t.c`, `main.c`, `vc.h`, with its tables declared in `harness/datacheck.py`; the three places that are told of setup, in `bars.c` with the bars' tables, `dialog.c` and `input.c`; entry 8 of `src/portable_c/DEVIATIONS.md`. Of the assembly: the file read and checked, S0, `Main00`, `Main40`, S7 with `Default`, `Exit` | 776 instructions carried over, of 934 in its routines, all of `Dialog` among them | 5: `setup-menu`, `setup-defaults`, `setup-missing`, `setup-damaged`, `setup-cancel`, 98 steps, which run 558 of the 776; the scenarios of A and B run 196 more, the fields of `Dialog` and the mouse's rows on 50 lines; 22 are listed as out of reach, 13 of them until a later piece has its scenario | 0a |
| A, the options. Done | `stopts.c`, `stopts_t.c`: S2, S3, S4, `F04_10`, S8 | 224 instructions, 3.5 KB of tables | Eight, 320 steps, see "Piece A" below. As planned: each box of S2 by the keys and by the mouse, the key bar switched off and on; S3 with its three groups and the line of characters; S4 with a delay of every form that `F04_10` tells apart, good and bad; S8; the last two on the Russian bench as well, for the decimal comma and the Cyrillic letters | 0b |
| B, the colours. Done | `stcolor.c`, `stcolor_t.c`: S5, `F07_60` to `F07_85`, S6; and `ShadCol`, taken out of `Shadow` of `screen.c`, which 0b left as it is. One agent, the list first and then the picture: each jumps into the other | 804 instructions, 8 KB of tables | About 6, some 250 steps: the list by its keys; every one of the 39 items entered once, which is the one way that `ColPage` shows; the grid by its keys, with F8; the item Shadow; the default colours asked for, Yes and No; the mouse in both; each of the three palettes; the key bar off | 0b |
| C, the help. Done | `sthelp.c`, `sthelp_t.c`: `RdHelp`, `Help`, `HlpLine` | 408 instructions | About 3: a long page by its keys; every page from its place, five that show and two that do not; the mouse; a help file that is not there | 0b |
| D, what is left of S1 and of leaving. Done | In `stmain.c`, after 0b: `Main50`, with the six instructions of `Main30` that call it; and scenarios for what crosses the pieces | 80 instructions, 74 of them the main program's routine | Six, 100 steps, see "Piece D" below: the mouse at S1; a setup file that cannot be written; a screen of 50 lines; the clock switched on inside setup; the left-handed mouse; and F1 from the other pieces' screens. A setup saved whose file is missing is A's, `setup-missing-saved` | 0b; A, B and C for three of the scenarios |
| E1, the program's own call of DOS goes. Done | `resident.c`: `New21`, `Int21`, `Vect21`, INT 21h taken and given back; the README, `DEVIATIONS.md`, the items of this file that speak of the call | Small | None new. `asked` and `handed-on` are not run for the portable C; the patch of `ended` is written again and read, `goldens.py patch --tree portable_c ended`, for the registers that the program leaves. See "Piece E1" below | A to D |
| E2, three settings go. Done | `stopts.c`, `stopts_t.c`: Memory allocation and TSR manager out of S2, Screen output out of S3; entries 1, 5 and 7 of `DEVIATIONS.md`, which said that the setup program still shows them; the two dialogs and their lines declared in `harness/datacheck.py` | The two dialogs and 23 lines of their boxes, and `StF03_00`: three fields gone, and seven lines of the right halves | None new. Ten scenarios that show the two dialogs have patches of the original's goldens, and six of them other steps where a line has moved, see "Piece E2" below | A |

While 0b is made, A, B and C can write their scenarios and derive the
goldens, which needs the original alone. A scenario that the portable C
cannot pass yet is declared not run for it, in `not_run` of
`TREES["portable_c"]` of `harness/deviations.py`, by its name, and
taken out of there by the commit that makes it pass. Nothing is
declared for the C translation: it is taken through no scenario of the
setup program, by their kind.

In all, some 22 scenarios and 600 steps were planned, where the main
program has 134 and 3,358 for twelve times the code: nearly every
instruction here is a key or a button of the mouse, and little of it is
something that other scenarios pass through anyway. With 0a to D done
there are 33 of 889 steps, and none waits in `not_run`. E1 has taken
the program's own call out since, and E2 the three settings: the
setup program is in the portable C whole.

**What 0a made of the harness.** The nine things that it was to
change, each as it is now, and what came out otherwise than planned.

1. `setups.py`: for a scenario that says `setup=True` the original has
   the distribution's `VCSETUP.COM` in D:\VC, written behind the files
   that every disk has (`setups.beside`, `Bench.start`). No other
   machine has it, and every golden that there was came out the same
   when all were derived again.
2. `scenarios.py`: `Scenario` has `setup`, and `setup_clock` for the
   sixth of these. A traced run of the original in such a scenario
   records what ran of `VCSETUP.COM`, and nothing of `VC.COM`.
3. `coverage.executed` takes `others`, the files of the other programs
   of the machine: see "What ran" below. It is given them where two
   programs run, in the scenarios of the setup program, and nowhere
   else.
4. `goldens.py`: the `.ran` of a scenario of the setup program is of
   `VCSETUP.COM`. It has the same name as any other, and `goldens.ran`
   keeps the two kinds apart: of the main program's scenarios for the
   batches of the main program, of the setup program's for its own.
   Nothing else reads the lists of all scenarios. Both files of the
   original are "the program under test" in the list of the disks.
   And `goldens.py derive` takes the scenarios side by side now: all
   of them are derived again in a minute and a half.
5. `deviations.py`: the line of the command is in `DEVIATIONS`, as
   spaces in a build; `Declared` has `in_place_of`, by which the
   portable C is taken through `"F9 o m"` where the original has the
   command, `SETUP_ITEM` and `SETUP_COMMAND` there, in every scenario;
   and `no_setup`, by which the C translation is taken through no
   scenario of the setup program, whichever there are. That is
   otherwise than planned: they are not listed for it by name, so a
   piece that adds one changes nothing for that tree, whose test it
   does not run. For the portable C each is listed in `not_run` until
   its code is there.
6. `driver.py`: `setup_facts`, for a scenario that says
   `setup_clock=True`. `DRIVER.ASM` has not changed. Seen on the bench
   with the harness: the colon of the setup program's clock is recorded
   as blinking with it, and not without, the same twice. In such a
   scenario the original's main program is not watched: see the top of
   `harness/scenarios.py` for what the scenario may then not do.
7. `ported.py`: `SETUP_BATCHES`, held against `VCSETUP.LST` and against
   what ran of `VCSETUP.COM`, for a tree that has the setup program; a
   batch has `not_carried` besides `unreached`, for the instructions of
   a routine that the one program has no place for. The list is empty,
   with the form of a batch written out in front of it; the mechanism
   was tried with a batch that is not kept. A routine that is the main
   program's word for word is not listed there. `reach.py --setup` and
   `show.py --setup` are there, and `harness/vcsetup/classes.py` counts
   what has run by class.
8. `datacheck.py`: a second original, `VCSETUP.COM` with its listing and
   with `VCSETUP.OBJ`, which the build assembles now. It was less than
   feared, and is done. An object `StX` of the portable C is held
   against `X` of the setup program, if that program has data of that
   name; no data of the main program has such a name. Four things that
   the plan did not have:
   - A word of the setup program that names a text or a table may hold
     where the tree has `StX`, or else where it has the main program's
     `X`, if that object is, as long as the tree has it, what the setup
     program has there: 23 objects are the same bytes in both programs,
     `Help_S`, `OkMn_D`, `SvSt0_S` to `SvSt3_S`, `Palets` among them,
     and need not be written twice.
   - A word that names a variable of the setup program, `DW WinBuf`,
     the field that a box ticks, is an address in that program's one
     segment. The tree's word is held against where it has a variable
     `StX`, or else against the place of the field `X` in the main
     data, as the main program's listing has the structure: `MD_AT(X)`
     of `tables.h`.
   - A variable that the main data has under another name wants an
     entry in `SETUP_NAMED_OTHERWISE`, with "field": the setup
     program's `BoxBuf`, which the tree has in `ErrBuf`.
   - The routine that a dialog names, `F04_10`, is a number there, as
     in the main program: "is", in the same table.
   The table was `NAMED_OTHERWISE` when 0a wrote this, and piece B gave
   the setup program's words one of their own: an entry there for an
   object of setup failed the C translation's test, which has no such
   object. Piece B also added `SETUP_COUNTED`, for a word that names a
   text without a zero byte which the code reads by a count: the four
   pictures of the colours, through `PagTab`.
   All of it was tried on a copy of the tree with a box and a dialog of
   the setup program written into it, right and with three things
   wrong. `reads.py` needed nothing: it goes by the names that the C
   has, and a function of setup's that reads a box by its offset goes
   into `READERS` there.
9. `ported.py`: the portable C is held to no multiple of the original's
   size, and the C translation to three times, `SIZE_LIMIT`; the row is
   gone from the README's figures.

**What 0b made, and what A, B, C and D go by.** `stmain.c` says at its
top how the setup is a part of the program, and `vc.h` has a section
for it, "The setup program": read both first.

- **The menu calls a piece's screens** as functions that return when
  the screen has been left, after which the menu is drawn again with
  the cursor on that item: `StF02_00`, `StF03_00`, `StF04_00` and
  `StF09_00` of `stopts.c`, `StF05_00`, `StF06_00` and `StF07_00` of
  `stcolor.c`, each the label of the original with `St` in front, and
  none takes anything: the original's code reads no register of the
  menu's there. `Input` calls `StHelp` of `sthelp.c` for F1, with the
  page in `HlpPage`. All eight are declared in `vc.h`.
- **Until a piece's file was there**, its functions were in
  `stmain.c`, under "What is still to come": an item waited for a key
  and returned, and the help did nothing. A piece added its file, put
  the file's name into the list of `add_portable_c` in
  `CMakeLists.txt`, which is the second code segment, and took its
  functions out of that block of `stmain.c`. A, B and C have done so,
  and the block is gone with the last of them. `setup-menu` has every
  item entered and left at once, "F2 Esc": it passed with the
  place-holders and passes with the screens.
- **A routine that a dialog names**, `F04_10`, is a number in the
  dialog's header: a `SUB_` more in `tables.h`, and its case in the
  `switch` of `Dialog`, which calls the function. The test of the data
  is told the number in `NAMED_OTHERWISE`, with "is".
- **Tables** are in a file of the piece's own, `stopts_t.c`, with `St`
  in front of every name, or the main program's own where that is the
  same bytes, as the test of the data allows. `stmain_t.c` has one that
  others may name, not static for that: `StPalets`, the 330 bytes of
  `Palets` and `DefInit`, which the palettes read for a default colour.
  A constant that only code reads is `static const u8 CODE` in the
  file of the code, as the main program has them.
- **What setup has in the memory** is under "How it fits" above:
  `sd`, `StUser`, and where the pages of the help can go.
- **The main program's routines are used as they are.** Of those that
  the setup program has otherwise, `PutKey0`, `Dialog` and `Input` go
  by `StOpen`. `Input0`, `Blank`, `LeaveTm`, `ColAddr` and `Shadow`
  are the main program's; `ShadCol`, which S6 calls by itself, is B's
  to take out of `Shadow`.
- **A batch** names its routines in `SETUP_BATCHES` of
  `harness/ported.py`. 0b's has the start of the program from `Init15`
  on, where most of what a program of its own does when it starts is
  listed as not carried over, and all of `Dialog`, whose fields A's
  scenarios run.
- **What 0b's batch lists as out of reach until a later piece has its
  scenario**, each with the address in `harness/ported.py`: the test
  fails on the entry once a scenario runs it, and the piece then takes
  it out.
  - D: nothing any more. `Main30`, six instructions that call
    `Main50`, is translated and run by `setup-mouse`; and `Exit5`,
    three, the line that DOS writes for a setup file that could not be
    written, by `setup-save-fails`.
  - A: nothing any more. Three of `Exit` were listed, for a setup
    saved whose file is not there, which takes a setting changed
    without a file: `setup-missing-saved` runs them in the original.
  - C: nothing, and C is done. The beep of a box that has the flag
    for it, in `Dialog`, which the box "Help contents is not
    available" alone has, runs in `setup-help-pages` and
    `setup-help-file`. The five instructions that call `RdHelp` when
    the setup program starts are carried over: `StSetup` calls
    `StRdHelp` where the original has the call, in front of `StFile`.
- **Scenarios** may start the setup program more than once and may
  look at what DOS has shown afterwards: the portable C leaves an
  empty row where the original has the line of the command, and moves
  the screen as the original does. The top of `harness/scenarios.py`
  says where such a line gets to and when "cls" is needed.
- **`setup`**, 0a's scenario, was not run for the portable C until the
  Configuration and Screen and Mouse were there, which its steps go
  through: it is run since piece A, and passes. Tried in its place
  before, with steps
  that are no scenario: a setup file saved by the main program with the
  key bar off, the setup entered and left with it, and Ctrl+o, show the
  same in the original and in the portable C, the prompt's row
  included.
- **What leaving resets.** `StSetup` ends with `StOpen` 0, the name of
  the setup file off `NcPath`, and the two interrupts given back; then
  the main loop starts the program over, and `IniData` and `Init40`
  set what a command leaves to be set: `BarPrev`, `Quote`, `HlpPage`,
  the mouse, the screen and its buffers, the panels' lists. A piece
  that uses a buffer of the main program's own has to leave it as that
  program can go on with: for `HelpBuf`, with the main help knowing
  that it has read nothing.

**What a piece does**, in this order:

- Its scenarios, in the form of `setup`, tried with `show.py --setup`
  and `reach.py --setup`, each derived twice and read, and listed in
  `not_run` of `TREES["portable_c"]`: that can land before the code.
- Its code and tables, with `St` in front of every name of the setup
  program's. `python3 harness/datacheck.py build/shareware-layout
  build/portable_c` names every constant of setup with "of VCSETUP",
  and fails for one that it cannot place.
- Its batch in `SETUP_BATCHES` of `harness/ported.py`, and its
  scenarios out of `not_run`, in the commit that brings the code.

### Piece A, the option dialogs: done, and what a later piece should know

The Configuration, Screen and Mouse, the Panel options and the Country
info are there, `stopts.c` with `stopts_t.c`, called from the setup's
menu, and held by their scenarios, with `setup`, 0a's.

**The scenarios.** Eight, 320 steps, each derived twice with the same
result:

| Scenario | Steps | What |
|---|---|---|
| `setup-config` | 60 | S2 by the keys: every box, every key that moves, Ok, Cancel, F10, the key bar off and on, saved |
| `setup-config-mouse` | 38 | S2 by the mouse: every case of the mouse in `Dialog` and `FlipFl` |
| `setup-screen` | 67 | S3: the groups by keys and mouse, `Select` whole, the line of characters, `InpStr` whole, two palettes taken inside setup, saved |
| `setup-panels` | 86 | S4: thirteen delays that are taken, twelve that are refused, the box closed in every way, saved |
| `setup-country` | 23 | S8 in the United States, and every way out |
| `setup-options-russia` | 23 | S8, S4 with the comma and Cyrillic characters in S3, in Russia, saved |
| `setup-delay-file` | 14 | S4 with a delay of 100.100 in the setup file, made in the editor: the two `MOV AL,99` of `F04_00` |
| `setup-missing-saved` | 9 | No setup file, Menu bar visible ticked in S2, saved: the three instructions of `Exit` for a file that is not there, which 0b's batch had listed as out of reach until the options were there |

Of the 224 instructions of the piece they run 221; `SetSnow` called
behind Screen and Mouse, 06FFh, is not carried over, and two of
`F09_00` are out of reach: `XCHG BH,AL` at 0F4Eh, for a country that
writes the year first, and `INC DI` at 0F68h, for one with no character
between thousands; the benches are the United States and Russia. The
batch is in `SETUP_BATCHES` of `harness/ported.py`: routines `F02_00`,
which is the stretch with `F03_00` and `F04_00`, `F04_10`, `F09_00`,
`F09_10` and `F09_20`. Of `Dialog`, which is 0b's and which only these
dialogs take through its fields, 0b's batch says what is out of reach.

**Connecting it changed one place of the code.** The portable C passed
the seven scenarios that were written for it, and `setup`, the first
time it was taken through them: every screen, the colours and what is
behind the boxes included, and the setup files that five of them save.
What was fitted to the skeleton:

- `StF02_00` takes the row of the key bar that has gone from `StUser`
  of `stmain.c`, where it read `md.UserScr` itself: the main data's
  copy of the screen has 49 rows, and the 50th is with the setup's
  data.
- `Dialog` of `dialog.c` has the case for `SUB_StF04_10` in the
  `switch` on the routine that a dialog names, as for `SUB_M30_60`,
  and `vc.h` declares `StF04_10` next to `M30_60`. The line that was
  typed is at `md.TmpErr + 3`, where the skeleton's `Dialog` keeps it
  while the setup is open.
- The four place-holders are out of `stmain.c`, and with them, the
  last, the block that held them and `StToCome`.

**The tables**, `stopts_t.c`: 77 objects, 3,065 bytes, every text
written by a script from the bytes of `VCSETUP.ASM` and each held
against `VCSETUP.COM` by the data's test. What is in it that others
name: `StOkMn_S`, " Ok ", and `StOk_S`, "[ Ok ]", which the help of the
setup has for its buttons too (`HlpEr_P`, and `Help` itself); they
stand in the assembly among the texts of Panel options and of Country
info, so they are defined here, not static, and the help declares them
`extern`. `OkMn_D` is the main program's, of `main_t.c`, the same
bytes. `StDelay_T` is in `stopts.c`, as `Delay_T` is in `commands.c`.
Two words are in `SETUP_NAMED_OTHERWISE` of `harness/datacheck.py`: the
`BoxBuf` of `StNoTim_P`, and `F04_10` in `StPanOp_B`, which is the
number `SUB_StF04_10` of `tables.h`, 5. The five lines that Country
info writes into are objects without `const` there, and the test holds
them like the others.

**The code**, `stopts.c`: `StF02_00`, `StF03_00`, `StF04_00` and
`StF09_00`, each an item of the menu, which take nothing and return
nothing, where the assembly jumps back to `Main00`; `StF04_10`, which
returns `struct Checked` as `M30_60` does; and `StF09_10` and
`StF09_20`, static. It asks nothing of the system.

**A setup made without a file and saved.** `setup-missing-saved` is the
one scenario in which the setup program writes a file of the setup that
it has made itself, and that file is not the original's: the 64 bytes
behind the path of each panel, which are zeros in the portable C (entry
8 of `DEVIATIONS.md`, and "The bugs of the original that stay" below).
The screens are the original's all the same. So the scenario does not
end with that file: Auto save setup is on in such a setup, the program
goes by what was saved at once, and a panel switched off, Ctrl+F2, has
it write its own setup, which has zeros there in the original too. The
file as the setup wrote it is held by no scenario, in the 383 bytes
that are not those 128 either: only by what the program then shows and
does, the menu bar and the saving by itself.

**What the assembly does**, read whole, so that it need not be read
again for the code:

- `F02_00`: notes `KeyBar`, calls `Dialog` with `Conf_B`, and if the
  bar was on and is off now, copies the last row of the screen from
  `UserScr` to `ScrBuf`, writes it out, `Window` with the row and 150h,
  and sets `BarPrev` to FFh. Whichever button it was: only Ok changes
  `KeyBar`.
- `F03_00`: `TmpBuf` 0 and 1 are `ColTyp`, 2 and 3 the first place of
  `Delay_T` whose minutes `BlDelay` is not below, 4 and 5 `SnowMod`;
  with Ok the three are taken back, `BlDelay` through `Delay_T`; then
  `SetSnow` either way, which the portable C has not. The line of
  characters is typed straight into `StarTab`, five bytes with whatever
  the line's buffer had behind a shorter text: "ab" typed over the five
  leaves a, b, 0 and the last two as they were, in the file too.
- `F04_00`: the delay written at `TmpBuf+9`, seconds without a leading
  zero, `Country.DecSep`, two digits of hundredths, each of the two
  numbers 99 at the most.
- `F04_10`, called by `Dialog` for Enter with the button in AL: not 0,
  Cancel, it returns at once. Else the text at `TmpErr+3`: nothing but
  blanks is taken and sets nothing. Seconds, up to 99; then the end, or
  the separator directly behind the digits; then the end, or digits
  directly behind the separator, and nothing but blanks behind those.
  A fraction below 10 is multiplied by 10, and then divided by 10 as
  often as it is not below 100, 10 or 1, for no zero, one, or more in
  front of it: ".5" is 50, ".05" 5, ".005" 0, ".123" 12. `TxtNum`
  answers FFFFh for more than a word holds. Anything else is the box
  `NoTim_B`, `HlpPage` 3 again, and the carry set: back to the dialog.
- `F09_00`: `F09_10` writes the capitals of 40 characters and `F09_20`
  their small letters, from `Inf05_S+4` to `Inf06_S+7` and `Inf07_S+7`,
  and from `Inf09_S+7`, 39 characters and a space, to `Inf10_S+7` and
  `Inf11_S+7`. The time at `Inf02_S+6`: "9", `TimeSep`, "00p", or with
  24 hours "21", `TimeSep`, "00". The date at `Inf02_S+22`: MM, DD and
  YY with `DateSep` between, in the order of `DateFmt`. The number at
  `Inf02_S+39`, a byte further on for a `Sep1000` of 0: "1" and three
  times `Sep1000` and "000". So those five lines are no constants: they
  are objects of the file of tables without `const`, which the code
  writes through far pointers with `AsmSeg`.

**For the other pieces.**

- 0b: `NoTim_P` keeps what it covers in `BoxBuf`, `ErrBuf` in the
  portable C (`NAMED_OTHERWISE`, "field"); `NoTim_B` has its button at
  `TmpErr+240`, behind the line of the dialog under it, which is at
  `TmpErr`; `PanOp_B` names `F04_10`, a `SUB_` number. The line of S3
  has `StarTab` itself for `mPoint`, and `Dialog` writes a zero behind
  what it copies back, into the first reserved byte of the setup.
- C or D: F1 in the box "Illegal format of" does nothing, its page
  being FFh, and F1 behind it shows the page of Panel options again,
  which is what `MOV HlpPage,3` of `F04_10` is for. That is seen on the
  bench, and `setup-delay-help` holds it, with the help of the setup
  in the tree.
- The 81st cell that `PutKey0` writes is decided, on the user's word:
  it is fixed, not kept. The portable C writes 80 cells, one routine
  for the main program and setup, declared as a deviation, which is
  0b's to do. No scenario of this piece shows the corner that the
  cell lands in: it shows only on a screen of 50 lines, in the top
  left of what DOS had shown once setup is left, and none of these
  has more than 25 lines.
- D: no scenario of this piece shows the clock, Clock is ticked and
  not within one step; and none uses the mouse after Left-handed mouse
  has been saved.

### How scenarios are written and held

All of this was seen on the bench, with the original, but the last
paragraph.

- The setup program is started from the command line of the original
  `VC.COM`, which the bench starts as always: the step
  `"d:\\vc\\vcsetup>nul" Enter`. The driver of the tests feeds it and
  writes its screens out as it does for the main program, unchanged:
  the setup program calls INT 28h every time it has gone round waiting
  for a key, and asks the mouse driver as the main program does. The
  script names VC, so the driver does not take the setup program for
  the program under test: it looks at no `AutoTim`, which the setup
  program never sets, at no stack, and at no clock.
- `>nul` sends the banner nowhere. Without it the banner moves what DOS
  has shown up by four lines, and has two texts of the binary that the
  published source has not. With it, what is behind setup is what DOS
  had shown, and one line more: the command as the program wrote it,
  `C:\WORK>d:\vc\vcsetup>nul`, on row 23 with the panels on a full
  screen of 25 lines. No box of setup covers that row but the picture
  of S6, which covers all of the line; S5 clears the columns from 58
  on, where the line is not.
- From DOS, with no copy of the main program in the memory: `F10`,
  `Enter`, and then `"d:\\vc\\vcsetup" Enter` to COMMAND.COM, which the
  driver feeds too.
- A recording of nine screens takes nine seconds, and two of the same
  steps were the same byte for byte, the screens, the disks and what
  ran.
- The clock switched on in S2 shows at once and blinks. For the driver
  to record both states, the script of the original's run has to name
  the setup program: a scenario says `setup_clock=True` for that. The
  portable C is one program, and is watched as always.
- The mouse is as written at the top of `harness/scenarios.py`:
  `Click@30,9` on a line of S1 and again is Enter there, a right click
  in a dialog is Enter, the middle button at S1 is Exit.
- A beep moves the screen up a line, as in the main program, since the
  cursor is hidden below the last row: the box "Help contents is not
  available" does. The line of the command is then on row 22, with the
  key bar that was on the screen in the row below it, and stays so
  when the box is closed; the window of a page of the help, opened
  afterwards, had the screen as it was before the beep again.
- The picture of S6 has a prompt of its own on row 23, `C:\DOS>`, and
  a key bar of the main program below it: they are of the picture.
- No scenario may show the clock for longer than the rule on the time
  of day allows, and none looks at what DOS has shown after setup, or
  at the history: see "The deviations to declare".

A scenario is written with the steps of the original, and is one of
`SCENARIOS` like any other: the top of `harness/scenarios.py` has the
form, with `setup` as the example. For the portable C the step that
starts setup is another, the keys of the item of the menu, by
`in_place_of`; every other step is the same, and every screen is held
against the original's golden, the one after that step too. The
command's line is in `DEVIATIONS` of `harness/deviations.py`, as
spaces, so the golden is derived without it: it is known wherever it
is the whole of a row. That the portable C then shows the same cell
for cell is what piece 0b has to prove: it is not seen yet. Should it
have to show something else on that row, the entry of `DEVIATIONS`
is what changes, and that is asked first.

### What ran: the coverage, and its fix

`coverage.Listing` reads `VCSETUP.LST` as it is, with the code between
procedures by its labels and the reserved room not counted. But
`coverage.executed` takes code of the main program for the setup
program's, when both are in one trace: the two files have routines of
the same bytes, in the same order, so blocks that ran in the main
program agree on a place where the setup program never was. The
original taken through dialogs and the help, with the setup program
never started, had 1,003 bytes of `VCSETUP.COM` count as run. The fix
is one line, in `coverage.executed` since 0a: a block that the file of
another program has too, one of `others`, does not vote for where a
program is. With it that run counts 0, and
what really ran in the setup program's place still counts, the
routines that both have included.

The other way round is the same, and is not relied on: what a scenario
of the setup program runs of `VC.COM` is not recorded at all, and so
is kept out of what the batches of the main program go by. That run
counted the same 11,055 bytes of `VC.COM` with `VCSETUP.COM` given as
the other program as without.

`setup`, of nine screens, ran 4,095 bytes of `VCSETUP.COM`.

With the fix, the nine screens of the trial ran 1,851 of the 4,861
instructions: 855 of the 1,745 that are the main program's word for
word, 420 of the 1,300 with another text, and 576 of the 1,816 that
are the setup program's alone, of which some 288 are of its start.

### The deviations to declare

In `src/portable_c/DEVIATIONS.md`, by the piece that makes each so.
Entry 8 there has what 0b made so: the first four rows, the first half
of the sixth, and the two that the plan did not have, which stand
last. And what E1 made so, the second half of the third: the call is
gone, with the entry of INT 21h, `asked` and `handed-on` are not run
for the portable C, and `ended` has its patch made again.

| What | What shows it |
|---|---|
| Setup is no program of its own: no banner, no word on the version of DOS or on the memory, no command line, nothing run from DOS without the main program, no "Error reading data from the help file." | No scenario: the banner is sent nowhere when a golden is derived |
| What DOS has shown has no line of a command that started setup, behind setup and afterwards: the row is empty, and the screen moves as the original's. And the history has no such command | Every scenario of setup, on the rows that `harness/deviations.py` has as spaces; the history in none |
| The program takes what was saved whatever its setup has for TSR manager, where the original's running copy is asked only with that on. And with E1 the program's own call of DOS is gone | `asked`, `handed-on`, `ended` |
| The Options menu has two lines more, at its bottom | `clock`, `clock-russia`, `menu`, which are held by patches of the original's goldens, of entries 1 and 8 |
| No Memory allocation and no TSR manager in S2, no Screen output in S3: entries 1, 7 and 5, since E2 | Ten scenarios, of A, D and 0a, that show the two dialogs: see "Piece E2" below |
| The program's box for a critical error of DOS while setup is open, where the original's setup program leaves the error to DOS's own words. And setup's calls for files are asked with the program's errors, where the original's setup program calls DOS itself: a drive that is not ready has the program's box, a call given up there is the error 53h, and DOS is asked what was wrong behind a call that failed | No scenario |
| The setup that setup makes without a good file, and for Set default configuration, has zeros in the 64 bytes behind the path of each panel, where the original has what stands behind a table of 24 bytes in the setup program: see the bugs below. A setup file saved of it differs from the original's in those bytes. **This waits for the user's word**: it was met while 0b was made, and is not among the seven things decided | No scenario: `setup-defaults` ends with the file as a program started anew saves it, where the bytes are zeros in the original too |
| The key bar of setup is 80 cells, the main program's routine: the 81st cell of the setup program's is a bug that is fixed, on the user's word of 13 October 2026. Entry 9 | `setup-lines`, with a patch of the original's golden for the corner: one cell, on two screens. Not run for the portable C since EGA lines went, and the patch with it: "The tall screen: untested in the portable C until a system gives it rows", below |
| The directory of the setup file is the one that the program has, made by the main program's rule: in capitals, and with a backslash behind whatever `VC=` names, where the setup program adds none behind a colon | No scenario: the bench has no `VC=` |

### The bugs of the original that stay

Each as it is, until its fix has been decided; the first two were seen
on the bench.

- The pages 6 and 7 of the help, Country info and Exit, are "Help
  contents is not available", with a beep. `RdHelp` reads the seven
  pages into what is left of the program's 64 KB, 8,155 bytes behind
  `HelpBuf`, which is at DF03h in the original, and the pages of the
  help file that was shipped are longer than that together: five fit.
  To show the same, the portable C has to go by that number.
- `PutKey0` writes 81 cells of a row of 80. The last is the first cell
  of what lies behind the row in `ScrBuf`: on a screen of 50 lines that
  is `UserScr`, and when setup is left the top left corner of what DOS
  had shown is a space, in the colour of the key bar's numbers. With
  fewer lines it lands where nothing shows it. This one does not stay:
  it is fixed on the user's word of 13 October 2026, entry 9 of
  `DEVIATIONS.md`, and the key bar of setup is the main program's
  routine with setup's tables. D's scenario of 50 lines shows the
  corner, and has a patch of the original's golden for it.
- `Err2`, behind Ok in S0, is to fill `SrcIni` with FFh, so that the
  defaults count as a change and are offered for saving. It does so by
  `REP STOSB` with CX as it happens to be, which is 0 on every way
  there: nothing is stored, and defaults that are left as they are are
  not saved.
- Cancel in S10 leaves, as Save does, and does not go back to S1.
- `Default` copies the 88 bytes that the file has of a panel from
  `DefWCB`, which is 24: the other 64 are what stands behind that table
  in the setup program's data, the box `IniEr_P` with its dialog and
  its buttons, 47 bytes with eight addresses of the original's layout
  in them, and the first 17 bytes of the dialog for registering, which
  only the binary has that was shipped and the published source has
  not. They land behind the zero that ends the path of each panel, and
  are in `VC.INI` when a setup made so is saved; the main program
  clears them when it has read such a file (`MkIni1` of `setup.c`).
  This one does not stay: the portable C cannot have the 17 bytes, and
  has zeros for all 64. See "The deviations to declare".
- A file that could not be written is said in a line on what DOS has
  shown, which the panels of a running copy cover at once, and the
  running copy takes the settings all the same. `setup-save-fails`
  holds that the program takes them and that the file stays; the line
  itself no scenario can hold, see "Piece D".
- `DeCode`, which takes the mark off what `RdHelp` has read, takes a
  count of 0 for one of 65,536: a help file whose index says that it
  has no text, or whose 62nd page is where its 55th is, has the setup
  program write over all of its segment, itself included. The portable
  C has it as it is, in the one `DeCode` of `help.c`, over the segment
  of the main data. No scenario has such a file.
- A second click on a line of the menu enters the item under the
  cursor, not the item that was clicked: `Main50` says only that it
  was a second click, and `Main32` goes by the cursor. They differ
  when a key has moved the cursor between the two clicks. Seen on the
  bench, and `setup-mouse` has it.

### Piece C, the help: done, and what a later piece should know

The help of the setup is in the tree: `sthelp.c` and `sthelp_t.c`,
called from `StSetup`, which reads the pages, and from `Input`, for F1
while the setup is open. Five of its six scenarios pass, each screen
the original's, and `setup-delay-help` waited in `not_run` of
`TREES["portable_c"]` for piece A's dialog: see "What was left" below.
The code passed them as it was written, with nothing changed but where
the pages are kept.

| Scenario | Steps | What it has |
|---|---|---|
| `setup-help` | 25 | A page of 82 lines by every key: lines, pages, both ends, a key that means nothing, F1 in the help, and the three keys that close it |
| `setup-help-pages` | 31 | F1 on each of the nine items: five pages, the palettes sharing one, and "Help contents is not available" twice, with its beep; a page shorter than the window; F1 in that box; the page of a box over the box, F8 and F1 |
| `setup-help-mouse` | 23 | Each third of the text with either button, a button moved out of the text while held, beside the text, outside the window, the middle button, Ok, and F1 and F10 of the key bar |
| `setup-help-file` | 41 | The help file changed in the editor, three times: the index made to name the 56th page as the 55th, so that the seven pages fit and the last two show; the file cut off in its index; and no file |
| `setup-delay-help` | 9 | Piece A's, which wrote it and left it out: F1 over the box "Illegal format of", which has no page, and behind it the page of Panel options, over a dialog whose cursor is in a line that is typed into. It needs piece A's dialog as well as the help |
| `setup-help-lines` | 8 | A screen of 50 lines |

They run all 408 instructions of `RdHelp`, `Help` and `HlpLine` in the
original, and all of `SeekDsk` and `ReadDsk`, which `RdHelp` alone
calls, but the `STC` of `SeekDsk`: DOS does not go to another place
than it is asked for. `python3 harness/reach.py --setup RdHelp Help
HlpLine SeekDsk ReadDsk` says so.

**The help file, and what the main program does with the same file.**
`VC.HLP` has the version at byte 8, `VC 4.05 Help`, and at 194h where
its index is, 1C2h in the file that is shipped. Every entry is 26 bytes
and then its text: the mode, 0 for the index and not 0 for a page, the
length of the text, the number of its lines with the title, and where
the index is; all of an entry, and the four bytes at 194h, have 1Ah
added to each byte without carry. The text of the index begins with six
bytes for each of its 62 lines, the last four of which are where the
page of that line is. The pages follow one another in the file in the
order of the index with nothing between them. The 55th to the 61st are
the setup program's, 8,628 bytes together; the 62nd, the last, is the
main program's again.

The main program (`help.c`) reads the index every time F1 is pressed,
and of the pages the one that is asked for, by its place in the index,
into `HelpBuf` of the main data, 3,000 bytes: it keeps nothing from one
F1 to the next. The setup program reads once, when it starts: the
index, to find where the 55th page is and where the 62nd, and then in
one piece what lies between the two, or as much of it as it has room
for. A page is then found by going from entry to entry in the memory,
by the lengths, and never by the index. So its pages are numbered from
1 by where they lie in that piece, and a help file with something
between two of them would show the setup program something else than
the main program.

**The length that the last two pages hang on.** `RdHelp` reads into
`HelpBuf`, which is the end of the setup program's data, and stops at
FEE0h of its one segment, 288 bytes below the top, where the stack is:
`MOV AX,0FEE0h` and `SUB AX,OFFSET HelpBuf`. `HelpBuf` is at DF03h in
the program that was shipped: 100h for what DOS has in front of a
program, the 24,278 bytes of `VCSETUP.COM`, and 7F2Dh of data that the
file does not hold, which is what `build/shareware-layout/VCSETUP.LST`
has for `HelpBuf` in its segment. The first word there is how many
bytes were read, so the pages have FEE0h less DF05h, 8,155 bytes: the
page of Country info begins at 7,898 of the piece and ends at 8,441,
and that of Exit begins there. The number follows the length of the
file, and with it the 275 bytes that the binary has and the source has
not. The translation has the two numbers, DF03h and FEE0h, and goes by
them: the first for more than the length, see the walk below.

**What the original does that the plan did not have.**

- `RdHelp` makes the name of the file by copying 13 bytes of a text of
  7: `VC.HLP`, its zero, and the first six characters of the version
  behind it. The main program does the same with the same two texts.
- It takes the mark off as many bytes as it asked DOS for, not as many
  as DOS read, and notes how many were read.
- Where it fails it says so and leaves `HelpBuf` as it is. The first
  word there, which `Help` takes for the length of the pages, is then
  the mode of whatever was read as the index, 0 for a real one, if it
  got that far; before that it is memory that nothing has set. With no
  help file F1 goes through what the memory held: on the bench that
  was "Help contents is not available" each time it was tried, by what
  was there and not by anything the program did. The portable C can
  only have a number there, and 0 is the one that shows the same.
- `Help` goes from entry to entry with addresses of 16 bits, and looks
  whether an entry ends beyond the end of what was read, and whether
  adding its 26 bytes or its length went round at 64 KB; where the
  end is, the start and what was read, it adds up without looking. So
  with a first word that is no length, behind a read that failed, what
  shows depends on where `HelpBuf` is in the segment: from 20FBh on
  the end is below the start, and no page is found. That is why the
  translation has DF03h and not only the length.
- The box "Help contents is not available" has three lines and its
  button on the third row, which is its frame: " Ok " stands in the
  box as a line of text and again in the frame below it, where the
  cursor is. The main program's box of that name has four lines.
- The window of a page is kept in `BoxBuf`, as that box is, and as the
  boxes "Illegal format of auto change dir time delay" and "Do you wish
  to set default colors" are, over which F1 does nothing: none of
  them has a page. The dialogs and the menu are kept in `WinBuf`.
- `HlpLine` clears the bytes 1 and 101 of `TmpErr` for `NgLine`, as the
  main program's does. In the setup program a line that is typed into
  is kept at `TmpErr`, and byte 1 is the high half of its cursor, which
  is 0: nothing shows.
- A button of the mouse has been asked for four times before `Help`
  looks where it is: `Input0`, the buttons of `MinMnu`, and `Help`
  itself. The top of `harness/scenarios.py` has what that means for a
  step.

**Decided, and what follows from it.**

- "Error reading data from the help file." is dropped, on the
  session's word, as the first row of "The deviations to declare" has
  it: setup is no program of its own, with no banner and nothing of
  what the setup program said when it started, which the user approved
  with the plan. The setup program prints that line through DOS when it
  starts and cannot read the file, and goes on. `StRdHelp` prints
  nothing: where the original goes to `RdHlp2`, it closes the file if
  one is open and returns. A help file that cannot be read shows from
  then on as it does in the original, F1 giving what the memory of the
  pages gives. `setup-help-file` needs no golden of its own for the
  line: the step that starts the original sends what it prints
  nowhere, so its golden has none either. The entry of
  `src/portable_c/DEVIATIONS.md` for that row is piece 0b's, and has
  this line in it. In the batch of `harness/ported.py` the three
  instructions that print, 113Dh to 1142h of `VCSETUP.LST`, are
  `not_carried`, with that reason.
- What leaving setup has to do for the main program's help: nothing,
  if setup's pages are not in `HelpBuf` of the main data, which has no
  room for them anyway. The main program's help keeps nothing between
  two F1, the main loop sets `HlpPage` to 0 each time round, and
  `Help` puts `HlpPage` back as it found it. `HelpBuf` of the main
  data also holds what the viewer and the editor last looked for.

**What is there.**

- `sthelp_t.c` has what `Dialog` and `MinMnu` take by its offset:
  `StHlEr1_S`, `StHlpMn_D`, `StHlpEr_P` and `StHlpEr_B`. `Help_S`,
  `OkMn_D` and `HlpMn_P` are the main program's, the same bytes, and
  are not written twice. `StOkMn_S`, " Ok ", and `StOk_S`, "[ Ok ]",
  which the help reads both, stand in `VCSETUP.ASM` among what is
  piece A's: they are defined in `stopts_t.c`, not static, and the
  help declares them. `sthelp_t.c` had them until that file was there.
- `sthelp.c` has `StRdHelp`, `StHelp`, `StHlpLine`, and `SeekDsk` and
  `ReadDsk` as static functions of its own, with `help.c` as the
  model: the setup program's two differ from the main program's, which
  take the handle from the main data. `DeCode` is the main program's,
  of `help.c`, which the setup program has word for word: it is not
  static there, `vc.h` declares it, and the batch does not list it, as
  `harness/ported.py` has it for such a routine. It takes a near
  pointer, which the pages have, being in the main data. `Help_T` is a
  `switch`. The
  window is kept at `md.ErrBuf`, the setup program's `BoxBuf`; the test
  of the data is told so for `StHlpEr_P` in `SETUP_NAMED_OTHERWISE`,
  and of two constants in `REVIEWED` of `harness/reads.py`.
- The pages are in `TreeBuf` of the main data, 20FDh bytes of it,
  `StHelpBuf`: as long as from where the original has `HelpBuf` to the
  end of its segment, so that every address which `StHelp` can come to
  in looking for a page is a place in them. A line of `HOLDS` says
  that they fit. Nothing else may use `TreeBuf` while setup is open.
- `StSetup` calls `StRdHelp` in front of `StFile`, where the original
  has the call: `NcPath` is the directory of the program there, which
  `MainFil` makes the name of the help file with, at `md.TmpBuf`.
  `StRdHelp` asks for the file with the program's errors, `FileOpen`,
  `FileSeek`, `FileRead`, `FileClose`, by decision 7: its row is in
  entry 6 of `DEVIATIONS.md`. It clears all of the pages' room first,
  and says nothing if it cannot read the file: see above. `StOpen` is
  still 0 then, so a critical error there has the main program's box
  as the setup file's has.
- `DeCode` with a count of 0 goes through 65,536 bytes, in the original
  as here: a help file whose index says that it has no text, or whose
  62nd page is where its 55th is, has the original write over itself.
  Here that is all of the segment of the main data, from the place in
  `TreeBuf` round to it again; no scenario does it. "The bugs of the
  original that stay" has it.
- The batch in `SETUP_BATCHES` of `harness/ported.py`: the routines
  `RdHelp`, `Help`, `HlpLine`, `SeekDsk` and `ReadDsk`, the functions
  `StRdHelp` and `StHelp`; out of reach the `STC` of `SeekDsk`, 1482h;
  not carried over the three instructions that print the line. All
  the rest runs in the original, and both functions run in the tree.

**What was left.** `setup-delay-help` wanted piece A's dialog and the
help both in the tree. It was in `not_run` with a reason of its own,
and came out with piece A, which landed second: it passes as it was
written.

**For piece D.** F1 from the screens of A and B is not in these
scenarios, which keep to the menu and to the box of F8 so as to need no
other piece: the Configuration, Screen and Mouse and Panel options with
a line that is typed into, the list of colours and the picture of one
(`HlpPage` 4 in both), Country info (no page: the box over a dialog),
and "Do you wish to save the current setup?" (no page either).

### What could go wrong

- **The size of the file.** Setup is some 8 to 10 KB of compiled code,
  by what the rest of the program came to, 12 KB of tables, and a
  kilobyte or so of relocations for the calls from the second code
  segment into the first: more than the build had below three times
  the original's size when this was written. The portable C is held to
  that no longer, once piece 0a has taken the check out; nothing else
  goes by the length of the file.
- **The program's data.** `DGROUP` is all but full: see "How it fits".
  A static buffer of a screen or of the help does not link.
- **The code segments.** The new files go into the second, which has
  the more room; the pictures and tables are no code. Should it pass
  its limit, a third segment is a change to `add_portable_c`.
- **`PutKey0`'s 81st cell** is fixed: see the bugs above. No scenario
  of 0b is on 50 lines.
- **The help.** That two pages do not show hangs on a length of the
  original's layout. And the main program's help keeps what it has read
  in `HelpBuf`: whatever setup does there has to leave the main help
  knowing that it has nothing.
- **Registers across jumps.** S5 and S6 are one stretch of code that
  carries BX and DX, where the window is and how large, BP, the
  palette, and SI and DI, the cursor and the first line, through every
  jump, as the editor's keys do ("What the assembly left in the C" of
  `CLAUDE.md`). `Main32` finds the command of an item by `Case` with
  the item's number as a key. `Col00` exchanges `Clock` and `KeyBar`
  for 0 and `ColTyp` for the palette while the picture is up, and
  `F07_80` and `F07_85` exchange the colour of the panels while the
  colour's name is written.
- **Text that the program writes into.** S8 fills lines of its own box
  before it shows it, `Inf02_S`, `Inf06_S`, `Inf07_S`, `Inf10_S`,
  `Inf11_S`: those are no constants, and the data's test holds what
  they are before.
- **The pictures of S6** have bytes that are no characters. Take every
  text from the assembly's bytes by a script, as "Moving the data of a
  file to C" of `CLAUDE.md` has it. In `Colr1_T` to `Colr4_T` a colour
  is the number of an item in the low six bits, with 40h for a cell
  that lies in a shadow and 80h for one that blinks, and FFh is plain
  grey.
- **The data's test.** It holds the tables of setup against
  `VCSETUP.COM` since 0a, by the names with `St` in front. A table
  that the C has otherwise than the assembly, a `switch` for a table of
  keys, is held by the screens alone, and `ColPage`, which picture an
  item has, is held by the test only if it is written as the table
  that the assembly has.
- **The item of the menu.** Done, twice. A line put in above others
  moves them, and an item is known by its number in code as well as in
  tables: the ticks of the Options menu, `F39_58` of `menu.c`, the keys
  of an open menu, `KeyTab`, with Ctrl-B, the commands, `Menu4_T`, the
  pages of the help, `MnuHlp`. Nothing else knew an item of that menu
  by its number. All of that was moved for the item behind the editors
  and moved back when the item went to the bottom of the menu, where a
  line moves nothing: there it is two entries at the end of `Menu4_T`,
  of `MnuHlp` and of the lines, and two rows in `MnuTab`. Its keys,
  "F9 o m", are those of a step that is left out for the portable C,
  Memory allocation switched: `Declared.steps` of
  `harness/deviations.py` leaves a step of the original out before it
  puts another in its place, and `goldens.changes` reads a golden by
  the same rule.
- **What a box of setup means to the main program's `Dialog`**: a menu
  bar to draw and a box to move down, unless `Dialog` is told. And the
  routine that a dialog names for Enter, `F04_10`, is a number there, a
  `SUB_` of `tables.h`.
- **What both programs have and each sets up for itself**: the main
  program has taken INT 24h, has BREAK off and the mouse started when
  setup is entered, and `IniData` does it all again when setup is left.
  What setup's own start and end did of that is not done twice. Two
  things are done all the same, because they show: the mouse is set up
  anew when setup is entered, which puts its pointer in the middle of
  the screen, and the main program puts it back afterwards; and the
  two interrupts are given back when setup is left, as before a
  command, so that `IniData` notes whose they were and not the
  program's own.
- **The time.** A scenario with the clock on in setup has it on the
  screen from then on.
- **`resident.c`** is in other hands than setup's until E1. That piece
  is done, and nothing of setup's went into the file.

### Piece B, the colours: its scenarios

Seven, of 226 steps, all of the bench of the United States: the list
and the picture look at no country. The portable C is taken through
all of them, and passes.

| Scenario | Steps | What it has |
|---|---|---|
| `setup-colors` | 31 | The list by its keys: the cursor, the pages, the lines moved by one, a key that means nothing, F8 answered No and Esc, a second palette, and nothing to save |
| `setup-colors-items` | 44 | Every one of the 39 items of the Color palette entered, each with its picture and with its cells shown by another background |
| `setup-colors-grid` | 47 | The grid by its keys, a colour taken and not taken, Shadow as each of the three things it can be, the default colours of the palette taken, and the setup saved: the program's panels on green |
| `setup-colors-mouse` | 25 | The mouse in the list and in the grid, each button, and saved |
| `setup-colors-palettes` | 36 | Black & White and Laptop, which the screen is not in: each of the four pictures in one of them, and saved |
| `setup-colors-own` | 32 | The laptop palette in use and the key bar off, both saved by the main program beforehand: the palette that setup is drawn in changed, saved, and shown by the program |
| `setup-colors-lines` | 11 | 50 lines: all 39 items on one page, a click below the last, a picture |

What they run of the original: 810 of the 813 instructions of
`F05_00` to `F07_57`, `F07_60` to `F07_85`, `Col00` to `Col55` and
`ShadCol` (`python3 harness/reach.py --setup F05_00 F07_60 F07_65
F07_70 F07_75 F07_80 F07_85 Col00..F08_00 ShadCol`). The three that
are left are for a monochrome display, mode 7, which the bench has
not: `MOV SI,OFFSET MonMd_S` at `F07_01`, `INC SI` in front of
`F07_19`, `INC DI` in front of `F07_54`.

What the original does there that the plan did not have, each as it
is, seen in the goldens:

- F8 in the list, answered Yes, takes the default colours and draws
  nothing: `F07_53` goes back to `F07_10` with the cursor and the first
  line as they are, which is the way that changes nothing on the
  screen. The colour named on the right, and the cursor in the grid,
  are those from before until the cursor of the list moves. And the
  box hides the cursor, Yes or No, which the grid has again only then.
- Where the lines move by one, `F07_16` has the BIOS scroll them and
  writes the one new line out 49 cells wide where a line is 41: the
  eight more are the frame and what is beside it, as the screen has
  them already.
- A shadow of the picture that blinks would lose the blinking while
  the item Shadow is chosen (`Col32` to `Col36` take the colour without
  the bit 80h, `Col05` with it). No cell of the four pictures is both.
- On 50 lines, leaving the list shows `PutKey0`'s 81st cell: the top
  left corner of what DOS had shown is a space from then on. The
  banner of the main program is then not known as the banner by
  `harness/deviations.py`, which knows it from its first letter on,
  and a golden would have its "shareware" where a build has
  "freeware". `setup-colors-lines` ends in the list for that; piece D,
  which is to show that corner, has the entry of `DEVIATIONS` to see
  to first.
- With the key bar off when setup is started, the prompt of the main
  program is on row 23 before and on row 24 after, as behind any
  command. `setup-colors-own` ends on such a screen, as `setup` does.

### Piece B, the colours: done, and what a later piece should know

The list of a palette's colours and the picture in which one is chosen
are in the portable C, behind F5, F6 and F7 of the setup's menu, and
the seven scenarios above pass, each screen the original's. Nothing of
it is declared as a deviation but two calls of the BIOS in entry 6 of
`src/portable_c/DEVIATIONS.md`, each with a register that the call does
not take.

**The code, `stcolor.c`**, with the tables in `stcolor_t.c`. One file
for both screens, in the second code segment. What the list knows is
not handed from function to function. The variables of the assembly,
`ColCur`, `ColFrst`, `ColNum`, `BackC`, `HistC` and `HstCuC`, are the
fields of `struct StData` that 0b has for them, and what the assembly
has in the registers BX, DX and BP through every jump, the box and the
palette, is `list`, a static variable of the file: six bytes of
`DGROUP` besides the one of `StOpen`, since no function is entered
twice at a time and a pointer to a local structure would have to be a
`STACK` pointer, which is Open Watcom's. Every label that the assembly
jumps to with a new cursor is a function that works the cursor out and
calls `StF07_10`, the one place that moves it; a jump back to where a
screen waits for a key is a return to the loop of `StF07_01` (the
list) or `StCol00` (the picture); and what a key or the mouse makes of
a screen besides is what the function returns, a `WHAT_`. The two
tables of keys are `switch` statements.

| Function | Of the assembly |
|---|---|
| `StF05_00`, `StF06_00`, `StF07_00` | The three entries, which the setup's menu calls: each calls `StF07_01` with the palette's name and number |
| `StF07_01` | `F07_01` to `F07_07`, the loop `F07_20`, the keys Home, End, Esc, F10 and Enter of `Color_T`, and the way out, `F07_57` |
| `StF07_10` | `F07_10` to `F07_19` |
| `StF07_30` | `F07_30` to `F07_39`, the mouse |
| `StF07_40`, `41`, `44`, `48`, `53` | Up, Down, PgUp, PgDn, F8 |
| `Lines` | `F07_60` |
| `StF07_66` | `F07_65` and `F07_66` |
| `StF07_70`, `StF07_75` | The same |
| `StF07_80` | `F07_80` with `F07_85`, which nothing else calls |
| `StCol00` | `Col00` to `Col10`, the keys of `ColSt_T`, `Col50`, `Col55` |
| `StCol11` | `Col11` to `Col19`, the mouse |
| `StCol30` | `Col30` to `Col40` |
| `ShadCol` of `screen.c` | `ShadCol`, split out of `Shadow` there, which calls it |

**How it was made, and what that showed.** The code was written and
tried before 0b was there, with a hook that was not kept: a key of the
main program that put what DOS had shown on the screen and called the
three entries. Six of the seven scenarios' steps were held against the
goldens so, and every screen of the list and of the picture was the
original's. Connected to 0b, all seven passed at the first run, and
nothing of the code had to change but the three places where it had
stood in for what 0b brings:

- The way out of the list, `F07_57`, puts back what DOS had shown by
  `StUser(0, md.Lines)`, which has the 50th row.
- F8, in the list and in the picture, reads the default colours from
  `StPalets` of `stmain_t.c`, the palettes that the object begins
  with. `Palets` of `start.c` is static, as it was.
- The six variables are 0b's fields, as above.

The key bars, 2 for the list and 1 for the picture, are `Input`'s, by
the number that it is called with and `StOpen`; so is F1, with the page
4 in `HlpPage`, which the box of F8 changes and the list sets again.
The pointer of the mouse in the middle of the screen from setup's
first screen on, which `setup-colors-lines` shows, is 0b's entry: it
resets the mouse driver as the original does.

**The batches**, two in `SETUP_BATCHES` of `harness/ported.py`: the
list, routines `F05_00`, `F07_60`, `F07_65`, `F07_70`, `F07_75`,
`F07_80`, `F07_85`, and the picture, `Col00..F08_00` and `ShadCol`.
Three instructions are out of reach, 07F2h, 09C7h and 0B39h of
`VCSETUP.LST`, which are for a monochrome display. `functions` of a
batch are those that other files call, which the link map has as
public: a static function is not found there, and the test ends with
"the link map has no function" for one.

**What is declared with the code.**

- `harness/reads.py`, `REVIEWED`, for `stcolor.c`: `StColrs_S` and
  `StColNm_S` (texts counted by their zeros, by an item below 39 and a
  colour below 18), `StColNm_P` (1: a colour's name has no markup that
  asks for another colour), `StColPage` (by the item under the cursor),
  `StPagTab` (by the picture of `StColPage`, 1 to 4), `StPalets` (240,
  the six palettes that it begins with), and the four names of a
  display or a palette, read as far as their zeros.
- `DEVIATIONS.md`, entry 6, two places where the BIOS is called with a
  register that the call does not take: in `StF07_10`, the BIOS's 06h
  and 07h by which the lines move, BL is the column of the lines, 8, in
  the original and 0 here; in `StF07_01`, the BIOS's 01h that puts the
  cursor's shape back when the list is left, AL is the low byte of the
  key that left, 1Bh for Esc and 0 for F10 and for the mouse, and BX
  and DX are the box of the list, where `CursorShape` hands over 0 in
  all three. AL of that call is one of the four registers that the
  entry says something is known to read, the video mode for some
  BIOSes: the original has no mode there either.

**For a later piece.**

- D, or whoever shows the corner on 50 lines: `setup-colors-lines` ends
  inside the list, on purpose. Leaving the list there puts all of what
  DOS had shown back and draws the key bar, whose 81st cell the
  original writes into the top left corner and the portable C does not
  (entry 9 of `DEVIATIONS.md`). A scenario that leaves the list on 50
  lines is one of that deviation, with a patch of the original's
  golden for the corner, as `setup-lines` has.
- A monochrome display is not on the bench. The three instructions for
  it are translated as they read: "Monochrome" for the name of the
  display, and the second colour of each pair of a palette, in
  `StF07_10` and in F8 of the list. The picture reads its colours
  through `ColAddr`, which knows.

**Traps met.**

- The mouse is asked where it is as often as the original asks, in the
  same places: the tests hold a button for so many askings. `F07_20`
  asks, and `F07_30` asks again at once; so `Col10` and `Col11`.
- `Window` writes whole rows of 80 cells when it is given more than one
  row, whatever the columns: the right of the screen and the picture go
  out with what the screen buffer has beside them.
- `Case` of the setup program takes no key whose scan code is 0, and an
  entry without a scan code, Enter and Esc, with any.
- `GetMous` wants the place that CX held, for a machine without a
  mouse; no code of this piece runs there but the two askings at the
  top of the loops, which go by the buttons alone.
- A file of code reads a table of `stcolor_t.c` as a far object, and a
  word of `StPagTab` is an offset in the segment that `AsmSeg` returns.

### Piece D, what is left of the menu and of leaving: done, and what a later piece should know

The mouse at the setup's menu is in the portable C, and six scenarios
hold it and what crosses the screens of the other pieces: the clock
under them, the buttons of the mouse exchanged in them, the help over
them. Each golden was derived twice with the same result. Five are held
against the original's golden and one against a golden of the portable
C's own, for one cell.

| Scenario | Steps | What | For the portable C |
|---|---|---|---|
| `setup-mouse` | 21 | S1 by the mouse: a line by either button, the frame and the margin of the box, a button held and moved, a click too short to be seen, a second click with a key between, one let go beside its line and one outside the box, Exit by its line, the middle button, the shadow, and the left button let go outside | As the original |
| `setup-save-fails` | 8 | The setup file read-only, a setup changed and saved: the file stays, and the program has the setup | As the original; it needed no code |
| `setup-lines` | 9 | 50 lines over what DOS has shown, a click on a line of the menu there, and the corner that the 81st cell of the key bar lands in | Against a golden of its own: entry 9 of `DEVIATIONS.md` |
| `setup-clock` | 16 | Clock ticked: the clock over the menu, a dialog, the help, the list of colours, not in a picture; unticked, and gone. `setup_clock=True` | As the original |
| `setup-left-handed` | 15 | Left-handed mouse ticked: the buttons exchanged at the menu, in a dialog and in the help, and in the program when it is saved | As the original |
| `setup-help-screens` | 31 | F1 from the Configuration, from the line of Screen and Mouse, from the list of colours, its box and a picture, from Country info and from "save the current setup?" | As the original |

`python3 harness/reach.py --setup Main50 Init15 Exit` says what the
scenarios of the setup run of the original: all 74 instructions of
`Main50`, the six of `Main30`, and `Exit5`. Nothing of piece D is
listed as out of reach.

**How the last three came in.** `setup-clock`, `setup-left-handed` and
`setup-help-screens` were written against the original while the
dialogs, the colours and the help were still to come, and waited in
`not_run`. With all three pieces in the tree they passed as they were
written, at the first run: nothing of the C of any piece had to change
where two of them meet. The golden of `setup-lines` of the portable C's
own, which was recorded while F6 was a place-holder, came out the same
with the list of colours behind that key, cell for cell: `python3
harness/goldens.py changes --tree portable_c setup-lines` has one cell
on each of the last two screens, the corner, and no other row.
It landed as a golden of the tree's own, like the fifteen others of
that kind, and is a patch of the original's golden now, as they are:
`harness/goldens/portable_c/setup-lines.patch`, one cell on two
screens. (The scenario is not run for the portable C since EGA lines
went, and the patch is in the history: see "The tall screen: untested
in the portable C until a system gives it rows".)

**A seventh that went.** `setup-save-new`, a setup saved without a
setup file, was written here while piece A had the same thing in hand,
and A's is the one that is kept, `setup-missing-saved`: the file
deleted, the setup started, a box of the Configuration ticked, saved,
and the main program's own setup written over the file. This one ran
nothing of the original that the other does not run, but for what the
Configuration and `PutKey` do with the key bar switched off, which was
the box that it changed and is what `setup` has; and it showed nothing
more.
The three instructions of `Exit` for a file that is not there are run
by `setup-missing-saved`.

**The code.** `Main50` is the main program's `F32_70`, of
`usermenu.c`, word for word but for its labels: the routine of the user
menu and of the history for the mouse in a list that is in a box.
`harness/vcsetup/procs.py` has it as the setup program's alone, since it
goes by names. `StMain00` calls `F32_70`, and `Main30` is the six
instructions around the call. `Main50` is listed in the setup's batch of
`harness/ported.py`, so that all of it has to have run in the setup
program. `HlpLine` is the main program's `Help77` by its text in the
same way, but names a table of colours, which the setup program has of
its own: piece C has written it, `StHlpLine`.

**What the original does there that the plan did not have.**

- The second click enters the item under the cursor: see "The bugs of
  the original that stay".
- A second click that comes up beside its line, or outside the box,
  chooses nothing and does not leave: `DoublMo` waits for the button
  itself, and `Main50` never sees where it came up.
- The shadow of the menu is outside the box for the left button, which
  leaves when it is let go there.
- The line for a file that could not be written goes where the banner
  goes, to DOS's standard output: the step that starts the original
  sends both nowhere, `>nul`. So the original shows no such line in a
  scenario, and the portable C, which writes it on the screen, has what
  DOS had shown a row further up than the original from then on. No
  scenario can hold the line, and none may look at what DOS has shown
  after a save that failed: the top of `harness/scenarios.py` says so.
  With the panels on, which is how `setup-save-fails` ends, the two
  show the same.
- F1 where the help has no page, over a dialog: the beep moves the
  screen up a row under a box that is kept in `BoxBuf` over a dialog
  that is kept in `WinBuf`, and each puts back what it covered as it
  was before the beep. What is left on the screen is in the golden of
  `setup-help-screens`, rows of the dialog a row up around the box.
- Clock unticked and Ok takes the clock off the screen at once.

**Decided here.**

- The corner on 50 lines is shown without a change to
  `harness/deviations.py`. The banner's first line is in the first row
  of what DOS has shown on the bench, and with its first cell blank the
  entry of `DEVIATIONS` there that reads the original's "shareware" as a
  build's "freeware" does not know it: the golden would have the
  original's word. `setup-lines` first has that line gone from the
  screen, by a command that prints nothing with the key bar off, which
  moves the screen by one row: the corner is then the C of "Copyright",
  which nothing in the harness goes by. The other way would have been
  to have that entry begin with `^[T ]he` in place of `^The`, which is
  a change to what the comparison takes for equal and was not made.
- `setup-lines` goes into the colours and back in one step, "F6 Esc":
  that is where the original first puts the screen back from what it
  kept, so the corner is blank under the menu already.

**For a later piece.**

- E2, which took Memory allocation and TSR manager out of the
  Configuration and Screen output out of Screen and Mouse: `setup-clock`
  gets to Clock by five times Down, `setup-left-handed` clicks a line
  of each of the two dialogs by its place, and `setup-help-screens`
  shows both under the help. Each has a patch of the original's golden
  since, and the last two another step each for the portable C: see
  "Piece E2" below.
- The line "There was an error while saving the setup file" is held by
  no scenario, as above: whoever changes where it is written has to
  look at it by hand, `harness/show.py --program` with the steps of
  `setup-save-fails` and Ctrl+o.
- A scenario that says `setup_clock=True` does not have the original's
  main program watched (the top of `harness/scenarios.py`):
  `setup-clock` unticks Clock before it leaves the setup, for that.

### Piece E1, the program's own call: done, and what a later piece should know

The call is gone from the portable C, and the entry of INT 21h with it
(`src/portable_c/DEVIATIONS.md`, 8, with 1, 6 and 7 brought in line).
What went, all of `resident.c` but the last: `New21`, the eight
instructions of assembly that every call of DOS passed; the interrupt
function `Int21`, which answered the call; `Vect21`, the variable of the
code segment that held where the interrupt went before; the interrupt
taken in `TakeDos` and given back in `Exit02`; and `PutVector` of
`dos.c`, which only that taking used. `TakeDos` is `TakeEnd` now, since
what it still does is to have the PSP name `Exit01`.

What was read first, for what else leaned on the entry, and found to
lean on none of it:

- How DOS ends the program. `Exit01` is named in the PSP, not reached
  through INT 21h, and `Exit05` and `Exit02` behind it are kept whole
  but for the interrupt given back: `ended` runs them.
- Ctrl-C and Ctrl-Break: `CtrlC`, taken by `SetBrk` and by `HookVct`
  of `main.c` through DOS, `Vector` and `SetVector`.
- Quick execute, `Int2E` and `Exec2E`: the commands that COMMAND.COM
  runs there make their calls of DOS, which passed `New21` like any
  other and were handed on untouched. `quick` and the other scenarios
  of INT 2Eh pass as they did.
- The driver of the tests knows the program's idle call by INT 28h and
  by who DOS has as the current program, and nothing of INT 21h.

For whoever goes on:

- What the program leaves in the registers when DOS ends it was
  decided by reading the assembly and then seen on the bench. With no
  interrupt to give back the original, where its setup has TSR manager
  off, leaves DS as it came and DX as it came but for the low byte,
  which is what it has just told DOS of Ctrl-C; `Exit02` leaves the
  same. The trial: the steps of `ended` behind a setup saved with TSR
  manager off, as a scenario that was not kept, derived from the
  original. The copy that DOS ends reads that setup, and the program
  that started it printed DX as 0101h and DS as its own segment, each
  less where INT 27h went then, which was to the first copy: 01h is
  the high byte of the address that it had in DX when it ran the copy,
  and the low byte says that DOS looked for Ctrl-C at every call when
  the copy started.
- So of the two words that `ended` had left out of the comparison one
  is compared again, DX, and one is still left out, DS: the segment of
  the program that prints, which lies behind the first copy and the
  COMMAND.COM that it ran, and follows the size of the program under
  test. A change of `harness/deviations.py` that leaves less out.
- `asked` and `handed-on` are in `not_run` of `TREES["portable_c"]`,
  and `handed-on.patch` is gone. `passed-on` and `stay`, which were
  not run before, say there that `handed-on` and `asked` hold what
  they held besides: those sentences were brought in line.
- `VARIABLES` of `harness/reads.py` is empty: `Vect21` was its one
  entry, and an entry there that the tree has no object for fails the
  data's test.
- The original's setup program, `VCSETUP.COM`, run from the portable C
  as a command, saves its file and is answered by nobody: DOS takes
  the call for its own AH 2Bh, a date to set, and refuses it. No
  scenario does that; the bench has no `VCSETUP.COM` next to a C tree.

### Piece E2, the three settings: done, and what a later piece should know

The Configuration of the setup has no Memory allocation and no TSR
manager, and Screen and Mouse no Screen output: entries 1, 7 and 5 of
`src/portable_c/DEVIATIONS.md` say so, each for its setting. Their
bytes of the setup file are kept as they are found.

**The two dialogs as they are now.** What stood below a line that went
stands further up in the right half of its box, and the box is as tall
as it was, since its left half is:

    ╔═══════════════════════════ Configuration ═══════════════════════════╗
    ║  ┌ General options ──────────┐  ┌ Execute options ───────────────┐  ║
    ║  │  [ ] Auto menus           │  │  [ ] Quick execute commands    │  ║
    ║  │  [x] Path prompt          │  └────────────────────────────────┘  ║
    ║  │  [x] Key bar              │  ┌ Keyboard options ──────────────┐  ║
    ║  │  [ ] Menu bar visible     │  │  [x] Alt alone selects menu    │  ║
    ║  │  [x] Full screen          │  │  [x] Esc toggles screen on/off │  ║
    ║  │  [ ] Clock                │  │  [x] Alternative viewer        │  ║
    ║  │  [x] Confirmation on quit │  │  [ ] Alternative editor        │  ║
    ║  │  [x] Error sound          │  │  [x] Clear keyboard buffer     │  ║
    ║  │  [ ] Auto save setup      │  └────────────────────────────────┘  ║
    ║  │  [ ] Show logical drive B:│                                      ║
    ║  └───────────────────────────┘                                      ║

    ╔═════════════════════ Screen and Mouse ═════════════════════╗
    ║  ┌ Screen colors ──────┐  ┌ Other options ──────────────┐  ║
    ║  │  ( ) Black & White  │  │  [x] Zooming boxes          │  ║
    ║  │  (•) Color          │  │  Screen saver chars: ·∙•♦☼  │  ║
    ║  │  ( ) Laptop         │  └─────────────────────────────┘  ║
    ║  └─────────────────────┘  ┌ Mouse options ──────────────┐  ║
    ║  ┌ Screen blank delay ─┐  │  [ ] Left-handed mouse      │  ║
    ║  │  ( ) 40 minutes     │  └─────────────────────────────┘  ║
    ║  │  ( ) 20 minutes     │                                   ║
    ║  │  (•)  5 minutes     │                                   ║
    ║  │  ( )  3 minutes     │                                   ║
    ║  │  ( )  1 minute      │                                   ║
    ║  │  ( ) Off            │                                   ║
    ║  └─────────────────────┘                                   ║

That is the lines taken out and nothing arranged anew.

**Open, for the user.**

- The layout. Both right halves are shorter than the left ones now, and
  Screen and Mouse has six empty rows there. Whether the groups are to
  be aligned at the bottom, or the two dialogs arranged anew, waits for
  the user's word, and for what is decided on Quick execute commands
  and on Show logical drive B:, which would change the Configuration
  once more; either would be new rows in the same patches.
- The volume of the ten patches. Every screen that shows one of the
  dialogs repeats the rows of its right half, 11 or 12, so the ten are
  several times all the patches that were there before. A rule of the
  tree that lays a region of the screen out anew, the rows of one half
  of a box moved and spaces below, would say it once, where a rule
  today puts a text over the cells of one line: the neighbour of
  "Leaving a part of the screen out of the comparison: future work"
  below, and not built.

**The fields.** A field of a dialog names the fields that Up, Down,
Left and Right go to, by their numbers, so every field behind one that
went has another number and those around it other neighbours. They go
by what the original's do: down a half and then to the buttons, and
sideways to the field on the same row, or else the one above that row.
In Screen and Mouse a field of the right half that has the first group
beside it now goes there to its left, where it had the second.

**The scenarios.** Ten show one of the dialogs or both, and each has a
patch: the rows of the right half on every screen that shows the
dialog, 11 or 12 to a screen, which is most of the 1,834 lines that the
ten have. Six are taken through other steps for the portable C, by
`in_place_in` of `deviations.TREES`, which was made for this: a step
for a step, or None for one that set what is gone.

| Scenario | Entries | Its patch, in lines expected otherwise | Steps for the portable C |
|---|---|---|---|
| `setup` | 1, 5, 7 | 34, on 3 screens | The original's |
| `setup-config` | 1, 7 | 550, on 49 screens, the setup file among them | Two left out, Memory allocation and TSR manager unticked; one other, Space for Down and Space |
| `setup-config-mouse` | 1, 7 | 302, on 27 screens | Five clicks a row or two further up |
| `setup-screen` | 5 | 723, on 58 screens, the setup file among them | Two left out, a line of Screen output chosen by Space and by a click; twelve others |
| `setup-options-russia` | 5 | 37, on 3 screens | One: a Tab less |
| `setup-delay-file` | 5 | 12, on 1 screen | The original's |
| `setup-missing-saved` | 1, 7 | 22, on 2 screens | The original's |
| `setup-clock` | 1, 5, 7 | 56, on 5 screens | The original's |
| `setup-left-handed` | 1, 5, 7 | 38, on 4 screens | One: the click on Left-handed mouse five rows further up |
| `setup-help-screens` | 1, 5, 7 | 60, on 5 screens | One: a Tab less |

None is not run: no scenario's whole point was one of the three. Every
other scenario of the setup is held as it was.

**How the patches were read.** Not row by row alone: there are too many
that say the same. `build/` had a script for it, which is not kept:
it made of each screen of the original's golden what the declaration
says, the right half of the box with its rows moved as above and
spaces below, and the cursor moved with its row, and held that against
the golden with its patch. What was left over was read by eye, and is
all that the headers of the patches say besides the rows: the cell of
the mouse pointer, which stays where it is while the lines move under
it, on 106 screens; five of the 45 cursors, which are on another field,
where Left or Up goes elsewhere now, the other 40 having moved with
their rows; and the setup file of the two scenarios that untick or
choose one of the three and save. The
bytes of those two files were held against the original's, by the
script of the notes on the harness: 1AFh and 1C0h, Memory allocation
and TSR manager, 1 where the original has 0, and 1BAh, Snow mode, 2
where it has 1, each with the sum behind it.

**In the data.** `StConf_B` and `StScrn_B` are declared without their
fields and with the links and rows of the others, by `_field` of
`harness/datacheck.py`. A line of a box is one text for both halves, so
23 lines are declared too, by `_lines`: a line whose right half is that
of a line further down is held against the original's line of its name
with the bytes between the two halves left out, which holds the right
half against the original's as well, and nothing is spelled out there
but two numbers. The boxes are the original's, since they name the
same lines.

**What stays of the three.** The bytes: `LoadFl`, `ChkTSR` and
`SnowMod` of the main data, with their defaults in `start.c` and in
`StPalets` of `stmain_t.c`, so a setup made of the defaults is the
original's. The handing over when the setup is left with Save
(`StExit` of `stmain.c`) is as the original's setup program does it:
the program keeps its `ChkTSR`, and takes the setup file's `LoadFl` by
way of `LoadRes`. Neither is read as a setting; `LoadFl` and `LoadRes`
are still what carries the bit for a command that is not to be shown.
And the help: the pages of the Configuration and of Screen and Mouse
that the setup shows for F1 have their paragraphs on the three, since
the help file is the distribution's.

## The portable C: what another compiler says of it

The portable C is built by one compiler, for DOS. The test
`portable_c-host` counts how far its source is from what another takes:
`harness/hostcheck.py` has gcc read every file for a machine of 64
bits, where a pointer is no segment and offset and fits no number of 32
bits, and counts each place that gcc refuses and each that only Open
Watcom reads. `harness/hostcheck.txt` has the counts of every file, and
the test holds the tree to them, so the record is always what is so:
the top of the script says how. This is what the kinds are waiting
for, with the counts of when it was written, but for the three kinds
that lost places when the entry of INT 21h went, which are as they
stand since; the test prints them all as they stand.

1,033 places in 38 of the 58 files, the three headers among them.
Twenty files are clean: the twelve of tables, `sys.h`, and `blank.c`,
`digits.c`, `line.c`, `mask.c`, `mouse.c`, `screen.c` and `string.c`.
The first count was 1,391, with two files of tables clean. 307 of what
went were the offset of a field worked out from a pointer at 0, `MD_AT`
of `tables.h` above all, which no pointer was meant by: they are
written as `offsetof` is now, `OFFSET_IN` of `vc.h`. 64 went with the
map of the memory and the watch on DOS that it rested on, `memory.c`,
its tables and half of `resident.c` (`src/portable_c/DEVIATIONS.md`,
7). And 13 came, on balance, with the functions of the system:
`dos.c` makes a segment and an offset of a pointer for DOS, and the
files that ask through it make fewer pointers of numbers.

| Kind | Places | Files | What it is | What it waits for |
|---|---|---|---|---|
| `to-number` | 595 | 33 | A pointer cast to a number of 16 or 32 bits, which does not hold it | Pointers in place of segments and offsets: see below |
| `to-pointer` | 390 | 28 | A number of 16 or 32 bits made a pointer | The same. Where the address is the machine's, the screen in `window.c` and DOS's routine for capitals in `dos.c`, the system interface |
| `layout` | 2 | 1 | `BOXHD` and `BOXMN` of `tables.h`, the header and a field of a dialog, are not as long as on DOS: each holds a pointer, which is a word there | Tables that are not held to the original's bytes where a pointer is no word: `LAID_OUT` for DOS alone, when the routines that read them take pointers |
| `function` | 5 | 2 | The address of a function kept as that of data, for a vector of an interrupt, and once the other way, the call into the PSP | The system interface |
| `interrupt` | 5 | 3 | `__interrupt`, where a function is declared or defined: `CtrlC` and `Exit05` of `resident.c`, `FatErr` of `main.c` | The system interface: what DOS enters is of DOS, and goes to a file that only the build for DOS has |
| `loadds` | 3 | 1 | `__loadds`: `Alloc0` and `Exec2E` of `resident.c` | The same |
| `naked`, `asm` | 2, 2 | 1 | The two functions of `resident.c` that are their `_asm`: `Exit01` and `Int2E` | The same |
| `asm-pragma` | 7 | 1 | The seven pieces of inline assembly of `dos.c`, a `#pragma aux` each | The same: `dos.c` is the side of that interface which is DOS |
| `registers` | 4 | 3 | A `#pragma aux` that names registers: `__U4M` and `__U4D` of `arith.c`, `CaseMapFn` of `dos.c`, `Int2E` of `resident.c` | The same for the last two. `arith.c` is Open Watcom's own, and another compiler's build leaves the file out |
| `intrinsic` | 1 | 1 | `_disable` and `_enable` of `dos.c` | The system interface |
| `data_seg` | 4 | 2 | The segment of the main data, in `main.c`, and of the stack, `stack.c` | A program that does not take the main data to be at offset 0 of its segment, which the tables and `MAIN_SEG` do; and a stack that the system gives |
| `aborts` | 8 | 3 | A `#pragma aux` that says a function does not return | Nothing: another compiler passes over the line. Four are in `vc.h`, which is not clean for them and for the `__interrupt` of `CtrlC`. A word of `vc.h` could say it to every compiler in its own way |

The script knows three kinds more, which have no place now:
`inconstant`, a length that gcc cannot work out when it compiles, which
two of those offsets were; `undeclared`, a function that nothing
declares or defines; and `other`, what gcc says that the script does
not know.

Most of the two casts are a few macros, and
`python3 harness/hostcheck.py --macros` counts them as they stand:

- `OFFSET`, 208, which 17 files define each for itself: a box, a dialog
  or a text of the tables handed on as its offset in their segment, to
  `Dialog`, `DialBox`, `Ask`, `OncZoom` and what else takes one. And
  `AT`, 154, which makes the pointer again of a segment and an offset.
  They wait for those routines to take pointers.
- `PANEL`, 148: a panel is its segment as a number, and the macro makes
  the pointer. `MAIN_SEG`, 98: the segment of the main data, handed on
  where a routine takes a panel's or that of the tables. They wait for
  a panel that is a pointer.
- `NEAR`, 50 of each kind: a far pointer that a shared routine returns,
  kept as a near one. With `__far` nothing the macro has nothing to do.
- `TRELIST`, 38: where the list of the tree begins in the main data, as
  a number.
- 144 and 58 are written out where they are, in no macro: a place of
  the main data kept as a word, `(u16)md.WinBuf`, `md.HstAdr`,
  `md.BatAdr`; DX of a call of DOS that is no pointer, written into
  `name` of `struct DosRegs`, which only `dos.c` does now; a far
  pointer made of a segment and an offset by a shift.

Since the words of the addresses are in `vc.h` with a second body for
a memory without segments ("The portable C in 32 bits: the plan", P1),
gcc reads that body, which casts nothing: `OFFSET`, `AT`, `PANEL`,
`MAIN_SEG`, `NEAR` and `TRELIST` make no place any more, no file
defines one of them for itself, and `layout` has none either, the two
lines of `tables.h` being held by what a pointer is long. What is
counted of the two casts is what does not go through those words yet,
and what is DOS's. The plan has the counts and what they do not show.

What the test does not see, since gcc only reads the files:

- Arithmetic that counts on an int of 16 bits. Two numbers of 16 bits
  are added and multiplied as numbers of 32 where an int has 32, and
  what wraps round on DOS does not there: `(u16)(a + b)` is the same on
  both, `(a + b) >> 1` and `a + b < a` are not. Another test looks for
  these since, `portable_c-int16`, with clang: `harness/int16.py`, and
  "P4, the arithmetic: done" in the plan of the 32 bits.
- A number of 16 or 32 bits read or written at an address that is not
  even, as `PUT16` does: right for the 8086 and its descendants, and
  not promised by the language.
- A pointer made of a number that is written out. gcc says nothing of
  a constant cast to a pointer: the three places of the BIOS's data,
  `BIOS_TICKS` of `system.c`, `BIOS_PAGE_OFFSET` of `window.c` and
  `KEYS_DOWN` of `keys.c`, are counted nowhere, nor is `NUMBER` of
  `tables.h`, a word of a dialog that holds a number where it could
  hold an address, in 37 lines of the tables.
- A pointer made of a number that gcc works out, for the same reason:
  `MD_PLACE` of the address of a field is a constant to it, so
  `(struct DIRENT *)TRELIST` is counted nowhere. "P2a, the places of
  the lists, the tree and the command line" in the plan of the 32
  bits has what finds these, and the one that is left.
- What a word that is taken for nothing meant. A file that is clean
  compiles; it is not thereby right on another system. `mouse.c` is
  clean and calls the mouse driver; the files of tables are clean and
  are the original's bytes only where a pointer is a word.

## The portable C: one program

The portable C stays in the memory whole while a command runs
(`src/portable_c/DEVIATIONS.md`, the first entry), and is one program:
the linker writes its file, DOS loads and relocates it, its main data is
a variable, and it has no part that stays, no copy of anything and no
tool that lays it out (the README, "Staying in the memory whole", has
what went and what is left of what rests on Open Watcom). What that has
left open:

- The memory model. The program's data is three segments, each reached
  in its own way: the compiler's group with the main data, which DS is
  on; the tables, `__far` through `AsmSeg`; and the stack, for which a
  pointer to a local variable is `STACK`. A constant of a code file is
  `CODE`, in the code segment. The README has how many places have each,
  under "The figures". With one group that
  DS and SS are both on, all of that is plain C. It is not in hand: it
  waits on the decision whether 16-bit DOS stays a target, since in a
  flat build of 32 bits these qualifiers fall away by themselves. What
  was found out about getting there, should it be done:
  - It does not fit: the main data is 62,603 bytes, the variables of
    `resident.c` and the saved setup 1,764, the tables 8,909 and the
    stack 4,096, 77 KB in a segment of 64. Some 13 KB of the main data
    have to leave it for far arrays of their own. The cheapest are not
    the tree of directories, `TreeBuf`, 12.3 KB with some 110 places
    that read it by near addresses kept in `TreAdr` and its like, but
    `UserScr`, 7,840 bytes that four places name, which makes room for
    the stack, and `ErrBuf1`, 7,392 that five name, three of them words
    of tables, which with it makes room for the tables.
  - The main data can stay where it is, at offset 0 and at the start of
    the file, with the tables and the stack behind it in the group: its
    zeros are in the file already. Much of the program takes it to be
    at 0: the tables, which name a place in it by `MD_AT`, the offset
    in the structure, 199 times; the routines that are handed
    `MAIN_SEG` for a panel and read the main data as one,
    `PANEL(MAIN_SEG)` by way of `BegBX`, `PutPrm`, `ClsLine`, `M60_02`,
    which run at every command; three calls of DOS whose DX is made of
    `&md` and a number, in `start.c` and `main.c`; and the driver of
    the tests, which reads `AutoTim` and `Clock` at the original's
    offsets from DS.
  - A buffer taken out of the middle of the main data moves what stands
    behind it, and the tables name `WinBuf` 40 times, `ErrBuf` 27 and
    `ErrBuf1` and `HelpBuf` by numbers that the data's test holds
    against the original's. The test would have to be told: a word that
    the original has as a place of the main data behind the buffer,
    and the portable C has as that less the buffer's length.
  - The stack in the group wants SS on the group when the program
    starts, and the linker has SS on the stack's own segment unless that
    is the first thing of the group, where the main data is: so some
    seven instructions at the entry, as every C library has them and as
    `Int2E` has them behind INT 2Eh, in place of `__loadds`. And the
    tables, or something else that is no code, have to stand between
    the stack and the code, for QEMU.
  - The program is entered on stacks that are not its own, where SS is
    not DS whatever the model, and a pointer to a local variable stays
    `STACK`, or far. What DOS and other programs enter always is:
    `New21` and `Int21`, where the program's own call arrives, and
    `Exit05`, where DOS goes on when it has ended the program, with
    what they call in `resident.c` and the functions of `dos.c` that
    `Exit02` asks by; that is two files, which could keep the word while
    the rest loses it. But the critical error is entered so
    too, when DOS has one while a viewer or an editor of the user's
    runs: `view.c` runs those with INT 24h still the program's, as the
    original does, so `FatErr` and everything that a box with its
    dialog, its keys, the mouse and the help calls then runs on the
    other program's stack. No scenario has that. Before `STACK` can go
    from those files, either `FatErr` goes over to the program's own
    stack when it finds itself on another, which is assembly, with the
    place that DOS keeps of the program's stack in its PSP, or the
    program gives INT 24h back for such a program, which is a
    deviation to ask for.
  - Then the qualifiers go from the rest: `STACK` can be defined as
    nothing first, in all files but those two, and taken out of its
    places when no other work has them in hand.
- Memory, on DOS. The program keeps all of itself and its panels while
  a command runs (the README has how much, under "The figures"). On the
  bench a second copy of it starts and has no memory to run a command,
  and behind a program that has stayed in the memory it does not start:
  `second-copy`, `start-memory` and `ended` have patches of the
  original's goldens that show that, and `passed-on` is not run. Whether that is
  what "functionally equivalent" allows is the user's to say. What
  would give the memory back without giving the program up again: the
  buffers of the main data that are rebuilt after a command anyway, 57
  KB of screens, boxes, the help and the tree, in the block that DOS
  gets back, which is the same work as the first item, done for all of
  them.
- `passed-on` is not run for the portable C, for want of memory on the
  bench: FreeDOS's COMMAND.COM keeps 73 KB while a program runs from it,
  having no XMS driver to swap itself out to, and the bench is the same
  for every tree and every golden. What that scenario holds of the
  entry of INT 21h, `New21`, the flags that a call is handed on with
  and the caller's stack left alone, `handed-on` held with one copy
  while the portable C had that entry. It has none any more, and takes
  neither INT 21h nor INT 27h: both scenarios have nothing to show of
  it, and neither is run.
- The file is longer than it has to be: the README has how long,
  under "The figures", and the scenarios' test holds the portable C to
  no multiple of the original's size any more. The main data's zeros
  are 62.6 KB of it, in the file because the main data
  has to be at offset 0 of a segment, the linker starts a segment on a
  paragraph only where the file starts, and nothing of C gives a
  segment an alignment. Tried: the linker's `segaddr`, which starts a
  class on a paragraph that the command names, and then writes what
  follows as if it had contents and a header that asks for a megabyte.
  Not tried: linking twice, the second time with the stack at the
  paragraph that the first shows free behind the tables, and the main
  data behind the stack, which is 4,096 bytes and so ends on one.
- No scenario has the program without memory for the viewer's 8,000
  bytes after a command, where it says "Program too big to fit in
  memory" and leaves (`TakeMem` of `src/portable_c/start.c`, `Round` of
  `main.c`), nor a DOS before 3.20, which the bench is not.
- A panel is still cut down to 256 files for a command, as the original
  cuts it to save it (`F21_40` of `src/portable_c/enter.c`). The limit
  has no reason any more; lifting it is a deviation for the user to
  decide.
- Whether Quick execute commands stays, of the Advanced options, is
  the user's to decide, and it stays whole until then: the setting, and
  `ShellRun` of `sys.h` with `Int2E` of `dos.c` behind it, by which the
  commands are handed
  to the COMMAND.COM that the program was started from, through its INT
  2Eh, in place of a second COMMAND.COM for each. The mechanism is of
  DOS alone, and has the one place where the stack is put back by
  hand. What it gives a user may not be: commands run by the shell that
  the program was started from, faster, and with what they set kept.
- Whether the phantom drive B: stays is the user's to decide, and it
  stays whole until then: what the program does for a machine whose one
  floppy drive answers as A: and as B:, of which DOS has it be one at a
  time. A user sees three things of it:
  - The prompt. Before the program touches a drive by a letter that
    shares the floppy drive with the one it goes by at the moment, it
    asks in a box, " Change drive ", "Insert diskette for drive B:",
    with Ok and Cancel: `Fantom` and `ChgDisk` of `system.c`, called at
    some fifty places in fifteen files. Ok has DOS take the drive for
    that letter, Cancel gives the command up. Without the box DOS asks
    in its own words, "Insert diskette for drive B: and press any key
    when ready", written over the panels, and waits for a key itself;
    the driver of the tests would in all likelihood not get past that.
  - The box of the drives, Alt+F1 and Alt+F2, `F62_01` of `drive.c`.
    With the byte `DriveB` of the setup 0 it offers of the two letters
    only the one that the drive goes by, A, C and D on the bench; the
    byte is "Show logical drive B:" of the original's setup program,
    which alone sets it, and 1 in the defaults. And a letter chosen
    there is made the drive's without the prompt. Without either the
    box would offer A and B both on such a machine, and every scenario
    that shows it would change.
  - "Can't read the disk in drive A:", `ErrRdy` of `main.c`, the box
    for a drive that is not ready, which every system has a use for:
    Enter there also makes the diskette that of the letter shown.
  So it is more than a prompt to change the disk, though all of it is
  of the machine with one floppy drive. Taking the prompt alone out
  would have DOS write its own over the panels. The scenarios that go
  by it: `drive-b`, `phantom`, `not-ready`, `diskette-command`,
  `delete-mask` and others type B: to get to a drive that refuses.
- EGA lines, Alt-F9, is gone, on the user's word of 13 October 2026
  (`src/portable_c/DEVIATIONS.md`, 12), with the call in it that had
  the BIOS print every line of such a screen at Print Screen: `F69_00`
  of `commands.c`, and `ScreenSmallFont` and `ScreenPrintAll` of
  `sys.h`. "The portable C: what EGA lines has left" below has what
  that left open.
- Names that say what no longer is: `resident.c`, of which nothing is
  resident; `FreMem2`, which is how many paragraphs it is from the main
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

- The program's own call (AH 2Bh, CX 'VC', DX 'OM'), by which the
  original's setup program reads and writes the setup of a program that
  is in the memory, was all that the entry of INT 21h was still there
  for when the map went. It is gone since, with the entry: the setup
  program is a part of the portable C, step 0 of the road in
  `CLAUDE.md`, "The portable C", and nothing asks a running copy for
  its data. `New21`, `Int21` and `Vect21` went with the call, and the
  interrupt is neither taken nor given back
  (`src/portable_c/DEVIATIONS.md`, 8; "Piece E1" in the plan above).
  `handed-on` and `asked` are not run for the portable C.
- When DOS ends the program, the program that started it gets the
  registers as the portable C leaves them: ES on the PSP, the low byte
  of DX what DOS was last told of Ctrl-C, and the rest as it came, DS
  among them, which is on the program that started this one. That is
  what the original leaves where its setup has TSR manager off. `ended`
  prints DS and DX, and `harness/deviations.py` leaves DS out of the
  comparison, for a line that only the portable C prints: the segment
  of the program that prints follows the size of the program under
  test. Nothing reads those registers but that scenario.
- `SetStat` is called with 0 where the program begins, as in the
  original: DOS is told to look for memory in the lowest block first,
  and not in the upper memory, whatever it did before, and is not told
  otherwise again. It was not part of the map, and stays; it is a
  thing of DOS's memory all the same.
- In `mouse-menu` the click that tried a dark item of the Commands
  menu, at a place of the screen, was on the line across the menu in
  the portable C while its menu was one line shorter, and nothing was
  chosen either way. With EGA lines gone too the place is Menu file
  edit, and the tree's click is put where the dark item is, two rows
  further up (`in_place_in` of `harness/deviations.py`): a dark item of
  that menu is clicked for the portable C again. A dark item of the
  Files menu is tried by its key, in `menu`.
- The original's help file, which the portable C reads, has its pages
  on the map of the memory, on Memory allocation and on EGA lines
  still. The file is the original's; a help of the portable C's own is
  not in hand.

What the items below say of the part that stays in the memory, under
"Known and not dealt with", they say of the C translation, `src/c/`.
Of the portable C these no longer hold: what is said of the second
copy, of the check sum and of the second way of the entry; the five
places "Where the part that stays in the memory is not as the
original", of which the first, the stack that the watch takes of
whoever calls DOS, holds for the portable C as it is written there;
"Where the size of the translation becomes other behaviour", which is
the second item above now; and "How the part that stays in the memory
is laid out by C".

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
- `md.TstEGA` is set and read in `IniData` and nowhere else now. It
  keeps its byte, as the main data is laid out as the original's.
- The key bar 9 of `KeyB_T` in `bars.c` is an empty place in the table,
  where the original has the bar of the panels for a display without
  EGA lines. It was not taken out: a bar is known by its number in the
  headers of the dialogs and in the code, and 10 to 14 would each be
  another. The row of the names that it had is out of `KeyC_T`, since
  only `KeyB_T` names a row.
- Print Screen on a tall screen prints what whoever made it tall told
  the BIOS: the program says nothing of it. Nothing was tried.
- `MouseRows` of `sys.h` is asked only where the screen has other rows
  than 25, by `IniMous` of `mouse.c` and by the setup's start in
  `stmain.c`: no scenario of the portable C has it asked now. The toy
  of `harness/trial32` still asks it of both `dos.c` and `dos32.c`.

What the work met, for whoever takes the next thing out:

- A second line out of a menu moves a click as well as a key. Step 5
  of `mouse-menu` clicks a dark item at a place of the screen; with one
  line gone the place was the line across the menu, where nothing is
  chosen either, and nobody had to look. With two it is Menu file edit,
  which the click chose: a box came up and the rest of the scenario
  went astray. `python3 harness/compare.py build/portable_c/VC.EXE`
  showed it as a screen with a box that no entry explains. The step is
  now put where the item is (`in_place_in`), and the pointer's cell is
  in the patch.
- A scenario written twice in one table of `harness/deviations.py`,
  `"menu-items": {...}` in `left_out_of` for 7 and again for 12, is the
  second alone: Python takes the last of two keys of a dict and says
  nothing, and the steps of 7 were pressed again. Add to the entry that
  is there.
- A command that changes what DOS has shown changes screens long after
  it. The original's two steps of EGA lines in `menu-items` leave
  nothing of DOS's screen, and six steps on, where a panel is off, the
  golden has an empty half there and the tree its banner: two rows of
  a patch. The other four scenarios that pass through 50 rows have
  panels over that half, or nothing in it, and need none.
- The data's test takes an entry for a table that is renumbered as one
  byte for each cell: `_cells` of `harness/datacheck.py` writes those.
  The cells were worked out by a script that read the table of
  `src/c/bars.c`, which is the original's, and the table of
  `src/portable_c/bars.c`, and printed where they differ.
- A page out of the toy of `harness/trial32` is a step out of `STEPS`
  of `both.py`: Tab went, which turned the page of the small font.
- No function of a batch ran on the tall screen alone: the scenarios'
  test of each build passes with nothing more in `harness/ported.py`
  than `F69_00` under `GONE`. What only a tall screen runs is branches
  of functions that run anyway, which that test does not count; they
  are listed below.
- A flaw of the tests under load, seen once: in the whole suite, with
  the tests of four other checkouts on the same four processors and a
  load of 37, `blank-timed` of `portable_c32-scenarios` was given up
  at its step that waits 1,300 ticks, "the program did not come to wait
  for a key within 300 s", with the stars of the screen saver on the
  screen as they should be, and the test failed for a scenario of the
  record. Beside one other test the scenario takes 136 seconds, and in
  the whole suite run again, with the others still testing, it passed
  in 332. "Piece H3" has why a machine's second is long while this
  program idles; the 300 seconds that a step has are near what that
  one step takes on a machine as busy as that.

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

## The portable C: the system's functions

What the program asks of the system it asks through
`src/portable_c/sys.h` (the README, "The system in one place"), and
`dos.c` alone knows how DOS is asked: the 348 calls that handed a
structure of registers to an interrupt went over a file at a time, the
last of them with `resident.c`, the drive B: that shares the unit of A:,
EGA lines and the extension file of a directory, and the functions that
took such a structure are gone, with the structure, which is `dos.c`'s
own. `python3 tools/calls.py` counts none. `CLAUDE.md`, "Moving a file
to the system's functions", has the table from DOS's services to the
functions, what the work met, and how a function is added.

`sys.h` is a seam for now and not the last word: its functions are
shaped as DOS and the BIOS have things, and what follows is what is of
DOS and is not behind it yet.

- DOS's critical error, Ctrl-Break and Ctrl-C. The program takes the
  three interrupts itself, by `Vector` and `SetVector` of `dos.c`, which
  are of DOS alone and declared in `vc.h`, and DOS enters `FatErr` of
  `main.c` and `CtrlC` of `resident.c`. They are to be something that
  `dos.c` raises and the program answers: the entries in `dos.c`, with
  one function of the program called for a device that fails, which is
  told the drive or the device, what is wrong and whether DOS lets it
  try again, and answers whether to; and functions of `sys.h` by which
  the program takes them and gives them back. `ErrDat` is then `dos.c`'s
  own, between its entry and the functions that are asked with the
  program's errors, and `BlocEr` stays the program's. What has to be
  kept: the order in which DOS is asked, which is both vectors read and
  then both set in `HookVct`; and that `SetBrk` of `resident.c` names
  `CtrlC` too, for DOS's Ctrl-C.
- What DOS does when it ends the program was `Exit01` and `Exit05` of
  `resident.c`, with `TakeEnd`, which had the PSP name `Exit01`, and
  `Exit02`, which had DOS look for Ctrl-C as before and left the
  registers for the program that started this one. Since the piece S3
  of the plan for the 32 bits the two entries are `dos.c`'s, with the
  registers, behind `ProgramEndedTake` and `ProgramEndedGive` of
  `sys.h`, and `Exit02` was the function of the program that was called
  there, until the program of 32 bits had to do the same and could call
  nothing: now the program says beforehand what DOS is to be told,
  `ProgramEndedBreakCheck`, `Exit05` tells it, and `Exit02` is a line
  of `Exit00` ("E, the end by another's hand" of the plan for the 32
  bits). The program's own call of DOS was here too, with its entry of
  INT 21h, `New21` and `Int21`, and `PutVector` of `dos.c`, which wrote
  into the machine's table of interrupts: they went when the setup
  program had been ported, as said above under what the map of the
  memory has left.
- The start of the program on DOS was `Alloc0` of `resident.c`. Since
  S3 it is `Alloc0` of `dos.c`, which asks for the PSP and calls `Main`,
  and what the program does first is `Alloc3` of `resident.c`, which
  asks through `sys.h`: the version, `ProgramEndOld` for a DOS before
  3, and `MemOwn` for the block of the program made its own size. The
  PSP is read in `dos.c` alone, since S1 for `ProgramTail` and
  `ProgramEnvironment`.
- Quick execute was `Int2E` and `Exec2E` of `resident.c`: COMMAND.COM's
  own interrupt, called in assembly, with the stack set by hand behind
  it. Since S3 it is `ShellRun` of `sys.h`, which comes back, with
  `Int2E` in `dos.c`. The feature stays whole until it is decided
  (above).
- `MemSetSearch` and `MemSetUpper`, by which `SetStat` has DOS look for
  memory in the lowest block first when the program begins, are
  functions of `sys.h` that only a PC with DOS has a use for: each goes
  with what it is for, if that goes. `ScreenPrintAll`, by which EGA
  lines told the Print Screen key of the rows, was a third, and went
  with EGA lines, as `ScreenSmallFont` did. `ScreenEga` and
  `ScreenFont` are what is left of that kind for the screen: the start
  asks them for the rows, and they go when the rows come from the
  system.
- `Scroll` of `window.c`, the original's `Int10m`, is the one routine of
  the program that was the BIOS called with any registers; it takes
  what it means now, and its seven callers were changed with it.

What it cost: `dos.c` was 2.6 KB longer for the first 80 functions, and
the callers gave back what they had of registers as they went over. The
last 35 calls, with the functions of the old way that went behind
them, gave back 738 bytes. The README's figures say where the file
stands.

## The portable C in 32 bits: the plan

Step 1 of the road (`CLAUDE.md`, "The portable C"): the portable C
becomes a 32-bit program for DOS, built by Open Watcom's `wcc386` with
DOS/32A, and judged on the same bench by the same scenarios; and what
of step 2 has to be settled with it. This is the plan, made on 13
October 2026 at `c0b3cad`, before any of it was begun, by reading the
tree and by trials that compile and link and run nothing. What the
bench does with such a program is in the section behind this one, "what
a trial on the bench showed", and is not said again. Two scripts of
this plan's trials were kept with the toys, in `harness/trial32/`,
`casts.py` and `whole.py`; the third, `int16.py`, was there too until
P4 made the test `portable_c-int16` of it, `harness/int16.py`, and
`whole.py` until B2 had the build link the program. A count below says
which of them counted it.

### In short

- The same source is built twice, by `wcc` and by `wcc386`, from the
  first piece to the last. Nothing is cut over.
- In step 1 a segment stays a number and an offset stays a number. The
  32-bit program has the 16-bit program's memory inside its flat one: a
  megabyte counted in paragraphs from 64K below the main data, in which
  the main data, the tables, the panels and the viewer's text are where
  their numbers say. Pointers in their place are step 2, made under the
  32-bit build alone, with the bench judging every piece of it.
- So of the 1,040 places where a pointer is kept in a number or made of
  one, 737 need no change where they stand: they go through a dozen
  macros, which get a second body. Some 190 lines are changed by hand,
  and 96 places are of DOS and go into `dos.c`.
- All of that is done under the 16-bit build first, and most of it
  leaves `VC.EXE` byte for byte as it was, which is a check that takes
  a second.
- `dos.c` stays the 16-bit one and `dos32.c` is written beside it,
  behind the same `sys.h`, and judged by toys on the bench before the
  program can run: 71 of the 84 functions go straight through the
  extender, 7 go through real mode, 6 change.
- The 32-bit build is first run when all of that has landed, and is
  then held to a record of the scenarios that it passes, which may only
  grow.
- 20 pieces can be named, and the convergence behind the first run is
  guessed at three to six more. The first is P1; D1 and H1 start beside
  it.
- Nothing is past return before the last piece, which deletes the
  16-bit build and is the user's to release.

### Taken by the session, on 13 October 2026

Ways of doing step 1 that the road in `CLAUDE.md` leaves open. They
are not the user's word: the session that runs the work took them, the
step being one that follows the setup program without waiting for the
user. The numbers are those that the two lists have between them.

| | What | Taken |
|---|---|---|
| 1 | Whether step 1 keeps segments and offsets as numbers, or has pointers first, under the 16-bit build: see "The strategy" | Numbers in step 1, in a megabyte of the program's own inside the flat memory. They become pointers in step 2, under the 32-bit build alone. It is scaffolding of some 40 lines of `vc.h` and 30 of `dos32.c`, which step 2 takes down again |
| 2 | Where the panels and the viewer's text are in step 1. With 1 they have to lie within that megabyte, so they cannot be DOS's memory below the megabyte, nor a block that the extender gives wherever it likes | A stretch of the program's own, 640 KB, that the three functions of `sys.h` for memory hand out as they hand out DOS's today. A command then finds all of DOS's memory free but the extender's 52 KB |
| 5 | Quick execute commands, which is the user's to decide and not decided (`TODO.md`, "The portable C: one program"). Under the extender COMMAND.COM's interrupt is called in real mode and comes back; the shell of the bench does not serve it either way (`quick`) | It stays whole through step 1, some 40 lines of `dos32.c`. Should the user decide that it goes, the hardest part of S3 and D2 goes with it |
| 6 | Release and debug, which the user asked for at this step | Two programs of every build: `VC.EXE`, compiled `-os` without debugging information, which is shipped and which every test judges; and `debug/VC.EXE`, compiled `-od -d2` with checks in the macros of the addresses, built always and run by hand. See "The build" |
| 7 | Where DOS/32A's licence text is kept, which the road has in the repository and the trial left open as to where | `LICENSE.DOS32A` beside `LICENSE`, the text of `binw/license.d32` as it is, and the sentence that the road has the README say, under a heading of its own |

Not taken here, since `CLAUDE.md` has them: the extender is DOS/32A,
its stub is linked from the pinned download and never committed, the
file is held to no length, and this step follows the setup program
without waiting.

### Open, for the user

To be asked when the work reaches them, and not before.

| | What | When, and what is thought of it |
|---|---|---|
| 3 | What deviation 1 says, the program staying in the memory whole, once the program is above the megabyte and a second copy has the memory to run again. That moves toward the original. `second-copy`, `start-memory` and `ended` have patches that say a second copy has none | At W, with the three scenarios as the 32-bit build shows them. Thought: declared as it is found, where the difference is of memory alone, and the three patches made anew and read |
| 4 | What the 32-bit program can do when DOS ends it by another program's hand. The 16-bit program has its say first, through its PSP, and leaves registers for the program that started it: `ended` prints them. Under an extender DOS would seem to go back to that program through the extender's own part of real mode, with nothing of the program run. That is from reading and was not tried | The piece that meets it, D2 or M1, tries it first and reports, and the user is asked with what was seen. Thought: declared, and `ended` not run for the portable C; `TakeEnd`, `Exit01` and `Exit05` go with the 16-bit build. **Tried by D2**: nothing of the program runs, but the extender takes itself out and leaves every interrupt of real mode as it found it; what is not put back is that DOS looks for Ctrl-C at every call, and whoever ended the program sees DOS's registers. "Piece D2" below has the table and the ways that there seem to be. **Decided on 13 October 2026**, the second of those ways: the program of 32 bits is told as the 16-bit one is, by a small routine of real mode that the system's functions keep for it (`CLAUDE.md`, "The portable C"). **Done**: "E, the end by another's hand" below; `ended` passes for both programs with the one patch |
| 8 | That the 16-bit build may go, which the road keeps for the user | Before Z1, when both builds pass every scenario from one source |

### The strategy

| Way | For | Against |
|---|---|---|
| A cut-over: the source changed until `wcc386` builds a program that runs, with the 16-bit build let go on the way | The least written twice | Against the rule: every step builds for DOS and passes its scenarios before it lands. And nothing would judge the tree between the first piece and the last |
| Pointers first: step 2 under the 16-bit build, a panel a far pointer, a place of the main data a pointer, then the switch | The switch itself is small | A far pointer in the main data is four bytes where a word is, and the main data's layout is held: by the setup file, by the driver of the tests, by the 284 words of the tables that say `MD_AT`. A table handed on as a far pointer costs a relocation of four bytes in the file at each of 231 places. And each of those pieces is judged by a build that is about to go, in a memory where a pointer has a segment that C knows nothing of |
| Two programs of one source, with the numbers kept. Taken | Every piece before the first run is the 16-bit program unchanged, and most can show it by the bytes of the file. The 32-bit program is the same program, so what its first run fails is what 16 bits hid and not a new mistake of translating. Step 2 is then made where pointers are plain, and the bench judges each piece of it | What gives the numbers a meaning in 32 bits is written for step 1 and taken out in step 2. A place where the 16-bit program counted on an offset wrapping round at 64K is not found by reading: see "What could go wrong" |

What only the 16-bit build needs, and where it stands so that it is
nothing in 32 bits and goes with that build:

| What | How much | Where it is | In 32 bits |
|---|---|---|---|
| `__far`, `FAR`, `STACK`, `CODE` | 790, 62, 262 and 96 places (`harness/hostcheck.py`) | Words in the source | Nothing already: `vc.h` defines them away where `__I86__` is not set. The words go by a script in the last piece |
| The macros of the addresses | 5 names that files defined for themselves, 41 definitions in 23 files, and 4 of `vc.h` | Since P1: one block of `vc.h` with two bodies, under `#ifdef __I86__` | The second body. The first goes with the build, the second in step 2 |
| The three groups of the data, the two code segments, `#pragma data_seg` | `CMakeLists.txt`; 4 pragmas in `main.c` and `stack.c` | The build. The pragmas were to be under `__I86__` after S3 and are not: a flat program needs both, see "S3" below | One flat segment of each kind, by the linker's order, with the class `STACK` for the stack and `MAIN` for the main data, which the two pragmas give them |
| `arith.c`, the compiler's two helpers | 1 file | Not built for 32 bits | `wcc386` multiplies and divides without them |
| Interrupt functions, `__loadds`, naked functions, `_asm`, registers in `#pragma aux` | 5, 3, 2, 2, 4 and 7 places when the plan was made; 4, 1, 2, 2, 4 and 7 since S3 | In `dos.c` alone since S3, but for the two registers of `arith.c` | `dos32.c` has its own, as inline assembly of 32 bits |
| `LAID_OUT(BOXHD, 10)` and `LAID_OUT(BOXMN, 11)` | 2 lines of `tables.h` | Nowhere, since P1: the two lines say 8 and 9 bytes and a pointer, which is 10 and 11 on DOS, and hold every compiler to it | 12 and 13 bytes: see "The tables" |
| `dos.c` | 1,148 lines | A file that only the 16-bit build has | `dos32.c` |

### A pointer in a number, by what it is

`harness/hostcheck.py` counts 1,035 places where a pointer is cast to a
number of 16 or 32 bits or made of one, and 5 where it is a function's
address; `harness/trial32/casts.py` asks gcc the same way and sorts
them by the macros that each is made through and by the words of its
line. It is a sorting by rule, and a few lines are in the wrong class.

| | Class | Places | Files | What it is | In step 1 | The work, and whether under 16 bits first |
|---|---|---|---|---|---|---|
| T | A table by its offset | 326 | 22 | `OFFSET(Conf_B)`, 231 times, and `AT(code, offset)`, which makes the pointer again: a box, a dialog or a text of the files of tables, handed to `Dialog`, `DialBox`, `Ask` and their like as its offset in the tables' segment | Stays an offset: from the paragraph 32K below `Title_S`, which is the tables' segment as `AsmSeg` answers it. The tables are 25 KB in a flat program, with the constants of the code beside them | None where they stand: the two macros get a second body. Yes, P1. But the 40 places of `dialog.c`, `main.c` and `stcolor.c` that read a pointer out of a table as a word are P3 |
| N | A place of the main data as a number | 210 | 28 | A near pointer, which on DOS is its offset in the main data: `(u16)md.WinBuf`, `(u8 *)md.BatAdr`, `md.HstAdr`, the tree's `TRELIST` | Stays a number: the offset in `md`. `MD_PLACE(pointer)` and `MD_POINTER(place)`, which are the two casts on DOS and take away or add `&md` elsewhere | 147 written out on 138 lines, each wrapped in one of the two words; 63 made by `TRELIST`, `MD`, `MDW`, `WAS` and `FIND`, whose definitions change. Yes: P2a, P2b, P3 and S3, by file |
| P | A panel by its segment | 150 | 22 | `PANEL(panel)`, 148 times, and `WCB_OF(file)` twice | Stays a number of paragraphs. `PANEL` adds it to where the megabyte begins | None, but the two of `WCB_OF`, which finds the panel in the segment of a far pointer and has to be handed it. Yes, P1 |
| R | A far pointer kept as a near one | 108 | 9 | What `MvzLine`, `AddFile`, `CutPath` return, known to be in the main data: `NEAR(pointer)`, 94 places, and 14 written out on 7 lines | The same pointer | The macro's second body is a cast; the 7 lines get the macro. Yes, P1 |
| M | The main data's segment as a number | 105 | 22 | `MAIN_SEG`, handed where a routine takes a panel's segment or the caller's: `Dialog` 48 times, `DialBox`, `ClsLine`, `Ask` | The number 1000h: the megabyte is counted from 64K below the main data so that it is a constant | None. Yes, P1 |
| D | Of DOS | 96 | 5 | A far pointer taken apart for DOS's registers or made of what DOS answers, 69 in `dos.c`; and 27 outside it, in `resident.c`, `start.c`, `main.c` and `view.c`: the PSP read by its offsets, an interrupt's address, the block that a program is run with | Not in the common code at all | S1 to S3 move the 27 outside `dos.c` into it or behind new functions of `sys.h`. `dos.c` keeps its own until it goes. Yes |
| V | The viewer's, the editor's and a copy's memory by segment | 39 | 7 | `TEXT(segment)`, `AT(address.segment, address.offset)` | Stays a segment and an offset | None: three that are written out with a shift get `AT`. Yes, P1 |
| G | A segment handed on, the main data's or a panel's | 6 | 2 | `AT(segment, field->mPoint)` in `dialog.c` | The same | None: one written out in `bars.c` gets `AT`. Yes, P1 |

That is 737 places that change by a definition (286 of T, 148 of P,
105 of M, 36 of V, 5 of G, the 94 of `NEAR` and the 63 of N that a
macro makes), 207 by hand, on some 190 lines (147 of N on 138 lines,
14 of R on 7, the 40 of T, 2 of P, 3 of V, 1 of G), and the 96 of DOS.
After P1 the host check was to count some 310 of the 1,040, since gcc
reads the second bodies, which cast nothing. It counts 249: "P1, the
words in one place: done" below says why it is fewer. When P2, P3
and S3 have landed it counts those of `dos.c` and no others. So its
record says how far the preparation is, file by file, and a file that
has gained a place has a cast outside the words.

What the host check does not see is said under "Arithmetic" and under
"What could go wrong".

### The memory

The megabyte: a segment `s` and an offset `o` are the address
`(u8 *)&md - 10000h + 16 * s + o`. Tried with both compilers:
`wcc386` takes such second bodies with warnings as errors, `MAIN_SEG`
being the constant 1000h, and gcc for 64 bits says nothing of them.
Whatever is named by a segment has to lie between the main data and
983 KB behind it, which the linker's order sees to. The bodies as
P1 wrote them into `vc.h`, where each has what it takes for granted
written at it:

    #define ARENA ((u8 *)&md - 0x10000L)
    #define MAIN_SEG 0x1000
    #define AT(segment, offset) \
        (ARENA + ((u32)(u16)(segment) << 4) + (u16)(offset))
    #define SEG_OF(pointer) \
        ((u16)(((const u8 *)(pointer) - ARENA) >> 4))
    #define OFFSET_IN_SEG(segment, pointer) \
        ((u16)((const u8 *)(pointer) - AT(segment, 0)))
    #define PANEL(panel) ((struct WCBdef *)AT(panel, 0))
    #define TABLES_BY(text) SEG_OF((text) - 0x8000)    /* AsmSeg */
    #define OFFSET(object) \
        ((u16)((const u8 *)(object) - AT(TABLES_BY(Title_S), 0)))
    #define MD_PLACE(pointer) \
        ((u16)((const u8 *)(pointer) - (const u8 *)&md))
    #define MD_POINTER(place) ((u8 *)&md + (u16)(place))
    #define NEAR(pointer) ((u8 *)(pointer))

| What | Today | In the 32-bit program of step 1 | In step 2 |
|---|---|---|---|
| The main data, 62,603 bytes | `md`, at offset 0 of the group that DS is on, in the file as zeros | `md`, a variable, anywhere; its layout as it is. `MAIN_SEG` is 1000h | The same variable. Its layout is free once three things no longer go by it: the driver, which takes the two addresses from the map by name then; the tables, which name a field by its address; and the words of it that held places and segments, which are pointers outside the part that the setup file is an image of |
| A place in it | A near pointer, or that as a word | A pointer, or its offset in `md` as a word | A pointer |
| The tables, 20,850 bytes | A group of their own, reached far through `AsmSeg` | Constants like any others, 25,131 bytes with the constants of the code; known by offsets from 32K below `Title_S`. The test of the layout holds them all to lie within 32K of it | Named by pointers: `Dialog(&Conf_B, ...)` |
| The stack, 4,096 bytes | A group of its own; a pointer to a local variable is `STACK` | Data of the program, 16 KB, in the file as zeros so that the tests know what it holds when loaded; the header of the file names it | The same |
| The panels and the viewer's text | One block of DOS's, all that there is, cut up in paragraphs: `Init11`, `GiveMem`, `TakeMem` of `start.c` | The same code and the same paragraphs, in 640 KB of the program's own that `dos32.c` has behind the main data, without a value and so not in the file. `MemTake`, `MemGive` and `MemResize` answer for it as DOS answers, the largest block with a refusal | Blocks that the extender gives, DPMI 0501h, as pointers; the three functions answer in addresses |
| A panel's segment in the main data: `WCBcrn`, `WCBsec`, `WCB1seg`, `WCB2seg`, `ViewSeg`, `MemSeg`, `FreMem2` | Words | Words: paragraphs of the megabyte | Pointers, in variables outside the laid-out part |
| The same in the setup file | None: the file is three letters, of each panel what a `WCBdef` has before `CurAdr`, of the main data what it has before `LoadRes`, and a sum; no word of those is a segment or a place | The same file | The same file |
| The copy of the screen, the boxes' and the help's buffers | Arrays of the main data, named by their offsets in 82 words of the tables | The same | Named by address |
| The screen itself, the BIOS's data | Far pointers of `dos.c`, B800:0 and 40:6C | Flat addresses of `dos32.c`, B8000h and 46Ch | The same |
| The PSP and the environment | Read by their offsets through `PspSeg`, in `resident.c`, `start.c` and `main.c`. Since S1 in `dos.c`, and since S3 the place where DOS goes on too: `PspSeg` is static there | Below the megabyte and outside the program's own: `sys.h` hands out the command tail, the environment and where the memory ends, S1 | The same |

What keeps its layout through step 1, and who holds it:

- The main data: `LAID_OUT(MainDat, 0xF48B)`, for both compilers. The
  setup file is an image of its start; the driver reads `AutoTim` and
  `Clock` at the main data's address, which the harness takes from the
  linker's map, plus the offsets of the original's listing; the tables
  name places in it by `MD_AT`.
- A panel, `LAID_OUT(WCBdef, 0x1A62)`: the setup file has its start.
- The tables, but for the words that name something.

**The tables.** A box's lines, a dialog's box and its buttons, the
table of the menus' tables, `Menu_T`, and the pictures of the setup's
colours, `StPagTab`, are written in C as pointers already. On DOS such
a pointer is a word, the offset in the tables' group, which is what the
original has there. `wcc386` makes it four bytes with a relocation:
`harness/trial32/whole.py` compiles all 16 files of tables once the two
lines of `LAID_OUT` say 12 and 13. The objects of `find_t.c`, read
byte by byte, are laid out as they are written, one behind the other
with nothing between: 90 bytes where the 16-bit build has 84, for the
three pointers of its dialog. A word that holds a number where it
could hold an address, `NUMBER(LenStr)`, is four bytes without a
relocation.

So in 32 bits a word that names something is a pointer, and the test
of the data compares an object field by field instead of byte by byte:

- It knows from the declaration which fields are pointers. There are
  five shapes: `BOX(n)`, `BOX1(n)`, `BOX2(n)`, `DIALOG(n)`, and an
  array of pointers; everything else is bytes.
- A pointer with a relocation is held against the original's word as
  today, by the name of what it points at and the distance from it. The
  relocations are in the file, where the reader of LE files finds them.
- A pointer without one is a number: its low word is the original's
  word and its high word is 0.
- A field that holds an offset in the main data is compared as a
  number, as today. Written so that it may also be a relocation that
  names `md` and that offset, it serves step 2 unchanged.
- Every other byte is compared as today.

The routines that read a table have to read a pointer as a pointer:
`DialBox` steps through a box's lines two bytes at a time and `Dialog`
takes `head->pDiBox` for a word. That is P3, under the 16-bit build,
where a near pointer read out of a far table is still the word.

### The system's functions

Two files behind one `sys.h`: `dos.c`, which stays what it is and goes
with the 16-bit build, and `dos32.c`. One file with two bodies would
share little: all seven pieces of inline assembly differ, the structure
of registers differs, and a call through real mode has no likeness in
16 bits. What they share is the order in which DOS is asked, which is
behaviour and is carried over a function at a time.

The 84 functions that `sys.h` had when this was written, by what the
trial found of DOS/32A. It has 85 now: with S1 `ProgramPrefix` left
it, and `ProgramTail`, `ProgramEnvironment` and `MemEnd` came, which
read the PSP below the megabyte and ask DOS nothing; and `MemEnd` has
gone again, with the Info panel's two lines on the memory. And 93
since S2, with the seven by which the program notes, takes and gives
back the critical error, Ctrl-Break and Ctrl-C, which are of none of
the three kinds below, since they ask the extender and not DOS; and
with `IntOn`, which asks nobody. And 98 since S3: `MemOwn` and
`ProgramEndOld`, which have nothing to do under the extender;
`ProgramEndedTake` and `ProgramEndedGive`, which are of the PSP and
ask DOS nothing; and `ShellRun`, which goes through real mode as the
seven of the second kind do, by a routine there and not by a service.
And 99 since the end by another's hand was made for 32 bits:
`ProgramEndedBreakCheck`, by which the program says what DOS is to be
told of Ctrl-C then, where it had a function of its own called.

| Kind | Functions | Which |
|---|---|---|
| Straight through: no address, or one that the extender translates, with the program's pointer as it is. D1 found six of these to want real mode: `FileSetAttributes`, `FindRecord`, `FindFirst`, `FindNext`, `DirCurrent` and `DirCurrentBare`. See "Piece D1" | 71 | The files but `FileTrueName`, 16; the directories, 8; the drives, 8; `TimeOfDay`, the two versions, `BreakCheck`, `SetBreakCheck`, `Idle`; `ProgramEnd`; the console, 2; the keyboard, 4; the screen, 17, of which four read and write the display's memory and the BIOS's data at flat addresses; the mouse, 8; `Ticks` |
| Through one block below the megabyte and a call in real mode, DPMI 0300h, or 0301h for a routine | 7 | `FcbParse` (29h), `FcbDelete` (13h), `CountryInfo` (38h), `CaseMap`, which calls the routine that 38h names, `FileTrueName` (60h), `ProgramRun` and `ProgramRunBare` (4Bh) |
| Other in 32 bits | 6 | `MemTake`, `MemGive`, `MemResize`: the program's own 640 KB. `MemSetSearch`, `MemSetUpper`: passed to DOS as they are. `ProgramPrefix`: gone from `sys.h` with S1, and from the common code with S3 |

Of the screen's 17 in the first row, two went from `sys.h` with EGA
lines, `ScreenSmallFont` and `ScreenPrintAll`
(`src/portable_c/DEVIATIONS.md`, 12), which leaves it two functions
fewer than the count above says: the README has the count as it
stands, under "The system in one place".

What is 16 bits in kind and changes shape for both builds, under the
16-bit build first:

| What | Today | After | Piece |
|---|---|---|---|
| `ProgramRun(name, block)` | `struct EPBs`: three far pointers of real mode, which `resident.c`, `start.c` and `view.c` make with `MAIN_SEG` | `ProgramRun(name, tail)`; the block is `dos.c`'s to make, with the two file control blocks where the original has them | S1, done |
| `ProgramPrefix()` and `PspSeg` | The PSP's segment, read at 0Ah, 2Ch, 02h and 81h in three files, 10 places | The command tail and the environment as pointers that `sys.h` answers with; the end of the memory as a number, if the Info still shows it then | S1, done for 2Ch and 81h: `ProgramEnvironment` and `ProgramTail`. 02h, the end of the memory, was `MemEnd` with S1 and is read no more: the Info does not show it (`DEVIATIONS.md`, 11). 0Ah, and the two themselves, are S3's: see "S1, the PSP and a program run" |
| `Vector`, `SetVector` of `vc.h`; `FatErr` of `main.c`, `CtrlC` of `resident.c` | The program takes INT 24h, 1Bh and 23h itself, with interrupt functions | Functions of `sys.h` that take and give back the critical error and Ctrl-Break, in the order of today's calls; the entries in `dos.c`, which call `FtErr1` of `main.c` as a plain function | S2, done: seven functions, and `FtErr1` is handed over where the critical error is taken and called through a pointer. See "S2, what DOS enters" |
| `Alloc0`, `TakeEnd`, `Exit00` to `Exit05`, `Int2E`, `Exec2E` of `resident.c` | The entry of the program, what DOS does when it ends it, COMMAND.COM's interrupt with the stack set by hand | In `dos.c`. `Main` is called by the entry; leaving is `ProgramEnd` with what has to be done first; Quick execute is a function that comes back, the stack kept and put back around the interrupt | S3, done: `Alloc0`, `Exit01`, `Exit05` and `Int2E` are in `dos.c`; `TakeEnd`, `Exit00` and `Exit02` stay the program's and ask through five functions that are new; `Exec2E` is gone. See "S3" below |
| The memory by segment and paragraph | `MemTake`, `MemGive`, `MemResize` | As they are, through step 1 | Step 2 |

In `dos32.c`, by the trial: the entry puts ES on the data and clears
the direction flag; one block below the megabyte is asked for when the
program starts, DPMI 0100h, for the seven functions above; the three
interrupts are taken with DPMI 0205h, and the handler of the critical
error goes over to the program's own stack before it calls `FtErr1`;
INT 22h is put back after a program has run; the vectors are given
back before the program ends, or the extender writes a warning onto
the last screen.

`dos32.c` is judged before the program can run: a toy that asks every
function of `sys.h` and prints what it is answered, built twice, with
`wcc` and `dos.c` and with `wcc386` and `dos32.c`, and run on the bench
by `harness/trial32/run.py`. The two have to print the same.

### Arithmetic

On DOS two numbers of 16 bits are added, taken from one another,
multiplied and shifted as numbers of 16 bits without a sign. Where an
int has 32 bits they are made ints first, and the result has a sign
and does not wrap. Put back into 16 bits it is the same; looked at as
it is, it is not: `a - b < c`, `(a + b) >> 1`, `a + b < a`, a product
kept in 32 bits, `a == -1`. The types being `u8`, `u16` and `u32`
everywhere, with no `int` in the tree but in two casts, and 244 places
that say `(u16)(` already, most of the code is the same in both.

The plan's script `int16.py` finds the rest: clang reads a file for a
machine whose int has 32 bits, and the script walks every expression
with its type. Held against nineteen lines written for it, it found
the nineteen. Of the 47 files of code it counted, in 34 of them, as
the tree stood when the plan was made; what P4 made of the script
counts otherwise, and "P4, the arithmetic: done" below has why:

| Kind | Places | What |
|---|---|---|
| compare | 17 | `if (item > sd.ColNum - 1)`, `if (cursor - first >= place.lines)` |
| divide | 3 | |
| shift | 9 | `key = item - table >> 1`, `Below` of `blank.c` |
| widen | 21 | The result becomes a number of 32 bits |
| address | 2 | The result is made a pointer; the host check has these too |
| index | 73 | `text[i - 1]`, `screen + (width - 2)`: the same in both builds unless the number leaves 16 bits, since one below 0 steps back in both |

So some 50 places to read and 73 to look over, which is one piece, P4.
Where a place can differ, `(u16)(a - b) < c` says what DOS does, in
both builds, and compiles on DOS to the code that it had. P4 makes the
script a test with a record, as the host check is. It did: 79 places
as its finder counts them, of which 21 got the cast and 58 are in the
record with their reasons.

The compiler's own warnings do not do it. Asked of the same files for
the same machine, clang's `-Wsign-compare` names no place, since both
sides of such a comparison are ints by then; and `-Wconversion` names
244, those where a result is put back into fewer bits, which are the
places that are the same in both builds. Its
`-fsanitize=implicit-conversion` wants a program that clang builds and
that runs, which is step 6 of the road.

What neither finds:

- A number of 16 bits that has wrapped round already and is then added
  to a pointer: FFFEh for two back. On DOS the pointer wraps round with
  it. In a flat memory it lands 64K further on. Where the number goes
  through `AT` or `MD_POINTER` first, the megabyte does what DOS does;
  where it is added to a pointer, it does not. Since P4 `ENTRY` of
  `vc.h` and `WORD_AT` of `view.c` cut what they are handed too, a
  panel and the viewer's text being where their segments begin.
- A far pointer stepped past the end of its segment, which begins
  again at 0 on DOS.
- What is read behind an object, or at an index that the program does
  not keep inside it: the bytes that follow are others, as they are in
  the portable C of 16 bits against the original already.

Of step 2, what waits until the 32-bit build passes and the 16-bit
build is gone, and would do harm mixed into step 1:

- A variable made wider. It changes what happens where the number
  wraps, which is the point of it, and the 16-bit build cannot follow.
- A pointer where a number is, in the main data or in what a routine
  takes. See the strategy.
- A boolean. Harmless and no use here: it changes the bytes of the
  16-bit file, and the check that most pieces of step 1 go by is that
  they do not.
- The words `__far`, `STACK`, `CODE` and `FAR` taken out: the 16-bit
  build needs them to its last day.

### The build

- `wcc386 -3r -mf -os -s -zl -zls -wx -we`: the flat model, small code
  as today, no check of the stack and no library. `vc.h` includes
  `stddef.h` for `offsetof` where `__I86__` is not set, and the build
  names the compiler's directory of headers for that one, `-i=`. The
  plan had `vc.h` write it out for `__WATCOMC__` as the 16-bit build
  does, so that no directory is named; B1 tried that, and it compiles,
  and left it: see "B1, the objects" below.
- `wlink format os2 le`, `option stub=` with `binw/dos32a.exe` of the
  pinned download, `option start=`, and an `order` line. The plan had
  the order as: the code; then the constants and the tables, which
  share the code's last page of 4K and of which 274 bytes are ever
  written; then the main data, what has a value and the stack; and
  last what has no value, the 640 KB above all, so that it is not in
  the file. The code needs another, and B2 linked that: the code; the
  stack; the constants and the tables, with what has a value behind
  them; the main data; what has no value. See "B2, the program"
  below, which has why.
- The banner bit of the stub is cleared in the file that the linker
  wrote, by a tool of ten lines of C, `tools/stub.c`: the build runs no
  Python. It is 40 lines, with what it refuses.
- Tried, `harness/trial32/whole.py`: 61 of the 63 files compile as they
  stand, all but `arith.c` and `dos.c`, with the two lines of
  `LAID_OUT` changed and warnings in `resident.c` alone, for its
  interrupt functions. Linked with a file of functions that do nothing
  in place of `dos.c`, the program is 277,132 bytes where the 16-bit
  one is 196,998: 121,807 of code against 104,521, the main data and
  the stack as zeros, and 4,440 relocations in 33,320 bytes of the
  file.

Release and debug, both built by every build, of the same source:

| | Release, `VC.EXE` | Debug, `debug/VC.EXE` |
|---|---|---|
| Compiled | `-os`, no debugging information | `-od -d2`, with `CHECKED` defined |
| What it is for | It is the program: shipped, and judged by every test | Finding where a scenario goes wrong. Its listings have the lines of the source. With `CHECKED` the macros of the addresses stop the program with a message when a place is outside the main data or a segment is outside what the program has, where the release build would read or write whatever is there: the bench has no paging, and a wild write is caught by nothing |
| The tests | All of them | None. It is built so that it does not rot, and `harness/show.py --program` runs it by hand. Unoptimised code takes more instructions, and a scenario that shows the time of day may not hold it |

The limits:

| Limit | For the 32-bit program |
|---|---|
| A code segment ends below F000h | Goes: one flat segment of code, and the near call that QEMU gets wrong is one that wraps round 64K |
| No code within 4K of the stack | In its new form: nothing that is written lies in a page that holds code. The constants stand between. As B2 linked it, the stack is in front of the constants and its low end shares the code's last page: what the test of the layout holds is that nothing which a scenario writes lies in such a page, the upper half of the stack being all of it that a scenario may use |
| Half of the stack stays untouched in every scenario | Stays, with the stack 16 KB and read by the driver above the megabyte |
| The length of the file | None, as for the 16-bit one |
| New: everything named by a segment lies within the megabyte | Held by the test of the layout, from the map: `portable_c32-layout`, `harness/layoutcheck.py` |
| New: the tables lie within 32K of `Title_S` | The same |
| New, with S3: `IniSave` ends within 64K of where the main data begins | The same |

### The harness

The trial's seven changes, set against the pieces of the program:

| | What | Piece | Has to be there before |
|---|---|---|---|
| 1 | The build | B1, B2 | B1: any later piece is held to compiling for both. B2: the first run |
| 2 | A reader of LE files and of the flat map | H1 | Everything below |
| 3 | `driver.py`: the facts of a flat program as linear addresses, and the script's second header, which became an event | H2, done | The first run that is to be judged. Before it `harness/trial32/run.py` runs the program and prints its screens, with every step that waits on `AutoTim` ended early |
| 4 | `DRIVER.ASM`: the block move for `AutoTim`, `Clock` and the stack; then every golden derived again, which must change none | H2, done: none changed | The same |
| 5 | `coverage.py` and `ported.py` on the reader, with the stub left out; the record of the scenarios that pass | H3, done | The 32-bit build can be held to anything. What ran of the C is asked only once every scenario passes |
| 6 | The scenarios' test: the stack, the limits above | H3, done | The switch |
| 7 | `datacheck.py`, with `tools/figures.py`; `reads.py` goes by the text of the C and stays | H4, done: the test `portable_c32-data` | The switch: until then the 16-bit build holds the tables, which are the same source, and both tests do |

The record: `harness/pass32.txt`, the names of the scenarios that the
32-bit build passes, written by `ported.py --record`. The test
`portable_c32-scenarios` fails when one of them fails, and when one
passes that is not in it, so that the record is what is so; a piece
commits it with the change that made it grow. "Piece H3, the scenarios'
test" below has how it is run and read.

### Piece H1, the reader: done, and what a later piece should know

`harness/le.py` reads the file of a flat program and the linker's map
of it, and the test `harness-le` holds it to a toy that it builds,
`harness/lecheck.py` with `harness/lecheck.c`. Nothing else of the
harness knows a flat program yet. The top of `le.py` says what such a
file is; this is what the pieces behind it start from.

What it gives:

| | |
|---|---|
| `le.Linear(contents)` | The file. `stub`, where the stub ends; `objects`, each with its `number`, its `size` in the memory, `at` and `held`, where its bytes are in the file and how many of them the file has, `place`, and `code`; `entry` and `stack`, each an object's number and an offset; `relocations` |
| A place | The one way of counting that everything has in common: an offset from where the first object begins in the memory. `place(object, offset)` makes one of an address of the map; `in_file(object, offset)` says where that is in the file, or None for what the file does not hold |
| `image()`, `image(base)` | The objects as they lie in the memory, from where the first begins: as the file has them, and with the address of the first object, as the extender leaves them. What the file does not hold is zeros, and so are the bytes between two objects, which are the extender's |
| `relocated()` | Which bytes of the image differ in the memory from the file: place -> the relocation there, with its `length`, 4 for an address and 2 for a selector, and what it names, `target` and `target_offset` |
| `le.BENCH` | Where the first object is on the bench of the tests, 00100010h |
| `le.Map(path)` | The linker's map, of a flat program or of one of 16 bits: `segments`, each with `name`, `kind`, `group`, the two numbers of its address, `first` and `offset`, and `size`; `symbols`, every one with its `module`, `name`, the two numbers and its `mark`, static ones included; `public`, name -> the two numbers, without the static ones; `segment(name)`; `named(name, module)`; `flat` |
| `le.Unknown` | Raised for a file that is no linear executable, or that has something the reader was not written for: a page that is not in the file as it is, a relocation of another kind than the two |
| `python3 harness/le.py FILE [MAP]` | Prints what a file has, and where its segments are on the bench |

Where the plan and the code differ:

- It is some 370 lines with the map and what it says of itself, where
  the plan had 150.
- The map is read in `le.py`, and `coverage.link_map` and
  `coverage.segments` read theirs through it: the text is the same for
  both kinds of program but for the digits. They answer for the maps of
  the 16-bit builds exactly what they answered, which was held for
  every map of both trees. For a flat map the first number of an
  address is an object's number, not a paragraph: what multiplies it by
  16 is wrong for one. `coverage.stack`, `driver.facts` and
  `ported.check_translation` and `check_layout` do, and
  `coverage.origin` and `coverage.relocated` read the header of a
  program of 16 bits: those are H2's and H3's to give a second form.
  H2 gave `driver.facts` its own and `coverage.stack` a second
  function beside it, `flat_stack`; H3 gave the others theirs:
  `coverage.place` for an address of either map, and `executed` by
  the image.
  `datacheck.py` and `arithcheck.py` each
  still read the lines of the 16-bit map by themselves; `le.Map` has
  what `Translation` of `datacheck.py` reads there, modules and static
  symbols included.
- `harness/trial32/whole.py` keeps its own twenty lines: it counts the
  records of the file by their length, which the reader does not give.

What was seen, of the files that there are:

- `whole.py`'s file, 277,132 bytes: the stub is its first 27,536; two
  objects, the code, 121,807 bytes, and everything else, 92,712, each
  as long as the map's segments in it add up to; the entry and the top
  of the stack as the linker's summary has them. Its 4,440 records are
  4,437 relocations: three addresses lie across the end of a page and
  are in the file twice. 4,435 are addresses, 3,689 in the code that
  name data, 186 in the code that name code, 555 in the data that name
  data and 5 there that name code; 2 are selectors, in `Exit01` and
  `Int2E` of `resident.c`, which go with S3. At every one of the
  addresses the file has the offset of what is named.
- The stub in a program is 32 bytes longer than `binw/dos32a.exe`, by
  its header, which the linker writes anew; behind the header it is
  that file byte for byte, and its header gives its length. The test
  holds both.
- An address in front of the object that it is counted from, as the
  megabyte's beginning is 64K in front of the main data, is written by
  the linker as an offset counted round 32 bits, FFFF00A0h for 64K in
  front of A0h, and the extender puts the right address in. The reader
  gives such an offset with its sign.
- The pages of the file are not the pages of the memory. An object
  begins where the one before ends, rounded up to 16 and 16 more, so
  the data begin in the middle of a page of 4K of the memory. A limit
  that is about pages, "nothing that is written lies in a page that
  holds code", counts by the address, `le.BENCH` and the place.
- A file may have more behind its pages than the header counts, as
  debugging information would be; the reader takes it. That was not
  tried with a file that has any.

On the bench, where the reader cannot be held to anything by a test
that runs no machine:

| Toy | What it wrote | What the reader says |
|---|---|---|
| `zero.c` | code at 00100010, data at 00100524, the bytes that are not in the file at 00104530, the stack 00100530 to 00104530 | The same: `_TEXT`, `_DATA`, `_BSS` and `STACK` of its map, by `python3 harness/le.py` |
| `auto.c` | `AutoTim` at 00100608 | The same |
| `keys.c`, traced | 75 blocks ran in its code, 26 of them with an address in them | Each is the image's bytes with the first object at `le.BENCH` |
| `lecheck.c`, the test's own | Where its code, a text, its table and what the file does not hold are; that all 4,096 entries of its table name what they should, the three among them that lie across the end of a page, with their sum; two addresses outside their object; two words that name code. Traced, 48 blocks, 30 with an address | The same five lines, and every block: `python3 harness/lecheck.py --screen` and `--trace`, as its top says |

For H2: `_md` is at `le.BENCH + program.place(*linked.public["_md"])`,
and `AutoTim` and `Clock` behind it by the original's listing as
today. The stack is the segment `STACK` of the map, whose end
`program.stack` is; what it holds when the program has been loaded is
`program.image()` there, zeros where the file has nothing, which is
what the bench had (`zero.c`).

For H3: `coverage.executed` can take `program.image()` for the file and
`program.image(base)` for the program at a place, with the places of
`relocated()` for its words; what it answers is then counted in places,
and a function of the map is at `program.place(*linked.public[name])`.
The stub is not in the image, so nothing of the extender's counts as
the program's. A block with a selector in it is not in the image as it
ran: `relocated()` lists those two bytes, and the image has them as the
file does. A file is known for a flat program by `le.Linear` taking
it.

For H4: `relocated()` has for every pointer of a table what it names,
an object and an offset that may be below 0. The object files of
`wcc386` have the records of 32 bits, 99h, 91h, A1h, 9Dh and B7h where
`ObjectFile` of `datacheck.py` reads 98h, 90h, A0h, 9Ch and B6h.

### Piece H2, the driver: done, and what a later piece should know

The driver of the tests reads of a flat program what it reads of any,
`AutoTim`, `Clock` and the stack, above the megabyte, by the BIOS's
block move, at addresses that `harness/driver.py` works out from the
program's file and the linker's map. A program of 16 bits is driven by
the instructions that it was driven by before. Nothing of the project
is flat yet, so no test of the build holds the first: `python3
harness/drivercheck.py` does, by hand, with toys on the bench.

What there is:

| | |
|---|---|
| `driver.facts(program, stack)` | As it was for a program of 16 bits. A flat one, which is one whose file `le.Linear` takes (`le.linear`, since H3), gets `Facts` with `flat` set, whose three places are addresses of the machine's memory: `_md` of the map, placed from `le.BENCH`, with the offsets of `AutoTim` and `Clock` from the original's listing as before; and the stack. So `scenarios.run`, `show.py --program` and the scenarios' test have the driver's side of a flat program with no change of theirs |
| `driver.flat_facts(name, program, linked, auto_time, clock, stack, base)` | The same of any flat program: each of the two is a symbol of the map and an offset from it, or None. The trial's toys go through it, with `_AutoTim` and `_Clock` of their own |
| `coverage.flat_stack(program, linked)` | Where the stack is, as two places: the segment of the map that ends where the header of the file has the top of the stack. What it holds when loaded is `image()` there |
| The script | Its header is as it was. For a flat program the four words say none, none, 0 and the stack's length, and an event 06 follows, before the first key: three addresses of 32 bits, 0 for one that is not above the megabyte |
| `DRIVER.ASM`, "A flat program" | The event; `RdHigh`, the block move; `I28F`, `Pump8` and `PutStF`, which do with what was read there what the driver does at `I28Q`, `Pump6` and `PutStk`; `Divert` |
| What the driver writes | `K` of a flat program has 0 for the segment. New, `F`: the BIOS refused a block move, and `driver.read` raises for it |
| `harness/drivercheck.py` | Builds `auto.c`, `keys.c`, `wait.c` and `sys.c` of `harness/trial32/`, runs them on the bench as the program, and holds what the driver did; `--trace`, `--below`, `--facts FILE`. Its top says what is held. Twenty seconds |

Where the plan and the code differ:

- No second form of the header: an event. The header is read where
  every script goes, and telling two forms apart there is instructions
  for the original. An event that no script of a 16-bit program has
  costs it nothing.
- The block move is not "taken only when the header says so" by a
  question at each of the three places: that is two instructions more
  for the original at every round of its idle call. The event writes a
  jump over the instruction at each place, once, before the first key:
  the driver changes its own code for a flat program and for no other.
  The machine's time is counted in instructions, a golden is what the
  original showed at one, and the lists of what ran beside the goldens
  are of what it had run by then; with the instructions the same,
  nothing of that can move, and it need not be found out for each
  scenario whether a few more would have.
- That they are the same was held, with `ndisasm` on the driver before
  and after: each of the 431 instructions from `RdSec` to `Init` is in
  the new one as it was, six bytes further on behind the two that were
  put in, with the jumps and calls across them pointing six further.
  The two are `cmp al,6` and a jump, where `Pump` tells the events
  apart: a script without the event comes to them once, at its end,
  behind its last screen. `Init` is unchanged. What is new stands
  between the interrupts and `Init`.
- The file is 2,048 bytes as before, 1,693 of them used where 1,226
  were, and the driver keeps as much of the memory: the stack is read a
  piece at a time into the 4,000 bytes that hold a screen while what
  blinks is looked for, which hold nothing while a step ends.
- `show.py` and `play.py` needed nothing: the first goes through
  `scenarios.run` and `driver.facts`, the second has no driver.
- 220 lines of `DRIVER.ASM`, 80 of `driver.py`, 15 of `coverage.py`,
  and the check, 470, where the plan had 170 in all.

What decided:

- Every golden of the original derived again, twice, `python3
  harness/goldens.py derive --together 8`: 167 scenarios each time, and
  `git status` of `harness/goldens/` clean after each. The second ran
  beside other agents' tests and took eight and a half minutes for the
  first's two and a half.
- The whole suite, before the plan's other pieces of that hour were
  pulled in and after.
- `python3 harness/drivercheck.py --trace --below`, run five times
  as it grew, with what each toy wrote the same every time, to the
  tick:

  | Toy | What the driver did |
  |---|---|
  | `auto.c`, told that `AutoTim` is at 00100608 and `Clock` at 0010060C, which is what the toy writes of itself | The step of `t` ended with "done by itself" on the screen, nine ticks behind the key. The steps with the clock on have the cell 0,20 as blinking and no other; those before have none. The stack: 16,384 bytes at 00100618, 16,246 of them never used |
  | `auto.c`, told nothing, as `run.py` runs it | The step of `t` ended at once, before the toy had done its thing: what the trial saw of the driver as it was |
  | `auto.c` with a main data: built with its two variables in an `md` of F48Bh bytes, at the offsets of the original's listing, and watched by what `driver.facts` makes of that file, as the program will be | The same as the first, with `AutoTim` at 00105105 and `Clock` at 00104707, which is again what the toy writes of itself |
  | `keys.c` | Ten steps, keys with Shift, Ctrl and Alt, both buttons of the mouse, a wait of 18 ticks, a text typed: each ended with its key on the screen, with `AutoTim` and `Clock` read above the megabyte at every round and saying nothing. The toy writes its stack pointer, 001046BC, which is inside the stack as it was written out, 001006FC to 001046FC, used from 00104666 up |
  | `wait.c` | Its keys came while it waited inside the BIOS; no stack was written out, as for any program that does not idle |
  | `sys.c`, D1's toy of the system's functions, with the tree's `md`, its `stack.c` and its `dos32.c`: by `scenarios.run`, the way of a scenario | Four steps, each with its page. The stack is that of `stack.c`, 4,096 bytes at 00103F70, the segment that ends where the file's header has the stack's top; 3,598 never used. `python3 harness/trial32/both.py` passes as before, with the driver told nothing |
  | `--trace` | By QEMU's log of the code that ran, the toy's first object was at 00100010, `le.BENCH` |
  | `--below` | Of the extender's stack of real mode, 512 bytes at 30C7:0000, 66 were used with the two variables read at every round, and 84 with a stack written out as well |

What the toys prove, and what waits for the first run of the program:

- Proven: the block move from the driver's place in an idle call that
  comes through DOS/32A, at every round of a run, with the extender
  unharmed; the three reads and what the driver makes of each; the
  stack read in pieces, 16 KB in five; the addresses by the file, the
  map and `le.BENCH`, for a program of two objects with its variables
  and its stack in the second, and for a main data found by `_md` with
  the listing's offsets; the whole way of a scenario, `scenarios.run`,
  for a flat program that the tree's own files make; that a step ends
  when it should and not before.
- Waits: that the program has `AutoTim` and `Clock` where its `md`
  says. That is the layout of the main data, which `LAID_OUT` holds
  for both compilers, and the toy with a main data is as near as a toy
  gets. M1 sees it at its first step that waits on `AutoTim`, a cursor
  moved in a tree (`tree`), and at the first scenario with the clock
  on.
- Waits: the program's stack. `sys.c` has the tree's `stack.c`, and
  its stack is found; if B2 links the program otherwise, with a stack
  that is not the segment that ends where the header has the stack's
  top, `coverage.flat_stack` is the one place to say so.
  `drivercheck.py --facts build/.../VC.EXE` prints what the driver
  will be told of a file.
- Waits: the time of day ("What could go wrong", 4). The block move is
  in every idle round of a flat program; in `auto.c` the key `t` was
  seen one tick later with it than without.
- A main data of zeros has 0 in `AutoTim` until the main loop has set
  it, where DOS's memory has what the last program left. The driver
  takes 0 for a time and believes it for its two seconds, once: the
  toy with a main data and `sys.c` both begin so. A box at the start
  of the 32-bit program, for a diskette that is to be put in, waits
  those two seconds as it does today for whatever the memory held.
- Never taken: `F`, the BIOS refusing. SeaBIOS refuses nothing.
- `le.BENCH` is the bench's, and `--trace` asks it of a run. A bench
  that changes what DOS/32A finds, an XMS driver or other memory,
  moves the program, and every flat scenario then waits on nothing or
  on the wrong bytes: run `drivercheck.py --trace` after such a
  change.

For H3:

- The driver's side is done: `scenarios.run` needs no change to drive
  a flat program, to record what blinks, or to answer `stack_left`.
  `Run.stack_segment` is 0 for one; `place.py` goes by it and is of
  the 16-bit trees.
- `coverage.stack`, `coverage.origin`, `coverage.relocated` and
  `coverage.executed`, and `check_translation` and `check_layout` of
  `ported.py`, still read a program of 16 bits and are yours, as H1
  listed them. `driver.facts` no longer comes to the first two for a
  flat program.
- The scenarios' test fails a tree that leaves less than half of its
  stack: of a flat program that is of `known.stack_size` as the map
  has it, with no change.
- The README says how a step is finished, "When a step is finished",
  for programs of 16 bits. What it should say of the block move goes
  in with W, when a program of the project is flat.

### Piece H3, the scenarios' test: done, and what a later piece should know

The scenarios' test judges the program of 32 bits: `harness/ported.py`
with `build/portable_c32/VC.EXE`, the test `portable_c32-scenarios`.
The program is held to a record of the scenarios that it passes,
`harness/pass32.txt`, which has 145 of the 159 that the portable C is
taken through: D2 landed while this was made, and the program runs.
The two trees of 16 bits are judged as they were: at each commit of
this piece the test of each printed what it printed before, to the
line.

**How to go by it.** The harness keeps its machines' disks in the
checkout's `build/tmp` by itself; no `TMPDIR` is wanted.

| To | Do |
|---|---|
| Run the test | `ctest --test-dir build -R portable_c32-scenarios --output-on-failure`, or `python3 harness/ported.py build/portable_c32/VC.EXE > build/NAME.txt`, which is the same and keeps what it printed. Three minutes and a quarter by itself, with other agents' tests on the machine; six in the whole suite, `ctest --test-dir build -j8`, where the three scenarios' tests run side by side and all fourteen take as long |
| Read what it says of a scenario that fails | A line `NAME: differs from the original, first after step 9: KEYS`, and under it the first row of that screen that differs, as it is expected and as the build showed it, with `^` under a cell whose character differs and `~` under one whose colour does. Or `NAME: stopped after step 4, KEYS: the program did not come to wait for a key within 60 s; it shows`, and the screen as it stood. The scenario was ended there: nothing behind its first screen that differs is known |
| Sort the failures by cause | Behind the scenarios, under "What fails, by what was first otherwise": the scenarios that showed the same thing in the same row are on one line, the largest family first |
| Run one scenario, with every row of every screen that differs | `python3 harness/compare.py build/portable_c32/VC.EXE NAME`. With `--first` as the test runs it; with `--limit 30` a step is given up after 30 seconds, where it has 300 |
| Try steps out, and see every screen | `python3 harness/show.py --program build/portable_c32/VC.EXE STEP...`, as for any program |
| Use the program for finding a fault | The same commands with `build/portable_c32/debug/VC.EXE`: the harness takes it for the same tree, with its rules and patches. It takes more instructions, so a scenario that shows the time of day may fail in it alone |
| See which functions ran, and where a run ended | `harness/trial32/ran.py`, as B2 has it, with a trace of `harness/trial32/run.py --trace` |
| Put a scenario on the record | `python3 harness/ported.py --record build/portable_c32/VC.EXE`, which runs them all and adds every one that has come to pass, and commit `harness/pass32.txt` with the change that made it pass. It writes nothing while a scenario of the record fails |
| Take one off | By hand, a line of the file, with the reason in the commit. From M1 on a piece that does so does not land |
| Give a scenario more time than the test does | `--limit SECONDS`, of the test and of `--record`: see the time below |
| Declare that the build shows something otherwise than the portable C of 16 bits | `TREES["portable_c32"]` of `harness/deviations.py`, in the words of `Declared.also()`; and for the rows of one scenario a patch of its own, `python3 harness/goldens.py patch --tree portable_c32 NAME`, which is written to `harness/goldens/portable_c32/` and then stands in the place of the portable C's for that scenario. That was tried, with `command` and with `banner`, and the files taken away again. Both are deviations, and asked first |

**What there is.**

| | |
|---|---|
| The tree | `portable_c32`, the name of the build's directory. `deviations.SOURCE` says that it is built from the source of `portable_c`, and its entry of `TREES` is that tree's with nothing added. `goldens.patch_path` gives it the portable C's patch for a scenario unless it has one of its own; `GONE` and `RENAMED` of `ported.py` are the portable C's with the entries of `dos.c` |
| `setups.Program.load` | The tree of a file is its directory, or the one above if that is named as a tree is: `debug/VC.EXE` is of `portable_c32` |
| `le.linear(contents)` | The flat program of a file, or None. It was `driver.linear` |
| `coverage.executed` | Of a flat program: by its image, `le.Linear.image(base)`, as far as that has code, counted in places. It answers an `Executed`, a dict with `at`, the addresses at which the program was found |
| `coverage.place(contents)` | Makes of an address of the map what `executed` counts by, for either kind of program. `check_translation` goes by it |
| `coverage.stack_size`, `coverage.STACK_LEFT` | The stack's length of either kind; and the one number for the half, which `layoutcheck.pages` reads too |
| `compare.compare` | Answers an `Outcome` for each scenario: the run, or why the machine did not get through, and how the run departs. `first` ends a scenario at its first screen that departs; `limits` gives a scenario its own time for a step |
| `scenarios.run(..., until=)` | Asked of every screen as the driver writes it; a `Run` that it ended is `cut`, with no disks |
| `ported.py` | `RECORDS`, `check_record`, `--record`, `--limit`; `check_flat_layout`, which is `layoutcheck.Check(...).all()` and nothing of its own; `check_place`; `SHORT_STEP` and `short_step` |
| `harness/pass32.txt` | The record: what it is, an empty line, and a name on each line, in the order of the scenarios |

**Where the plan and the code differ.**

- A scenario is ended where it first departs, which the plan did not
  have. The program as B2 left it ended at once, and DOS's prompt took
  the keys of every scenario: 38 of them waited a minute each for a
  click of the mouse that a prompt does not take, and the run was more
  than ten minutes. Ended at the first screen that differs it was two.
  So nothing is known of a failing scenario behind that screen, and the
  test says so by what it prints.
- The time of a step. A scenario of the record has 300 seconds, as
  those of every tree have. One that is not has 60, `SHORT_STEP`, and
  twice its longest `Wait*ticks` besides: a program that hangs in a
  scenario that is expected to fail is waited for that long. That
  cannot fail the test for a slow machine, since such a scenario is not
  held to pass; what it can do is keep a scenario that would pass off
  the record, reported as stopped. Run it by itself then, and write the
  record with `--limit 300`.
- Once the program has not come to its first screen in a scenario, the
  others of that country are not run and are reported as stopped at the
  start: every scenario starts the program the same way. Without that,
  a program that hangs when it starts costs 145 times 300 seconds in
  fours, three hours, of which CTest allows one.
- `--record` only adds. The plan had it write the record. With a
  scenario of the record failing it writes nothing and says which.
- What ran of the C is not asked while a scenario fails, as the plan
  has it; that every function of a batch is linked, or is gone or
  renamed as the tables say, is held from the first day. `FatErr`,
  `Exit01` and `Exit05` are gone from this tree and `Alloc0` is
  `Start`, as S3 and D2 wrote for this piece. The entry of the critical
  error in `dos32.c`, `DeviceEntry`, is static, and the map as
  `coverage.link_map` reads it has no static name: when what ran is
  asked, nothing holds that it ran but the scenarios that show the box.
- `check_place`, which the plan did not have: from QEMU's log, the test
  holds in every scenario that the program's code ran with its first
  object at `le.BENCH`, where the driver reads. H2 had that as a thing
  to ask by hand after a change of the bench.
- The stack is looked at in the scenarios that pass, of every tree: one
  that fails has used its stack for what failed it.
- A scenario that the machine does not get through is reported and no
  longer ends the test with a traceback, for the trees of 16 bits too.
- The limits are as the plan's table has them. The length of the file:
  none. Code below F000h: not asked of a flat program. The page rule,
  the megabyte, the tables and `IniSave`: `layoutcheck.py`, which the
  scenarios' test asks before it runs anything and whose lines it
  prints; the stub is the layout's own test's to hold. Half of the
  stack: as for every tree, of 16,384 bytes.

**What was seen.**

- The program as B2 left it, which printed that it was too big and
  left: all 159 scenarios failed at the start, one family, in two
  minutes; the record was empty and the test passed with it. Its code
  ran at 00100010h in every one, and of the functions that B2 lists the
  ones asked by name had run, `Start`, `Main`, `Init00`, `Init11`,
  `AlocEr`, `Exit00`, and `IniData` had not.
- The program as D2 left it: 145 pass, in two runs one behind the
  other, and the test passes with the record of them, in 194 seconds.
  A scenario takes 3.1 seconds at the middle, where one of the 16-bit
  program takes 2.5.
- The driver's facts with the real program, which H2 left waiting.
  The debug program's listings have `md.AutoTim` at `_md+0xaf5` and
  `md.Clock` at `_md+0xf7`, which are the original's listing's offsets,
  so the driver is told the addresses that the program's own code has.
  `tree`, which waits on `AutoTim`, and the six scenarios of the clock
  pass. The stack is read: 16,384 bytes, of which 14,790 are left at
  the least, in `attributes-directory`; the 16-bit program leaves 3,514
  of 4,096 at the least, in `copy-dirs`.
- A machine's second takes longer while this program idles than while
  the 16-bit one does: `blank-timed` has the driver wait 1,300 ticks,
  71 seconds, which were 14 of the test's for the 16-bit program and
  more than 60 for this one. That is the block moves and the extender
  at every idle round, as "What could go wrong", 4, has it; no scenario
  that shows the time of day fails for it.

The 14 that fail, by cause as far as the first screen that differs
tells it: nothing behind that screen is known of any of them, and no
cause was looked for in the program. The steps are counted from 0, the
start. "Second copy" says whether the scenario had started the program
a second time by then, so that the driver was reading the first copy's
place: see below.

| | Cause, by the look of it | Scenarios | First otherwise, in the one named first | Second copy |
|---|---|---|---|---|
| 1 | The editor, rows of the screen never written. One cause for the three: the same step and the same rows | `editor-long`, `editor-full`, `editor-buffer` | After step 9, `Ctrl+y Ctrl+u Ctrl+u Up Up End Del`, twice over: rows 16, 18, 20 and 22 hold zeros, character 00 in colour 00, where the original has blanks in 1Bh; the cursor is where it should be. The test prints both rows as blanks, with `^` under every cell | No |
| 2 | The editor, where a line ends: the cursor is on line 1 at the column behind what was typed, where the original is on a line further down at column 1. Each has typed a line and Enter into a file of the program's own | `user-menu-run`, `user-menu`, `comspec`, `view-commands`, `editor-save` | After step 6, `"a:" Tab "all" Enter`, row 0: `Edit: vc.mnu  *  Line 1  Col 7` for `Line 2  Col 1`. The others at steps 47, 45, 29 and 23, in `vc.mnu`, `vcview.ext` and `D:\VC\vc.ext` | `comspec` alone, and it shows what the four without one show |
| 3 | The editor, the cursor in a line: perhaps 2 again, since the step begins with Enter | `editor-keys` | After step 7, `Enter Up End Left Left Left Backspace Del`, rows 0 to 13: the status has `Col 157` for `Col 13`, and the cursor is at column 74 of row 1 for column 12 | No |
| 4 | The editor opened where the original does not open it | `editor-big` | After step 28, `F4`, row 0: `Edit: b  Line 1  Col 1` and the editor's screen, where the original shows the panels | No |
| 5 | What the machine's memory is said to be while a command runs from the program | `command` | After step 7, `Ctrl+o`, rows 15 and 17: `mem` of DOS says 15,360K of extended memory, in all and free, for 15,232K, and 16,384K of memory in all for 16,256K, 128K more of each. What the extender makes of the memory: not looked into. If it is a figure that follows what the program is and not what it does, `harness/deviations.py` leaves such out, which is asked first | No |
| 6 | A second copy of the program: the patches of the 16-bit build say that it has no memory to run, and here it has. 3 of what is open for the user | `second-copy`, `start-memory` | After step 6, `Ctrl+o`, row 15: `C:\WORK>d:\vc\vc ver` and what follows it, where the patch expects an empty row. `start-memory` after step 12, `Ctrl+o`, row 18: an empty row where the original has `C:\WORK>d:\vc\vc k` | Both |
| 7 | The program ended by another's hand. 4 of what is open for the user, which D2 tried and wrote down. **Mended by E**, below: `ended` passes and is on the record | `ended` | After step 40, `Ctrl+o`, row 0: of the six words that the scenario prints of the registers left for the program that started this one, `2222 1111 EEF6 436D FF75 0202` for the patch's `2222 1111 EE4F #### 0000 0202` | Yes |

So ten of the fourteen are the editor's, 1 to 4, and none of those is
the second copy's doing by what shows; 6 and 7 wait for the user's
word, and may have the driver's blindness in them besides.

Since C1 the nine of 1, 2 and 3 pass, by one mend, and are on the
record; 4 is no fault of the program's and waits for the user: "C1,
the editor" below.

**A second copy, which the driver does not know.** In six scenarios
the program is started a second time from the command line of the
first, and QEMU's log has its code at 00100010h and at 001DA1A0h:
`second-copy`, `comspec`, `ended`, `start-setup`, `start-screen`,
`start-memory`. The driver of the tests is told one place for
`AutoTim`, `Clock` and the stack, the first copy's, and takes an idle
call for the program's by the name of the program that DOS has as the
current one: while the second copy idles, it reads the first copy's
variables and writes out the first copy's stack. Of a program of 16
bits it reads in the segment that DS is on at the idle call, so either
copy's. What that does: a step in the second copy that waits on
`AutoTim`, a cursor moved in a tree, ends at once or by a time that the
first copy left; a clock that the second copy has on and the first has
not is not looked at for what blinks, or the other way round; and the
stack that is written out says nothing of the second copy.
`start-setup` and `start-screen` pass all the same. Whether one of the
four that fail does so for this is not known, and by the table above
it does not look so. The test names these scenarios in its line on the
place.

What would mend it, by reading and not tried:

- Where the second copy is, the harness can work out, and the driver
  cannot find: nothing in the registers of an idle call says where a
  flat program's objects are. The extender puts the second copy's
  first object where the next block of its memory begins, by the rule
  that `le.py` has for one object behind another: the end of the first
  copy, 001DA18Bh as the program is linked now, rounded up to 16 and
  16 more, which is the 001DA1A0h that the log has in all six.
  `driver.flat_facts` takes a `base` already, so a second set of the
  three addresses is one more call of it.
- The driver is told both: a second three addresses in the event 06,
  or a second event. It asks DOS for the current program's PSP at
  every idle call already, to see by its name whether the call is the
  program's (AH 62h, in front of `I28N` of `DRIVER.ASM`); it keeps the
  PSP of the first call that was the program's, and where a later one
  has another PSP of that name, it reads at the second set. That is in
  the part that only a flat program comes to, `I28F`, `Pump8` and
  `PutStF`, so the original goes through the instructions that it went
  through, which H2 held with `ndisasm` and by deriving every golden
  again; the same has to be held of this.
- A third copy would want a third place. No scenario had one in what
  was run of it, by the log; `check_place` of `harness/ported.py` prints every
  place that a copy ran at, and can hold each to be one that the
  harness worked out.
- The stack of the second copy is then written out as its own, and
  `driver.stack_left` has to hold it against what a second copy's
  stack holds when loaded, which is the same zeros.

**What waits.**

- What ran of the C, until every scenario passes. `check_translation`
  was run by hand against a traced scenario of the flat program, and
  finds `Start_` and `Main_` at their places. In the program that is
  shipped several functions of `dos32.c` are one address (B2): one of
  them counts as run when another did.
- The checks of the words of the addresses in the debug program,
  `CHECKED`, which B2 left to whoever first has a wild read to find.
- `harness/place.py` and `harness/notice.py` know the two trees of 16
  bits and no other.

For the convergence: a piece runs the test, takes a family from "What
fails", mends it under both builds, runs `--record`, and commits the
record with the fix; `ctest --test-dir build -j8` then has both builds
judged. The record is a file of names one to a line: two pieces that
each add a scenario meet in it at a rebase only where their lines are
neighbours, and the one that rebases writes it anew.

For W: the tree's name is in `deviations.SOURCE` and `TREES`,
`goldens.TREES`, `RECORDS`, `GONE` and `RENAMED` of `ported.py`, and
the test's name in `CMakeLists.txt`. When the 32-bit build is the
portable C and the 16-bit one has another name, `SOURCE` turns round,
and the patches that are then the 16-bit build's alone go to the
directory of its name. A record that has every scenario says nothing
any more: `RECORDS` loses the tree, and it is judged as the others are.
The README's "When a step is finished" is still of a program of 16
bits (H2).

### H4, the test of the data: done, and what a later piece should know

The test `portable_c32-data` holds the texts and tables of
`build/portable_c32/VC.EXE` against the original's: `harness/datacheck.py`
with that directory, the same file that makes `c-data` and
`portable_c-data`, which print what they printed, to the line. Its top
says how a flat program is held; this is what was decided, what was
seen, and what a later piece starts from.

    python3 harness/datacheck.py --source src/portable_c \
        build/shareware-layout build/portable_c32

As the build links the program it passes, as B2 linked it and with
what D2 has made of `dos32.c` since, and so does the program for finding
a fault, by hand, `build/portable_c32/debug`, which no test of the data
judges:

    600 objects of the C code, 25273 bytes, are as the original, 37 of
        them but for what src/portable_c/DEVIATIONS.md declares
    149 of them, 12523 bytes, are the original's setup program's
    153 of them are boxes, dialogs and tables of pointers by their
        declarations, with 643 words that are pointers of 4 bytes, 555
        of them addresses that the extender sets: 23987 bytes as the
        original lays them out

Those are the 600 objects of `portable_c-data`, each by the same name in
the same file against the same address of the original, with the same
note, which was held line by line; and 23,987 bytes as the original lays
them out is that test's count of bytes, 25,273 being two more for each
of the 643 pointers. Not compared are the same six, and what only this
program has, the data of `dos32.c`: see below.

**Where each thing is read, and why.**

| What | Where | Why there |
|---|---|---|
| The bytes of an object, and which of its words are addresses and of what | The file, through `le.Linear`: `image()` and `relocated()` | It is what is loaded, and the linker has left nothing to be worked out. An object file has the same before the linker, in records of 32 bits that would have to be read and then believed |
| Where an object is | The linker's map, `le.Map`, static ones too | As for a program of 16 bits |
| How long an object is | Its object file: the next symbol of its segment there, or the segment's end | Neither the file nor the map says. `wcc386` rounds what a file has of `CONST2` up to four bytes, so the last object of a file ends up to three bytes before the first of the next begins, and the map does not have where a file's part of a segment ends. `ObjectFile` reads the two records that this takes in their forms of 32 bits, 99h for a segment, 91h and B7h for symbols; A1h and 9Dh, what a segment holds and the addresses in that, it does not read |
| Which fields of an object are pointers | The declaration, in the source: `declared_shapes` | The plan's way. A pointer that holds a number, `NUMBER(LenStr)` or the 0 of a line across a box, has no relocation, so the file cannot say that those four bytes are one word |

**The shapes.** `SHAPES` of `datacheck.py` has the five of the plan as
four words and one rule: `BOX(n)`, `BOX1(n)`, `BOX2(n)` and `DIALOG(n)`,
read where a declaration begins its line, `(static) const BOX1(4) Name =`,
as all 151 of the tree do; and "pointers", an array of pointers, which
is `Menu_T` of `commands_t.c` and `Fnc_T` of `dirfunc.c`, or an array of
a structure that is declared with it and has nothing but pointers,
`StPagTab` of `stcolor_t.c`. A structure with pointers and other things
in it stops the test and asks for a shape. Everything else is bytes.
What a shape is, in bytes and pointers, is written there once, from
`tables.h`: 8 bytes and the lines for a box, with 4 more for each text
that it puts in; the word of the box and 8 bytes, a field of 9 bytes
and its word, and a byte for a dialog.

**How an object is held.** It is read as the original lays it out, a
pointer as a word, and then held by the rules that a program of 16 bits
is held by, with every table of the test applying as it does there:
the places of `DECLARED`, `NAMED_OTHERWISE` and `COUNTED` are counted in
the original's layout, and so is the place in a message, which says
where that is in the object itself too. Of a pointer:

| The original has there | The pointer has to be | Else the test says |
|---|---|---|
| A name, held against where the tree has the thing: a text, a box, buttons, a table of commands, a picture | An address that the extender sets, of what the tree has by that name, static copies and the setup program's things as in 16 bits, as far behind it as the original's word is behind its label | `StDfCol_P+10, which is +12 of it as it is here: the translation has the address of StDfCo2_S, 0002:00006ea5, which is not where it has DfCo1_S, 0002:00006e83: the original names DfCo1_S there`; or `the translation has the number 1234h, in a word that could name something, and no address that the extender sets: the original names YesNo_S there, and it is to be where it has YesNo_S` |
| A number, or a name that a tree has a number for: a place of the main data, the number of a command or of a routine | No address: its low half is held as the two bytes are in 16 bits, and its high half is 0 | `Find_B+19, which is +21 of it as it is here: the translation has the address of FnFl_P, 0002:00005fda, where the original has a number and names nothing, 40h its low byte`; or `Find_B has at +34, as it is here, a pointer that is no address which the extender sets and holds 00010140h: a number in the place of what a word names is one of 16 bits, and the program reads no more of it` |

And of the object: one that is not as long as its declaration makes it
with pointers of four bytes fails, and so does an address of the file
anywhere in it but in a pointer of its shape, in bytes or across two
fields. That is how a declaration read wrong shows, and how a gap would
show that the compiler had put into a structure.

**No gap, and nothing moved.** Of the 606 objects that both programs
have, 605 are, read so, exactly as long as in the program of 16 bits;
the other is `Fnc_T`, five pointers to functions, which are far on DOS
and so 20 bytes in both, and which the original has not. `wcc386`
puts nothing between the fields of a box or a dialog nor between two
objects of a file, with the structures of `tables.h` packed and a box's
pointers falling on a multiple of four by themselves (8, 12 and 16 bytes
in front of them). So there is no finding about the layout, and
`tables.h`, `vc.h` and the files of tables are as they were. A dialog's
fields are 13 bytes each and their pointers lie at any address, which
the 386 reads.

**Seen to fail.** Each planted in the source, both programs built, and
the two tests asked. `portable_c-data` fails for each as well, in its
own words, but for the number of more than 16 bits:

| Planted | `portable_c32-data` says |
|---|---|
| A byte of a text, `" filez\0 found."` | `FnFl6_S+5: the translation has 7Ah, the original 73h` |
| Two lines of a box exchanged, in `StDfCol_P` | The first message of the table above |
| A dialog that names another's box, `SrEr_B` with `&FndEr_P` | `SrEr_B+0: the translation has the address of FndEr_P, 0002:0000a1ef, which is not where it has SrEr_P, 0002:0000a225: the original names SrEr_P there` |
| A field that names other buttons, `StColNm_P` for `YesNo_D` | `StDfCol_B+19, which is +21 of it as it is here: the translation has the address of StColNm_P, 0002:00007292, which is not where it has YesNo_D, 0002:000062c6: the original names YesNo_D there` |
| `MD_AT` of another field, `SrchStr` for `FindStr` | `Find_B+17, which is +19 of it as it is here: the translation has 7Fh, the original 3Eh` |
| The same where a box of the setup keeps what it covers, `WinBuf` for `ErrBuf` | `StDfCol_P+6: the translation has 6228h, which is not where the main data has ErrBuf, 7F08h: the original names BoxBuf there` |
| A number where a name belongs: `NUMBER(0x1234)` for a line, `NUMBER(5)` for buttons, 0 for a line | The second message of the table above, and the same with `0005h` and `0000h` |
| A name where a number belongs, `FnFl_P` for `NUMBER(LenStr)` | The third message of the table above |
| A count of lines one off, 3 for 4 | `StDfCol_P+4: the translation has 03h, the original 04h` |
| Two tables of `Menu_T` exchanged | `Menu_T+2, which is +4 of it as it is here: the translation has the address of Menu3_T, 0002:0000433c, which is not where it has Menu2_T, 0002:0000431a: the original names Menu2_T there` |
| The halves of a picture of `StPagTab` exchanged | `StPagTab+4, which is +8 of it as it is here: the translation has the address of StColr2_T, 0002:00008a70, which is not where it has Page2_S, 0002:0000786f: the original names Page2_S there` |
| Another command in a menu's table | `Menu1_T+2: the translation has 0050h, which is not the number of the command M01_00 in portable_c/vc.h, 004Fh: the original names M01_00 there` |
| A number of more than 16 bits, `NUMBER(0x10100 + LenStr)`: the 16-bit program does not have it, the compiler cutting it, and that test passes | The fourth message of the table above |
| An array of pointers that the original has not, `Planted[1]` | `the original has no data called Planted: say in ELSEWHERE or in OWN of datacheck.py what Planted is` |

And with the reading of the declarations made wrong by hand, in the
test itself: `Menu_T` taken for bytes, `Menu_T has an address at +0 of
its 24 bytes, that of Menu1_T, where its declaration has no pointer`; a
box with a count one less, `Filt_P is declared BOX(6), which is 32
bytes where a word that names something is 4, as tables.h has it, and
the object is 36 bytes`; and a `BOX(4)` taken for a `BOX1(3)`, which is
as long, `StDfCol_P has an address at +8 of its 24 bytes, that of
StColor_S, where its declaration has none, BOX1(3)`.

**What only this program has.** The data of `dos32.c`, 17 objects
today, where `dos.c` has none.

- Its constants are in `OWN_OF_FLAT` of `datacheck.py` by their names,
  each with its reason, a table like `OWN_OF` that a program of 16 bits
  is not held to: `Shell_T`, the instructions of real mode by which a
  command is handed to the shell, and `NoStart_S`, the words of
  `Aloc_S` once more, ended as DOS takes a text. `reads.py` has both in
  `REVIEWED`.
- Its variables with a value, 15 of them, a selector, a vector that was
  taken, where the PSP is, are left out by what they are and not by
  their names: an object of the file of the system's functions that the
  compiler has with what has a value, `SYSTEM_VARIABLE` there. The
  piece began with a name and a reason for each, as every other object
  that is not compared has one. B2's file had two; D2 landed beside
  this with fifteen, every one written `= 0`, and each that the
  convergence adds would have failed this test until someone gave it a
  line, for no table of the original's held any better. And a name
  among them could be one of the original's by chance, and would then
  be held against whatever that is; none is today. By the rule it is
  left out before it is looked for there. The rule holds for `dos.c`
  in a program of 16 bits alike, where it meets nothing. It is a change
  to what this test leaves out, and it was not asked: say if each is
  to have its line.

**`reads.py`.** Nothing of it is of 16 bits in kind: it finds the
places in the text of the source, and a count is of bytes of a text or
of a table of bytes, which are the same in both programs. For the flat
program it lists what it lists for the one of 16 bits, place for place,
with the lengths of the boxes and dialogs as they are there, none of
which is read by a count, and the two places of `dos32.c`. Two things it had to be told. Which file of
the system's functions the program is built of, `SYSTEM_FILE`: it read
every file of the directory, `dos32.c` too for the program of 16 bits,
which named no constant then; now the other one is not read and what
`REVIEWED` has for it is not held against the program. And where the
source is, for a build's directory that has not the tree's name:
`python3 harness/reads.py build/portable_c32 src/portable_c`.

**The figures.** One row, behind the addresses that the extender sets:
how many words of the tables are pointers, and how many of those are
such addresses, 643 and 555. `datacheck.check` gives them, the test's
own counts; `main` of that file is the command line around it. The
data of the program has 560 addresses in all, by `le.py`: these 555,
which name data, so that every one of them is in a table that the test
holds, and five that name code, `Fnc_T`, which is not compared. H1
counted the same two numbers in the trial's file.

**`arithcheck.py`** has nothing to hold in a program of 32 bits and was
left as it is: it runs `__U4M` and `__U4D` of `arith.c`, which the
compiler for 16 bits calls and `wcc386` does not, so the file is not
built for 32 bits and there is no test `portable_c32-arith`. It goes
with `arith.c`, in Z1.

**Otherwise than the plan had it.**

- "A field that holds an offset in the main data ... written so that it
  may also be a relocation that names `md` and that offset" is not
  written so. Such a field is a number of 16 bits in every table today,
  `MD_AT`, and in step 1 an address in its place would be wrong: the
  code reads 16 bits of it. The test fails for one. Step 2, which makes
  those fields pointers, changes the shapes of `tables.h` and of
  `SHAPES` together, and gives the rule then: where a number is to be
  the place of a field, an address of `md` and that many bytes.
  `Flat.variable` and `Flat.main` have what it takes.
- The test reads object files after all, for the lengths alone.
- No test for the debug program: it passes, the tables being the same
  source, and the plan has that program judged by none.

**What a later piece should know.**

- M1 and the convergence: a constant that `dos32.c` gains fails the
  test, "the original has no data called X", until it is in
  `OWN_OF_FLAT`, and "how much of X is read here is not known" until
  `REVIEWED` of `reads.py` has the place; one that goes has to go from
  both, which the test says as well. A variable of that file needs
  nothing, with a value or without. In any other file a variable with a
  value is an object like a constant, as the five texts of `stopts_t.c`
  are that the program writes to.
- A file of tables that is changed is held by both tests, and each
  says the same thing in its own way: read the 16-bit one first, it
  is shorter. Where only `portable_c32-data` fails, it is one of the
  two things that only 32 bits can have wrong, a number where an
  address belongs or the other way round, or the test's reading of a
  declaration: a new way of declaring an object with pointers wants a
  line in `declared_shapes`.
- A box cannot be declared before it is defined, `static const BOX1(4)
  Name;`: each `BOX(n)` is a structure of its own to the compiler, and
  the second is "Type does not agree with previous definition". Only
  arrays can stand behind what names them, as buttons do.
- W: the test is named for the build, and its tables go by the source's
  name, `--source`. When the tests of the portable C judge the program
  of 32 bits, `OWN_OF_FLAT` and `SYSTEM_FILE` still tell the two
  programs apart for as long as both are built; with Z1 the first
  becomes a part of `OWN_OF` and the second goes.
- Step 2: `NAMED` going, and a table handed on as a pointer, changes
  nothing here: the tables hold pointers already. What changes is a
  field that becomes a pointer, above, and `struct Picture` of
  `stcolor.c`, which nothing holds to `StPagTab`'s own declaration
  (P3).

### The pieces

P and S change the program under the 16-bit build; D, B and H make the
32-bit side; M and C bring the two together; W and Z end it. P1, D1
and H1 are started at once by the session.

| Piece | What it does | Owns | Size | Needs; beside | Right when | Could go wrong |
|---|---|---|---|---|---|---|
| P1, the words in one place. Done: see below | `vc.h` gets the macros of the addresses once, with two bodies: `AT`, `WORD_AT`, `OFFSET`, `PANEL`, `MAIN_SEG`, `NEAR`, `MD_PLACE`, `MD_POINTER`, `TRELIST`; the files lose their own. The classes P, M, V, G and R whole | `vc.h`, `tables.h` for `LAID_OUT`, and the tops of 23 files | 46 definitions out of 23 files, 17 lines by hand | Nothing; lands first and alone, D1 and H1 start beside it | The 16-bit `VC.EXE` is the same bytes; the host check falls to some 310; `whole.py` still links | A file's own `OFFSET` meant something else: `dos.c`'s does, and keeps its own under another name. And nothing runs the second bodies before M1: a mistake in them is found there |
| P2a, places of the main data: the lists, the tree, the command line. Done: see below | N in `treebox.c`, `tree.c`, `enter.c`, `usermenu.c`, `cmdline.c`, `place.c`, `info.c`, `extfile.c`, `drive.c`, `input.c`, `panel.c`, `files.c`, `read.c` | Those files | 97 places, 59 by hand | P1; beside P2b, P3, P4, S | The same bytes; those files clean in the host check | A number that is no place of the main data: `(u16)(u32)file` is the offset in a panel |
| P2b, the same: the viewer, the editor, a copy, the menus. Done: see below | N in `view.c`, `edit.c`, `copy.c`, `menu.c`, `help.c`, `find.c`, `stcolor.c`, `sthelp.c`, `window.c`, `bars.c`, `dirfunc.c` | Those files, but the lines that S1 has in `view.c` | 66 places | P1; beside the others | The same | `EditBuf`, `CpSrc`: names that are places of the main data by a macro |
| P3, the tables read as what they are. Done: see below | `DialBox`, `Dialog` and their routines read a box's lines, a dialog's box and its buttons as pointers; `main.c` reads `Menu_T` so and `stcolor.c` `StPagTab`; a word that is a number is read as one | `dialog.c` whole, with its 17 of N; `tables.h`; three lines of `main.c`, five of `stcolor.c` | 40 places of T, 17 of N | P1; beside the others | The 16-bit scenarios, since the code changes; `dialog.c` clean in the host check | The one routine that every box and dialog goes through |
| P4, arithmetic. Done: see below | `int16.py` becomes a test with a record; each place that it names is read, and gets its cast or its reason | `harness/int16.py`, its record, `CMakeLists.txt` for the test; single lines of 34 files | 50 places read, 73 looked over | P1; beside the others, and lands behind P2 | The same bytes where a cast was added; the record | A place taken for one that cannot wrap |
| S1, the PSP and a program run. Done: see below | The command tail and the environment through `sys.h`; `ProgramRun` takes a name and a tail; `PspSeg` and `ProgramPrefix` go from the common code | `sys.h`, `dos.c`; the places in `resident.c`, `start.c`, `main.c`, `view.c`, `info.c` | Some 3 functions new, 2 changed, 6 places of the PSP and 6 of the block | P1; beside the P pieces | The 16-bit scenarios; `python3 tools/calls.py` | The two file control blocks that a program is run with are bytes of the main data, as the original has them: kept |
| S2, what DOS enters. Done: see below | The critical error, Ctrl-Break and Ctrl-C taken and given back through `sys.h`; the entries in `dos.c` | `sys.h`, `dos.c`, `vc.h`; `HookVct`, `RestVct`, `FatErr` of `main.c`; `SetBrk`, `CtrlC` of `resident.c` | 3 entries, 7 places | S1 | The 16-bit scenarios, `not-ready` and `phantom` above all | The order in which DOS is asked for the vectors, which is behaviour |
| S3, the entry, the end, Quick execute. Done: see below | `Alloc0`, `TakeEnd`, `Exit00` to `Exit05`, `Int2E`, `Exec2E` into `dos.c`; the 30 of N in `resident.c`, `main.c`, `start.c`; the four `#pragma data_seg` under `__I86__` | `resident.c`, `dos.c`, `main.c`, `start.c`, `stack.c` | 12 routines moved, 4 places of the PSP, 30 of N | S2 | The 16-bit scenarios, `ended` and `quick` above all; the host check has interrupt functions, `__loadds`, `_asm` and registers in `dos.c` and `arith.c` alone | The one place where the stack is set by hand |
| D1, `dos32.c`: what goes straight through. Done: see "Piece D1" below | The entry without a library, the 71 functions, and a toy that asks each | `dos32.c`; `harness/trial32/` | Some 900 lines | Nothing; starts at once | The toy prints the same built both ways, on the bench | A service that the trial did not run: 0Eh, 3300h, 3301h, 4400h, 440Fh, 5701h |
| D2, `dos32.c`: the rest. Done: see "Piece D2" below; the two functions that waited for the user are E's | The block below the megabyte, the 7 functions through real mode, the 640 KB, the three interrupts with a box drawn from inside the critical error, a program run, the end, Quick execute | `dos32.c`; the toys | Some 500 lines | S3 for the shapes; its toys can begin beside S | The toy; a toy of the critical error with Retry and with the phantom drive | See "What could go wrong", 3. The piece most likely to be more than one agent holds: the critical error can be cut off as a piece of its own |
| E, the end by another's hand. Done: see below | On the user's word of 13 October 2026: the program of 32 bits has DOS told of Ctrl-C when DOS ends it, by a routine of real mode below the megabyte that the PSP names. `sys.h` has the program say what is to be put back, a value, where it handed a function over | `sys.h`, `dos.c`, `dos32.c`, `resident.c`; the toy | 8 instructions, 3 functions for 2 | D2, H3 | `harness/trial32/ended.asm` shows the same row of registers for both programs, and `BREAK` on; `ended` on the record; the whole suite | A DOS or an extender's host that the bench has not |
| B1, the objects. Done: see below | Every file compiled for 32 bits by the default build, with warnings as errors, but `arith.c`, `dos.c`, and `resident.c` until S3 | `CMakeLists.txt`, `tools/refused.cmake` | 50 lines | P1 | The build | Little |
| B2, the program. Done: see "B2, the program" below | The link, the stub with its banner bit, release and debug, the licence text and the README's sentence | `CMakeLists.txt`, `tools/stub.c`, `LICENSE.DOS32A`, `README.md` | 100 lines | D2, S3 | A file that the bench starts | What has no value ending up in the file: the trial's toy was 232,840 bytes for 32,404 by the order alone |
| H1, the reader. Done: see "Piece H1, the reader" below | LE files, their objects, pages and relocations; the flat map | `harness/le.py`, `coverage.link_map` | 150 lines | Nothing; starts at once | It reads `whole.py`'s file and the trial's toys | |
| H2, the driver. Done: see "Piece H2, the driver" above | The facts of a flat program; the block move, from `DRIVERX.ASM` of the trial; the stack by it; the goldens derived again | `harness/driver.py`, `DRIVER.ASM`, `show.py`, `play.py` | 170 lines | H1 | `auto.c` of the trial through the harness itself; no golden changed | A golden changes: the work stops there and the user is asked (`CLAUDE.md`, "Work by several agents") |
| H3, the scenarios' test. Done: see "Piece H3, the scenarios' test" above | The 32-bit build as a tree of the harness, declared as the portable C is, with patches of its own where the two builds are declared to show otherwise; the record; the limits | `harness/ported.py`, `coverage.py`, `deviations.py`, `CMakeLists.txt` | A day | H2, B2 | The test runs and writes a record | |
| H4, the test of the data. Done: see "H4, the test of the data" above | The five shapes and the relocations; the figures of the README | `harness/datacheck.py`, `tools/figures.py` | A day or more | H1, B2, P3 | It passes for the 32-bit build, and fails for a word changed by hand | |
| R1, `resident.c` renamed | The file of the portable C gets a name for what it is, what the program does first, the commands that it runs by DOS and what it does when it leaves: nothing of it is resident. The C translation's file keeps its name, so the three tables of the harness that know the file by it for both trees have to tell the trees apart, or take the name from the tree: `source="resident.c"` of three batches of `harness/ported.py`, with a text there that says `CtrlC of resident.c`; the keys of `REVIEWED` in `harness/reads.py`, seven entries, and one of the table there of what the portable C has no more; and a text of `harness/datacheck.py`. Then the comments of the tree, `CMakeLists.txt`, the README and the notes that name it | `git mv`, those three files of the harness, the comments | Small | The first run, M1; beside the convergence, which changes the file little | The same bytes of both programs, and the tests that read the source, `portable_c-data` above all | A table that still finds the old name in the C translation and so passes with the entry of the portable C dropped: hold the counts of `reads.py` and of the batches against those before |
| M1, the first run | The program started on the bench, and what keeps it from its first screen and from leaving mended | Whatever it meets, each fix under both builds | Unknown | All of P, S, D, B; H2 | `banner` passes; the first record | |
| C1 to Cn, the convergence | The scenarios that fail, by family: one piece takes those of one cause | By what fails | Three to six pieces, a guess | M1, H3; side by side | The record grows and the 16-bit scenarios pass | See "What could go wrong", 1 and 2 |
| W, the switch | The tests of the portable C judge the 32-bit build, and the 16-bit one under another name beside it; entry 1 of `DEVIATIONS.md`, and what the others say of `ended`; the patches of `second-copy`, `start-memory`, `ended`; the README | `CMakeLists.txt`, the harness's names, `DEVIATIONS.md`, `README.md` | Small | Every scenario passes; H3, H4; the user's word on 3 and 4 above | Both builds pass everything from one source | |
| Z1, the 16-bit build goes | `dos.c`, `arith.c` with its test, the first bodies in `vc.h`, the build's 16-bit half, what the README says rests on it; `dos32.c` becomes `dos.c` | Those | Small | The user's word | The 32-bit tests | |
| Z2, the words go | `__far`, `FAR`, `STACK`, `CODE` out of the source, by a script | Every file | 1,210 places | Z1 | The 32-bit `VC.EXE` is the same bytes | |

The order:

    at once      P1                  D1          H1
    P1 landed    P2a P2b P3 P4  B1   S1 > S2 > S3     H2
    S3 landed    D2 > B2                              H3 H4
    then         M1 > C1 .. Cn > W > (the user) > Z1 > Z2
    M1 landed    R1, beside the convergence

The longest way through is S1, S2, S3, D2, B2, M1 and the convergence:
six pieces one behind the other before the first run. D2 shortens it
by having its toys ready when S3 lands.

No return: none before Z1. Up to and with W every commit builds both
programs and passes the scenarios with the 16-bit one, and W is a
change of names. Just before Z1 the tree is one source from which two
programs are built that pass every scenario, the 32-bit one judged by
default. What the user is asked then: that the 16-bit build may go;
and, if not asked before, 3 and 4 of what is open, since the
patches of W rest on them.

What a piece goes by:

- A piece of P or S lands when the 16-bit scenarios pass, as every
  piece does. Where the table says "the same bytes", the agent also
  holds `build/portable_c/VC.EXE` against the one of the commit that
  it started from, with `cmp`: the build is the same file twice from
  the same source, which was seen. A file that differs where it was
  not to says where the change did more than it meant; `wdis` on the
  two objects shows the place.
- The host check's record and that of `int16.py`, `harness/int16.txt`,
  are committed with the change that moves them. The second moves for
  a line that is changed and is in it, also where the change has
  nothing to do with arithmetic: `python3 harness/int16.py --record`
  then, which keeps every reason whose line is as it was.
- From B1 on, a piece that does not compile for 32 bits does not land.
  From M1 on, one that takes a scenario out of the record does not.
  The scenarios take 195 to 200 seconds for a tree by itself
  (`LastTest.log`), so both builds are seven minutes.
- What a piece learns that the next needs goes into `CLAUDE.md`,
  "Notes on the portable C", before it is committed, as always.
- A fix that the 32-bit program needs is made in the common source,
  under both builds. Where the two have to differ it is in `dos.c`
  against `dos32.c`, or in the two bodies of `vc.h`, and nowhere else:
  no `#ifdef` in a file of code.

### P1, the words in one place: done, and what a later piece should know

`vc.h` has the words of the addresses in one block, under "The words of
the addresses": each with a body for DOS in 16 bits, which is what the
files did, and a body for a flat memory, with what that body takes for
granted written at it. The files have lost their own: 41 definitions
out of 23 files, 20 of `OFFSET`, 13 of `AT`, 4 of `NEAR`, 2 of
`MAIN_DATA` and 2 of `SEGMENT`, and 30 lines of code were changed by
hand. `build/portable_c/VC.EXE` was the same bytes with all of that.
The one thing that could not leave them alone is a commit of its own,
the panel that was found in the segment of a pointer: see `WCB_OF`
below.

Nothing runs the second bodies. They compile: every file but `arith.c`
and `dos.c` under `wcc386 -3r -mf -os -s -zl -zls -wx -we`, with
`resident.c` alone warning as it did, twice of one line; and the host
check's gcc says nothing of them. `harness/trial32/whole.py` links the
tree with them, 286,640 bytes where it was 277,132: 126,649 of code
where it was 121,807, and 5,056 relocations where it was 4,440, which
is what the megabyte costs. And they were read against "The memory"
above, and once run on this computer, for what that is worth: a file
of a few lines that has gcc take them for 64 bits, with a main data
that has 64K in front of it and a megabyte behind, and holds each word
against the address that the account gives, the tables in front of the
main data and behind it at every place in a paragraph. It is not kept:
it says that the bodies do what the account says, not that the account
is right for the program.

The words, with what each takes and gives. The pointer is far in 16
bits and plain in 32.

| Word | Takes | Gives | The second body |
|---|---|---|---|
| `MAIN_SEG` | | The segment of the main data, a number | 1000h |
| `AT(segment, offset)` | A segment of the megabyte and an offset in it | `u8 *`, the byte there | `ARENA + 16 * segment + offset`, each cut to 16 bits first |
| `SEG_OF(pointer)` | A pointer | Its segment | The paragraph that it is in, which is the segment it was made with only where the offset was below 16: `Line` of `find.c` makes such |
| `OFFSET_IN_SEG(segment, pointer)` | The segment that a pointer was made with, and the pointer | Its offset there: what `AT` wants back. The first body does not read the segment | How far the pointer is behind `AT(segment, 0)` |
| `PANEL(panel)` | A panel's segment | `struct WCBdef *` | `AT(panel, 0)` |
| `TABLES_BY(Title_S)` | `Title_S` and nothing else | The segment of the tables. `AsmSeg` of `misc.c` is this, and code calls that | The paragraph 32K below `Title_S` |
| `OFFSET(object)` | A box, a dialog or a text of the tables | Its offset in the tables' segment | How far it is behind `AT(TABLES_BY(Title_S), 0)`. No constant: it cannot stand in the value of a static object, and stands in none today |
| `NEAR(pointer)` | A pointer that a routine returns far and that is into the main data | `u8 *`, near | The pointer |
| `MD_PLACE(pointer)` | A pointer into the main data | Its place there, a word | How far it is behind `&md` |
| `MD_POINTER(place)` | A place of the main data | `u8 *` to it | `&md` and the place, cut to 16 bits first |
| `TRELIST` | | Where the list of the tree begins, a place | One body, on `MD_PLACE` |
| `DOS_AT(segment, offset)` | A segment of DOS's own and an offset: the PSP, the environment, the header of a device | A far pointer | None of its own: it is the cast in both, the host check counts it, and S1 to S3 take its 13 places away: S1 took eight and S2 one, and four are left, in `resident.c`, which are S3's |

What is otherwise than the plan had it:

- `WORD_AT` is not in `vc.h`: three files mean three things by it.
  `dialog.c` keeps its own, the word at a segment and an offset, which
  is made of `AT` and so has both bodies; `view.c` has the word at a
  place of the text and `usermenu.c` the word at a pointer, and neither
  makes a pointer of a number. `start.c` had it for the PSP alone, and
  has `PSP_WORD` there now, of `DOS_AT`. Likewise `TEXT` of `view.c` is
  `AT(segment, 0)`, and `FIELD` of `dialog.c` was made of `AT` already.
- `SEG_OF` and `OFFSET_IN_SEG` are new. The first is what `find.c` and
  `read.c` had as `SEGMENT`; the second is for a pointer into a panel
  or into the viewer's text that is taken for its offset, which
  `MD_PLACE` is not for: see the examples.
- `DOS_AT` is new, for the 13 places where a file outside `dos.c` made
  a far pointer into DOS's own memory with its `AT`. Left to `AT` they
  would have gone from the host check's count while being as wrong in
  32 bits as before.
- The two lines of `LAID_OUT` are not under `__I86__`. They say
  `8 + sizeof(const void *)` and `9 + sizeof(const void *)`, which is
  10 and 11 on DOS, 12 and 13 for `wcc386`, and holds gcc's 16 and 17
  as well: no build changes them, and `whole.py` does not any more.
- `dos.c` has its `OFFSET`, the offset of a far pointer, as
  `OFFSET_OF`.
- `AsmSeg` says `TABLES_BY(Title_S)` and not a word without an
  argument, because `harness/reads.py` goes by the text of a file: its
  entry for `misc.c` and `Title_S` wants the file to name the constant,
  and the C translation has the same entry.

The host check counts 249 places where it counted 1,040. The plan
thought some 310, which is 308 by its own counts, and the 59 between
are 24 of P3 and 34 of DOS that the check sees no more, two of N that
went with the lines they stood on, less one that the plan's sorting
had in a class that went:

| Class | Was | Is | What went, and what is left |
|---|---|---|---|
| T | 326 | 16 | All that goes through `OFFSET` and `AT`. Left are the 16 of `WORD` in `dialog.c`. The plan had 40 for P3, and thought them all left: what they are besides those 16 goes through `AT`, `FIELD` and `WORD_AT` and went from the count with the rest, 25 places of `dialog.c`, 8 of `main.c` and 3 of `stcolor.c`. Those of them that read a pointer out of a table as a word are as much P3's as they were, with nothing counting them |
| N | 210 | 170 | The 38 of `TRELIST`, which leave `place.c` clean; the one of `bars.c`, whose line was changed for its segment; and the one of `info.c` that is no place of the main data but the offset of a file in its panel, on the line of `WCB_OF` |
| P | 150 | 0 | `PANEL`, and the two of `WCB_OF`: see below |
| R | 108 | 0 | |
| M | 105 | 0 | |
| V | 39 | 0 | One of them was the environment, in `resident.c`, and is among D now |
| G | 6 | 0 | |
| D | 96 | 63 | 30 of `dos.c`, which names `MAIN_SEG` and is read with the constant: it has 39. And 4 outside it for the same reason, the block that a program is run with, `(u32)MAIN_SEG << 16 | ...` in `resident.c`, `view.c` and twice in `start.c`: the two of `start.c` are counted nowhere now, and are S1's all the same |

`WCB_OF(file)` was the panel that a file's entry is in: the segment of
the pointer to the entry, with the offset 0. A pointer has no segment
in a flat memory, and a panel is as long as its list of files, so the
address says nothing of where the panel begins: the panel has to be
handed on, as the plan said. It had two places. `PutSize` of `format.c`
asked whether the sizes of directories had been worked out, and now
takes the panel from `SzDtTm`, which takes it from its two callers in
`panel.c`, both of which have it. And the Info panel's loop over the
other panel's files has `PANEL(other)`, with `OFFSET_IN_SEG(other,
file)` for where it is in the list. That is other code, one more word
on the stack of a call and a segment loaded from the number: `VC.EXE`
is 32 bytes longer, and what decided was the scenarios, all of which
pass.

So a file that is clean in the record is one that makes no pointer of a
number outside the words. It is not thereby right in 32 bits: gcc has
nothing to say of a table's word read as 16 bits through `AT`, of
`MAIN_SEG` shifted into a far pointer for DOS, or of a pointer stepped
past where its segment ends.

`harness/trial32/casts.py` knows `DOS_AT` for a place of DOS, and
sorts the tree as it stands: `python3 harness/trial32/casts.py N
view.c` lists what a piece has left in a file.

What the pieces that start here go by, each line as it is and as it
becomes. Every one of them was tried under the 16-bit build and gave
the same bytes, and was taken back where it is another piece's.

| Whose | The place | As it is | As it becomes |
|---|---|---|---|
| P2 | A place of the main data handed on as a number | `SavWin(begin, size, (u16)md.WinBuf)` | `SavWin(begin, size, MD_PLACE(md.WinBuf))` |
| P2 | A pointer kept as a place | `md.HstAdr = (u16)entry` | `md.HstAdr = MD_PLACE(entry)` |
| P2 | The address of a field | `(u16)(u8 *)&md.CrntCol` | `MD_PLACE(&md.CrntCol)`: `ClsLine` of `bars.c` has it |
| P2 | A place read as a pointer | `entry = (const u8 *)md.HstAdr` | `entry = MD_POINTER(md.HstAdr)` |
| P2 | The same, to something that is no byte | `((struct DIRENT *)md.TreAdr)->DirLevel`, `*(u16 *)starts = line.at` | `((struct DIRENT *)MD_POINTER(md.TreAdr))->DirLevel`, `*(u16 *)MD_POINTER(starts) = line.at` |
| P2 | What a routine returns far, kept as a place | `(u16)(u32)MvzLine(md.MenuBuf, tried + 2, 13)`, `(u16)NEAR(CutPath(CpSrc))` | `MD_PLACE(NEAR(MvzLine(md.MenuBuf, tried + 2, 13)))`, `MD_PLACE(NEAR(CutPath(CpSrc)))` |
| P2 | A place worked out | `(u16)md.TmpBuf - 2`, `*(u16 *)(u16)(TRELIST + count)` | `MD_PLACE(md.TmpBuf) - 2`, `*(u16 *)MD_POINTER(TRELIST + count)`: the sum is made in the number, inside the word, where it wraps round as on DOS |
| P2 | A pointer into a panel taken for its offset: no place of the main data | `(u16)(u32)file` | `OFFSET_IN_SEG(panel, file)`, with the segment that the pointer was made with: `PutInf` of `info.c` has it. The difference of two far pointers, written out, is other bytes |
| P1 | A far pointer kept as a near one | `to = (u8 *)(u16)(u32)AddFile(to - 1)` | `to = NEAR(AddFile(to - 1))` |
| P1 | A segment and an offset with a shift | `(const u8 __far *)((u32)segment << 16 \| at)` | `AT(segment, at)` |
| P1 | The segment of the main data, worked out | `SEGMENT(&md)`, `MAIN_DATA()` | `MAIN_SEG` |
| S1 | The PSP and the environment | `AT(PspSeg, 0x81)` | `DOS_AT(PspSeg, 0x81)`, until it is a function of `sys.h` |

And what each should know besides:

- P2a, P2b: `TRELIST` is done, and with it `place.c`; what is left of N
  is written out, but for `FIND` of `tree.c`. `MD_PLACE` wants a
  pointer into the main data. `(u16)(u32)(wcb->WinPath + 3)` of
  `read.c` and `enter.c` are offsets in a panel, and `(u16)(u32)name`
  of `files.c` is one in the segment of a list of files:
  `OFFSET_IN_SEG` is theirs. `(u16 *)(u16)(u32)cell` of `edit.c` is a
  far pointer kept as a near one, and wants `NEAR`. `MD`, `MDW` of
  `dialog.c` and `WAS` of `resident.c` are P3's and S3's, as the plan
  has them.
- P3: `dialog.c` has `AT` of `vc.h`, and `WORD_AT`, `FIELD`, `WORD`,
  `MD` and `MDW` of its own as they were. The record does not show the
  24 places that the table above names: go by the row of the piece,
  not by the count.
- P4: the words cut what they are handed to 16 bits, so a sum that is
  an argument of `AT`, `MD_POINTER` or `PANEL` wraps round as on DOS and
  needs no cast. `MAIN_SEG` is an int in 32 bits, the constant 1000h,
  and stands in nine sums with a segment or a count of paragraphs, in
  `find.c`, `read.c`, `copy.c`, `edit.c`, `view.c`, `info.c` and
  `start.c`: each is put into a word of 16 bits or cut by a mask.
- B1: the files compile as they stand with the command above, which
  needs `-i=` for the compiler's `h`, since `vc.h` includes `stddef.h`
  where `__I86__` is not set.
- S1: `start.c` reads the PSP through `PSP_WORD`, and `resident.c`
  through `PSP_EXIT` and `PSP_ENV`, all of `DOS_AT`; `main.c` has
  `DOS_AT` for the command tail and for the header of the device in
  `FatErr`.

### P2b, the viewer, the editor, a copy, the menus: done, and what a later piece should know

The eleven files of the row say `MD_PLACE`, `MD_POINTER`, `NEAR` and
once `OFFSET_IN_SEG` where they cast between a pointer and a number,
in four commits, each of which left `build/portable_c/VC.EXE` the same
bytes. The host check counted 65 places in them when the piece began,
the 66th, of `bars.c`, having gone with P1; it counts one, which is
S1's, and has ten of the files clean. `casts.py` has no place of N in
any of them. All eleven compile under `wcc386` with warnings as errors,
by the command of "The build".

| Files | Places | What they were |
|---|---|---|
| `window.c`, `menu.c`, `find.c`, `help.c`, `stcolor.c`, `sthelp.c` | 20 | The buffer that a window is kept in and put back from: `SavWin` and `ResWin` take it as a place and make the pointer, and their callers hand them `WinBuf`, `ErrBuf` and `ErrBuf1`. And two of the help's text, where a line is behind `HlpInfo`, both in `HelpBuf` |
| `copy.c`, `dirfunc.c` | 14 | The two names of a copy in `TmpBuf`, by the file's own `CpSrc` and `CpDest`; `CpBgNam`, a word of `TmpBuf` that holds where the source's name begins; the directory at `TreAdr`; the line in `DosBuf` that `F35_61` hands to `InSt10`. Six are DX as DOS left it, which the original goes on with, the address of a name: its place in the main data, in both memories the number that it is today |
| `edit.c` | 13 | `EditBuf`, which is `MenuBuf`, handed on with `MAIN_SEG` as a segment and an offset, and two pointers into it held against one another as numbers; the end of a name in `TmpBuf`; a window's buffer. And the two below that are no place of the main data |
| `view.c` | 17 | The words of `TmpBuf` where PgDn notes the lines of the next screen; `MenuBuf`, read on in by a search; a command of an extension file or a menu file, which `F34_30` and `F34_40` take and answer as a place of `MenuBuf`; the buffers of a box |

What each place points into was read before it was wrapped, as P1 has
it, and none of them is the text of the viewer or of the editor. That
text is in a segment of its own in both files and is numbers
throughout: a segment and an offset, or a count of bytes from its
beginning (`OFFSET_OF` and `AddressOf` of `edit.c`), made a pointer by
`AT` or `TEXT` and nowhere cast. What a copy reads a file into is the
same, segments from `ViewSeg` on. So nothing of those was wrapped, and
nothing of them is left to wrap.

Two places are no place of the main data, and have the word that is
theirs:

- `edit.c`, in `T72_00`: `(u16 *)(u16)(u32)cell`, a cell of the screen
  buffer that `HexByt` returns far, kept near. `NEAR`, as P1 said.
- `edit.c`, where the editor's help is written below its window:
  `(u16)(u32)line.line`, a far pointer into the tables that `CntLine`
  has stepped to the next text, taken for its offset there and handed
  to `T76_00` with the tables' segment. It is of the class T, and
  `MD_PLACE` of it would be a number that means nothing. It is
  `OFFSET_IN_SEG(code, line.line)`, `code` being what `AsmSeg` answered
  and what the pointer was made with: the same bytes, and in a flat
  memory the offset that `OFFSET` gives of a table.

Left, and whose:

- `view.c`, `Aexec0`: `md.EPB.CmdLine = (u32)MAIN_SEG << 16 |
  (u16)(md.TmpBuf + 100)`, the tail of the command line in the block
  that a viewer of the user's is run with, a far pointer of real mode.
  It is S1's, with `ProgramRun`, and the one place that the host check
  has in these files.

What a later piece should know:

- P2a: `F34_30` and `F34_40` of `view.c` take a place of `MenuBuf` and
  answer with one, and make the pointer themselves; `usermenu.c` hands
  the first `(u16)text`, which is `MD_PLACE(text)`. `InSt10` takes the
  line that is typed into as a place of the main data: `copy.c` hands
  it `MD_PLACE(line)`, and `cmdline.c` and `treebox.c` hand it
  `(u16)(md.MenuBuf + SPEED)`.
- P3: `SavWin` and `ResWin` make the pointer with `MD_POINTER`, so what
  `DialBox` puts into `struct Box` as `saved`, a word of a box's
  header, has to be a place of the main data and nothing else. It is:
  every such word of the tables says `MD_AT`. `InSt10` is in
  `dialog.c` and still casts what it is handed.
- P4: no line of this piece is a sum or a comparison that it was not
  before. `MD_PLACE` gives a number of 16 bits in both builds, and the
  places that are taken from one another or held against one another
  are put into a word of 16 bits or cut to one where they stand:
  `starts` of `view.c`, `cursor` and `i` in `T26_00`, `count` in
  `T00_00` of `edit.c`, `(u16)(MD_PLACE(NEAR(text)) -
  MD_PLACE(hd.HlpInfo))` of `help.c`. `MD_PLACE(md.TmpBuf) - 2` in the
  viewer's PgDn is a place two below the buffer, in `LowTab`, and is
  stepped up before it is written through.
- M1 and the convergence: the six places of DX as DOS left it rest on
  the main data being laid out as it is, which step 1 keeps. They are
  reached only where DOS refuses to say what a handle is or when a
  file was written; whether a scenario gets there was not looked up.

### P2a, the places of the lists, the tree and the command line: done, and what a later piece should know

The thirteen files of the piece make no pointer of a number and no
number of a pointer outside the words of `vc.h`: `treebox.c`, `tree.c`,
`panel.c`, `cmdline.c`, `enter.c`, `input.c`, `usermenu.c`,
`extfile.c`, `info.c`, `drive.c`, `files.c` and `read.c`, and
`place.c`, which P1 left so. All are clean in the host check, which
counted 62 places in them when the piece began. With P2b's files,
`casts.py` has N left in the common code in the files of P3 and S3
alone: `dialog.c`, `resident.c`, `main.c` and `start.c`.
`build/portable_c/VC.EXE` was the same bytes after each of the four
commits, and each of the files compiles under `wcc386` with warnings
as errors.

72 places, and one that was no cast, in 67 words that are written:
`MD_PLACE` 39 times, `MD_POINTER` 24 times, `OFFSET_IN_SEG` 4 times,
and `NEAR` inside `MD_PLACE` at three of them. The host check had
counted 62 of the 72.

What is otherwise than the plan had it, and what P2b, P3 and S3 look
for in their files:

- **A place that gcc does not count.** A pointer made of `TRELIST`,
  `(struct DIRENT *)TRELIST`, is made of a number, and the host check
  said nothing of it: the second body of `MD_PLACE` takes one address
  of the main data from another, gcc works that out to a constant
  when it compiles, and of a constant cast to a pointer it says
  nothing. Ten places of the piece were such: `LIST` of `treebox.c`,
  used five times, the two sums of the tree file there, and the start
  of the list in `tree.c`, in `PutTree` and in `FIND`. In 32 bits each
  would have been a pointer to the address that the place is as a
  number, below 64K. They are `MD_POINTER(TRELIST)` now. So "clean in
  the host check" does not say that a file has none left. What finds
  them: a copy of the tree in `build/` in which the second body of
  `MD_PLACE` is a function,
  `extern u16 md_place(const void *);` and
  `#define MD_PLACE(pointer) md_place(pointer)`, read by gcc with the
  host check's options; what gcc says of that and not of the tree is
  such a place. Over all the files that leaves one, which is S3's:
  `root = (struct DIRENT *)TRELIST` in `Init90` of `main.c`.
  Another constant made a pointer, one that does not come from
  `MD_PLACE`, is not found so, and nobody has looked for one.
- **A pointer to nothing has no place.** `MD_PLACE` of a pointer that
  is 0 is 0 on DOS and some other number where the memory is flat, and
  `MD_POINTER(0)` is a pointer to the main data's first byte there and
  none that compares equal to 0. One place counted on the first: the
  user menu hands `Run_00` its pointer into the table of keys for
  whether a file is being viewed, the register BP of the original, and
  clears it before a line that names another menu. `key = 0` is
  `key = (u16 *)MD_POINTER(0)` there now, the pointer whose place is 0,
  which is the same bytes. Where a pointer is cast to a number, look
  at whether it can be 0; where a number is made a pointer, at whether
  the pointer is held against 0 afterwards. The files of this piece
  have no other, by reading every place that was wrapped.
- **What is no place of the main data**, and is `OFFSET_IN_SEG`:
  - `enter.c`, `F23_02`: whether the name left of a panel's path is
    behind its root. Both are in the panel, and the segment is the
    panel's.
  - `read.c`, `ReDr00`: the index into `ColTab` that is where the
    fourth character of the path is in a panel, a bug of the original
    that stays. `OFFSET_IN(struct WCBdef, WinPath) + 3` says the same
    with no pointer in it, and is other bytes: left for step 2.
  - `files.c`, `NearFl`: the offset of the pointer that it is handed,
    for where an entry is in the old list of a panel. The routine is
    not told which segment the old list is in, and a pointer does not
    say it in a flat memory. It names `md.ViewSeg`, where `ReDr50` of
    `read.c` has the old list, the one routine that calls it; the
    first body does not read the segment, so nothing changed in 16
    bits, and nothing there shows it if a second caller comes with a
    list elsewhere. Handing `NearFl` the segment and the place in two
    arguments would say it better and is other bytes: step 2 makes it
    a pointer to the list and a place.
- **A number stepped inside a cast.** `c = *(const u8 *)entry--` of
  `F26_00` in `enter.c` steps the number that it makes a pointer of.
  The words name what they are handed once in each body, but the step
  is a statement of its own now, and that is the same bytes.
- A far pointer that a routine is handed and that every caller has
  into the main data is `MD_PLACE(NEAR(pointer))`, as for what
  `MvzLine` returns: `Found` of `extfile.c`, which `FndItem` answers
  through, and `CutPath(md.ViewFil)` in `Run_00`.

The routines of this piece that take or answer a place of the main
data as a word, for those whose files call them: `F24_01` takes the
path, which `main.c` hands it as `(u16)md.TmpBuf`; `Run_00` takes the
text of a command; `FindExt` answers where one is, which `view.c`
goes on with, and `FndItem` where an entry is. And those of other
pieces that this one calls with `MD_PLACE`: `SavWin` and `ResWin` of
`window.c` take the buffer, `InSt10` of `dialog.c` the text that is
typed, `F34_30` of `view.c` the text of a command.

Not looked at, being P4's: what is added to a place outside a word,
`MD_PLACE(md.InfBuf) + md.InfoLen` of `info.c` and `md.HstAdr - 1` of
`enter.c`, where an int of 32 bits does not wrap round.

### B1, the objects: done, and what a later piece should know

Every build compiles `src/portable_c/*.c` a second time, with `wcc386`,
into `build/portable_c32/`: an object and its listing for each file,
61 of them, as the target `portable_c32`, which is of the default
build. Nothing is linked of them and no test reads them. What they are
for is that the build stops at a line which the compiler for 32 bits
does not take, with every warning an error, as it stops for the one for
16 bits. `add_portable_c32` of `CMakeLists.txt` does it, some 100 lines
with what they say of themselves, where the plan had 50.

    wcc386 -q -3r -mf -s -zl -zls -wx -we -os -i=<Open Watcom>/h -fr

What a piece sees of it:

- `Compiling portable_c/NAME.c for 32 bits`, and under it the
  compiler's error, when a change does not compile there. The 16-bit
  program may have been built by then and pass every test: the piece
  does not land all the same. It is mended in the common source, for
  both, as "What a piece goes by" has it.
- `build/portable_c32/NAME.lst` is what `wcc386` made of a file, beside
  `build/portable_c/NAME.lst`: what a word of `vc.h` comes to in its
  second body is read there.
- A new file of the tree is compiled for both without a word in the
  build, as before.

The files that are not of both builds are three lists, and each name on
one has to be a file that is there, or the build does not configure:

| List | Files | What the build does | Comes off |
|---|---|---|---|
| `portable_c_16_alone` | `arith.c`, `dos.c` | Compiles them for 16 bits alone. They are not tried for 32 | Z1, with the files |
| `portable_c_32_alone` | `dos32.c` | Compiles it for 32 bits alone. It was D1's line in `add_portable_c` | Z1, when it becomes `dos.c` |
| `c_waiting`, in `add_portable_c32` | `resident.c` | Tries it for 32 bits at every build, `Trying portable_c/resident.c for 32 bits`, by `tools/refused.cmake`: what the compiler said is left as `build/portable_c32/resident.refused`, and no object. When the compiler takes the file, the build stops and says to take it off the list | S3 |

The piece was set out with `arith.c`, `dos.c` and `resident.c` on one
list of what cannot compile yet, each to be tried. They are two kinds.
`resident.c` is to be a file of the 32-bit program and is not ready;
trying it is what tells S3 that it is. `arith.c` and `dos.c` are never
to be of it: that they do not compile there, for registers of 16 bits
in their `#pragma aux`, is nothing that the work rests on, and a build
that stopped on the day one of them compiled would stop for nothing.

`resident.c` is refused for one line, twice: `Exit05` hands the address
of the registers and of the flags that it was entered with to `Exit02`,
and what an interrupt function is entered with is far to `wcc386` as
well, where `STACK` is nothing: W112, "Pointer truncated". The list
does not hold a file to its reason: one that comes to be refused for
something else stays refused, and nothing says so. Read
`resident.refused` when the file is touched.

What was decided:

- The compiler's directory of headers is named, `-i=`, for `stddef.h`.
  The plan had `vc.h` write `offsetof` out for `__WATCOMC__`, so that
  the build names none. That was tried: one word of `vc.h`, and all 61
  compile without the option. It was left, because of the order that
  the portable C goes by: `offsetof` of the header is C, and what
  `vc.h` writes out in its place is Open Watcom's, there for a build
  that has no header. The 32-bit build would be held to Open Watcom's
  way by it, and after Z1 there would still be a line for
  `__WATCOMC__` where otherwise the whole `#ifdef` goes. `stddef.h` is
  a header that needs no library. And the directory is needed for more
  than `offsetof` as soon as P3 lands, right behind this piece: its
  `NUMBER_OF` has `size_t` of the same header in its body for 32 bits,
  which `vc.h` has from that include where `__I86__` is not set. So
  the option is not one to take out again by writing `offsetof` out.
- `-fr`: `wcc386` writes no file of its errors. It would be
  `build/NAME.err`, the name under which `wcc` leaves its own of the
  same file, and both run at once.
- Listings are written, by `wdis`, as for 16 bits. All 61 cost 0.14 s
  of the processor.
- The directory and the target have the name that the plan has for the
  tree of the harness, `portable_c32`.

Release and debug. The plan has two programs from B2 on, and B1 makes
the objects of the first. `add_portable_c32(target directory
option...)` is a call for each: what the two have in common is in the
function, and what differs is the caller's, the target, the directory
and the options behind them. Today's one call is

    add_portable_c32(portable_c32 ${CMAKE_BINARY_DIR}/portable_c32 -os)

and B2's second would be

    add_portable_c32(portable_c32-debug
        ${CMAKE_BINARY_DIR}/portable_c32/debug -od -d2 -dCHECKED)

with the link written once, in the function, of the objects of its
directory. Not tried: the second call was not made, since nothing reads
`CHECKED` yet and unoptimised objects that nothing links are held to
nothing more than the first are.

For B2:

- The function ends with `made`, the objects and the `.refused` files
  together, as what the target depends on. A link wants the objects
  alone; a file that is still waiting then is in neither program.
- The options of a file are one variable, `compile`, the same for
  every file. Where a file wants its own, as the tables may for "What
  could go wrong", 14, the 16-bit build's `c_compile` shows the shape:
  the options behind the common ones, by the file's name.
- `wdis -s` puts the lines of the source into a listing where the
  object has them, `-d1` or more: the debug program's listings.
- `.err` files: see `-fr` above. A linker's file of errors was not
  looked at.

What the build costs. A build from nothing, `cmake --build build
--target clean` and then the build, with and without the target
`portable_c32`, turn about, three times each, on a computer that other
agents were testing on (load 31 to 40):

| | Without | With | More |
|---|---|---|---|
| The processor's time, `-j8` | 9.3, 9.7, 9.4 s | 10.8, 11.2, 11.1 s | 1.6 s, a sixth |
| The processor's time, one job | 10.1, 10.4, 10.4 s | 12.1, 12.0, 12.2 s | 1.8 s, a sixth |
| Elapsed, one job, under that load | 76, 94, 87 s | 106, 105, 99 s | a fifth |

Elapsed with `-j8` was not to be had that way: the three without took
19 to 21 s and the three with 13 to 16, which would be the build going
through the targets one behind the other when they are named to it;
that was not looked into. Before the change and with the computer less
taken (load 6 to 9) a build from nothing was 5 s with `-j8` and 16 to
20 s with one job; a sixth more of that is one second and three. A
file changed is two compilers run where it was one, some 20 ms of the
processor more.

`harness/trial32/whole.py`, and what of it the build has taken over.
It copied the tree, compiled every file itself, without `-wx -we`, to
see which compile, and put a file of functions that do nothing in
place of `dos.c`. Whether the tree compiles for 32 bits is the build's
to say now, at every build and with warnings as errors, and the
script's own compiling of 61 files said the same less strictly. And
since D1 the tree has `dos32.c`: the script compiled that too, and the
linker took `dos32.c`'s 84 functions and wrote 84 warnings for the
script's own. So the script was changed, in a commit of its own, and
its linking kept:

- It has the build make the target `portable_c32` and links those
  objects, `dos32.obj` among them.
- It compiles itself only what the build does not make: each file that
  has a `.refused` and no object in `build/portable_c32/`, with its
  warnings, which is `resident.c`; and `Vector`, `SetVector`, `IntOn`
  and `__GETDS` as functions that do nothing, which are of `dos.c` and
  no part of `sys.h`, until S2 and S3 change them.
- It prints 292,152 bytes at the head that P3 landed with: 130,977 of
  code, 92,720 of everything else, 5,248 relocations. The figure moves
  with every piece, and is what the script prints, not what stands
  here. When the script was changed it printed 290,892, 129,904 and
  5,073, where as it stood, with D1 landed, it printed 290,900,
  130,141 and 5,074: the difference is its own 84 functions, which the
  linker kept the code of. P1 had 286,640 with every function of
  `sys.h` doing nothing.

It is still the one thing that links the tree for 32 bits, and what an
S piece runs to see that it does; B2 takes that over, and the script
can go then.

### B2, the program: done, and what a later piece should know

Every build links the portable C for 32 bits, twice:
`build/portable_c32/VC.EXE`, the program, and
`build/portable_c32/debug/VC.EXE`, the same source for finding a fault,
each with its map `VC.MAP` and its objects and listings next to it. The
bench starts both. `add_portable_c32` of `CMakeLists.txt` does it, 125
lines with what they say of themselves, 75 of them new, where the plan
had 100.

    wlink option quiet format os2 le name VC.EXE
          order clname CODE clname STACK clname DATA clname MAIN clname BSS
          file <the objects, dos32.obj last>
          option map=VC.MAP,start=Start_,static
          option stub=<Open Watcom>/binw/dos32a.exe
    stub VC.EXE

What there is:

| | |
|---|---|
| The targets | `portable_c32` and `portable_c32-debug`, both of the default build. The second call is as B1 had it, `-od -d2 -dCHECKED`, and its directory lies inside the first's |
| `tools/stub.c` | Clears the bit of the banner in the file that the linker wrote, bit 3 of the byte 15h behind `ID32`, and says at its top how the bit was found. It stops, and the build with it, for a file whose stub has the letters not once in its first 512 bytes, or has another number than 090Ch at 18h of the block, which is taken for the extender's version: the bit is known for that one |
| The stack | `LenStk` of `vc.h` has a second number where `__I86__` is not set, 4000h. So `stack.c` is 16K in the program of 32 bits, with a value and in the file |
| `harness/layoutcheck.py` | The tests `portable_c32-layout` and `portable_c32-debug-layout`: the file and the map against the layout that the program rests on. Below |
| `LICENSE.DOS32A` | `binw/license.d32` of the pinned download, the same bytes. The README says where it is from and what it covers, under "Licence", and has the sentence that the licence asks for under a heading of its own, "The extender", behind "The program of 32 bits", which says what the two programs are |
| `tools/figures.py` | Nine rows of the program of 32 bits behind those of the 16-bit one, which say what they said |
| `harness/trial32/ran.py` | The functions of a flat program in the order in which each first ran on the bench, by QEMU's log: where the program stops. Below |
| Gone | `harness/trial32/whole.py`, which linked the tree by hand; `tools/refused.cmake` and the list `c_waiting`, which were empty |

**The order of the segments, which is not the plan's.** The plan had
the code, the constants and the tables, the main data, what has a
value, the stack, and what has no value. Three things that the pieces
before this one wrote down do not let it be so:

- The tables that the program writes to are of `_DATA`, and have to be
  within 32K of `Title_S` ("What could go wrong", 14). `_DATA` is 282
  bytes, five texts of `stopts_t.c` and two variables of `dos32.c`,
  and is
  of the class `DATA` as the constants are: the class is linked whole,
  `CONST2` and behind it `_DATA`, in front of the main data.
- `IniSave` of `setup.c` has to end within 64K of where `md` begins
  (S3), and has no value, so what has no value has to follow the main
  data at once: the main data leaves 2,932 bytes of the 64K, and
  `IniSave` ends 2,347 short of them. Nothing can stand between `MAIN`
  and `BSS`, the stack least of all.
- What has no value has to be last, or it is in the file.

So the stack is in front of the main data. Between the tables and the
main data it does link: `Title_S` is then 32,744 bytes in front of
`md`, 24 short of the 32,768 that `TABLES_BY` allows, and the next
text that is added behind `Title_S` breaks it. So it is first of the
data, in front of the constants: `Title_S` is 16,360 bytes in front of
`md`, and the tables reach from 8,596 in front of `Title_S` to 16,352
behind it, of 32,768 each way.

What that costs is the rule of the page. The extender packs the data
behind the code, in the same page of 4K, and what shares that page now
is the low end of the stack, 816 bytes of it as the file is linked
today and never more than 4,095. A program writes there when it has
used three quarters of its stack, and the scenarios' test fails a
scenario that uses half. So the rule is held in this form: nothing
that a scenario writes lies in a page that holds code, the upper half
of the stack being what a scenario may write of it. The plan's limits
have the line. Should the half become another fraction, `pages` of
`harness/layoutcheck.py` has it once.

`dos32.obj` is linked last, whatever its name. A segment holds what
the objects have of it in their order, and what `dos32.c` has without
a value, the 640 KB above all, is then behind `IniSave` and everything
else of `_BSS`. The check was run with 640 KB linked so: it passes,
and the file is no longer for it. Linked in the order of the names,
`dos32` in front of `setup`, `IniSave` ends 718,548 bytes behind `md`.

What else was found of the linker and the compiler, which
`CLAUDE.md`, "Notes on the portable C", has as notes: `md` is in the
file, as zeros, though it has no value; `option stack=` makes a
segment of the class `STACK` longer; the name of the file is in the
file; the compiler gives functions that do nothing one address.

**Otherwise than the plan had it.**

- The order, above.
- The stack's length is the source's, `LenStk`, and not `option
  stack=16k` of the linker, which also works: it makes the segment of
  `stack.c` 16K whatever the source says. Then `stack.c` would say
  4,096 bytes of a stack that has four times as many.
- `tools/stub.c` is C, as the plan has it, and 40 lines for 10: it
  refuses a stub that it does not know. The trial's `quiet.py` stays
  for the toys, which `build.sh` builds.
- The debug program is linked without the linker's `debug` directive:
  what `-d2` puts into the objects is for the listings, which have the
  lines of the source between the instructions (`wdis -s=`), and is
  not in the file. No debugger runs on the bench. Its code is 220,278
  bytes for 130,211.
- The layout has a test of its own, which starts no machine, and is
  not a part of the scenarios' test: H3 can call it or leave it.
- `LICENSE.DOS32A` has nothing at its top. It is the file of the
  download byte for byte, and the README says where it is from.

**`CHECKED`, and what is missing.** The debug program is compiled with
`-dCHECKED`, and nothing reads it: `vc.h` has no word of it, so the two
programs differ in how they are compiled and in nothing else. What the
plan means by it has to be written, and was not, since nothing could
have run it:

- A third body of the words of the addresses, under `CHECKED` where
  `__I86__` is not set: `AT`, `PANEL`, `MD_POINTER`, `MD_PLACE`,
  `OFFSET`, `OFFSET_IN_SEG`, `SEG_OF` and `NEAR`, each of which says in
  its comment what it takes for granted. That is what a check holds: a
  segment that is the main data's, the tables' or one that the memory
  handed out, a place within the main data, a table within reach.
- A function that they call, which knows how far the program's memory
  goes, so `dos32.c`'s to answer or to be asked, and that stops the
  program with a message that says the file and the line: the console
  and `ProgramEnd` of `sys.h`, with the screen as it is.
- `OFFSET` stands in no value of a static object today and `MD_PLACE`
  is a constant to gcc in places (P2a): a body that calls a function
  is neither, which the host check does not see, since it reads the
  bodies without `CHECKED`.

It is a piece for whoever first has a wild read to find, M1 or the
convergence.

**The check of the layout.** `python3 harness/layoutcheck.py --stub
<dos32a.exe> VC.EXE VC.MAP` prints where things are and fails for what
is wrong; its top says which word of `vc.h` goes by each thing.

| What is held | Seen to fail for |
|---|---|
| Every object of a file of tables, a module of the map whose name ends in `_t`, lies in the 64K that begin at the paragraph which 32K below `Title_S` is in, as `OFFSET` counts | 30,000 bytes of constants linked behind `commands_t.obj`: 79 of the 509 objects outside, the first `PtMnu_S` |
| 32K below `Title_S` is not in front of 64K below `md`, nor a megabyte behind that | 20,000 bytes of constants behind `main_t.obj` |
| `IniSave` lies behind `md` and ends within FFFFh of where that begins | The plan's order, 79,572; the trial's, 88,728; 640 KB linked in front of `setup.obj` |
| The program ends no more than F0000h behind `md`, 983,040 bytes | 960 KB without a value linked last |
| The stack is the segment that ends where the header has the stack's top, `coverage.flat_stack`, of the class `STACK`, as long as the linker's summary says, and in the file as zeros | |
| The file has of the data all that lies in front of the first segment of the class `BSS`, and nothing of another class lies behind that | The trial's order and no order at all, which have the stack last: 593 bytes of zeros in the file today, and 640 KB with the memory |
| No segment that is written begins in a page of the memory that has code, by `le.BENCH`: all of the data but `CONST`, `CONST2` and the lower half of the stack | The trial's order and the main data behind the code |
| Two objects, the code whole in the file, each as long as the map's segments in it; the entry at `Start_`, and as the linker's summary has it | |
| Every relocation an address, in what the file holds, with the offset of what it names there; none a selector | |
| The stub is `dos32a.exe` behind its header but for the banner's bit, which is clear | A link without `tools/stub.c`; CauseWay's stub, which `tools/stub.c` refuses too |

The rows with nothing in the second column were not seen to fail: no
link was found that breaks them alone. The stack's are held by what
H2's toys showed of `coverage.flat_stack`. That a table is known by
`OFFSET` only if it is of a file of tables was looked at once, by the
names that the tree hands to `OFFSET`, 174 of them, against the
modules of the map: every one is of a `_t` file. The test does not
look at that.

What it cannot hold: that the 640 KB begin on a paragraph of the
megabyte, which is `dos32.c`'s to see to when it has them; and
anything of the debug program beyond its layout.

**On the bench: where the program stops.** Started as the harness
starts any program, `TMPDIR=$PWD/build/tmp python3 harness/show.py
--program build/portable_c32/VC.EXE`, with the driver told what
`driver.facts` makes of the file (`drivercheck.py --facts`: `AutoTim`
at 0012AB89h, `Clock` at 0012A18Bh, the stack 16,384 bytes at
0011FCD0h), the first screen is, with the cursor at column 8 of row 3:

    Vibe Commander 0.1, based on Volkov Commander 4.05
    Copyright (C) 1991-2000 Vsevolod V. Volkov, (C) 2026 Maksym Motorny
    Program too big to fit in memory
    C:\WORK>

Nothing of the extender is on it: no banner in front, and no warning
behind, the program having taken no vector. The debug program shows
the same. By QEMU's log (`run.py --trace`, `ran.py`), 154 blocks of
the program's code ran, 27 of them with an address that the extender
had put in, and every one is what the reader has at `le.BENCH`: the
code is at 00100010h, the data at 0011FCD0h, and `md` at 0012A094h. No
fault, and nothing of the second bodies of the words went wrong as far
as it got, which is `AsmSeg` and little more.

The way there, in the order in which each function first ran, from the
debug program, which gives every function a place of its own; the
functions of `sys.h` that are not made are marked:

    Start, Main,
    Alloc3: SystemVersion, MemOwn, SetStat: MemSetSearch*, MemSetUpper*
    Init00: ProgramEnvironment*, AsmSeg, MnPth0, CutPath,
            ReadOne: FileOpenBare,
            DriveCurrent, DirCurrentBare, ChkMode: ScreenState,
            MvzTxt, ConsoleText,
            SetBrk: BreakCheck, SetBreakCheck, CtrlCTake*,
            TakeEnd: ProgramEndedTake*
    Init11: MemTake*
    AlocEr: OutStr0, OutStr, ScreenType
    Exit00: ProgramEndedGive*, Exit02, ProgramEnd

- The first function of `sys.h` that it calls and that is not made is
  `MemSetSearch`, in `SetStat`, and `MemSetUpper` behind it: both do
  nothing, and nothing shows.
- The first that changes what happens is `ProgramEnvironment`, in
  `Init00`: it answers an environment of no texts with no path behind
  it, so the program does not know where its own file is. `ReadOne`
  asks DOS to open the setup file and the extension file by their
  names without that path, which is in the directory that DOS is in,
  and gets neither: `FileReadBare` never ran. The program is on its
  defaults, and the bench's setup, which has the clock off, is not in
  it.
- `CtrlCTake` and `ProgramEndedTake` take nothing.
- It stops at `MemTake(0xFFFF)`, in `Init11`: the function answers
  that it failed, with no paragraphs as the largest block. `Init11`
  holds that against what the viewer and two panels need, answers
  failed, and `Round` goes to `AlocEr`, which writes the line and
  leaves by `Exit00`, with DOS's prompt behind it. That is the 16-bit
  program's own way out for a machine with too little memory.

So the program does what it is written to do with those answers, as
far as `Init11`, and M1 begins where D2 has `MemTake`, the PSP and the
rest: the next thing that the program asks is `Memory` of `start.c`,
which cuts the block up by segments, and `IniData` behind it.

Not seen, since the program never idles: the stack as the driver
writes it out, and a step that waits on `AutoTim`. H2's list of what
waits for the first run stands as it is. Its toy of the system's
functions has the tree's `stack.c` and so the 16K: `python3
harness/drivercheck.py` finds 16,384 bytes of stack for `sys.c`, at
001041ACh, 15,874 of them never used, where H2 wrote down 4,096 and
3,598, and says "Nothing wrong"; `python3 harness/trial32/both.py`
passes as before.

For D2:

**The 640 KB, first of all.** In this layout it has to be a plain
variable of `dos32.c` without a value, no `#pragma data_seg` and no
`= { 0 }`: it is then of `_BSS`, and since `dos32.obj` is linked last,
the last thing of the program, behind `IniSave` and not in the file.
And its start has to be rounded up to a paragraph counted from `&md`
itself, an address that is `&md` and a multiple of 16, since that is
what a segment of the megabyte names: the linker puts the variable on
no such address by itself.

- Keep the 640 KB a variable of `dos32.c` without a value, so that it
  is of `_BSS`: it is then last in the program and not in the file,
  and the check holds both. One in a segment of its own, by a `#pragma
  data_seg`, wants its class in the linker's `order`, behind `BSS`, and
  the check wants to know that class for what has no value (`held` of
  `layoutcheck.py` takes `BSS` alone for that).
- It ends within the megabyte with room to spare: the program ends
  63,197 bytes behind `md` today, of 983,040, so 640 KB behind that
  leave 264,483.
- Where it begins is the linker's to say, and not on a paragraph of
  the megabyte by itself: a paragraph is an address that is `&md` and
  a multiple of 16, whatever `&md` is. `md` is at 0012A094h on the
  bench.
- `Start` is where the linker's command has the program begin,
  `Start_`; the test fails if the entry is anything else.
- The stack is 16K, `LenStk`, and `Stack` of `stack.c` is where it is,
  should the critical error want to go over to it.

For H3:

- The tree is `build/portable_c32/VC.EXE` with `VC.MAP` beside it,
  which has the static symbols. `scenarios.run` and `show.py
  --program` take it as it is. The debug program is in a directory
  inside that one: a pattern over `build/portable_c32/` that goes into
  directories finds both.
- `layoutcheck.Check(contents, map).all(stub)` answers whether the
  layout holds, with `problems`, `found` and `room`, if the scenarios'
  test is to say it too. The two limits of the 16-bit program that
  `check_layout` holds are in the plan's table with what each becomes.
- In the program that is shipped several functions are one address:
  the compiler gives a function no instructions of its own where the
  function behind it in the file has the same, one that does nothing
  or that is the same call. Nineteen names of `dos32.c` are three
  places today, of 5, 5 and 9. A function is then seen to have run
  when another of the same place did. The debug program has no such.
- A piece of a function may stand in front of its label, behind the
  function before, with no name in the map: what `ran.py` says of it.

For H4: a table that is written to is of `_DATA`, behind `CONST2`, in
the same object as the constants; the modules of the map whose names
end in `_t` are the files of tables, 509 symbols.

For M1: the report above. `harness/trial32/ran.py` with the debug
program says where a run stops; `python3 harness/le.py` with a file
and its map says where everything is on the bench. A fault of the
processor does not show on a screen: `run.py --qemu "-d int -D FILE"`
has QEMU log it, as the trial did for CauseWay.

### P3, the tables read as what they are: done, and what a later piece should know

A word of a table that names something, which `tables.h` has as a
pointer, is read as a pointer wherever code reads one, and asked what
it names through two words that `vc.h` has beside `OFFSET`, each with
its two bodies:

| Word | Takes | Gives | The first body | The second body |
|---|---|---|---|---|
| `NAMED(names)` | A word of a table that names something: a line of a box, the box of a dialog, the buttons of a field, a table of `Menu_T`, a half of a picture of `StPagTab` | What it names as a number, its offset in the tables' segment: what `OFFSET` gives of a table that the code names itself, and what `DialBox`, `MinMnu`, `MnMn40` and `AT` take | The word: `(u16)(names)` | `OFFSET(names)`: the word is a pointer |
| `NUMBER_OF(names)` | A word of that kind that holds a number, written with `NUMBER` of `tables.h` | The number | The word | The pointer's low 16 bits, by way of `size_t` |

So a pointer is still handed on as an offset, which is what step 1
keeps: `AT(code, NAMED(word))` is the pointer to what a word names,
in both builds, and in 32 bits it is the word itself by way of its
offset and back. Step 2, when `Dialog` and `DialBox` take pointers,
writes the word where that stands, and `NAMED` goes with `OFFSET`.

What was read so, 24 places in three files:

| File | Places | What |
|---|---|---|
| `dialog.c` | 11 of `NAMED`, 9 of `NUMBER_OF` | `DialBox` reads a line twice, once for the width and once to write it. `Dialog` reads `pDiBox` three times, for `DialBox`, for the colours and for `MinMnu`, and `mCtrl` of buttons six times; `mCtrl` of a line that is typed into, of a group and of a box to tick is a number, nine times. Its own `WORD`, which was the cast for both kinds, is gone, and `WORD_AT` with it |
| `main.c` | 1 of `NAMED` | `F39_01` reads the word of `Menu_T` that names the table of a menu's commands. `Menu_T` is declared there as what it is, an array of pointers, and stepped over by `sizeof *Menu_T`. The commands in such a table are numbers of 16 bits in both builds, and that line stays |
| `stcolor.c` | 3 of `NAMED` | `struct Picture` has two pointers where it had two words, as the nameless structure of `StPagTab` in `stcolor_t.c` has, and the three places that read one ask `NAMED` |

`DialBox` no longer goes by the numbers 4, 5, 6, 8 and 12 for where a
box has what. `tables.h` has `struct BOXTOP`, what every box begins
with, the `BOX_HEAD` of the three macros under a tag, held to 8 bytes
by `LAID_OUT`; the texts that a box puts in are `struct BOXTX` behind
it, and the lines stand behind those: `const u8 * const __far *line`,
a far pointer to a word that names a text, stepped by `line++`, which
is 2 bytes on DOS and 4 in 32 bits. A line across the box is the
pointer 0 and is looked for in the word itself, `*line != 0`, before
`NAMED` is asked: `NAMED` of a word that names nothing is a number
like any other in 32 bits. `Dialog` walks its fields by `sizeof(struct
BOXHD)` and `sizeof(struct BOXMN)` as it did, which is right in both.

The 17 of N in `dialog.c` went with it: `MD` and `MDW` are made of
`MD_POINTER`, `InSt10` makes its pointer with it, and the two calls of
`InSt10` hand on `MD_PLACE(text)`.

The plan had 40 places of T for this piece, by the sorting of the
three files. Of those, 16 were the `WORD` of `dialog.c`, which the
host check counted to the last; six more read such a word with nothing
counting them, two in `DialBox`, one in `main.c` and three in
`stcolor.c`; and the mouse's loop of `Dialog` has three reads where it
had one. The rest take a table by its offset, `AT(code, box)`,
`FIELD(item)`, and are right as they stand. Nothing else in the tree
reads such a word: the files of tables have pointers in boxes,
dialogs, `Menu_T` and `StPagTab` and nowhere else, and `menu.c` and
`box.c` read none.

What decided: the scenarios, all of which pass. `main.c` and
`stcolor.c` compile to the bytes they had, by their listings. Of
`dialog.c`, `DialBox` is 6 bytes longer, for walking a far pointer
where it added to an offset, and `Dialog` 11, where the mouse's loop
over the fields read `mCtrl` once before it knew the kind of the field
and now reads it by the kind; the other fifteen routines are the same
bytes. `VC.EXE` is 20 bytes longer. `LAID_OUT` and the test of the
data hold the tables as they did, and no file of tables was changed.
The host check has `dialog.c` clean and counts 105 places where it
counted 138, P2a and P2b having landed in front of this.

The second bodies are read by the compilers and no more, as P1's:
the build compiles them for 32 bits, with warnings as errors, since
B1; `whole.py` links the tree with them, and `wcc386` makes of a line of
a box a read of four bytes, a step of four and the offset of what it
found. They were tried once on this computer as P1 tried its own, in
a file that is not kept: gcc for 64 bits, a main data with 64K in
front and a megabyte behind, a box of two texts and three lines, one
of them a line across, and a dialog of two fields, at every place in
a paragraph, in front of the main data and behind it; walked as
`DialBox` and `Dialog` walk, each word gave what was put there.

What a later piece should know:

- H4: which fields are pointers is as the plan has it, five shapes.
  `struct BOXTOP` is no shape of its own and no object has it. A line
  across a box is a pointer without a relocation whose four bytes are
  0, and a `NUMBER` is one whose high word is 0: the program cuts it
  to 16 bits and never looks at the rest. `struct Picture` of
  `stcolor.c` is `StPagTab`'s declaration a second time, and nothing
  holds the two to one another but that they read the same in both
  builds today.
- M1 and the convergence: `NAMED` takes for granted what `OFFSET`
  does, that what a word names lies within 32K of `Title_S`: a text,
  a box, buttons, a table of commands, a picture, all of files of
  tables. A box or a dialog that shows a wrong line where its other
  lines are right has a text outside that.
- On DOS a pointer in a table is near, and code that reads through
  one as it stands reads the main data, with no warning: `NAMED`
  first. Nothing holds the code to that but the scenarios.
- A name for the type of such a word was thought of and left: a
  typedef more in a header can change the order of two variables
  (`CLAUDE.md`, "Notes on the portable C"), and the pointers of
  `tables.h` say what each names by their own types, `const u8 *` for
  a text, `const void *` for a box or for buttons or a number.

### P4, the arithmetic: done, and what a later piece should know

`harness/int16.py` finds where the portable C can tell an int of 32
bits from one of 16, and the test `portable_c-int16` holds the tree to
`harness/int16.txt`, which has every such place with the reason why it
is the same in both builds. A place where it is not, or is so only by
what cannot be seen there, got the cast that says what DOS does, and is
no place any more. `build/portable_c/VC.EXE` was the same bytes after
each commit, and every file compiles for 32 bits.

The finder is not the plan's script any more. That one went by the
shape of an expression: an operator on numbers of 16 bits, and what
looks at it. Held against the tree by eye it named what is no place
and was silent on what is one, so it was written anew, to work out by
the rules of C what each expression is in both builds: its type where
an int has 16 bits and where it has 32, and between which numbers its
value lies. An expression is the same in both when its value fits the
type that it has on DOS; a place is where one that does not fit is
looked at. The top of the script has the whole of it.

| | The plan's script | The finder |
|---|---|---|
| Was silent on | `x /= a - b`, `p += a - b`: an assignment that works. `a > c - d` with two bytes: a number with a sign against one of 16 bits without. `~0`, `1 << 15`: what is worked out when the file is compiled. A function that clang refuses, which was passed over without a word | Finds them; stops with clang's words for a file that it does not take whole. The tree has three of the first, each a pointer stepped by a sum, `screen += apart + 1`, and none of the others |
| Named needlessly | Some 200 shifts in the second bodies of `OFFSET` and `SEG_OF`, once P1 had landed. 8 of `widen`, most of them where a length, which is a number of 32 bits without a sign there, makes a sum one that is put back into 16 bits at once: `FILE_ADDRESS` of `place.c` | What is written in a second body of `vc.h` is no place; what is put back into 16 bits is none |
| Did not know | How long anything is, or what a loop's variable can be | `sizeof` and `offsetof`, which clang is asked for: `INI_LEN` is 511. And a variable inside the loop that counts with it, `for (i = 0; i < 13; i++)`: `md.TmpBuf[100 + i]` is no place there. 57 places are none for these two |
| Added to a pointer | Any number that may leave 16 bits either way: 73 | Three kinds. One that may be 65,536 or more, `index`: 64K further on in a flat memory. One that may only be below 0, `back`: back in both builds. One that is within 16 bits whatever its sign on DOS, `MD(field->mPoint) * 160` of bytes: no place, since DOS adds it to an offset of 16 bits |

It is held against `harness/int16check.c` every time it runs: 69 lines
each with the kinds that are to be found in it, and over a hundred in
which nothing is to be found.

What it counted, at `e345195`, and what became of each:

| Kind | Places | Got the cast | In the record |
|---|---|---|---|
| `compare` | 16 | 8: `help.c`, `info.c` twice, `keys.c` four times, `view.c` | 8 |
| `divide` | 3 | 2: `FILE_NUMBER` and `TREE_NUMBER` of `place.c`, where they are used | 1 |
| `shift` | 9 | 1: `tree.c` | 8 |
| `index` | 32 | 1: `ENTRY(wcb, file + gap)` of `files.c`, by `ENTRY` | 31 |
| `back` | 19 | 9: the viewer's `WORD_AT(text, at - 2)`, by `WORD_AT` | 10 |
| `signed`, `widen`, `truth`, `address` | none | | |
| all | 79 | 21 | 58 |

The plan had some 50 to read and 73 to look over. That no place is of
`widen` or `truth` is the translation's doing: it says `(u16)(` 244
times and `(u32)` before a product.

**Cast or reason.** A cast where the number is a place of the main
data and a length (`info.c`, which P2a had listed), a difference of two
addresses or of the cursor and the first entry shown, a sum that the
code itself cuts a line below (`keys.c`), or a count that is a byte
with its sign (`help.c`). A reason where the line above asks that the
number fits, or the lines above set it. Some fifteen reasons rest on
what is not at the place, and say what: that an item of a menu, a row
of the screen, the number of a colour or of a drive is small; that DOS
holds the environment to 32K (`start.c`, twice); that no text has
65,535 characters (`line.c`).

**A number added to a pointer**, which the plan had as 73 places to
look over, comes to this. On DOS it is added to an offset of 16 bits,
which wraps round; in a flat memory nothing wraps. So:

- A sum that carries out of 16 bits is always another byte in a flat
  memory, 64K further on. Where the pointer is the beginning of a
  segment the cut says what DOS does, `text + (u16)(at)`, since the
  megabyte has at every offset of a segment what DOS has there. That
  is why `ENTRY` and `WORD_AT` cut.
- A number below 0 steps back in both builds, to the same byte unless
  the pointer is less than that far behind the beginning of its
  segment. `md.DosBuf[cursor - 1]` is such, and so is every `text--`
  that no finder names. No cast mends that: a cut number steps 64K
  forward in a flat memory.
- So nothing that is written at a place makes a pointer wrap round as
  DOS does, but for a pointer that is a segment's beginning. A pointer
  that the 16-bit program does step round the end of a segment is found
  by the bench, as "What could go wrong" has it, 1 and 2.

What is otherwise than the plan had it:

- `md.HstAdr - 1` of `enter.c`, which P2a listed with `info.c`'s, is
  no place: it is put into a word of 16 bits where it stands, `u16
  entry`.
- The test reads the 16 files of tables as well as the files of code:
  a constant of a table is worked out by each compiler for itself.
  They have no place.
- `arith.c` and `dos.c` are not read, nor `dos32.c`: none is compiled
  for both ints. `resident.c` is, while it is of both builds.

What the pieces behind this one should know:

- **Every piece.** A line that is changed and is in the record is
  another place to the test, which then fails for it twice, as new and
  as gone. Read it again and run `--record`; the notes of `CLAUDE.md`
  say when it is a cast and when a reason. A place has no number of a
  line, so lines added above move nothing.
- **D2 and M1.** `start.c` has two places of the environment,
  `env[at + i]` in `MnPth0` and in `IniData`, recorded as within 32K,
  which is what DOS holds an environment to. S1 has it come through
  `ProgramEnvironment` of `sys.h` and left the two lines as they were;
  what `dos32.c` answers there has to end within 64K of where it
  begins, as DOS's does.
- **S3.** `Alloc0` of `resident.c` is in the record for the paragraphs
  that the program keeps. In `dos.c` it is read no more, and `--record`
  takes it out.
- **M1 and the convergence.** What the finder cannot see is at its
  top. Above all: a variable of 16 bits that holds a wrapped number,
  FFFEh for two back, and is added to a pointer as it is, `text +
  shift`, is no arithmetic at the place and is named nowhere. The
  places where the finder did find the sum, kind `index`, are all sums
  of small numbers. C1 met the first such, the step of `T71_00`, and
  taught the finder the half of it that can be seen: a number that is
  written below 0 and kept in 16 bits without a sign is a place, kind
  `minus`. See "C1, the editor".
- **Step 2.** A cast of this piece marks a number that can wrap round
  on DOS or cannot be seen not to: when the variables are made wider
  each is a place to decide, and the two commits of the piece that
  change the code list them. The record's reasons hold for
  numbers of 16 bits: a variable made wider makes a place again where
  a reason said "a byte".
- **Whoever changes the finder.** A line of `harness/int16check.c` is
  worked out by hand before the finder is asked. Five of those written
  for it were wrong and the finder right; the one that teaches most is
  that 65,530 written in decimal is a long on DOS, so `x + 65530` is
  added in 32 bits there as well, and `x + 0xFFFA` is not.

### What could go wrong

1. An offset that has wrapped round, added to a pointer. Nothing finds
   it by reading. On the bench it is a read or a write 64K off, which
   nothing catches: garbage on a screen, or a program that stops. The
   debug build's checks catch what goes through the macros. The pieces
   of the convergence will meet these; how many there are is what is
   least known here. C1 met one, a step of -160 in a word of 16 bits
   added to a pointer into the screen buffer, which was nine of the
   fourteen scenarios that failed: "C1, the editor".
2. A far pointer stepped past its segment's end, and what lies behind
   an object: the same.
3. The critical error inside the extender. The trial saw the handler
   called and its answer taken. It did not try Retry, the phantom
   drive, or a box drawn from inside, which waits for keys, calls the
   BIOS and the mouse and makes the idle call, each through the
   extender again, on a stack of the extender's whose length is not
   known. D2 tries it with a toy before anything rests on it.
4. The time of day. A call of the system costs 175 to 280 instructions
   more, and an idle round 1,400 more for the block moves, where a
   round of the toy was 1,500. "Scenarios" has a C tree take up to a
   sixth more than the original to a step, and a scenario that shows
   the time done by fifty seconds past. If the 32-bit program takes
   more, the driver can read the two variables less often; if that
   does not do, the rule is the user's.
5. The goldens when the driver changes: H2. None changed, and the
   original goes through the driver by the instructions that it went
   by: see "Piece H2, the driver".
6. COMMAND.COM's interrupt: one try in the trial, which came back and
   ran nothing, as the 16-bit program finds it on the bench. It cannot
   be seen to run a command there in either build.
7. Memory that the program reads before it has written it: zeros in
   the 640 KB where DOS's block has what the last program left.
8. The stubs of QEMU. CauseWay does not run in it for one fault of its
   emulation; flat code and DOS/32A have met none so far, through the
   toys.
9. The eight stacks of 512 bytes that DOS/32A has for real mode: the
   driver's work in an idle call used 82 bytes of one in the trial,
   and uses 84 as it is now, with a stack written out (`python3
   harness/drivercheck.py --below`). A critical error has calls three
   deep.
10. The functions of `dos32.c` that no scenario reaches, when what ran
    of the C is asked: each is listed with its reason, as elsewhere.
11. Two pieces of P touching one file: they own lines of different
    files by the table, and P4 single lines of all; it rebases last.
12. The megabyte forms addresses that are in no object, 64K in front
    of the main data, which the language does not promise and both
    compilers here take. It is gone, with step 2, before clang builds
    the program.
13. Two far pointers held against one another. In 16 bits which of two
    is the greater, and how far they are apart, goes by their offsets
    alone, whatever their segments; in a flat memory it goes by the
    addresses. Two pointers of one segment are the same in both, and
    most are that. A place that counts on the other, a pointer of one
    segment compared with one of another or taken from it, shows
    nothing to either compiler, and nobody has looked for one: the
    pieces that run the 32-bit program first, M1 and the convergence,
    look where a list, the viewer's text or a copy goes wrong at the
    end of a segment or between two.
14. The tables that are written to. `OFFSET` counts from 32K below
    `Title_S` and holds for what lies within 32K of it. The 274 bytes
    of the tables that the program writes are no constants to
    `wcc386`, which puts them with what has a value; linked behind the
    main data, where the order of "The build" has what has a value,
    they are more than 64K from where the offsets begin, and `OFFSET`
    of one wraps round with no word said. They have to be linked with
    the constants, and the test of the layout, of H3 or B2, has to
    hold every object of a file of tables, the written ones among
    them, to lie within 32K of `Title_S`, and `Title_S` no more than
    32K in front of the main data. B2 did both: what has a value is
    one class with the constants in the linker's order, and
    `portable_c32-layout` holds the two.
15. A second copy of the program. It is loaded behind the first, and
    the driver of the tests reads the first copy's `AutoTim`, `Clock`
    and stack while the second idles: "Piece H3, the scenarios' test",
    which met it, has the six scenarios.

### The estimate

20 pieces that can be named, and three to six of the convergence.

| Kind | Pieces | What it is like |
|---|---|---|
| Mechanical, under the 16-bit build, the file's bytes the same | P1, P2a, P2b | A file moved to the functions of `sys.h`: the 348 calls went over in 25 commits between 20:41 and 22:33 on 12 October, by agents side by side |
| Under the 16-bit build, the code changed and the behaviour kept | P3, P4, S1, S2, S3 | `resident.c` moved to `sys.h`, the last and the hardest of those 25; S one behind the other |
| New code against the bench, with toys | D1, D2 | The trial itself |
| The build and the harness | B1, B2, H1 to H4 | Piece 0a of the setup program, which was 800 lines where 200 were thought. The trial's own sizes add up to three days of one person |
| The first run and the convergence | M1, C1 to Cn | Not known: the plan has not run the program at all. It is the kind of work that "What batches have lost time to" is the record of |
| The end | W, Z1, Z2 | Small |

The setup program was eight pieces, whose commits are from 23:02 on
the 12th to 03:44 on the 13th. This is two and a half to three times
as many, with one chain of six, so not less than a day of agents side
by side before the first run; and what follows the first run is the
part that the plan cannot count.

### Piece D1, `dos32.c` straight through: done, and what a later piece should know

`src/portable_c/dos32.c` has the 71 functions of `sys.h` that go
through the extender as they are, the 13 others as functions that do
nothing and say so, and the entry, `Start`. The toy that judges it is
`harness/trial32/sys.c`, one source built both ways by
`harness/trial32/build.sh sys 16` and `sys 32`: with the `dos.obj`,
`stack.obj` and `arith.obj` that the project's build made for the
program, and with `dos32.c` and `stack.c` compiled by `wcc386` and
DOS/32A for the stub. `python3 harness/trial32/both.py` builds both,
runs each on a bench and holds the screens one against the other; it
takes ten seconds, ends with 1 for a line that differs and should not,
and was run for both countries. Nothing of it is built or tested by
the project's build.

What the toy asks: every one of the 71, each also where it fails, a
handle that is none, a file and a directory that are not there, a
drive that there is not, a count of none, the low byte of an argument
with something above it; 30,000 bytes written and read at one call; a
search with other calls between two finds, with its record kept aside
and put back, and with the record moved; the keys of ten steps as
`KeyWaiting`, `KeyRead` and `KeysHeld` have them, and a button of the
mouse. In 32 bits it also looks, before everything that it writes,
whether ES is still on the data and the direction flag clear: they
were, behind every call.

What it showed, and what was done:

| What | 16 bits, `dos.c` | 32 bits, as first written | Now |
|---|---|---|---|
| The carry flag set before a call that DOS leaves it at: `DriveSpace(3, 1)`, AH 36h | Comes back set: failed, with what 59h answers, 0 | Came back clear: the extender does not carry the program's flag through DOS. Not failed, AX 1 | `Ask` of `dos32.c` takes the flag for set afterwards if it was to be set before. The same in both. It sets no flag for DOS at all. `FcbDelete`, 13h, goes the same way when D2 makes it |
| `failed` of a function that asks bare, where DOS reports an error | FFFFh: what SBB makes of the flag | 1 | FFFFh |
| AX where the call did not fail, of 4301h, 47h, 4Eh and 4Fh: `FileSetAttributes`, `DirCurrent`, `DirCurrentBare`, `FindFirst`, `FindNext` | What DOS left: CX as it went for 4301h (1, 0), 0100h for 47h, 0 for 4Eh and 4Fh | The AX that the call was made with: 4301h, 4700h, 4E00h, 4F00h. DOS/32A translates these four and gives back its caller's EAX | Those five are made in real mode, by DPMI 0300h, and `FindRecord` with them, with the name and the record of the search in a block below the megabyte: what DOS left comes back. The same in both |

Everything else is the same, line for line. So of the services that
the trial had not run, these answer through DOS/32A as they do
without: INT 21h 0Eh, 3300h, 3301h, 4400h, 440Fh, 5701h and 40h with a
count of 0, which cuts the file off; INT 10h 00h, also with the bit
that keeps the screen, 06h, 07h, 0Eh, 1112h and 12h with 20h; INT 16h
00h and 01h; INT 33h 8 and 21h. A search goes on with its record put
back or moved, as on DOS: the extender copies the record down before
4Fh. The idle call, the keys and the mouse turned every page of both
toys alike, shift states included.

The third is other than the plan had it, which counted the search,
the current directory and the attributes among what goes straight
through. `sys.h` has `value` be what DOS left in AX also where nothing
failed, since some callers go on with it: `DirFunc` of `dirfunc.c`
hands the high byte of what a search answered on to the routine that
it calls for each file. No call that the extender translates brings
that back, so the five are made in real mode, and `FindRecord` with
them, as the seven of D2 are to be: 65 functions go straight through,
6 through real mode are made, 7 through real mode and 6 that are
other are not. A commit of its own
has it, behind the one that has them straight through, so that it can
be taken back alone should what DOS left in AX at those four calls be
shown to reach nothing.

What that brought, which D2 builds on:

- `Start` asks for the block when the program starts, DPMI 0100h: 192
  bytes now, 128 for a name and 64 for the record of a search (`LOW_`
  of `dos32.c`). D2 makes it longer for what its seven need. A program
  that does not get it ends with 8 and says nothing: whoever makes the
  entry whole has it say so.
- `struct DosRegs` has `low`: `SetLow` in place of `Set` makes a call
  one of real mode, with DS and ES on the block and the registers
  places in it, and `Call`, `Tell` and `Ask` do the rest, the program's
  errors included. `Extender` is INT 31h.
- The record of a search is the program's and DOS's is in the block:
  the 43 bytes go down before 4Fh and up behind 4Eh and 4Fh, failed or
  not. `FindRecord` sets DOS's own, in real mode, and remembers the
  program's. The extender's own record is not used any more.
- A call made in real mode could have the carry flag set for DOS, in
  the registers that DPMI 0300h takes: `FcbDelete` may want that. As
  it is, `Ask` makes up for the flag whichever way a call is made.
- What the two more calls cost was not measured: DPMI 0300h was 380
  instructions more than a call made directly in the trial, and the
  copies are 43 bytes each way for a file found.

What the toy cannot tell: AX of the other translated calls where they
do not fail is the same in both on the bench, 3900h, 3A00h, 3B00h,
4100h, 4300h, 5600h, which is also what the call was made with. So
whether DOS/32A brings back DOS's AX there or its caller's does not
show under FreeDOS, which leaves AX alone at those; under a DOS that
does not, the two programs would differ in what `value` is.

Decided on the way, for whoever goes on:

- The entry is `Start` (`option start=Start_`): ES on the data, the
  direction flag clear, then `Main`. It does not ask for the version of
  DOS, give memory back or set how DOS looks for memory, which `Alloc0`
  does for 16 bits: S3 says where those go.
- A register that a call does not read is 0 in `dos32.c`, where `dos.c`
  hands DOS what the stack happened to hold: with a translated call the
  extender reads all 32 bits of a register that holds an address.
- `Screen` of the main data holds in 32 bits what it holds in 16, the
  segment of real mode above the offset; `ScreenPut` and `ScreenGet`
  make the address of it. Nothing but the system's functions reads it.
- `Ticks` keeps the machine from interrupting and lets it again, as
  `dos.c` does, though a number of 32 bits is read at once: the
  original lets the machine interrupt there, and something may count
  on it.
- The 13 that are not made call nothing: a call that the extender does
  not translate has DOS write at whatever the registers point to in
  real mode. `FileTrueName`, `FcbDelete`, `MemGive`, `ProgramRun` and
  `ProgramRunBare` answer failed with the error 1, `MemTake` and
  `MemResize` failed with 8 and no memory, `FcbParse` FFh, `CaseMap`
  the letter, `ProgramPrefix` 0, and the rest nothing.
- `Vector`, `SetVector` and `IntOn` of `dos.c` are no part of `sys.h`
  and are not in `dos32.c`: S2 and S3 change them first.

For D2:

- The last page of the toy asks ten of the 13 already, and
  `both.py` leaves that page out of what it fails for: it is known by
  its first line, `NOT_MADE`. Its table `EXPECTED`, for a line that
  is known to differ elsewhere, is empty. What `dos.c` answers there is the first
  column of `both.py --show`. `ProgramRun`, `ProgramRunBare` and
  `ProgramPrefix` are not asked: S1 changes what they take.
- `MemTake(FFFFh)` answers with how much there is, which follows the
  length of the program: the toy writes only that it is between 256
  and 640 KB. The 16-bit toy makes its block as long as it is first,
  as `Alloc0` does, in its own entry, and asks for its PSP itself, not
  through `sys.h`.
- On DOS in 16 bits the main data leaves 2.9 KB of the segment that DS
  is on, so the toy has its texts in the code segment (`TEXT` and
  `SAY` of `sys.c`) and its buffers in the main data, and takes the
  address of no local variable. A page holds 25 rows: a `+` in the
  last corner says that more was written than fits, and then a page
  wants dividing.
- A step of the script is what turns a page, and a page that is added
  wants a step in `STEPS` of `both.py`.

For B1, B2 and the harness:

- `CMakeLists.txt` takes `dos32.c` out of the files of the 16-bit
  program, which are all of `src/portable_c/*.c`; `tools/calls.py` and
  `tools/figures.py` leave it out too (`NOT_BUILT`). The host check
  reads it and has its row, and the README's count of `FAR` has its 31.
- `build.sh` has the lines that compile and link for 32 bits, with the
  plan's options. `-i` with the compiler's own headers is there for
  `stddef.h`, until `vc.h` no longer reads it for `wcc386`.
  `stack.c` compiles for 32 bits as it is, into a segment of the class
  STACK that the linker takes: 4,096 bytes, in the file.
- `VC.EXE` of the 16-bit build is the same bytes as before the piece.

### S1, the PSP and a program run: done, and what a later piece should know

What was 16 bits in kind in the header for a program run and for the
PSP is gone from it, and `dos.c` has it. `sys.h` had 86 functions
with this piece, where it had 84. It has 85 since 13 October 2026:
`MemEnd()`, which the piece made for the word of the PSP at 02h, the
segment where the memory ends, went with the Info panel's two lines on
the memory, the one thing that asked it (`DEVIATIONS.md`, 11): out of
`sys.h`, `dos.c`, `dos32.c` and the toy, with the line of `IniData`
that called it, `md.Memorys = MemEnd()`. The functions that are left:

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `ProgramRun(name, tail)`, `ProgramRunBare(name, tail)` | A program run, with its command tail as DOS has one: a count, the characters, a carriage return. They took DOS's block of three far addresses, which three files made with `MAIN_SEG` | `Run` fills in the block, `EPB` of the main data, and names it to DOS at ES:BX | Not made: they fail with the error 1, as before. D2's |
| `ProgramTail()` | What this program was told behind its name, as far as a carriage return | The PSP at 81h | Not made: a carriage return. D2's |
| `ProgramEnvironment()` | The environment, and behind its end the path of the program's own file | Offset 0 of the segment that the PSP has at 2Ch | Not made: 71 zero bytes. D2's |
| `ProgramPrefix()` | The segment of the PSP, as DOS says | In the last part, with what is of DOS alone, and declared in `vc.h` | None |

The places, each as it was and as it is. That is the six of the block
and six of the PSP that the row of the piece had, but for the one of
`MemEnd`, in `IniData`, which is gone:

| File | Was | Is |
|---|---|---|
| `start.c`, `IniData` | `md.EPB.EnvSeg = 0`, and `FCB1` and `FCB2` on `MAIN_SEG` with 5Ch and 6Ch | Gone: `Run` of `dos.c` |
| `resident.c`, `Exec02` | `md.EPB.CmdLine = (u32)MAIN_SEG << 16 \| (u16)before`, and `ProgramRunBare(name, &md.EPB)` | `ProgramRunBare(name, before)` |
| `view.c`, `Aexec0` | `md.EPB.CmdLine = ...` on `md.TmpBuf + 100`, and `ProgramRun(md.TmpBuf, &md.EPB)` | `ProgramRun(md.TmpBuf, md.TmpBuf + 100)` |
| `resident.c`, `Exec02` | `DOS_AT(PSP_ENV, md.ComSpec)` | `ProgramEnvironment() + md.ComSpec` |
| `start.c`, `Init00` and `IniData` | `DOS_AT(PSP_WORD(PSP_ENVIRON), 0)`, twice | `ProgramEnvironment()` |
| `main.c`, `Init40` | `FndLet(DOS_AT(PspSeg, 0x81))` | `FndLet(ProgramTail())` |

What decided: the whole suite, nine tests of nine, with `VC.EXE` 62
bytes longer, 197,152; `python3 tools/calls.py` counts none; the host
check's record is written anew, with `start.c` down to one place, a
place of the main data, which is S3's; `python3
harness/trial32/both.py` passes, and `whole.py` links.

What DOS is handed at 4B00h is what it was handed, read in the listing
of `dos.c`: AX, the name at DS:DX, ES:BX on `EPB` of the main data, and
in the block no environment, the tail by the main data's segment and
its offset, and the two file control blocks at 5Ch and 6Ch of the main
data. CX, SI and DI are as they come, which entry 6 of `DEVIATIONS.md`
has for both callers already; no row was added. No call of DOS was
added, taken away or moved: the three new functions read the PSP and
ask nothing, and they read it where the lines that they replace read
it, the segment of the environment out of the PSP at every call.

The block is filled in at every run, all four of its fields, where
`IniData` filled in three of them once and the callers the tail.
Between the start of the program and its first run of another the 14
bytes of `EPB` are therefore zeros where they held three of the four.
Nothing reads them: `EPB` lies behind what the setup file is an image
of, and the one other line that names it, `EPB_WAS` of `resident.c`,
takes its address, for what the original has in BX where it writes a
text.

What is otherwise than the plan had it:

- `PspSeg` and `ProgramPrefix` have left `sys.h` and three files, not
  the common code. `resident.c` goes by both still, in routines that
  are S3's to move: `Alloc0` asks for the PSP, makes the program's
  block shorter by it and leaves a DOS before 3 through it; `TakeEnd`
  and `Exit00` write where DOS goes on, `PSP_EXIT`, at 0Ah; `Exit02`
  leaves the segment in ES. So `PspSeg` is a variable of `dos.c` that
  `Alloc0` sets, and `vc.h` declares it and `ProgramPrefix` with what
  is of DOS alone, beside `Vector`, `SetVector` and `IntOn`. When S3
  has those routines in `dos.c`, both are static there.
- `info.c` had no place: it showed `Memorys` of the main data, which
  `IniData` set, and shows nothing of the memory since.
- `struct EPBs` stays in `vc.h`: the main data has it, and its layout
  is held.
- Two tools of the trial went by the header as it was and were mended:
  `harness/trial32/whole.py`, which links the build's objects of 32
  bits with `dos32.c` since B1 and has what is of DOS alone in a list
  of its own, `OUTSIDE`, where `ProgramPrefix` and `PspSeg` are now;
  and the toy, `harness/trial32/sys.c`, whose entry for 16 bits now
  sets `PspSeg`, and whose last page asks the three new functions.

For S2 and S3:

- `DOS_AT` has five places left. `FatErr` of `main.c` makes the address
  of the device's header of what DOS has in BP:SI: S2's. `PSP_EXIT`
  three times and the far call into the PSP in `Alloc0`, in
  `resident.c`: S3's. With the last of them `DOS_AT` goes from `vc.h`.
- `dos.c` has `POINTER(segment, offset)` for a far pointer of two
  numbers and `PSP_WORD(offset)` for a word of the PSP; `PSP_EXIT`
  wants the same for a number of 32 bits, written to.
- A function that is added to `sys.h`, or that leaves it, is in six
  places besides `dos.c`: `dos32.c`, made or as one that does nothing,
  with the counts at its top and in front of its second part, 71 made
  and 14 not; the README's table of the groups, and its 85; the table
  of `CLAUDE.md`, "Moving a file to the system's functions"; `NAMED` of
  `harness/reads.py`, if it takes a name; and the toy's last
  page. What is of DOS alone and no function of `sys.h` was a seventh,
  `OUTSIDE` of `whole.py`, until the build linked the program of 32
  bits: such a name stops the build now.
- The toy for 16 bits is linked of `dos.obj` as the build made it, the
  stack and the arithmetic, and nothing else of the program
  (`harness/trial32/build.sh`). Whatever `dos.c` comes to call of the
  program, `FtErr1` for S2, `Main` and what leaving does for S3, the
  toy has to have under the same name, or its link fails. `both.py`
  says so in ten seconds.
- Behind `ProgramRunBare`, `Exec02` calls `SetBrk`, failed or not, which
  asks DOS whether it looks for Ctrl-C, tells it not to and takes
  INT 23h; `Aexec0` of `view.c` does none of that behind `ProgramRun`.
  That is the original's, and S2 keeps it where it changes `SetBrk`.
- The scenarios of both trees took 15 minutes with other agents
  testing.

For D2:

- `Run` of `dos.c` is what a program run is told, in ten lines, and
  the comment in front of it says why the file control blocks are what
  they are. In 32 bits the name, the tail and the block go into the
  block below the megabyte: the tail is `tail[0] + 2` bytes; the two
  file control blocks are the bytes of the main data from 5Ch and from
  6Ch, colours of the setup, which DOS copies into the PSP of the
  program that it runs and by whose first bytes it says in AX whether
  each names a drive that there is. How many bytes DOS takes of each
  was not looked up: 16 is an unopened one.
- The three that read the PSP want its address, which is below the
  megabyte. In 16 bits the entry asks DOS for it, AH 51h, as the first
  thing that it asks; `Start` of `dos32.c` is entered with ES on it, a
  selector. S3 says what the entry asks and in which order.
- `ProgramEnvironment` answers with a pointer that is read on behind
  the end of the environment: `MnPth0` of `start.c` copies the path of
  the program's own file from three bytes behind the last zero, as 68
  bytes whatever the path. The environment's own segment, as it lies
  below the megabyte, has that; a copy would have to have it too. And
  `Exec02` adds `ComSpec` of the main data to the pointer, an offset
  in the environment that `IniData` found.
- The toy starts with no tail, so `ProgramTail` prints as an empty
  one in both already; a tail wants `run.py` to start the toy with
  one. `ProgramRun` and `ProgramRunBare` are not asked: they want a
  program on the toy's disk to run and something that shows that it
  ran with the tail.

### S2, what DOS enters: done, and what a later piece should know

The three interrupts that the program takes are taken through `sys.h`,
and where they arrive is in `dos.c`. `sys.h` has 93 functions, where it
had 85, and a group of its own for them, "What the system asks the
program, and the keys that stop one":

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `DeviceErrorNote()` | Who is asked about a device that fails is noted | AX 3524h, into `ErrVct` of the main data | Not made: nothing |
| `DeviceErrorTake(ask)` | The program is asked from now on, by the function `ask` | `ask` kept, and AX 2524h with the address of `FatErr` | Not made: nothing, and `ask` is never called |
| `DeviceErrorGive()` | Whoever was noted is asked again | AX 2524h with `ErrVct` | Not made |
| `CtrlBreakNote()`, `CtrlBreakTake()`, `CtrlBreakGive()` | The same for Ctrl-Break, which is taken so that nothing happens | AX 351Bh into `BrkVct`, 251Bh with the address of `CtrlC`, 251Bh with `BrkVct` | Not made |
| `CtrlCTake()` | Ctrl-C taken so that nothing happens; DOS gives it back itself | AX 2523h with the address of `CtrlC` | Not made |
| `IntOn()` | The machine let interrupt the program again. It was of `vc.h`, with what is of DOS alone | `_enable`, as before | Made: `_enable` |

What the program is asked by is a type of `sys.h`, `DeviceErrorFn`:

    u16 ask(u16 how, u16 error, const u8 FAR *device)

`how` is DOS's word for the error, AX as INT 24h has it, with the drive
in the low byte, 1000h where DOS can try again and 8000h where no
sector failed; `error` is DOS's number, DI whole, as the original reads
it; `device` is 0 for a drive and else the name of the device, its 8
characters in the header that DOS names. It answers `DEVICE_RETRY` or
`DEVICE_FAIL`, DOS's 1 and 3. `FtErr1` of `main.c` is that function,
static there, and `HookVct` hands it over. No address of real mode and
no register crosses the header: the entry, `FatErr` of `dos.c`, reads
BP:SI and the two bytes of the header that say what the device is.

The seven places, each as it was and as it is:

| Routine | Was | Is |
|---|---|---|
| `HookVct`, `main.c` | `md.ErrVct = Vector(INT_ERROR)`, `md.BrkVct = Vector(INT_BREAK)`, `SetVector(INT_ERROR, FatErr)`, `SetVector(INT_BREAK, CtrlC)` | `DeviceErrorNote()`, `CtrlBreakNote()`, `DeviceErrorTake(FtErr1)`, `CtrlBreakTake()` |
| `RestVct`, `main.c` | `SetVector(INT_ERROR, md.ErrVct)`, then `RstVc1` | `DeviceErrorGive()`, then `RstVc1` |
| `RstVc1`, `main.c` | `SetVector(INT_BREAK, md.BrkVct)` | `CtrlBreakGive()` |
| `SetBrk`, `resident.c` | `SetVector(INT_CTRL_C, CtrlC)` behind the two calls for the check | `CtrlCTake()` behind them |

And the three entries: `FatErr` was an interrupt function of `main.c`
and is one of `dos.c`; `CtrlC` was one of `resident.c` and is one of
`dos.c`, static; and the header of the device, the one place of
`DOS_AT` that was S2's, is read there. `Vector` and `SetVector` are
static in `dos.c` and gone from `vc.h`.

What decided: the whole suite, with `VC.EXE` 26 bytes longer, 197,052
behind the Info panel's piece, which landed in front of this one; no
golden changed; `python3 tools/calls.py`
counts none; the host check's record is written anew, with `main.c`
down to 8 numbers and no interrupt function, `resident.c` an interrupt
function and a function's address fewer, `vc.h` without its interrupt
function, and `dos.c` with the two entries and the three addresses of
them; `python3 harness/trial32/both.py` passes, and `whole.py` links.

What DOS is asked and in which order is what it was: the four calls of
`HookVct` are still four calls in its own lines, both read and then
both set, and each function is the one call that the line was, through
the same `Vector` and `SetVector` with the same registers. No row was
added to entry 6 of `DEVIATIONS.md`, and no deviation is declared: no
call was added, taken away or moved, and none hands DOS anything else.
One thing is read earlier, which no call and no screen shows: the entry
looks at the header of the device before it asks the program, where the
original looks at it only when it shows a box, behind `ErrDat` and
`BlocEr`. It is a byte of DOS's memory read, at an address that DOS has
just handed over.

The stack: every scenario leaves to the byte what it left before, 159
figures of 159, 3,502 at the least, in `copy-dirs`; `not-ready` and
`phantom` 3,662, `device` 3,504. That was measured on the tree as it
was before the Info panel's piece. The call through the pointer costs
what the call of a static function of the same file cost, a far return
address, and what `ask` takes goes in registers.

What is otherwise than the plan had it:

- The program's function is handed over, and not called by its name.
  The plan had `dos.c` call `FtErr1` of `main.c`; "An implementation of
  the layer for tests" asks for call backs "that the layer names and
  the program hands it", and that costs four bytes of data and a call
  through them. So `dos.c` names nothing of the program, `FtErr1`
  stays static, and the toy needs no function of that name: S1's
  warning for this piece is moot, and it stands for S3, which has
  `dos.c` call `Main`.
- Ctrl-Break and Ctrl-C have no call back: the program does nothing
  when either arrives, and `CtrlC` of `dos.c` is the routine that only
  returns. A system that has something to say there gets a function
  handed over as the critical error has.
- `FatErr` is not static in `dos.c`, though only `dos.c` names it:
  `harness/ported.py` holds that the function of that name has run, in
  the batch of the critical error, and looks for it among the map's
  symbols that are not static. The first run of the suite failed for
  that alone, with every scenario passed. Both entries keep the names
  that the routines have in the assembly, since they are what those
  are.
- Who had the two is kept where it was, in `ErrVct` and `BrkVct` of the
  main data, which `dos.c` writes and reads now and nothing else does:
  the layout of the main data is held, as it was for `EPB` in S1.
- `SetBrk` stays in `resident.c` and is the program's: three functions
  of `sys.h` in the original's order, with nothing of DOS left in it.

`AskEs`, and what was found of it. It was not moved, and the entry's
move did not force it: `FtErr1` sets `md.ErrDat` as before, and `AskEs`
of `dos.c` clears it in front of a call and reads it behind. What
keeps that decision under the header is not the entry any more. It is
that a call given up is known by a byte of the program's: the answer
`DEVICE_FAIL` does not say it, since `FtErr1` answers that also where
it shows no box, with `BlocEr` set and `ErrDat` clear, and such a call
is not made the error 53h but asked about, AH 59h. So `dos.c` cannot
go by what the function answered without a third answer for "fail, and
I did not ask". To have the decision above the header, the functions
that are asked with the program's errors would be the bare ones with a
routine of the program around each, which clears `ErrDat`, looks at it
and asks for the extended error: one function more in `sys.h`, for AH
59h, and some forty to go through. That is step 3's to weigh.

`IntOn`, which is not of this piece's row and is where the critical
error still shows in the common code: DOS enters `FatErr` with the
machine kept from interrupting, the compiler's interrupt function
leaves it so, and the box that `FtErr1` shows gets its keys because
`Window`, `MovScr` and `MinBx3` call `IntOn`. Those three are of the
common code, which a program of 32 bits is linked of, and the fourth
caller is the entry, `Alloc0`, for the direction flag. So `IntOn` is a
function of `sys.h` now, in the group of the critical error, in a
commit of its own: `dos.c` has it as it was, `dos32.c` has it made, and
`whole.py` has one name fewer outside `sys.h`. The file is as long as
it was and not the same bytes, the function having moved within
`dos.c`, and the whole suite was run for it.

The other way was not taken, and is the user's to decide if it is
wanted: `FatErr` could let the machine interrupt before it asks the
program, and the three calls could go from the common code. Then the
machine can interrupt from the entry on, where in the original it
cannot before the box is first put on the screen. No call of DOS and
no screen would show that; it is the program doing otherwise all the
same.

For S3:

- `dos.c` has interrupt functions now, with `struct Saved` of `vc.h`
  for the registers, and `HANDLER(function)` for the address of one as
  DOS takes it, which is what `ROUTINE` of `resident.c` is. When
  `Exit01` and `Exit05` are there, `struct Saved` can leave `vc.h`.
- A function of a batch's `functions` in `harness/ported.py` that
  becomes static is not found: see `FatErr` above. Look at the batches
  for the names that move, before the suite is run.
- A macro of `dos.c` may not have the name of one of `vc.h`: `ENTRY` is
  a file of a panel by its address there, and the build stopped for
  it, W140.
- The program gives Ctrl-Break back when it leaves, `Exit` of `main.c`
  by `RstVc1`, and not the critical error or Ctrl-C, which DOS puts
  back from the PSP. That is the original's, and stays the program's
  row. The plan has the extender write a warning for a vector that is
  still taken when the program ends: giving those two back is then
  `dos32.c`'s own business where it ends the program, and no call of
  the program's.
- `DOS_AT` has four places left, all in `resident.c`: `PSP_EXIT` three
  times and the far call into the PSP in `Alloc0`.
- `Alloc0` calls `IntOn` first, for the direction flag, which is a
  function of `sys.h` now and says nothing of that flag there: in
  `dos.c` the entry can say `_enable()` itself. What is left outside
  `sys.h` then is `ProgramPrefix` and `PspSeg`, and `whole.py`'s list
  has those two and `__GETDS`.
- The four functions of `sys.h` that the entry of the program asks
  before `Main`, and what leaving does, are untouched by this piece.

For D2:

- The seven are stubs in `dos32.c`, at the end of its second part, 21
  that are not made where it had 14, and 72 that are made, with
  `IntOn`; the comment in front of them says
  what the call has to be. What the plan has for them stands: DPMI
  0204h and 0205h for the three, with who had the first two kept in
  variables of `dos32.c`, six bytes each, and not in `ErrVct` and
  `BrkVct`, which are four.
- The routine that the critical error arrives at has to call
  `ask(how, error, device)` with AX and DI as DOS has them and
  `device` a pointer: the header is at BP:SI of real mode, 16 times BP
  and SI in a flat memory, the byte at 5 says with 80h that it is no
  drive, and the name is at 10. AL of the answer goes back to DOS; the
  original has AH 0 there, and `dos.c` hands back the whole word.
- It has to go over to the program's stack first, and that stack is
  in use: DOS was called from somewhere in the program, or a program
  that this one has run called it (`Aexec0` of `view.c` runs the
  viewer and the editor that a user has set up with the critical error
  still the program's). So the routine wants the stack pointer as it
  was where DOS was called, and goes on below it: `Dos` of `dos32.c`
  and the call in real mode are where that is known.
- What `ask` does inside: it draws a box, `Dialog`, which waits for
  keys, calls the BIOS and the mouse, reads the timer and makes the
  idle call, INT 28h, each through the extender again; and
  `IntOn`. "What could go wrong", 3, is that.
- The toy calls the seven in the program's order on its last page,
  with a function that answers `DEVICE_FAIL`, and prints a line; in
  16 bits that takes the three and gives two back on the bench.
  Nothing makes a device fail there. A file made on the diskette that
  the bench boots from does, by the trial; in 32 bits that waits for
  the functions, since DOS's own question would wait for a key.

### S3, the entry, the end, Quick execute: done, and what a later piece should know

What DOS enters of the program, and what the program asked of DOS
outside `sys.h`, is in `dos.c`, all of it: no file but `dos.c` and
`arith.c` has an interrupt function, `__loadds`, a naked function,
`_asm` or a register in a `#pragma aux`; no file but `dos.c` reads the
PSP; and the three files of the row are clean of casts in the host
check. `sys.h` has 98 functions, where it had 93, and every file of
the tree compiles for 32 bits. It was made a family at a time, a
commit each, with the whole suite at each, ten tests of ten: the
entry; the ends; Quick execute; the places of the main data. `VC.EXE`
is 197,070 bytes, 18 longer than it was; no golden changed; `python3
tools/calls.py` counts none; `python3 harness/trial32/both.py` passes
and `whole.py` links, of the build's objects alone.

The five functions, each with its family below:

| Function | Group of `sys.h` | In `dos32.c` |
|---|---|---|
| `MemOwn()` | Memory | Made: nothing to do |
| `ProgramEndOld()` | Other programs | Made: nothing to do |
| `ShellRun(command)` | Other programs | Not made: no command is run |
| `ProgramEndedTake(told)`, `ProgramEndedGive()` | What the system asks the program | Not made: nobody is told |

**The entry: done.** Where DOS enters the program is `Alloc0` of
`dos.c`, and it is what is of DOS there and no more: the direction
flag, DS, and the PSP asked for, AH 51h, the first thing that DOS is
asked. Then it calls `Main` of `main.c`. What the original's `Alloc0`
does from its label `Alloc3` on is the program's, a function of that
name in `resident.c` that `Main` calls first: the version of DOS, with
what the program says of one that is too old, the program's block made
as long as the program, with what it says where that fails, and
`SetStat`. It asks through `sys.h`, by two functions that are new, 95
where there were 93:

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `MemOwn()` | The block that the system loaded the program into, made as long as the program is. Answers as `MemResize` | `MemResize` of the PSP's segment, to where the stack ends | Made: nothing to do, and not failed |
| `ProgramEndOld()` | The program ended on a DOS before 3, which the original leaves by returning into its PSP | The far call to the PSP's first bytes, INT 20h | Made: nothing, and the program goes on to say that DOS is too old |

DOS is asked what it was asked, in the order that it was: 51h, 30h,
4Ah, 5801h, 5803h, each through the function that asked it before,
and `MemOwn` works out the paragraphs as `Alloc0` did. `VC.EXE` is 58
bytes longer, 197,110.

The stack: 157 of the 159 scenarios leave 12 bytes more than they did,
3,514 at the least, in `copy-dirs`. The frame of `Alloc0`, which lay
under everything, had the version, the end of the program and the
saved registers in it, 14 bytes; now it is the one word of DS that
`Alloc0` of `dos.c` pushes, and `Alloc3` has returned before anything
else is called. `mouse-find` leaves 16 more and `command` 2 fewer.
That is taken to be an interrupt of the machine, the timer's or the
keyboard's, which takes of the stack wherever the program is, meeting
the deepest call elsewhere in a file that is laid out otherwise: 14
bytes are what such an interrupt takes at the least. It was not looked
into further.

The toy is entered as the program is, at `Alloc0_` of `dos.obj` in 16
bits, and has no entry of its own any more; its `Main` asks `MemOwn`
first, as the program does, and its first page prints whether that
failed. `whole.py` starts its program at `Start_` of `dos32.c`.

**The ends: done.** Where DOS goes on when it has ended the program
is `dos.c`'s: `Exit01`, the four instructions that the PSP names, and
`Exit05`, the interrupt function that they jump to, with `struct
Saved`, which has left `vc.h`, the return address behind it, the flags
that an instruction leaves, and `Vect22`, where the PSP had DOS go on
before. The program takes its end by two functions of `sys.h`, 97
where there were 95, in the group of what the system asks the program:

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `ProgramEndedTake(told)` | From now on the program is told when the system has ended it by another's hand: `told` is called, a function that takes and answers nothing | `told` kept, the PSP's address at 0Ah kept in `Vect22`, and `Exit01` written there. No call of DOS | Not made: nothing, and `told` is never called |
| `ProgramEndedGive()` | Nobody is told any more: said before the program ends itself | `Vect22` written back to the PSP | Not made: nothing |

What the program does at its end is the program's, `Exit02` of
`resident.c`, which has DOS look for Ctrl-C as it did, the one call
of DOS that the original makes there with nothing to release: `TakeEnd`
hands it over, `Exit05` calls it through the pointer, and `Exit00`
calls it where the program leaves by itself, between
`ProgramEndedGive` and `ProgramEnd`. The order is the original's: the
PSP written back, 3301h, 4Ch.

What `Exit05` leaves for the program that started this one is what
`Exit02` of `resident.c` left: ES on the PSP, the flags of the segment
that the program goes on in compared with 0, every other register as
it came, and in DL what DOS was last told of Ctrl-C. For the last,
`SetBreakCheck` of `dos.c` keeps what it tells DOS, in a byte of its
own: the original has DL there because that call was the last, and
`dos.c` reads nothing of the program's for it. `ended` shows the
registers and passes as it did. No row of entry 6 of `DEVIATIONS.md`
is new: the two calls are made by the functions that made them.

Otherwise than it was, where nothing shows: `Exit00` gave the PSP its
address back and made the segment of `Vect22` 0, by which `Exit02`
knew whether to end the program or to return to DOS's caller; the two
are two functions now and nothing is asked. `ProgramEnd` itself gives
nothing back, so a program that ends by it without `ProgramEndedGive`
first, as the start does for an old DOS or for want of memory, before
`TakeEnd`, has taken nothing yet.

`PspSeg` is static in `dos.c` and `DOS_AT` is gone from `vc.h`: no
file but `dos.c` reads the PSP. With `Exit05` gone from it,
`resident.c` compiles for 32 bits, and the build said so: it is off
B1's list, which is empty, with `tools/refused.cmake` still there for
it. `whole.py` makes one object itself, for `__GETDS`, which `Exec2E`
still asks for. `VC.EXE` is 4 bytes shorter, 197,106. The stack: 157
figures of 159 as the entry left them, `command` 14 more, which is the
12 of the others after all, and `mouse-select` 2 fewer.

**Quick execute: done.** It is a function of `sys.h` that comes back,
as the plan's row has it, 98 functions where there were 97:

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `ShellRun(command)` | A command handed to the shell that the program was started from, which runs it itself; `command` is as a command tail is, a count, the characters and a carriage return. It answers nothing | `Int2E`, fourteen instructions of assembly: SS and SP kept in two words of the data, DS:SI on the command, INT 2Eh, DS found again by its segment, SS and SP put back | Not made: nothing, and no command is run |

The feature is whole: the setting, the command that changes the drive
first, `DosChg`, with the command kept at `BatPoin` meanwhile, and
`SetBrk` behind the interrupt, in the original's order. What has
changed is how the program goes on behind the interrupt, which comes
back with every register lost. The original sets its stack anew there,
to where it is empty, and jumps on; it can, since it never returns
from running commands but starts the program over at `Init11`. The
portable C did the same and so could not return either: `Int2E` set
the stack to its end and jumped to `Exec2E`, a `__loadds` function,
which ran what was left of the commands and went on in `Again` of
`main.c`, a second way into the program's loop. Now the stack is kept
and put back, `ShellRun` returns, and `Exec02` goes on at its label
`Exec07` as the original does behind the interrupt. `Exec2E` and
`Again` are gone, with two `aborts` and the argument by which `Exec02`
was told to go on behind a command.

This is the one place where the piece did otherwise than "moved as it
is", and it is the plan's row that says so. What decided: what is
asked of DOS and of the shell and in which order is the same, read
line for line against the assembly (`Exec02` to `Exec07` of `VC.ASM`);
`TakeMem` follows the commands in `Round` as it followed them in
`Again`; and a function that does not return could not be a function
of `sys.h` that `dos32.c` makes by a call which does return. The
assembly is six instructions more for it.

What INT 2Eh is handed: DS:SI on the command, as before, by a far
pointer taken apart, so DX is the segment too; entry 6 of
`DEVIATIONS.md` has a row for it, which it had not, with what the
original has in the registers that the interrupt does not take.

`quick` passes as it did, and the stack figures did not move: `quick`
3,760. It hands the shell a command three times, the second time
behind the command that changes the drive first, so both ways through
`Exec02` are run. No bench of the project can show a command run by
INT 2Eh, before this piece or after it: the shell there is FreeCOM,
which runs nothing by that interrupt and says nothing. So the scenario
shows that the program comes back and goes on, with the stack whole,
and not that a command runs; that the stack comes back right behind a
shell which does run one is from reading, as it was of the stack set
anew. The toy hands the shell `VER` on its last page and writes that
it came back, in both builds.

With this nothing but `dos.c` and `arith.c` has an interrupt function,
`__loadds`, `_asm`, a naked function or a register in a `#pragma aux`:
the host check's record has `resident.c` with 13 numbers, 4 pointers
and one `aborts`, which the last family takes down. `VC.EXE` is 10
bytes shorter, 197,096.

**The places of the main data: done.** The three files say
`MD_PLACE` and `MD_POINTER` where they cast, and are clean in the host
check but for an `aborts` in `resident.c` and in `main.c`, which wait
for nothing:

| File | Places | What |
|---|---|---|
| `resident.c` | 7 lines and a macro | `BatAdr` and `BatPoin`, the command that is run, kept as places and read as pointers. And `WAS`, which made of an address of the main data the one that the original has for it: it takes a place now, `WAS(md.BatAdr)`, and `EPB_WAS` is `WAS(MD_PLACE(&md.EPB))` |
| `resident.c` | 1 | `OutStr` returned where its text ends, a far pointer cut to a word, and nobody read that: it returns nothing |
| `main.c` | 6 | What the registers of the loop hold as numbers, `r->si` and `r->di`, and the buffer handed to `F24_01` |
| `main.c` | 2 | The text of a critical error, a far pointer into the tables handed to `Dialog` as its offset there: `OFFSET_IN_SEG(assembly, text)`, as P2b has it in `edit.c` |
| `main.c` | 1 | `root = (struct DIRENT *)MD_POINTER(TRELIST)` in `Init90`: P2a's place that gcc does not count |
| `start.c` | 1 | `md.HstAdr` |

One of them is no place of the main data: `r->di = MD_PLACE(IniSave) +
INI_LEN` in `Loop`. `IniSave` is a variable of `setup.c`, behind the
main data in the segment that DS is on, and the number is what a
register holds when the comparison of the setup is done, which goes
round the loop and which a command may go by. On DOS it is the offset
that it was. In a flat memory it is how far the variable is behind
`md`, cut to 16 bits, which means something only while the linker has
it within 64K: the comment at the line says so. Nothing follows it as
a pointer.

`main.c` and `start.c` compile to the code that they compiled to,
listing for listing. `resident.c` does not: `WAS` took `&md` away from
an address at run time, which is 0, and adds a constant to a place
now, and `OutStr` has lost its last instruction; `VC.EXE` is 26 bytes
shorter for the two, 197,070, and the whole suite decided.

P2a's finder, the tree read by gcc with the second body of `MD_PLACE`
a function, says of that what it says of the tree, in all files: no
constant that comes from `MD_PLACE` is made a pointer anywhere now.

The host check counts `dos.c`, `dos32.c` and `arith.c`, and of the
rest only what waits for nothing: `data_seg` in `main.c` and `stack.c`
and `aborts` in `main.c`, `resident.c` and `vc.h`.

**Not done, and why: the four `#pragma data_seg` under `__I86__`.**
The row has them so, and the code needs them where they are. The
stack's is what gives a flat program its segment of the class STACK,
without which the linker's header names no stack and the extender
does not get the program going (the trial, "Building one, with no C
library"); the toy of `dos32.c` is linked by it, `order ... clname
STACK`. The main data's gives it a class of its own, `MAIN`, in 32
bits too, which is the one handle that B2 has to say where `md` is
linked: in front of the 640 KB, and no more than 32K behind the
tables. And an `#ifdef` in a file of code is what the plan itself
rules out. `wcc386` takes the four lines as they are.

What is otherwise than the plan had it:

- Five functions are new in `sys.h`, where the plan had none: it had
  `Main` called by the entry and leaving be `ProgramEnd` "with what has
  to be done first". The entry does call `Main`. But what the program
  does first and last is in an order with DOS's calls that is
  behaviour, the version asked before the block is made shorter and
  Ctrl-C put back before the program ends, so those stay rows of the
  program, and each thing in them that only `dos.c` can do is a
  function: `MemOwn`, `ProgramEndOld`, `ProgramEndedGive`.
  `ProgramEnd` is as it was.
- Of `Exit00` to `Exit05`, the two that DOS enters went to `dos.c`,
  `Exit01` and `Exit05`. `Exit00`, `Exit02` and `TakeEnd` are the
  program's, in `resident.c`, and know nothing of DOS but the functions
  that they call.
- The program's function for its end is handed over, as S2 has
  `FtErr1`, and `dos.c` names of the program only `Main`. The toy
  defines that already, for `Start` of `dos32.c`.
- Quick execute comes back, which the row has and the brief had as
  "moves as it is": see its family above.
- The four `#pragma data_seg` are not under `__I86__`: see the last
  family above.
- `resident.c` came off B1's list with the second family and not the
  last: it was `Exit05` alone that `wcc386` refused. The compiler for
  32 bits takes a block of `_asm` with registers of 16 bits, `seg` and
  all, and `__loadds`, with no word said, so "it compiles" said nothing
  of `Int2E` and `Exec2E`, which were still there then.
- The stack figures moved once, by 12 bytes in every scenario, with
  the entry. Two scenarios, `command` and `mouse-select`, and once
  `mouse-find`, moved by 2, 6, 14 or 16 bytes otherwise, back and
  forth, at each of the four commits. That is taken to be an interrupt
  of the machine meeting the deepest call elsewhere as the file is
  laid out otherwise, and was not looked into further. So "the figures
  did not move" was held to 157 of the 159 at each commit and no more.

What is left of `resident.c`, and its name. Fourteen functions and 384
lines: `Alloc3`, with `SetStat` and `Alloc9`, what the program does
first; `Exec`, `Exec02` and `Exec10`, the commands, with `NotFind`,
`OutStr` and `OutStr0`, which write for them; `SetBrk`; and `Exit00`,
`Exit02`, `TakeEnd` and `AlocEr`, leaving. It asks DOS through `sys.h`
like any file, has no word of Open Watcom but one `aborts`, and
compiles for 32 bits. Nothing in it is resident, and nothing has been
since the program stays in the memory whole: the name is the C
translation's, and says what the file was. It was left, since three
tables of the harness know the file by it, for both trees:
`source="resident.c"` of three batches in `harness/ported.py`,
`REVIEWED` and its like in `harness/reads.py`, and a text of
`harness/datacheck.py`. What it is now would be `commands.c`, which is
taken, or `shell.c`. It keeps its name for now, by the session's word:
the rename is the piece R1 of the table, for after the first run.

For D2, what `Start` of `dos32.c` has to do before it can call `Main`.
`Main` asks at once, in this order, and each has to answer:

- `Alloc3`: `SystemVersion`, made. `MemOwn`, made. `MemSetSearch(0)`
  and `MemSetUpper(0)`, not made: they are passed to DOS as they are,
  by the plan, and take no address.
- `Init00` of `start.c`: `ProgramEnvironment`, and behind the
  environment's end the path of the program's own file, read as 68
  bytes, by which the setup file, the extension file and the help are
  found; then the current drive and its directory, the setup file
  read, the screen's mode, the banner by `ConsoleText`; then `SetBrk`,
  which is `BreakCheck`, `SetBreakCheck` and `CtrlCTake`; then
  `TakeEnd`, which is `ProgramEndedTake`.
- `Init11`: `MemTake(0xFFFF)`, which has to fail and say the largest
  block, and `MemTake` of that. With too few paragraphs the program
  says "Program too big to fit in memory" and leaves by `Exit00`.
- `IniData`, at the head of `Round`: `HookVct`, which is the four of
  the critical error and Ctrl-Break, the country and its capitals,
  `ProgramEnvironment` again, and the rest of the machine.

So before `Main`, `Start` has to have:

1. ES on the data and the direction flag clear, which it does.
2. The PSP's address, of which nothing is kept now: the extender
   enters `Start` with ES on the PSP, a selector, and `Settle` puts ES
   on the data as the first thing. The selector has to be read before
   that, and its base asked for; `ProgramTail` is that and 81h. Where
   the environment is under the extender, the word at 2Ch, a segment
   of real mode or a selector, was not looked at by the trial.
3. The block below the megabyte, which it asks for, with room for what
   the seven functions of real mode and `ShellRun` put there.
4. The 640 KB of the program's own, as `MemTake` is to hand them out:
   beginning on a paragraph of the megabyte, which is counted from 64K
   below `md`, so that `AT(segment, 0)` is where the block begins.
5. Nothing for `MemOwn`, and nothing taken: the three interrupts and
   the program's end are taken by the program, later, through the
   functions.

In 16 bits the first thing that DOS is asked is where the PSP is, AH
51h, and nothing is asked before `Main` besides. What `Start` asks of
the extender or of DOS before `Main` is its own, and behaviour that
the 16-bit program has not: the block below the megabyte is DOS's
memory, 48h in real mode, which a command run later finds taken.

For D2, what the ends have to give back. The program leaves in this
order: `Exit` of `main.c` gives Ctrl-Break back, `CtrlBreakGive`, and
puts the screen back; `Exit00` calls `ProgramEndedGive`,
`SetBreakCheck` with what `SetBrk` noted, and `ProgramEnd`. The
critical error and Ctrl-C it does not give back: DOS puts both back
from the PSP when a program of 16 bits ends. So:

- `ProgramEnd` of `dos32.c` gives back what the file has taken
  through the extender and the program has not given back, the
  critical error and Ctrl-C, and INT 22h where a program was run (the
  trial, "Building one"), or the extender writes its warning onto the
  last screen. It is also called with nothing taken, by `Alloc3` for a
  DOS that is too old and by `Alloc9`; and by `AlocEr`, through
  `Exit00`, where the start finds too little memory, with Ctrl-C and
  the program's end taken and the critical error and Ctrl-Break not
  yet.
- `ProgramEndedGive` gives back what `ProgramEndedTake` took, if that
  comes to take anything.
- `ProgramEndedTake(told)` is what is open for the user, 4 above: in
  16 bits DOS goes on in the program when it has ended it, the
  program's function has DOS look for Ctrl-C as before, and the
  program that started it goes on with ES on the ended program's PSP,
  DL what DOS was last told of Ctrl-C, and the flags of a segment
  compared with 0, which `ended` prints. Whether the extender lets
  anything of the program run then is to be tried before it is
  written; `Exit05` of `dos.c` is the whole of what there is to
  carry over.
- `ShellRun`: the comment at it in `dos32.c` has what the trial did.
  The command is 127 bytes at the most, a count, 123 characters and
  the carriage return, or the four bytes of `DosChg`; it is in the
  main data, above the megabyte, and has to be copied down. `quick`
  cannot tell a function that runs nothing from one that calls the
  interrupt, on the bench: the shell there runs nothing by it.

For B2:

- The program of 32 bits begins at `Start_` of `dos32.c`; the one of
  16 bits at `Alloc0_`, of `dos.c` now, under the name that the
  linker's command had.
- `whole.py` is the link, less its order and the stub's bit: 62
  objects of `build/portable_c32/`, none made beside them, no name
  left over. Its two relocations of 5 bytes are gone, which were the
  segments of 16 bits that `Int2E` of `resident.c` named.
- `c_waiting` of `add_portable_c32` is empty and
  `tools/refused.cmake` has nothing to try: both can go with the piece
  that next has `CMakeLists.txt`. A stale `resident.refused` may lie
  in a `build/portable_c32/` that was made before; `whole.py` passes
  over it.
- The classes `MAIN` and `STACK` are in the objects of 32 bits, by the
  two pragmas that stayed.

For H3: three batches of `harness/ported.py` hold that `Alloc0`,
`Exit01` and `Exit05_` have run, and the build of 32 bits has none of
them, as it has no `FatErr`: `GONE` wants them for that tree, with
`Start` for the first if it is to be held at all. And `Alloc3` is not
in the batch of the entry, which holds `Alloc0` alone: the C
translation has no function of that name, and the tables are one for
both trees.

### Piece D2, `dos32.c`, the rest: done but for what is the user's, and what a later piece should know

`dos32.c` makes 96 of the 98 functions of `sys.h`. The two that it does
not are `ProgramEndedTake` and `ProgramEndedGive`: what a program of 32
bits can do when DOS ends it by another's hand was tried, as 4 of what
is open for the user asks, and is reported in the last part below; the
two do nothing until that is decided. (They are made since: the
user decided for the second of the three ways of the last part, and "E,
the end by another's hand" behind this section has it.) The toy of the two builds,
`python3 harness/trial32/both.py`, writes the same for every function,
28 screens for 27 steps, for both countries; its last page has the two
that are left, which write the same too, since nobody ends the toy.

And the program itself runs: the piece B2 linked it while this one was
under way, and with the first part of this piece in it
`build/portable_c32/VC.EXE` shows its panels on the bench. With all of
it, `harness/show.py --program build/portable_c32/VC.EXE` took it
through F3, Esc, a command, `ver`, which the shell ran and which came
back, Ctrl+o twice, F10 and Enter: it left with DOS's prompt and no
word of the extender's. And the steps of three scenarios of the
critical error were given to both programs by `show.py`, `device`
with F10 and Enter behind it, `phantom` and `not-ready`, 9, 13 and 56
steps: what it prints of the
screens, their characters and where the cursor is, is the same for
`build/portable_c/VC.EXE` and `build/portable_c32/VC.EXE`, line for
line, the box of the critical error with Retry and Abort among them.
No screen was held against a golden, and no colour was looked at;
that is M1's.

The piece was made in five parts, a commit each, in this order: the
start, the memory and the PSP; the functions through real mode; a
program run and Quick execute; what DOS enters; the ends. Each part
below says what is done and what was found. `both.py` passed at each,
for both countries, and the tree linked: by the trial's `whole.py`
until B2 landed, and by the build since, whose two tests of the layout
pass.

**The start, the memory, the PSP: done.** `dos32.c` makes 81 of the 98
functions and has 17 that do nothing. New are `MemTake`, `MemGive`,
`MemResize`, `MemSetSearch`, `MemSetUpper`, `ProgramTail` and
`ProgramEnvironment`.

What `Start` has ready before `Main`, and whom it asks:

| What | How | Found |
|---|---|---|
| ES on the data, the direction flag clear | One piece of inline assembly, `Entered`, which answers with what ES was: the PSP's selector. It is the first thing of `Start`, so nothing is lost any more | |
| Where the PSP is | DPMI 0006h, where the segment of that selector begins. DOS is not asked, where the program of 16 bits asks 51h first | 25E60h on the bench, the PSP as DOS has it, 25E6 |
| Where the environment is | The word of the PSP at 2Ch, and DPMI 0006h of it | **The extender has put a selector there**, 0060h on the bench, where DOS had the segment, 25DC. The trial had not looked. Should the extender not know the word as a selector, it is taken for a segment |
| The block below the megabyte | DPMI 0100h, as D1 had it | A program that does not get it writes "Program too big to fit in memory" by DOS, 09h, and ends with 8. D1 had it end and say nothing, and left the saying to this piece. The words are the original's for the nearest case that it knows, too little memory to run, where it ends with 1: `dos32.c` has a copy of its own, since it reads nothing of the program's tables. See what is the user's, below |
| The program's own memory | `OwnSet`: no call | |

That word at 2Ch matters beyond `ProgramEnvironment`: DOS itself reads
it. A program run with no environment named gets a copy of its
parent's, which DOS finds through the parent's PSP, and there it is a
selector now. The next part but one has to name the environment's
segment to DOS at 4Bh, or the program that is run gets whatever lies
at 0060:0000. The trial's `command.c` named none, and its commands
were such that it did not show.

The memory, as "The memory" above has it: `Own` of `dos32.c`, 640 KB
and a paragraph, among what has no value. `OwnSet` works out its first
paragraph in the megabyte, `(Own + 15 - ARENA) >> 4`, and takes all of
it or none: if it does not lie behind `ARENA` with its end below the
paragraph FFFFh, `MemTake` answers that there is no memory, and the
program says "Program too big to fit in memory" and leaves. Nothing
else checks where the linker has put it, so:

- The 640 KB are a variable of `dos32.c` without a value, `Own`, and
  so of `_BSS`, as the piece B2 asks: with `dos32.obj` linked last it
  is the last thing of the program and not in the file, and the tests
  `portable_c32-layout` and `portable_c32-debug-layout` pass with it.
  In the program as it is linked today `Own` begins 63,200 bytes
  behind `md`: its first paragraph of the megabyte is 1F6Eh, and the
  640 KB end at BF6Fh, of FFFFh.
- Those tests hold that the program ends within the megabyte, and so
  that `Own` does. That it begins on a paragraph is `OwnSet`'s doing.
- The toy has `md` among what has no value too, in front of `Own` by
  the order of its objects, so the toy says nothing of the order that
  the program needs; the layout's tests do.
- Nothing promises that what has no value is zeros when the program
  is entered (the trial, "What the bench has"). `OwnSet` writes what
  it needs, and every other variable of `dos32.c` has a value and is
  in the file. A program that reads its 640 KB before it has written
  them finds whatever the memory held: "What could go wrong", 7.

The blocks are handed out as DOS hands out its own, since the program
goes by that (`Init11`, `GiveMem` and `TakeMem` of `start.c`, `Aexec0`
of `view.c`): a paragraph in front of each block says whether another
follows, whether it is taken and how long it is; the lowest block that
is long enough is given; a refusal says the longest; free blocks that
lie one behind the other become one when the memory is looked through,
not when one is given back; a block that is to grow is first made as
long as it can be and stays so when that is not enough. That is
FreeDOS as its source has it and as the toy shows it on the bench, and
what comes back in `value` is what it leaves in AX:

| | Not failed | Failed |
|---|---|---|
| `MemTake` | The block | 8, `failed` FFFFh, `largest` the longest that is free; 7 if what is in front of a block is spoilt |
| `MemGive` | 4900h, also for a block that was given back already | 9 for what is no block |
| `MemResize` | The block: FreeDOS leaves ES in AX there | 8 with how long the block has become; 7 for what is no block |

`MemSetSearch` and `MemSetUpper` are passed to DOS as they are, 5801h
and 5803h, as the plan has it: they say how DOS looks through its own
memory, which a program that this one runs is given of.

One thing is otherwise than on DOS and stays so: with the largest
block taken, DOS on the bench still has memory to give, a block or two
lower down that are free besides, one paragraph on the bench of the
United States and more than 80h on the Russian one. The program's own
memory is one stretch. The toy takes those blocks too before it asks
for more, so that both builds give of what the first block leaves.

The toy has a page of its own for these, behind a step `m`, 12 pages
for 11 steps: all the memory taken as the program takes it and written
at both ends through its segment, which in 32 bits holds that the
block is where `AT` of `vc.h` looks for it; three blocks taken, given
and taken again, with where each lies behind the first; a block made
shorter and longer, refused with a block behind it and with too little
free behind it; what is no block given back and made longer; a block
given back twice. And of the PSP the tail, the first text of the
environment, and behind the environment's end the two bytes and the
path of the program's own file, `D:\VC\VC.EXE` in both. The page is
the same in both, line for line.

**The functions through real mode: done.** 86 of the 98 are made, 12
do nothing. New are `FileTrueName`, `FcbParse`, `FcbDelete`,
`CountryInfo` and `CaseMap`. The block below the megabyte is 416 bytes
now, with a place of its own for each thing that DOS reads or writes
at an address (`LOW_` of `dos32.c`).

| Function | The call | What goes down and comes up |
|---|---|---|
| `FileTrueName` | 60h, DS:SI the name, ES:DI the path. With the program's errors | The name down; 128 bytes of the program's buffer down and up |
| `FcbParse` | 29h, DS:SI the name, ES:DI the block. Bare | The name down; the 16 bytes of an unopened block down and up, since `how` can have DOS keep what the block has |
| `FcbDelete` | 13h, DS:DX the block. With the program's errors, and with `flagged` as D1 has it for 36h | 44 bytes down: an extended block, which is what the program hands over. Nothing up: the function takes the block as one that it does not write |
| `CountryInfo` | 38h, DS:DX. Bare | 34 bytes down and up |
| `CaseMap` | DOS's routine, by DPMI 0301h, AL there and back | Nothing |

Down first and up afterwards, wherever DOS writes: the program then
has what DOS wrote and, of the rest, what it had, as when DOS writes
into its buffer itself. It shows at once: FreeDOS writes 24 of the 34
bytes of the country and leaves the path alone where 60h fails, and
the toy, which fills each buffer first, prints the same in both.

`Real` of `dos32.c` is the one call of real mode: an interrupt by DPMI
0300h, or a routine called far by 0301h, with DS and ES on the block
and on a stack of the extender's. `Extender` takes the registers of a
service as `Dos` takes DOS's, so a service that answers in CX and DX
can be asked.

`CaseMap` answers with the letter as it came until `CountryInfo` has
been asked. `dos.c` calls whatever its four bytes hold then, which are
zeros: the toy of 16 bits ended in "Invalid Opcode" when it was asked
so, and is not asked so any more. The program asks for the country
first, in `IniData`.

`CaseMap` costs a call of real mode by 0301h, some 700 instructions
by the trial's figure for 0300h. The program asks it 128 times as it
starts, for its table of small letters, and then for a letter beyond
ASCII that is typed or compared.

The toy has a page for these behind a step `n`: the full path of a
name, in small letters, with dots, in a directory that is not there
and on a drive that there is not; a mask and a name made a file
control block, with a drive, with a drive that there is not and with
the block's own drive kept; files deleted by a mask, twice, with the
flag, and by an extended block; the country as its 34 bytes; and
seven letters made capitals. The same in both, for both countries.

**A program run, and Quick execute: done.** 89 of the 98 are made, 9
do nothing. New are `ProgramRun`, `ProgramRunBare` and `ShellRun`. The
block below the megabyte is 512 bytes.

`ProgramRun` and `ProgramRunBare` are 4B00h in real mode, the first
with the program's errors, with everything that DOS reads in the
block: the name; the tail, 128 bytes of it whatever its count, since
DOS copies 128 into the PSP of the program; the two file control
blocks, 16 bytes each, the bytes of the main data at 5Ch and at 6Ch
as `Run` of `dos.c` has them; and the 14 bytes that name those.

| What | 16 bits | 32 bits | Why |
|---|---|---|---|
| The environment in DOS's block | 0: DOS copies this program's | The segment of this program's environment: DOS copies that | The word of the PSP at 2Ch, through which DOS finds the environment to copy, is a selector of the extender's. Tried with 0: a second copy that the toy ran by itself had an environment without `COMSPEC`. Run through the shell it had one all the same, since FreeCOM gives what it runs its own; that is why the trial's `command.c`, which named none, showed nothing |
| INT 22h of real mode | DOS has it name where it was called from, and puts it back from the PSP when the program ends | Read before the call and put back behind it, DPMI 0200h and 0201h | The extender ends first, and writes "DOS/32A warning (9003): real mode interrupt vector has been modified: INT 22h" onto the last screen. Tried without: it does; with it the last screen is as in 16 bits |
| CX, SI and DI at the call | As they come | 0, as D1 has every register that a call does not read | |

`ShellRun` is the trial's way: 26 bytes of real mode in the block,
which keep the stack in four bytes behind their own segment, call INT
2Eh, put the stack back with the machine kept from interrupting, and
return far, called by DPMI 0301h with DS:SI on the command, which is
in the block too. INT 22h is put back behind it as behind a program,
since a shell that serves the interrupt runs programs. On the bench it
comes back and runs nothing, as in 16 bits: FreeCOM does not serve the
interrupt. That the stack comes back right behind a shell which does,
is from reading here as it is in `dos.c`.

What was not looked at: whether a shell that serves INT 2Eh leaves
INT 23h and 24h of real mode as it found them. DOS puts both back from
the PSP of a program that ends, so a program run by 4Bh leaves them;
a shell that takes them for a command of its own interrupt and does
not give them back would leave the extender's routines out of DOS's
reach, and the extender with a warning at the end. No bench of the
project can show it.

The toy has three pages more, 17 for 16 steps: the shell found in the
environment and run bare for a command that leaves a file, which is
read; a file that is not there run both ways; the toy's own file run
with something behind its name, by itself and then by the shell as a
command, each of which is a second copy that writes a page of its own
with its tail, its shell by its environment, the path of its file and
whether it has memory, waits for a key that the driver gives it, and
ends; and a command handed to the shell. The same in both, for both
countries, and no warning on the last screen.

For W: entry 6 of `DEVIATIONS.md` has what DOS is handed in the
registers that it does not read, call by call, for the program of 16
bits. In 32 bits each of those is 0, at every call, and the
environment is named at 4Bh: the entry wants a sentence for that.

**What DOS enters: done.** 96 of the 98 are made; the two that are
left are the program ended by another's hand. New are the seven of
the critical error, Ctrl-Break and Ctrl-C. This is "What could go
wrong", 3, and it did not go wrong: the program is asked from inside
a call of DOS, draws, waits for keys through the extender, idles
there with the driver of the tests taking each idle call and ending
its steps on it, answers, and DOS goes on.

How it is made:

- The three are taken by DPMI 0205h, with a routine of protected mode
  each, and who had one is asked by 0204h and kept in six bytes of
  `dos32.c`. DOS is not asked for a vector, where `dos.c` asks 35h and
  25h. `ErrVct` and `BrkVct` of the main data are not written.
- `DeviceEntry` is where INT 24h arrives, 33 instructions of assembly.
  It keeps the registers on the extender's stack, 44 bytes of it with
  what the interrupt left there; finds the data by its selector, which
  `Start` notes, read through CS; goes over to the program's stack;
  calls `Arrived`, which is C; and goes back. `Arrived` reads the
  header of the device at BP:SI of real mode, as `FatErr` of `dos.c`
  does, and calls the function that the program handed over. AX whole
  goes back to DOS.
- Where the program's stack is free: `Below`, the stack pointer where
  `Call` last called DOS or `Extender` the extender, less 512 bytes
  (`STACK_KEPT`). The extender is entered on the program's stack and
  keeps there what it needs of the call that is under way, so the
  routine cannot go on right below where DOS was called. A toy that
  fills the stack below the stack pointer and looks what a call has
  written, `harness/trial32/depth.c`, saw 190 bytes at a call that the extender translates, 3Dh,
  3Fh, 4Eh; 130 at one made by DPMI 0300h; 74 at every other, of DOS,
  the BIOS, the mouse and the idle call; and 70 for the timer's
  interrupts in protected mode. The 512 are that and as much again.
  **The program's stack is used that much deeper at a critical error
  than in 16 bits**: for the test of the stack that stays half empty,
  H3.
- Ctrl-Break and Ctrl-C arrive at one instruction, `iretd`.
- `CtrlCTake` notes who had Ctrl-C the first time that it is taken:
  `sys.h` has no function for that, since DOS puts it back by itself
  when a program of 16 bits ends.
- `ProgramEnd` gives back what is still taken of the three before it
  asks DOS, by the extender alone. Tried without: the extender writes
  "warning (9003): real mode interrupt vector has been modified" for
  INT 1Bh and for INT 24h onto the last screen, and nothing for 23h.
  With it the last screen is as in 16 bits.

What the toy shows, on eleven steps of four pages, 28 screens for 27
steps, the same in both builds and for both countries:

| What | What was seen |
|---|---|
| A file made on the diskette that the bench boots from, which cannot be written | The toy's function is asked, AX 3900h and DI 0, a drive. It lets the machine interrupt, writes a line, and waits as the program waits, by `Idle`, `MouseWhere`, `KeyWaiting` and `KeyRead`; a step of the script ends there, inside the question, in both builds |
| Answered by `r` | DOS tries again and asks again |
| Answered by `f`, noted in `ErrDat` as `FtErr1` notes it | The call fails with 53h |
| Answered to fail and not noted | The call fails with what DOS says of it, 13h |
| `FileSetAttributes` there, a call that `dos32.c` makes in real mode | The same |
| A byte written to `COM2`, which the bench has not | Asked with AX B800h, DI 00B0h and the name `COM2    ` |
| The shell run for a copy to the diskette | The toy is not asked: FreeCOM takes the critical error itself and asks by its own words, in both builds. DOS has put the toy's back when the shell has ended |
| A second copy of the toy run, which makes a file on the diskette and has not taken the critical error | The first copy is asked, in the middle of a call of DOS that the second made: in 32 bits a second extender is in real mode then, and the first comes over to protected mode from under it, waits for its key, and goes back. The second gets its error, and the first is answered 53h for the run, as in 16 bits, since `ErrDat` is the first one's |
| Ctrl-Break | Nothing, and a key of no character in the BIOS's buffer |
| Ctrl-C, with DOS told to look for it at every call, and DOS asked the time | DOS writes `^C`, takes the key, and the toy goes on |

Not tried, and why:

- The phantom drive of `not-ready` and `phantom`, by a toy. What the
  program does there is its own, `Fantom` and `FtErr1` with `BlocEr`,
  through `DriveLetter` and `DriveSetLetter`, which D1 made; to
  `dos32.c` it is a question like any other. The program itself was
  asked instead, once B2 had linked it: see the top of this section.
- The answer Ignore: the program never gives it.
- A program run by `Aexec0` that is of real mode and does not take the
  critical error itself. The second copy stands for it, and is the
  harder case.
- How long the extender's stack of protected mode is. The routine
  takes 44 bytes of it.

For M1 and the convergence: a critical error inside a call counts
toward what the program's stack has to hold twice over, the 512 and
then what `FtErr1` and `Dialog` take. And `IntOn` inside the question
is `sti` in protected mode, which the extender lets a program do.

**The ends: `ProgramEnd` done, and what was seen of the program ended
by another's hand.** `ProgramEnd` gives back what is still taken, as
the part before has it; `ProgramEndOld` has nothing to do. For the
other two it was tried what happens, with a toy of real mode,
`harness/trial32/ended.asm`. It does what `p.com` of the scenario
`ended` does: it takes INT 28h, runs a program with 1111h in SI and
2222h in CX, and at that program's first idle call asks DOS which
program runs, 62h, and has DOS end it, 4Ch. Then it writes the
registers that it got back, what DOS says of the end, DOS's largest
block, the memory above the megabyte, and every interrupt of real mode
that no longer goes where it went before. It was given the program of
16 bits and that of 32, each as it stood at its first screen, and
keys were typed at DOS's prompt behind it:

| | The program of 16 bits | The program of 32 bits |
|---|---|---|
| Where DOS goes on | In the program, `Exit01`, which tells the program's function and goes on in the parent | In the parent, behind its call |
| CX, SI | 2222h, 1111h | The same: as they went |
| ES | The PSP of the ended program | The parent's own PSP: as DOS leaves it |
| DX | 0201h: in DL what DOS was last told of Ctrl-C | 02DFh, 0271h, 0260h in three runs: as DOS leaves it |
| The flags | 0202h | 0202h |
| What DOS says of the end, 4Dh | 0009h | 0009h |
| Whether DOS looks for Ctrl-C at every call, which the bench has on and the program switches off while it runs | On again: the program's function did that | **Off**: nothing of the program ran |
| Interrupts of real mode that go elsewhere than before | 1Bh, into the ended program, where Ctrl-Break was taken and is never given back; and 22h there too | 22h alone, to the parent's own call, which is DOS's doing |
| DOS's largest block, the memory above the megabyte, the mode | 7898h paragraphs, 3B80h KB, real | The same |
| The machine afterwards | A command runs, Ctrl-Break writes `^C` | A command runs; the program of 32 bits starts again and runs; Ctrl-Break writes `^C` once `BREAK ON` has been typed |

So what the plan had from reading is half so. Nothing of the program
runs, as it thought. But the extender is not left lying in the memory:
while the program runs, INT 21h of real mode goes to the extender, in
the program's own block (2707:0E99 for a PSP at 26F1), and when the
call that ends the program comes by there in real mode, the extender
takes itself out, before DOS does anything. Seen: at the idle call
INT 1Bh, 23h and 24h went to the extender's routines, 32F2:0000, :0032
and :004B, the program having taken all three, and behind the end
every one of the 256 went where it had gone before the program was
run, 1Bh among them, which DOS does not put back. The memory is DOS's
again, and a second run starts as the first did. The program of 32
bits leaves less behind than the one of 16, which leaves Ctrl-Break
pointing into memory that is free.

What is not put back is the one thing that the program's function is
there for: DOS looks for Ctrl-C as the program set it, off. And the
parent sees DOS's registers where the original leaves it ES on the
ended program, DL and the flags of a segment compared, which `ended`
prints.

One thing that looked like a fault and is none: with `BREAK` off,
Ctrl-Break at DOS's prompt ends no step of the script, on any
program, the toy of real mode alone included: `run.py` then says that
nothing was written for its limit of seconds. With `BREAK ON` typed
first it writes `^C`.

What is the user's of this piece:

- What the program of 32 bits does when DOS ends it by another's
  hand: 4 of what is open, with the report above and the ways below.
- The words of a program that cannot start for want of memory below
  the megabyte. It says "Program too big to fit in memory", the
  original's words for the nearest case, by the session's word, so
  that the program has no words that the original has not; the case
  is one that the program of 16 bits cannot be in, and the user may
  want others. `NoStart_S` of `dos32.c` is the one place.

That is the report, for 4 of what is open. The ways that there seem
to be, for the user:

| Way | What it takes | What it leaves |
|---|---|---|
| Declared, as the plan thought: a program of 32 bits that DOS ends is not told | Nothing: the two functions do nothing, as they do. `ended` gets a patch or is not run for this build | DOS no longer looks for Ctrl-C at every call behind such an end, where it did before the program ran. Whoever ended the program sees DOS's registers |
| A stretch of real mode for where DOS goes on, in the block below the megabyte, named in the PSP at 0Ah as `Exit01` is: it tells DOS to look for Ctrl-C as before, 3301h, makes the registers what `Exit05` makes them, and goes on where the PSP said | Some 30 bytes of real mode, written as bytes as `ShellRun`'s are, and run in a block that DOS has freed by then, as `Exit01` runs in a program that it has freed. Not tried | It cannot call the program's function, which is of protected mode in an extender that has gone: it does what that function does, so `dos32.c` knows what the program puts back, where `sys.h` has the program say it. `ProgramEndedTake` would want to take that as a number and not as a function, in both builds |
| The extender left in place until the program's function has run | Not looked at: the extender takes the call in real mode before DOS, and nothing here is asked first | |

Until it is decided `told` is never called, and the comment at the
two in `dos32.c` says what was seen. (Decided since, for the second
way, and made: the section behind this one.)

**For M1 and those behind it**, of the whole piece:

- The program's stack. A critical error takes of it what the program
  had used where it asked DOS, 512 bytes that are left alone, and what
  `FtErr1` and its box take. The toy, of 16K, leaves 15,558 bytes
  never used without the pages of the critical error and 15,442 with
  them.
- `CaseMap` is a call of real mode, and the program asks it 128 times
  as it starts.
- A function of `sys.h` that is asked from inside the critical error
  and goes by `Call` or by `Extender`, `CaseMap` say, sets `Below`
  anew, deeper; `Arrived` puts it back when the program has answered.
  DOS must not be asked there at all, as `sys.h` says.
- What DOS is handed in the registers that a call does not read is 0
  here, at every call, and the environment is named at 4Bh: for entry
  6 of `DEVIATIONS.md`, at W.
- The 16-bit program gives who had the critical error and Ctrl-Break
  in `ErrVct` and `BrkVct` of the main data; here those two stay as
  the memory had them, and nothing reads them.
- `harness/trial32/both.py` and `sys.c` are the place to try a
  function of `dos32.c` by itself, in ten seconds; a page wants a step
  in `STEPS`. `build/portable_c/dos.obj` has to be built for the 16-bit
  half.

### C1, the editor: nine of the ten pass, and one is the user's

The first piece of the convergence took the ten scenarios that H3 has
as the editor's, four families by the look of the first screen that
differs. They are two causes. One was in the program and is mended:
nine scenarios pass with it and are on the record, which has 154 of
159. The other is how much memory the program has, which is no fault
of a statement and is the user's to decide: `editor-big`.

| | Cause | Scenarios | What became of it |
|---|---|---|---|
| 1 | A step of FF60h, 160 back, added to a pointer into the screen buffer: `T71_00` of `edit.c` | `editor-long`, `editor-full`, `editor-buffer`; `user-menu-run`, `user-menu`, `comspec`, `view-commands`, `editor-save`; `editor-keys`: H3's families 1, 2 and 3 | Mended, the 16-bit program the same bytes |
| 2 | The editor has room for 641,824 bytes where the original has it for 345,840 and the portable C of 16 bits for 291,360: the 640 KB of the program's own, 2 of what the session took | `editor-big`: H3's family 4 | Open, for the user: see below |

**Cause 1, `T71_00`.** The routine moves the rows of the editor's or
the viewer's window by one in the screen buffer, before the BIOS
scrolls the screen: `place` is where the first row goes to and `step`
the way from one row to the next, 160 to move them up and -160 to move
them down, in a word of 16 bits, so FF60h. It said

    u16 *from = (u16 *)((u8 *)to + step);
    ...
    from = (u16 *)((u8 *)from + step);
    to = (u16 *)((u8 *)to + step);

| | What the statement means |
|---|---|
| In 16 bits | `to` is a near pointer, an offset of 16 bits in the main data, and FF60h added to it wraps round: 160 bytes back, the row above |
| In 32 bits, as it was | The pointer has 32 bits and nothing wraps: 65,376 bytes on. The first row is read from there, and every row behind it is written 65,376 bytes further on than the one before |
| As it is mended | `MD_POINTER(MD_PLACE(to) + step)`: the sum is made in the place, a number that the word cuts to 16 bits, where it wraps round as on DOS. `to` is `BasAdr` of a place and so in `ScrBuf` of the main data, which is what `MD_PLACE` takes. On DOS the two words are the two casts and the code is what it was |

It is 1 of "What could go wrong" and what P4's finder says that it
cannot see: no arithmetic at the place, a variable of 16 bits that
holds a number below 0 as one above 32,767.

What it did, by the numbers of the program as it is linked: `ScrBuf`
is at 2448h of the main data, the 640 KB begin 0F6E0h behind `md`,
and the text of the editor and of the viewer, at `ViewSeg`, is 12BD0h
behind it, behind the two panels.

- A window of 23 rows, a file without help below it: the last row is
  at 32A8h of the main data, and FF60h on from there is 638h of the
  text. Seen, in the program before the mend: `Shift+F4` on the
  bench's `command.com`, `Enter`, `F4`, and the editor shows at 638h
  of the text, where the file has zeros, the 160 bytes that the file
  has at 10598h, FF60h on: "of memory.", "If this error persists".
  The mended program shows the zeros. The row that the window got in
  place of the one above it was what the memory has at 638h of the
  text, which is zeros where the text is shorter: H3's family 1, rows
  that hold zeros. How each of its four rows came to be where it is
  was not followed.
- A window of 13 rows, the menu file or an extension file with the
  help below: the last row is at 2C68h, and FF60h on is 8 bytes in
  front of the text. So Enter, which moves the rows below the cursor
  down, wrote 152 bytes of what lies 64K on, zeros, over the start of
  the text, the line that had just been ended with it: the editor
  found no second line and stayed in the first. That is family 2, and
  3, which begins with Enter.
- The rows behind the first went 65,376 bytes further each. With
  Enter in the first row of a window of 23 rows that is twenty rows
  written, the eleventh and those behind it beyond the end of the
  program, which is 0AF6FFh behind `md`: into memory that the extender
  had not given it, where a second copy of the program is loaded.
  Nothing was seen to come of that.

What leaned on the same body: the viewer, which calls `T71_00` with
-160 for Up where the window moves down by a line (`V05_02` of
`view.c`). The BIOS scrolls the screen and the viewer writes the new
first row, so the screen was right; the screen buffer was not moved,
its last row got what lies 64K on, and 160 bytes at 638h of the text
that the viewer had loaded were written over, with the rows behind
that as in the editor.

**Scenarios that passed went through it.** Eleven of the 145 that
were on the record before the mend made the call with -160, by what
ran in the original, `harness/goldens/NAME.ran`: the three
instructions that load the step for it, `MOV DX,-2*80` at 5485h,
644Ah and 6853h of the listing.

| The call | Passed before the mend | Failed before it |
|---|---|---|
| The viewer, Up: `V05_02`, 5485h | `view`, `view-parts`, `view-keys`, `mouse-view` | |
| The editor, the window a line down: `T00_34` of `T00_20`, 644Ah | `editor-search`, `start-screen` | `editor-full`, `start-memory` |
| The editor, Enter or a line put back: `T02_03`, 6853h | `dirinfo`, `editor-words`, `editor-dirinfo`, `mouse-edit`, `start-setup`, `start-screen` | The nine that the mend brought to pass |

So eleven scenarios read a row from 64K on and, wherever more than
one row was to move, wrote 64K off, into the text and beyond it, and
showed nothing, which says that none of them puts that part of the
text or that row of the buffer on the screen afterwards. The viewer's four did so in every
run, a window of 23 rows having 21 rows to write. `start-memory`
fails still, for its second copy. They are judged by the mended
routine now, and no scenario was written for it. A scenario that
passes says that nothing wrong came to the screen or the disk, not
that nothing wrong was written.

**What would have stopped at it, and `CHECKED`.** The debug program's
checks, which B2 left to be written, would not have, as the plan has
them: they are a third body of the words of the addresses, and hold
what a word is handed, a segment that the program has and a place
within the main data. This write went through no word. It was a sum
on a pointer, and that is the fault: the mend is that it goes through
two words now. Nor could a check of the address alone tell the first
of the writes from a right one, since 64K on from the screen buffer
is the program's own 640 KB, memory that it may write.

What the checks are still for is the other half of 1 and 2 of "What
could go wrong": a wrapped offset or a segment that is handed to
`AT`, `PANEL` or `MD_POINTER` and names something that the program
has not. No such fault has been met yet. What would have stopped at
this one:

- A check that knows the object and not the address: that a pointer
  made from `ScrBuf` stays in `ScrBuf`. Nothing in C written for Open
  Watcom gives that. clang's sanitizers do, which is step 6 of the
  road, a program that clang builds and that runs.
- For the rows from the eleventh on, memory that faults behind the
  end of the program. The bench has none: the extender does not page,
  and what lies there is the machine's.
- The finder's new kind, had it been there when P4 read the tree: see
  below. It names the three calls, and whoever reads them opens
  `T71_00`.

So the argument for writing `CHECKED` is not this fault. It is that
eleven scenarios passed over a write 64K off: the scenarios do not
find wild writes, and the 32-bit program has no other net under it
until clang builds it. A check costs nothing in the program that is
shipped, and the piece that writes it should know that it covers the
words and not the sums beside them; `harness/trial32/added.py` lists
those.

Its like elsewhere, looked for by reading and by `grep`:

- A number written below 0 and made one of 16 bits: `(u16)-160` twice
  in `edit.c` and `(u16)-(2 * 80)` in `view.c`, the three calls of
  `T71_00`; and `(u16)-(int)sizeof(struct DIRENT)` in `treebox.c`,
  which is held against a difference and added to nothing. The other
  windows that the BIOS scrolls, of the panels, the tree's box, the
  help and the file search, write their rows anew and move nothing in
  the buffer.
- `edit.c` was read whole for a number of 16 bits that may have
  wrapped round and is added to a pointer. `T51_30`, which copies the
  text down from addresses made with offsets of 0FFEFh and more, goes
  through `AT` for each piece of 0FF00h bytes and steps no further
  than 0EFh; `T54_00`, whose offset goes below 0 and is put right by
  taking a lower segment, hands the number to `AT`; `T50_00`, `T72_00`
  and the keys step a pointer up by a count, or back by one that has
  no sign. Nothing else was found.
- `(u8 __far *)wcb + file` of `files.c`, `copy.c` and `commands.c`
  adds an offset in a panel to the panel, which is its segment's
  beginning; `ENTRY` does the same with the cut. `screen = (u16
  *)((u8 *)screen + count)` of `dialog.c` adds twice a column.

**Cause 2, `editor-big`: open, for the user.** The scenario makes a
file of 381,338 bytes, "within 64K of what the editor has room for",
and has the editor say that it is too large. The editor's room is
`EdSpace`, the paragraphs between `ViewSeg` and the end of the memory
that the program got at its start, `MAIN_SEG + FreMem2 - ViewSeg`.
What the editor shows as free with the bench's `command.com` of
87,772 bytes loaded, and so the room:

| Program | Free | Room |
|---|---|---|
| The original | 258,068 | 345,840 |
| The C translation | 204,516 | 292,288 |
| The portable C of 16 bits | 203,588 | 291,360 |
| The portable C of 32 bits | 554,052 | 641,824 |

So the file is too large for three and fits the fourth, which opens
it, where the original shows its box. Nothing else is in it: with
`OWN_PARAGRAPHS` of `dos32.c` at 5000h, 320 KB, tried and taken back,
the scenario passes whole. The figure is of the kind that
"Scenarios" of `CLAUDE.md` leaves out of a comparison, one that
follows the size of the program, and the count of free bytes in the
editor's status line is left out already; but here it decides what
the program does. The ways that there seem to be, none of them taken:

| Way | For | Against |
|---|---|---|
| The scenario makes a file that no build has room for, eight times `command.com`, 702,176 bytes, in place of four times and `country.sys` | It tests what its title says, on every build, and nothing is declared | The file is no longer within 64K of the room of any build, which is what the scenario was written to try: whether the lengths are held against one another as numbers of 32 bits. The golden of the original is recorded anew, of a changed scenario. Drive C: of the bench has to hold it |
| The scenario stays, and a second one has the larger file | Both are tried | The first still fails for this build and wants a patch or to be left out for it |
| A patch of the scenario for this build: the editor opens the file | Says what is so | A deviation: the program does otherwise than the original with the same file, for memory alone. It is the other way round from 3 of what is open, the second copy that has memory to run: there the program of 32 bits does again what the original does, and here it leaves it |
| The program's own memory made as short as DOS leaves the 16-bit program | The scenario passes as it is | A number chosen to make a test pass; and it is gone with step 2, when the memory is the extender's |

For the piece that takes it: nothing of `editor-big` behind its step
28 is known to pass but by the trial with 320 KB.

**What the finder was taught.** `harness/int16.py` could not see the
fault: `(u8 *)to + step` is a pointer and a variable, which may hold
any number of its type. What can be seen is where the number comes
from: `(u16)-160`, a number written below 0 and made one of 16 bits
without a sign, at the three calls. That is a kind of place now,
`minus`, found where such a number is cast, handed to a function,
returned or given to a variable, and the reader follows it to where
it is used. The tree has four: the three calls of `T71_00`, whose
reason is that the routine adds the step to a place, and the
comparison of `treebox.c`. It is fifteen lines of the finder and
twelve of `harness/int16check.c`, six that are to be found and six
that are not. Had it been there for P4, the three calls would have
been read with `T71_00` open.

What it still cannot see is a number that is worked out and has
wrapped round, `first - shown`, kept in a variable and added to a
pointer later. No rule says of a variable what it holds, so those
were read: `python3 harness/trial32/added.py` lists every place of
the tree where a variable of 16 bits, a function's own or one that it
is handed, is added to a pointer as it is or is the index of one, but
for a variable that a loop counts with. 158 places, of which `T71_00`
had been three. Each was read for whether the variable can stand for
a number below 0 there, and none can: they are counts, columns,
lengths of names, and offsets in a panel or in the viewer's text,
added to where the segment begins. `EdSub1` of `input.c`, which
steps a cursor back, asks that it is not 0 first. What the list
leaves out is a field of a structure added to a pointer, `(u8 __far
*)wcb + wcb->CurAdr`, of which the tree has many and which hold
offsets by what they are; and a pointer stepped by itself past where
its segment ends, 2 of "What could go wrong", which nothing lists.

The host check could not be taught it: gcc reads the second bodies
and has nothing to say of a sum.

For the pieces behind this one:

- Nine of the fourteen were one statement. A family by the look of
  the first screen is not a cause: three looks here were one write
  64K off, which showed as rows of zeros, as a line that did not end
  and as a cursor in a wrong column, by where the window's last row
  happened to be.
- How it was found, in the order that paid: the editor's file read
  for what `TODO.md` names, a number that has wrapped round added to a
  pointer; the place mended and the ten run, `python3
  harness/compare.py build/portable_c32/VC.EXE NAME --first`, nine of
  which passed; then the proof, the text shown in hex by both programs
  after one Enter, held against the file on the bench's floppy. The
  program before the mend was kept beside the build for that, in a
  directory named as its tree is, `build/c1/old/portable_c32/`, which
  the harness takes for the same tree.
- A wild write shows where it happens to land, and may show nowhere:
  the viewer's was in every scenario that goes up a line.

### E, the end by another's hand: done, and what a later piece should know

On the user's word of 13 October 2026 (`CLAUDE.md`, "The portable C"),
which is the second of D2's three ways: when DOS ends the program of 32
bits by another program's hand, DOS is told of Ctrl-C again as the
program found it, and the program that did it goes on with the
registers that the 16-bit program leaves it, by a small routine of real
mode that `dos32.c` keeps. `sys.h` has 99 functions where it had 98,
and `dos32.c` makes all of them. `ended` passes for the program of 32
bits with the patch that the portable C has, and is on the record.

**The shape of `sys.h`, and why.** The program handed the system a
function, `ProgramEndedTake(told)`, which the system called when DOS
had ended the program. A program of 32 bits cannot be called then: the
extender is gone before DOS goes on, and the program's memory with it.
So what the header says is otherwise now, for both builds: the program
says beforehand what it has changed of the system that the system does
not put back by itself, as a value, and the system puts that back.

| Function | What it is | In `dos.c` | In `dos32.c` |
|---|---|---|---|
| `ProgramEndedBreakCheck(on)` | Whether the system is to look for Ctrl-C at every call when it has ended the program, as `SetBreakCheck` takes it. Said whenever the program has noted that anew, before `Take` or after | Kept in a byte, `EndedBreak` | Kept in a byte, and written into the block below the megabyte, where the routine reads it |
| `ProgramEndedTake()` | From now on the system puts that back when it has ended the program by another's hand, before the program that started this one goes on | The PSP's address at 0Ah kept in `Vect22`, and `Exit01` written there, as it was | The same address kept, the routine and what it reads copied into the block, and the routine's address written there |
| `ProgramEndedGive()` | No more: said before the program ends itself | `Vect22` written back | The address written back |

Why a value for both, and not a function where one can be called and a
value where not: a header that takes both says two things, and each
file would honour one of them. A function of the program's that the
system calls with the program gone is a promise that only one of the
two files can keep; a value both keep in full, and `sys.h` says one
thing. It is also what a later system can keep: what has to be done
when a program is ended from outside is done where little of the
program can be relied on.

Why three functions and not two, with the value handed to `Take`: the
original reads `BreakFl` where DOS has ended it, so what is put back
is what `SetBrk` noted last, after every command. The program has to
say it again each time, and the start takes its end behind `SetBrk`
and not in it. With one function for both, either the start would take
its end two calls of DOS early, or the value would be stale for two
calls of DOS after every command. As it is, `SetBrk` says the value in
the line behind the one that notes it, `TakeEnd` takes where the
original's `TakeDos` did, and no call of DOS is made for either, in
either build.

**The program's side.** `resident.c` alone: `SetBrk` has one line
more, `ProgramEndedBreakCheck(md.BreakFl)`, behind the line that notes
it; `Exit02`, which was the function handed over and one line long, is
that line in `Exit00`; `TakeEnd` calls `ProgramEndedTake()`.

**16 bits.** `Exit05` of `dos.c` tells DOS itself, `SetBreakCheck` with
what the program said, where it called the program's function, which
did the same; DL for the program that started this one is that value,
and `BreakTold`, the byte in which `SetBreakCheck` kept what it told
DOS last, is gone. `Take` takes once and `Give` gives once, by
`Vect22`, which is 0 while nothing is taken: the toy takes twice.
`VC.EXE` is 197,084 bytes, 14 longer. What DOS is asked, in which
order and with which registers is as it was: entry 6 of
`DEVIATIONS.md` has the row under `Exit00` and `Exit05` now, with the
same registers.

**32 bits.** Eight instructions of real mode, 28 bytes, `Ended_T` of
`dos32.c`, written as their bytes with the instruction beside each,
since the compiler writes for a segment of 32 bits. They are copied to
`LOW_ENDED` of the block below the megabyte, which is 560 bytes for
that where it was 512, and read three things behind them there,
through CS: `LOW_WAS`, where the PSP had DOS go on; `LOW_PSP`, the
PSP's segment; `LOW_BREAK`, what the program said of Ctrl-C.

    push ax
    mov  ax,3301h
    mov  dl,cs:[LOW_BREAK]
    int  21h
    pop  ax
    mov  es,cs:[LOW_PSP]
    cmp  word ptr cs:[LOW_WAS+2],0
    jmp  dword ptr cs:[LOW_WAS]

That is `Exit01` and `Exit05` of `dos.c`: DOS told, AX kept, DL what
DOS was told, ES on the ended program's PSP, the flags those of the
segment that the program goes on in compared with 0, every other
register as it came. DOS has freed the block by then, and its bytes
are still there, as the program's own are where `Exit01` runs. The
extender is out of INT 21h by then, so the call goes to DOS; where a
first copy of the program runs underneath, through that copy's
extender, which passes it on.

The build has nothing new: no tool, no step, no line of
`CMakeLists.txt`. The bytes were read back once with `ndisasm -b16`.
`harness/datacheck.py` and `harness/reads.py` have an entry each for
the table, as they have for `Shell_T`.

**What was seen.** `harness/trial32/ended.asm`, which prints the break
state itself now, as the last word of its second row, against each
program as it stands at its first screen:

| | 16 bits | 32 bits, before | 32 bits, now |
|---|---|---|---|
| AX, CX, SI, DX, DS, ES, the flags | `4B00 2222 1111 0201 25E6 26F1 0202` | `4B00 2222 1111 0271 25E6 25E6 0202` | `4B00 2222 1111 0201 25E6 26F1 0202` |
| `break` typed at DOS's prompt behind it | on | off | on |
| Interrupts of real mode that go elsewhere than before | 1Bh and 22h, into the ended program | 22h, to the parent's own call | 22h, into the block, `3391:0200` |
| DOS's end code, its largest block, the memory above the megabyte, the mode | `0009 7898 3B80 0010` | the same | the same |

DX in the parent is what the parent had there when it ran the program,
with the low byte made the break state: `ended.asm` has its name at
0271h, so 0201h, and `p.com` of the scenario at 01A8h, so 0101h, in
both programs. D2 took the 02DFh, 0271h and 0260h of its three runs
for DOS's own: they were the toy's own DX as the toy was edited.

The scenario: `python3 harness/compare.py build/portable_c32/VC.EXE
ended` passes with `harness/goldens/portable_c/ended.patch`, 45
screens and the disks, both rows of registers `2222 1111 EE4F ####
0000 0202`; so does the program of 16 bits, as before. No patch of the
32-bit tree's own, and no word of `DEVIATIONS.md` for the registers:
they are the same.

The toy, `python3 harness/trial32/both.py`: 29 screens for 28 steps,
the same in both builds and for both countries, and none of its pages
is of "what is not made" any more. Its last page takes the end twice
and gives it back twice, and writes whether the PSP names another
address or the one that it had; then it takes the end once more and
ends so, with `ProgramEnd` and no `Give`. DOS then goes on in the
routine of either build, as when another program ends the toy, and the
last step types `break` at DOS's prompt: "BREAK is on" in both, where
the toy had switched it off.

**What is not the same in 32 bits, and cannot be.**

- Ctrl-Break, INT 1Bh. The program of 16 bits, like the original,
  never gives it back when DOS ends it, and it goes into memory that
  is free from then on. The extender gives back every interrupt that
  the program took through it before DOS sees the call. Making that
  the same would be to break the machine on purpose.
- Where INT 22h goes afterwards, which DOS sets from the PSP: into the
  block, where it goes into the ended program in 16 bits. Both are
  memory that DOS has freed, and DOS sets the interrupt anew when it
  next runs a program.
- What DOS is handed at that 3301h in the registers that the call does
  not read: DH, DS and the others are as they came from the program
  that started this one, which is what the original hands DOS there;
  the 16-bit portable C has DH 0 and DS on the main data. For W, with
  the rest of what entry 6 of `DEVIATIONS.md` is to say of 32 bits.

**A second copy.** Each copy has a PSP and a block of its own, with
its own routine and its own three things behind it, and neither reads
or writes the other's. In `ended` the copy that DOS ends is the second:
its routine tells DOS what it found when it started, which is what the
first copy had put back before it ran the command, on; the first copy
then notes that behind the command, as it does behind every command.
Before this piece the second copy left it off, the first noted off,
and put off back when it left: `BREAK` was off for good.

**What `ended` does not show, for W.** The scenario passes with the
16-bit program's patch, whose reason says that the second copy has no
memory to run a command. In 32 bits it has: at `"p" Space "t" Enter`
the second copy runs `t.com`, which stays in the memory, 448 bytes,
and is not released when DOS ends the copy, since the portable C keeps
no list of such programs (`DEVIATIONS.md`, 7). Seen with `mem /c`
typed behind the scenario's steps: a line `T 448` between the first
copy and its `COMMAND`, and 170K for the largest program where 364K
are free; the 16-bit program has no such line. `p.com` clears the
screen, so no screen of the scenario differs. The row of `ended` in
entry 1's table, and the patch's reason, say what is so of 16 bits
only: W has them to write for the program of 32 bits, by the user's
word that the deviation says only what still differs.

**Not tried.**

- Any DOS but the bench's FreeDOS, and the extender under a host of its
  own kind: with EMM386 or in a DOS box of Windows the extender goes
  another way into protected mode, and whether it then sees the call
  that ends the program in real mode, and takes itself out before DOS
  goes on, was not seen. The bench has neither.
- A program that DOS ends while a critical error's box is up, or
  inside a call of real mode: the routine reads nothing of the
  program, so it does what it does; what the extender does there is
  the question.

**The scenarios' test under load, a flaw of the harness.** While this
piece was tested, four other agents' suites ran on the machine, with a
load of 17 to 38. The test of the program of 32 bits then took 886 and
978 seconds where it takes 194, and each time, and in the run of
`--record` between them, it stopped `blank-timed` at its step of 1,300
ticks, which did not end within the 300 seconds that a step has: "on
the record, and fails". The program as it was before this piece, kept
as `build/p32-base/portable_c32/VC.EXE` and run beside the new one,
stopped at the same step the same way, so it is not this piece's; and
by itself, with a load of 17, the scenario passed in 103 seconds. H3
wrote that a machine's second is long while this program idles: with
the machine that busy, the 71 seconds of that step are longer than the
limit. Every other scenario of the record passed in all three runs,
`ended` among them. So the record was not written by `--record`, which
writes nothing while a scenario of the record fails: `ended` was put
into `harness/pass32.txt` by hand, at its place in the order of the
scenarios, which is the line that `--record` would have added. When
the piece landed, behind C1, the whole suite passed, fourteen tests of
fourteen, with a load of 22 to 31: `blank-timed` ended in time, and the
test held the record of 155 to be what passes, no more and no fewer.
What would mend it is not this piece's and was not tried: fewer instructions
at an idle round of the driver for a flat program, or more time for a
step that waits by `Wait*`, as a scenario that is not on the record
already has (`short_step` of `harness/ported.py`).

For whoever takes Quick execute out: `LOW_SHELL` and `LOW_KEPT` are 48
bytes of the block in front of `LOW_ENDED`; the places behind them can
move down, since `Ended_T` is written by their names.

## The portable C in 32 bits: what a trial on the bench showed

The first step of the portable C's road (`CLAUDE.md`, "The portable C")
is a 32-bit program for DOS, built by Open Watcom's `wcc386` in the flat
model with a DOS extender, and judged on the same bench. Before anyone
touched the program, on 12 October 2026, toys of a few dozen lines each
were built that way, with no C library, and run on the bench through the
harness as it is. This is what they showed, for whoever does the step.
The toys are kept in `harness/trial32/`, with a README that says what
each asks and how to run one; nothing there is built or tested by the
project. What is said here of a toy can be seen again by running it.

In short: DOS/32A runs a flat program on the bench as it is, and the
driver of the tests as it is takes its idle call and feeds it keys and
the mouse. CauseWay does not run there, for a fault of QEMU's. The one
thing the driver cannot do is read `AutoTim`, `Clock` and the stack,
which are above the megabyte: the BIOS's block move does that, and was
tried. The limit most likely to be met is the size of the file.

### The extender: DOS/32A, since CauseWay does not run in this QEMU

The pinned Open Watcom has two extenders with a clean licence,
CauseWay (`binw/cwstub.exe`, 47,152 bytes) and DOS/32A
(`binw/dos32a.exe`, 27,504 bytes), each the stub of the program's file.
DOS/4GW and PMODE/W are there too and are not open: they were neither
linked nor run, and must not be.

- CauseWay says of itself, in its banner and nowhere else in the
  snapshot, "No copyright. Public domain software. No rights retained."
  There is no licence file for it.
- DOS/32A has `binw/license.d32`: redistribution in source and binary
  form, with or without modification, provided that the copyright
  notice, the conditions and the disclaimer go with what is
  distributed, that documentation for the user says "This product uses
  DOS/32 Advanced DOS Extender technology.", and that nothing derived
  from it is called DOS/32A. So a program that is shipped with it needs
  that sentence in the README and the licence's text beside it; the
  stub itself is fetched with Open Watcom at build time and is not
  committed. Where the text goes is the user's to say.
- Open Watcom's own `license.txt` says nothing of either.

CauseWay stops every program on the bench with "Exception: 0D, Error
code: 1284" at 00EF:000000B5, at the program's first software
interrupt: a toy, and Open Watcom's own hello-world with its C library
and start-up (`hello.c`); with `cwstub.exe` and with `cw32.exe`; on a
486 and on a Pentium, with 16 MB and with 32, with an XMS driver and
without. The cause is QEMU's, 11.1.2:

- **The fault.** In a code segment whose base is not 0, while DS, ES
  and SS all have the base 0, an instruction with 32-bit addressing
  that reads memory through `cs:` reads at the same offset from 0
  instead. A processor reads through CS.
- **How it was shown without CauseWay.** `csover.c`, under DOS/32A,
  makes a code segment at its own data and runs `mov ax,cs:[18h]`
  there: it reads 12C2h, the word at linear 18h, where the segment has
  0830h at its 18h. The two forms that load a segment register the
  same way fault with that word as their error code.
- **In CauseWay** the instruction is `mov ds,cs:[0000000Ch]`, the first
  thing its entry of every interrupt does once the registers are
  pushed; the word there is the good selector 0083h (`cwtables.c`
  reads it), the word at linear 0Ch on the bench is 1285h, and the
  error code is that.
- **QEMU's own log shows it**: with `-d int -D FILE` (`run.py --qemu`)
  the fault is the line `v=0d e=1284 ... IP=00ef:000000b5`, with the
  registers.
- A flat program has CS at 0 and is not touched by it. DOS/32A is not
  either, through everything below; that its own code is of 16 bits,
  where QEMU always adds the segment, is a guess at why, and where the
  fault's edge lies was not looked for.

So CauseWay cannot be judged on this bench until QEMU is another, and
what is said of it anywhere is from its manual (`docs/cw.pdf`,
`binw/cw.ihp`), not from a run.

### What the bench has, and where the program lands

`rmbench.asm`, a program of real mode run there: no XMS (INT 2Fh with
4300h comes back as it went), no DPMI (1687h the same), no EMS (INT 67h
is the BIOS's, F000:FF53, and `FDCONFIG.SYS` loads no driver), real
mode, 15,232 KB above the megabyte by INT 15h function 88h and 639 KB
below.

DOS/32A therefore runs raw: it says so (INT 31h with EEFFh answers
"D32A", kind of system 0, version 090Ch), takes the memory from 100000h
itself and keeps the line A20 on. The program runs at privilege level
0, with CR0 00000011h: no paging, so a wild write is caught by nothing,
as under DOS today. CS, DS and SS are flat, ES is the PSP's selector
where the program is entered, and FS a second flat one.

| What | Where, on the bench |
|---|---|
| The PSP, and DOS/32A's part of real mode behind it, one block of DOS's named for the program | segment 25E6, 0C9Fh paragraphs: 51,696 bytes |
| The first object of the file, the code | 00100010h |
| The second, the data | behind the code, at its end rounded up to 16, and 16 more |
| Blocks of DOS's that the program asks for (DPMI 0100h) | from segment 3286 on, right behind that block |
| The same with an XMS driver loaded, which is not the bench | code at 00127C10h, kind of system 1 |

The objects are packed, not put on pages: the first of the data shares
a page of 4K with the last of the code. What was not in the file was
all zeros when the program was entered (`zero.c`); whether DOS/32A
clears it or the bench's memory had simply never been written was not
told apart.

What starting costs, by the machine's ACPI timer between the entry of
a program of real mode and that of the toy (`rmstart.asm`, `start.c`):

| The file | Machine time | Instructions |
|---|---|---|
| 32 KB: the stub and a toy | 83 ms | 324,000 |
| with 180,000 bytes more, and no address in them | 358 ms | 1.4 million |
| 393 KB: 100,000 bytes and 20,000 relocations | 896 ms | 3.5 million |

About 1.5 ms for each KB of the file and 50 instructions for a
relocation: some 0.4 s for the program, before a first screen at ten
seconds. What DOS takes to load today's `VC.EXE` was not measured.

DOS/32A sets the timer's counter 0 anew as it starts, to mode 3 where
the BIOS leaves mode 2 (status 36h for 34h, `timer.c`): the tick's
phase begins again once, the same in every run. A count read from that
counter is no place in the tick in mode 3; the toys measure by the
ACPI timer, port 608h, 3,579,545 counts to a second.

### Building one, with no C library

`harness/trial32/build.sh` is the whole of it. What had to be found
out:

- `binl64/wlink.lnk`, the linker's file of systems for Linux, has
  neither `causeway` nor `dos32a`; `binw/wlink.lnk` has. The linker
  warns of an "undefined system name" and goes on without what the
  system would have said. Say `format os2 le` and `option stub=` with
  the stub's path.
- The compiler: `wcc386 -mf -zl -zls -s`: flat, no library asked for,
  and no check of the stack, which would call one. Division and
  multiplication of 32 bits need no helper.
- The entry is a function named to the linker, `option start=Start_`.
  The extender enters it on the stack that the file's header names,
  with interrupts on. `Start` of `lib.c` is all a start-up has to do:
  put ES on the data, clear the direction flag, go on.
- The linker wants a segment of the class STACK, or it warns, the
  header names no stack, and neither extender gets the program going.
  That is `#pragma data_seg("STACK", "STACK")` with 16 bytes that have
  a value, and `option stack=16k`, which makes the segment that long
  without putting it in the file.
- The linker's command needs `order clname CODE clname DATA clname
  STACK clname BSS`. Without it the stack is behind what has no value,
  and all of that is in the file as zeros: 232,840 bytes for a toy
  that is 32,404 with it. With the main data among what has no value,
  that is 62 KB of the file.
- Both extenders write a banner when they start. DOS/32A's goes with
  bit 3 of the byte 15h behind the letters `ID32` in the stub cleared
  (`quiet.py`; found by clearing each bit of its settings in turn,
  since the snapshot does not describe them). It then writes nothing
  and leaves the cursor at 0,0. The same settings seem to have the
  number and the length of its stacks at the bytes 0Ah to 0Fh, 08h,
  08h, 0020h and 0020h: eight of 20h paragraphs for real mode, which
  fits the 512 bytes seen below, and as many for protected mode. That
  reading was not tried by changing them.
- DOS/32A writes "warning (9003): real mode interrupt vector has been
  modified" as the program ends, for every vector that is not as it
  was, on the last screen of a scenario. The program gives back what
  it took: the vectors of 1Bh and 24h set with DPMI 0205h went quiet
  that way (`errors.c`). And a command run with INT 21h function 4Bh
  leaves INT 22h of real mode changed, which DOS/32A warns of too:
  put back as it was at the start, with DPMI 0201h, it does not
  (`command.c`, the keys v and r).

### The idle call and the keys

An `int 28h` in protected mode reaches real mode like this, as a copy
of the driver that writes it out saw it (`DRIVERX.ASM`, `run.py
--debug`):

| | The toy under DOS/32A | The 16-bit portable C |
|---|---|---|
| Comes from | 25FC:176C, DOS/32A's code in the program's own block, whose PSP is 25E6 and whose name is the program's | 42B2:32DC |
| DS | 0000, and ES too | 25F6, on the main data |
| SS:SP | 30C7:01FA, a stack of 512 bytes of the extender's | 4F4B:0F6C, the program's |
| Flags | 3012h: interrupts off | 0246h |

The driver as it is takes the call for the program's: it comes from
the memory of the program that DOS has as the current one, and that
has the script's name. It ended every step after two rounds and wrote
the record of the stack, which it writes for the program's call only.
Of that stack of 512 bytes its own work, DOS's and the BIOS's used 68
at most, 82 with the block move below.

Through the extender, by INT 16h functions 11h, 10h and 02h and INT
33h function 3, the toy got what the driver put in through the
keyboard controller, as the BIOS makes it (`keys.c`, `wait.c`): a, F5,
Shift+F6 (5900h, shift 02h), Alt+F1 (6800h, 08h), Ctrl+o (180Fh, 04h),
Up, Ins, a text typed, Cyrillic letters on the Russian bench (1EE4h),
`Click@3,5` and `RightClick@40,12` with the driver counting the asks,
`Wait*18`, and a key waited for inside the BIOS. Nothing has to change
for the idle call or the keys.

What does: DS is 0 there, so the driver's read of `AutoTim` at
DS:[offset] reads the table of interrupt vectors. A flat program must
not be given the offsets of the 16-bit one.

### What the driver reads: the BIOS's block move

`AutoTim`, `Clock` and the stack are in the program's data, above the
megabyte. Three ways:

| Way | What it costs |
|---|---|
| The driver reads there with INT 15h function 87h, at linear addresses that the harness gives it | The driver and the harness change, the program does not. Some 1,400 instructions more for each idle round: a round of the toy was 1,500 without and 2,900 with |
| The two variables in a block of DOS's memory | The program is changed for the tests' sake, and the driver still has to find the block |
| The idle call made by DPMI 0300h with DS of real mode on a copy of the main data below the megabyte | No change to the driver; 62 KB of conventional memory, and a program arranged around the tests |

The first was tried and is the one to take: nothing in the program
tells the tests where it is. `auto.c` keeps the two variables in its
data; with the driver as it is the step that sets `AutoTim` ends at
once, and with `DRIVERX.COM` told the two addresses it ends when the
toy has done its thing nine ticks later, and a step with `Clock` set
takes the 20 ticks of watching what blinks. That run made 2,124 block
moves, and the extender went on unharmed: it is in real mode proper
while the driver runs, and sets its own tables again on the way back.

The addresses are the linker's map, object and offset, and where the
extender puts the objects, which was the same in every run (the table
above). `coverage.py`'s vote already finds where the code is from a
trace, so one traced run of a program on a bench can say it instead of
the rule being trusted.

### DOS's services under DOS/32A

`doscalls.c` made them from protected mode with every address above
the megabyte; `realcalls.c` made those that failed in real mode, by
DPMI 0300h with a buffer from DPMI 0100h.

| | Services | What was seen |
|---|---|---|
| Translated | 09h, 1Ah, 39h, 3Ah, 3Bh, 3Ch, 3Dh, 3Fh, 40h, 41h, 4300h, 4301h, 47h, 4Eh, 4Fh, 56h; 2Fh and 35h in what they answer | They work. 3Fh and 40h took 12000h bytes at one call, read back without a wrong byte. 4Eh and 4Fh fill a DTA above the megabyte, and 2Fh answers its address there. 35h answers the vector of protected mode |
| No address | 06h, 0Bh, 19h, 2Ch, 30h, 3306h, 36h, 3Eh, 42h, 4408h, 4409h, 440Eh, 4Ch, 4Dh, 5700h, 59h, 62h | They work. 62h answers the PSP's selector, and by what it costs is not passed on to DOS at all |
| Not translated | 13h, 29h, 38h, 60h | Wrong from protected mode, and none says so in a way to go by: 60h answers error 2, 29h leaves the block empty, and 38h answers without an error and fills nothing in above the megabyte. Where its 34 bytes went was not looked at; with DS 0 in real mode, as at the idle call, that is low memory. All four work in real mode with a low buffer |
| 48h | | Gives a block of DOS's memory and answers a selector, not a segment: 0828h for a block at segment 3286. BX FFFFh answers error 8 and the largest, 6CF9h paragraphs |
| 4Bh | | In protected mode the name is translated and the program runs, but the block of parameters as DOS/4GW has it, which was a guess, did not deliver the tail: COMMAND.COM came up for keys. By DPMI 0300h, with the name, the tail and the block in a low block, it works |
| INT 2Eh | | A stretch of 28 bytes of real mode in a low block, `int 2Eh` and the stack put back behind it, called with DPMI 0301h, comes back; COMMAND.COM answered AX FFFFh and ran nothing. One try, with the command as a count, the text and a carriage return; why was not looked for |
| Not run | 0Eh, 25h, 3300h, 3301h, 4400h, 440Fh, 49h, 4Ah, 5701h, the calls for a label by a file control block | Of these 0Eh, 3300h, 3301h, 4400h, 440Fh and 5701h take no address. The vectors were set by DPMI 0205h, not by 25h |

So for `dos.c`: files, directories and the search of a directory go
straight through, with the program's own pointers. What goes by a file
control block, 29h, 38h, 60h, 4Bh, and whatever answers an address of
real mode, as 38h does for DOS's routine for capitals (00D8:12EA), go
through one block below the megabyte and DPMI 0300h, or 0301h for a
routine. CauseWay's manual lists much the same calls as translated and
those by a file control block as not supported.

The BIOS and the mouse from flat code (`realcalls.c`): writing the
screen at B8000h and reading the BIOS's data at 400h and on work as
they are. INT 10h functions 01h, 02h, 03h, 08h, 09h, 0Fh and 12h, INT
33h functions 0, 1, 2, 3, 4 and 0Ah, and INT 2Fh with 150Bh work
directly. INT 10h with 1130h answers CX and DL directly, and its
ES:BP only to a call in real mode (C000:7260). Not run: INT 10h 06h,
07h, 0Eh and 1112h, INT 33h 7 and 8; none takes an address.

What a call costs, in instructions of the machine, of 64 in a row
(`rmcalls.asm` for real mode, `realcalls.c`):

| Call | From real mode | From the 32-bit toy |
|---|---|---|
| INT 16h, 11h | 194 | 367 |
| INT 16h, 02h | 170 | 343 |
| INT 10h, 03h | 107 | 301 |
| INT 10h, 0Fh | 100 | 301 |
| INT 33h, 3 | 46 | 243 |
| INT 21h, 2Ch | 1,107 | 1,386 |
| INT 21h, 19h | 604 | 883 |
| INT 21h, 0Bh | 1,018 | 1,297 |
| INT 21h, 62h | 78 | 136 |
| INT 31h, 0006h | | 85 |
| INT 10h, 03h by DPMI 0300h | | 683 |

There and back is 175 to 200 instructions for the BIOS and 280 for
DOS; a call made by DPMI 0300h is 380 more than one made directly.

### The critical error and Ctrl-Break

`errors.c` sets handlers of protected mode for 24h, 23h and 1Bh with
DPMI 0205h; DOS/32A puts callbacks of its own into the vectors of real
mode (31E7:004B and so on).

| Interrupt | Made by | What was seen |
|---|---|---|
| 24h | a file made on A:, the diskette that the bench boots from, which cannot be written | The handler was called once, with AX 3900h, DI 0 and BP:SI 0070:0606, the device's header as an address of real mode, on a stack of the extender's, 0018:00031E32. It answered AL 3, and DOS came back with the carry flag, AX 5 and the extended error 13h |
| 1Bh | Ctrl+Break | The handler was called. The harness has no name for that key: `run.py` adds Break, E0 46 |
| 23h | Ctrl+c, with the keys left to DOS | The handler was called, and DOS wrote `^C` |

Not tried: the phantom drive of the scenario `not-ready`, which needs
the program's own dialogue; the answers Retry and Ignore; and a box
drawn from inside the handler. The handler is entered on a stack of
the extender's whose length was not measured, and the settings have
the stacks of protected mode at 20h like those of real mode, which are
512 bytes: it should go over to the program's own stack before any
compiled code runs.

### Running a command

`command.c`, with 4Bh made in real mode:

- `COMMAND.COM /C` ran a command and came back, with the exit code by
  4Dh.
- DOS's largest free block while the toy runs is 446,352 bytes: the
  extender's 51,696 bytes below the megabyte are all that the program
  keeps there, with what it asks for itself.
- A second copy of the toy ran inside a command of the first: its
  code right behind the first copy's above the megabyte, with some
  321 KB of DOS's memory free in it. It ended with 7, which came back,
  and the first copy went on.
- INT 22h has to be put back: see "Building one".

### Memory

`memory.c`: DPMI 0500h says 00EDB650h bytes are free, 15.58 MB, with
the toy loaded; DPMI 0501h gives blocks at linear addresses that flat
code uses as they are, 001049B0h and on, right behind the program. INT
21h function 48h is DOS's conventional memory behind a selector, as
above; 49h and 4Ah were not run.
What the panels are today, one block of DOS's cut up by segment
arithmetic, is ordinary memory then: arrays among what has no value,
or one block of 0501h, with pointers where the segment numbers are;
the three functions of `sys.h` for memory answer in addresses.

### What breaks in the harness, and the changes in order

| File | What breaks with an LE file and a flat program |
|---|---|
| `coverage.py` | It takes DOS/32A's own code for the program's, 2,404 instructions of it in a run of `keys.c`, and none of the toy's: a place that is not on a paragraph by the MZ header is dropped. Of the toy's 65 blocks that ran, 22 hold a relocated address: the file has the target's offset in its object there, the memory that and where the object is |
| `driver.py`, `facts` | `_md` at 0000:0000 of the map, and the stack by the SS of the MZ header, mean nothing. `link_map` reads offsets of four digits, and the map has eight |
| `DRIVER.ASM` | Reads `AutoTim` and `Clock` through DS, and writes the stack out from the segment that SS is on: see above |
| The scenarios' test, `ported.py` | The length of the file: below. The stack that stays half empty: it is above the megabyte, and what it holds when loaded is zeros if it is not in the file. No code segment beyond F000h: nothing of the kind in a flat program. No code within 4K of the stack: the data begins in the code's last page, so what comes first of the data should be what is not written, the tables, as today |
| `datacheck.py`, `reads.py` | They read the MZ file and the 16-bit map; and a word that names a text is a pointer of four bytes in a flat program, or stays an offset from where the tables begin, which is the port's to decide |
| `tools/figures.py` | The segments, the relocations of the MZ header and the limits that it reports |

The changes, in the order to make them:

| | What | How much |
|---|---|---|
| 1 | The build: `wcc386` and `wlink` as `build.sh` has them, with DOS/32A as the stub and the banner bit cleared by a tool of ten lines | 50 lines of `CMakeLists.txt`; half a day |
| 2 | A reader of LE files for the harness: the header, the objects, the map of pages, the relocations; and of the 32-bit map | 100 to 150 lines; half a day to a day |
| 3 | `driver.py`: the facts of a flat program as linear addresses, by the map and one traced run, and a second form of the script's header with fields of 32 bits | 100 lines; half a day |
| 4 | `DRIVER.ASM`: the block move for `AutoTim`, `Clock` and the stack, where the header says so. The copy of the trial has the first two, in 1,526 of the file's 2,048 bytes with its record for the trial in. Then the goldens derived again: the driver's length and what it keeps of the memory must stay, and a golden that changes stops the work (`CLAUDE.md`, "Work by several agents") | 70 lines; half a day |
| 5 | `coverage.py` and `ported.py` on that reader, with the stub left out | half a day |
| 6 | The scenarios' test: the stack by the new record, the limit of F000h dropped for this tree, the rule of the page in its new form, and the length of the file as the user decides | half a day |
| 7 | `datacheck.py`, `reads.py` and `tools/figures.py`, once the port has decided how a table names a text | a day or more |

### The length of the file

When this was tried, the scenarios' test failed a file longer than
three times the original's 65,090 bytes: 195,270. It holds the portable
C to no multiple of that since, on the user's word of 12 October 2026:
see 5 of what is decided in "The setup program in the portable C: the
plan". `wcc386` was given the tree as it
was that day, with `-i` for the compiler's own headers, which `vc.h`
includes `stddef.h` from: it took 40 of the 55 files, and those link to
106,853 bytes of code, where the 16-bit build of the same day had
97,039, with 3,319 relocations in the file, 23,509 bytes of it: 3,175
of 7 bytes, 141 of 9, for a target beyond 64K of its object, and 3 of
5. A relocation is every place where code or a table names something
by its address: the calls are not.

It refused 15 files:

| Files | Why |
|---|---|
| `dos.c` | Its inline assembly has registers of 16 bits for what a function returns and changes: "invalid return register in #pragma", "Illegal register modified" |
| `arith.c` | The same, for the two routines that a 32-bit program does not need |
| `dialog.c`, and the twelve files of tables that include `tables.h`: `bars_t.c`, `commands_t.c`, `copy_t.c`, `dirfunc_t.c`, `edit_t.c`, `find_t.c`, `help_t.c`, `keys_t.c`, `main_t.c`, `system_t.c`, `treebox_t.c`, `view_t.c` | `LAID_OUT(BOXHD, 10)` and `LAID_OUT(BOXMN, 11)`: the header of a box and a field of a dialog hold pointers, and are not as long as the original has them when a pointer is four bytes |

That a file compiles says nothing of whether it works: a panel is
still a segment's number there, and memory is cut up by segments.

With the stub's 27.5 KB, the tables, `dos.c`, the relocations of both
and the main data out of the file, the program comes to somewhere
between 185 and 200 KB: at what the limit was. The README's figures
have what the file is today.

### What was not tried

- Anything of CauseWay beyond its fault.
- The services marked as not run above; 4Bh in protected mode with
  another block; INT 2Eh beyond one try.
- At the critical error: Retry and Ignore, the phantom drive, a box
  drawn from inside the handler, and how long the extender's stack is
  there.
- The program itself, or any file of it, running in 32 bits; so also
  how its scenarios that show the time of day fare, with a start some
  0.4 s later and 175 to 280 instructions more for every call of the
  system.
- What DOS takes to load today's `VC.EXE`, to hold the start against.
- DOS/32A with an XMS driver, beyond seeing it start there.
- Whether the line A20, which DOS/32A leaves on, troubles anything
  that a command runs.

## An implementation of the layer for tests: future work

The road of the portable C (`CLAUDE.md`, "The portable C") has a
platform abstraction layer designed at its step 3, the implementation
for DOS moved under it at step 4, and at step 5 a second implementation
that builds and is not meant to run, a stub. An implementation of the
layer made for tests would be a third: one that does run, on this
computer, built by clang, with the common code of the program above it,
and that answers the program where a system would. It is future work,
on the user's word of 13 October 2026, with everything that it would
give written down here. It is not started, and is not to be started
without the user. The stub of step 5 stays a stub: making the
implementation for tests in its place was proposed, and not taken.

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
   | A drive that is not ready, the critical error | 19 | reached, if the layer has the critical error as a call back into the program: see below |
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
   anything. Those functions need the layer for nothing but to be
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
   | BX and DX going round the main loop from one command to the next: Rename behind Down offered a name that the original does not, and no scenario had the two keys one behind the other | Nothing on this computer. Sequences of keys made at random, on the bench, against the original: the first of the two things that need no layer, below |
   | Texts read behind their end: `CrLf_S` with a zero byte too many, `User_S` copied as 80 bytes | The address sanitizer, in the first scenario that reads them |
   | The table of the blanker's stars, three bytes wrong where no screen shows | Not this: `c-data` holds it now. Mutation testing would have said that no screen watches the blanker |
   | A `name` that was never set, so DOS was handed what the stack held, in `SetStat` and at the entry | The memory sanitizer, in a build of 64 bits |
   | CX as `T75_00` leaves it: the editor's count of free bytes wrong, a figure that is not compared | A property test of the editor's buffer: the count falls by one for every character typed, which "Known and not dealt with" asks for |

### What it has to be

The rule of "Scenarios" in `CLAUDE.md` is that a test puts no stand-in
where something is that the project does not own, and that what a test
knows of DOS, the BIOS and the machine it gets from the real ones. The
layer is the project's. What a function of it promises is a statement
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

> The layer is the project's, and an implementation of it made for
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
  numbers widened, a routine written anew above the layer, what is
  drawn into the program's own buffer. Any change to what the program
  asks of the system, one call more or two in another order, ends the
  trace of every scenario that passes there, and those are recorded
  again on the bench, from a run that has passed against the
  original's golden. The same holds, for all of them at once, of a
  change to the layer itself, so no trace is worth recording before
  the design of step 3 has settled. Two things keep this from being a
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

### What it asks of the layer's design

The design of step 3 is made before any of this is built, and is not to
close the door on it. What of today's `sys.h` helps (read):

- 84 functions, each one thing asked, with no register in them.
- A failure is a value that comes back, `struct Answer`, so an
  implementation can be told to give it.
- The screen is cells put and got, `ScreenPut` and `ScreenGet`, and the
  keys are `KeyWaiting` and `KeyRead`: a test reads the one and fills
  the other.
- The timer and the time of day, memory, the version, the country,
  the display and the mouse are all asked there, so a test can have
  each be otherwise.
- `Idle` is a call of its own. It is where `harness/DRIVER.ASM` knows
  that a step is over, and where an implementation for tests would.
- `dos.c` decides next to nothing: 5 conditional branches of the 3,302
  counted (IR). An implementation without decisions needs no coverage,
  and the bench is enough to judge it.

And what hurts:

- A decision of the program's under the seam: `AskEs` of `dos.c` makes
  the error 53h of a call that the user gave up, asks DOS for its
  extended error, and reads `md.ErrDat`. Every other implementation
  would have to do the same again.
- What a system enters of the program is outside the header: `FatErr`
  of `main.c` is an `__interrupt` function that `SetVector` hooks, and
  `resident.c` has the entries. With the critical error not a call
  back that the layer names, the build of this computer does not link,
  and the 19 instructions of a drive that is not ready stay out of
  reach.
- What DOS happens to do, in what the functions take and answer: some
  25 of the 84 have such a thing (read, and approximate). `value` as
  DOS left AX where nothing failed, `flagged`, what `FileSeek` answers
  when it fails, the five functions with Bare in their names, the page
  and the colour of `ScreenType`, `FcbParse` and `FcbDelete`,
  `ScreenMemory` and `ScreenPageStart`, `ProgramPrefix`, which has left
  the header since, memory by the segment. Each is one thing more that every implementation and every
  trace has to do as DOS does. Taking one out is a deviation, to be
  asked.
- What the system keeps between two calls and no call names: the
  current drive and directory, the record of a search, `FindRecord`,
  how often the pointer was hidden.
- Nothing that a test can call to start the program: it is entered in
  `resident.c`, as DOS enters it, and `ProgramEnd` does not return.

So, of the design:

- Low and narrow, as `sys.h` is. A layer that knows more, a file system
  or a console, takes logic under it, where only the bench sees it and
  every implementation writes it again; and replay rests on a trace of
  small questions being all that the program knew.
- An implementation that decides nothing; what the program decides
  about an answer is above the layer.
- The layer reads nothing of the program's data, and the program
  nothing of the machine's.
- What a system enters of the program, the critical error and
  Ctrl-Break, as call backs that the layer names and the program hands
  it.
- Every answer said in full by its function, and nothing "as DOS left
  it" that a caller goes on with unsaid.
- The shapes of DOS that the screens show, a name of eight and three,
  the bits of the attributes, a date and a time packed, the number of
  an error, as the layer's own, written in its header: another system
  answers in them.
- Numbers of a width that is said, so that what is recorded of a
  program of 32 bits can be answered to one of 64.
- No waiting but through the layer: time is what the layer says it
  is.
- Memory asked of the layer in blocks that are pointers, so that on
  this computer each is a block of its own and the sanitizer sees one
  panel run into the next.
- One call that says whether the program has anything left that it
  will do by itself, where the driver reads `AutoTim` and `Clock` at
  their addresses today.
- All that is kept, by the program and by the layer, set anew by one
  call; and the program started by a function that a test can call,
  and ended so that the test goes on.
- One implementation to a program, chosen when it is linked, `_dos.c`,
  `_posix.c` and one for tests beside them, as the road has it. What
  tells that one to fail is its own header, and no part of the layer.

### Open, for the user

- Whether a build of the program for DOS that writes out what it asks
  and what it is answered may exist, to derive traces from, where
  "Scenarios" has that no build is made for the tests. It would judge
  nothing: the build that is shipped is the one that is held against
  the goldens.
- Whether the rule gets the wording above, or another, before any of
  this is built.
- Two things need no layer and could be done now. Neither is started:
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

## A deviation held by a patch: done, and what comes behind it

Done: what the portable C is expected to show where a deviation changes
a scenario is the original's golden with the declared difference made
to it when it is read (`CLAUDE.md`, "The portable C"; the top of
`harness/goldens.py` for the form of a patch, and the notes of
`CLAUDE.md` for how one is made and read). The sixteen goldens of the
tree's own are gone: twelve are patches, 293 rows where the files had
9,050, and four needed nothing but their steps left out. `changed` of
`deviations.TREES` is gone with them: a scenario has a patch if the
file is there, and the header of the patch has the entry and the
reason.

What the next pieces meet:

- The bench's right panel, with every golden derived again: done, and
  the next section says how. Every patch
  on a row that comes out otherwise then fails before anything is run,
  "does not fit ... Nothing was run", with both rows and the cells
  marked. `python3 harness/goldens.py patch --tree portable_c NAME...`
  makes each again from a run, keeps its header, and says which rows
  expect the same in the same cells on a row of the original that is
  otherwise now and which say something else, are new or are gone:
  the second kind is what has to be read. `start-memory`, `lost` and
  the menus over the right panel will have such rows, since what they
  uncover or show of that panel is another thing then.
- The program's name, rules of the portable C alone: done as far as
  the next section says, with a second kind of rule, `Part` of
  `harness/deviations.py`.
- A rule that leaves cells out for one tree, should that be built (the
  next section): the comment at `rules` of `deviations.Declared` says
  how it would be written.

What no scenario has yet, and was tried by hand only: a patch with a
line for a cell that blinks. It was made to a golden and held against a
run in a trial; no committed patch has one, since no deviation so far
changes what blinks. Lines for the disks, which were tried in the same
way, two patches have since: `setup-config` and `setup-screen`, for the
setup file that each saves.

## The program's name in the portable C: done

Decided by the user on 13 October 2026 (`CLAUDE.md`, "The portable C";
`src/portable_c/DEVIATIONS.md`, 10 and 11). Done: the title, the banner,
the name without a version in the box of F10, the line that asks for
Enter, and the top of the Info panel. `main_t.c` has the two texts as
macros, `NAME_VERSION` and `BASED_ON`, and of them `Title_S`, the help's
title and the banner's first line, `NameVer_S`, the Info's title, and
`BasedOn_S`, the line below it.

The Info panel. Its two lines on the memory are gone, as something that
only made sense on DOS (`DEVIATIONS.md`, 11), and its top is: the title;
"based on Volkov Commander 4.05", centred; the line across; an empty
row; and the label of the drive on the row where it was. `Panel` of
`panel.c` writes the first three and `PutInf` of `info.c` begins at the
label. The word of the main data that held where the memory ends,
`Memorys`, keeps its place and is neither set nor read.

How it is held. Four rules of `TREES["portable_c"]`, each a `Part`: the
part as the original shows it and as the portable C does, made to a
screen of the golden wherever twelve characters of the original's show
in their places, cell for cell, with what covers the part left as the
golden has it. Three are of one text each, the banner's two lines and
the help's title. The fourth is the top of the Info panel, four rows of
40 cells with their colours, known in the shadow of a box and with its
first rows off the top of the screen. The notes of `CLAUDE.md` have what
making them met. The test says how many screens each rule expects
otherwise. Thirteen scenarios have rows in a patch for it: the box of
F10, which is two cells narrower and uncovers what no rule can know, in
eight; the line that asks for Enter, in one of those; four where a rule
is wrong of a cell or does not know the text; and `move`, where a row
of the Info is left twice on a screen that a beep has moved up.
`DEVIATIONS.md` lists them.

What is left out of the comparison. The count of free memory in the
Info is left out for the C translation alone now, `IGNORED_OF` of
`harness/deviations.py`: the portable C shows no such figure, and the
rule expects every cell of the row empty. `IGNORED`, which holds for
every tree, still has three patterns of the map of the memory, which
the portable C has not, and one of a line that only the portable C
prints, of `ended`: each is of one tree as well, and could move to
`IGNORED_OF` in the same way. That was not done with this, since it
changes what the comparison leaves out and was not asked.

## The bench with a list of files on the right: done, and what was put off

Done, on the user's word of 13 October 2026 ("Scenarios" in `CLAUDE.md`):
the bench makes the panel on the right one of files, in the original's
own Right menu, before the original saves the setup that every scenario
starts from (`harness/setups.py`). The setup file is the one of before
but for two bytes: 05Bh, the kind of the right panel, 02h and now 00h,
and the low byte of the sum. Every golden of the original was derived
again. Nothing under `src/` changed for it.

How it was shown that nothing a scenario was written for is lost. Each
scenario was held against its golden of before, screen by screen, and
what ran in it against what ran before:

- 125 scenarios are as they were: they say what they said, without the
  Info panel in the picture. All 34 of the setup program are among them.
- 22 had a step that was there for another reason and is otherwise
  now. For 21 it is one: a `Ctrl+l` that was written to take the Info
  panel away, and brought it. Those steps are gone. The other is
  `russian`, which empties the line of F5's dialog before it types.
- 20 leaned on the Info panel, and switch it on now, `Ctrl+l`, as their
  first step or at the step that needs it: `memory`, `dirinfo`,
  `view-window`, `reread`, `menu`, `compare`, `move`, `rename-keys`,
  `copy-ask`, `copy-big`, `mouse-menu` at the start; `cursor`,
  `user-menu-run`, `command-line`, `enter`, `menu-items`,
  `make-directory`, `copy-tree`, `mouse-list`, `mouse-edit` at a step.

For each of the 42 the screens behind the mend are those of the golden
of before, every cell, the cursor and what blinks, and so are the disks
but for the setup file; for `russian` they are so but for the list of
files on the right. That is in the history:

    diff <(git show 59252a7:harness/goldens/NAME.txt | sed 's/^== [0-9]*: /== /') \
         <(sed 's/^== [0-9]*: /== /' harness/goldens/NAME.txt)

shows the screens in front of the mend, the step that is new or gone,
and nothing behind it but the line of `D:\VC\VC.INI` among the disks.
And what ran: with the bench changed and no scenario mended, 73
instructions of the original were run by none, in fifteen routines, and
1,754 more only by steps that had gone astray. With the scenarios
mended, every instruction that ran before runs, and none that did not:
the report of every batch is what it was, line for line, and no entry of
`unreached` was added or taken away.

What the change found of the two C trees. With the bench changed and no
scenario mended, both were taken along paths with a list of files on
the right that they had never been taken along. Nothing differed from
the original but by a declared deviation or a figure that is known to
follow the size of the program, each time where a step had gone astray:
the Info panel on D:, the Info in both panels with its second "Bytes
Free", the Info of a second copy, and the Commands menu, a line shorter
in the portable C, left open.

What was found of driving the program is at the top of
`harness/scenarios.py`: what F5 and F6 offer next to a list of files,
where the tree is next to one, Compare directories, `Ctrl+]`, a name
typed behind `Alt+RightShift`, and the time. The first screen is 4.7
seconds of the machine's time sooner, some five seconds past noon: that
is what the Info panel takes the first time it is drawn, and a scenario
that switches it on has it then.

The patches of the portable C. `clock`, `clock-russia`,
`mouse-drag-menu`, `start-memory` and `lost` were made again and say
what they said, on rows of the original that are otherwise now: the
shadow of a menu on the line of a list where it fell on a blank row of
the Info, the panels that the portable C still shows with the list on
the right, and, in `start-memory`, six rows fewer, the Info of the
second copy, which is on no screen now. Those of `menu`, `mouse-menu`,
`tree-choose` and `second-copy` say what they said row for row, on
screens whose steps have other numbers; those of `setup-config` and
`setup-screen` have the other setup file among the disks.

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

## Leaving a part of the screen out of the comparison: future work

Recorded on the user's word of 13 October 2026, as an idea for later. It
is not built, and nothing needs it today.

What it is. A golden stays what it is, every cell of every screen that
the original showed: it is the only witness of the original, and
recording the whole screen costs nothing. What changes is the
comparison, for one tree: a named part of the screen is left out of it,
"what the Info panel shows", "what DOS had shown behind the panels",
where that part cannot be what the original's is and is on many screens
that are about something else. It is the same thing that
`harness/deviations.py` does today for single figures, the count of free
memory and the clock, made larger. It is not a golden that records
less.

What raised it. Giving the portable C its own name touched the top rows
of the Info panel, which the distribution has on the right, on some
2,000 screens of scenarios that have nothing to do with it. That case
was settled otherwise: the bench now starts every scenario with a panel
of files on the right ("Scenarios" in `CLAUDE.md`), which takes the
part off those screens and leaves every cell that remains compared.
Where a part can be taken off the screen by how the bench is set up,
that is the better way.

Where it will be wanted. Where a part cannot be configured away: above
all the first build that is not for DOS, in which what DOS had shown
behind the panels is not there at all, and whatever of the Info panel a
system does not have, a label of a drive, the size of a disk as DOS
counts it, differs wherever the panel shows.

What it has to be, if it is built:

- Declared, for one tree, by the name of the part and the reason, in
  `src/portable_c/DEVIATIONS.md` like any other difference.
- By the part, not by a rectangle. A menu or a box drawn over the part
  is not left out with it: only the cells that show the part are. That
  takes knowing which cells those are on a screen where something
  covers some of them; one way is to leave out a cell only where the
  golden has, at that place, what the part has there when nothing covers
  it.
- Every part that is left out is still compared whole in at least one
  scenario that is about it, and the test says so if none is.

What it costs. The comparison of whole screens notices harm in places
that nobody was looking at: a count that was wrong in the editor showed
only in a status line drawn elsewhere on the screen (the note on
`T75_00` in `CLAUDE.md`). Inside a part that is left out, that is given
up. So a part is left out when nothing else will do, and as small as
will do.

For whoever designs it: the rules of `harness/deviations.py` that
belong to one tree should be able to say "these cells are not compared"
as well as "expect this in their place".

## The last of the assembly

Done: the C translation has no assembly file, and its build runs no
assembler for it. The assembler that the build fetches, JWasm, builds the
original from `src/asm/`, which the tests hold the translation against and
hold against the shipped binary, and the driver of the tests. This is a
short account of how the rest went, for whoever reads the history.

When every routine was C, `src/c/asm/` still held the data and the glue.
It went in this order:

1. The texts, boxes, dialogs, buttons and colours, a file of the
   assembly at a time, into files of constants, `src/c/*_t.c`, by the
   recipe in `CLAUDE.md`, "Moving the data of a file to C": several
   agents side by side, each with one file. The test `c-data` held every
   object against the original's bytes while it moved (below).
2. The words of a dialog that name a routine, which became numbers that
   `Dialog` goes by.
3. The commands and the main loop. A command was a label that the loop
   jumped to and that jumped back; each is a function that the loop
   calls from a `switch`, and the tables of the menus hold commands by
   their numbers. With the stubs went ten files. `Stay` of
   `src/c/resident.c` calls the rest of the program as one function,
   `TrnMain` of `src/c/main.c`.
4. The data and the stack of the part that stays in the memory, which
   are of `src/c/resident.c`; the word with the main data's segment,
   `SecDat`, a variable of `src/c/main_t.c`; and last what the one file
   left, `VC.ASM`, still said without an instruction or a byte of data:
   how the program is laid out. That is said by two options of the
   compiler and by the linker's command, and held by `tools/mkexe.c`:
   the README has it under "The C translation", and each place that rests on
   the compiler under "What rests on Open Watcom". The layout of the
   main data that the file had, its `STRUC`, nothing read any more:
   `struct MainDat` of `src/c/vc.h` is the only one.

What the last step found, beyond what the README says:

- The group of the constants, `DGROUP`, does not begin at the PSP any
  more, since nothing of C can hold it there: the first bytes of the
  file are of the other group. A word of a box that names a text is an
  offset in that group, whatever it begins at, and `AsmSeg` is its
  segment. Three places took the group to begin at the PSP: `Init11` of
  `src/c/start.c`, which worked out the length of a block from the
  segment of the constants; `Stay`, which read a far address in a
  constant at its offset in the segment `RunPar`; and `Alloc0`, which
  read a text of the rest through DS.
- The part that stays names something of the part that goes by its
  segment as the linker has it, to which it adds where that part is:
  `TRN_SEG` of `src/c/resident.c`. `tools/mkexe.c` keeps such a word out
  of the table of relocations.
- `md` is still an `extern` object, named to the linker as `Psp`. A
  layout at the number 0 does not work with this compiler, and neither
  does one based on the data segment: `CLAUDE.md`, "C for the translation",
  has why.
- Looked at and not taken: an object file without code or data that a
  tool writes, for the segments and the groups, which would do the same
  and is neither C nor the linker's command; the code of the part that
  stays in a segment of its own, which wants the vectors, `SetBrk`,
  `Exit01` and INT 2Eh told another segment than the PSP's; and the
  data linked in front of the code, which puts that code behind the
  stack, on the page that QEMU is slow on.

### What holds the data

The test `c-data`, `harness/datacheck.py`, holds every object of the
translation that has the name of an object of the original against the
original's bytes, without a machine: 96 constants of the C code, 2,976
bytes, and 373 objects of the assembly, 8,545 bytes, when it was made, and
469 objects of the C code, 11,973 bytes, now that all are C. It names
each. A word that holds an address is held by what it names: the
original's source is assembled once more as an object file, whose
relocations say which words those are, and the translation has to have at
such a word the address of what has that name in the translation. So nothing
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
  `Scan_P`, `Fnc_T`. And of the part that stays in the memory the seven
  numbers that `tools/mkexe.c` writes, and `Stays`, what that part
  keeps, which is zeros in the file; and `TrnBeg`, the room of a
  block's header, zeros too.
- `Suff_S`, the edition in the banner: it lies in room that is reserved
  for what only the shipped binary has.
- Data that became code: the tables of keys are `switch` statements, and
  the scenarios answer for them.
- The tables by which the stubs of the commands went on, which are
  gone with them, `Main_T`, `CmdNx_T`, `MnuNx_T` and `F35_T`: they are
  `switch` statements of `src/c/main.c`, as the tables of keys are.

The tables of the menus' commands it holds in a way of their own. The
original has the address of a command for every item; the translation has
the number of the command, a `CMD_` of `src/c/vc.h` that is named after the
original's label, and the test reads those numbers there: an item that
has another command than the original's fails it.

What is read behind the end of an object is held too, as far as the
code says how much that is: a name copied as 13 bytes, a path as 68, a
title as 80. `harness/reads.py` lists every place where code names a
constant with the bytes read there, 400 places, and the test fails for
an object that is shorter, for a text read to a zero byte that has
none, and for a place it does not know (see "What is read behind the
end of an object" in `CLAUDE.md`). Eighteen texts are written longer
than the assembly has them for this; `User_S` was the last found.

Weighed against that, and not done: the constants laid out as one
block in the original's order, so that what follows each object is the
original's by construction. What it would take: the original's two
segments of data are 13,096 bytes; the translation has 11,568 of them under
their names, in thirteen files of constants and some ninety arrays that
the code files read through CS, and the rest is gone, tables of keys
that are `switch` statements and tables of addresses. One block is one
file in the original's order with filler for what is gone; the arrays
of the code files out of the code segments, so that each of some two
hundred places that read one by its name reads through a far pointer; and the
arrangement by file given up, which let five agents move the data side
by side. What it would cure beyond the sweep is nothing that is known.
The reads that the sweep cannot make good, the menu that is none and
the 88 bytes of a panel's defaults, go on into addresses of routines
and into what is no data, and a word that holds an address is another
number in the translation wherever it stands. What the block would have and
the sweep has not is that it needs no reading of the code: the tool
goes by the text of the C, and a read that it takes for another form
than it is would pass.

## Known and not dealt with

- The setup program is not translated. `VCSETUP.COM` exists only as the
  assembly of `src/asm/`, some 24 KB of it, which the test `asm-bytes`
  builds and holds against the shipped binary; no C tree has it, and no
  scenario runs it. It goes into the portable C, as a part of the one
  program: "The setup program in the portable C: the plan" at the top
  of this file. The C translation stays without it.

- The number of bytes that the editor has free is not compared with the
  original's (see `harness/deviations.py`). What works it out is C now,
  in `T00_20` of `src/c/edit.c`, and the number still wants a check of
  its own: that it falls by one for every character typed, at least.
  It has been wrong once without a test saying so, when a stub that it
  called handed back another CX. Held against the original by hand when
  the editor's keys were translated, over some thirty steps of
  `editor-keys`, `editor-hex-keys` and `editor-buffer`, in text and in
  hex: 27,104 less in each, 331,648 for an empty text, with the translation
  at 90,560 bytes.
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
- Of `LeaveTm`, six instructions of the original have run in no
  scenario and are listed as out of reach: they are for DESQview. The
  portable C has nothing for DESQview any more
  (`src/portable_c/DEVIATIONS.md`, 3).
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
  Both follow the size of the translation, and the number is left out of the
  comparison in every scenario (see `harness/deviations.py`). So nothing
  checks the number itself: that the translation has less by what it is
  larger, at least. The same goes for what `mem` says in the scenario
  `command`, 416K free under the original and some 7K less under the
  translation, whose part that stays is 7,008 bytes longer, 3,840 of them
  its stack. With those numbers left out,
  no scenario shows any more that the program gives up its memory while
  a command runs: a check of its own would hold the translation's figure
  against the original's less what the link map says the part has grown
  by.
- Every disk of the bench has a volume label, so no scenario has the
  program look for one and find none. The Info does say "has no label"
  in `reread`, where it does not look: a directory named DIRINFO stops
  it before. The program asks whether to delete a label and does not
  get it off under FreeDOS: a disk without one would have to be made for
  the bench.
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
- Every instruction of the commands that start the viewer or the editor
  has run in a scenario, those that tell a program run as the viewer of
  the mouse too, since the bench has one; what they tell it shows on no
  screen.
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
- Of `__U4D`, which divides 32-bit numbers for the compiled code, no
  scenario runs a division by more than 16 bits: nothing divides by one
  yet. And a product or a quotient that is wrong for some numbers only
  would pass the scenarios: three files divide and one multiplies such
  numbers, `find.c`, `info.c` and `copy.c`. So the two helpers, which
  are C, `U4M` and `U4D` of `src/c/arith.c`, are held against other
  arithmetic, in two ways:
  - The test `c-arith`, `python3 harness/arithcheck.py build/c`, runs
    their bytes, as the linked program has them, on a model of the 8086
    that knows the instructions they are made of, and holds the product,
    the quotient and the remainder against Python's, for 729 pairs of
    numbers at the edges and as many random ones as it is told with
    `--pairs`, 4,000 unless told, in two seconds: about 2,200 of the
    divisions are by more than 16 bits. It also says if a register that
    is to be kept comes back
    changed, if the stack of the caller is written to, and if a division
    by 0 is not the processor's divide error. The two routines in
    assembly that were there before passed it on the same 200,729 pairs
    as the C does, `--pairs 200000`; four changes of one instruction
    made to see it work were each found.
  - `src/c/arith.c` compiles for the computer that builds as it is, with
    `-DVC_H` and the three types of `src/c/vc.h` at their widths: there
    it was held against that computer's arithmetic on 101 million pairs
    for the product and 98 million for the division, 47 million of them
    by more than 16 bits. The program that did it is not kept.
  Each covers what the other does not: the second knows nothing of the
  registers, and the first runs far fewer numbers.

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
  are constants of `src/c/find_t.c`, each 15 bytes long with what the
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
  copies them from `Mask_S`, a constant of `src/c/system_t.c`, which is
  written with what follows it in the original.
- `FindFil` calls itself, through `DirFunc`, once for every level of
  the directories it searches. The original took 18 bytes of its stack
  of 256 for a level, by reading; what the translation takes of its 4,096
  has not been measured, and how deep a tree either can search has not been
  tried. The scenarios search three levels. DOS's critical error can come up with
  all of that on the stack: `FindFil` calls DOS for the files, and to
  open and read them.
- The tables by which the menu bar finds the command of an item are
  constants of `src/c/commands_t.c`, with the boxes, dialogs and texts of
  the menus' commands, and the main loop calls the commands. Where the
  keys of the panels go on in three of them, `M01_01`, `M03_00` and
  `M06_00`, the loop calls those too, by what the key's command returns:
  `CmdEnd` of `src/c/main.c`.
- Of the commands of the menus, two instructions are listed as out of
  reach: one for a display that is neither EGA nor VGA, and one for a
  kind of panel that nothing asks for. The one of EGA lines that keeps
  the pointer of the mouse on the screen when it is below the 25th row
  and 50 lines become 25 runs at the end of `mouse-help`.
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
- Drive... of a panel's menu, "F9 l d", showed the box for a panel that
  was none in the translation, from the batch of the menu bar until that of
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
  the translation. It does not press F1: the page of the help is a byte that
  the original takes from its data and the translation from its code, the
  word behind the table of the pages read as a place in that table.
  Tried by hand, both showed the same box, "There was error reading
  data from the help file", for a page that is none; another build of
  the translation may have another byte there. Enter, Down and Left there
  were not tried, and F1 stays out of the scenario: they open a menu
  from behind the table, or show a page by that byte, so the original
  goes by stray memory there and the translation cannot be held to it.
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
- The map of the memory, Alt+F5, names the program by its file: VC.COM
  in the original and VC.EXE in the translation. `harness/deviations.py` has
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
- What the map does in the original and the translation does after it:
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
    call only. By reading: the bench has no such device.
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
- The functions of `src/c/copy.c` are on the stack when DOS brings up
  its box for a critical error during a copy, a copy to the floppy A:
  for one. No scenario has that.
- `copy-full` fills C: by copies, 32 MB, in half a minute. So a full
  disk can be made on the bench after all, and what the editor does when
  it saves to one, listed as out of reach for `T80_00`, could be shown.
- The `c-scenarios` test has five scenarios more for copying, 304 steps,
  which take two and a half minutes of machine time between them:
  `copy-full` alone 33 seconds on an idle machine and a minute next to the
  tests of other batches, the longest of all. It moves some 80 MB. Its first
  form did that in steps of up to 25 MB, and a step ran into the 20
  seconds that a step may take, twice, with other `c-scenarios` tests and
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
  editor, of the original and of the translation alike. One that fits, with
  a size in the same 64K as the room, has no place in a scenario: the
  translation's room was 27,104 bytes less than the original's then and
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
  in the translation alike: it would have to be told where that is in each,
  as it is told where `LeaveTm` is.
- The bench has a mouse now, and scenarios use it: `mouse-panel`,
  `mouse-bars` and `mouse-dialog`, with which 349 of the instructions
  that `harness/ported.py` listed as out of reach for the mouse came off
  the lists, and the scenarios that followed with nearly all the
  others. What is still listed, by the file of the translation that has it:
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
- `FncSel`, which goes through the selected files of a panel, and
  `FncExt`, where a command ends that may have changed the disk, are
  functions of `src/c/dirfunc.c`. In the original they are code in the
  program's data, behind `DirFunc`; `harness/coverage.py` names such
  code by its label.
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
- Enter, `F21_00`, saves what the program needs to start over, gives
  the main data back to DOS and returns `NEXT_EXEC` or `NEXT_INIT11`
  (`F21_31` of `src/c/enter.c`); the main loop returns that, and
  `TrnMain` of `src/c/main.c` returns to the part of the program that
  stays in the memory, which runs the commands, or starts over at
  `Init11`. The program also comes through `F21_31` when it has
  started. Which command a key has is a `switch`, `Func12` of
  `src/c/main.c`.
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
- Of the keys of the panels, what Tab does when DOS will not go to the
  directory of the other panel, or has not told the current one, has
  run in no scenario and is translated by reading, 18 instructions of
  `F20_02`: whatever removes or renames a directory has both panels
  read again, and the drives of the bench are ready. What they do for
  the mouse runs in `mouse-panel`, `mouse-list` and `mouse-select`.
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
- What `F38_70` does when DOS says that a drive is not ready, or will
  not go back to the directory it came from, what `M30_50` does
  when DOS refuses the attributes of a file that it has just listed,
  `M30_60` where the year is written first, and `DirFunc` for a path
  of 68 characters or more ran in no scenario and rest on reading.
- A directory deleted with what is below it takes about 90 bytes of
  the stack for every level in the translation, where the assembly took 22.
  The 32 levels that a path of DOS can have fit the translation's stack;
  they did not fit the original's 256 bytes.

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
- What a command leaves in the registers reaches the next command
  through the main loop, and a command that is C leaves other values
  there than the assembly did. What is read of it is at the top of
  `src/c/main.c`, found by reading every stub when the loop came to call
  the commands: the mouse's commands, the key for the command line, CX
  by EGA lines, and BX and DX by Rename and Copy where they do not
  count what is selected. For the last the translation was wrong until then,
  and no scenario showed it. The note here said that `Func05` loads BX
  with an address whenever a command has run, so that DX decides
  nothing. That holds for a command that comes back to the loop at
  `Func05` or before, and the keys of the cursor come back at `Func07`,
  behind it: their stub left 0 in BX and in DX, where the original
  leaves what moving the cursor made of them, an address or a size in
  DX. So Shift-F6 behind Down offered the name of the file under the
  cursor, and the original offers none. The cursor's keys leave both as
  the original does now, in `struct Regs`, and `rename-keys` holds it.
  After the speed search, which goes on without `Func05` too, the loop
  hands on BX, DX, SI and CX as the assembly left them.
- What the program does by the hundredths of the time of day is the
  same every time a scenario is run with the same program, but need not
  be the same in the translation, which gets to a step some instructions
  later than the original, and so some hundredths. `DoublMo` adds 0.4
  seconds to the time of day of a click, and three instructions of it, `INC
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
- There is no `src/c/asm/` any more. What was said here of each piece of
  its `VC.ASM` that was code, and what became of it:
  - The stubs of the start-up, `Init00` and `Init11`, are `TrnMain` of
    `src/c/main.c`, which the part that stays calls: it calls the three
    functions of `src/c/start.c` and the main loop, and puts DS on the
    main data and back by the instruction of inline assembly that
    `FatErr` has. What `src/c/start.c` names of the part that stays is
    in one list at its top: `ComnIni`, `WCB1sav`, `WCB2sav`, `Counter`,
    `Vect22` and `Vect27`, which are members of `struct Stays`,
    `Vect21`, `RelTab`, the functions `Exit01`, `New21` and `New27`, by
    their offsets, and `SetBrk`, which it calls through a pointer to a
    far function.
  - The part that stays in the memory is C, `src/c/resident.c`, with its
    data: the entry of the program, `Alloc0`, which DOS enters; `Reloc`,
    which that part applies to the other whenever it has loaded it; the
    outermost loop of the program, `Stay`, which calls the part that
    goes, far; `Exec`, which runs the commands and loads the program
    again; `Exit00` to `Exit05`; `New21` and `New27` with what keeps
    the list of the programs that stay in the memory; `Release`, which
    the map of the memory calls through a pointer to a far function;
    `SetBrk`, `NotFind`, `OutStr`, `GetStat` and `SetStat`. The three
    labels that the part that goes jumped back to, and the stub of
    `Release` with `CallPSP`, are gone.
  - `FatErr`, what DOS calls for a critical error, is C, an interrupt
    function in `src/c/main.c`. Its texts and its two boxes are
    constants of `src/c/main_t.c`, and so is `SecDat`, the word that
    holds the segment of the main data, but that it is a variable:
    `src/c/start.c` writes it, and `FatErr` loads DS from it, by the one
    instruction of inline assembly there.
- Ported as they are, in `src/c/start.c`: see the comments of `Init00`
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
    translation has no such table any more, the keys are a `switch`, and
    `Init00` of `src/c/start.c` puts zeros where the original has those
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
    for seven readings either way. The portable C has no such test, and
    does not wait for a display (`src/portable_c/DEVIATIONS.md`, 5).
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
  translation after it (`src/c/resident.c`); whether to mend any of it is
  not decided:
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
- The entry of the program, in the original and in the translation:
  - The program sets how DOS looks for memory to what it is when DOS
    starts, the lowest block that fits and no upper memory, whatever it
    was when the program was run. Ported as it is.
  - The original cannot always make its block of memory as long as the
    program needs it, its file being shorter than the program in the
    memory by the data of the part that stays, and then asks DOS for a
    block elsewhere for the part that goes: so it can be loaded into
    a block that holds its file and little more, the upper memory for
    one. The translation's file is as long as the program in the memory, so
    DOS loads it where all of it fits or not at all, and that second
    way is not in the translation: it could only be taken with DOS's chain
    of blocks destroyed, where it fails too. The translation ends then as
    the original does when both ways fail, "Program too big to fit in
    memory".
  - The check sum by which the program knows its file again after a
    command leaves out the data of the part that stays, in the translation:
    the stack is there and in use while the sum is taken.
  - A DOS before version 3 is left by its function 0 in the translation,
    where the original returns to the INT 20h that a .COM file has at
    the bottom of its stack. Both end the program the old way.
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
- Where the part that stays in the memory (`src/c/resident.c`) is not
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
  - Where the processor leaves the auxiliary flag undefined, the translation
    has it clear in the flags that it enters DOS with: after the `XOR`
    of a list that is full, and after the `SHL` of INT 27h. The
    processor of the bench leaves it clear after `SHL` as well.
  - No scenario shows the flags where the list of programs is full, or
    where DOS will not make a program's block longer for its record.
    The first was tried by hand, with nineteen programs from the second
    copy of `passed-on`: a program that is ended for a full list, AH
    31h made 4Ch, came to the watcher with zero, parity, carry and
    interrupts let in, 0247h, under the original and under the translation
    alike. It is not kept. The watcher is on the first copy's list, so
    that list is full a program sooner, and the first copy ends the
    eighteenth, of which the second has a record all the same; whether
    it still has it when the next one comes follows where the memory
    was taken, which follows the size of the program, and so the
    original's second copy was not full for INT 27h where the translation's
    was. A watcher that runs the second copy itself, as its parent,
    would be on no list. The second rests on reading.
  - The second way of the entry is not there: the original, when DOS
    will not make its block of memory as long as the program needs,
    asks for a block elsewhere for the part that goes, so it can be
    loaded into a block that holds its file and little more. The
    translation's file is as long as the program in the memory, so DOS loads
    it where all of it fits or not at all (see the entry, above). As
    the original: the file would have to be shorter than the program
    again, with the data of the part that stays laid over what
    follows it, which `tools/mkexe.c` could do by leaving that data
    out of the file; the entry would then run on a stack outside what
    it moves and set the program's own afterwards, four instructions
    of assembly (`CLI`, SS, SP, `STI`) and a jump as behind INT 2Eh,
    and the check sum and `harness/driver.py`, which knows what the
    stack holds when loaded, would follow the new layout.
- Where the size of the translation becomes other behaviour, for the user to
  weigh. A second copy of the translation that runs a program which stays in
  the memory may have no room to load itself again afterwards, and
  ends with "Program too big to fit in memory", where the original
  goes on. It follows from how large the translation is in the memory and in
  its file, and from nothing else: the translation does there what the
  original does. A program that stays lies behind the COMMAND.COM that
  ran it, a gap of 71K when that has ended. The part that is loaded
  again is 111K in the translation and 51K in the original: the original's
  fits the gap, the translation's goes behind the program that stays. And it
  is loaded from the program's file, which wants its own length behind
  that once more, 130K in the translation and 64K in the original. With
  another program staying in front of the copy, behind a gap of its
  own, the watcher of `passed-on`, that is more than the bench has
  left; without one in front the translation has room, which was tried. The
  scenario switches Memory allocation off in the copy, which keeps all
  of it loaded. The translation may use more memory than the original as
  long as it behaves the same: this is a place where it does not.
- How the part that stays in the memory is laid out by C, and what it
  rests on (`src/c/resident.c`, `tools/mkexe.c`). Each of these is in
  the README's list, "What rests on Open Watcom"; this is what would
  show if one stopped holding:
  - Its code and its data are one group, `R_GROUP`, by the compiler's
    options `-nd=R` and `-g=R_GROUP`, and the group begins at the PSP,
    since its first segment is the code, which begins with the room of
    the PSP. SS, DS, CS and the offsets that the rest of the program
    has for that part all count from there. `tools/mkexe.c` stops the
    build if the group or `Psp` is not at 0.
  - What needs an order is one structure, `Stays`, since the compiler
    lays out what has no value in an order of its own: `WCB2sav` right
    in front of `ComnIni`, where the setup file is read across the
    two, `Counter` in front of `PrgList`, `SavePtr` behind `IntTab`.
    What needs a name is a variable by itself, since a member has none
    to the linker: the seven words of `tools/mkexe.c`, `RunPar`, the
    texts, which the test `c-data` holds, and the stack.
  - That the stack is the end of that part is by the compiler's
    segments and not by the language: `DataSav` is the one object of
    that file without a value, and what has none is in the last of the
    four, `R_BSS`. `Stays` is written with a value for that, zeros,
    and last of what has one. `tools/mkexe.c` stops the build if
    `R_BSS` holds another symbol, if `DataSav` is not at its start, or
    if anything from `Stays` to the part that goes is not zero in the
    file.
  - `Vect21`, where INT 21h went before the program took it, is read
    through CS by the entry of that interrupt, `New21`, which passes a
    call on before it has saved a register: a word of the code's own
    segment is all that can be reached there with no register and no
    stack. It is a variable declared `CODE`, with a value, so that the
    compiler does not write it in front of the PSP's room.
  - What the check sum of the file leaves out is from `Stays` on: the
    layout numbers, the texts and `OvlPrm` are summed.
  - `SecDat`, the word with the main data's segment, is a variable of
    the compiler's own group, in `src/c/main_t.c`: an interrupt function
    has DS on that group when it begins, which is how `FatErr` reads
    the word. `tools/mkexe.c` refuses static data in a file of the part
    that goes that has code, because the code has DS elsewhere; a file
    without code may have a variable, with a value, so that it is in
    the file. The group is relocated like any segment of that part,
    and `src/c/start.c` writes the word after every command.
  - The constants behind the stack, `CONST2`, some 8,900 bytes, keep
    code off the page that the stack ends in: it is the linker's
    command that has them in front of the code, and `tools/mkexe.c`
    that stops the build if they are not.
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
- The list file gets, behind its name at TmpBuf+100, the six bytes that
  follow `vc.lst` in the assembly, and the menu's name at TmpBuf the six
  that follow `VC.MNU`: both are copied from the assembly, `List_F` and
  `Menu_F`, as long as it has them. Nothing reads the six.

- `VCSUB1.INC`, `VCSUB2.INC` and `VCSUB3.INC`, the shared routines, are
  gone. Their last stubs were `ReDir` and `ReadTre`: `ReDir` is one
  function of `src/c/read.c`. Their last entries for C were `AsmSeg_a`,
  which is `AsmSeg` of `src/c/misc.c`, and `CallPSP`, by which the map of
  the memory had a program released: C code makes such a call through
  a pointer to a far function. Their texts and tables are constants of
  `src/c/bars_t.c`, `src/c/treebox_t.c` and `src/c/system_t.c`. Four names
  of files are copied with what follows them, and are written there as
  long as what is copied, with what the original has behind each:
  `Mask_S` and `Ext_F`, 68 bytes by `MvzLine`, `DrIn_F` and `Init_F`,
  the 13 of a name. Nothing is known to read those bytes where they
  get to.
- No entry in the assembly calls an interrupt for C code any more. A
  drive made the current one is `SelDrv` of `src/c/system.c`; the FCB
  that deleting by a mask has DOS make is `Fcb29` of `src/c/dirfunc.c`;
  what the map of the memory asks of DOS is in `src/c/memory.c`,
  `Dos5802` to `Dos2523`. All go through `Int21d` of `src/c/dos.c`, as
  the assembly called DOS itself there, without `Intr21`. `LeaveTm`'s
  three interrupts are `Int28`, `Int2F` and `DvPause` there.
- The jump of the main loop to the command of a key is a `switch`,
  `Func12` of `src/c/main.c`, and `Case` is gone. The loop calls the
  commands, from a second `switch`, `Command`. The structure of
  registers is still there: `struct Regs` of `src/c/vc.h`, for the few
  that one command reads as another left them. Its fields SI, DI and BP
  are read by the mouse's commands alone, which the loop sets them for
  just before, and what else the loop writes there, as the original
  leaves those registers, nothing reads.
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

The full `c-scenarios` test took 105 seconds on an idle machine with 59
scenarios and twelve minutes with eight checkouts busy, so four other ways
were tried on 11 and 12 October 2026, and a fifth, a driver that stays
resident in the guest, which is what the tests use now (see the README). What was measured first: 77% of a
scenario's time is the guest running while the harness waits, most of it
the program waiting for the timer while a box grows; pressing keys is
13%; reading the machine's memory for the stack 6.5%; starting a machine
1%. Recording what was executed costs nothing that shows. None of the
four is in use; each is on a local branch of the checkout named, not
pushed.

- A build of the translation that tests itself (`proto-selftest`, checkout
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
  - 4 minutes: a key lost in the editor. The `c-scenarios` test failed
    before anything was translated, an Enter gone from a step of seven keys:
    the editor empties the keyboard buffer after every key. Found by reading
    its loop; the scenario switches the option off first. Noted at the
    top of `harness/scenarios.py`.
  - 4 minutes: two steps that could not stay, each a golden derived, 23
    seconds, and one of them a `c-scenarios` test failed, 34: the volume
    label, which the program does not get off the disk, and the memory free
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
    were derived and 60 before the first `c-scenarios` test. The rebase, the
    build and the test after it took a minute and met no conflict: the
    other batch was not on `main` yet.
  - 1 minute: a run of `harness/show.py` that pressed Enter on a file
    of A:\ instead of a directory and so ran `fdauto.bat`; the files
    there are not in the order of the bench's. Noted at the top of
    `harness/scenarios.py`.

- The viewer's keys, 2,290 bytes that are no procedure but for 20 of
  them, 40 minutes to a translation that passed its own scenarios the
  first time it was built, next to seven other batches; then the
  `c-scenarios` test did not pass, for scenarios of copying, and the batch
  waited an hour and a quarter for its turn to land:
  - 7 minutes: the first full `c-scenarios` test, 4 minutes 6 seconds at a
    load of 17 on two processors, and the two scenarios it failed in run
    again by themselves, a minute and a half and 44 seconds. `copy-full`
    stopped both times at step 36, '"c1" Enter', a copy of 3 MB that did
    not end within the 20 seconds a step had then. `move` differed at
    step 66, '"d:\" Enter', in the full run and not by itself: the
    translation's screen was taken with the dialog of F6 off the screen and
    the box of the move not yet drawn, and the next two screens were a step
    behind. The end of a step of the translation has been tied to the keys
    it has taken since. The other 56
    scenarios were as the original, and the checks that the test makes
    after the comparison were run for the viewer's scenarios by a
    script of thirty lines around the functions of `harness/ported.py`,
    49 seconds: every instruction ran or is listed, every function ran.
    A `c-scenarios` test that could leave a scenario out, or that went on to
    its checks after a scenario has failed, would have said the same in
    one run.
  - 1 minute: a search expected to begin at the top of the file again,
    which begins at the place found last: one trial of 19 seconds on
    the translation's build that signals. At the top of
    `harness/scenarios.py` now.
  - 1 minute: the golden of `view-keys` derived twice, 21 seconds the
    second time, for a file between what the viewer holds and 64K,
    which `harness/reach.py` showed to be missing when the rest was
    reached.
  - What saved time: trying every scenario on the translation's build that
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
    `c-scenarios` test, 5 minutes 33 seconds, passed at once.

- The editor: addresses in its text, a line on the screen, saving, 13
  routines, 1079 bytes, 40 minutes in all, next to the viewer's batch:
  - 5 minutes: a scenario that could not stay as written, found by the
    `c-scenarios` test before anything was translated. It showed D:\VC in a
    panel, where the program's file has another name and size; and it
    saved to the write-protected floppy, which left 4 bytes of the stack
    in the translation as it was. Two goldens derived again, 33 seconds
    each, and the stack of every step read from the machine by a script of
    ten lines around `scenarios._stack_left`: `ported.py` says how much a
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
    the `c-scenarios` test with the routines still assembly took 138 seconds
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
    original and the translation as it was, and was 504 bytes too large for
    the translation with the editor in C: the scenario failed on the first
    run of the translation. It has a file now that is too large for
    both. At the top of `harness/scenarios.py`.
  - 3 minutes and a half: a full `c-scenarios` test that failed on nothing
    but `T60_00_c` and `T61_00_c`, listed for an earlier batch and called
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
    it and F4 before and after them, which the translation passed twice and
    then failed, in a full `c-scenarios` test of nine minutes: the key bar
    was missing. It is drawn only where the program waits and finds no
    key, and the translation need not wait within a step. Found by reading
    `Input0` and `PutKey0`; at the top of `harness/scenarios.py` now.
  - What saved time: `harness/reach.py` with steps and `--changes`
    written to a file, 10 seconds for 30 steps, and the scenarios run
    on the translation before anything was translated, with
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
    two `c-scenarios` tests, and 25 before a derivation that then stopped at
    once, the scenarios not yet in the file: an edit had been refused
    and the refusal not read.
  - 1 minute: the C function named `Help_c`, which the assembler takes
    for `Help_C`. In `CLAUDE.md` now.
  - 1 minute: the byte 1Ah at the end of the files of `src/c/asm/`,
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
    and some three minutes of a `c-scenarios` test that took 4:43 where it
    takes 1:16 by itself.
  - 1 minute and a half: a derivation that stopped, "the screen did not
    settle", at the step that ends in the help; the next one went
    through.
  - 1 minute: `FndAdr_c` not executed in the translation. Its stub is called
    by Go to alone, which no scenario had: three steps more, and the
    golden derived again.
  - The `c-scenarios` test failed on the stack alone, none of 256 bytes left
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
    derived, which then took a minute, 143 and 134 before the two
    `c-scenarios` tests, the one before translating and the one after, and
    43 before a comparison of four scenarios. Two runs of `harness/reach.py`
    with steps, 20 and 36 seconds, did not wait.
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
    and 77 before the goldens were derived, 27 before a `c-scenarios` test
    and 98 before a run of `harness/show.py` on the translation. The others'
    tests follow one another, and a run that wants the computer alone waits
    for a gap.
  - 6 minutes: `EXTERN` for the C functions at the top of `VCF1.INC`,
    which is included inside the code. The assembler made near calls of
    the far ones without a word; deleting worked and the attributes
    ended in "Invalid Opcode". A `c-scenarios` test failed, 77 seconds,
    three runs of `harness/show.py` on the translation, and the bytes of a
    stub read in the program file. In `CLAUDE.md` now.
  - 6 minutes: the golden of `delete` derived four times, 71 seconds
    each. The tree's file is gone from the root after a directory has
    been deleted from the tree; a panel lists the new one only when it
    is read again; and `DelTr0`, a label inside `DelTree` that `^R` in a
    tree calls, was found by the assembler when the scenario was done,
    and wanted a step. At the top of `harness/scenarios.py` now. Reading
    `tools/routines.py` for labels that others call, before the
    scenarios are written, would have saved the last one: it does not
    show them.
  - 4 minutes: `harness/reach.py` and the `c-scenarios` test did not know
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
  - 3 minutes: the `c-scenarios` test took 268 seconds next to the tests and
    the recordings of seven other batches, where it takes 105 by itself. The
    steps were tried on the translation's build that signals first, 6 to 10
    seconds a run, and the three goldens were derived once, in 19
    seconds.
  - 2 minutes: the commit had the message of another batch. The file
    it was written to was in the directory for scratch files that the
    session gives its agents, which all of them share, and another agent
    had written one of the same name meanwhile. Seen in the log before
    anything was pushed. Such files belong in the checkout's own
    `build/`.
  - 55 minutes for getting onto `main`, six times. The first rebase met no
    conflict and no entry twice, and its `c-scenarios` test failed after 324
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
    the editor landed. The sixth had `src/c/edit.c` and `VCEDIT.INC` in
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
    golden was derived, 100 before the `c-scenarios` test with the routines
    still assembly; and the first test of the C took 156 seconds where
    it takes 78 by itself.
  - 3 minutes: "not executed in the translation" for three functions that
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
    and `struct Box` was in its C file and in `src/c/vc.h`. Behind
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
    banner shows, which the original and the translation have differently:
    found by `harness/compare.py` on the translation before anything was
    translated, 20 seconds. Then a step for the one instruction left,
    and one for the function that only the menu calls. At the top of
    `harness/scenarios.py` now.
  - What saved time: the scenarios were tried on the translation's build
    that signals, six seconds for twenty steps, by a script of ten lines
    around `scenarios.run` and `show.print_screens`, and held against
    their goldens with `harness/compare.py` before the translation and
    after it, 20 seconds; the full `c-scenarios` test ran once before the
    rebase, 193 seconds next to the others.
  - 3 minutes for the rebase, behind what Copy and Rename ask first: one
    file in conflict, the notes at the top of `harness/scenarios.py`,
    where both had added to the end of the list. Two entries were there
    twice without a conflict: `SetPth`, which the other batch had made
    next to the routine and with an answer, and the call of DOS that
    makes a drive current, `SelDrv_a` there and `SelDsk_a` here, the
    same instructions. Those of this batch went.
  - 7 minutes: the `c-scenarios` test after the rebase, 356 seconds with a
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
    panel off next to the left one, and the translation showed "are" where
    the golden has "ware": `harness/deviations.py` knows the banner by its
    beginning only (see above). It looked like the translation, and was
    in the translation without it: the original and the translation were run
    through "Ctrl+l" and "Ctrl+p" with `harness/show.py`, and the scenario
    switches the left panel off now. The `c-scenarios` test with the batch
    listed without functions, before translating, would have said so
    at once; it was left out to save a run of three minutes.
  - 2 minutes: F1 on an item showed the index of the help. The table of
    the pages was a constant of two dimensions, which the compiler read
    in the main data. Found in the listing; in `CLAUDE.md` now.
  - What saved time: `harness/compare.py` with `--signals` and the name
    of the one scenario holds the translation against its golden in ten
    seconds, and both of these showed there, not in the full test.
    `harness/show.py` on the translation's build that signals tried the 49
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
    the same list of notes. The full `c-scenarios` test, 218 seconds each
    time where it takes 105 on an idle machine, passed before the first
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
    the original, 3 minutes of them, before a run of the translation with
    the keyboard buffer printed at every key showed what it was: after a
    box with an error the dialog of F5 comes up slowly, and the keys
    behind F5 fill the buffer. It looked like one letter of one name,
    because the sixteenth key was a `t` each time. `reach.py`,
    `show.py` and `goldens.py` say which key and not which step, and
    print no screen of the steps before. Noted at the top of
    `harness/scenarios.py`.
  - 19 minutes waiting for a turn at the computer, in 43 runs, about
    half of them while other work went on: a derivation of goldens
    waited a minute or two each time, eight times, and the last
    `c-scenarios` test took four minutes where it takes one and a half.
  - 12 minutes: goldens derived again. `move` three times, 2 minutes
    each with the waiting: once for steps added, once because a step of
    the editor in hex stopped with "the screen did not settle"; the
    file it was to make is now copied from the help file. `copy-full`
    four times, 2 or 3 minutes each: for a better ending; because that
    ending, going on after a full disk with a file that was appended
    to, left other contents in the translation than in the original (see
    above), which the `c-scenarios` test found after the rebase; and for
    smaller steps, after a `c-scenarios` test of four minutes that stopped
    in it at the 20 seconds a step may take.
  - 1 minute: F6 and Enter on a directory, which moves it into itself.
  - 31 minutes from the first rebase to the push: four rebases, each
    with a build and a `c-scenarios` test of two and a half to four minutes,
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
  - What saved time: steps tried on the translation's build that signals,
    which is the original's code until something is translated, needs
    no turn alone and takes a tenth of the time. A script of fifteen
    lines around `scenarios.run` and `show.print_screens` ran a whole
    scenario of `scenarios.py` that way; four scenarios of sixty steps
    were right when their goldens were first derived.

- What Copy and Rename ask first, the code of six commands and three
  routines, 903 bytes, 25 minutes to the commit, next to seven other
  batches. The translation passed the first time it was tested, and no
  run waited: nothing takes turns any more. Two goldens were derived in
  20 seconds, the first `c-scenarios` test took 151 seconds.
  - 3 minutes: three drafts of the scenarios, tried on the translation's
    build that signals, 10 seconds each. A step typed into the buttons of
    the dialog instead of its line, and after a directory of the tree is
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
  - What saved time: all steps tried on the translation's build that signals
    first, six runs of `harness/show.py --changes` of 5 to 15 seconds,
    before the goldens were derived, which then took 21 seconds for
    four scenarios and were right, but for Ctrl+F8, which was added.
    And `harness/compare.py` with only the scenarios of the batch, 20
    to 45 seconds, before the full test.

- The user menu, the command line, the history and the box of the
  drives, nine names in `harness/ported.py`, 3,429 bytes, 67 minutes
  in all, next to seven other batches. The translation passed its own
  four scenarios the first time it was run.
  - 14 minutes: the full `c-scenarios` test failed once, 3 minutes 35, in
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
    the size of the program, found by comparing the translation as it was
    with the new golden, which was derived again, 33 seconds.
  - 2 minutes: three builds for a variable named `far`, and for a far
    pointer kept in a near one. In `CLAUDE.md` now.
  - 1 minute: the scratchpad directory is one for all agents, and a
    directory made there for a second derivation held another agent's
    goldens under the same name. Nothing was lost; the files are in
    `build/` since.
  - What saved time: steps tried on the translation's build that signals,
    six seconds for twenty steps; the four scenarios, 272 steps, were right
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
    to follow the size of the program too, which the `c-scenarios` test of
    the scenarios showed before anything was translated; and for a step
    more.
  - 6 minutes: deciding what to do about the name of the program in the
    map, VC.COM or VC.EXE, which no scenario of the map can avoid
    showing.
  - 25 minutes to get onto `main` an hour later, behind fourteen
    commits: four files in conflict, each where both sides had added,
    the list of scenarios cut at the lines that two scenarios end with.
    What git did not show: `PutMnu` had become C, and the keys of the
    panels had made entries for `SavMous` and `CurMous` too, which went
    for those next to the routines. The first `c-scenarios` test, 9 minutes,
    failed in `command-edit`, a scenario of another batch: "Invalid
    Opcode" after a command of an extension file. The compiled code of
    the first segment was 64,636 bytes with `memory.c` in it, 900 below
    the 64K, and the linker had said nothing. The same scenario passed
    on `main` as it was, 17 seconds, and with `memory.c` in the second
    segment, where the first has 61,177 bytes; the second test, 254
    seconds, passed. Why the program fails with a first segment that
    full was found out afterwards: see `CLAUDE.md`, "C for the translation".

- The commands of the menus, 28 commands and three routines, 1,210
  bytes, 50 minutes in all, next to three other batches and the mouse of
  the bench:
  - 10 minutes to get onto `main`, behind the two commits that gave the
    bench its mouse: no conflict, the goldens of the four new scenarios
    derived again for the pointer, and one `c-scenarios` test, 256 seconds
    next to the tests of others. Eight of the nine instructions that EGA
    lines has for the mouse ran at once, with a mouse there.
  - 5 minutes: one `c-scenarios` test more, 100 seconds on a quiet machine
    and 270 next to the tests of others, because the scenario of the floppy
    under its second letter reached four instructions of `ReDir` that
    its batch had listed as out of reach. `harness/reach.py` says so
    beforehand only for a routine that is asked about: after goldens are
    derived it could say for every batch which of its listed
    instructions have run.
  - 4 minutes: Drive... of a panel's menu, which the translation got wrong
    before this batch (see above). It showed in reading what the menu's
    table jumps to, before any scenario chose the item.
  - 3 minutes: a scenario with the Info in both panels, which fails on
    the translation by the second "Bytes Free" of a row. The top of
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
  - 4 minutes: the first run of the `c-scenarios` test, which failed on
    seven instructions that two earlier batches had listed as out of reach
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
    same names; theirs stayed, and both are declared in `src/c/vc.h`
    now. The three scenarios of the mouse passed on this C at once, so
    nothing was lost to the mouse at the panels; a fourth was written
    for what they leave, and derived five times because one record of
    what ran differed (see above).
- The shared routines of `VCSUB1.INC` and four of `VCSUB3.INC`, 49 of
  them, 1,822 bytes, with the file for DOS and the BIOS, some 45 minutes
  to a translation that passed, next to three other batches and one
  change to the harness. No golden was derived twice, and the full
  `c-scenarios` test passed both times it ran, 173 and 233 seconds.
  - 2 minutes: the halves of a number of 32 bits in `[dx cx]`, which are
    the other way round than in `[dx ax]`: the cursor came back to the
    wrong place after the help, in the third scenario compared. In
    `CLAUDE.md` now.
  - 5 minutes: the first comparison of all scenarios, 4 minutes 26 with
    the other checkouts at work, where the `c-scenarios` test takes three.
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
    which went into `src/c/dos.c`, the first by making `Int10` take the
    structure that their code already filled; and the new scenario of
    the left-handed mouse ran 55 instructions that the dialogs had
    listed as out of reach, which the first `c-scenarios` test said, 132
    seconds. The second passed, 348 seconds next to other runs. The
    mouse's routines, read and never run before, needed no change.
- Enter, the keys that change the directory, the commands that view
  and edit, and `FncSel`: 2,252 bytes, all but 281 of them no procedure,
  about 50 minutes to the commit, next to four other batches. The
  translation passed its scenarios and the `c-scenarios` test the first time
  it was built.
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
    `enum` of `src/c/vc.h` for all files, `M03_00` among them, which is
    the name of a routine that another batch has in hand. Undone before
    the commit; in `CLAUDE.md` now.
  - 2 minutes: the first scenarios copied the driver of the tests,
    which is on the floppy as driver.com, and showed the bench's
    fdauto.bat in the viewer: both change when the harness does, and
    another batch was changing it. Two goldens derived again, ten
    seconds. At the top of `harness/scenarios.py` now.
  - What saved time: `harness/reach.py --steps` with `--changes`, three
    seconds a try, said for every idea whether it reached what was
    left, a dozen tries in all; and the `c-scenarios` test before anything
    was translated, which showed that the three scenarios hold.
  - 30 minutes for getting onto `main` twenty minutes later, behind
    five commits: the mouse on the bench, the commands of the menus,
    the main loop and the shared routines, with every entry for C of
    theirs gone. Ten files in conflict. What git did not show: the C
    of this batch called DOS, the BIOS and the cursor by entries that
    were functions of `src/c/vc.h` by then, and its own entry for the
    BIOS went for `Int10`; 38 stubs of shared routines had lost their
    last caller in the assembly with Enter and the viewer's commands,
    found by `tools/routines.py` on both trees and taken out by a
    script, with their lines of `EXTERN` and of `harness/ported.py`;
    the four instructions for the mouse ran, now that the bench has
    one, and left the list; the three goldens were derived again for
    the pointer. The build passed at once, and the `c-scenarios` test, ten
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
    that, in the C and in three files of notes. The `c-scenarios` test
    passed with it. It showed when the setup file of a copy started on the
    defaults was held against the original's, to see how they differ:
    by those three bytes and nothing else. In `CLAUDE.md` now, and the
    scenario ends with that file on its disk.
  - 3 minutes: the flag for a setup file that is not correct was set
    for every file that could be read, a mistake of the C. The two
    scenarios with a second copy said so at once, in the line of the
    banner.
  - 25 minutes: four runs of the `c-scenarios` test, five to nine minutes
    each next to the tests of others, where it takes under two alone,
    most of it spent writing the notes meanwhile. The
    first failed on three instructions that two earlier batches had
    listed as out of reach for want of a monochrome screen, which
    `start-screen` has; `harness/reach.py` on the routines of the batch
    does not show those of others.
  - 5 minutes to get behind the shared routines, which landed while the
    scenarios were written: one conflict, in the list of scenarios, and
    the calls of DOS written against `src/c/dos.c` from the start. That
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
    `c-scenarios` test took 78 seconds, alone on the machine.

- What was left of the shared routines, seven of them with the screen
  blanker, 717 bytes, and the entries that called DOS for C code: some
  45 minutes to a translation that passed, with other checkouts at
  work. Every scenario compared as the original the first time it ran
  on the C, twelve of them in 20 seconds and then all.
  - 18 minutes: the `c-scenarios` test twice, 468 seconds for the base
    before anything was changed and 612 for the batch, where it takes 100
    alone. The base need not have been tested: the batch before had
    just passed on it.
  - 8 minutes: why the translation's sky is another than the original's. A
    model of the routine in Python, 40 lines, found the original's
    stars for a seed of 0 and none of the translation's for any number of
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
    translation with `Rand1` and `Rand2` set as `Blank` begins, 5BA1h and
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
    goldens came out as before on the new bench, and the `c-scenarios` test
    passed the first time, 150 seconds.

- The data of `VC.ASM`, `VCSUB2.INC` and `VCSUB3.INC`, and the
  compiler's helpers, with four other files of data moving in checkouts
  of their own:
  - 5 minutes: `src/c/vc.h` declares `Title_S`, `Var_S` and `Tree_S` for
    the code, far, and `src/c/tables.h` includes it. A file of constants
    that defines one of the three was "Modifiers disagree with previous
    definition", and one whose box names one "Pointer truncated", which
    stops the build. Every checkout met it. These files got round it by
    a `#define` around the `#include`, until `src/c/tables.h` came with
    the rebase, which gives each of the three a near name: the file
    that defines one writes it under its name and no `#pragma aux`.
  - 3 minutes: the first of the sixteen names that `VC.ASM` declares
    `EXTERNDEF` went to C, and the test `c-data` failed on each, "the
    translation has 53h": the assembler's listing has such a name in `CONST`
    at 0, marked `External`, and `harness/datacheck.py` took it for an
    object of the assembly. Two checkouts mended that, each for itself;
    the one that landed first is what is there.
  - 4 minutes, at the rebase: with the last text gone from the
    assembly's `CONST`, `harness/datacheck.py` stopped at `TrnBeg`, the
    label at its start, "min() iterable argument is empty". A label
    with nothing of the assembly behind it is no object now. And
    `TODO.md` was in conflict in every commit: its changes were made
    again on the new text by the scripts that had made them, and the
    last row of the inventory added up from the others.
  - The `c-scenarios` test took 320 and 219 seconds where it takes 150, with
    the other checkouts testing. Nothing came out different.
  - What saved time: the lines of the assembly were taken out by a
    script that names the lines it expects, and the names of the test's
    output were held against those of before by another, which also
    says which objects have another length.

- The data and the stack of the part that stays, to C:
  - A quarter of an hour before a line was written, on where the data
    can lie. The plan had its group linked first, to begin at the PSP,
    which puts the code of that part behind the stack, on the page
    that QEMU is slow on; and the four segments that the compiler
    makes with `-nd` are of one class, `FAR_DATA`, not `DATA` and
    `BSS`, so they cannot stand on both sides of the code. What settled
    it was ten lines of assembly and a C file of the same in `build/`,
    linked by hand: the map says in a second where the linker puts a
    group, a class and an object, and whether an empty segment holds a
    group at the PSP. Try a layout there before the program.
  - A member of a structure has no name to the linker, and the stubs
    of the assembly, four C files, `tools/mkexe.c`, the harness and
    the test `c-data` each went by such names: what needs a name became
    a variable of its own, the stack among it, and the four files name
    a member through `STAYS` under the name it had.
  - 3 minutes: the twelve scenarios of that part came out as the
    original under `harness/compare.py`, and the `c-scenarios` test stopped
    in `harness/coverage.py`, which read at 100h of the file where the
    table of relocations is. `compare.py` does not trace what ran: it
    is `ctest -R c-scenarios` that says whether the harness still knows the
    file.
  - What saved time: the lines of the assembly were taken out by a
    script that names what it expects, and the three commits were made
    from the finished tree, kept on a branch while the first two were
    cut out of it and tested.

## Ideas put off

- Keeping the booted bench from one run of a tool to the next. Booting DOS
  for the two countries and making the bench's setup file take about five
  seconds of every run. What is kept would have to be thrown away whenever
  the harness, a download or QEMU changes.
- A file or a directory that a scenario makes gets the time of day, to
  the second, and the golden lists it; deriving is slower than testing, so
  the two differ. No scenario makes one yet. Such a time stamp could be
  recorded only as made during the run.
