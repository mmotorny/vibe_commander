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

Seven things were the user's to decide, and an eighth was met while
the setup was made. They are decided, and stand first.

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
| 8 | The 64 bytes behind the path of each panel in a setup that setup makes without a good file, or for Set default configuration: the original has there what stands behind a table of its setup program, of which the portable C cannot have a part | Zeros, as the portable C has them: the user confirmed it on 13 October 2026, and it stands. It was met while piece 0b was made and declared in entry 8 of `src/portable_c/DEVIATIONS.md` before it was asked |

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
| The setup that setup makes without a good file, and for Set default configuration, has zeros in the 64 bytes behind the path of each panel, where the original has what stands behind a table of 24 bytes in the setup program: see the bugs below. A setup file saved of it differs from the original's in those bytes. It was met while 0b was made, was not among the seven things decided then, and was decided on the user's word of 13 October 2026: it stands | No scenario: `setup-defaults` ends with the file as a program started anew saves it, where the bytes are zeros in the original too |
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
  has zeros for all 64, which was decided on the user's word of 13
  October 2026 and stands. See "The deviations to declare".
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
as it was, since its left half is. The Configuration is shown as it is
since Quick execute commands went too, which was not of this piece
("The portable C: without Quick execute commands" below): E2 left
Execute options with that one line, and the Keyboard options two rows
further up than the original's, where they are five now.

    ╔═══════════════════════════ Configuration ═══════════════════════════╗
    ║  ┌ General options ──────────┐  ┌ Keyboard options ──────────────┐  ║
    ║  │  [ ] Auto menus           │  │  [x] Alt alone selects menu    │  ║
    ║  │  [x] Path prompt          │  │  [x] Esc toggles screen on/off │  ║
    ║  │  [x] Key bar              │  │  [x] Alternative viewer        │  ║
    ║  │  [ ] Menu bar visible     │  │  [ ] Alternative editor        │  ║
    ║  │  [x] Full screen          │  │  [x] Clear keyboard buffer     │  ║
    ║  │  [ ] Clock                │  └────────────────────────────────┘  ║
    ║  │  [x] Confirmation on quit │                                      ║
    ║  │  [x] Error sound          │                                      ║
    ║  │  [ ] Auto save setup      │                                      ║
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

- The layout. Both right halves are shorter than the left ones now:
  the Configuration has five empty rows there and Screen and Mouse six.
  Whether the groups are to be aligned at the bottom, or the two
  dialogs arranged anew, waits for the user's word. What was decided
  on 13 October 2026 has changed the Configuration once more, and is
  made: Quick execute commands is gone, and Show logical drive B:
  stays.
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
for a step, or None for one that set what is gone. The table is of this
piece as it was made: the seven that show the Configuration have other
rows in their patches since Quick execute commands went, and
`setup-config` and `setup-config-mouse` other steps, which the section
on that has.

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
bits and counts each place that gcc refuses and each that only Open
Watcom reads. `harness/hostcheck.txt` has the counts of every file, and
the test holds the tree to them, so the record is always what is so:
the top of the script says how. This is what the kinds are waiting
for, with the counts of 13 October 2026, when the words of the 16-bit
compiler had gone from the source; the test prints them as they stand.

65 places in 5 of the 65 files. Sixty files are clean. All that gcc
refuses is in `dos32.c`, the system's functions for DOS; `main.c`,
`resident.c`, `stack.c` and `vc.h` each have a line or a few that only
this compiler reads. The first count, of the program of 16 bits, was
1,391, with two files of tables clean.

| Kind | Places | Files | What it is | What it waits for |
|---|---|---|---|---|
| `to-number` | 15 | 1 | A pointer cast to a number of 32 bits, which does not hold it on a machine of 64: `dos32.c` hands DOS and the extender addresses in registers | Nothing in the common code. The layer, steps 3 to 6 of the road: `dos32.c` is the side of it that is DOS |
| `to-pointer` | 24 | 1 | A number of 32 bits made a pointer, in `dos32.c`: an address of the machine, or one that DOS or the extender answered with | The same |
| `naked`, `asm` | 2, 2 | 1 | The two functions of `dos32.c` that are their `_asm`, `DeviceEntry` and `NothingEntry` | The same |
| `asm-pragma` | 11 | 1 | The eleven pieces of inline assembly of `dos32.c`, a `#pragma aux` each | The same |
| `registers` | 1 | 1 | The `#pragma aux` of `Arrived` in `dos32.c`, which names its registers | The same |
| `intrinsic` | 1 | 1 | `_disable` and `_enable` of `dos32.c` | The same |
| `data_seg` | 4 | 2 | The segment of the main data, in `main.c`, and of the stack, `stack.c`: a line in front of each and one behind | For the first, a program that counts nothing from where the main data begins, which the words of the addresses do until step 2 of the road has taken them down; for the second, a stack that the system gives |
| `aborts` | 5 | 3 | A `#pragma aux` that says a function does not return: `Alloc9` of `resident.c`, `Round` of `main.c`, and `AlocEr`, `Exit00` and `Main` in `vc.h` | Nothing: another compiler passes over the line. A word of `vc.h` could say it to every compiler, which the plan of the flat memory has in F9 |

The script knows six kinds more, which have no place now: `layout`, a
structure that is not as long as the program takes it to be; `inconstant`,
a length that gcc cannot work out when it compiles; `function`, the
address of a function kept as that of data; `undeclared`, a function
that nothing declares or defines; `interrupt`, a function declared
`__interrupt`, which the compiler for 32 bits takes too and `dos32.c`
does without; and `other`, what gcc says that the script does not
know. The kind `loadds` went from the script with the words of the
16-bit compiler, `__far`, `FAR`, `STACK` and `CODE`, which it counted
in its summary while `vc.h` took them for nothing: one of those that
is written again is what gcc refuses, and a place of the kind `other`.

What follows is of the program of 16 bits, and is kept for what it says
of where the places were.

Most of the two casts were a few macros, which
`python3 harness/hostcheck.py --macros` counts as they stand:

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

Since the words of the addresses are in `vc.h` with a body for a
memory without segments ("The portable C in 32 bits: the plan", P1),
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
- Quick execute commands, of the Advanced options, is gone, on the
  user's word of 13 October 2026: the setting, and `ShellRun` of
  `sys.h` with `Int2E` of `dos.c` behind it, by which the commands were
  handed to the COMMAND.COM that the program was started from, through
  its INT 2Eh, in place of a second COMMAND.COM for each
  (`src/portable_c/DEVIATIONS.md`, 14; "The portable C: without Quick
  execute commands" below). The one place where the stack was put back
  by hand went with it.
- The phantom drive B: stays, with its prompt and its setting, for as
  long as DOS is a target, on the user's word of 13 October 2026: what
  the program does for a machine whose one
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
  one step takes on a machine as busy as that. Mended since, and the
  cause found: "T, the time of a wait" in the plan for the 32 bits.

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

The editor of the program of 32 bits has room for a file that the
original's has none for (`src/portable_c/DEVIATIONS.md`, 13), and the
scenario that has the editor refuse such a file, `editor-big`, is not
run for that program, on the user's word of 13 October 2026
(`CLAUDE.md`, "The portable C"): `TREES["portable_c32"]` of
`harness/deviations.py`. The portable C of 16 bits and the C
translation are taken through it as they were. This is what the
scenario alone tested, and so what no test holds of the program of 32
bits; what that program was seen to do with the scenario all the same;
what two scenarios that it passes no longer get it to; and what would
test all of it again. Made at `87291dc`, by reading the scenario and
what ran of the original, and by three trials on the bench that were
not kept. It stands behind the tall screen's section, which is of the
same kind: what a scenario that is no longer run was alone in testing.

**The scenario.** Its steps, counted as the test counts them, from 0
for the start:

| Steps | What they do | What they show |
|---|---|---|
| 1 | The mini status off | Nothing of the editor: the row would show the time of a file that the scenario makes |
| 2 to 7 | `Shift+F4` on the bench's `command.com`, 87,772 bytes; `F7` for a text, `Shift+F7` for it again, `Esc` | The editor on a text that is read from its file in two pieces, and a text found more than 32K into it and once more 50 bytes on |
| 8 to 27 | A file `b` made by `Shift+F5` of that file, three times the same appended, and `country.sys` appended | A file of 381,338 bytes: above the room of the original's editor on the bench, 345,840 when C1 measured it, and in the same 64K |
| 28 | `F4` on `b` | The box "The file b is too large for Edit.", with View and Cancel, over the panels and the box "Reading the file" |
| 29, 30 | `Enter`, `Esc` | View: the viewer on the file, and the panels again |
| 31, 32 | `F4`, `Esc` | The box again, and left by Esc: the panels as they were |
| 33 to 37 | Into `C:\STAMPS`, `F4` on `night.dat`, `Esc` | A file whose directory entry says 9 bytes and that has none: the editor opens it as an empty text |
| 38 to 40 | `F4` on `noon.dat`, `Right Enter` | A file whose entry says a megabyte: the box, and left by Cancel |
| The disks | | `b` as it was made, by its length and the sum of its contents |

**What it alone runs of the original.** By `harness/goldens/NAME.ran`,
the lists of what ran: 36 bytes of `VC.COM` run in `editor-big` and in
no other scenario, all of them in `T00_00`, where the editor opens a
file. A script of a dozen lines made the list: `goldens.read_ran` of
this scenario less that of every other of `scenarios.names(setup=False)`,
and the lines of `coverage.Listing` at what is left.

| In the listing | What | In `T00_00` of `edit.c` | Steps |
|---|---|---|---|
| 613Fh and 6143h | The low words of the room and of the file's length compared, which the original comes to where the high words are the same | A part of one comparison of two numbers of 32 bits, `(u32)ed.EdSpace << 4 <= ed.EdLen` | 28, 31 |
| 614Fh, and 6217h to 6220h, `T00_10` | The box left without View: the window put back from what was kept of it, and the editor left with nothing opened. A new file that is asked about and not wanted comes to the same place, and no scenario has one | `closed:`, three statements | 32, 40 |
| 6223h to 622Dh, `T00_11` | The length of the text made what the file gave, where that is less than its entry says | The three statements behind `if (read.value != count)` | 36 |

What ran does not tell everything apart. By the steps of the other
scenarios, `editor-big` is also the one that has the editor find a
text far into a long one, steps 4 to 6: `editor-search` looks in a
file of three lines. And the one that has the box and View in a window
that is the whole screen. It is not the one with a text read in two
pieces, nor with the box: `help-broken` and `setup-help-file` open the
help file in the editor, 68,836 bytes; and `view-window` has the box
and View for a dirinfo of a megabyte, in the narrow window of the Info
panel, which the program of 32 bits passes, a megabyte being more than
its room too.

**The lengths as numbers of 32 bits.** The file was made to lie in the
same 64K as the room of the original, so that the original has to
compare the low words: that is what "within 64K of what the editor has
room for" in the scenario means, and `editor-big` is the one scenario
that runs those two instructions, the jump taken one way only ("Known
and not dealt with"). Of a C tree it says less. There the lengths are
held against one another in one statement, and the room of both C
trees of 16 bits is in the 64K below the file by C1's figures, 292,288
and 291,360 against 381,338: a comparison of the high words alone, or
of the low words alone, would refuse the file as well. So for the
program of 32 bits what goes untested is not a comparison that the
scenario held to be right for any C tree, but that a file a little
above the room is refused at all; the statement is the same for both
builds, and `view-window` runs it with a megabyte.

**What the program of 32 bits does with the scenario, seen once and
held by nothing.**

- As it is: `python3 harness/compare.py build/portable_c32/VC.EXE
  editor-big --first`, before the scenario was declared, differs first
  after step 28, row 0: `Edit: b`, `Line 1`, `Col 1`, the editor on
  the file, where the original shows its box over the panels. The 27
  steps before it are as the original.
- Without the five steps at the file: a script that put
  `left_out_of={"editor-big": {("F4", "Enter", "Esc", "F4", "Esc"):
  ...}}` into the build's entry of `TREES` in the memory and called
  `compare.compare`. The scenario passed, "as the original without the
  5 steps that are left out, 36 screens and the disks". So that
  program opens the 87K file and finds the text in it, makes `b`,
  opens `night.dat` as an empty text, and refuses `noon.dat` with the
  box and leaves the box by Cancel, as the original does: a megabyte
  is above its room. `closed:` and the three statements of `T00_11`
  ran right that once.

**So this is untested in the program of 32 bits**, each of it compiled
by `wcc386` from statements that the scenario does hold of the program
of 16 bits:

| | What | What still holds a part of it |
|---|---|---|
| 1 | That the editor refuses a file a little larger than its room, and one that is far larger, where the window is the whole screen: the box over the panels | `view-window`: the box for a megabyte, in the window of the Info panel |
| 2 | View from that box: the viewer with its status line on the file that was refused | `view-window`: View, in a window without a status line |
| 3 | The box left by Esc and by Cancel: `closed:` of `T00_00`, which nothing else runs | Nothing |
| 4 | A file that gives fewer bytes than its directory entry says, opened as long as it gave: `T00_11` | Nothing |
| 5 | A text found more than 32K into the editor's text, and found again | `editor-search`, in a short text |
| 6 | The editor on a text of 87K | `help-broken`, `setup-help-file`: 69K, a line of that length |

**Two scenarios that pass and do not get there.** The same memory is
what the editor fills and what a copy reads into, and two scenarios
that are run for the program of 32 bits were made to reach its end.
They pass, since a count of free memory is left out of the comparison
(`IGNORED` of `harness/deviations.py`) and their steps were written to
end on the same screen however much room a build has, for the C
translation, which is full a few keys before the original. Nothing
says that they no longer test what their titles say.

- `editor-full`, "with all of its memory full". Seen, by
  `harness/show.py --changes` with the scenario's steps up to the two
  behind `"ab"`, the count of free bytes in the status line:

  | | The original | The portable C, 16 bits | The portable C, 32 bits |
  |---|---|---|---|
  | With the long line made | 267,852 | 213,372 | 563,836 |
  | After the six steps that put a line back as often as they have keys for | 1 | 1 | 247,855 |
  | After `"ab"` | 0 | 0 | 247,853 |

  So the editor of the program of 32 bits is never full there. What
  that leaves untested: `T51_00` where a line has grown and there is no
  room for it, with its beep and the line cut ("Known and not dealt
  with" has what the original does then), and a byte typed in hex
  behind the last one with no room.
- `copy-big`, "one that the memory does not hold". By the numbers, not
  seen: the file is 526,632 bytes, six times `command.com`, and a copy
  reads into what the editor has for a text, cut to whole pieces of 512
  bytes (`copy.c`, `left`). That is some 291,000 bytes in the program
  of 16 bits, two parts, and 641,000 in that of 32, one. Untested then:
  a file read and written in more than one part, and the question
  whether to overwrite asked with only a part of the file read, which
  the scenario answers with Skip, by the comments at its steps. Since
  F1 of step 2 the room is 524,240 bytes, the panels having 64K each,
  and the file is read in two parts again, the first of 523,776 bytes:
  both are tested again, until the memory is the extender's ("F1, three
  blocks where there was one").
- The list of the file search is in the same memory. No scenario fills
  it in any build.

Both are of entry 13 too, which names them. Neither is declared in the
harness, which has no word for a scenario that is run and does not
reach its point.

**What would test it again.** None of it is for the piece that wrote
this.

| Way | What it tests again | Against |
|---|---|---|
| For the program of 32 bits the five steps 28 to 32 are left out of `editor-big`, `left_out_of`, where the scenario is not run for it now | 3 to 6 of the table, and the far larger file of 1: all but the box in a whole screen for the file that fits, and View from it. Tried, as above: it passes as the program is | The user's word is that the scenario is not run for that program: this is another thing than was decided, and is asked first. And it holds only while a megabyte is above the room: with the memory the extender's, step 2 of the road, `noon.dat` may fit, and steps 39 and 40 then go the way of 28 |
| A scenario of its own with a file that no build has room for, as C1's table has it: eight times `command.com`, 702,176 bytes | 1 and 2 for every build, with nothing declared | A new golden of the original; drive C: of the bench has to hold the file; and it is in the same 64K as no build's room. Seven times `command.com` and `country.sys`, 644,654 bytes, would be in the same 64K as the room of the program of 32 bits and 2,830 bytes above it, by C1's figure: by arithmetic alone, a number that is gone with step 2 |
| `editor-full` with as many steps more as fill 640 KB, and `copy-big` with a file of more than that | What those two no longer reach | Both scenarios change, so both goldens are derived anew, and every tree is taken through the longer steps |

## The portable C: without Quick execute commands

Quick execute commands is gone from the portable C, the setting and
the way through INT 2Eh, on the user's word of 13 October 2026
(`src/portable_c/DEVIATIONS.md`, 14). Every command is run by a
COMMAND.COM of its own, with `/C`. `src/c/` and `src/asm/` keep the
feature, and `quick` is run for the original and the C translation.

**The two dialogs as they are now.** The Advanced options of the
program, a row shorter, and the setup's Configuration without Execute
options, which "Piece E2" above shows:

    ╔════════════ Advanced Options ═══════════╗
    ║  ┌ Key configuration ────────────────┐  ║
    ║  │  [x] Alt alone selects menu       │  ║
    ║  │  [x] Escape toggles panels on/off │  ║
    ║  │  [x] Alternative viewer           │  ║
    ║  │  [ ] Alternative editor           │  ║
    ║  └───────────────────────────────────┘  ║
    ║  ┌ Other options ────────────────────┐  ║
    ║  │  [x] Clear keyboard buffer        │  ║
    ║  └───────────────────────────────────┘  ║
    ║    Press  Space  to change an option,   ║
    ║      arrows to move between options     ║
    ╟─────────────────────────────────────────╢
    ║           [ Ok ]   [ Cancel ]           ║
    ╚═════════════════════════════════════════╝

Every link of both was tried on the bench, each field with Up, Down,
Left and Right: the Advanced options go down their one column and round
by the buttons; in the Configuration, Right from a box of the left half
is the box on its row, or Clear keyboard buffer from Clock down, Left
from a box of the right half the box on its row, Down from Show logical
drive B: Alt alone selects menu, and Down from Clear keyboard buffer
the buttons.

**What went from the program.**

| Where | What |
|---|---|
| `commands_t.c` | The line `Set08_S` and the field of the setting; `Sets_P` has 14 lines and `Sets_B` 6 fields, with the links and the rows of what stands below as they are now |
| `stopts_t.c` | The field of the setting, the last of Execute options, and with it the group: seven lines have the right half of the line five further down and five have spaces there |
| `resident.c` | The branch of `Exec02` that `ExecTyp` chose: the command made a tail without `/C`, the command of `DosChg` handed over first with the command kept in `BatPoin`, `ShellRun`, `SetBrk` and the jump to `Exec07`, which is no label any more |
| `enter.c` | In `F21_31`: DOS asked for the current drive, and the command that goes there written into `DosChg` |
| `start.c` | In `IniData`: DOS asked for the current drive, kept in `DosChg` |
| `sys.h` | `ShellRun`, one function of 97 |
| `dos.c` | `ShellRun` and `Int2E`, fourteen instructions of inline assembly, a function that was `__declspec(naked)` with a `#pragma aux` of registers, and the two words that the stack was kept in, `ShellSs` and `ShellSp` |
| `dos32.c` | `ShellRun` and `Shell_T`, 26 bytes of instructions of real mode, with `LOW_SHELL` and `LOW_KEPT`, 48 bytes of the block below the megabyte: what stood behind them, `LOW_ENDED` and what its instructions read, is that much further down, and the block is 512 bytes where it was 560 |
| `harness/trial32/sys.c` | The call of `ShellRun` on the page of the copies, and its text; `python3 harness/trial32/both.py` passes |

What stays: the byte `ExecTyp` of the main data and of the setup file,
0 in the defaults of `start.c` and of `StPalets`, which nothing reads;
`BatPoin`, a word of the main data that keeps its place; and `DosChg`,
whose last four bytes are where the count and `/C` of the first
command are written, as they were. `Exec02` to `Exec10` were read
against `VC.ASM` once more for what else leaned on the way that went:
nothing did. `SetBrk` is called behind `ProgramRunBare` as it was.

**The scenarios.** `quick` is not run for either build of the portable
C, and is off the record of the program of 32 bits. 34 have a patch
that names entry 14, 20 of them new:

| | Scenarios | Steps for the portable C |
|---|---|---|
| The Advanced options on two screens, Clear keyboard buffer unticked | `command-edit`, `comspec`, `dirinfo`, `ended`, `sizes`, `view-search`, `editor-hex`, `editor-long`, `editor-save`, `editor-words`, `editor-full`, `editor-keys`, `editor-hex-keys`, `editor-buffer`, `editor-dirinfo`, `mouse-edit`, `start-setup`, `start-screen`, `start-memory`, `setup-delay-file`, `setup-help-file` | Four times Down and Space in place of five, by `in_place_in`, behind "F9 o o" |
| The same on one screen | `user-menu`, `user-menu-run`, `view-commands` | The same, with Enter |
| The Advanced options on one screen | `command-line`, `setup-cancel`, `setup-defaults` | The original's |
| The setup's Configuration | `setup-config` | Three steps left out, which set the three settings of Execute options, and Space in place of Down and Space for Alt alone selects menu |
| | `setup-config-mouse` | The click on Quick execute commands left out; four clicks on Alt alone selects menu five rows further up |
| | `setup`, `setup-missing-saved`, `setup-clock`, `setup-left-handed`, `setup-help-screens` | The original's |

**How the patches were read.** By a script, as E2's were, which is not
kept: of each screen of the original's golden it made what entry 14
says, held that against the golden with its patch, and printed what
was left. For the Advanced options: the rows of the box from that of
Quick execute commands down taken from a row further down, the row
that the original's box ends on taken from the screen before the
dialog, in the shadow, and the row below that from the same screen as
it is; the cursor a row further up where it is on Clear keyboard
buffer. For the Configuration: the right half taken from five rows
further down and spaces below, the cursor with it. Where a scenario
had a patch already, the patch of before was made to the golden first,
so that what was printed was what the new patch has otherwise than the
old one with the declaration. What was left over, and is all that the
patches say besides: the cell of the mouse pointer, which stays in the
middle of the screen while the lines move under it, on every screen of
the dialogs where no click has moved it; and two cursors of
`setup-config`, on Full screen where the original has Show logical
drive B:, Left from Clear keyboard buffer. The setup files that
`setup-config` and `setup-config-mouse` leave were held against the
original's byte for byte, by the shim that the notes of `CLAUDE.md`
have: 1B6h, `ExecTyp`, 0 where the original has 1, in both; 1AFh and
1C0h, 1 where it has 0, in the first, as before; and the sum at 1FDh.

**What a later piece should know.**

- A scenario that opens the Advanced options needs a patch for the
  portable C, and one that goes to Clear keyboard buffer there, as
  nearly every scenario of the editor does, an entry of `in_place_in`
  by its name: the top of `harness/scenarios.py` says so where it says
  how such a scenario is written. The same keys in every scenario
  would have been `in_place_of`, one entry for all; they are not the
  same thing everywhere, since `setup-clock` ticks the Clock of the
  setup's Configuration by them.
- The patches of the Advanced options say the same ten rows on some
  fifty screens, as those of the setup's dialogs do theirs: a rule of
  the tree cannot, since the box is a row shorter and the row that it
  no longer covers shows what is behind it, which is another thing in
  every scenario. "Leaving a part of the screen out of the comparison"
  below would not help either: the row is to be held.
- DOS is asked for the current drive in two places fewer, AH 19h, when
  the program starts and before commands are run. No scenario of the
  portable C sees a call of DOS that the program makes for itself; a
  program of a scenario that counted the program's calls would.

**What landing it met.** The piece was made on a tree that had neither
the end by another's hand ("E", below) nor the removal of EGA lines
("The portable C: what EGA lines has left", above), and was rebased
onto both, and then onto the driver's second copy ("C2"). For whoever lands a piece that has waited so:

- The entry's number was 12 when the piece was made and is 14: EGA
  lines took 12 and the editor's room 13. It is in the heading of the
  entry; in the header of each patch that names it, 34 files; where
  the other entries of `DEVIATIONS.md` cite it, "see 14" and "in 14";
  and in `harness/deviations.py`, `harness/scenarios.py`, the README's
  table under "What a user sees no more", these notes and those of
  `CLAUDE.md`. No comment of the source has the number. The test
  `portable_c-scenarios` fails for a patch whose header names an entry
  that does not name its scenario.
- A count that two pieces each took one from is merged by git without
  a word, and is wrong: the README said 97 functions of `sys.h` on both
  sides, each one fewer than 98 for another reason, where there are 96.
  Count again whatever is a count, also where nothing conflicted: the
  table by group there adds up to it.
- The block below the megabyte: E had put `LOW_ENDED` behind the 48
  bytes that went. The places behind them are 48 further down, and
  `Ended_T` needed nothing, being written by their names.
- No patch had to be made again. EGA lines leaves four steps out of
  `user-menu`, which has its patch of this piece: a patch is of the
  original's steps, and the two fit together as they were. The row of
  `user-menu` in entry 12 says that the scenario has a patch for 14.
- Written anew and not merged: `harness/hostcheck.txt` by `--record`,
  the README's figures by `tools/figures.py --write`. In
  `harness/pass32.txt` `quick` went and what the others had added
  stayed.
- A patch that a build has of its own stands in the place of the
  tree's whole, so what the tree's patch gains the build's has to gain
  too, and nothing fails before a machine is run: both fit the golden.
  The driver's piece ("C2") had given the program of 32 bits a patch of
  its own for `start-memory`, which opens the Advanced options: it was
  made again by `python3 harness/goldens.py patch --tree portable_c32
  start-memory`, which called 21 rows new on the two screens of the
  dialog, the rows of the 16-bit program's patch there, and its header
  names 14 beside 1. `second-copy`, the other, opens no dialog.
- Two tables of the harness had an entry of the other piece next to
  the one that went, `OWN_OF_FLAT` of `harness/datacheck.py` and
  `REVIEWED` of `harness/reads.py`, for `Ended_T` beside `Shell_T`: the
  first stays and the second goes.

**A flaw of the tests that was met, and is not of this piece.**
`blank-timed` is on the record of the program of 32 bits and was
"stopped after step 4, Wait*1300 x: the program did not come to wait
for a key within 300 s" in the whole suite, twice, with the machine at
a load of 25 to 30 on its four processors, other agents' tests among
it. With `--limit 1200` it passes, as the original, the scenario having
taken 375 seconds. It is what "Piece H3" has under the time of a step:
a second of the machine takes most of one of the host's while this
program idles, and the 71 of that wait more than 60 on a machine by
itself, which a machine with six times its processors' work makes more
than the 300 that a step has. The limit is one that a test holds a
build to, and was left as it is. It is mended since, and was no limit
of that kind: "T, the time of a wait" in the plan for the 32 bits.

**Open.**

- The layout of the two dialogs, as "Piece E2" has it for the setup's:
  the Advanced options have a group of one line now, Other options
  with Clear keyboard buffer alone, and the right half of the
  Configuration is five rows shorter than the left. Arranging either
  anew is the user's to decide.
- The original's help file still explains Quick execute commands, in
  the page of the Advanced options and in the setup's page of the
  Configuration. The file is the distribution's; a help of the
  portable C's own is not in hand.

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
  it. From S3 on it was `ShellRun` of `sys.h`, which came back, with
  `Int2E` in `dos.c`. It is gone, function and feature (above).
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

**Step 1 is done**, on 13 October 2026. The portable C is that program,
`build/portable_c/VC.EXE`, its build for 16 bits is gone, and `src/c/`
is the 16-bit program. What follows is the plan as it was made and
followed, kept for what each piece met, and it is not brought up: it
speaks of two builds, of `build/portable_c32/`, of `dos.c` beside
`dos32.c`, of `resident.c` and of a record `harness/pass32.txt`, none of
which is there any more. "W and Z1, the switch and the end of the 16-bit
build", near its end, says what became of each, and what was done to
the documents. Every piece has a section, "done, and what a later piece
should know", under its name in the table of "The pieces": P1 to P4 for
the source, S1 to S3 and D1, D2 and E for the system's functions, B1
and B2 for the build, H1 to H4 for the harness, C1 and C2 for the
scenarios that failed, T and L for what was found on the way, W, Z1, Z2
and R1 for the switch. Nothing of the plan is open: what was the user's
is decided, in "Open, for the user", and the next step of the road has
a plan of its own, "The portable C in a flat memory: the plan".

### In short

As it was planned: the two builds were one from the switch on, and the
16-bit one is gone. See the top of this plan.

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
  16-bit build and is the user's to release. The user released it on 13
  October 2026, with the switch, in one piece: "W and Z1" below.

### Taken by the session, on 13 October 2026

Ways of doing step 1 that the road in `CLAUDE.md` leaves open. They
are not the user's word: the session that runs the work took them, the
step being one that follows the setup program without waiting for the
user. The numbers are those that the two lists have between them.

| | What | Taken |
|---|---|---|
| 1 | Whether step 1 keeps segments and offsets as numbers, or has pointers first, under the 16-bit build: see "The strategy" | Numbers in step 1, in a megabyte of the program's own inside the flat memory. They become pointers in step 2, under the 32-bit build alone. It is scaffolding of some 40 lines of `vc.h` and 30 of `dos32.c`, which step 2 takes down again |
| 2 | Where the panels and the viewer's text are in step 1. With 1 they have to lie within that megabyte, so they cannot be DOS's memory below the megabyte, nor a block that the extender gives wherever it likes | A stretch of the program's own, 640 KB, that the three functions of `sys.h` for memory hand out as they hand out DOS's today. A command then finds all of DOS's memory free but the extender's 52 KB. The editor has more room in it than the original's has on the bench, and opens a file that the original refuses: declared on the user's word, entry 13 of `src/portable_c/DEVIATIONS.md`, with `editor-big` not run for this program |
| 5 | Quick execute commands, which was the user's to decide. Under the extender COMMAND.COM's interrupt is called in real mode and comes back; the shell of the bench does not serve it either way (`quick`) | It stayed whole through S3 and D2, some 40 lines of `dos32.c`. Decided on 13 October 2026: it goes, and it has gone, from both programs ("The portable C: without Quick execute commands") |
| 6 | Release and debug, which the user asked for at this step | Two programs of every build: `VC.EXE`, compiled `-os` without debugging information, which is shipped and which every test judges; and `debug/VC.EXE`, compiled `-od -d2` with checks in the macros of the addresses, built always and run by hand. See "The build" |
| 7 | Where DOS/32A's licence text is kept, which the road has in the repository and the trial left open as to where | `LICENSE.DOS32A` beside `LICENSE`, the text of `binw/license.d32` as it is, and the sentence that the road has the README say, under a heading of its own |

Not taken here, since `CLAUDE.md` has them: the extender is DOS/32A,
its stub is linked from the pinned download and never committed, the
file is held to no length, and this step follows the setup program
without waiting.

### Open, for the user

To be asked when the work reaches them, and not before.

What differs in the program of 32 bits from the original, declared or
not, is in one list for the user, with the ways for each: "The program
of 32 bits: what differs from the original, for the user", behind this
plan. 9 below is its first row and 10 its second, and every row of it
is decided: nothing of this table is open any more.

| | What | When, and what is thought of it |
|---|---|---|
| 3 | What deviation 1 says, the program staying in the memory whole, once the program is above the megabyte and a second copy has the memory to run again. That moves toward the original. `second-copy`, `start-memory` and `ended` have patches that say a second copy has none | **Decided, on 13 October 2026** (`CLAUDE.md`, "The portable C"): where the program of 32 bits does again what the original does, it is held to the original's golden, and the deviation says only what still differs. Done by C2 for `second-copy` and `start-memory`, each with a patch of the 32-bit build's own; `ended` passes since E, with the 16-bit program's patch |
| 4 | What the 32-bit program can do when DOS ends it by another program's hand. The 16-bit program has its say first, through its PSP, and leaves registers for the program that started it: `ended` prints them. Under an extender DOS would seem to go back to that program through the extender's own part of real mode, with nothing of the program run. That is from reading and was not tried | The piece that meets it, D2 or M1, tries it first and reports, and the user is asked with what was seen. Thought: declared, and `ended` not run for the portable C; `TakeEnd`, `Exit01` and `Exit05` go with the 16-bit build. **Tried by D2**: nothing of the program runs, but the extender takes itself out and leaves every interrupt of real mode as it found it; what is not put back is that DOS looks for Ctrl-C at every call, and whoever ended the program sees DOS's registers. "Piece D2" below has the table and the ways that there seem to be. **Decided on 13 October 2026**, the second of those ways: the program of 32 bits is told as the 16-bit one is, by a small routine of real mode that the system's functions keep for it (`CLAUDE.md`, "The portable C"). **Done**: "E, the end by another's hand" below; `ended` passes for both programs with the one patch |
| 8 | That the 16-bit build may go, which the road keeps for the user | Before Z1, when both builds pass every scenario from one source. **Decided, on 13 October 2026**, with 9 and 10 declared (`CLAUDE.md`, "The portable C", step 1 of the road): the tests then judge the program of 32 bits as the portable C, and the 16-bit build goes with the switch, in one piece |
| 9 | Whether the program of 32 bits may take all of the memory above the megabyte while a command runs. Its extender does, as it comes: a command is told by the BIOS that there is none, and with an XMS driver none is free, where a command of the original or of the 16-bit program has all of it. A program with the same extender gets memory all the same, a second copy among them. `command` shows it in what `mem` prints, by 128K | Found by C2, which has the figures, and what the extender's setting for it does: "C2" below. Thought: the extender held to what the program and a copy of it need, by the build, which moves the program in the memory and wants the harness told; or declared as it is. The user's word of 13 October 2026 was first that it stays open, to be decided before the 16-bit build goes, with `command` failing and off the record until then. **Decided by the user, the same day**: declared as it is, with no change of the program (`CLAUDE.md`, "The portable C"). **Done**: a point of "The program of 32 bits" in entry 1 of `src/portable_c/DEVIATIONS.md`, "The memory above the megabyte, while a command runs", with the row of `command` there; and a patch of this build's own, `harness/goldens/portable_c32/command.patch`, two rows of what `mem` prints. `command` is on the record |
| 10 | What becomes of a program that a command has left in the memory, with interrupts taken, when the program of 32 bits is left. Its extender puts every interrupt of real mode back as it was when the program started, and writes a line of warning for each: such a program keeps its memory and is cut off, where the original releases it and the program of 16 bits leaves it at work (entry 7 of `DEVIATIONS.md`). And Ctrl-Break it loses while the program still runs. `stay-left` shows it | Found by W, 2 of the list of what differs. The user's word of 13 October 2026 was first "mend it", on being told that it was one bit of the extender's settings; it was not, and told what the mend took and that it still left such a program without INT 21h, the user had it dropped the same day: "L, a program that has stayed in the memory" below has the mend in words and what was read of the extender. It was open then: declared as it is, or another mend, with `stay-left` failing and off the record. **Decided by the user, the same day**: declared as it is, with no change of the program (`CLAUDE.md`, "The portable C"). **Done**: a point of entry 7 of `src/portable_c/DEVIATIONS.md`, "Behind the program of 32 bits such a program keeps its memory and is cut off from its interrupts", with the row of `stay-left` there; and a patch of this build's own, `harness/goldens/portable_c32/stay-left.patch`, three screens of that and the rows that the 16-bit program's patch has for 10 and 14. `stay-left` is on the record, which is every scenario that this program is taken through, 155 of 155 |

### The strategy

As it was planned, and as it was followed until the switch; see the top
of this plan. Of the two programs of one source the 32-bit one is left.

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

As it was planned, for a second build beside the 16-bit one; see the
top of this plan. The build that there is is `add_portable_c` of
`CMakeLists.txt`, once for the program and once for `debug/`.

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
| Declare that the build shows something otherwise than the portable C of 16 bits | `TREES["portable_c32"]` of `harness/deviations.py`, in the words of `Declared.also()`; and for the rows of one scenario a patch of its own, `python3 harness/goldens.py patch --tree portable_c32 NAME`, which is written to `harness/goldens/portable_c32/` and then stands in the place of the portable C's for that scenario. That was tried, with `command` and with `banner`, and the files taken away again. Both are deviations, and asked first. The first that is there is a scenario not run for the build, `editor-big`: `also(not_run={...})`. The test holds of such an entry what it holds of a tree's, that it names a scenario and that an entry of `DEVIATIONS.md` names that scenario, and besides that the portable C is taken through the scenario, or the entry says nothing. It does not hold that the build still does otherwise there, which would take running it |

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
  twice its longest `Wait*ticks` besides, until "T, the time of a
  wait" gave every scenario its wait once: a program that hangs in a
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
  has it (asked since W while the record holds, of the scenarios that
  pass: "W, the switch" below); that every function of a batch is
  linked, or is gone or renamed as the tables say, is held from the
  first day. `FatErr`,
  `Exit01` and `Exit05` are gone from this tree and `Alloc0` is
  `Start`, as S3 and D2 wrote for this piece. The entry of the critical
  error in `dos32.c`, `DeviceEntry`, is static, and the map as
  `coverage.link_map` reads it has no static name: when what ran is
  asked, nothing holds that it ran but the scenarios that show the box.
  (Held since W: the map has static names, `le.Map.named`, and
  `FatErr` is renamed to it for this tree.)
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
  that shows the time of day fails for it. Under load it is more: with
  a load of thirty to forty on the machine's four processors, other
  agents' tests, that wait was not over in the 300 seconds that a
  scenario of the record has for a step, and the test failed for
  `blank-timed`, "stopped after step 4", in three runs of three within
  an hour, one of them in the whole suite. Run alone by
  `harness/compare.py`, at a load of thirty, the scenario passed in
  240 seconds; and the test passed whole, with `--record`, from a
  script of five lines that set `scenarios.STEP_TIME` to 900 and
  called `ported.main()`: `blank-timed` took 403 seconds in it. The
  program was the same bytes throughout. That is a
  test that comes out otherwise under load (`CLAUDE.md`, "Work by
  several agents"), seen when `editor-big` was declared, and not
  mended: `STEP_TIME` of `harness/scenarios.py` is the same for a step
  that waits 71 of the machine's seconds as for one that waits none.
  Mended since: "T, the time of a wait" below has what a round of
  idling costs the host and why, which is not the block moves but the
  processor's mode changed thirteen times a round, and what a step's
  time is counted in now. `short_step` is gone with it: a wait has its
  own seconds for every scenario, in `scenarios.run`.

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
| 5 | What the machine's memory is said to be while a command runs from the program. Looked into by C2: the extender has all of the memory above the megabyte then, which was the user's, 9 of what was open, and is declared as it is since, with a patch of this build's own | `command` | After step 7, `Ctrl+o`, rows 15 and 17: `mem` of DOS says 15,360K of extended memory, in all and free, for 15,232K, and 16,384K of memory in all for 16,256K, 128K more of each. What the extender makes of the memory: not looked into. If it is a figure that follows what the program is and not what it does, `harness/deviations.py` leaves such out, which is asked first | No |
| 6 | A second copy of the program: the patches of the 16-bit build say that it has no memory to run, and here it has. 3 of what is open for the user | `second-copy`, `start-memory` | After step 6, `Ctrl+o`, row 15: `C:\WORK>d:\vc\vc ver` and what follows it, where the patch expects an empty row. `start-memory` after step 12, `Ctrl+o`, row 18: an empty row where the original has `C:\WORK>d:\vc\vc k` | Both |
| 7 | The program ended by another's hand. 4 of what is open for the user, which D2 tried and wrote down. **Mended by E**, below: `ended` passes and is on the record | `ended` | After step 40, `Ctrl+o`, row 0: of the six words that the scenario prints of the registers left for the program that started this one, `2222 1111 EEF6 436D FF75 0202` for the patch's `2222 1111 EE4F #### 0000 0202` | Yes |

So ten of the fourteen are the editor's, 1 to 4, and none of those is
the second copy's doing by what shows; 6 and 7 wait for the user's
word, and may have the driver's blindness in them besides.

Since C1 the nine of 1, 2 and 3 pass, by one mend, and are on the
record; 4 is no fault of the program's, and is declared and not run
for this program, on the user's word: "C1, the editor" below.

**A second copy, which the driver does not know.** It knows since C2,
and otherwise than is thought out here: see "C2" below. In six scenarios
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
  bits and no other. (`notice.py` builds the program of 32 bits since
  W, `--build portable_c32`; `place.py` is of programs of 16 bits by
  what it is for.)

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
| R1, `resident.c` renamed: it is `exec.c`. Done: see the fifth stage of "W and Z1" below | The file of the portable C gets a name for what it is, what the program does first, the commands that it runs by DOS and what it does when it leaves: nothing of it is resident. The C translation's file keeps its name, so the three tables of the harness that know the file by it for both trees have to tell the trees apart, or take the name from the tree: `source="resident.c"` of three batches of `harness/ported.py`, with a text there that says `CtrlC of resident.c`; the keys of `REVIEWED` in `harness/reads.py`, seven entries, and one of the table there of what the portable C has no more; and a text of `harness/datacheck.py`. Then the comments of the tree, `CMakeLists.txt`, the README and the notes that name it | `git mv`, those three files of the harness, the comments | Small | The first run, M1; beside the convergence, which changes the file little | The same bytes of both programs, and the tests that read the source, `portable_c-data` above all | A table that still finds the old name in the C translation and so passes with the entry of the portable C dropped: hold the counts of `reads.py` and of the batches against those before |
| M1, the first run | The program started on the bench, and what keeps it from its first screen and from leaving mended | Whatever it meets, each fix under both builds | Unknown | All of P, S, D, B; H2 | `banner` passes; the first record | |
| C1 to Cn, the convergence | The scenarios that fail, by family: one piece takes those of one cause | By what fails | Three to six pieces, a guess | M1, H3; side by side | The record grows and the 16-bit scenarios pass | See "What could go wrong", 1 and 2 |
| L, a program that has stayed in the memory, when the program is left. Tried, and dropped on the user's word of 13 October 2026: see "L, a program that has stayed in the memory" below | A program that a command has left in the memory was to be at work when the program of 32 bits has been left, as behind the one of 16 bits. The mend took two bits of the extender's settings, the three interrupts given back in real mode too, and a routine of real mode at every end of the program, and left such a program without INT 21h: it is in no commit. What is kept is the scenario, what was read of the extender, and the mend in words | The scenario `stay-left`; this plan | A scenario | E | `stay-left` for the original, the C translation and the program of 16 bits; the program of 32 bits failed it and is held to it by a patch of its own since the difference was declared | What was open, 10 of that table, is decided: declared as it is |
| T, the time of a wait. Done: see "T, the time of a wait" below | Why a wait costs the host four times as much for the program of 32 bits, and what the time that a step has is counted in | `harness/dos.py`, `harness/scenarios.py`, `harness/ported.py` | 60 lines | H3; beside the convergence | The goldens derived again are as they are; a machine that hangs is given up and one that is kept from running is not; the whole suite | A program that hangs on a busy host is waited for until its machine has had its time |
| W, the switch. Done, with Z1 as one piece and without a name for the 16-bit build: see "W and Z1" below | The tests of the portable C judge the 32-bit build, and the 16-bit one under another name beside it; entry 1 of `DEVIATIONS.md`, and what the others say of `ended`; the patches of `second-copy`, `start-memory`, `ended`; the README | `CMakeLists.txt`, the harness's names, `DEVIATIONS.md`, `README.md` | Small | Every scenario passes; H3, H4; the user's word on 3 and 4 above | Both builds pass everything from one source | |
| Z1, the 16-bit build goes. Done; the first bodies in `vc.h` went with the words, and the README was brought up by the fifth stage of "W and Z1"; and `dos32.c` keeps its name: see "W and Z1" below | `dos.c`, `arith.c` with its test, the first bodies in `vc.h`, the build's 16-bit half, what the README says rests on it; `dos32.c` becomes `dos.c` | Those | Small | The user's word | The 32-bit tests | |
| Z2, the words go. Done: the third and fourth stages of "W and Z1" below | `__far`, `FAR`, `STACK`, `CODE` out of the source, by a script | Every file | 1,210 places | Z1 | The 32-bit `VC.EXE` is the same bytes | |

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
   does not do, the rule is the user's. What a round of idling costs
   the host, which is another thing than the machine's time, is in "T,
   the time of a wait".
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
  wants a step in `STEPS` of `both.py`. (`both.py` went with the
  16-bit build: `asked.py` beside it runs the toy with `dos32.c`, has
  `STEPS`, and holds what the toy writes against nothing.)

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

**Quick execute: done.** This is how it was made in this piece, and
is history: the feature has gone from the portable C since, on the
user's word of 13 October 2026, with `ShellRun`, `Int2E` and the way
through them in `Exec02` ("The portable C: without Quick execute
commands"). It was a function of `sys.h` that came back,
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

`ShellRun`, which has gone since with Quick execute commands, and
with it the 48 bytes of the block that it had, was the trial's way: 26
bytes of real mode in the block,
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
  half. (Since the 16-bit build went there is no such half: the script
  is `asked.py`, which runs the toy with `dos32.c` and prints what it
  writes, for a reader to hold against what it printed before.)

### C1, the editor: nine of the ten pass, and one is declared and not run

The first piece of the convergence took the ten scenarios that H3 has
as the editor's, four families by the look of the first screen that
differs. They are two causes. One was in the program and is mended:
nine scenarios pass with it and are on the record, which has 154 of
159. The other is how much memory the program has, which is no fault
of a statement and was the user's to decide: `editor-big`. It is
decided, declared, and the scenario not run for the program of 32
bits: "Cause 2" below.

| | Cause | Scenarios | What became of it |
|---|---|---|---|
| 1 | A step of FF60h, 160 back, added to a pointer into the screen buffer: `T71_00` of `edit.c` | `editor-long`, `editor-full`, `editor-buffer`; `user-menu-run`, `user-menu`, `comspec`, `view-commands`, `editor-save`; `editor-keys`: H3's families 1, 2 and 3 | Mended, the 16-bit program the same bytes |
| 2 | The editor has room for 641,824 bytes where the original has it for 345,840 and the portable C of 16 bits for 291,360: the 640 KB of the program's own, 2 of what the session took | `editor-big`: H3's family 4 | Decided by the user on 13 October 2026: declared, entry 13 of `src/portable_c/DEVIATIONS.md`, and `editor-big` not run for the program of 32 bits. See below |

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

**Cause 2, `editor-big`: decided by the user, declared, and not run
for the program of 32 bits.** The scenario makes a
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
the program does. The ways that there seemed to be, as C1 wrote them
down for the user:

| Way | For | Against |
|---|---|---|
| The scenario makes a file that no build has room for, eight times `command.com`, 702,176 bytes, in place of four times and `country.sys` | It tests what its title says, on every build, and nothing is declared | The file is no longer within 64K of the room of any build, which is what the scenario was written to try: whether the lengths are held against one another as numbers of 32 bits. The golden of the original is recorded anew, of a changed scenario. Drive C: of the bench has to hold it |
| The scenario stays, and a second one has the larger file | Both are tried | The first still fails for this build and wants a patch or to be left out for it |
| A patch of the scenario for this build: the editor opens the file | Says what is so | A deviation: the program does otherwise than the original with the same file, for memory alone. It is the other way round from 3 of what is open, the second copy that has memory to run: there the program of 32 bits does again what the original does, and here it leaves it |
| The program's own memory made as short as DOS leaves the 16-bit program | The scenario passes as it is | A number chosen to make a test pass; and it is gone with step 2, when the memory is the extender's |

What the user decided, on 13 October 2026 (`CLAUDE.md`, "The portable
C"): the editor of the program of 32 bits has room for a file that the
original's has none for; that is declared; the memory is not made
short for it; and `editor-big` is not run for that program, with what
it alone tested listed as untested. Of the ways above that is the
third's declaration, with the scenario left out for the build where
the table has a patch; the fourth, the memory made short, is refused.
Done so:

| What | Where |
|---|---|
| The declaration | Entry 13 of `src/portable_c/DEVIATIONS.md`, which says that it is of the program of 32 bits alone |
| The scenario not run | `TREES["portable_c32"]` of `harness/deviations.py`, the first thing that is said there for that build alone: `also(not_run=...)`. The portable C of 16 bits and the C translation are taken through the scenario as before. The record, `harness/pass32.txt`, did not have it and is as it was; the test had 158 scenarios for that program then, of which 154 passed, and with EGA lines gone, which landed beside this, 155 of which 152 pass |
| What is untested | "The editor's room: untested in the program of 32 bits", further up, beside the tall screen's section, with what the program of 32 bits was seen to do with the steps behind the file, and what would test it again |

C1 had nothing of `editor-big` behind its step 28 known to pass but by
the trial with 320 KB. It is known now, by a trial with the five steps
at the file left out for that program: every other step passes. That
section has it.

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
the table, as they had for `Shell_T`, the table of Quick execute
commands, which is gone since.

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
  the same would be to break the machine on purpose. (What the
  extender puts back then, by its code, is six interrupts whoever has
  taken them, to where they went when it started, Ctrl-Break among
  them: see L.)
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
already has (`short_step` of `harness/ported.py`). Mended since, in
another way than either: "T, the time of a wait" below.

Quick execute commands is out since ("The portable C: without Quick
execute commands"): `LOW_SHELL` and `LOW_KEPT` were 48 bytes of the
block in front of `LOW_ENDED`, and the places behind them have moved
down by that, `Ended_T` being written by their names. `LOW_ENDED` is
at 464, and the block is 512 bytes again; the 560 and the address
`3391:0200` above are of the block as it was when this was made.

### C2, a second copy in the driver, and `command`: done, and what was the user's is decided

Two things. The driver of the tests reads of a copy of the 32-bit
program what is that copy's, and `second-copy` and `start-memory` are on
the record for it, each with a patch of this build's own. And `command`
was not on the record and nothing of it is left out: what `mem` shows
there is a state of the machine that the extender makes, which was the
user's to decide on, 9 of what was open. Decided on 13 October 2026:
declared as it is, and `command` is on the record with a patch of this
build's own. The end of the part on `command`, below, has it.

**The driver and a copy of the program.**

| | |
|---|---|
| `le.Linear.behind(base)`, `copies(count, base)` | Where the extender puts the first object of a program that is started while this one is in the memory, and of each of so many copies, one started by the other: behind the last object as an object is behind the one before, the end rounded up to 16 and 16 more. The top of `le.py` says why: a program of DOS/32A that another one runs takes no memory of its own |
| `driver.COPIES`, `Facts.copies` | A script says the places of 3 copies: `flat_facts` works out `AutoTim`, `Clock` and the stack of the second and the third as it does the first's |
| Event 06 | A count, and for each copy the three addresses |
| `DRIVER.ASM`, `Copy` | Which copy idles: it counts the programs of the script's name that lie in DOS's memory in front of the one that DOS has as the current one, along the line of DOS's blocks from the driver's own, and puts `LSet` on that copy's places. `I28F` asks it at every idle call of the program; `Pump8` and `PutStF` read at `LSet` |
| What the driver writes | New, `C`: a copy idled that the script has no places for, a fourth. `driver.read` raises for it, as for `F` |
| `check_place` of `ported.py` | Holds every place that a copy ran at, by QEMU's log, to one of `copies(COPIES)`, and names the scenarios with a copy and where it was |
| `harness/trial32/copies.c`, and `copies` of `drivercheck.py` | The toy: `auto.c` with a key that runs a copy of the toy through the shell. The check: three copies deep and back, twelve steps; again with the driver told that every copy is where the first is, which is the driver as it was; again four deep |

Where H3's reading and the code differ:

- Not "the PSP of the first call that was the program's". That takes a
  program which is started again elsewhere for a second copy.
- Not by who ran whom either, which DOS would seem to keep, at 16h of
  a PSP. FreeCOM names itself there, also when a program has run it
  for one command: a program of real mode that printed that line of
  PSPs from under a second copy wrote `62A4 5110 CHAIN` and `5110 5110
  COMMAND`, and the line ends there. Written first, and it took every
  copy for the first.
- A list of the PSPs in the order in which each first idled was
  written next and passed everything here. It takes a copy for the one
  before it if that one has never idled, as when a copy is started
  with a copy for its command. The count along DOS's blocks does not
  go by what has idled, and asks DOS nothing but the current PSP,
  which the driver asks at every idle call already (AH 62h, which DOS
  answers on the caller's stack at any time): the list of lists, AH
  52h, is not asked, since an idle call of the program comes from
  inside the critical error too. It takes it that a program lies
  behind the one that ran it, as DOS loads them on the bench, and that
  the driver lies in front of both.
- `I28Q` is diverted for every flat program, where it was for one
  with an `AutoTim`: that is where the copy is told. The other two as
  before, by what the script has of the first copy.
- 115 lines of `DRIVER.ASM`, 50 of `driver.py`, 25 of `le.py`, 125 of
  the check and the toy's 95.

What decided:

- The original's instructions. The driver's file is the old one byte
  for byte from 100h to 577h, everything in front of "A flat program",
  but for three bytes: the two of how far `jmp Init` goes, at 101h,
  and one of how far `je PumpL` goes, at 334h, which a script without
  the event comes to once, behind its last screen. `Init` is the same
  88 bytes, further on. Compared as bytes, which says more than
  `ndisasm` of both. The file is 2,048 bytes, 1,845 of them used where
  1,693 were, and what the driver keeps of the memory is the same.
- Every golden of the original derived again, `python3
  harness/goldens.py derive --together 8`: 167 scenarios, and `git
  status` of `harness/goldens/` clean. Twice, once for the driver with
  the list and once for the driver as it is.
- The whole suite, before and after: the tests of the two trees of 16
  bits printed the same to the line, 283 and 431 lines, but for the
  seconds.
- `python3 harness/drivercheck.py --trace --below`: what it held of
  the four toys before, to the address and the tick, and

  | Toy | What the driver did |
  |---|---|
  | `copies.c`, told that its copies are at 00100010, 00104940 and 00109270 | Each copy says of itself where its two variables are, and that is what the driver was told: `AutoTim` at 001008DC, 0010520C and 00109B3C. The step of `t` ended with "done by itself" in each copy, and again in the second and the first when the copy that each ran had ended. With the second copy's clock on, its cell was blinking at the three steps that ended with that copy idling, and nothing was at the two that ended in the third. The stack written out at each step held addresses of the code of the copy that idled and of no other, with that copy's stack pointer inside what was used |
  | The same, told that every copy is where the first is | The step of `t` in the second copy ended at once, before the copy had done its thing: the driver as it was |
  | The same, four deep | `C`, and `driver.read` raised |
  | `--below` | Of the extender's stack of real mode, 512 bytes, 72 were used with the two variables read at every round, where 66 were, and 84 with a stack written out, as before |

- `check_place`, in the scenarios' test of the 32-bit program: "In 6 a
  copy of it ran as well, started from the command line of the first,
  where the driver reads that copy", each at 001DA1A0h as the program
  was linked then, and at whatever `copies` works out since: the place
  moves with every change of the program's length.

- The time of a step is not the driver's doing. `blank-timed`, whose
  step of 1,300 ticks is the longest that this program idles, was run
  on the program of 32 bits with the driver as it was and as it is, at
  the same time, twice: 128 and 120 seconds, then 79 and 68. But under
  a host as loaded as it was that hour, three agents' suites at once
  and a load of 25 to 30, the same step took more than its 300 seconds
  in three runs of `--record` one behind the other, which wrote nothing
  for it, and the scenario 306 seconds in a suite, where the 16-bit
  program took 80: what H3 has on a machine's second while this
  program idles. A flaw of the bench under load. What it cost is
  mended, by a commit of its own: a scenario has ten seconds more for
  each second of its longest wait before the program counts as stuck,
  `WAIT_TIME` of `harness/scenarios.py`, 1,014 seconds for this one.
  That is a stopgap: the piece that finds out why this program's waits
  cost the host so much looks at it, and may take it out again.
  Why a second of the machine costs the host so much while this
  program idles was not looked into: the extender's way to real mode
  and back and the BIOS's block move at every round, by the look of
  it. It has been since, and the stopgap is replaced: "T, the time of
  a wait" below.

What is not held: a copy that lies in front of the one that ran it in
DOS's memory, which the bench does not make; a program of the script's
name that has stayed in the memory, which would count for a copy; and
the place of a copy if the extender is ever told to take less than all
of the memory above the megabyte, see below.

**The scenarios that start a second copy**, each run by itself on the
program of 32 bits with the driver as it is now. Its code ran at
00100010h and 001DA1A0h in each, as it was linked then.

| Scenario | Against the original's golden as the rules leave it, with no patch | Now |
|---|---|---|
| `second-copy` | Every screen cell for cell but one: behind step 8, F10, rows 5 to 11, the box that names the program, which is 10 of `DEVIATIONS.md` and the second half of the 16-bit program's patch, row for row. The copy runs `ver` and shows what it printed, as the original's does. The disks are the original's | On the record, with `harness/goldens/portable_c32/second-copy.patch`, those seven rows, of 10 alone |
| `start-memory` | The same but for steps 11 to 14. The copy runs `k`, as the original's does, and `k` stays in the memory with all but 75 KB of DOS's. The original's copy cannot get its data back, says "Program too big to fit in memory" and ends. This one needs none of DOS's memory to come back, says nothing and is still there: behind step 12 the command line shows `C:\WORK>d:\vc\vc k` and the two lines of the banner a row further down, rows 19 to 21, with no fourth line; behind steps 11, 13 and 14 the panels are the copy's, with the mini status, rows 20 to 22, which are the rows that the 16-bit program's patch has there. The disks are the original's | On the record, with a patch of its own, those four screens, of 1, which says so for the program of 32 bits now; and, since Quick execute commands went, the two screens of the Advanced options as the 16-bit program's patch has them, of 14 |
| `ended` | Two rows, the one that the scenario prints of the registers behind steps 40 and 43: `2222 1111 EEF6 436D FF75 0202` both times, where the original has `0000 1111 0000 0000 0000 0202` and `0000 0002 0000 0000 0000 0202`, and the 16-bit program's patch `2222 1111 EE4F #### 0000 0202`. Every other screen, and the disks, as the original's. That is what H3 saw: the driver's blindness had no part in it | Not this piece's. On the record since E, which has the program of 32 bits told when DOS ends it: those rows are the 16-bit program's patch's now |
| `comspec` | Before C1's mend of the editor, against the 16-bit program's patch: first otherwise behind step 45, Enter in the editor, `Line 1` for `Line 2`, and everything behind followed from that; nothing of the second copy showed. With that mend it passes | On the record since C1 |
| `start-setup`, `start-screen` | Pass, as before | On the record already |

`unpatched` of `deviations.TREES` is new and nothing uses it: a build
of another tree's source says there that it wants no patch for a
scenario for which that tree has one. Neither scenario came out as the
original's cell for cell, so each has a file of its own instead, with
the rows that still differ; the top of `goldens.py` has both ways.

**`command`: what `mem` says of the memory above the megabyte.** Not on
the record when this was written, and nothing left out; declared since,
and on the record: the end of this part.

What the figure is. FreeDOS's `mem` (its source is in its package of
the pinned image, `SOURCE/MEM/SOURCES.ZIP`, `check_xms` of `mem.c`)
takes for the extended memory in all the sum of what INT 15h with
E820h lists as memory from the megabyte on; if that is nothing, what
E801h says; then function 88h; then, if that is 0 too, the two bytes of
the clock's memory at 17h. What is used of it is that less what an XMS
driver has free, and "Total memory" is that and 1,024K.

What a command is told, by `harness/trial32/above.asm`, run as a
command of each program (`harness/trial32/run.py --extra
VC/ABOVE.COM=build/trial32/above.com PROGRAM '"d:\vc\above" Enter'
Ctrl+o`):

| | From the program of 16 bits | From the program of 32 bits |
|---|---|---|
| INT 15h, 88h | 3B80h, 15,232K | 0 |
| INT 15h, E801h | 3B80h | Refused, with the carry |
| INT 15h, E820h | Six stretches, one of them memory from 00100000h, 00EE0000h long | Refused |
| The clock's memory, 17h and 30h | 3C00h, 15,360K | The same |
| A20 | On | On |
| XMS, DPMI | Neither | Neither |

So the extender has INT 15h while a command runs and says that there
is no memory above the megabyte, since it has taken all of it; `mem`
falls through to the clock's memory, which has 128K more than the BIOS
says, the BIOS keeping that much at the top for itself. The 128K is
the least of it. The figures of that screen that follow the same
thing: the extended memory in all and used, and the total in all; the
total that is used follows it too and is left out already for
following the program's size.

That is not a figure of the kind that "Scenarios" leaves out: it is
what the machine is while a command of the program runs, and a user
meets it. With an XMS driver loaded (`run.py --himem`, FreeDOS's
HimemX of the pinned image), `mem` as a command says:

| | Extended, in all | Used | Free | Conventional, free |
|---|---|---|---|---|
| From the program of 16 bits | 15,232K | 254K | 14,978K | 348K |
| From the program of 32 bits | 15,232K | 15,232K | 0K | 431K |
| From the program of 32 bits with its extender told to take 2 MB | 15,232K | 2,302K | 12,930K | 499K |

A command run from the program of 32 bits finds no memory above the
megabyte, by XMS or by the BIOS: a program with another extender, a
cache, a RAM disk. A program with DOS/32A does, since that asks the
first one for memory, as a second copy does. And the command has 68K
less of DOS's memory, the second row against the third: FreeCOM, which
moves itself out to XMS while a command runs where it can, would seem
not to be able to.

What would change it, tried on copies of the file and not made: the
stub's settings (`tools/stub.c`) have at 10h behind `ID32` four bytes,
FFh each, which are the most memory above the megabyte that the
extender takes, in bytes. With 00200000h there: the rows above with an
XMS driver; without one, 88h says 13,184K to a command, E801h and
E820h are still refused, and `mem` has 13,184K and 14,208K. But:

- The extender then takes the top of the memory and not the bottom:
  with 1 MB the program ran at 00EE0010h, with E1000h at 00EFF010h.
  `le.BENCH` is another then, by the setting.
- A second copy lives in what the first took, not in what is left:
  with 1 MB, which holds the program's 893,307 bytes once, `d:\vc\vc ver`
  ends with "DOS/32A fatal (4003): not enough extended memory to load
  application exec". So the setting is how much the program and what
  it runs of its own kind have between them, and `second-copy`,
  `start-memory`, `comspec`, `ended`, `start-setup` and `start-screen`
  want twice the program, with the copy where `behind()` has it or
  wherever the extender then puts it, which was not looked at.
- `mem` on the bench still shows other figures than the golden, since
  E820h is refused: those would then follow the size of the program,
  which is the kind that is left out.

For the user, 9 of what is open: whether the program of 32 bits may
take all of the memory above the megabyte while a command runs, or is
held to what it needs, and to how much. The user's word of 13 October
2026: it stays open, and is theirs to decide before the 16-bit build
goes; `command` stays failing, for the 128K, and off the record until
then.

**Decided by the user, on 13 October 2026**, the same day and later:
declared as it is, with no change of the program (`CLAUDE.md`, "The
portable C"). What was done for it, and nothing else:

- Entry 1 of `src/portable_c/DEVIATIONS.md`, "The program of 32 bits",
  has a point of its own, "The memory above the megabyte, while a
  command runs": what the original and the program of 16 bits leave a
  command, what the extender leaves it and how a command is told, what
  was seen and what is only from what a command is told, and that the
  setting of the extender was seen on a copy and is not made. The
  entry's reason has the user's word, and its table the row of
  `command`.
- `harness/goldens/portable_c32/command.patch`, made by `python3
  harness/goldens.py patch --tree portable_c32 command` and read: one
  screen, behind step 7, `Ctrl+o`, and two rows of it, 15 and 17, nine
  cells. `Extended (XMS)` has 15,360K in all and used where the
  original has 15,232K, and `Total memory` 16,384K where it has
  16,256K: the clock's memory, 3C00h, in the place of the BIOS's
  3B80h, and that with 1,024K. The total that is used, the next figure
  of row 17, is left out of the comparison as it was. The program of
  16 bits has no patch of the scenario, so there were no rows of
  another entry to carry.
- `command` is on the record, `harness/pass32.txt`.

The extender takes all of the memory as before: `le.BENCH` and
`behind()` stand as they are.

And of the second copy, the same day: this work lands as it is, and no
more goes into scenarios with more than one copy of the program unless
the user asks.

**For the pieces behind this one.**

- A flat program's copy is watched; a scenario may wait on `AutoTim`
  in one, switch its clock on, and count on its stack being looked
  at. Four deep is refused.
- If the extender is told how much memory to take, `le.BENCH` and
  `behind()` are to be found again, by `drivercheck.py --trace` and
  the place line of the scenarios' test, which fails for a copy that
  is elsewhere.
- For W: the README's "When a step is finished" says nothing of a
  flat program yet (H2), nor of its copies.
- `ended` passes since E, which landed beside this piece, with the
  16-bit program's patch: "E, the end by another's hand" above.

### W, the switch: what does not wait is done, and what waits

W is the piece that ends the step: the tests of the portable C judge
the program of 32 bits, and the 16-bit one has another name beside it.
Its row asks that every scenario pass first, and one does not,
`command`, which is the user's and open (9 of what is open). So the
switch itself waits. This is what the row asks, what of it was done
already or is done now, and exactly what is left. Made on 13 October
2026, from `1eb233c`.

**Since this was written**, the same day: the user decided 1 and 2 of
the list of what differs, each declared as it is, and that the 16-bit
build may go with the switch, in one piece (`CLAUDE.md`, "The portable
C"). `command` and `stay-left` are declared, in entries 1 and 7 of
`DEVIATIONS.md`, each with a patch of this build's own, and the record
is every scenario that the program of 32 bits is taken through, 155 of
155. So where the tables below say that the switch waits, for
`command`, for `stay-left` or for the user's word on 8, and where they
say 153 of the 155, they say what was so when W wrote them: the switch
waits for nothing, and it is made as W thought best, at the end of this
section, the program of 32 bits taking the names and the 16-bit build
going in the same change.

**What the row asks, and where each thing stands.**

| The row | Stands |
|---|---|
| The tests of the portable C judge the 32-bit build, and the 16-bit one under another name beside it | Waits: for `command` and, since that scenario was added, for `stay-left`, and see "The switch" below. What those tests hold is held of the program of 32 bits today, under the names `portable_c32-`: the next table |
| Entry 1 of `DEVIATIONS.md` | Done. C2 wrote "The program of 32 bits" there, with the rows of `second-copy` and `start-memory`; E the row of `ended`; W two points more, on the user's word: what the program wants of the machine, and what the extender is in it |
| What the others say of `ended` | Entry 1's row, done by E and C2. Entry 8, what is left of the machine when DOS ends the program: done with the piece L, of the program as it is, the mend of 2 of the list having been dropped |
| The patches of `second-copy`, `start-memory`, `ended` | Done: the first two are C2's, of this build's own; `ended` is one patch for both programs, and W had its reason say what is so of each |
| The README | "The program of 32 bits", "The extender" and the rows of the tests were written by the pieces as they went. W wrote what was left for it: how such a program is driven, "When a step is finished", how its code is known, "What ran", and where its stack is, "The machines". What turns round with the names waits with the switch |
| Entry 6, which D2 and E left for W | Done, on the user's word: "The program of 32 bits" there, the registers and the calls, seen first with `harness/trial32/calls.asm` |

**What the tests of the 16-bit program hold, and whether those of the
program of 32 bits hold it.**

| What | The 16-bit program | The program of 32 bits |
|---|---|---|
| Every scenario as the original's golden, with what is declared | `portable_c-scenarios`: all | `portable_c32-scenarios`: the record, 153 of the 155 that it is taken through. `command` is off it, and `stay-left`, a scenario that was added since and that this program fails (2 of the list of what differs); `editor-big` is not run, by entry 13 |
| Every instruction of a batch ran in the original | The same check, of the goldens | The same, by the same run |
| Every C function of a batch ran | Held | Held since W. It was asked only "once every scenario passes", which `command` kept from ever being so. It is asked now while the record holds, of what ran in the scenarios that pass. A run of them all was kept and asked first: each of the 169 functions that the batches name for this program ran in the 153, and none of them shares its place in the map with another function, which `wcc386 -os` may do to two that have the same instructions |
| The entry of the critical error ran, `FatErr` | Held: a function of a batch | Held since W, as `DeviceEntry` of `dos32.c`. It was listed as gone, for being static; the linker's map has static names, and a function that a tree has under another name may be static now (`RENAMED`, `check_translation`). It ran in five scenarios, `not-ready`, `device`, `tree-choose`, `find-drives` and `attributes-directory`, and without those the check fails, which was tried |
| `Exit01`, `Exit05`, where DOS goes on when it has ended the program | Held: functions of a batch | Not as functions: they are 28 bytes of real mode in a block below the megabyte, `Ended_T`, of which QEMU's log of the program's code has nothing. That they ran is held by the registers that `ended` shows |
| Half of the stack left in every scenario | Held: 4,096 bytes | Held, of 16,384: 14,790 left at the least, in `attributes-directory`, where the critical error is 512 bytes deeper |
| The layout that the machine of the tests asks | Code below F000h in its segment, none within 4K of the stack, the stack last | The first is not asked of a flat program. The other two in their flat form, and what the words of the addresses rest on: `harness/layoutcheck.py`, asked by the scenarios' test before it runs anything and by `portable_c32-layout` |
| Where the driver reads | In the segment that DS is on | `check_place`: the program and every copy of it ran where the driver was told, by QEMU's log |
| The tables against the original's | `portable_c-data`, with `harness/reads.py` | `portable_c32-data`, the same tables and the same reads, field by field where an object has pointers |
| The comparison itself | A cell changed or an entry of a disk gone is reported | The same, by the same run |
| What is declared fits the harness | `check_declared` | The same, and that what is said for the build alone names a scenario that the portable C is taken through |
| The source as another compiler reads it, and its arithmetic | `portable_c-host`, `portable_c-int16` | They are of the source, not of a build, and read `dos32.c` with the rest. Nothing to take over |
| `__U4M` and `__U4D` | `portable_c-arith` | The program has neither: `wcc386` multiplies and divides by itself. The test goes with `arith.c`, in Z1 |
| The length of the file | Held to none | Held to none |
| The debug program | There is none of 16 bits | `portable_c32-debug-layout` alone. No machine runs it, since it takes more instructions and a scenario that shows the time of day may fail in it alone |

So nothing that the 16-bit program's tests hold goes unheld of the
program of 32 bits today but what `command` shows, and what waits in
the list of what differs.

**What W did**, each a commit:

- The list, "The program of 32 bits: what differs from the original,
  for the user", behind this section, and what the user decided of it.
- `harness/ported.py`: which functions ran, and `DeviceEntry`, as the
  table has them. Tried without a machine, on what a run of all the
  scenarios had kept: the check passes, fails with the five scenarios
  of the critical error left out, and says of the two trees of 16 bits
  what it said.
- `harness/notice.py` built nothing any more, for either tree: the
  build has made a tool of `tools/stub.c` since B2, and the copy that
  the script builds had no such file. Mended. And `--build
  portable_c32` has it build and run the program of 32 bits, through
  the scenarios of its record: when the 16-bit build goes, that is the
  only program that a change to the tree can be tried on.
- The README, as the first table has it.
- The reason in `harness/goldens/portable_c/ended.patch`.
- `harness/trial32/calls.asm`, and with what it showed the two parts of
  `DEVIATIONS.md`.

**Not done, and why not.**

- `harness/place.py` says where the stack of a program of 16 bits ends
  in its page of the memory, for the driver's choice of where DOS loads
  the program. The stack of a flat program is where the linker put it
  above the megabyte, whatever DOS does, and the rule that it answers
  to is `layoutcheck.py`'s. It stays a tool of the 16-bit trees and of
  the original, and needs nothing.
- `harness/trial32/both.py`, the toy that holds `dos32.c` against
  `dos.c` function for function, is run by hand and passes, 28 screens.
  It could be a test of the build for as long as there are two files
  to hold against one another, which is until Z1. Not made one: it
  builds with a script of its own and runs two machines, and what it
  guards, a change to `dos32.c` that makes the two answer otherwise,
  has not been lost to yet.
- `CHECKED`, the checks of the debug program, which B2 left to whoever
  first has a wild read to find, and C1 argued for. Not W's.

**The switch, which waits.** A piece of its own when it can be made.

| | |
|---|---|
| Waits for | `command` on the record, by whichever way the user decides 1 of the list; `stay-left` on it, by whichever way the user decides 2, the mend that was tried for it having been dropped (L); and best the user's word on 8 of what is open, whether the 16-bit build may go, since that decides whether the switch is worth making at all: see below |
| The build | The targets and directories of `CMakeLists.txt` change names: `portable_c` is the program of 32 bits, with `debug/` in it, and the program of 16 bits a name of its own, `portable_c16` say. The names of the tests follow the directories |
| The harness's names | `deviations.SOURCE` turns round, the 16-bit build being the one of another tree's source. `TREES`: the tree's entry is then what is declared of the program of 32 bits, with `editor-big` not run, and the 16-bit build is that with something taken away, which `Declared.also()` cannot say: it only adds. Either a common entry that both add to, or a way to take away. `GONE` and `RENAMED` of `ported.py` turn round in the same way, the entries of `dos.c` being the 16-bit build's. `RECORDS` loses the tree and `harness/pass32.txt` goes, once the record is every scenario. `goldens.TREES`, and the patches: those of `harness/goldens/portable_c32/` become the tree's, and the tree's of `second-copy` and `start-memory` go to a directory of the 16-bit build's name. `tools/figures.py`, `harness/notice.py`, `harness/place.py`, `harness/datacheck.py` and `harness/reads.py` by the names that they have for a build's directory |
| The documents | The README is written of the program of 16 bits throughout, "that everything else here is said of": "The figures", "Staying in the memory whole", "The system in one place", "What rests on Open Watcom", "Inline assembly". `DEVIATIONS.md` says "the program of 32 bits" for the one that is other, in entries 1, 6 and 13. Both turn round, and that is the larger part of the work |
| Size | Not the "small" of the table of pieces: half a day to a day, most of it the documents and the way for a build to be declared to do less than its tree |
| Right when | Both builds pass everything from one source, under the new names; every golden derived again leaves `harness/goldens/` clean; the 16-bit `VC.EXE` is the same bytes |

What W thinks of it, for the session and the user: the switch gives the
16-bit build a name and a place in every table, to be deleted by the
next piece. If the user releases the 16-bit build when asked, the two
are better one piece: the program of 32 bits takes the names and the
16-bit build goes in the same change, with no name invented for it, no
`also()` that takes away, and the documents turned round once. The
switch as the plan has it is worth its work only if the two builds are
to stand side by side for a while behind it.

**For the piece behind this one.**

- A tree with a record is asked which functions ran as soon as its
  record holds. A build that starts a record from nothing fails that
  for every function that its passing scenarios do not reach: such a
  build wants the condition in `main()` of `ported.py` put back as it
  was for its first days, one that waits for every scenario.
- `harness/trial32/calls.asm` says what DOS is asked, call for call. A
  change to `dos32.c`, or to the extender's settings, that is to leave
  the calls as they are can be held by it: the two lists of this piece
  were made by the line at its top and compared by AX.
- A patch that two builds are held to says in its reason what is so of
  each: `ended` did not, and nothing fails for a reason.

### W and Z1, the switch and the end of the 16-bit build: done

On the user's word of 13 October 2026 the switch and the end of the
16-bit build are one piece (`CLAUDE.md`, "The portable C", step 1 of
the road), as the section above thought best: the program of 32 bits
took the names, and the 16-bit build went in the same piece, with no
name invented for it. Made from `038d8e6`, in five stages, of which the
first two are done and landed, `d297d35` and `7e73258`, and are what
this section says; the other three are at its end, with what each has
to do and the places as exactly as they are known.

**What is called what now.**

| | Before | Now |
|---|---|---|
| The program | `build/portable_c/VC.EXE`, 16 bits, by `wcc` and `add_portable_c`; `build/portable_c32/VC.EXE` and `debug/VC.EXE` there, by `wcc386` and `add_portable_c32` | `build/portable_c/VC.EXE` and `build/portable_c/debug/VC.EXE`, by `wcc386` with DOS/32A and `add_portable_c`. They are the files that were in `build/portable_c32/`, byte for byte: the SHA-256 of each is what it was |
| The targets | `portable_c`, `portable_c32`, `portable_c32-debug` | `portable_c`, `portable_c-debug` |
| The tree, to the harness | `portable_c`, the 16-bit program; `portable_c32`, "another build of a tree's source", with `deviations.SOURCE`, `Declared.also()` and `unpatched` | `portable_c`, the one tree, by the name of its directory in `build/` as before. `SOURCE`, `source()`, `also()` and `unpatched` are gone, with `goldens.own_patch_path` |
| What is declared | `TREES["portable_c"]`, and `TREES["portable_c32"]` made of it with `editor-big` not run | `TREES["portable_c"]`, with `editor-big` among what is not run |
| The patches | `harness/goldens/portable_c/`, and `harness/goldens/portable_c32/` for the scenarios that the program of 32 bits shows otherwise | `harness/goldens/portable_c/` alone. The patches of the 32-bit build's own took the places of the tree's or were added to them |
| The record | `harness/pass32.txt`, with `ported.RECORDS`, `check_record`, `--record`, `--limit` and `SHORT_STEP` | None: the test holds the tree to every scenario that it is taken through, as the 16-bit test did. `compare.py` keeps `--first` and `--limit`, for a scenario by hand |
| The tests | `portable_c-data`, `-arith`, `-scenarios` of the 16-bit program; `portable_c32-data`, `-scenarios`, `-layout`, `-debug-layout` | `portable_c-data`, `portable_c-scenarios`, `portable_c-layout`, `portable_c-debug-layout`. `portable_c-host`, `portable_c-int16` and `harness-le` as they were, and `asm-bytes`, `c-data`, `c-arith`, `c-scenarios` untouched |
| `GONE` and `RENAMED` of `ported.py` | The tree's, and the build's made of them | The tree's, with what was the build's in them: `Exit01` and `Exit05` gone, `FatErr` as `DeviceEntry`, `Alloc0` as `Start` |
| `OWN_OF_FLAT` of `datacheck.py` | `Ended_T` and `NoStart_S`, of `dos32.c` | In `OWN_OF` |
| `tools/figures.py` | The rows of the 16-bit file, then those of the program of 32 bits | One program. What a program of 16 bits keeps of DOS's memory while a command runs, and what that leaves a command on the bench, are no rows any more: neither follows from the file of a program that is loaded above the megabyte |
| `harness/notice.py` | `--build portable_c32` | Gone: the tree's build is that program |
| `harness/place.py` | The original and both C trees | The original and the C translation: it is of programs of 16 bits |

**What each thing that the two sets of tests held is held by now**, the
table of the section above read again: every scenario, what ran of the
original and of the C, the stack, the comparison itself and what is
declared, by `portable_c-scenarios`; the layout by that test and by
`portable_c-layout`; the tables by `portable_c-data`; `__U4M` and
`__U4D` by nothing, the program having neither. Which C functions ran
is asked of every scenario again, as of a tree without a record.

**What went with the 16-bit build.**

- `src/portable_c/dos.c`, the system's functions for DOS in 16 bits,
  and `src/portable_c/arith.c`, the compiler's two helpers, with the
  test `portable_c-arith`. `harness/arithcheck.py` stays, for `c-arith`.
- Of `CMakeLists.txt`: the function that built the 16-bit program, with
  its three groups of data, its two code segments and its list of the
  files of the second; the lists of the files that were of one build
  alone; and what `c_compile` had for a tree with a `sys.h`.
- Of the harness, what was there for a program of 16 bits as the linker
  writes it, which only the portable C was: the limits of
  `ported.check_layout` (`CODE_END`, `PAGE`: a code segment below F000h,
  no code within 4K of the stack, the stack last); in `coverage.py`,
  `made_by_mkexe`, the header's table of relocations and the second of
  `STACKS`; in `driver.py`, the main data at offset 0 of the segment
  that DS is on and the stack by the header's SS; in `datacheck.py`, the
  classes `MAIN` and `STACK` of an object file of 16 bits; in
  `reads.py`, the file of the system's functions that a program is not
  built of. A program of 16 bits is the C translation's now, a file that
  `tools/mkexe.c` made, and nothing else.
- `harness/trial32/both.py`, which held `dos32.c` against `dos.c`
  function for function, and `build.sh sys 16`. What is kept of it is
  `harness/trial32/asked.py`: it builds the toy `sys.c` with `dos32.c`,
  runs it through the same steps and prints its 28 pages, for holding a
  change to `dos32.c` against what the toy printed before. Nothing
  judges those pages any more. `casts.py` and `added.py` are of the
  source and stay.
- Nothing of `tools/`: `mkexe.c` is the C translation's and `stub.c`
  the flat program's. The 16-bit link of the portable C used no tool.

**`dos32.c` keeps its name.** The plan had it become `dos.c` (the row
Z1). It does not, for three reasons:

- The harness has tables that are one for both C trees and go by the
  name of a file, and the C translation has a `dos.c`, another file:
  `REVIEWED` of `harness/reads.py`, where an entry for a file that does
  not name its constant fails the test, tells the two apart today by
  their names alone (`SYSTEM_FILE`). With one name the tables would
  have to be told which tree an entry is of, which is the work that the
  row R1 has for `resident.c`.
- The notes of `CLAUDE.md`, this document and the history say `dos.c`
  for the file of 16 bits and `dos32.c` for this one, some hundreds of
  times. With the names as they are, each of those still says which
  file it means.
- Step 6 of the road names the files of the system's functions anew,
  `_dos.c` and `_posix.c` beside common headers. A rename now is made
  twice.

What reads wrongly for it: the rule of `CLAUDE.md` that says "Today
that place is `sys.h`, with `dos.c` behind it". The session was told.

**Where the plan and the code disagreed.**

- The plan had the 16-bit build keep a name beside the program of 32
  bits after the switch. It has none: see above.
- The plan's row Z1 has "the first bodies in `vc.h`" go with the build;
  they go with the words, in the stage below, so that the deletion of
  two files and of a build is one commit and the change to every file
  of the tree another.
- An object that the compiler for 16 bits left in `build/portable_c/`
  has the name of the one that `wcc386` is to write there, and the
  build did not see that the command had changed: it would have linked
  the old objects. Every object of the portable C follows a file now,
  `compiled.txt` of its directory, which the configuration writes anew
  only when the compiler's command is another. So a checkout that is
  pulled across this piece builds without being cleaned. What it keeps
  that nothing reads: `build/portable_c32/`, and `arith.obj`, `dos.obj`
  and their listings in `build/portable_c/`.

**Nothing is left of the piece.** Its fifth stage, the documents, was
the last, and is done: see there. The
first two stages are on `main` at `7e73258`, and the third and the
fourth, Z2, in the three commits that follow "Write down how step 2 of
the road is divided": "Take the words of the 16-bit compiler out of the
portable C", "Have the host check and the figures follow the words" and
"Join the lines that the words had broken".

**The checks, and one trap.** `sha256sum build/portable_c/VC.EXE
build/portable_c/debug/VC.EXE` is `a02a28c6...a4f945` and
`3c2acbdf...eee1a9`, at `7e73258` and behind each of the three commits
of Z2; the whole suite is `ctest --test-dir build -j8`, four and a half
minutes, eleven tests; `python3 harness/goldens.py derive` leaves
`harness/goldens/` clean. The trap is of the build: an object is
compiled again when its file, a header, or the compiler's command has
changed (`compiled.txt`), and for nothing else. A change to
`CMakeLists.txt` that changes how the portable C is linked, and not how
it is compiled, links again by itself; one that is to compile every
file again and does not change the command, does not: `rm -rf
build/portable_c` then.

3. **The words' 16-bit bodies (Z2): done.** What it did, and what
   whoever goes on should know:
   - `__far`, `FAR`, `STACK`, `CODE` and `TEXT_FAR` are in no file of
     `src/portable_c/` and `vc.h` defines none of them; `__near`,
     `__loadds` and `__interrupt` stood nowhere. `vc.h` has no
     `__I86__`: `u32` is an int, `LenStk` 4000h, `OFFSET_IN` is
     `(u16)offsetof(type, field)`, and each word of the addresses has
     its one body, below, the second of before word for word. Nothing
     of step 2 of the road went in.
   - The words were nothing to `wcc386`, since `vc.h` defined them
     away, so that taking them out leaves the bytes was certain. Three
     things besides were not, and the same two hashes held them: some
     130 casts `(struct Where STACK *)&where`, which are the pointer
     that the address is already and went, where the compiler would
     have refused a type that changes; sixteen that made a far pointer
     of a near one of the same type, `(u16 __far *)BasAdr(...)`,
     `(const u8 __far * STACK *)&text`; and `STACK_C`, by which
     `stack.c` kept `vc.h` from declaring its array far. Two casts of
     that form stay, `*(u8 *)&text->first` of `format.c`, which write
     one byte of a word. Sixteen lines that went on in a second for the
     length of the words were joined, and the build for finding a fault
     is the same bytes for that too: its file has no numbers of lines.
   - A script did it, file by file, and is not kept: a word goes with
     the space behind it; a comment or a backslash that ended a line in
     a column keeps the column; a line that went on inside a
     parenthesis, under what followed it, stands there again. What it
     got wrong was read in the diff: a comment's column on a line that
     had only moved, seven times.
   - The 17 `#pragma aux` and the two intrinsic functions stay, as the
     plan had it: twelve of `dos32.c`, and the five that say `aborts`.
     So do `__declspec(naked)` and `_asm` of `dos32.c`, the two
     `#pragma data_seg` and the seven pairs of `#pragma pack`.
   - The comments went with the words. Those of far and near pointers,
     of the three groups and of `DS` say what is so in a flat memory:
     the tops of `vc.h`, `tables.h`, `stack.c`, `string.c`, the main
     data in `main.c`, `AsmSeg` of `misc.c`. `dos.c` is `dos32.c`
     wherever the file of the system's functions is meant. `dos32.c`
     itself pointed to `dos.c` in eighteen places, for the reason of a
     thing or for "as it does": each says the thing now, or says "the
     program of 16 bits" where that is history which explains what is
     there. The two that leaned on a comment of `dos.c` have what it
     said: why a program run gets the bytes of the main data at 5Ch and
     6Ch, and where DOS goes on when the program is ended, `Exit01` and
     `Exit05` of the original.
   - `harness/trial32/sys.c` has no line for 16 bits either;
     `harness/drivercheck.py` says "Nothing wrong" and
     `harness/trial32/asked.py` prints its 28 screens. `casts.py`
     there still names `dos.c` and a line with `__far` among what it
     sorts: it is the trial's tool, and finds neither.

   **The words of the addresses**, each with its one body (`vc.h`,
   "The words of the addresses"). They are the megabyte of step 1, and
   what step 2 takes down:

       ARENA                    ((u8 *)&md - 0x10000L)
       MAIN_SEG                 0x1000
       AT(segment, offset)      (ARENA + ((u32)(u16)(segment) << 4)
                                 + (u16)(offset))
       SEG_OF(pointer)          ((u16)(((const u8 *)(pointer) - ARENA)
                                 >> 4))
       OFFSET_IN_SEG(segment, pointer)
                                ((u16)((const u8 *)(pointer)
                                 - AT(segment, 0)))
       PANEL(panel)             ((struct WCBdef *)AT(panel, 0))
       TABLES_BY(text)          SEG_OF((text) - 0x8000)
       OFFSET(object)           ((u16)((const u8 *)(object)
                                 - AT(TABLES_BY(Title_S), 0)))
       NAMED(names)             OFFSET(names)
       NUMBER_OF(names)         ((u16)(size_t)(names))
       NEAR(pointer)            ((u8 *)(pointer))
       MD_PLACE(pointer)        ((u16)((const u8 *)(pointer)
                                 - (const u8 *)&md))
       MD_POINTER(place)        ((u8 *)&md + (u16)(place))

   Behind the block: `TRELIST`, which is `MD_PLACE` of a field of the
   tree's buffer, and `ENTRY`, which cuts what it is handed to 16 bits.
   `TEXT(segment)` of `view.c` is `AT(segment, 0)`.

4. **The checks that read the source: done.**
   - `harness/int16.py` knew what is written in a word of the addresses
     by where `__I86__` was not defined. It knows the words by their
     names now, `WORDS` there, and what is written in the `#define` of
     one is no place. **A word that step 2 adds to `vc.h`, or a body
     that it changes, is read by that list**: a new word that shifts or
     takes one address from another is a place at every use until its
     name is there, 209 of them when the list was empty. It reads
     `dos32.c` like any file, `ONE_BUILD` being gone, and found one
     place in it, `OwnSet`, which has its reason. Its top says what the
     test is for with one build left: the code was written and judged
     with an int of 16 bits and is compiled with one of 32, a place is
     where the program can do otherwise than what it was translated
     from by the int alone, and it is where step 2 widens; the plan of
     the flat memory makes it the finder of the widths (N0).
     A place is known by the text of its line, so the one line of the
     record that had `__far` in it, `StHelp` of `sthelp.c`, was a place
     gone and a place new, and its reason was carried over by hand.
   - `harness/hostcheck.py`: the record did not move, since the words
     were never in it, only in the summary. The function that counted
     them is gone, with the row of the README's table that
     `tools/figures.py` made of it and the kind `loadds`; one of those
     words written again is what gcc refuses, a place of the kind
     `other`. `interrupt` stays a kind: the compiler for 32 bits takes
     the word, and `dos32.c` does without. "The portable C: what
     another compiler says of it", above, has the kinds and counts as
     they stand and what each waits for.
   - `tools/calls.py` needed nothing. `harness/trial32/casts.py` and
     `added.py` run as they did: 39 places of `dos32.c`, 160 places.

5. **The documents: begun, and what is left.** Each says what is so
   now, once; what was true of the 16-bit build alone goes, or is kept
   as a sentence of history where it explains something that is still
   there.

   **Done**, in the four commits from "Rename resident.c of the portable
   C to exec.c" on:
   - R1. The file is `src/portable_c/exec.c`, with a comment at its top
     that says what it was called and why. `harness/reads.py` keeps the
     entries of `REVIEWED` and `GONE` under the C translation's name,
     one for both trees as before, and `FILES` there says which file of
     a tree that is; `harness/ported.py` has a `FILES` too, for what a
     batch prints. Without the first the data's test fails for each of
     the seven places, which was tried. **The program is not the bytes
     that it was**: the build links the files in the order of their
     names, so `exec.obj` stands elsewhere among them than
     `resident.obj` did. The same objects linked by hand in the order
     of before give the file of before, byte for byte, so the name is
     all that differs. The hashes above are of before; they are
     `9887141a...567d9452` and `fbd3db81...9f3a8ff5` since. Whoever
     adds or renames a file of the tree moves the program in the same
     way, and a piece that is to leave the bytes alone holds them
     against a build of its own first commit.
   - `README.md`. "The program of 32 bits" is folded into "The portable
     C" and "Staying in the memory whole"; "What rests on Open Watcom"
     has a table of its own for the flat program and "Inline assembly"
     the thirteen pieces of `dos32.c`; "The system in one place" is of
     `dos32.c`; the tests are the eleven. What went from the tree is
     named there as the C translation has it.
   - `src/portable_c/DEVIATIONS.md`. Entry 1 and entry 6 say what the
     program does; the column of entry 6's table for what the program
     of 16 bits handed over is gone, since `dos32.c` hands over 0 in
     every register that a call takes nothing in. Entry 13 is as it
     was, title and all. One thing went that nothing says in its place:
     how a DOS before 3 is left, which was of the program of 16 bits;
     the extender does not start on one, and what a user is told there
     was not looked at ("The program of 32 bits: what differs from the
     original, for the user", its last lines).

   **The rest of it, done** in the three commits from "Say in the
   notes what the end of the 16-bit build changed" on. Documents and
   comments only: `VC.EXE` is the bytes that it was,
   `9887141a...567d9452`, and the tests that start no machine pass.
   - This plan, beyond its top: "In short", the strategy and "The
     build" each begin with a line that says they are as they were
     planned and points to the top. Nothing of them is rewritten. The
     two places that told a reader to use `both.py`, at the ends of
     D1's and D2's sections, say what there is in its place. "What a
     trial on the bench showed" names `both.py` nowhere and is as it
     was.
   - `CLAUDE.md`, the notes. The note that said how the others are
     read stands first in "Notes on the portable C" now, with what it
     lacked: the files that went or were renamed, the compiler and its
     model, the words that are gone, and what a check by the bytes of
     `VC.EXE` still is and no longer is. "Moving a file to the system's
     functions" has such a paragraph of its own behind its first. With
     those two a note of history is left as it was written, `dos.c`,
     `resident.c`, far pointers, the same bytes and all. Mended in
     place is what would send a reader wrong all the same, some thirty
     places: what `add_portable_c` takes; the command that compiles
     one file, which was `wcc`'s; the copy of a build, which was to be
     put through `arithcheck.py`; the cast of `portable_c-int16`, which
     is code now and no longer "the same bytes"; `OWN_OF_FLAT`, which
     is `OWN_OF`; the notes on the layout's, the data's and the
     scenarios' tests of the program of 32 bits, which named
     `build/portable_c32`, the tests `portable_c32-...` and the record;
     `notice.py --build`; in the table of the calls the five rows that
     named something of `dos.c`, `MemOwn`, `ProgramRun`, the PSP at
     0Ah, 51h and the vectors; where a new function is written; and
     `both.py`. The table was held against `sys.h` by a script: every
     function that it names as what a call is, is declared there, but
     for `ShellRun`, `ScreenPrintAll` and `ScreenSmallFont`, which it
     says went with their commands; and every function of `dos32.c`
     that is not static is declared in `sys.h`, but `Start`.
   - The harness and the tools. The tops had been brought up by the
     pieces that changed the scripts; every line of `harness/`,
     `harness/trial32/` and `tools/` that names the tree `portable_c32`,
     the record, 16 bits, `dos.c`, `resident.c`, `arith.c`, `both.py`,
     `__far` or `STACK` was read, 157 of them, and what is of the C
     translation or of the original stays. Mended: a comment of
     `harness/ported.py` that had the entry of the critical error in
     `dos.c`, one of `harness/trial32/ended.asm` that named the 16-bit
     file, and the top of `harness/scenarios.py`, which had where the
     16-bit portable C shows its first screen.
   - Not done, being code and no comment: `harness/trial32/casts.py`
     sorts its places by names that are gone, `dos.c`, `DOS_AT` and
     `(void __far *)Stack`. It runs, and has one class left, the 39
     places of `dos32.c`.
   - Not the notes' and so not touched: the rule of `CLAUDE.md` that
     says "with `dos.c` behind it", and the rule of "Scenarios" that a
     C tree takes up to a sixth more instructions than the original,
     for which the next point has what was counted.
   - **The machine's clock in the portable C, counted for that rule**,
     which the session has changed by it since: the section behind
     this one, "The machine's clock of the portable C against the
     original's", has the table, what was found of a search for a
     text, and how it was counted.

   What the list said before any of it was done:
   - **R1 goes with this stage: `resident.c` of the portable C becomes
     `exec.c`.** The name was proposed by the agent of Z2 and accepted
     by the session: the largest part of the file is `Exec`, the
     running of the commands, with `Exec02`, `Exec10`, `NotFind` and
     `OutStr`, and around it what the program does first, `Alloc3`, and
     when it leaves, `SetBrk`, `TakeEnd`, `Exit00` and `AlocEr`;
     `start.c` and `main.c` are taken. The file has 371 lines. The C
     translation's file keeps its name, so a place that means
     `src/c/resident.c` stays. Where the name stands, as `grep -c
     'resident\.c'` has it at the end of Z2:
     - The three tables of the harness that know the file by its name
       for both trees, and have to tell the trees apart or take the
       name from the tree (the row R1 of the table of pieces has the
       risk: an entry that still finds the old name in the C
       translation and so passes with the portable C's dropped; hold
       the counts of `reads.py` and of the batches against those of
       before). `harness/ported.py`, five lines: `source="resident.c"`
       of three batches (1012, 1033, 1057), the text "CtrlC of
       resident.c" (1246), and line 26, which is of `src/c`.
       `harness/reads.py`, five: the keys of `REVIEWED` at 232, 233 and
       235, a comment at 409, and `BadFl_S` at 466, in the table of
       what the portable C has no more. `harness/datacheck.py`, two
       texts, at 207 and 245.
     - `harness/deviations.py` (432) and `harness/coverage.py` (17, 65,
       410) say `src/c/resident.c` and stay. `harness/le.py` (57) says
       "two routines of resident.c" take the segment of something, of
       the program of 32 bits: read whether that is still so.
     - `CMakeLists.txt`, five lines, 215 to 276, all of `add_c_tree`,
       the C translation's build: they stay. The portable C is built of
       every C file that is in its directory, so `git mv` is all that
       its build wants. `tools/mkexe.c`, six, is the C translation's
       too.
     - Comments of the tree, ten in seven files: `dos32.c` (1938),
       `enter.c` (12, 127), `main.c` (1536), `main_t.c` (13), `start.c`
       (196, 402), `stmain.c` (596), `vc.h` (395); and the top of the
       file itself, which says what its name was for.
     - The documents: `src/portable_c/DEVIATIONS.md` on 9 lines, the
       README on 28, `CLAUDE.md` on 25, of which the notes of how the
       translation was made are of `src/c/resident.c` and stay, and
       this file on 87, most of them history.
     The check is the row's: both programs the same bytes, and the
     tests that read the source, `portable_c-data` above all.
   - Z2 mended four sentences of the README that the words made
     untrue, and no more: the one behind the table of the figures, two
     of "What rests on Open Watcom, for the portable C" (which still
     speaks of the interrupt functions of `dos.c` behind them), and
     the row of `portable_c-int16`. And the note of `CLAUDE.md` on the
     figures. The line numbers below are those of `7e73258`, a few off
     since.
   - The notes of `CLAUDE.md` name `__far`, `STACK`, `FAR`, `CODE` or
     `TEXT_FAR` on 21 lines, as what the portable C is written in:
     "Notes on the portable C" on `TEXT_FAR`, on a pointer to a local
     variable that "wants `STACK`", on the audit with clang "with the
     words of Open Watcom defined away". Each is history now.
   - `README.md`. The names that are no more stand on lines 278, 281,
     1112, 1113, 1133, 1146, 1150, 1152, 1177, 1216, 1221, 1229, 1232
     to 1234, 1406, 1752 and 1753: `build/portable_c32/`,
     `add_portable_c32`, `harness/pass32.txt`, the tests
     `portable_c32-...` and `portable_c-arith`, `notice.py --build`.
     The sections, by their lines: "Building" (235), which has two
     programs built; "What rests on Open Watcom" (468) and "Inline
     assembly" (519), written of `wcc`, the medium model, the three
     groups of data, `dos.c` and `arith.c`; "The figures" (644), which
     is of the program as it is; "Staying in the memory whole" (697),
     of the 16-bit layout and of what is kept while a command runs;
     "The system in one place" (869), of `dos.c`; "The program of 32
     bits" (1098) and "The extender" (1180), which become what the
     program is; the table of the tests (1229 to 1234), whose rows are
     to be the eleven tests of this section's table; "The machines"
     (1419), "When a step is finished" (1502) and "What ran" (1625),
     which W wrote of both kinds of program. The file has 39 lines
     that say "16 bits", "16-bit", "32-bit" or "the program of 32
     bits". The sentence that DOS/32A's licence asks for stays as it
     is.
   - `src/portable_c/DEVIATIONS.md`. It is written of the 16-bit
     program, with parts "**The program of 32 bits.**" in entry 1
     (line 175) and entry 6 (750), and entry 13 (1669) is of that
     program by its title. 25 lines say "16 bits" or "16-bit". Four
     places were brought up with the switch and say so in passing:
     the top (39), the table of entry 1 (304, where a transitional
     sentence says how its rows are read), and entry 13 (1672, 1727).
     Each entry is to say what the portable C does; entry 1 above all,
     since the program does not stay in DOS's memory at all, and what
     it says of a second copy is what the patches of `second-copy`,
     `start-memory`, `ended`, `command` and `stay-left` now say. The
     test `portable_c-scenarios` holds that every scenario with a
     patch or not run is named, as `` `name` ``, in the entry that the
     patch names: run it after the rewrite.
   - This plan: "In short", the strategy and "The build" say "the same
     source is built twice"; the tables of "W, the switch" are of
     before, and say so at the top.
   - `CLAUDE.md`, the notes only. A note of history stays. What
     misleads: 1319 (`add_portable_c` "takes that list too": it takes
     a target and a directory); the note on the medium model that
     follows it; 2440 to 2482 (`build/portable_c32`,
     `portable_c32-layout`, `portable_c32-data`); 2526 to 2534 (the
     record, the tree `portable_c32`); 2617 and 2621; 2646
     (`notice.py --build`); 3174 and 3264 (`both.py`); 3298; the note
     on the figures, which has three things that the script "takes",
     of which the bench's 425,536 bytes and the two panels are gone
     from it; and the notes that say "the same bytes" of a 16-bit file.
     The note at 2650 says how all of them are read meanwhile, and
     goes when they are right. The rule above the notes that says
     "with `dos.c` behind it" is the session's to change.

### The machine's clock of the portable C against the original's: counted

For the rule of `CLAUDE.md`, "Scenarios", that a scenario which shows
the time of day does so within the first minute of its machine: the
machine's clock is counted in the instructions that it executes, and a
C tree takes more of them than the original to get to the same step.
The rule had "up to a sixth more", from `copy` and `delete` on the
programs of 16 bits. Counted on 13 October 2026, with the fifth stage
of the switch, for the portable C as it is, a program of 32 bits under
its extender, in each of the 155 scenarios that it is taken through:
what QEMU says the machine has executed when the driver has written
the last screen, 3,906,250 instructions to a second, from noon, so the
five seconds in which the machine starts are in both figures. The top
of `harness/scenarios.py` says the same in fewer words.

| At its last step, the portable C's clock against the original's | Scenarios |
|---|---|
| Less than the original's | 5: `setup-menu`, `setup-mouse` and `setup-missing`, whose original is two programs, of which the second is started; `mouse-drag-lists`, which the tree is taken through with steps left out; and `diskette-command`, 0.82, which was not looked into |
| Up to a tenth more | 42 |
| A tenth to a third more | 102 |
| 1.34 to 1.37 times | 4: `mouse-edit`, `editor-buffer`, `tree-box`, `editor-full` |
| Twice | 2: `view-search`, 2.05, and `find`, 2.01 |

The middle one of the 155 is at 1.13 times. Of scenarios of each
kind, in seconds past noon at the last step, with the C translation
where it was counted, which was in ten:

| Scenario | The original | The C translation | The portable C | The portable C against the original |
|---|---|---|---|---|
| `view-search` | 13.01 | 15.41 | 26.62 | 2.05 |
| `find` | 10.20 | 11.80 | 20.48 | 2.01 |
| `mouse-edit` | 40.83 | 52.92 | 55.78 | 1.37 |
| `editor-buffer` | 8.19 | | 11.17 | 1.36 |
| `tree-box` | 12.00 | 16.04 | 16.17 | 1.35 |
| `editor-full` | 49.38 | 72.32 | 66.04 | 1.34 |
| `enter` | 18.31 | 28.93 | 23.43 | 1.28 |
| `copy-full` | 195.02 | 198.91 | 228.99 | 1.17 |
| `stamps`, which shows times | 6.55 | | 7.59 | 1.16 |
| `delete` | 21.06 | 23.80 | 24.38 | 1.16 |
| `clock`, which shows the clock | 5.02 | | 5.74 | 1.14 |
| `menu` | 11.67 | 12.78 | 13.02 | 1.12 |
| `editor` | 5.32 | | 5.92 | 1.11 |
| `copy` | 22.18 | 23.57 | 24.10 | 1.09 |
| `setup-colors` | 7.55 | | 8.00 | 1.06 |
| `setup-clock` | 8.27 | | 8.71 | 1.05 |
| `blank-timed` | 86.77 | | 87.48 | 1.01 |

- The first screen is 4.97 seconds past noon in the original, 5.31 in
  the C translation and 5.59 in the portable C, on the bench of the
  United States; on the Russian one the original and the portable C
  are each 0.4 seconds later.
- The C translation is beyond a sixth more as well, in four of the
  ten: 1.58 in `enter`, 1.47 in `editor-full`, 1.34 in `tree-box`, 1.30
  in `mouse-edit`. The sixth was of the two scenarios that it was
  taken from.
- **A search for a text takes three and a half times the machine's
  clock in the portable C, and that is all of the two scenarios at
  twice.** The step that searches takes 2.60 seconds in the original
  and 8.70 in the portable C in the viewer (`view-search`, the Enter
  of its step 14; the Enter of step 18 is 2.35 and 8.14), and 3.01
  and 11.07 in the file search (`find`, the Enter of its step 19). The
  C translation does not show it: it ends the two scenarios at 1.18
  and 1.16 times the original. Why was not looked into. The piece F4
  of the plan for a flat memory, which makes the viewer's text and
  the list of the search pointers, counts those steps before and
  after: its row there says so.
- No scenario of the portable C shows a time of day of its run past
  45 seconds. Five go on past that, `mouse-edit`, `editor-full`,
  `enter-many`, `blank-timed` and `copy-full`, and none of them has
  the clock on or shows the time of a file that it has made, by their
  goldens. And none makes a file past the first minute, by the stamps
  that all 155 leave on their disks: the latest is `DIRINFO` of
  `mouse-edit`, 54 seconds past noon where the original makes it at
  40, whose time is on no screen, the cursor being on `..`; the next
  is of `move`, at 34.
- How it was counted, by scripts that are not kept: `scenarios.run`
  with `until`, which is called with every screen as the driver has
  written it and asks `Machine.executed()` then, the machine having
  been noted in a wrapper of `Machine.wait_for_end` that also has
  QEMU's process waited for a millisecond at a time and not ten; a
  step more at the end of every scenario, `Wait*40`, since the machine
  ends behind the last screen and cannot be asked then; and for the
  files, the stamps of `Run.disks` between noon and ten past. A
  scenario comes to a step at the same count whenever it is run:
  `menu` twice gave 11.66 and 11.67 seconds, the difference being how
  soon the host asked. All 155 in both programs, three side by side,
  took some ten minutes.

### T, the time of a wait: done, and what a later piece should know

A step that has the driver wait was given up under load for the program
of 32 bits alone: `blank-timed`, at its step of 1,300 ticks, in the
suites of several agents (the notes of H3, E and C2 above, and one
each under "The portable C: what EGA lines has left" and "The portable
C: without Quick execute commands"). This piece was to find out why
such a wait costs the host so much more for that program, whether the
program can be mended, and what the time that a step has should be
counted in. In short: it is the processor's mode, changed thirteen
times at every round of idling, which QEMU pays for at each change;
nothing in the program mends that without asking DOS less; and a step's
time is now the host's and the machine's own, both.

**What was measured.** `blank-timed`, its step "Wait*1300 x", on this
computer by itself, a load of 1 to 2 on four processors, each program
twice. The step is 81 seconds of the machine's clock, 316.5 million
instructions, the same for every program to the third figure: the
machine's clock is counted in instructions (`CLOCK_SHIFT` of
`harness/dos.py`), so what differs is what the host pays for one.

| Program | The step, seconds of the host | Rounds of idling, by the driver's INT 28h | Instructions a round | The host's microseconds a round | Blocks that QEMU translated again | Times that QEMU flushed its table of addresses |
|---|---|---|---|---|---|---|
| The original | 9.1 | 268,581 | 1,178 | 34 | 2,592 | 88 |
| The C translation | 8.2 | not counted | | | 2,893 | 204 |
| The portable C, 16 bits | 8.2 | 237,516 | 1,333 | 35 | 281 | 176 |
| The portable C, 32 bits | 31.1 and 35.2 | 117,806 | 2,687 | 264 | 678,858 | 1,595,868 |
| The same, the driver told no place of `AutoTim`, so that it reads nothing above the megabyte | 29.3 | 130,270 | 2,430 | 215 to 225 | 742,922 | 1,489,112 |
| A trial: the keyboard, the mouse and the idle call made by DPMI 0300h | 20.7 and 21.0 | 86,328 | 3,666 | 242 | 53,111 | 1,183,260 |
| The same, and the time of day | 19.9 | 85,598 | 3,698 | 232 | 915 | 1,171,418 |
| The same, and the BIOS for the screen | 19.5 and 19.7 | 85,557 | 3,699 | 229 | 187 | 1,170,931 |

Under load it takes what the load makes of that: the scenario was 192
seconds for the program of 32 bits beside eight other machines, a load
of 17, where it is 36 by itself, and the others saw 136 to 403.

**Why.** A round of idling is the same calls in both programs, as the
original makes them: the BIOS for the keyboard, the mouse driver, DOS's
idle call, and DOS for the time of day while the blanker's minute runs.
By QEMU's log of interrupts (`-d int`, which has those made in
protected mode), the program of 32 bits made 718,034 in that step, at
`cb6b20a`, for 125,373 rounds: INT 16h 322,870, INT 33h 197,112, INT
28h 125,373, INT 21h 71,356 and INT 10h 634. That is 5.7 a round, and
the extender takes each to real mode and back, two changes of the
processor's mode; the driver's read of `AutoTim` by the BIOS's block
move is two more: 13.5 a round, which is what QEMU's own count of its
flushes comes to. Three things then cost, and a fit of the eight rows
to them says roughly how much:

| | A round of the program of 32 bits | Of its 264 microseconds |
|---|---|---|
| Its instructions, at what one costs the host in the 16-bit program, 26 nanoseconds | 2,687 | 70 |
| The changes of mode. At each, QEMU throws away what it has worked out of the machine's addresses and of where its translated code is, and works it out again | 13.5, some 10 microseconds each | 130 |
| The extender's own code, changed. Where it passes an interrupt to real mode, DOS/32A writes the number into an `INT` instruction of its own, 25FC:176A on the bench, and executes that; QEMU translates the block again every time, the one block 717,342 times of the 719,160 that it translated in the step | 5.7, some 11 microseconds each | 63 |

So half is the switch itself, a quarter is the extender writing its
own code, and a quarter is instructions, of which the program of 32
bits has twice as many a round. On a processor none of this is much:
it is QEMU's way of emulating that makes a change of mode cost what
400 instructions do.

The driver is not the cause, as C2 found by holding the old against the
new: with nothing read above the megabyte a round is 215 to 225
microseconds for 264, and the program then goes round more often in the
same wait, so that the step is 29 seconds for 31.

**No mend in the program.** What costs is one change of mode for each
interrupt, and the interrupts are the original's: fewer of them is the
program asking DOS and the BIOS less, which is a deviation. The one
thing that the program could do otherwise is the way to real mode.
DPMI 0300h does not go through the `INT` that the extender writes, and
the three trials above made the calls of a round by it, `Real` of
`dos32.c` in place of the instruction (not kept; four functions
changed in the first, six in the last). The blocks translated again go, and the step is 20 seconds
for 31. But a round is then 229 to 242 microseconds for 264, a tenth
less: the service costs 250 instructions more for each call, a round
is 3,700 instructions for 2,687, and the program goes round 66 times a
tick where it goes 91 (the 16-bit program 183, the original 207). Most
of what the trial saves, it saves by making the idle call a quarter
less often by the machine's clock. That was not taken, and nothing of
the program was changed. What real mode sees would differ besides: by
the extender's own way the registers are the program's as they happen
to be, with DS 0001h and ES 0, on its stack at 30C7:01F4; by 0300h
they are what is asked and zeros, with DS and ES on the program's
block below the megabyte, at 30C7:01FA. The first trial passed every
scenario of the record, 153, so it is there to be had if a wait is
ever to cost less: it is the user's to want.

What the harness could do: the driver could read `AutoTim` and `Clock`
less often than at every round, which "What could go wrong", 4, names
for the time of day. It would take a sixth off a round's cost at the
most, and change when every step of that program ends. Not done.

**The time that a step has.** `STEP_TIME` was 300 seconds of the host's
clock in which the driver writes nothing. C2's stopgap gave a scenario
ten seconds more for each second of its longest wait, 1,014 for
`blank-timed`, and H3 had given a scenario that is not on the record
twice its wait. Both are numbers of the host's clock, which a busier
host outruns, and the first has a program that hangs in such a
scenario waited for 17 minutes at every step. They are replaced by one
rule, in `Machine.wait_for_end` of `harness/dos.py`: when the host's
seconds are over, the program is given up only if the machine has also
executed as many seconds of its own clock since the driver last wrote,
3,906,250 instructions for each, or has executed nothing at all for
as long. QEMU says what the machine has executed (`query-replay` over
QMP, `Machine.executed`); it is asked once a second while nothing is
written, so a step that ends within a second asks nothing. A scenario
that waits has the seconds of its longest wait besides, once
(`scenarios.run`); `WAIT_TIME` and `short_step` are gone.

Why both clocks and not the machine's alone: a step may be many seconds
of the machine's clock and none of the host's, 34 in `mouse-edit` in a
third of a second, and `--limit 10` of `show.py` would give that up.
With both, nothing that ended in time before is given up now, and a
program that hangs on a host by itself is found when it was. What it
costs: on a busy host a program that hangs is waited for until its
machine has had its time, which is the point, and is long where the
machine is slow anyway. A second of the host's is, on this computer by
itself: 59 of the machine's for a program in a loop of one instruction;
10 for the 16-bit program idling and 2.6 for that of 32 bits; 1.25 for
a machine that is halted with interrupts on, whose clock runs on at 715
seconds a second and which executes 375 instructions a tick; and none
for one halted with them off, which is what "nothing at all" is for.

What the longest steps are, in the machine's seconds, by a run of all
scenarios of the program of 32 bits with QEMU asked at every write:
`blank-timed` 81, `mouse-edit` 34, `copy-full` 27, `find` 11,
`view-search` and `editor-full` 9, `enter-many` 7, and every other
under 5, which is what the program takes to start. The 300 are not
near, and since these are counted in instructions they are the same on
any host.

What decided:

- `python3 harness/goldens.py derive`: `git status` of
  `harness/goldens/` clean. And once more with QEMU asked at every turn
  of the wait, a hundred times a second, 27,763 times in the 167
  scenarios: clean again, the lists of what ran among them. Asking
  changes nothing in the machine.
- Three machines that hang, each a program of four bytes run as the
  program by `harness/trial32/run.py --limit 6`: a loop, given up after
  6.1 seconds; halted with interrupts on, 6.1; halted with them off,
  7.1.
- A machine that is kept from running: `blank-timed` of the 16-bit
  program with its QEMU stopped for fifteen sixteenths of the time and
  a limit of 20 seconds, 91 with the wait. It passed in 145 seconds.
  With the host's clock alone deciding it was "stopped after step 4,
  Wait*1300 x: the program did not come to wait for a key within 91
  s", which is the failure that the three agents saw.
- The whole suite, when the piece landed.

For whoever goes on:

- The limits of `harness/dos.py` for a machine that boots, loads a
  saved state or saves one, `BOOT_TIME` and its like, are still of the
  host's clock alone. None was seen to fail under the loads of that
  day. `Machine.executed` is there for them.
- A connection to QEMU may be reset while it is asked something, when
  the machine ends at that moment: `_reply` takes that for a machine
  that has ended. It showed in the first run that asked QEMU at every
  write, which is when a machine ends.
- How the cost was found, since the tools were not kept: a script that
  replaced `Machine.wait_for_end` and noted, at every write of the
  driver, the host's time, the processor time of the QEMU process from
  `/proc`, and what the machine had executed; QEMU's `info jit` by
  `human-monitor-command` for its counts of blocks invalidated and of
  flushes; and QEMU's logs written to a FIFO that a thread counts, so
  that nothing is kept of gigabytes: `-d in_asm` by the first address
  of each block, which named the one block at once, `-d int` by the
  vector and the place, and `-d exec` for the driver's handler of INT
  28h, 2381:0463 on the bench whatever the program, which is always
  entered from QEMU's loop and so always logged. `perf` is not let
  look at a process on this computer.
- Processor time is not the measure to trust under load: the same step
  was 33 seconds of it at a load of 2 and 54 at a load of 12. Measure
  on a host by itself, or count.
- Two machines at a time, `--together 2`, take all scenarios of the
  program of 32 bits in 128 seconds on a host by itself, and derive
  every golden in 140.

### L, a program that has stayed in the memory, when the program is left: tried, dropped on the user's word, and declared as it is

For 2 of "The program of 32 bits: what differs from the original, for
the user" below: a program that a command has left in the memory, with
interrupts taken, is cut off from them when the program of 32 bits is
left, where behind the program of 16 bits it goes on working, as entry
7 of `DEVIATIONS.md` says. The user had said "mend it", on being told
that it was one bit of the extender's settings. It was not. A mend was
made, in a checkout and never landed: two other bits of the settings,
the program's three interrupts noted and given back in real mode too,
and a routine of real mode of 69 bytes, written by hand as its bytes,
in which DOS went on at every end of the program; and it still left
such a program without INT 21h. Told that, the user decided on 13
October 2026: **the mend is dropped.** The program of 32 bits is as it
was before this piece, to the byte, and so is the program of 16 bits.

What is left of the piece, and is here: the scenario that shows the
matter, `stay-left`; what was found out of the extender; the mend,
said so that it could be made again; and what is open. **Open, the
user's:** whether the difference is declared as it is or mended some
other way. Nothing is declared for it. `stay-left` fails for the
program of 32 bits and is off its record, as `command` is.

That was so when the piece ended. **Decided by the user since, on 13
October 2026: declared as it is**, with no change of the program
(`CLAUDE.md`, "The portable C"). "Declared", at the end of this
section, has what was written and how the patch was read.

**The way that was decided for is none.** The list had the mend as one
bit, bit 4 of the byte 14h of the extender's settings, "seen once on a
copy". With that bit clear the extender does not load the program:
"DOS/32A fatal (4002): not enough DOS memory to load application exec
"D:\VC\VC.EXE"" is the first screen, and then DOS's prompt (seen again
when this was written, `harness/show.py --program` with a copy of the
file in a directory named as its tree is). The bit is half of a number
that says where a program is loaded, and what is left of it says below
the megabyte only. So the three steps of that trial were typed at
DOS's prompt, where FDAPM stays and is found by anyone, and the "empty
line of DOS's prompt" was F10 and Enter typed there. The two rows of
that table for the bit 4 said nothing of the program.

**How the extender was read.** The pinned Open Watcom has neither the
source of DOS/32A nor a description of its settings. `binw/dos32a.exe`
of that download is not packed, and the piece read it with a
disassembler, `ndisasm -b16`, from where its header has its two
segments of code: where it reads its settings when it starts, and
where it takes the options of its environment variable, `DOS32A`,
whose names stand in the file one behind the other, each of which
sets or clears bits of the settings' copy in the memory, which names
the bits. The listing was kept in `build/` while it was read and is
in no commit, and no instruction of the extender is written down
anywhere here: what follows is what it does, in words. It is of the
DOS/32A that answers 090Ch for its version, the one that
`tools/stub.c` knows, and of no other. What of it was also seen on
the bench is said with each thing; the rest is from reading alone.

**The settings.** They are the block behind the letters `ID32` that
`tools/stub.c` has byte for byte. Of its first 16 bytes, which are of
the part of the extender that goes into protected mode, only this was
read: the four at 10h are how much memory above the megabyte it takes
at most, all of it as they come, and the byte at 05h how many page
tables it has for that; the option `EXTMEM` sets the two together (1
of the list below, and C2). The rest:

| Byte | Bit | What it is, and the option of `DOS32A` that sets it |
|---|---|---|
| 14h, 3Fh as it comes | 0 | Warnings are written at all: `PRINT`; `QUIET` clears it with bit 1 and the banner |
| | 1 | A run-time error sounds: `SOUND` |
| | 2 | All 256 interrupts of real mode are put back when the program ends, as they were when it started, with the masks of the two interrupt controllers: `RESTORE` |
| | 3 | The warning 9003 is written for each interrupt that is not as it was. No option; `NOWARN` takes a warning out by its number |
| | 4 and 5, a number | Where the program is loaded: 0 below the megabyte and, without room there, above; 2 below only; 3, as it comes, above only; 1 by a flag of each part of the file. With bit 4 clear it is 2, and the program of 32 bits is not loaded: seen |
| | 6 | A run-time error is written on a screen set anew |
| | 7 | A watch on the first bytes of the memory, by the processor's debug registers: `NULLP` |
| 15h, 09h as it comes | 0 | The environment variable `DOS32A` is read |
| | 1 | INT 2Fh with AX 168Bh is called when the program starts |
| | 2 | The parts of the file are loaded at multiples of 4K, not of 16 bytes |
| | 3 | The banner: `NOC`. The one bit that `tools/stub.c` clears |
| | 4 | What is loaded where is written at the start: `VERBOSE` |
| 16h, a word | | The buffer below the megabyte through which the extender hands DOS what the program reads and writes, in paragraphs: `DOSBUF`, in KB |
| 18h, a word | | The extender's number, 090Ch |

Since the variable is read, whoever runs the program can set any of
this otherwise, by the options that the table names; the piece wrote
`SET DOS32A=/RESTORE:ON` for their form. That was read and not tried.

**What the extender does with the interrupts, by its own code.** Three
things:

- The part that the program is loaded by keeps the 256 interrupts of
  real mode as they are when the program starts, with the masks of
  the two interrupt controllers. At the program's end, in its handler
  of AH 4Ch in protected mode, it holds the table against that copy
  and writes the warning 9003 for each that is otherwise, if bit 3 of
  the byte 14h is set, and puts all 256 back, with the masks, if bit 2
  is. Both are set as the stub comes. There it also tells the mouse
  driver to call nobody, INT 33h with AX 000Ch, so a program that has
  stayed and has given the driver a routine of its own would lose
  that: read, not tried. None of this is reached when DOS ends the
  program by another's hand: then the extender sees the call in real
  mode, and its other part alone runs.
- The part that goes into protected mode keeps INT 15h, 1Bh, 1Ch, 21h,
  23h, 24h and 2Fh as they are when it starts, and puts the six from
  1Bh on back when the program ends, by either way, whatever any
  setting says and whoever has taken them since. INT 21h of real mode
  it takes itself when it starts: that is how it sees AH 4Ch, and it
  marks a program run through it, AH 4Bh, so that the end of that one
  is passed on. INT 15h it takes where it runs with no XMS driver and
  no other memory manager, as on the bench, to answer 88h with nothing
  and refuse E801h and E820h (1 of the list below), and puts it back
  then.
- Told to set INT 1Bh, 1Ch, 23h or 24h in protected mode, DPMI 0205h,
  it sets the interrupt of real mode as well: to a routine of its own
  that comes over, where a program takes it; and where the interrupt
  is given back, the selector being the extender's own, to where it
  went when the extender started.

So a program that has stayed in the memory loses every interrupt when
the program of 32 bits is left, by the first; with bit 2 clear alone
it would still lose the six, by the second, which is why FDAPM, which
takes INT 16h, 28h, 2Ah and 2Fh and is asked by INT 2Fh whether it is
there, said that it was not in the trial of W with that bit clear; and
one that has taken Ctrl-Break loses that while the program still runs,
by the third: the first time that the program, having taken
Ctrl-Break again behind a command, gives it back, in front of the next
command or at its end, it goes where it went when the extender
started.

**Who gives which interrupt back**, in the program of 32 bits as it
is:

| | The program ends itself | Around a command | DOS ends the program by another's hand |
|---|---|---|---|
| The program, `dos32.c` | `GiveAll`: the critical error, Ctrl-Break and Ctrl-C, each by DPMI 0205h to who had it in protected mode when the program noted that, which is the extender | `RestVct` before: the critical error and Ctrl-Break, the same way. `HookVct` behind: both noted and taken. Ctrl-C stays taken, as in 16 bits. And INT 22h of real mode read before the program is run and put back behind it, for the warning (D2) | Nothing: it cannot run |
| The extender, at each such 0205h | INT 1Bh, 23h and 24h of real mode to where they went when it started | INT 1Bh and 24h the same; taken again, to a routine of its own | |
| The extender's part that loads the program | A line of warning for each of the 256 that is not as it was when the program started, and all of them put back, with the masks; the mouse driver told to call nobody | Nothing | Not reached |
| The extender's part that goes into protected mode | INT 1Bh, 1Ch, 21h, 23h, 24h and 2Fh, and INT 15h on the bench, to where they went when it started | Nothing. INT 21h stays its own, and INT 15h | The same six and INT 15h |
| DOS | INT 22h, 23h and 24h from the PSP | INT 22h to where the program was run from; INT 23h and 24h from the PSP of the program that ends | INT 22h, 23h and 24h from the PSP: INT 22h to the routine in the block below the megabyte |
| The routine of real mode, `Ended_T` (E) | Not entered | | DOS told of Ctrl-C and the registers left, and on to where the PSP had DOS go on before. It sets no interrupt |

In 16 bits, for the same: the program gives the critical error and
Ctrl-Break back by DOS, to who DOS said had them, at its end and
around a command; DOS puts INT 22h, 23h and 24h back from the PSP; and
when DOS ends the program, nothing gives Ctrl-Break back, which then
goes into the ended program, the original's bug, carried, as INT 22h
does. Nothing puts back an interrupt that another program has taken.

**The scenario, `stay-left`.** Two programs made in the editor, in hex.
`t.com` stays by INT 27h with INT 1Bh, 1Ch, 21h and 2Fh taken, each
handed on to who had it, and INT 60h; on INT 2Fh it answers AX EE00h
with its segment. `q.com` asks that and prints, for each of the five,
its number if it goes to the segment that answered, and two dashes if
not or if nobody did. It is run before `t`, with `t` in the memory,
and at DOS's prompt when the program has been left:

| | With `t` in the memory | When the program is left | `q` at DOS's prompt |
|---|---|---|---|
| The original, and the C translation | `1B 1C 21 2F 60` | Nothing | `-- -- -- -- --`: released |
| 16 bits | the same | Nothing | `1B 1C 21 2F 60`, by `harness/goldens/portable_c/stay-left.patch` |
| 32 bits, as it is | `-- 1C 21 2F 60` | Four lines of the extender's warning, for INT 1Ch, 21h, 2Fh and 60h | `-- -- -- -- --`. It failed, at the first of these screens, while nothing was declared; since, it is held to these three by `harness/goldens/portable_c32/stay-left.patch` |
| 32 bits with bits 2 and 3 of the byte 14h clear and nothing else changed: a copy of the file, run once when this was written | `-- 1C 21 2F 60` | Nothing | `-- -- -- -- --`: INT 2Fh is among the six, so nobody answers `q`, whatever else still goes to `t` |
| 32 bits with the mend, as its piece saw it | `1B 1C 21 2F 60` | Nothing | `1B 1C -- 2F 60` |

The five are of the kinds that there are: INT 60h is one that only
the table put back takes away; INT 1Ch and 2Fh the extender puts back
whatever its settings say; INT 1Bh the program takes itself; and INT
21h the extender has. `q` finds `t` by INT 2Fh: where that one is
lost it prints five times two dashes and says nothing of the others.
The patch of the program of 16 bits has the box of F10 (10) and the
Advanced options (14) besides, and is named in those entries. The
golden is the original's, derived again when this was written, to the
same file.

**The mend that was dropped, so that it could be made again.** It was
some 150 lines, in `tools/stub.c`, `src/portable_c/dos32.c` and
`harness/layoutcheck.py`, and is in no commit of this history. Its
three parts:

| Where | What |
|---|---|
| `tools/stub.c` | Bits 2 and 3 of the byte 14h cleared with the banner's bit: the extender neither puts the table back nor writes the lines. `harness/layoutcheck.py` held all three bits of the stub, and said of each what the extender would do with it set |
| `Note` and `Give` of `dos32.c` | Who has an interrupt in real mode is noted with who has it in protected mode, DPMI 0200h beside 0204h, and it is given back there too, 0201h behind the 0205h that gives it back in protected mode. For all three that the program takes; it matters for Ctrl-Break, which then goes to the program that stayed around every command and at the end |
| `Ended_T` and three functions around it, `Keep`, `Routine` and `Left` of `dos32.c` | The routine of real mode in which DOS goes on when it has ended the program by another's hand (E) became the routine in which DOS goes on when the program has ended at all. Behind what E's instructions do, and in place of their jump, it saves DS, puts it on the segment 0, stores four addresses into the table of interrupts, for INT 1Bh, 1Ch, 2Fh and 22h, each by one instruction that has the place and the address in it, so that it changes no flag and no register and no interrupt comes into the middle of it, gets DS back and jumps to where the PSP had DOS go on before, by an address in the instruction too. 69 bytes where E's are 28: E's 23 without their jump, 4 around the stores, 36 for the stores, and the jump's 5. A program that ended itself entered it behind E's part, 23 bytes in |

How the routine came by its addresses: `Keep` asked the extender where
INT 1Ch and 2Fh went in real mode, DPMI 0200h, and took for Ctrl-Break
who was noted when the program took it, or who had it if the program
had it not, and wrote the three into the routine's instructions. It was
called when the routine was put into the block, behind every program
run, and at the end: nothing but a program that stays changes those
interrupts while the program runs, and such a one comes by a command.
`Routine` copied the bytes into the block and wrote into them where
DOS was to go on, twice, for INT 22h and for the jump, and in front of
them the PSP's segment and what the program has said of Ctrl-C. INT
22h was set to where the PSP had DOS
go on before, which is what it names behind a program of 16 bits that
has ended itself: DOS has just set it from the PSP, to the routine.
`ProgramEnd` called `Left`, which had the PSP name the routine 23
bytes in if the program had given its end back, as `Exit00` does
first, and left the PSP as it was if not, so that a program which ends
with its end taken went through all of the routine, as in `dos.c`.
Without the block, where the program could not start, there was no
routine.

What went with it: `GoesOn` and `SetGoesOn`, by which INT 22h is put
back behind every program run, which is for the warning alone (D2).
The block below the megabyte stayed 512 bytes: its places were laid
out anew, each behind the one before, with 48 bytes for the record of
a search where there are 64, the PSP's segment and what the program
has said of Ctrl-C, which the routine reads through CS, in front of
it, and 11 bytes free at its end. Two rows of
entry 6 said what the program asked of the extender then; the entries
of `harness/datacheck.py` and `harness/reads.py` for `Ended_T` said
what it had become; and `harness/trial32/build.sh` set the stub of the
toy of both builds by `build/tools/stub`, since `dos32.c` counted on
the settings.

What it achieved, by the scenario's table: four of the five
interrupts, and no line on the screen. FDAPM, run as `fdapm apmdos`
from the program, answered `fdapm stats` at DOS's prompt afterwards as
behind the program of 16 bits. A program of a few bytes that prints a
sum of all 256 interrupts, typed in hex and run at DOS's prompt behind
each program left with nothing run from it, printed the same four
digits behind the original, the program of 16 bits and the mended
program of 32 bits. With the steps of `ended` and the two programs of
`stay-left`, the copy running `t` before DOS ended it: `1B 1C -- 2F
60` in the first copy and at DOS's prompt. `harness/trial32/ended.asm`
had the registers and `BREAK` of E's table, and INT 22h at the place
behind the toy's own call, where it names the block today.
`harness/trial32/both.py` had no line that differed, for both
countries.

What it did not achieve: INT 21h, and INT 15h on a machine without a
memory manager. Those are the extender's while the program runs, so a
program that takes one of them meanwhile hands its calls on to the
extender, which is gone when the program has been left; the extender
puts both back as it goes, since what it leaves behind is memory that
is free. A program that had stayed and had taken one of them was left
with its others and without that one. The piece had three ways with
it, none of which was decided, the mend being dropped first:

- (a) It stands, declared: such a program has lost that interrupt and
  keeps the others.
- (b) The extender's INT 21h taken out from under a command and put
  back on top behind it: the program would read where the extender
  hands INT 21h on, a place in the extender's own memory that only
  its code tells, set the interrupt to that while a command runs, and
  behind the command write there whoever has it now. That would keep
  such a program whole, and rests on the inside of one version of the
  extender; with another host under the extender it was not read. Not
  tried.
- (c) Such a program cut off from all of its interrupts, as every one
  is without the mend: the routine would put the whole table back if
  INT 21h is not as the extender had it. Then no program is left with
  a part of its interrupts, and one that takes INT 21h is as behind
  the original, but for its memory.

What it rested on. That the bytes of a block which DOS has freed are
still there when DOS goes on in them, at every end of the program
where E's routine counts on it only when another program has DOS end
this one; that nothing but a command changes INT 1Ch and 2Fh while the
program runs; and on what one version of the extender does by its
code, read and seen under FreeDOS in QEMU alone, with no memory
manager, no EMM386 and no DOS box of Windows under it, where the
extender goes into protected mode another way and was not read.

**What of the piece's work on the tools was left behind.** All of it
that was for the mend: the three bits in `tools/stub.c` and
`harness/layoutcheck.py`, the entries of `harness/datacheck.py` and
`harness/reads.py`, and `build.sh` setting the toy's stub by the
build's tool, which is the same bit as `quiet.py` clears while the
program's stub has only the banner's cleared. `tools/stub.c` has a
sentence more in its comment, on what the banner's bit is.

**For the pieces behind this one.**

- A setting of the extender that is seen to mend something on a copy
  of the file is looked at on every screen of the run, the first above
  all: a program that is not loaded leaves DOS's prompt, which takes
  the steps of a script as the program would.
- When a scenario leaves the program behind a command, clear DOS's
  screen first, `"cls" Enter`: the prompt that the program leaves is
  written into a row of what the command printed.
- A resident program for a scenario is typed in hex; the comments of
  `stay-left` have its instructions. To try one before it is a
  scenario: `harness/show.py` with the steps, which prints the screens
  at DOS's prompt too.
- A toy of the trial that is built with `quiet.py` has the extender as
  the program has it, the banner's bit cleared and nothing else.

**The user's.**

1. What becomes of 2 of the list: declared as it is, or another mend.
   Its row there has the ways as they are known now. **Decided**:
   declared as it is, below.
2. Whether the variable `DOS32A` is to be read at all: bit 0 of the
   byte 15h. Nothing was changed there; with that bit clear the
   program would go by its file alone.
3. The mouse driver told to call nobody when the program ends itself,
   by the extender: above. Written here so that it is not lost. Entry
   7 says it among what was read, and that it is not among what was
   decided.

**Declared**, on the user's word of 13 October 2026, by a piece of its
own behind this one, which changed no source of a program:

- Entry 7 of `src/portable_c/DEVIATIONS.md` has a point of its own,
  "Behind the program of 32 bits such a program keeps its memory and
  is cut off from its interrupts", where it had a sentence that this
  was open: what the extender does as the program is left, what of
  that is the original's (no interrupt) and what is not (the memory
  kept, the lines on the screen), Ctrl-Break lost while the program
  still runs, and what is known and how: seen in `stay-left` and with
  FDAPM, read in the extender's code, with the mouse driver and the
  end by another's hand among what was only read. The row of
  `stay-left` in its table says what each program shows.
- `harness/goldens/portable_c32/stay-left.patch`, made by `python3
  harness/goldens.py patch --tree portable_c32 stay-left`, which takes
  the header of the 16-bit program's patch and says how the rows stand
  to that one: 28 lines as they were, the two screens of the Advanced
  options (14) and the box of F10 (10); 11 new; 1 gone. The new ones,
  each read against the table of the scenario above:

  | Screen | Rows | What |
  |---|---|---|
  | 45, `Ctrl+o`, both answers of `q` | 22 | `-- 1C 21 2F 60` where the original has `1B 1C 21 2F 60`: Ctrl-Break lost while the program runs. `q` is the first command behind `t`, and in front of it the program gives Ctrl-Break back through the extender |
  | 49, `Enter`, the program left | 20 to 24, and the cursor | Four lines of "DOS/32A warning (9003): real mode interrupt vector has been modified: INT 1Ch", 21h, 2Fh and 60h, in rows 20 to 23, and DOS's prompt in row 24 where the original has it in row 22, with the cursor |
  | 50, `"q" Enter`, at DOS's prompt | 18 to 21 | The four lines, two rows further up. Rows 22 to 24 are the original's: the command, five times two dashes, the prompt |

  Gone is the one row of 7 that the 16-bit program's patch has of its
  own, row 23 of the last screen, `1B 1C 21 2F 60`: this program shows
  the original's dashes there. So the patch has every row of the
  tree's patch but that one, and the reason in its header says why
  the row is not there. Nothing else differs on any of the 51 screens,
  nor on the disks.
- What `q` says at DOS's prompt is the same row as behind the
  original, and says less: it finds `t` by INT 2Fh, which is put back,
  so nobody answers and it prints dashes for all five whatever else
  goes where. That the others are put back is by the four lines of
  that run and by what was read. The entry says so, and the patch's
  header.
- `stay-left` is on the record with `command`: 155 of the 155
  scenarios that the program of 32 bits is taken through.

## The program of 32 bits: what differs from the original, for the user

Collected by the piece W on 13 October 2026, at `1eb233c`, from what the
pieces of the plan above wrote down as otherwise in the program of 32
bits or left "for W": H3, D1, D2, C1, C2, "E, the end by another's
hand", the trial on the bench, and "The editor's room: untested in the
program of 32 bits". Each thing was held against
`src/portable_c/DEVIATIONS.md` for whether it is declared, and one was
found while the list was made, 2.

The user's word of that day is that a deviation of the program of 32
bits from the original is decided by the user, one at a time, and that
nothing gives leave to remove or break a feature of the original
without asking. So the list declared nothing when it was made, and no
line of `DEVIATIONS.md` was written for it. It was what there was to
decide.

**What was decided**, by the user on 13 October 2026, with the list put
to them item by item, as the session that runs the work handed it on:

| | Decided | What became of it |
|---|---|---|
| 1 | First: open still, to be decided before the 16-bit build goes. Then, the same day: **declared as it is**, with no change of the program, the way (b) of its row | A point of "The program of 32 bits" in entry 1, with the row of `command`; `harness/goldens/portable_c32/command.patch`, two rows of what `mem` prints; `command` is on the record. "C2" above has it at the end of its part on `command` |
| 2 | "Mend it", on being told that it was one bit of the extender's settings. It was not; and told what the mend took and what it still left, the user had it dropped, on the same day. Then, the same day still: **declared as it is**, with no change of the program, the way (a) of its row | The piece L, "L, a program that has stayed in the memory" above. With bit 4 of that byte clear the extender does not load the program, and what the row had as "seen once" was DOS's prompt. The mend that was made, which is in no commit, took two other bits of the settings, the program's three interrupts given back in real mode too, and a routine of real mode at every end of the program, and left a program that had taken INT 21h without it. Kept of it: the scenario `stay-left`, which the program of 32 bits fails and which is off the record; what was read of the extender; and the mend in words. Entry 7 said in a sentence that this was open; it has a point of its own since, with what is known and how, and the row of `stay-left` says what each program shows; `harness/goldens/portable_c32/stay-left.patch` holds the three screens, and `stay-left` is on the record: "Declared" at the end of L. 10 of what was open in the plan. The row below is written anew, with the ways as they were known when the mend was dropped |
| 3, 4 | Decided before the list, and nothing more to decide | Entry 1 |
| 5 | Decided before the list, entry 13. What goes untested waits for step 2 of the road, when the memory is the extender's: none of the three ways is taken now | "The editor's room: untested in the program of 32 bits" |
| 6, 7, 8, 9, 10 | "Declare all five": what is inherent in running under an extender and shows on no screen is declared as it is | 6 and 7: two points of "The program of 32 bits" in entry 1. 9 and 10: "The program of 32 bits" in entry 6. Written by W. 8: a point of entry 8, written with the piece L when the mend of 2 had been dropped, of the program as it is: Ctrl-Break given back by the extender, and INT 22h into the block below the megabyte; the registers at the routine's call are 6's. Seen again for it with `harness/trial32/ended.asm`: `22 :3391 01D0` and no other interrupt behind the program of 32 bits, `1B` and `22` into the ended program behind that of 16 bits |
| 11, 12 | Notes: nothing to decide | They stay here |

The rows below are as they were put to the user, but that 9 and 10 say
what was seen since, where they had what was read: see "What a
program asks of DOS, seen" behind the tables.

What the portable C does otherwise than the original in both of its
builds is not here: `DEVIATIONS.md` has that. A row is here where the
program of 32 bits does otherwise than the original and the entries, as
they say it of both builds, do not cover it, or cover it only since a
piece of today wrote it in. "16 bits" is `build/portable_c/VC.EXE`, "32
bits" `build/portable_c32/VC.EXE`. Of the ways in the last column the
first is the one that W would take; of 2, whose ways L wrote anew when
its mend was dropped, the one that L would.

**What a user can meet.**

| | What | The original | 16 bits | 32 bits | A scenario shows it | Declared | The ways |
|---|---|---|---|---|---|---|---|
| 1 | The memory above the megabyte while a command runs | Takes none of it: a command has all, 15,232K by the BIOS on the bench, and 14,978K free with an XMS driver | The same | The extender has all of it, and INT 15h: a command is told 0 by 88h and refused at E801h and E820h, and with an XMS driver 0K are free. A program with DOS/32A gets memory all the same, a second copy among them; one with another extender, a cache or a RAM disk would find none, which is from reading and was not tried | `command`, in what `mem` prints: 128K more of extended and of total memory. It failed and was off the record until this was declared; it is held by a patch of this build's own since | Yes, since the user decided for (b): entry 1, "The program of 32 bits", the point on the memory above the megabyte. 9 of what was open in the plan | (a) The extender held to what the program and one copy of it need, 2 MB, by four bytes of its settings in `tools/stub.c`: seen by C2 on a copy of the file, a command then has 12,930K by XMS and 13,184K by 88h. It costs: the program then runs at the top of the memory, so `le.BENCH` and `behind()` of the harness are found anew and the six scenarios with a second copy run again; a third copy has no room and the extender says so in its own words; and `command` still shows other figures, E801h and E820h being refused, which wants a patch of this build's own. Half a day. (b) Declared as it is, with a patch of `command` for the two rows. Small. (c) Left open until the 16-bit build is to go, `command` failing meanwhile, which is how it stands |
| 2 | A program that stays in the memory and takes interrupts, run from the program, when the program is then left. Found by W: see below, and "L, a program that has stayed in the memory" above | Keeps a list of such programs and releases each when it is left: its memory is free and its interrupts are back. Seen: FDAPM, run from it as `fdapm apmdos`, is gone afterwards; and in `stay-left` none of five interrupts goes to the program any more | By entry 7 nothing is released: the program is in the memory afterwards and goes on working, as when it was run from DOS's command line. Seen: `fdapm stats` finds it; and in `stay-left` all five interrupts still go to it | The program stays in the memory, but the extender, as it leaves, puts every interrupt of real mode back to where it went when the program was started, and writes a line for each onto the screen that is left: four of "DOS/32A warning (9003): real mode interrupt vector has been modified: INT 16h", 28h, 2Ah and 2Fh behind FDAPM, and for INT 1Ch, 21h, 2Fh and 60h in `stay-left`. The resident program is cut off from its interrupts and keeps its memory: `fdapm stats` says that none is resident. And Ctrl-Break, INT 1Bh, it has lost before, while the program of 32 bits still runs: the program takes that interrupt through the extender behind every command and gives it back in front of the next, and the extender then has it go where it went when the extender started | `stay-left`, since L: this program failed it, at the three screens that show the above, and was off the record until this was declared; it is held to those three by a patch of this build's own since | Yes, since the user decided for (a): entry 7, the point "Behind the program of 32 bits such a program keeps its memory and is cut off from its interrupts". 10 of what was open in the plan | (a) Declared as it is: a point in entry 7, and a patch of `stay-left` of this build's own for its three screens. Small. What would be declared: such a program keeps its memory and none of its interrupts, which is the original's in the interrupts and not in the memory or in the lines on the screen; and it loses Ctrl-Break while the program still runs. No program is then left with a part of what it took. (b) The mend that was tried, and dropped by the user on 13 October 2026: L has it in words, some 150 lines, with a routine of real mode written as its bytes that DOS goes on in at every end of the program. It leaves such a program at work but for INT 21h, and INT 15h on a machine like the bench, which the extender has while the program runs and which nothing in the program can keep for another; L has three ways with that, of which the one that keeps such a program whole rests on the inside of one version of the extender. (c) The extender's settings alone, bits 2 and 3 of the byte 14h cleared in `tools/stub.c`: seen on a copy, with `stay-left`. No line is written, and the program that stayed keeps what it took but for INT 1Bh, 1Ch, 21h, 23h, 24h and 2Fh, and INT 15h on the bench, which the extender puts back whatever its settings say: FDAPM is still not found. That is neither the original's nor what entry 7 says, leaves a program with a part of its interrupts, and would be declared like (a). The one bit that this row had when it was put to the user, bit 4 of that byte, is no way: with it clear the extender does not load the program |
| 3 | A second copy of the program, started from the first with a command to run | Runs the command. In `start-memory` the command stays in the memory with nearly all of DOS's, and the copy cannot get its data back: "Program too big to fit in memory", and it ends | Has no memory for the command and says so; in `start-memory` it is still there afterwards | Runs the command, as the original. In `start-memory` it needs none of DOS's memory to come back, says nothing and is still there | `second-copy`, which is the original's but for the name; `start-memory`, by a patch of this build's own | Yes: entry 1, "The program of 32 bits", on the user's word of 13 October 2026 that it is held to the original where it does again what the original does | Nothing to decide. What `start-memory` still shows otherwise follows from the memory being the program's own (5) |
| 4 | `ended`: the program that the second copy leaves in the memory before DOS ends the copy | Runs it, and releases it when DOS ends the copy | Has no memory to run it | Runs it; it stays in the memory, 448 bytes, by entry 7 | No screen: the program that started the copy clears it. Seen by `mem /c` typed behind the steps | Yes: the row of `ended` in entry 1 | Nothing to decide |
| 5 | The room of the editor, which is also what a copy reads into | 345,840 bytes on the bench: a file of 381,338 is refused with a box | 291,360 | 641,824, of the 640 KB of the program's own: it opens that file | `editor-big`, which is not run for this build. `editor-full` and `copy-big` pass and no longer get to their point: the editor is never full, and the file is read in one part | Yes: entry 13, on the user's word, which names the two scenarios too | Decided. What is still open is what goes untested, and each way is asked first: (a) `editor-big` run for this build with its five steps at the file left out, which was tried and passes, and tests again all but the box for a file that fits; small. (b) A scenario of its own with a file that no build has room for; a new golden. (c) `editor-full` and `copy-big` made longer; two goldens derived anew, every tree taken through more steps. W would take (a) now and leave (b) and (c) until step 2 of the road, when the memory is the extender's and every one of these numbers is another |
| 6 | The program cannot start | Too little memory: "Program too big to fit in memory", and it ends with 1. A DOS before 3 it leaves without a word, and one before 3.20 with a line of its own. It runs on any PC | The same | Without the 512 bytes below the megabyte that the system's functions keep: the same words, by DOS, and it ends with 8. Without a 386, without the memory above the megabyte that it wants, some 900 KB, the extender does not start it and says so in words of its own: "DOS/32A fatal (4003): not enough extended memory to load application exec" was seen, of a copy. On a DOS before 3 the extender is what such a DOS meets first, since it asks DOS for its version before the program is entered (seen: AH 30h among its own calls); what it then does was not looked at, no bench having such a DOS, and entry 1 says nothing of it | None | In part: entry 1 has the words, the 512 bytes and the extender's words for too little memory. That it wants a 386, and the 8, are said nowhere | (a) A sentence in entry 1 for the 386, a DOS before 3 and the 8; the words stay. Nothing can be mended where the program does not run yet, and the road names the extender. (b) Other words for the case of the 512 bytes, which D2 left as the user's: `NoStart_S` of `dos32.c` is the one place |

**What only DOS, the machine or another program sees.** No screen of
the program shows any of these, and no scenario does.

| | What | The original | 16 bits | 32 bits | Declared | The ways |
|---|---|---|---|---|---|---|
| 7 | What the extender is in the machine while the program runs, a command of it included | Nothing: a program of real mode, a part of which stays in the memory while a command runs | A program of real mode, all of it in the memory while a command runs, by entry 1 | The extender's part of real mode, 51,696 bytes below the megabyte, and 512 bytes more that the system's functions keep there; INT 21h of real mode goes to the extender, and INT 15h (1); the timer's counter 0 is set anew when it starts, to mode 3 where the BIOS has mode 2; the line A20 is left on; the program runs at privilege level 0; and the program's idle call, INT 28h, comes from the extender, on a stack of its own, with DS and ES 0 | In part: entry 1 has the 52 KB and the 512 bytes | (a) A sentence in entry 1: it is what an extender is, and the road names this one. Nothing to mend |
| 8 | When DOS ends the program by another program's hand | Releases what it has on its list, tells DOS to look for Ctrl-C as before, and leaves registers for the program that started it. Ctrl-Break, INT 1Bh, is never given back and goes into memory that is free from then on; INT 22h goes into the ended program | DOS told and the registers left, as entries 7 and 8 have them; INT 1Bh and 22h as the original | DOS told and the registers left as in 16 bits, by the routine of real mode that the user decided for (E). But INT 1Bh is given back, by the extender, before DOS sees the call; INT 22h goes into the block below the megabyte, which DOS has freed, as the ended program is in 16 bits; and at the routine's call 3301h DH, DS and the registers that the call does not read are as they came from the program that started this one, which is what the original hands DOS there, where 16 bits has DH 0 and DS on the main data | Yes, since L: a point of entry 8, as the program is; the registers of the routine's call are in 6 | (a) A sentence in entry 8: the program of 32 bits leaves the machine with less astray than the original does, and making INT 1Bh point into free memory would be breaking the machine on purpose. Taken. It was to be seen again if 2 was mended by its bit; that mend was dropped, and whatever is decided of 2 that changes what the extender puts back wants this point read again |
| 9 | The registers that a call of DOS or the BIOS does not read: entry 6 | As the code before left them | By entry 6: 0 in AL and in the half of BX that is not taken; DS on the main data; elsewhere 0 or what the stack holds, "as it comes" in the rows of its table | 0 in AX, BX, CX, DX, SI and DI wherever a call takes nothing there, at every call. DS and ES of real mode are the extender's: seen, at a call that takes no address DS holds the number that is in AX and ES 0; at a call that the extender translates DS is on the extender's buffer below the megabyte; at the thirteen functions that are made in real mode both are on the program's block there. The carry flag is clear at the two calls that the original makes with it set, 36h and 13h, and counted as set afterwards, which is the same for a DOS that leaves it. At 4Bh the environment is named, where the original and 16 bits name none and DOS copies the program's own: the same environment, since the word of the PSP through which DOS would find it is a selector of the extender's; and CX, SI and DI are 0 | No: entry 6 is of the program of 16 bits, and D2 and E left its sentence for 32 bits to W | (a) A paragraph in entry 6, with no table: it is the entry's own rule, DOS handed nothing that it is not asked, kept more strictly. Nothing to mend |
| 10 | Which calls DOS is asked, and in which order | | The original's, call for call: entry 6 says so | Fewer, and one more: seen, call for call. Not asked: 51h, where the PSP is, the first call of 16 bits; 4Ah for the program's own block; 48h, 49h and 4Ah for the memory of the panels, the viewer and the editor, at the start and around every command, which are the program's own 640 KB; 35h and 25h for the critical error, Ctrl-Break and Ctrl-C, which are taken and given back through the extender, so that `ErrVct` and `BrkVct` of the main data are never written. More: 48h for the 512 bytes, when the program starts. Every other call is made by both, in the same order and with the same AX. And through the extender, where DOS does not see it: INT 22h read before a program is run and put back behind it, and the three interrupts given back when the program ends | In part: entry 1 has the 640 KB and the 512 bytes; the calls are said nowhere | (a) A paragraph in entry 6, which says of both builds that "the calls, their order and every register that a call takes are the original's": not so of this one. To mend it would be to take the memory from DOS, below the megabyte, which the session's choice 2 of the plan gave up and step 2 of the road changes again |
| 11 | What DOS left in AX where a call did not fail, on a DOS that changes AX there | As DOS left it | As DOS left it | As DOS left it at the calls that are made in real mode for that reason, the search, the current directory, the attributes set. At the other calls that the extender translates, 39h, 3Ah, 3Bh, 41h, 4300h, 56h, it may be the AX that the call was made with: FreeDOS leaves AX alone there, so the bench cannot tell | No | Nothing to decide today: not known to differ on any DOS. Written here so that it is not lost |
| 12 | Memory that the program reads before it has written it | What the last program left in DOS's block | The same | Zeros, in the 640 KB of its own | No | Nothing to decide: no place is known where it shows, and every scenario that passes went by it |

**Not a difference of the program.** A wait costs more of the host's
time while the program of 32 bits idles, `blank-timed` above all: that
is the bench's, and no screen differs for it. "T, the time of a wait"
in the plan has what it costs and why, the processor's mode changed
thirteen times at every round of idling, and that nothing in the
program mends it without asking DOS less. The
stack is 16,384 bytes where 16 bits has 4,096, and a critical error
goes 512 bytes deeper on it: the test holds half of it free as for
every tree, and H3 saw 14,790 bytes left at the least. Hosts that the
bench has not, EMM386 and a DOS box of Windows, where the extender goes
into protected mode another way, were tried by no piece.

**What a program asks of DOS, seen.** For 9 and 10, before they were
declared: `harness/trial32/calls.asm`, a program of real mode that takes
INT 21h, runs the program from the root of D:, and writes every call
that comes by to a file, with AX, BX, DS and ES as DOS gets them. Each
program was taken through its start, `ver` as a command, and F10 and
Enter, by the line at the toy's top, and the two files were held
against one another by a script of a dozen lines, kept in `build/`.

- The program of 16 bits: 171 calls, COMMAND.COM's for the command
  among them. That of 32 bits: 269, of which the first 119 are the
  extender's own while it loads the program: FF87h and FF88h, which
  are its own questions, 4Ah twice, 30h, 51h, 1Ah, the file opened and
  read by 3Fh and 42h, 48h and 49h, 3Eh.
- From the program's first call on, the two lists are the same call
  for call, AX for AX, but for the calls of the table in entry 6: 51h
  and the first 4Ah; 2523h, 48h twice, 3524h, 351Bh, 2524h and 251Bh at
  the start, and the same with 49h in front of the two 48h behind a
  command; 2524h, 251Bh and 4Ah in front of a command; 251Bh where the
  program is left. And 48h with 20h in BX as the first call of the
  program of 32 bits. The only other AX that differed was COMMAND.COM's
  own, at 4Ah, twice.
- DS and ES in the program of 16 bits: both on the main data, 2E10h,
  at every call but those that take an address elsewhere: 4Ah and 49h,
  where ES is the block; 25h, where DS is the segment of the routine;
  and 4Bh, where DS is on the environment, which has the shell's name.
  In the program of 32 bits: 1900h and 0 at
  19h, 2C00h and 0 at 2Ch, the same at 06h, 30h, 3300h, 3301h, 3306h,
  440Eh, 5801h, 5803h, 5902h and 4C00h; 3581h, the extender's buffer,
  at 09h, 3Dh and 3Fh, with ES 0 or a small number; 3AA0h in both, the
  program's block, at 1Ah, 38h, 47h, 4Bh, 4Eh, 4Fh and the 59h behind
  one of those.
- What it does not show: a call that the run did not come to, a file
  written, a directory made, a search of the tree; the calls of the
  BIOS and of the mouse driver, which are other interrupts; and the
  extender's services, by which the program asks where INT 22h goes
  and takes the three interrupts, which DOS does not see. The program
  finds no setup file next to itself in the root and starts on its
  defaults.

**How 2 was found, and seen.** D2 has the extender's warning for an
interrupt of real mode that is not as it was when the program ends, and
put INT 22h and the program's own three back for it. A program that
stays in the memory and takes interrupts leaves others changed, and the
bench has one, FDAPM, which the scenarios of the map of the memory ran
for the original. The same three steps for each program, `python3
harness/show.py --last --program FILE '"a:\freedos\bin\fdapm" Space
"apmdos" Enter' 'F10 Enter' '"a:\freedos\bin\fdapm" Space "stats"
Enter'`, and what DOS's screen has behind them:

| Program | Behind `fdapm apmdos`, when the program has been left | `fdapm stats` |
|---|---|---|
| The original | Nothing | "No FDAPM / POWER driver resident yet." |
| 16 bits | Nothing | "Found resident FDAPM / POWER driver version 1.0", and what it has saved |
| 32 bits | Four lines of the extender's warning, for INT 16h, 28h, 2Ah and 2Fh | "No FDAPM / POWER driver resident yet." |
| 32 bits, bit 0 of the settings' byte 15h cleared | The four lines | The same |
| 32 bits, bit 0 or bit 3 of the byte 14h cleared | Nothing | The same |
| 32 bits, bit 1, 2 or 5 of the byte 14h cleared | The four lines | The same |
| 32 bits, bit 4 of the byte 14h cleared | An empty line of DOS's prompt, which was not looked into | "Found resident FDAPM / POWER driver version 1.0", as in 16 bits |

What L found of these rows, by the extender's code: bit 0 is whether
warnings are written at all and bit 3 whether these are, so either
silences the lines; bit 2 is the table of interrupts put back, and with
it clear alone FDAPM still lost INT 2Fh, by which `stats` asks for it,
since the extender puts that one back whatever its settings say. And
the row of bit 4 is of no program: with that bit clear the extender
does not load the program, and the three steps were typed at DOS's
prompt. The first screen of such a run has the extender's words for
it, "DOS/32A fatal (4002)".

The byte 14h of the block of settings is 3Fh as the stub comes, and
`tools/stub.c` knew only the banner's bit of the byte behind it then
(L has both bytes bit by bit since). Each
of its six bits was cleared in a copy of the program, in a directory
named as the tree is, and the copy run once; nothing was kept, and
nothing but those three steps was run with any of them. So the original
and the program of 32 bits leave the resident program cut off, the
first by releasing it and the second with its memory still taken and
four lines on the screen, and the program of 16 bits leaves it at
work, which is what entry 7 declares.

**Whose these were, when the list was made.** The table at the top has
what was decided since: all of it, with 1 and 2 declared. What is left
for the user of what the pieces wrote down is not of this list: 2 and
3 of "The user's" at the end of L, the extender's variable and the
mouse driver.

- The user's, and not decided then: 1, which is 9 of what is open in
  the plan; 2, which was new; the sentences of 6, 7, 8, 9 and 10, each
  of which said of the program of 32 bits something that no entry
  said; and of 5 the three ways to test again what goes untested.
- Covered by a decision that was written down: 3 and 4, by the user's
  word on a second copy (`CLAUDE.md`, "The portable C", and entry 1); 5
  itself, by entry 13; the routine of 8, by the user's word on a
  program ended by another's hand; that the extender is DOS/32A and
  that the file is held to no length, by the road.
- Nothing to decide: 11 and 12.

Not looked at, and so in no entry: what a DOS before 3 and a machine
without a 386 are told, by the extender; and DS and ES of real mode at
the calls of the BIOS and of the mouse driver.

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

## The portable C in a flat memory: the plan

Step 2 of the road (`CLAUDE.md`, "The portable C"): the program "uses
what 32 bits give: one flat memory, with pointers where there were
segments and offsets, and numbers and values of 32 bits. A variable is
widened wherever wrapping round at 16 bits would be unexpected
behaviour, and stays 16 bits only where the program counts on the
wrapping. A number that is only ever true or false becomes a boolean."
This is the plan, made on 13 October 2026 at `9031733`, just before the
switch to the one build, by reading the tree and counting in it; nothing
of it was tried but that `wcc386` takes `_Bool`, and since then what
the trial X tried. The scripts that counted are a dozen lines each and
were not kept; each count says how it was made. It was revised twice on
that day, by the user's answers: "What is decided, and by whom" has
that. Steps 3 to 6 are not planned here.

### In short

- Step 2 is two things. **The addresses**: a segment becomes a pointer
  and the megabyte of step 1 goes. That is what a machine of 64 bits
  cannot do without. **The numbers**: one of 16 bits becomes one of 32
  or a boolean, or stays 16 bits with its reason written down.
- Both are of this step, on the user's word of 13 October 2026
  (`CLAUDE.md`, step 2 of the road): the addresses first, then the
  booleans and the widths together. And the memory for the panels and
  the texts comes from the extender from this step on, as much as the
  machine has.
- The addresses are the pieces F0 to F10: a net and a record, a trial
  of the extender's memory on the bench (X), one piece that proves the
  way, two sweeps that the compiler holds complete by the types, five
  pieces side by side by file, the memory taken from the extender, and
  the megabyte taken down.
- The numbers are the pieces N0 to N21, a cluster of files each, in
  which every number of 8 or 16 bits is sorted once and then widened,
  made a boolean, or kept with its reason in a record that a test
  holds. N0 makes the finder, the record and the named types, and
  tries the rule. A cluster starts when the piece of the addresses
  that has its files has landed, so most of them run beside F4 to F10.
- What crosses to the system, the size of a file, a count of bytes
  read or written, a size in the memory, gets a named type now, 32
  bits wide, so that the layer can say later how wide it is (S). Sizes
  beyond 32 bits are the layer's.
- The main data keeps the original's layout through all of it. A word
  of it that held a segment or a place leaves it and is a pointer
  beside it, and a number of it that is widened does the same; its
  word stays where it was, unused.
- The program has three blocks where it had one to cut: a block for
  each panel, as long as a panel can get, and the room behind them.
  They are made so first, inside the 640 KB (F1), and come from the
  extender when no segment names one any more (F9). Until then the
  640 KB and the megabyte stand, and the program runs after every
  piece.
- The numbers that count the room are widened before the room is the
  machine's: five clusters, N3, N5, N6, N9 and N11, stand between F4
  to F7 and F9. So no cap on the room is ever in a program that has
  landed, where the plan had one for as long as it put the widths
  behind the layer.
- A limit of the original that goes when a number is widened is a
  deviation, and each is asked when the work comes to it: the 256
  files kept over a command (4), the editor's room and its lines (8,
  5), a scenario of the portable C's own where the original has no
  golden (9), the files of a panel (3).
- With one build the bytes of the program check nothing any more. What
  takes their place: the main data and the program's blocks as they
  are at the end of every scenario, held against those of the build
  before (F0).

### What is decided, and by whom

| What | Who |
|---|---|
| The words of step 2, above, and that step 3 follows it, with the layer's design approved before anything is built on it | The user, 12 October 2026 (`CLAUDE.md`) |
| That every deviation is declared, and that any which is not the removal of something that only made sense on DOS is asked first; that a bug of the original stays until its fix is decided | The user (`CLAUDE.md`) |
| That the editor of the program of 32 bits has more room than the original's, declared and not mended by a number made for a test | The user, 13 October 2026 (`src/portable_c/DEVIATIONS.md`, 13) |
| That the memory for the panels and the texts comes from the extender from this step on, as much as the machine has, with what a user then sees otherwise declared: the editor opens whatever fits | The user, 13 October 2026 (`CLAUDE.md`, step 2 of the road): the answer to question 2 |
| That the widening is of step 2 and does not wait for the layer: the pointers first, then the booleans and the widths together, with the reason written down for every number that stays 16 bits and a named type for what crosses to the system; and that a limit of the original that goes when a number is widened is a deviation that is asked as the work comes to it | The user, 13 October 2026 (`CLAUDE.md`, step 2 of the road, as of `8f6a2b9`): the answer to question 1, as it stands |
| The first five ways under "The way taken" below, and the data's image as the net in the place of `CHECKED` (F0) | Accepted by the session, 13 October 2026 |
| Questions 6 and 7 at the end: the flags of the setup file stay bytes, and the main data keeps its layout through step 2 | Taken by the session as recommended, 13 October 2026. Neither changes what the program does |
| The sixth way: three blocks where there was one, with a panel's block fixed at 1000h paragraphs from F1 on; F9 as the piece that takes the memory from the extender, and the trial X at once | Accepted by the session, 13 October 2026 |
| The clusters N0 to N21 with S, M and P, and their order; that no cap is in a program that has landed, with F9 behind the five clusters that count the room, and the cap as the fallback, which wants the session's word; `FileSize`, `ByteCount` and `MemSize` and the answer of `FileRead` and `FileWrite` as a structure of their own, as the names of this step, which the layer may change; the 16 bytes round each block of the 640 KB, in the program as it is shipped from F1 to F9, held by F0's tool | Accepted by the session, 13 October 2026 |
| Questions 3, 4, 5, 8 and 9 | With the user, each when the work comes to it: "The questions for the user" |

For an hour of that day question 1 was answered otherwise: the
pointers, the booleans and a record of the widths in step 2, and the
widening after the layer's design, to the types that the layer names.
The plan had proposed that. It did not hold, for three reasons. Most
of what is widened is the program's own and never crosses to the
system: the number of a line, a column, a count of files, the length
of a buffer, a room counted in bytes where it was in paragraphs. What
does cross gets a named type now, so that widening it once more for
the layer is a line. And with the memory from the extender the limits
of 16 bits are within reach on DOS: the plan had to invent a guard for
that, the room capped at 0FFFFh paragraphs, which was a way round the
split and nothing else. One fault of the kind was real already, C1's,
a number of 16 bits added to a pointer. What the split left in the
plan is taken out; this paragraph is where it is told.

### What it rests on of the switch

The switch is the piece "W and Z1" of the plan for the 32 bits, in
five stages. Its first four have landed, at `8031e96`; of the fifth,
`resident.c` is renamed `exec.c` and the README and `DEVIATIONS.md` are
turned round, and the notes of `CLAUDE.md` are left. There is one tree, `portable_c`, built by `wcc386` with
DOS/32A, and the 16-bit build is gone, with `dos.c`, `arith.c` and
`harness/pass32.txt`. What the plan goes by of that:

- The tests of the portable C are `portable_c-scenarios`,
  `portable_c-data`, `portable_c-layout`, `portable_c-debug-layout`,
  `portable_c-host` and `portable_c-int16`. There is no test with
  `portable_c32-` in its name and no record of the scenarios: the
  scenarios' test holds the tree to every scenario that it is taken
  through, 155 of them, and so is every piece here.
- One file is behind `sys.h`, `dos32.c`, which keeps its name: "the
  system's file" below.

And of the third stage, Z2, which no piece but F0 started before:

- One body of each word of the addresses in `vc.h`, the flat one, with
  no `#ifdef __I86__`, and `LenStk` one number.
- `__far`, `FAR`, `STACK`, `CODE` and `TEXT_FAR` out of the source.
  The sweeps F2 and F3 go by patterns in the text, and are written
  against the text as that stage leaves it.

The fourth stage has said anew what `harness/hostcheck.py` and
`harness/int16.py` are for with one build left, and N0 builds on the
finder as it left it. Nothing here waits for the fifth.

The counts below are of the tree before the third stage, as it was at
`9031733`, with `dos.c`, which has gone since, and the definitions of
`vc.h` left out. The third stage changes none of them.

### The addresses: what there is

**The words**, as whole words outside comments, in the files of code:

| Word | Places | Files | The most | What it is in step 2 |
|---|---|---|---|---|
| `OFFSET` | 230 | 20 | `dirfunc.c` 48, `copy.c` 39, `edit.c` 29, `view.c` 28, `commands.c` 22 | Goes: a table is handed as a pointer to it. F3 |
| `PANEL` | 149 | 21 | `keys.c` 28, `commands.c` 20, `read.c` 16, `enter.c` 11, `main.c` 11 | Goes: a panel is a `struct WCBdef *`. F2 |
| `MD_PLACE`, `MD_POINTER` | 102, 48 | 23, 13 | `view.c` 17, `edit.c` 16, `usermenu.c` 15, `copy.c` 13, `enter.c` 11, `treebox.c` 10 | Go where the place is kept in a variable, which becomes a pointer. They are right C as they stand, a pointer taken from another of the same object, and rest on no megabyte: F4 to F8, by file |
| `AT` | 97 | 16 | `edit.c` 29, `dialog.c` 18, `view.c` 9, `stopts.c` 9, `main.c` 8 | Goes: what it adds to is a pointer. F3 for the tables and a dialog's fields, F4 to F6 for the texts |
| `MAIN_SEG` | 85 | 21 | `dirfunc.c` 19, `edit.c` 11, `view.c` 10 | Goes: `&md`, where a routine is handed what its dialog's fields are in. F3. Eight of them are arithmetic on paragraphs: F1 |
| `NEAR` | 61 | 12 | `dirfunc.c` 25, `copy.c` 13, `find.c` 6 | Goes: it is a cast that does nothing. F4 to F8 |
| `ENTRY`, `DSKBUF` | 60, 58 | 12, 15 | `enter.c`, `read.c`, `files.c` | Stay: an offset in a panel, added to the panel. See below |
| `AsmSeg` | 28 | 13 | `view.c` 6, `dialog.c` 5 | Goes, with `TABLES_BY`. F3 |
| `TRELIST` | 24 | 7 | `tree.c` 8, `treebox.c` 8 | A pointer to the tree's list. F7 |
| `NAMED`, `NUMBER_OF` | 15, 9 | 3, 1 | `dialog.c` | `NAMED` goes: the word of a table is read as the pointer that it is. `NUMBER_OF` stays. F3 |
| `OFFSET_IN_SEG`, `SEG_OF` | 8, 1 | 6, 1 | | Go. They are the bridge while the pieces land: see "The way taken" |
| In the tables: `MD_AT`, `WCB_AT`, `NUMBER` | 280, 3, some 65 | 15 files of tables | `dirfunc_t.c` 75, `stopts_t.c` 62, `commands_t.c` 45 | Stay as they are: an offset in whatever the dialog is handed |

**The kinds**, which is what the pieces are cut by:

| | Kind | How much | Where |
|---|---|---|---|
| a | A segment in a word of the main data | Seven words, 195 mentions: `WCBcrn` 47 in 15 files, `WCB1seg` 37 in 14, `WCB2seg` 32 in 13, `WCBsec` 16 in 7, `ViewSeg` 39 in 8, `MemSeg` 15 in 3, `FreMem2` 9 in 7 | F1 and F2 |
| b | A segment handed from routine to routine | `u16 panel` is declared 259 times, 88 of them in `vc.h`, and `panel` is written 719 times in 23 files. `u16 segment` 45 times: the text of the viewer and the editor (`view.c` 65 mentions, `edit.c` 116), the list of the file search, a copy's buffer, and what a dialog's fields are in (`dialog.c`) | F2, F3, F4 to F6 |
| c | A place of the main data in a word of it | `TreAdr` 28 mentions in 9 files, `TreFrst` 24 in 7, `TreCrnt` 4, `HstAdr` 8 in 4, `BatAdr` 8 in 2. `ComSpec` is a place in the environment and is added to what `ProgramEnvironment` answers: it stays a number | F7 |
| d | A place of the main data in a variable or handed on | The 150 of `MD_PLACE` and `MD_POINTER` | F4 to F8 |
| e | An offset in a panel: `CurAdr`, `First`, `EndBuf`, `EndFul` of `WCBdef`, and every `file`, `cursor`, `first` that is made of them | The 60 of `ENTRY` and more that are plain numbers | Stays |
| f | An offset in a text: the viewer's `struct Line`, `ViPoint`, `ViEndTab`, `ViFirst`, with its table of lines at the top of the same segment; the editor's, which are numbers of 32 bits of which 20 count already, `EdOffst`, `EdLen`, `EdFirst`, made a segment and an offset below 16 by `AddressOf` some 22 times | `view.c`, `edit.c` | F4, F5: `text + place`. `AddressOf` and `OFFSET_OF` go |
| g | A far address in the main data: `Screen`, `ErrVct`, `BrkVct`, the block `EPB` | 4 fields | Not step 2: only the system's file reads them. They leave the main data when that file moves under the layer |
| h | Arithmetic on segments | `PANEL_PARAS` and `VIEW_PARAS`, 9 places in `start.c`; the room behind the panels, `MAIN_SEG + md.FreMem2 - md.ViewSeg`, written out five times in `view.c`, `edit.c`, `copy.c` and twice in `find.c`, and twice otherwise in `read.c`; `panel + 0x1000` and `0xFFF` paragraphs in `ReDr00`, which is the 64K of a panel; `GiveMem`, `TakeMem`, `Aexec0` of `view.c` | F1 |
| i | The main data read as a panel, as the original does with DS left on it | 5 places: `BegBX(MAIN_SEG)` in `main.c` and `keys.c`, `PutPrm(MAIN_SEG)` in `enter.c`, `M60_02(MAIN_SEG)` in `commands.c`, and `md.ColTab[OFFSET_IN_SEG(panel, wcb->WinPath + 3)]` in `read.c` | F2: written as what it is, `(struct WCBdef *)&md`, with its comment. As far as was read they read within the first 82 bytes, which are bytes of the setup in both structures: F2 holds each to that |
| j | A number of 16 bits added to a pointer, which was right on DOS because the offset wrapped with it | `harness/trial32/added.py` lists 158 places and found none wrong; `harness/int16.txt` has 10 of the kind `back` and 4 of `minus`. C1's fault was of this kind and through no word (`TODO.md`, "C1, the editor") | Each piece reads those of its files again when the pointer changes: see "What could go wrong", 1 |

**The memory.** The program asks for the largest block that there is
and cuts it up itself: the first panel, the second, and behind them
whatever is at work, the text of the viewer or the editor, the list of
the file search, a copy's buffer, the old list while a directory is
read. A panel that grows moves the other one up (`ReDr00`), and both
are moved together before and after a command (`GiveMem`, `TakeMem`).
Nothing of that needs a segment: it is one block, three places in it
and its end. From F1 on it is three blocks, and nothing is moved: "The
memory from the extender" below.

### The way taken

1. **The main data keeps its layout**, `LAID_OUT(MainDat, 0xF48B)`,
   and so does a panel. A word that becomes a pointer leaves the
   structure for one beside it in `main.c`, and its word stays in
   `struct MainDat` with "of the original. Not set or read here", as
   `Snow`, `TopView` and `Memorys` are already. The plan for the 32
   bits had the layout freed in step 2; that is not taken, for what
   holds it:

   | Who reads the layout | What breaks if a field moves | The way out, if it were freed |
   |---|---|---|
   | The setup file: the first `CmnIni` bytes of the main data and the first `WCBini` of each panel, with a sum. Every one of those fields is a byte | Nothing, as long as those parts stay bytes: no pointer and no number wider than a byte is in them | None needed. Only a boolean in them would touch it: question 6 |
   | The driver of the tests: `AutoTim` and `Clock`, at the address of `md` plus the offsets of the original's listing (`harness/driver.py`) | Every step waits on the wrong word, and every scenario fails | The two offsets from the build: a constant of the program that holds `offsetof` of each, found by name in the linker's map |
   | The test of the data: a word of a table that names a place of the main data is held against the original's word, by the field of that name in the original's listing | Every such word fails, 280 of them | The same constant, for every field that a table names |
   | The tables themselves, `MD_AT` | Nothing: `offsetof` follows the structure | |
   | What is read behind the end of a field, a path copied as 68 bytes whatever it is, a title as 80: what follows is the next field, as in the original | What a scenario shows, or nothing, by chance. Nothing lists these reads for the main data as `harness/reads.py` does for the tables | Each found by a scenario that fails, or not at all |
   | The structures laid over its buffers, `EdData` and `ViewDat` in `TmpBuf`, `FndData` there too, `HlpData`, `StData` in `MenuBuf`, most with a `LAID_OUT`; the editor's fields lie over the viewer's on purpose | A field made wider moves the ones behind it, in one of two structures that have to agree | The two made one |
   | The five places of kind i | Nothing, if they read within the first 82 bytes, which F2 looks at | |

   So freeing the layout buys nothing that step 2 needs and risks the
   fifth row, which nothing would catch. Whether it is ever freed is
   for when the layer is designed.

2. **A segment becomes a typed pointer to what is there.** A panel is
   `struct WCBdef *`, a text `u8 *`. Where one routine is handed now a
   panel and now the main data, because a dialog's fields are in the
   one or the other, it takes a pointer to whichever it is and the
   table keeps the offset in it: `Dialog(&Filter_D, ..., panel)` and
   `Dialog(&Conf_B, ..., &md)`.

3. **An offset stays an offset where it is the original's limit or the
   base varies**: in a panel (kind e), whose 64K is what the offsets of
   16 bits make it and is question 3, and in a table (`mPoint`, `from`,
   `saved`). That is plain C, a number added to a pointer, and rests on
   no megabyte. `ENTRY` keeps its cut to 16 bits for as long as the
   offsets are 16 bits.

4. **A place of the main data becomes a pointer** where a variable or
   a word of the main data keeps it; the main data does not move.

5. **The megabyte is the bridge and goes last.** While it stands a
   segment and a pointer say the same thing, by `PANEL` or `AT` one way
   and `SEG_OF` the other, exactly, as long as the block begins on a
   paragraph of the megabyte, which the 640 KB of the system's file
   do. So a piece changes what it owns and writes `SEG_OF(pointer)` or
   `AT(segment, 0)` where it meets what another piece owns, and the
   program runs after every piece. The count of the words goes up with
   the first pieces and to nothing with the last; a record holds it
   (F0). F10 takes the megabyte down when the record has no word left
   that rests on it.

6. **The memory comes from the extender**, on the user's word, and
   the program has a block for each panel and one for the room where
   it had one block to cut. "The memory from the extender" below has
   it, with what stands meanwhile: the shape is made first, inside the
   640 KB, and the blocks are the extender's once no segment names
   one. The three functions of `sys.h` stay three, asked in the order
   in which the program asks today, in bytes of `MemSize` from F9 on;
   what the layer makes of them is step 3's. The screen, the BIOS's
   data, the PSP and the environment
   are behind `sys.h` already and stay there; `md.Screen` and the
   three other fields of kind g are the system's file's own and wait
   for step 4.

### The widths: what there is

**What 16 bits are today.** `u16` is written on 2,352 lines of the
tree and `u8` on 2,108, `u32` on 195; there is no number with a sign
but in two casts. Of 216 functions that return a `u8` or a `u16`, 46
return nothing but 0, 1 or a comparison (a count by pattern: it misses
what returns a flag in a structure). The member `failed` of the
answers of `sys.h` is read 281 times. 45 comparisons are with 0FFh or
0FFFFh, a number that stands for "none".

**Where a result is cut.** That is where a wider variable would keep
another number, so it is the reading list:

| What | Places | By |
|---|---|---|
| A result of more than 16 bits kept in fewer with no cast | 234, of which 232 into a `u16`: `edit.c` 23, `dialog.c` 22, `main.c` 17, `stcolor.c` 17, `view.c` 14 | clang, `-Wimplicit-int-conversion`, for the 386 |
| The same with the cast written | `(u16)(` 194 times and `(u8)(` 233 | `grep` |
| A result looked at before it is cut, and why it is the same number as on DOS | 59 with a reason: 29 `index`, 10 `back`, 8 `compare`, 7 `shift`, 4 `minus`, 1 `divide` | `harness/int16.txt` |
| A variable stepped below 0 or beyond 65,535 by `--`, `++`, `-=`, `+=` | Not counted: no warning names them. The finder of `harness/int16.py` knows the range of every expression and can | N0 |

A guess from those: 600 to 1,000 places to read, of which most cannot
leave 16 bits.

**What the program counts on**, as far as the notes have it: the two
random numbers, `Rand1` and `Rand2`; the sums of the setup file and of
the tree's file; a loop that goes round 65,536 times for a count of 0
(the editor's rows, `do ... while (--count)`); a number 0FFFFh for
"none" that is made by stepping 0 back; a step of -160 kept as FF60h
(`T71_00`); the 20 bits of an offset in the editor's text; what P4
gave a cast to say what DOS does, 21 places.

**What is not the program's to widen**: what a file has (`DTAs`,
`DIRENT`, `DIRTREE`, the bytes of the setup), and what DOS and the
BIOS take and answer in (a handle, attributes, a time and a date, a
key, a cell of the screen, a place of the cursor, the mouse's). Of
`sys.h` the counts of bytes and the sizes are widened, to named types:
"What crosses to the system" below.

### The rule for a number

Every number of 8 or 16 bits is one of five things, and the cluster
that reads a file says of each which:

| | It is | It becomes | How it is known |
|---|---|---|---|
| 1 | As wide as it is by what it is. Laid out by someone else: a field of a structure that a file, DOS, the BIOS or the screen has, or of the main data and a panel, whose layout is the original's. Or a thing that has its width: a character, a colour, a key with its scan code, a cell of the screen, a handle, a byte of the setup | Stays. A number that is taken out of a structure into a variable is of the other kinds | The structure says whose it is, once, in the record; a variable by one word there, `character`, `colour`, `key` |
| 2 | True or false and nothing else: every value put into it is 0, 1 or a comparison, and every use asks whether it is 0 | `bool` | The finder: a range of 0 to 1 and no use in arithmetic, in a shift, or held against a number other than 0 and 1. A number that is tested by its bits, held against 0FFh, or returned as "1, 0, or 100h to stop" is none |
| 3 | Counted on to wrap round | Stays 16 bits if it wraps all its life, a random number, a sum. If it wraps at one place, the variable is widened and the cut is written there, `(u16)(a - b)`, with the reason in the record | Read, at every place where the finder says that what is kept is cut |
| 4 | A number that may stand for one below 0 and is added to a place or a pointer | A number with a sign, of 32 bits: a new type of `vc.h`, the tree's first | The same reading. It is what a machine of 64 bits gets wrong where 32 bits happen to get it right: FFFFFF60h added to a pointer of 64 bits is 4 GB on |
| 5 | Anything else: a count, an index, a length, a number of a line, a size in the memory | `u32`, or the named type where it is handed to the system or comes from it | What is left |

And three rules for what a reader finds at a place where a result is
cut today and would not be:

- It cannot leave 16 bits there, by what stands at the place: widen,
  and nothing changes.
- It can, only with more memory or more files than the original can
  have: widen. Where the program of 32 bits can have that much, which
  it can from F9 on, what a user then sees is in "What a user would
  see otherwise" or it is handed back.
- It can, in the original, within a user's reach, and what the
  original then shows is wrong: that is a bug of the original. It
  stays as it is, with the cut written out, and the session asks the
  user with what was seen (`CLAUDE.md`: "Any other deviation, a fix of
  a bug of the original included, is asked first").

A limit that the program holds itself to, a list that is full, a file
that is too large for Edit, is none of these: nothing wraps there. A
variable behind such a check is widened and the check stays as it is,
its number written as the program's own. Moving or lifting it is a
deviation and a question: "The questions for the user".

**The main data and a panel** are of kind 1, field for field. A field
of them that the rule would widen, a count that can leave 16 bits,
leaves the structure as a word that becomes a pointer does: a variable
beside it, its word left unused. No cluster does that: N0 sorts the
fields once, the record names those that are to leave with every file
that names them, and one piece takes them out when the clusters are
done, M. Until then a cluster that reads such a field into a variable
of 32 bits has what the field holds.

**The structures that a file lays over a buffer of the main data** are
not of kind 1: `EdData` and `ViewDat` in `TmpBuf`, `FndData` there
too, `HlpData`, `StHlpData`, `StData`. Nobody but their file reads
them by their fields, and a field of one is sorted as a variable is.
Where one is widened the structure grows, and what holds it is its
`LAID_OUT` line with the new length and a `HOLDS` line that it fits
its buffer. Two things to look at first. The last 38 bytes of `EdData`
are the first 38 of `ViewDat`, on purpose, `EdSpace` on `ViSpace`,
`EdLen` on `ViLen`, `EdTxHex` on `ViTxHex`, `EdBase` on `ViBase`: a
field made wider in one moves what both read, so N6 names what the two
files share, in one structure that both begin with or by a `HOLDS`
line for each shared field, before it widens anything. And a table
may name the same bytes for another use: the tables have `TmpBuf[100]`
fifteen times, which is where `EdData` begins.

**Across clusters.** A parameter that is widened changes no caller,
but one that counted on the call to cut what it hands over: the finder
names those, and the cluster writes the cut at the call, in whichever
file that is. A return that is widened is cut by a caller that keeps
it in 16 bits, as it was: the place is in the record of the caller's
file, and its cluster sorts it, or the widening cluster does if that
file is done. So the clusters need no order among themselves.

**The record.** `harness/width.txt`, held by a test as
`harness/int16.txt` is, so that a reviewer reads one file: every
structure of kind 1 by name with whose it is; and every variable,
parameter, return and cast that is still 8 or 16 bits outside those,
with its reason, by file, function and the text of its line. For the
common reasons it has one word, as `harness/int16.txt` has `index` and
`back`, and a sentence for the others. The test fails for a number
that is not in the record. A file that no cluster has had yet is in
it as a whole, `not sorted yet`; when the clusters are done the record
is the list of everything narrow that is left and why.
`harness/int16.py` is its finder: what that test said of two builds,
where a number is looked at before it is back in 16 bits, is with one
build exactly where a wider variable changes a result, and its
reasons are the first entries. Two things the finder has to learn in
N0: a variable stepped beyond its width by `++`, `--`, `+=`, `-=`, and
a number cut by being handed to a parameter that takes fewer bits.

### What crosses to the system

Three named types, defined in `sys.h` by N0 with what each counts,
and 32 bits wide in step 2:

| Type | What it counts | Where |
|---|---|---|
| `FileSize` | The length of a file and a place in one, counted from its start. With it a type with a sign for a step from where the file is or from its end, of kind 4 | `FileSeek`'s offset and the `place` that it answers with; the sizes that the program adds up and shows |
| `ByteCount` | How many bytes are read or written at once | `FileRead`'s, `FileReadBare`'s and `FileWrite`'s `count`, and what they answer: a structure of their own in the place of `struct Answer`, with the count in this type and the error beside it, so that a caller which was not changed does not compile |
| `MemSize` | A size in the memory, in bytes | What the three functions for the memory take and answer, from F9 on; what the room's function answers, from the first of the five clusters that count the room |

And `DriveSpace`'s three figures, the clusters that are free, the
clusters of the drive and the bytes of a sector, become `u32`: plain
counts, of kind 5.

Who does it: S, a sweep that the compiler holds complete, for the
files and the drive; F9 for the memory, where the three functions
change in what they are asked and answer and not only in their unit
("X, the extender's memory on the bench").

**What DOS is asked does not change by it.** DOS takes a count of 16
bits. The system's file loops where it is asked for more than DOS
takes at once, and stops at a piece that came back short. No caller
asks for more in step 2: a copy reads and writes 8000h bytes at a
time, the editor its pieces, and those pieces stay, since they are
what DOS is asked and what a user sees between two of them, a disk
that is full, Ctrl-Break. So the loop is written and no scenario runs
it; S gives the toy of `harness/trial32/` a read and a write of more
than 64K, so that it has
run once.

**What is left for the layer**: sizes beyond 32 bits, and all of the
shape of `sys.h`. "Left for the layer's design", at the end.

### Booleans

- `wcc386` takes `_Bool`: one byte, also in a packed structure, and a
  number made one is 0 or 1 by the code (`test`, `setne`). Tried.
- The tree takes `bool`, `true` and `false` from `<stdbool.h>`, in
  `vc.h` beside `<stddef.h>`: a header that a compiler has without a
  library, Open Watcom's among them, by the directory that the build
  names already. A `typedef` of the tree's own would do for both
  compilers today and stop clang the day its C has `bool` as a word of
  the language.
- A flag in a file or handed to DOS keeps its width there: the
  structure is of kind 1, and the code says `!= 0` where it reads and
  writes 0 or 1 where it sets. The bytes of the setup are the case
  that matters: question 6.
- Making a number a `bool` is silent in C where it is wrong: 0FFFFh
  put into one is 1, and one held against 0FFFFh is never equal. So
  the finder sorts first, and after a piece clang is asked for what it
  says of the result, `-Wtautological-constant-out-of-range-compare`,
  `-Wconstant-conversion`, `-Wint-in-bool-context`, each of which
  names such a place.
- `failed` and its like in `sys.h` are the layer's shape and are left
  to step 3: `Refused` of the system's file answers 0FFFFh in one.
- The flags of the setup stay bytes, taken by the session: question
  6.

### The memory from the extender

On the user's word of 13 October 2026: from this step on the memory
for the panels and the texts comes from the extender, as much as the
machine has, and what a user then sees otherwise is declared. The
bench has 16 MB.

**It cannot be the first piece.** A block that the extender gives lies
wherever the extender has it, megabytes from the main data, and a
segment of the megabyte names nothing there. So the memory is taken
from the extender when no segment names a block any more: after F2 and
F4 to F8, as F9. What can be done first, and is, is the shape.

**Three blocks, where there was one to cut** (F1, inside the 640 KB):

| | Today | From F1 on |
|---|---|---|
| A panel | Begins as long as 256 files need, and grows by having the other panel and the room moved up, as far as 64K or as the memory goes (`ReDr00`) | A block of its own, 1000h paragraphs, which is as long as a panel's offsets of 16 bits reach. Taken once; never moved, made longer or given back |
| The room: the text of the viewer and the editor, the list of the file search, a copy's buffer, the old list while a directory is read | What lies behind the second panel, to the end of the block | A block of its own, the largest that there is when the two panels have theirs |
| `Init11` | Asks how large a block can be, takes it, makes the two panels at its start | Takes the two panels, asks how large a block can then be, and takes it for the room. Fails as today when the room is less than the least that the viewer needs |
| `GiveMem`, before a command | Puts the panels one behind the other and makes the block as short as the two | Gives the room back. The panels stay |
| `TakeMem`, after a command | Gives the block back, takes the largest that there is, moves the panels if that is another | Takes the largest block that there is for the room, and fails as `Init11` does |
| `Aexec0` of `view.c`, a viewer or an editor of another's run | The block made as short as the panels and the least of the viewer, then as long as it goes | The same of the room: made as short as the least of the viewer, then as long as it goes |
| The room behind the panels, worked out in seven places | `MAIN_SEG + md.FreMem2 - md.ViewSeg`, paragraphs | One function, which answers in paragraphs as the seven places did. Inside the 640 KB that cannot pass A000h, and no cap is written: see "The numbers that count the room" |
| The words `ViewSeg`, `MemSeg`, `FreMem2` | In the main data | Unused there. Beside it: where the room begins and where it ends |

A panel that never moves takes away what was most to fear of pointers
to panels, a pointer that is stale after a directory was read. The
room may still move, when it is made longer or taken again: whoever
keeps a pointer into it across `GiveMem`, `TakeMem` or `Aexec0` reads
it again.

Inside the 640 KB the room is then some 511 KB on the bench, where it
is some 627 KB today, the panels being 64K each from the start: F1
changes the figure in entry 13 of `DEVIATIONS.md`. No scenario is
expected to say so; `copy-big` reads its file in two parts again, and
ends on the same screens.

**From the extender** (F9). The three functions of `sys.h` for the
memory answer with a pointer to a block that the extender gave, DPMI
0501h, and take a number of bytes of 32 bits where they took
paragraphs of 16: how large a block can be is what DPMI 0500h says is
free. The system's file keeps the extender's name for each block by
its address, for 0502h and 0503h. The 640 KB and what hands them out
go from that file, and the program is that much shorter in the memory;
its file is not, since they have no value. What the extender does with
such blocks was tried on the bench, and "X, the extender's memory on
the bench" has it with what F9 goes by: above all that 0503h, refused,
does not say how long a block could be, as DOS's 4Ah does and `Aexec0`
asks; that a block is given with what the memory held; and that the
extender ends a program which has written behind the end of a block.

**A command run from the program.** The original gives its memory
back to DOS around a command and takes it again. The program of 32
bits asks DOS for none of that today, since its blocks are of its own
640 KB (`DEVIATIONS.md`, 6, "The program of 32 bits"), and will ask
DOS for none of it then: the extender is asked, by INT 31h. What a
command finds of DOS's memory is what it finds today. What it finds
above the megabyte is nothing, today and then, since the extender
holds all of that from when the program starts (entry 1, "The memory
above the megabyte, while a command runs").

So nothing has to be given back for a command of DOS. The room is
given back all the same, as the table has it, for the one command
that takes memory from the same extender: a second copy of the
program, and any program of that extender. If the first copy kept
all that the machine has, a second would find no room and leave with
the original's words for no memory, where the original runs
(`second-copy`, `start-memory`). That a second copy gets what the
first has given back, and nothing while the first holds all, was seen
on the bench with toys: "X, the extender's memory on the bench", rows
17 to 20.

**The 256 files kept while a command runs** (`F21_40` and `F21_31`
of `enter.c`, `enter-many`). Nothing needs them cut from F1 on: a
panel keeps its block. F1 cuts them all the same, since what a user
sees of it is behaviour and a golden holds it. Whether the cut goes
is question 4, which is asked when F1 has landed, and N14 carries the
answer out.

**The numbers that count the room.** The room on the bench is some 15
MB once it is the extender's, and these are 16 bits today. Each is
widened before F9, by the cluster that has its file, so F9 lands with
nothing that 15 MB can wrap:

| Still 16 bits | What more memory would do | Widened by | If F9 had to land first |
|---|---|---|---|
| The room in paragraphs, as its one function answers it from F1 on, and what takes it: `EdSpace` of the editor, `count` of the viewer, `room` of the file search, a copy's `left`, `size` in `ReDr00` | Wrap at a megabyte: 15 MB would count as what is left of it beyond a multiple of a megabyte | The first of the five clusters to get there adds a function beside it that answers bytes in `MemSize`, each of N3, N5, N6, N9 and N11 moves the calls of its file to that, and F9 takes the one in paragraphs out, which nothing calls by then: a call that was left does not compile | The function in paragraphs answers no more than 0FFFFh, 1,048,560 bytes, as a number of the program's own, declared in entry 13; the last of the five takes that out |
| How many lines the file search keeps, `FndSize`, and its other counts, `FndCur`, `FndFrst`, `FndNumb`, `FndFls` | Wrap at 4.2 MB of room | N5. The list is then full where the room is, some 230,000 lines on the bench | Some 16,130 under the cap |
| The editor's line in hex, `EdLine` and `EdLin0`: a sixteenth of the place | Wrap at a megabyte | N6 | The cap is exactly what they count |
| The editor's count of free bytes, which shows 0 for a megabyte or more in a text, and has seven cells; the 20 bits of a place in its text | A text beyond a megabyte | F5 takes `AddressOf` out, and with it the 20 bits. N6 the test of the count, and what is shown in the cells by the answer to question 8 | Not reached under the cap |
| The editor's line in text, 16 bits | More than 65,535 lines: within reach in the original with lines of fewer than six bytes | N6, by the answer to question 5 | Not guarded by the room |
| The viewer's window on a file | Nothing: the viewer takes 0FFEh paragraphs at the most itself, and that stays its number. N3 writes it in bytes | N3 | None needed |
| A panel's offsets | Nothing: its block is 1000h paragraphs whatever the machine has | Not widened in the clusters: question 3, and P if it is lifted | None needed |
| What `Init11` answers with for `AlocEr`, paragraphs in 16 bits that the BIOS is then handed as a page | Cut | F9 answers 0FFFFh for more, with a row in entry 6 | The same |
| What the functions of `sys.h` are asked for, paragraphs in 16 bits | No block of more than a megabyte could be asked for | F9: bytes in `MemSize` | The same |

Whether that order is the better one, since F9 could as well land
first with the cap in the last column: it is, and the plan is made
so. Before F9 the room lies in the 640 KB and no number of 16 bits
can be passed, so the five clusters change nothing that a scenario
shows and are judged as every cluster is. F9 is then the one commit
in which the room becomes the machine's, with one declaration, entry
13 written once, where the cap would have had a megabyte declared and
taken back, and `view-window` passing in between by 16 bytes, a
megabyte less a paragraph against a file of a megabyte. What it
costs: F9 and F10 wait for five clusters behind F4 to F7, an agent
each, side by side; and questions 5 and 8 are asked before F9. What
is lost is a commit that changes nothing but the number: F9 changes
whose the memory is and how much of it there is at once. So F9 runs
the scenarios once with the room cut to what the 640 KB gave, in its
own checkout and not committed, before it runs them with all of it,
and a scenario that fails only the second time has met a number and
not a block. If a question keeps one of the five from landing, the
session may have F9 land first with the cap after all; the last
column is that fallback, and wants the session's word.

A cluster that widens a number which counts memory, or a place in a
block, says in its commit how it knows that nothing writes behind the
block's end with the number wide: by the check that stands at the
place, named, or by the block's length worked out beside it. "What
could go wrong", 14 and 15, says what finds a write that was missed.

### What a user would see otherwise

Once the room is the machine's, from F9 on, some 15 MB on the bench.
"Covered" is by the
user's answer of 13 October 2026, "an editor that opens whatever fits,
a visible change to declare now"; F9 declares each in entry 13 or
entry 1 of `DEVIATIONS.md`.

| | What | Today, 640 KB | With the machine's memory | Scenarios | Covered, or a question |
|---|---|---|---|---|---|
| a | The editor's room | 524,240 bytes since F1, on any machine, and 641,824 on the bench before; the original has 345,840 there. A file that is not shorter is refused, "is too large for Edit." | What the machine has, some 15 MB on the bench, with the same box for a file that is not shorter | `editor-big`, not run today. `view-window`, and step 38 of `editor-big`, have a file that says a megabyte in its entry, which is refused today and fits then: question 8, "The test" | Covered. What the status line shows of so much room is question 8, and a test of a text beyond the original's room question 9 |
| b | The editor with its memory full | Never full in the scenario | The same, with more left | `editor-full`, which passes without reaching its point | Covered. Untested, as today |
| c | The editor's count of free bytes in its status line | What is left of the room, in seven cells | The same of the larger room, which from a million on does not fit the cells | Left out of the comparison | Covered as a figure. What stands in the cells from a million on is question 8 |
| d | What the viewer reads at a time | 65,504 bytes at the most | The same: it does not take more | `view` and its like | Nothing to see |
| e | A copy's buffer | The room in whole 512 bytes; a longer file is read and written in parts | The same of the larger room: a file of up to some 15 MB is read whole before any of it is written. DOS is asked in the pieces of 8000h bytes that it is asked in today. A user sees parts only where something comes between two, a disk that is full or changed, Ctrl-Break, a file that is skipped half read | `copy-big`, which reads its file in two parts again since F1, as the original does; `copy-full` | Covered: entry 13 says it of 640 KB already |
| f | The file search's list | Full at some 9,870 lines on the bench | Full at some 230,000, with its counts in 32 bits (N5) | None fills it | Covered: entry 13 names it |
| g | The tree | 768 directories, a buffer of the main data | The same | `tree-deep` | Nothing to see |
| h | The files of a panel | 64K, 2,704 files, or fewer if the memory is short | 64K, 2,704, always: a panel has its whole block from the start. The original shows fewer on a machine with little memory | None fills a panel | Covered, as more room. The limit itself is question 3 |
| i | The old list while a directory is read, by which the cursor and what is selected are kept | As much of it as the room holds, 64K at the most | The same | | Nothing to see |
| j | A machine with too little memory | The extender does not load the program, which has the 640 KB in it, and says so in its own words: 892,606 bytes above the megabyte, for its two objects | The program is 640 KB shorter, some 237 KB, and the extender loads it where that fits; with less, the extender says so as today, "DOS/32A fatal (4003): not enough extended memory to load application exec", and the program ends with FFh. Loaded, it wants two panels and the least of the viewer besides, some 139 KB; with less, `Init11` fails and the program leaves with the original's words for no memory (`AlocEr`). Some 376 KB in all, where the plan had 140 before X | None: the bench has 16 MB. `harness/trial32/second.c` makes the bench such a machine for the program that it runs | Covered: toward the original. Entry 1, "what the program wants of the machine", is written anew |
| k | "Program too big to fit in memory", of the system's file | For want of the block below the megabyte that the calls in real mode need | The same | | Nothing changes |
| l | "Not enough memory to load", a viewer or an editor of another's | For want of DOS's memory | The same | | Nothing changes |
| m | What a command finds of memory | DOS's whole; none above the megabyte | The same | `command`, by what `mem` prints | Nothing changes. X saw it: `mem` and `above.asm`, run from a toy that holds the largest block of the extender's and from one that has given it back, are told the same, which is what `command`'s patch has today (row 21 of X) |
| n | A second copy of the program | Runs, with the 640 KB of its own | Runs, with what the first copy gave back. It is loaded behind the first copy's two panels, not right behind the first copy | `second-copy`, `start-memory` | Nothing to see. X saw a second program of the extender get what the first gave back, by 0502h or by 0503h, and none while the first holds all (rows 17 to 20 and 27 of X). The driver of the tests is told the copy's new place, which is F9's |
| o | The 256 files kept while a command runs | Cut | Cut, until question 4 is answered | `enter-many` | Question 4, asked when F1 has landed |

What the scenarios lose by it is what they have lost already: the
refusal of a file that is too large and the editor with its memory
full, which no scenario reaches in this program (`TODO.md`, "The
editor's room: untested in the program of 32 bits"). The first can be
had again against the original's golden, by a file that is too large
for both, and what lies beyond the original's reach only by a
scenario of the program's own: questions 8 and 9.

### The tools that judge

| Tool | In step 2 |
|---|---|
| The scenarios | The judge, all 155 for every piece. No piece is to change a screen or a file but F9, by what is declared, and a piece that carries out an answer of the user's to one of the questions |
| The bytes of the program | Gone as a check, with the second build |
| The data's image, new (F0) | At the end of every scenario, the main data and each block that the program has, by name, a panel, the other panel, the room, by QEMU's `pmemsave`, which `harness/dos.py` has; a tool holds the images of two builds against each other. For a piece that is to change nothing they are equal but for what the piece names. It is what would have shown C1's fault, where eleven scenarios passed over a write 64K off. Not a test of the build: run by the piece, before and after |
| The words' record, new (F0) | The count of each word of the addresses by file, held as the host check's record is. It is the measure of the addresses: up at F1 to F3, nothing left that rests on the megabyte at F10 |
| The 16 bytes round a block, new (F1) | A pattern in front of each block of the 640 KB and behind the last, held by the image's tool at the end of every scenario: a write just behind the end of a block, or in front of one, is named with its scenario from F1 on, where the extender ends the program for it from F9 on. "What could go wrong", 15 |
| `portable_c-host` | Clean already in every file of the common code, since gcc reads the flat bodies, which cast nothing: it does not measure step 2. It guards it: a file must stay clean. Of what it still counts, `aborts` (5 places) can go to a word of `vc.h` in F10; `data_seg` and all of the system's file wait for the layer |
| Not counted by it, and worth knowing | gcc's `-Wcast-align=strict` names 376 casts that make a pointer to something wider of a pointer to bytes, `copy.c` 85, `view.c` 64, `stcolor.c` 53. Right on the 386 and on x86-64; what clang's sanitizer will name first in step 6. Not step 2's to mend, and not held to |
| `portable_c-int16` | Its question, whether two builds differ, is gone with the second build. Its finder is the finder of the widths (N0), and its record the start of theirs. N0 has made that finder, `harness/width.py`, and left this test, its record and its reasons beside it: a widening moves its places, and each is read as before |
| The test of the data and `harness/reads.py` | Unchanged by the plan, since no table and no layout changes. `reads.py` goes by the text `OFFSET(name)`: F3 teaches it the new form |
| The test of the layout | Loses in F10 the three rules that are of the megabyte: what a segment names within it, the tables within 32K of `Title_S`, `IniSave` within 64K of the main data |
| `CHECKED`, the debug program's checks | Not worth writing now. They were to be a third body of the words, and the words are what goes; and C1 showed that the fault to fear goes through no word. The image is the better net and costs less |
| clang's sanitizers | Step 6. F10 leaves the tree with no address formed outside an object by the megabyte, which is what they would stop at first |
| A scenario of the program's own, with no golden of the original | Not there, and not without the user's word. Question 9 has the least that it takes |

### The pieces

F are the addresses and the memory, X a trial, N the numbers, the
booleans and the widths together, S what crosses to the system, M and
P what is left at the end. Sizes are guesses; a sweep is small in
thought and long in lines.

| Piece | What it changes | Owns | Needs; beside | Right when | Writes down |
|---|---|---|---|---|---|
| F0, the net and the record. **Done**: "F0, the net and the record" below | The data's image at the end of a scenario, of the main data and of the program's blocks by name, and the tool that holds two builds' images against each other. The record of the words, with its test | `harness/`, `CMakeLists.txt` for the test | Nothing | The image of a build against itself is equal; against a build with one byte of the main data written wrongly by hand, it is not. Every golden derived again leaves `harness/goldens/` clean | What differs between two runs of one build, if anything, and how a piece names what it means to change |
| X, the extender's memory tried. **Done**: "X, the extender's memory on the bench" below | A toy on the bench, of `harness/trial32/memory.c`: what DPMI 0500h says is free of the 16 MB; the largest block taken, given back and taken again, and where it then is; a block made shorter and longer, 0503h, whether it moves and keeps what it held; a second copy of the toy while the first holds the largest block, and while it has given it back; what a command's `mem` prints either way; and by `harness/trial32/calls.asm`, that DOS is asked nothing by any of it | `harness/trial32/` | Nothing; beside F0 and F1 | It has run, and the table of what it found is in this plan | What F9 goes by: whether the room is all that the extender has or leaves some, and whether rows m and n of "What a user would see otherwise" stand |
| F1, three blocks where there was one. The proof. **Done**: "F1, three blocks where there was one" below | Inside the 640 KB: a block of 1000h paragraphs for each panel and one for the room. `Init11`, `GiveMem`, `TakeMem` and `Aexec0` as "The memory from the extender" has them; `ReDr00` moves no panel; the room is one function, which answers in paragraphs of 16 bits as the seven places did and needs no cap, since inside the 640 KB it cannot pass A000h; 16 bytes of a pattern in front of each block and behind the last, written when a block is taken, which the image's tool holds at the end of every scenario ("What could go wrong", 15); `ViewSeg`, `MemSeg`, `FreMem2` leave the main data. Elsewhere `SEG_OF(pointer)`. Entry 13 gets the room's new figure | `start.c`, `main.c` for the structure, `vc.h`; the hand-out of the 640 KB in the system's file and the image's tool, for the 16 bytes; the top of `ReDr00` in `read.c`, `Aexec0` in `view.c`; the 63 places that name the three words | The switch's third stage, F0 | The scenarios, `start-memory`, `second-copy`, `command`, `enter-many` above all; the images: the main data equal but for the three words, each panel equal as far as its list goes, the room equal where it was written | Whether the bridge is as cheap as thought, what the image showed, and how long the piece was: the sizes of F2 to F8 are set by it |
| F2, a panel is a pointer. A sweep | `WCBcrn`, `WCBsec`, `WCB1seg`, `WCB2seg` beside the main data; every `u16 panel` a `struct WCBdef *`; `PANEL` gone; the five places of kind i written as what they are | Lines of 23 files, by a script and then by what the compiler refuses: a number where a pointer is taken stops the build | F1; alone | The scenarios; the images equal; `PANEL` at 0 in the record | Every variable that held a panel under another name, and every place where two panels are held against each other |
| F3, what a dialog is handed. A sweep | `Dialog`, `DialBox`, `Ask`, `ClsLine`, `MinMnu` and their like take a pointer to the table and a pointer to what its fields are in: `OFFSET`, `AsmSeg`, `TABLES_BY`, `NAMED` gone, `MAIN_SEG` gone from their calls. `harness/reads.py` knows the new form | `dialog.c`, `misc.c`, `tables.h`; lines of 21 files; `harness/reads.py` | F2; alone | The scenarios; the test of the data with the counts of `reads.py` as before; the images | What a table is typed as where it is handed on, since each box is a structure of its own |
| F4, the viewer and the file search | The text a `u8 *`, the table of lines at its top, the list of the search; their places of the main data. The room, the viewer's `count` and `FndSize` are left as wide as they are, for N3 and N5 | `view.c`, `find.c` | F3; beside F5 to F8 | The scenarios of the viewer and the search; the images. And the machine's clock of the step that searches for a text, counted before the piece and after it, in `view-search` and in `find`: it is three and a half times the original's there, 8.70 seconds against 2.60 and 11.07 against 3.01, which the C translation does not show and nobody has looked into ("The machine's clock of the portable C against the original's", above, has the figures and how they are counted) | Where the viewer counts on an offset wrapping at the top of its segment |
| F5, the editor | The text a `u8 *` and a place in it a number added to it; `AddressOf`, `struct Address`, `struct Where` go; its places of the main data. `EdSpace`, its lines and its count of free bytes are left as wide as they are, for N6 | `edit.c` | F3; beside | The editor's scenarios; the images. The piece most likely to be more than one agent holds: `T50_00` to `T54_00`, which move the text, can be cut off | Every place where a segment was stepped to keep an offset small |
| F6, a copy and what goes through files | The buffer of a copy; `NEAR`, 38 places; their places | `copy.c`, `dirfunc.c` | F3; beside | Their scenarios; the images | |
| F7, the tree, the history, the commands | `TreAdr`, `TreFrst`, `TreCrnt`, `HstAdr`, `BatAdr` beside the main data as pointers; `TRELIST`; the old list of `ReDr00` | `tree.c`, `treebox.c`, `place.c`, `panel.c`, `files.c`, `read.c`, `cmdline.c`, `enter.c`, `exec.c`, `extfile.c`, `usermenu.c`, `input.c`, `info.c`, `drive.c` | F3; beside | Their scenarios; the images | |
| F8, the rest | What is left of the words in `main.c`, `keys.c`, `commands.c`, `menu.c`, `help.c`, `bars.c`, `window.c`, `system.c`, `setup.c`, `start.c` and the setup's files | Those | F3; beside | The scenarios; the images | |
| F9, the memory from the extender | The three functions of `sys.h` answer with a pointer to a block of the extender's and take bytes in `MemSize`; the 640 KB go. `DEVIATIONS.md`: entry 13 written anew, with no cap and every row that "What a user would see otherwise" has as covered; entry 1, what the program wants of the machine; entry 6, that the extender is asked and DOS is not. The refusal of a file that is too large held to the original's golden again, and `view-window` kept as it is, by a scenario's machine with less memory: question 8, "The test". By what X found: the panels are taken first, so that the room has nothing behind it; "as long as it goes" is the room given back and taken anew, since 0503h refused answers no length; `behind()` of `harness/le.py` has a second copy behind the first copy's panels; entry 1 says that memory is given as it was left | `sys.h`, the system's file, the lines of `start.c` and `view.c` that call the three, `harness/scenarios.py`, `harness/deviations.py`, `harness/le.py`, `DEVIATIONS.md`, the README | F2, F4 to F8, X; the five clusters that count the room, N3, N5, N6, N9 and N11, and with N6 the answers to questions 8 and 5 | The scenarios, `second-copy`, `start-memory`, `command` above all, run once with the room cut to what the 640 KB gave, which is not committed, and then with all of it; `harness/trial32/calls.asm` lists the calls of DOS that it listed; the images, block for block | What the room is on the bench, and what the layer will be asked of the memory, call for call |
| F10, the megabyte goes | `ARENA`, `AT`, `SEG_OF`, `OFFSET_IN_SEG`, `PANEL`, `MAIN_SEG` out of `vc.h`. Three rules out of the test of the layout. `aborts` behind a word. The README and the notes, where they say megabyte | `vc.h`, `harness/layoutcheck.py`, the documents | F9; may be one piece with it | The scenarios; the images; the words' record empty of what rests on the megabyte | |
| N0, the finder, the record, the types. **Done**: "N0, the finder, the record, the types" below | `harness/width.py` of `harness/int16.py`, its record `harness/width.txt` and its test, with the two things that the finder has to learn ("The rule for a number"); `bool` in `vc.h`, and the type with a sign for kind 4; `FileSize`, `ByteCount` and `MemSize` in `sys.h`, defined and not yet used; the fields of the main data and of a panel sorted, once. The rule tried whole on the five files that have no word of the addresses and are handed no panel, `digits.c`, `mask.c`, `line.c`, `mouse.c`, `blank.c`, 738 lines: each number sorted and then widened, made a `bool`, or recorded | `harness/`, `CMakeLists.txt` for the test, those files, their lines of `vc.h`, the types' lines of `sys.h` | The switch's fourth stage, F0; beside F1 to F3 | The scenarios; the images equal; the record holds those five files and has every other as not sorted yet; clang's three warnings of "Booleans" silent | The rule as it stood up, with a line as it was and as it is, in `CLAUDE.md`; how long the record gets; and how many numbers a cluster of one agent holds, by which the session cuts the clusters again |
| N1 to N21, the numbers by file: "The clusters" below | In its files every number of 8 or 16 bits sorted by "The rule for a number" and then widened, made a `bool`, or kept and recorded with its reason, so that a file is read once; each cut that the program counts on written out; a call in another file that counted on a parameter to cut, given its cut | Its files, their lines of `vc.h`, their part of the record | N0, and the piece of the addresses that has its files; beside everything else | The scenarios; the images equal but for what it names; clang's three warnings silent; the test of the record | A number that wraps within a user's reach, with what the original was seen to do, handed back and not mended. For a number that counts memory or a place in a block: how it is known that nothing writes behind the block's end |
| S, what crosses to the system | `FileRead`, `FileReadBare` and `FileWrite` take a `ByteCount` and answer with a structure of their own; `FileSeek` takes and answers a `FileSize`; `DriveSpace`'s three figures are `u32`; the system's file loops where it is asked for more than DOS takes at once. In the callers the variable that takes the answer and the lines that read it, some 40 places in 13 files, and no call's own line, which F4 to F8 change. No caller asks for more than it asked | `sys.h`, the system's file, those lines, a line of the toy in `harness/trial32/` | N0, F3; beside F4 to F8 and the clusters | The scenarios; the images; `harness/trial32/calls.asm` lists the calls of DOS that it listed, each with the count that it had; the toy has read and written more than 64K at once | What the layer will be asked of files, call for call |
| M, the main data's numbers | Each field of the main data or of a panel that the record has as to be widened: a variable beside the structure, its word left unused, and every line that names it | Those lines, in whatever file | Every cluster; alone | The scenarios; the images, the main data equal but for those words | |
| P, a panel as long as its directory. Only if question 3 is answered so | "The questions for the user", 3: a panel's block longer than 64K, its four offsets beside it in 32 bits, `ENTRY` without its cut, and a scenario that fills what the original can hold | `files.c`, `read.c`, `place.c`, `panel.c`, `enter.c`, `start.c`, and the lines of other files that keep an offset in a panel; `harness/scenarios.py`, `DEVIATIONS.md` | M; alone | The scenarios, with the new one held to the original's golden and its patch; the images | |

**The clusters.** Each is the work of one fresh agent that hands back
after some hundred tool calls. The two figures are the lines of the
files and, of those, the lines that have a `u8` or a `u16`, counted by
`grep` after the switch's third stage: a measure, not a count of
numbers. A dagger marks the five that count the room, which F9 waits
for.

| Cluster | Its files | Lines; narrow | Starts when | Besides sorting |
|---|---|---|---|---|
| N1 | `string.c`, `format.c`, `box.c`, `screen.c`, `misc.c` | 891; 133 | F3, N0 | |
| N2 | `dialog.c` | 1,126; 110 | F3 | |
| N3 † | `view.c`, its first part: `struct ViewDat`, `V00_02` and what reads the file and keeps the table of its lines, `Aexec0` | Half of 1,965; 269 | F4 | The viewer's room in bytes. It names what `ViewDat` and `EdData` share, for N6. It says where it cut the file |
| N4 | `view.c`, the rest: what draws and what searches | The other half | N3 | |
| N5 † | `find.c` | 1,044; 125 | F4 | The list's room in bytes, and its counts |
| N6 † | `edit.c`, its data: `struct EdData`, `struct Where`, every line that names a field of them, and `T00_00` where the editor takes its room and reads its file | A third of 2,699; 307 | F5, N3 | The room in bytes, the line in hex. Meets questions 8 and 5, and 9 with them |
| N7, N8 | `edit.c`, the rest, in two halves by its routines; N6 says where | A third each | N6 | |
| N9 † | `copy.c` | 1,571; 147 | F6 | The buffer in bytes |
| N10 | `dirfunc.c` | 1,405; 174 | F6 | |
| N11 † | `files.c`, `read.c`, `place.c` | 1,137; 124 | F7 | The old list's room in `ReDr00`. Meets question 3 |
| N12 | `panel.c`, `info.c`, `drive.c` | 1,020; 108 | F7 | |
| N13 | `tree.c`, `treebox.c`, `extfile.c` | 1,174; 99 | F7 | |
| N14 | `cmdline.c`, `enter.c`, `exec.c` | 1,490; 147 | F7 | Carries out the answer to question 4 |
| N15 | `input.c`, `usermenu.c` | 1,406; 131 | F7 | |
| N16 | `main.c` | 1,601; 117 | F8 | |
| N17 | `keys.c`, `commands.c` | 1,414; 183 | F8 | |
| N18 | `menu.c`, `help.c` | 1,371; 131 | F8 | |
| N19 | `bars.c`, `window.c`, `system.c`, `setup.c`, `start.c` | 1,602; 185 | F8, and F9 for `start.c`, which has the lines that ask for memory | |
| N20 | `stcolor.c`, `stopts.c` | 1,133; 158 | F8 | |
| N21 | `stmain.c`, `sthelp.c` | 1,086; 121 | F8 | |

A cluster and a piece of the addresses never have the same lines: a
cluster starts when the piece that has its files has landed, and of
`vc.h` each has the lines that declare the functions of its own files.
F9 has `sys.h`, the system's file and the lines that ask for memory,
and F10 the words of `vc.h` and the test of the layout: every cluster
but N19 can run beside both. What has to wait, and for what:

- A number that counts a block in paragraphs is widened when the
  block is a pointer's, which is why N3, N5, N6 and N9 stand behind F4
  to F6 and not beside them.
- The room is counted in bytes when it is one function, which F1
  makes; and it is the machine's, F9, when all five that count it have
  landed.
- An offset in a panel waits for question 3, and for P if it is
  lifted. A field of the main data waits for M.
- `struct EdData` waits for N3, which names what it shares with the
  viewer's.

The order:

    at once                      F0   X (done)
    the switch's third stage     F1
    its fourth stage, and F0     N0                      beside F1 to F3
    F1 landed                    F2 > F3                 question 4 asked
    F3 landed                    F4  F5  F6  F7  F8      N1  N2  S
    F4 landed                    N3† > N4    N5†
    F5 and N3 landed             N6† > N7  N8            questions 8, 5, 9 asked
    F6 landed                    N9†  N10
    F7 landed                    N11†  N12  N13  N14  N15    question 3 asked
    F8 landed                    N16  N17  N18  N20  N21
    F2 to F8 and the five †      F9 > F10                N19 behind F9
    every cluster landed         M > P, if question 3 says so

What a piece goes by:

- It lands when every scenario passes, as always. A piece of F or N
  also holds the images of its build against those of the commit that
  it started from, and says in its commit what differs and why.
- The words' record, the host check's and the record of the widths
  are committed with the change that moves them. A word of the
  addresses that is added to `vc.h`, a bridge among them, is named in
  `WORDS` of `harness/int16.py`.
- No piece changes a golden, a patch, a rule, or what the comparison
  leaves out, but F9, as the user's answer of 13 October 2026 covers
  and the table of "What a user would see otherwise" says row for row,
  and a piece that carries out an answer to one of the questions 3, 4,
  5, 8 and 9, as far as the answer goes. Anything else is a deviation
  that was not decided, and the work is handed back.
- A cluster does not lift a limit. Where widening a number would move
  one that the original holds itself to or runs into, the number is
  widened and the limit written as the program's own, at its place,
  until the user has said otherwise.
- A `bool` or a number of 32 bits where a `u16` was changes no caller,
  but one that counted on a cut: "The rule for a number", "Across
  clusters". So the clusters need no order among themselves.
- A piece that makes a size of a block or a place in one wider says
  how it knows that nothing writes behind the block's end: "What could
  go wrong", 14 and 15.
- What a piece learns goes into `CLAUDE.md`, "Notes on the portable
  C", before it is committed.

### What could go wrong

1. A number that has wrapped round, added to a pointer. While `AT`
   and `ENTRY` cut what they are handed, `AT(segment, i - 1)` with `i`
   at 0 is the byte at FFFFh of the segment, as on DOS; written
   `text + (i - 1)` it is the byte in front of the text. Where the
   original counts on the first, the table of lines at the top of the
   viewer's segment, the cut has to be written: `text[(u16)(i - 1)]`.
   The pieces F4 to F6 read every `AT` that they take out for that.
   Shows: in the images, as bytes written where the build before
   wrote none; on a screen, by chance.
2. A pointer into something that moves. From F1 on a panel does not
   move. The room does, when it is taken again after a command or made
   longer after a viewer of another's has run, and from F9 on to
   wherever the extender has it: a pointer into it that is kept across
   `GiveMem`, `TakeMem` or `Aexec0` is stale. Shows: a text or a list
   read from where the room was.
3. Two pointers held against each other. `second < first` of two
   panels and a place held against `HstAdr` are of one block and
   right. A comparison of two things that are of two blocks said
   something on DOS by their offsets and says nothing now. Nobody has
   looked for one.
4. A variable that held a panel under another name, `other`, `top`,
   `behind`: the compiler finds it where it meets a pointer, and not
   where it is only held against another number.
5. The sweeps are larger than an agent holds. F1 is there to say so
   before they are begun; F2 can be cut by family of files, with the
   bridge between.
6. A `bool` that held a third value: see "Booleans".
7. A cut that was counted on and not found: a variable widened where
   the program went by its wrapping. The finder names every store that
   cuts; what it cannot name today is a cut that is made by handing a
   number to a function that takes fewer bits, and a variable that is
   stepped beyond its width, both of which N0 adds to it. Shows: in a
   scenario that reaches the wrap, or not at all.
8. The time of day. Numbers of 32 bits are shorter code for the 386
   than those of 16, so the program takes fewer instructions to a
   step, which is the safe direction for a scenario that shows the
   time.
9. The stack. A variable of 32 bits is as long on the stack as one of
   16 is there already; nothing is expected, and the scenarios' test
   holds half of it free.
10. A read of the main data that the build before made of memory that
    it had not written, and that differs between two builds with no
    fault in either: the images would say so at F0, against a build
    that differs in nothing but a line of code.

11. A number of 16 bits that counts memory and is not in the table of
    "The numbers that count the room": it wraps round on the bench
    from F9 on, where the room is 15 MB. Since every scenario then
    runs with more memory than 16 bits count, such a number shows in
    whichever scenario uses it, the editor's, a copy's, the file
    search's; one that no scenario uses does not show at all. F9's two
    runs, with the room cut and with all of it, tell it from a fault
    of the blocks.
12. A second copy of the program, or a command of the same extender,
    that finds no memory while the first copy holds the room. X tried
    it: it finds none, and its extender ends it with a line of its
    own, while the first holds the largest block; it gets what the
    first has given back (rows 17 to 20 of X). `second-copy` and
    `start-memory` hold it afterwards.
13. The extender's blocks. What 0503h does to a block that cannot
    grow where it is, X tried: it moves the block and what it holds,
    or refuses and leaves it (rows 9 and 10 of X). The room has
    nothing behind it and does not move. That a block given back is
    had again after a command has left a program in the memory was
    not tried: the extender's memory is not DOS's, and `start-memory`
    is the scenario with such a program.
14. A write behind the end of a block, found by X (row 25). The
    extender keeps 16 bytes of its own between two blocks and looks at
    them at every call for memory: a panel or the room written a byte
    too far, which the 640 KB of today take without a sign, ends the
    program at its next `GiveMem` with "DOS/32A run-time (6004):
    extended memory blocks have been corrupted". The original reads
    and copies behind the end of things in places, and where one of
    them writes behind the end of a panel that is 64K whole or behind
    the room, F9 meets it as a scenario that ends so, with the block
    named. What the program is then to do there is not F9's to
    choose: it is handed back and asked.

15. A write behind the end of a block, before F9 can find it. From F9
    on the extender finds it, 14. Before, in the 640 KB, nothing ends
    the program for it, and the image of F0 shows it only as bytes
    that differ in the next block, and only for a piece that is to
    change nothing. What finds it earlier: F1 has the hand-out of the
    640 KB put 16 bytes of a pattern in front of each block and behind
    the last, where the extender will keep 16 of its own, and the
    image's tool holds them at the end of every scenario and names the
    block and the scenario. That is a net from F1 on, under every
    piece of the addresses and every cluster that lands before F9. It
    is the one form of the old `CHECKED` that still fits, and it is in
    the program as it is shipped, not in the program for finding a
    fault: the tests run what is shipped, the debug program is in no
    test of the machine, and a second program through 155 scenarios
    would double them for a check that costs 64 bytes of the room and
    no time. What it does not find: a write further off than 16 bytes,
    which is the image's to show; a read behind the end, which nothing
    shows; and a write that only happens with more room than the 640
    KB give, a number that goes wrong above a megabyte, which the
    extender finds at F9 if a scenario gets there.
16. Widened numbers that no scenario takes above 16 bits. Every
    scenario runs in a room of 15 MB from F9 on, so what counts the
    room is run; what counts a text, a list or a file is not, since
    no text of the bench is longer than a megabyte and no list longer
    than a screen or two. `editor-huge` of question 9 is for the
    editor. For the rest the reason in the record is all there is,
    which is why it is written for a number that is widened as for one
    that is kept. And the room is not zeros to its end: the machine's
    start has left 76 KB written in its last 250 KB (X, row 14), so a
    routine that reads the room before it has written it shows there
    what no room inside the 640 KB showed.
17. Two structures over the same bytes. `EdData` lies over `ViewDat`
    on purpose and a field widened in one moves what both read; and a
    table may name bytes of `TmpBuf` that a structure has for another
    use ("The rule for a number"). Shows: the editor entered from the
    viewer in the wrong window or mode; the images, in `TmpBuf`.
18. The loop of the system's file for more than DOS takes at once is
    run by no scenario, since no caller asks that. S's toy runs it
    once; nothing holds it afterwards.
19. A record of the program's own, question 9, says that the program
    does what it did when the record was read, not that it was right.
    One that is recorded again without being read says nothing. So
    few of them, each with who read it.
20. The clusters are sized by counted lines. N0 says how many numbers
    an agent holds, and the session cuts again by that. A cluster
    that is too large hands back at a commit, with the record saying
    which functions of its files are sorted.
21. A cluster lifts a limit without meaning to: a check that held a
    number under 16 bits by the width of a variable and not by a
    comparison, a list that was full because its count wrapped. That
    is the third rule of "The rule for a number" where the original
    shows something wrong, and a deviation where it shows a limit:
    either way it is handed back, and the test that would say so is
    only the reading.

### Two opinions, as asked

**Whether all of step 2 is worth doing before the layer is designed.**
The plan's opinion was no, for the widths: a machine of 64 bits does
not need them, and the layer names the types. The user took that and
took it back within the hour ("What is decided, and by whom"). The
plan's opinion now, since one is asked for: the user's reasons hold,
and the third most of all. With the room the machine's, a number of
16 bits that counts it is a fault on DOS, today, and not a matter for
some later machine; the cap that the plan had put in its way was a
second limit made to keep the first. What the first opinion had right
is small and is kept: how wide a thing is that crosses to the system
is the layer's to say, so it gets a name now and no more, and sizes
beyond 32 bits wait. What the decision costs is time in front of step
3: some twenty clusters that sort and widen, where the split had seven
that sorted.

**The smallest piece that proves the way.** For the addresses still
F1: three blocks where there was one, inside the 640 KB. It has each
thing that the rest depends on: words that leave the main data with
the layout held, a segment that becomes a pointer with the bridge at
every place that is not yet its turn, the room said in one place, and
the image as its judge, block by block, with the 16 bytes round each.
It does not take the memory from the extender, and cannot: see "The
memory from the extender". If F1 goes badly, what is rethought is the
bridge, the layout and the three blocks, before 259 declarations are
changed. For the numbers it is N0: the rule on five small files that
nothing else touches, with the finder and the record. If the rule
does not stand up there, or the record is longer than a reviewer
reads, that is known before twenty agents go by it.

### The questions for the user

**Answered, 13 October 2026** (`CLAUDE.md`, step 2 of the road):

1. **The order.** The widening is of this step and does not wait for
   the layer: the pointers first, then the booleans and the widths
   together, with the reason written down for every number that stays
   16 bits and a named type for what crosses to the system.
2. **The memory for the panels and the text.** "From the extender
   now", as much as the machine has, and what a user then sees
   otherwise is declared: the editor opens whatever fits.

**Taken by the session, 13 October 2026**, as recommended, neither
changing what the program does:

6. **The flags of the setup file** stay bytes in the program too,
   tested as the original tests them. Every other true-or-false number
   becomes a boolean.
7. **The main data's layout** is kept as the original's through step
   2, with the pointers and the widened numbers beside it.

**Asked as the work comes to them, and not before.** Each is a limit
of the original that would go, so a deviation, and the user decides
them one at a time (`CLAUDE.md`, step 2 of the road). The piece that
meets one first hands back with what it saw, the session asks, and the
piece named carries the answer out. They are here in the order in
which the work comes to them; each is short enough for a dialog, the
recommendation first.

4. **The 256 files kept while a command runs.**
   - *When*: F1 has landed. From then on a panel keeps its block
     through a command, and nothing needs the cut. N14, which has
     `enter.c`, carries the answer out.
   - *The original*: gives its panels back to DOS for a command and
     keeps 256 files of each. Of a longer list the rest is dropped,
     and when the list is read again behind the command, the files
     beyond the 256th are no longer selected (`F21_40` of `enter.c`;
     the setup cuts alike, `StSetup`).
   - *Recommended*: gone. The list is kept whole, and what was
     selected stays selected. Declared as the removal of what only
     made sense on DOS, memory given back for a command.
   - *Or*: kept, 256 as a number of the program's own.
   - *The test*: `enter-many` as it is, held to the original's golden
     with a patch for what stays selected. Nothing new.

8. **The editor's room.** Answered in what matters: "the editor opens
   whatever fits". What is left is what it shows where a number no
   longer fits its cells.
   - *When*: N6, the editor's data, before F9.
   - *The original*: has room for 345,840 bytes on the bench and
     refuses a file that is not shorter, "The file b is too large for
     Edit.", with View and Cancel. With its memory full it takes no
     more.
   - *What it takes*: the room counted in bytes (N6: `EdSpace`, the
     line in hex, and the test for "more than there is" in the count
     of free bytes, which is at 20 bits today), and F9 with no cap.
   - *What is then shown at the limit, the machine's memory*: the same
     box, for a file that is not shorter than the room, some 15 MB on
     the bench; the editor with its memory full, as the original's is;
     and on a machine with too little memory, what row j of "What a
     user would see otherwise" has.
   - *Asked*: the status line has seven cells for the free bytes,
     "345,840", and from a million on the number does not fit them.
     *Recommended*: in K from a million on, "15,009K". *Or*: the field
     three cells wider to the left, if N6 finds them free. *Or*: no
     figure from a million on. N6 says with the question what the
     routine shows if nothing is done, the last seven characters of
     the number as far as was read, and every other place where the
     editor shows a number that a text beyond a megabyte makes longer,
     the place in hex above all. The figure is left out of the
     comparison already; the pattern that leaves it out
     (`harness/deviations.py`, `IGNORED`) follows its new form.
   - *The test*: the refusal, against the original's golden, by a file
     that is too large for both. That needs care at F9: `noon.dat` of
     the bench says a megabyte in its entry and has no bytes, which is
     too large today and fits 15 MB, so `view-window` and step 38 of
     `editor-big` would open it. F9 tries first that a scenario may
     say how much memory its machine has, a field of `Scenario` handed
     to QEMU's `-m`, 16 if nothing is said, and gives those two a
     machine in which a megabyte does not fit: their goldens derived
     again have to come out as they are, since the original looks at
     nothing above the megabyte. If one changes, that is handed back.
     The other way is an entry on the bench that says more than 16 MB,
     which changes the goldens that show `C:\STAMPS` and is a change
     of the bench, asked first. On a machine small enough that the
     room is under 381,338 bytes, `editor-big` may pass again as the
     original does: F9 tries that too. What lies beyond the original's
     limit is question 9.

5. **The editor beyond 65,535 lines or columns.**
   - *When*: N6, with 8. Within reach in the original: a text of
     empty lines has that many in 131,072 bytes, and the original has
     room for 345,840.
   - *The original*: counts its lines in 16 bits (`EdLine`, `EdLin0`,
     the line of `struct Where`) and writes the number at the left of
     five cells. What it does beyond is not known. N6 finds out on the
     bench before anything is asked, with the original under `harness/show.py`: a file of
     short lines made by doubling with Shift-F5, as `editor-big` makes
     its file; the number shown, where the cursor's keys and a search
     go, and whether the text that is saved is the text. For the
     columns it reads what the line's buffer does with a line of more
     than 65,535 bytes.
   - *Recommended*: counted in 32 bits, so that the number is right
     and the place is kept. From 100,000 lines on the number has more
     than its five cells: asked with the count of free bytes, in the
     same way.
   - *Or*: as the original. The variable is wide all the same, since
     the line in hex has to be, and the cut is written where the line
     in text is stepped, kind 3 with "a bug of the original, kept".
   - If the original harms the text there, that is a bug whose fix is
     asked by itself, with what was seen.
   - *The test*: within the original's reach, so a scenario of both
     with the original's golden, and the fix as a patch on the rows of
     the status line. Nothing new.

9. **A scenario whose expectation is the portable C's own.**
   - *When*: with 8.
   - *What*: beyond the original's limit there is no golden of the
     original to hold the program to: a text longer than the
     original's room, with its lines in hex beyond FFFFh and a place
     beyond 20 bits; a copy in one piece of 15 MB; a list of the file
     search beyond 65,535 lines. `CLAUDE.md` has the portable C judged
     by goldens recorded from the original, so such a scenario is a
     new kind of thing and wants the user's word.
   - *Recommended*: one, now, for the editor: `editor-huge`, a text of
     some 1.4 MB made as `editor-big` makes its file, the end of it,
     hex, a byte typed far in, saved, the file held by its length and
     the sum of its contents. Arithmetic of widened numbers that no
     test reaches is where C1's fault was. And no other until the
     implementation of the layer for tests ("An implementation of the
     layer for tests: future work"), which can tell the program that
     it has the original's room to the byte: `editor-big` and
     `editor-full` are then held to the original's goldens again, and
     a limit is tested where it is and not where the bench's memory
     puts it. What stays untested until then is listed, as "The
     editor's room: untested in the program of 32 bits" lists.
   - *Or*: none; all of it listed as untested.
   - *The least that the harness needs*: a scenario says whose it is,
     `own="portable_c"`, and is not derived from the original nor run
     for the C translation. `python3 harness/goldens.py derive --tree
     portable_c NAME` records screens and disks from the build into
     `harness/goldens/portable_c/NAME.own`, apart from the goldens of
     the original, with a header as a patch has: the entry of
     `DEVIATIONS.md`, why the original has no golden for it, and that
     it was read; a header not filled in fails the test. The test
     holds the build to it as to a golden, after what the comparison
     leaves out and without the tree's rules, and a screen that then
     differs is a failure to look at, not to record again. It has no
     list of what ran, and counts for nothing in what
     `harness/ported.py` holds to have run. `python3 harness/goldens.py
     changes --tree portable_c` names it. What such a record is worth:
     that the program does tomorrow what it did when it was read. That
     it was right then, only the reading says, once.

3. **How many files a panel holds.**
   - *When*: N11, which has `files.c`, `read.c` and `place.c`, before
     it sorts the offsets in a panel: the answer is the reason that
     the record gives for them.
   - *The original*: reads a directory until 64K are full, 2,704
     files, beeps, and shows those.
   - *Recommended*: lifted, in a piece of its own at the end of the
     step, P: a panel's block as long as holds the 65,535 entries that
     a directory of DOS can have, some 1.6 MB, or as the memory goes;
     `CurAdr`, `First`, `EndBuf` and `EndFul` beside the panel in 32
     bits, their words left unused; `ENTRY` without its cut. It is the
     limit of 16 bits that a user of a file manager meets first, and
     nothing that comes later makes lifting it cheaper. It is as broad
     as F2.
   - *Or*: kept, as a number of the program's own, and looked at again
     when the layer is designed.
   - Either way N11 leaves the offsets 16 bits and records which.
   - *The test*: within the original's reach, the beep and the 2,704,
     so a scenario of both with the original's golden and the lift as
     a patch. It needs a directory of more files than the bench has
     anywhere: made in the scenario by a command of DOS from the
     program's command line, which P tries first, or put on the
     bench's disk where no other scenario lists it. A change of the
     bench that changes a golden is asked first.

Everything else that a user would see otherwise by the memory is
covered by answer 2, as its table has it row for row. Two rows there
said "if X finds it so", m and n, and X found it so: a command is
told of the memory what it is told today, and a second copy runs in
what the first gave back. What X found besides that a user could
meet is in row j: a machine with too little memory for the program
itself is told so by the extender, as today, and only one with room
for the program and not for its panels by the program. And a cluster
may find a number that wraps within a user's reach where nobody has
looked: it hands that back like these, with what the original was
seen to do.

### Left for the layer's design

- **Sizes beyond 32 bits**: of a file, of the files that are selected
  taken together, of the room that a drive has free. They want numbers
  of 64 bits, for which the tree has no helper routines, having no C
  library (the compiler calls `__U8M` and `__U8D` as it called `__U4M`
  in 16 bits), and room for more than ten digits on a panel. The named
  types of "What crosses to the system" are where the layer says so.
- **The shape of `sys.h`** otherwise: `failed` and what else answers
  yes or no, a handle, DOS's forms of a time, a date and attributes.
- **Whether the main data's layout is freed**: "The way taken", 1.
- **The proper test of a limit**, with the system's answer made for
  the test: "An implementation of the layer for tests: future work".

### Where this plan guesses

- The sizes of the pieces, all of them. No piece was begun, and the
  sweeps were sized by counting lines, not by trying.
- That the bridge is exact. It is by the definitions; nothing ran.
- That the image is cheap and steady from run to run: `pmemsave` is in
  the harness, the tool is not written, and 10 of "What could go
  wrong" is unknown.
- That places inside a panel are best left offsets. If the user lifts
  the limit of question 3 they become numbers of 32 bits beside the
  panel, which is P, as broad as F2.
- The count of places for the widths, 600 to 1,000, and that most are
  plain.
- The sizes of the clusters, by counted lines: no cluster was begun,
  and N0 is there to measure one.
- What the editor does beyond 65,535 lines, and whether any other
  number wraps within a user's reach: only the reading of N1 to N21
  finds those, and for the editor N6 on the bench.
- That nothing of 64 bits is within a user's reach but sizes: not
  looked for. The original adds the sizes of the files that are
  selected in 32 bits, and FreeDOS has disks on which they can pass 4
  GB; whichever cluster reads that sum reports what the original
  shows.
- That no table names a byte of `EdData` or `ViewDat` while the
  editor or the viewer has it: `TmpBuf[100]` is named fifteen times,
  and for what was not read.
- That the goldens of `view-window` come out as they are on a machine
  with less memory, and that QEMU takes a memory in which the room is
  under 381,338 bytes, for `editor-big`. X has another way to a
  machine with little memory, `second.c`, which keeps all but so much
  and runs the program in the rest.
- That the status line of the editor has free cells to the left of
  its count: read from two calls, not from a screen.
- That 16 bytes round each block of the 640 KB change nothing that a
  scenario shows.
- That no caller of `FileRead` or `FileWrite` gains by asking for more
  than DOS takes at once.
- That no scenario fills a panel, read from the scenarios' names.
- What `harness/reads.py` needs for the new form of a table handed on:
  its patterns were looked at, not changed.
- What the plan says of DPMI 0500h to 0503h under DOS/32A was a guess
  and is none any more: X tried it, with what it overturned at the
  top of its section. Still guessed there: the program as the first
  of two copies, which only F9 can show; the figures of the program
  once its 640 KB are gone, worked out from today's objects; and any
  host of DPMI but the pinned extender.
- That 1000h paragraphs for a panel from the start change nothing
  that a scenario shows, and that the room of some 511 KB in the 640
  KB after F1 does not either.
- The figures of the file search's list, worked out from the room and
  not seen.

### X, the extender's memory on the bench: done, and what F9 goes by

The trial of 13 October 2026, with two toys of `harness/trial32/`,
`blocks.c` and `second.c`, on the bench as it is: QEMU, FreeDOS, 16
MB, the pinned DOS/32A, which answers 090Ch for its version, as the
stub of each toy, and no XMS driver unless a row says so. Nothing of
the program changed. Each row has the run that showed it; the steps
are those of `harness/trial32/run.py`, and `blocks` and `second` stand
for `build/trial32/blocks-d32.exe` and `build/trial32/second-d32.exe`.

**What it overturns, or adds, of the plan as it stood:**

1. *The program does not start in some 140 KB.* The extender loads
   the program's two objects into the same memory that 0501h hands
   out, and refuses in its own words when they do not fit. 140 KB is
   what `Init11` will ask for behind that. With the 640 KB gone the
   objects are some 237 KB (892,606 bytes today, less 655,360), so
   the program wants some 376 KB above the megabyte, and a machine
   with less is told so by two hands: below the 237 KB by the
   extender, "DOS/32A fatal (4003): not enough extended memory to load
   application exec", and the program ends with FFh; between that and
   376 KB by the program, with the original's words. Row j is written
   anew.
2. *A write behind the end of a block ends the program.* The extender
   keeps 16 bytes in front of every block, in the memory itself, and
   looks at all of them at every call of 0500h to 0503h. With those of
   one block overwritten, the next such call does not come back: "DOS/32A
   run-time (6004): extended memory blocks have been corrupted
   (#address)", and the program is at DOS's prompt. In the 640 KB of
   today such a write lands in the next block or in nothing and is
   not noticed. 14 of "What could go wrong".
3. *0503h refused says nothing of how long the block could be*, where
   DOS's 4Ah answers that in BX and `Aexec0` of `view.c` asks for
   0FFFFh paragraphs to learn it. What F9 does in its place is below.
4. *A block is given with what was in the memory*, not zeros: what the
   program had there before it gave the block back, what a second
   copy left, what the program before left. That is what the original
   has of DOS, and the 640 KB of today have not (item 12 of "The
   program of 32 bits: what differs from the original"): toward the
   original, and a thing that F9 declares away from entry 1, not into
   it.
5. *The second copy is elsewhere*, which the driver of the tests has
   to be told: see the last row.

Everything else that the plan says of the extender's memory stood up.
Rows m and n are as the plan expected, and say so now.

| | What | How it was seen |
|---|---|---|
| 1 | 0500h says how much the largest block is that 0501h gives, to the byte: that many bytes are given, one more is refused with the carry and 8013h in AX. It is all that there is in one piece: FE0000h, where the BIOS's 15,232K end, less where the program's objects end, less 16. 15,574,416 bytes for `blocks`, 15,577,232 for `second`; for the program some 15.36 million once the 640 KB are gone | `blocks f l`: "0500h says 00EDA590", "the largest that 0501h gives is 00EDA590 bytes; one more is refused", "taken: at 00105A70" |
| 2 | 0500h's other numbers: EDFh pages of memory, EDAh free and as many not locked and locked; no paging file, FFFFFFFFh. There is no paging: an address is the memory's own | `blocks f` |
| 3 | With an XMS driver, FreeDOS's HimemX, everything below is the same, 162,816 bytes further up and with as many fewer: 15,411,600 for `blocks` | `run.py --himem`, `blocks f l r g`, `second m t m a c s c b c` |
| 4 | With the largest block taken nothing is left: 0500h says 0, and a block of one byte is refused, 8013h. DOS's memory is not touched by any of it: its largest block is 6CF9h paragraphs before and meanwhile | `blocks l` |
| 5 | A block is at an address that is a multiple of 16, and its length is taken up to a multiple of 16. In front of it are 16 bytes of the extender's: 12345678h, the length with the highest bit set while it is taken, 0, 12345678h. The name that 0501h answers in SI:DI is the address of those, the block's address less 16, in every block seen. Blocks follow one another with nothing but those 16 bytes between: 1 byte asked for at 00105A70, 17 at 00105A90, 4,096 at 00105AC0, 4,097 at 00106AD0, 65,536 at 00107AF0. Nothing is on a page | `blocks a`, and `w` for the 16 bytes |
| 6 | A block is taken at the lowest place that holds it. One that was given back is given again at the same place, if nothing lower has become free meanwhile | `blocks l`: "taken again: at 00105A70"; `g`: with 1 MB at 00105A70 given back, 512 KB are taken there, 512 KB less 16 bytes right behind them, and 1 MB behind everything |
| 7 | What 0500h says is the largest free piece, not the sum: four blocks given back in front of a fifth leave it as it was, and the fifth makes it whole again | `blocks a` |
| 8 | 0503h with nothing behind the block: longer and shorter in its place, the name the same, what it held kept. It grows to its length and what 0500h says and 16 bytes, and not a byte beyond | `blocks r`: 1 MB at 00105A70 made 2 MB and 512 KB, there; `x`: "made as long as what is free, 16 bytes more: at 00115A80 ... free 00000000", and with 17 refused |
| 9 | 0503h with a block behind it and not room enough before that: the block moves to the lowest place that holds the new length, with another name, and holds there what it held; where it was is free. Made shorter it stays | `blocks r`: 512 KB at 00105A70 with 4 KB behind them, made 1 MB, are at 00186A90, their 512 KB still 5Ah |
| 10 | 0503h refused, for more than there is: the carry and 8013h, and the block is where it was, as long as it was, with what it held. Nothing says how long it could have been | `blocks r`: "made a megabyte longer than there is: refused, AX 8013", and its 256 KB are still 5Ah; `x` |
| 11 | Made shorter, a block has 16 bytes of the extender's written right behind its new end, for the free piece that follows. Made longer again it has those where it had its own. DOS does the same to a block of its own, so the original has that too | `blocks x`: 1 MB of 5Ah made 8 KB, given back, and taken with all that is free: "of 00100000 bytes 00000010 are not 5A, from 00117A80 to 00117A8F: 12345678" |
| 12 | A block is given with what the memory held. One that was written, given back and taken again has what was written; one that was moved leaves its bytes where it was | `blocks g`: "512 KB: at 00105A70 ... of 00080000 bytes 00000000 are not C3", the C3h that the megabyte given back had been filled with |
| 13 | So has a program that is started later, of what an earlier one wrote: the largest block filled with A5h by a toy that then ended is A5h to the next toy, every byte | `blocks d q '"d:\vc\vc" Enter' z`, one machine: "of 00EDA590 bytes 00EDA590 are not 00, from 00105A70 to 00FDFFFF: A5A5A5A5" |
| 14 | On a machine that has just started, the largest block is zeros but for 76,425 bytes between 00FA2F10h and 00FBFFFBh, in the last 250 KB of it: what the machine's start left there. A room of a megabyte right behind the program is zeros at every scenario's start, as the 640 KB are; a room of all that there is would not be | `blocks z` as the first step: "of 00EDA590 bytes 00012A89 are not 00, from 00FA2F10 to 00FBFFFB". Not behind `l`, which leaves the extender's 16 bytes wherever a block of its halving ended |
| 15 | What a file does not hold of a program's objects the extender clears as it loads the program: `zero.c`, run on the machine where the toy before had filled that memory with A5h, finds its 200,000 bytes all 0. The trial of step 1 had left that open | `--extra VC/Z.EXE=build/trial32/zero-d32.exe`, `blocks d q '"d:\vc\z" Enter'` |
| 16 | DOS is asked nothing by 0500h, 0501h, 0502h or 0503h. Of the calls that DOS gets while the toy runs, as `calls.asm` writes them down, none lies between the two with 30h that the toy makes around each key's work, but the one 48h that the toy asks itself, for DOS's largest block | `run.py --name VC.COM --extra VC.EXE=build/trial32/blocks-d32.exe --keep build/trial32/calls build/trial32/calls.com f l a r x g z h q`, then `listed.py build/trial32/calls.D.img`: 51 calls, the pairs `3000` with B0xxh and E0xxh in BX one behind the other |
| 17 | A program of the same extender that is run while the first has the largest block: its extender finds no memory, writes "DOS/32A fatal (4003): not enough extended memory to load application exec" with the file's name, and ends it with FFh. 4Bh itself comes back without the carry. So the room has to be given back, or made short, before anything of the extender's kind is run, as the plan has it | `second t c`: "came back: carry 0 AX 4B00, ended with 00FF; free 00000000" |
| 18 | What the first gives back the second gets, whichever way it was made free. With the block made 68 KB shorter by 0503h, a copy of the toy is loaded into that, at 00FCF43Ch, and is told that 48 KB are free, which are the rest of it. With the block given back by 0502h, the copy is right behind the first toy and is told 15,192 KB, all but itself | `second t c 1 c s c b c` |
| 19 | When the second ends, the first has free what it had, whether the second gave its blocks back or not: the copy takes the largest block and ends with it | The same run: "free 00010FF0" in front of the copy and behind it |
| 20 | The program of the portable C as the second, in what a toy leaves it: with 888,816 bytes free the extender refuses it as in row 17, with 892,912 it runs, to its panels and to F10. Its objects are 892,606 bytes: it wants those, and at most 306 more | `--extra VC.EXE=build/portable_c/VC.EXE`, `second t`, 13 of `s`, 9 of `1`, `p F10 Enter 1 p F10 Enter` |
| 21 | What a command is told of the memory is the same whether the program that runs it holds the largest block, has given it back, or has taken nothing. `mem`: `Extended (XMS) 15,360K 15,360K 0K`, `Total memory 16,384K`, which are the two rows of `command`'s patch as it is. `above.asm`: 88h answers 0, E801h and E820h are refused, the clock's memory has 3C00h, no XMS, no DPMI: the table of C2. With an XMS driver, 15,232K, all of it used, either way | `--extra VC/ABOVE.COM=build/trial32/above.com`, `second m a t m a b t Shift+s m a`; the same with `--himem` |
| 22 | When a program ends, by itself, with blocks taken, the next has all: 0500h says to a toy started behind one that kept the largest block what it said to that one | `blocks h q '"d:\vc\vc" Enter' f`, one machine |
| 23 | When DOS ends a program by another's hand with a block taken, the same: the toy as `T.EXE` keeps the largest block and idles, `ended.asm` has DOS end it, and `memory.c` run from DOS's prompt is told 00EDB650h, what it is told on a machine that has run nothing | `run.py --name VC.COM --extra VC/T.EXE=build/trial32/second-d32.exe --extra VC/M.EXE=build/trial32/memory-d32.exe build/trial32/ended.com '"d:\vc\m" Enter'` |
| 24 | And the same of a second program inside a first: the first has given 64 KB back, `ended.asm` is run from it and runs `T.EXE`, which takes what there is, and DOS ends that. The first has its 64 KB free again, and a copy run next is where `T.EXE` was | `--extra VC/ENDED.COM=build/trial32/ended.com --extra VC/T.EXE=build/trial32/second-d32.exe`, `second t s e f c` |
| 25 | A write of 16 bytes behind the end of a block, where the next block's 16 bytes are: the next call, which was 0500h, does not come back. "DOS/32A run-time (6004): extended memory blocks have been corrupted (#001068B0)", the number being the name of the block whose bytes they were, and DOS's prompt | `blocks w` |
| 26 | 0502h for a block that was given back already is not refused: no carry | `blocks r`, its last line |
| 27 | With two blocks of 64 KB kept by the first, as the two panels will be while a command runs, and the largest block behind them given back, the second is loaded 20020h further up than with nothing kept: twice a panel and its 16 bytes | `second c k t b c`: the copy's code at 0010539C and then at 001253BC |

**Rows m and n.** m stands: row 21. n stands: rows 17 to 20 and 24,
with toys for the first copy and the program itself for the second.
What no toy can show is the program as the first copy; `second-copy`
and `start-memory` hold that when F9 has made it.

**For question 8**, as it stood when this was tried: a guard of the
room, a megabyte and no more until the widths were widened. The plan
has no such guard any more ("The numbers that count the room"), so
what is said here of the room without one is what F9 lands with.
Nothing here spoke against the guard or for it by itself. Under it
the room is a megabyte of the bench's 15, so 14 are
free behind it at all times; without it, as with a machine of little
memory, the room is all that there is and nothing of the extender's
kind runs until it is given back (row 17), which the program does
around every command anyway. And a room of all that there is has in
its last 250 KB what the machine's start left there (row 14), where a
room of a megabyte is zeros at a scenario's start: of no weight unless
a scenario reads the room that far before it has written it.

**What was not tried.** A block taken again after a command has left
a program of DOS's in the memory (13 of "What could go wrong"): the
extender's memory is not DOS's and DOS is asked nothing (rows 4 and
16), so nothing was expected and nothing was run; `start-memory` is
the scenario that has such a program. A block that can grow neither
where it is nor anywhere in one piece, with free pieces on both sides
of it. Memory with holes in it, as a machine with more than 16 MB may
have. Another version of DOS/32A than the pinned one, and any other
host of DPMI: the 16 bytes, the names and the message of row 25 are
this extender's, and nothing of DPMI promises them.

**What F9 goes by:**

- *How large a block can be* is the first number of 0500h, and 0501h
  gives exactly that. Then nothing is left at all (row 4): no later
  call of 0501h can be met while the room is the largest block, by
  the program or by a second one.
- *The order makes the room the last block*: the two panels first,
  then the room. A block with nothing behind it is made shorter and
  longer where it is (row 8), and given back and taken again it is
  where it was (row 6). So no pointer into the room goes stale on the
  bench by `GiveMem`, `TakeMem` or `Aexec0`. That is this extender's
  way and not DPMI's promise: 0503h answers with an address, and the
  code takes that address, whatever it is.
- *As long as it goes.* `Aexec0` and `TakeMem` learn the most that a
  block can be by asking DOS for too much. 0503h does not answer that
  (row 10). Two ways that were both seen to work: make the room
  as long as it is and what 0500h says and 16 bytes (row 8), which
  rests on the 16 bytes and on the free piece behind the room being
  the largest; or give the room back and take what 0500h then says,
  which rests on neither, lands where the room was (row 6) and leaves
  what it held but the 16 bytes of row 11. The second is the one to
  write. Either way the function of `sys.h` that makes a block longer
  can no longer answer "the largest" when it is refused, unless the
  system's file works it out by the first way: that is the one place
  where F9 changes what the three functions are asked, and not only
  their unit.
- *The name of a block* is kept with its address, in the system's
  file, for each of the three blocks. That it is the address less 16
  here (row 5) is not to be counted on.
- *Around a command* the room is given back whole, or made as short
  as the least of the viewer for another's viewer, and whatever is
  then run of the extender's kind lives in it (rows 17 to 19). A
  second copy of the program wants the first to have free, behind its
  two panels, the second's objects, its two panels and the least of
  its viewer: some 376 KB with the figures of today.
- *The end.* The extender takes back what a program holds when the
  program ends, by itself or by DOS's hand (rows 22 to 24). Nothing
  has to be given back for the memory's sake when the program leaves.
- *What is in a block* is what was there (rows 12 to 14). On the
  bench, at a scenario's start, that is zeros in the megabyte behind
  the program, as today; behind a command of the program it is what
  the room held before, as today; behind a second copy it is what the
  copy left, where today the first copy's 640 KB are untouched by the
  second. The images of F0 will differ there, in the room and not in
  the main data.
- *The 16 bytes.* A write behind the end of a panel or of the room,
  or in front of one, ends the program at its next call for memory
  (row 25). The scenarios then hold every scenario to it, which is a
  net that the 640 KB never were; and a scenario that ends in
  "run-time (6004)" names the block. A read behind the end reads the
  16 bytes and then the next block, where in the 640 KB it read the
  next block at once: 14 of "What could go wrong".
- *The driver of the tests* knows the second copy's place by
  `behind()` of `harness/le.py`, which has it right behind the first
  copy's objects. With two panels kept between, of 10000h bytes each,
  it is 20020h further (row 27). `second-copy`, `start-memory`,
  `comspec`, `ended`, `start-setup` and `start-screen` start such a
  copy.
- *A machine with little memory* can be had on the bench as it is,
  for the words of `AlocEr` and for row j: `second.c` keeps all but as
  much as its keys say and runs the program in the rest, `second t`,
  some `Shift+s`, `s` and `1`, then `p` (row 20).
- *DOS is asked nothing* (row 16): entry 6 of `DEVIATIONS.md` loses
  nothing and gains nothing by it, and `calls.asm` with `listed.py`
  holds F9 to that.

### F0, the net and the record: done, and what a later piece should know

Two tools of the harness, and no line of the program: `VC.EXE` is the
same bytes before and after.

**The two commands of the image**, for a piece that is to change
nothing that the program does:

    python3 harness/dataimage.py save build/portable_c/VC.EXE build/before
    python3 harness/dataimage.py compare build/before build/portable_c/images

The first, with the build that the piece starts from, before anything
is changed: it takes the program through every scenario and keeps what
it has in its memory at the end of each, holding it to no golden; two
to three and a half minutes by the load. The second, when the
piece's own build has been through `portable_c-scenarios`, which writes
the same images of the build that it judges into `images` next to the
program. In place of the first, `cp -r build/portable_c/images
build/before` does, if the test has just run on the build that the
piece starts from; and where "before" has to be made again later, of
the commit that the piece now stands on, `save` takes any `VC.EXE` with
its `VC.MAP` next to it, in a directory called `portable_c`, as a copy
of the tree that is built beside has it. The images are of a build, 3.7
MB for all scenarios, and are under `build/`, never committed. The top
of `harness/dataimage.py` says what an image is and how it is read.

**When it is taken.** The driver of the tests resets the machine when
it has written the last step's screen, which it does when the program
waits for a key with nothing left to do. QEMU is told, by its
`set-action`, to pause a machine that would end, and the memory is read
there with `pmemsave` (`linger` and `memory` of `harness/dos.py`): the
same instruction in every run. Nothing of the driver or of the bench
changed for it, and every golden derived again leaves
`harness/goldens/` as it was. That the machine has come to its end QEMU
says by itself, and the harness listens: asking QEMU a hundred times a
second, which was the first try, took 25 seconds of a run of 100.

**What is steady**, tried with all 155 scenarios:

| Held against each other | What differs |
|---|---|
| The same build, three times: twice by the scenarios' test, with QEMU's log of what ran, on a machine that did little else and on one that did more, and once by `save`, without the log, with ten other machines running | Nothing, in any scenario: not a byte of the program's data, the stack included (9 scenarios were held whole, the rest as the tool holds them) |
| The same build with the driver writing the program's stack out at every step, as the test has it, and without | 20 scenarios, in `Rand1` and `Rand2`, `Stars`, `MousTim`, the time of a file in the `DTA` and in a panel's list. The program is the same, but it gets to a step some instructions later, and these follow the time. `save` takes the scenarios as the test does for that reason, and `save --early` as it does not: the way to see whether a thing follows the time, with one build |
| A build whose data the linker has laid out otherwise, two variables added and nothing else: the main data 40 bytes on, the 640 KB 64 | `OwnFirst` of `dos32.c` in every scenario, a paragraph on; the seven words of the main data that hold a segment; two more segments that the program had left in its buffers, `TmpBuf+128` after a copy and `MenuBuf+48` after a dialog; and one pointer of a variable, in every scenario. No time, since no instruction is another |
| A build that takes 20 instructions more for every round of its wait for a key | 9 scenarios, in the same fields as the second row but `MousTim` |

So the image is steady from run to run and under load, which answers
10 of "What could go wrong": no read of memory that was never written
shows, in any scenario. And two builds differ, with no fault in either,
in what follows the time and in what holds an address, and in nothing
else that was seen.

**What is left out, and how.** All of it is in `harness/dataimage.py`,
each with its reason:

- `ADDRESSES`: the seven words of the main data that hold a segment or
  a count of paragraphs, `WCB1seg`, `WCB2seg`, `WCBcrn`, `WCBsec`,
  `ViewSeg`, `MemSeg`, `FreMem2`. Not left out but held by what they
  name: the block and how far into it. A word that names another place
  in one build than in the other is a difference.
- A word of the main data that differs and is a segment of the same
  place of the same block in both builds, and a variable of four bytes
  or a multiple whose words differ and name the same thing by the two
  maps: counted in the last line of the summary, and no difference.
  That is cheap, and it is all that is known of a pointer: where there
  is one is not known, only that two numbers that differ name one
  thing. A pointer in the main data or in a block is not looked for;
  the plan has none there.
- `UNSTEADY`, 140 bytes of the main data: `Rand1`, `Rand2`, `Stars`,
  `MousTim`, and the time and date in the `DTA`. `UNSTEADY_VARIABLES`:
  `OwnFirst`. `UNSTEADY_ENTRY`: `TimeF` of an entry of a panel's list,
  and the same bytes where a copy of the list lies behind the panels.
  These are compared, printed with `--all`, counted as "may differ",
  and fail nothing.
- The stack, 16 KB: return addresses, another thing in every build.
- A block that is free, and the paragraph that `dos32.c` keeps in front
  of each block: the lengths of the blocks and which are taken are held
  in their place.
- What is not the program's data at all: the registers, the screen, the
  extender's memory and DOS's, where `dos32.c` has its block of real
  mode.

What is then held, in a scenario: the main data, 62,603 bytes but for
those 140 and the 14 of the addresses; the blocks that are taken, all
655,360 bytes in most scenarios, the panels and whatever is behind
them, written or not; the 937 bytes of the variables that the map names
outside the main data, by the file and the name of each; and the 25,080
bytes of the tables, which are held against the program's own file as
the extender loads it, when the image is taken, so a write into a table
shows with no second build.

`python3 harness/dataimage.py check DIRECTORY` holds a directory of
images against itself and then against copies with one byte changed, in
the main data, in a panel's structure, in an entry of its list, behind
the panels, at the end of the block, in a variable and in a word that
holds a segment: each has to be named, and is, as "the main data,
TmpBuf+3" or "the panel of WCB1seg, entry 2 of its list, AttrF".

**How a piece says what it means to change.**

- `--changed NAME`, once for each: a field of the main data, a
  structure that lies over one, `EdData`, `ViewDat`, `FndData` or
  `HlpData`, a variable as `file.Name`, a thing in a block as `"the
  panel of WCB1seg"` or `"behind the panels"`, or `"the blocks"` for
  how many blocks there are and how long each is. What differs
  there is printed with `[declared]` and fails nothing. The commit says
  the same. A piece that widens a field of a structure laid over a
  buffer names the structure so: the fields behind the one that grew
  are then at other places, and the tool, which names a byte by the
  original's listing, says the old names for them.
- A word that leaves the main data for a pointer beside it, as F1's
  three and F2's four: its line of `ADDRESSES` gets `variable=("main",
  "_Name")`, the file and the symbol as the linker's map has them. A
  build that has the variable is read there, as a pointer, and one that
  has not is read in the field, so the build before and the build after
  are held against each other by what the word names. The field's two
  bytes are not compared in either. That edit is committed with the
  piece.
- A thing that differs and follows the time: `save --early` of the
  piece's own build into a third directory, and `compare` of that with
  the piece's images. If it differs there too, it follows the time and
  gets a line of `UNSTEADY` with its reason, which is a change to the
  net and is said in the commit; if it does not, the piece has changed
  what the program writes.

**Where the blocks are, now and when the extender has them.** The tool
knows no address but that of the program's data on the bench, which the
driver of the tests goes by too (`le.BENCH`): the main data is at `_md`
of the map, and the blocks are read from what `dos32.c` keeps in front
of each inside `Own`, which the map names. What is compared of the
blocks is what the program has in them, by name: "the panel of
WCB1seg", "the panel of WCB2seg" and "behind the panels", each from
where its word of `ADDRESSES` says to the next of them or to the end
of its block. So they are the same things whether the program cuts
them out of one block, as today, or has a block for each, as from F1
on, wherever the blocks lie; and an address is held by which of them
it is in and how far. Where two builds have one of them of two
lengths, as F1's will have each panel and the room, it is held as far
as both have it and the two lengths are said, which `--changed "the
blocks"` declares, with how many blocks there are and how long. What
lies behind the end of a panel's list in the build that has the panel
longer is then held against what the other build happens to have
there: F1 reads those, if there are any.

With the memory from the extender, F9, a block lies wherever the
extender put it, outside the program's data. The image has room for that: it is
a list of stretches, each with a name, an address and a length, and
`_handed` of `harness/dataimage.py` is where the list is made, empty
today. What it needs is a line of `dos32.c`, which was not F0's to
make: **a table of the blocks that the program holds, each with its
address and its length, as a variable that the linker's map names**,
and the count of them. `dos32.c` will have such a table in any case,
for the handle that the extender wants back with each block. The tool
then takes each stretch with the same `pmemsave`, at the address of the
table, which is right for as long as the extender's memory is where
the machine's is, as it is on the bench. Whoever moves the memory
writes `_handed` and `_blocks` for the table, and `check` says whether
the net still catches a byte in a block.

**The 16 bytes round a block, for F1.** The extender keeps 16 bytes
of its own between two blocks and ends a program that has written into
them; the 640 KB take such a write without a sign. So from F1 on the
hand-out of the 640 KB keeps 16 bytes of a pattern round its blocks,
and the tool holds them: `guards` of `harness/dataimage.py`, at the
moment an image is taken. A scenario whose image has them otherwise
fails `portable_c-scenarios` whatever its screens show, with a line
that names the scenario, the block and the first byte that is another,
and `save` and `compare` say the same of their images. It is built and
tried, with what the image has in front of its first block today taken
for a pattern, right and then wrong by a byte, through a scenario and
in `check`; and it looks at nothing until a pattern is declared. What
F1 tells the tool, in that file:

- `GUARD`: the pattern, as the 16 bytes that `dos32.c` writes.
- `_guarded`: where they are. As it stands: the bytes that end where
  each block that is taken begins, and those that begin where the last
  block of the memory ends, taken or not. Today the paragraph in front
  of a block is where `dos32.c` keeps the block's length, and the last
  block ends where `Own` does, to within the 15 bytes that round it to
  a paragraph: F1 gives the pattern its room in both places, and makes
  `_guarded` say what it has made.
- `_blocks`: the blocks, which it reads from that paragraph in front of
  each. If the hand-out keeps its blocks otherwise, a table, this reads
  the table, by the symbol of the map that names it.

A pattern that is not in the image, behind the end of the program's
data, is said as that and fails too.

**What the next pieces should know.**

- The scenarios' test writes the images of every run, into
  `build/portable_c/images`: the next run of the test overwrites them.
  Keep "before" elsewhere, before the first build of the piece.
- The test holds nothing by the images. A piece is held to them by its
  own `compare`, and says in its commit what that printed in its last
  lines.
- A scenario that did not get to its end has no image, and `compare`
  fails for one that only one side has.
- An image is of the first copy of the program. Where a scenario ends
  in a second copy, started from the command line, that one's memory
  is not in it.
- Two builds whose code differs are expected to print "as before, but
  for what may differ" for some ten to twenty scenarios, and "0
  differences".
- The time that the saving costs: 0.4 seconds of reading the machines'
  memory, 1.2 of packing and writing and 0.2 of asking QEMU, for all
  155 scenarios together and summed over the four that run side by
  side. Against the test's own time that is not to be measured on a
  machine that others use: by the clock the test took 217 and 311
  seconds with images and 222 and 322 without, by the load, and the
  whole suite 271 and 478 seconds with them. By the processors' time,
  which the load moves less, 48 scenarios as the test runs them took
  190 and 204 seconds without images and 197 and 210 with, in the
  order 190, 197, 204, 210 as the load rose from 3 to 7.

**The record of the words.** `harness/words.py` counts how often each
word of the addresses is written in each file of `src/portable_c`,
`harness/words.txt` has the counts, and the test `portable_c-words`
holds the tree to them, fewer as well as more, as `portable_c-host`
holds it to its own: `python3 harness/words.py --record` in the commit
that moves a count, and the record's lines there say what went. A run
prints the table of the words: the places and the files of each, the
file with the most, and the piece that is to have it at nothing. As F0
leaves it, by that piece: F1 68 places, F2 536, F3 277, F5 45, F6 44,
F7 67, F8 212, F10 198; `ENTRY`, `DSKBUF`, `TRELIST`, `NUMBER_OF` and
the three of the tables stay. What it counts besides the words of
`vc.h`: `md.NAME` for the twelve words of the main data that hold a
segment or a place, `u16 panel` and `u16 segment` for a segment kept in
a number, and `AddressOf` and `OFFSET_OF` of the editor. Three things
to know:

- A word is counted in the body of another's definition, so `vc.h` has
  `AT` and `ARENA` for as long as `PANEL` and `OFFSET` are made of
  them.
- A word of the addresses that a piece adds to `vc.h` is named in
  `WORDS` of `harness/words.py`, with the piece that is to have it at
  nothing, as it is named in `WORDS` of `harness/int16.py`.
- A segment that is kept under another name than `panel` or `segment`,
  `other`, `top`, `behind`, is not counted: 4 of "What could go wrong"
  stands.
- The table says which piece is to have a word at nothing; the test
  does not hold a piece to it. `SEG_OF`, `OFFSET_IN_SEG` and `AT` go up
  first, at every place where a piece meets what another still has as
  a segment.

### F1, three blocks where there was one: done, and what a later piece should know

The program has a block for each panel and one for the room, inside
the 640 KB, and nothing is moved any more. All 155 scenarios pass with
no golden and no patch changed, at the first run.

**What it is now.**

- `Init11` takes two blocks of `PANEL_BLOCK`, 1000h paragraphs, and
  then the largest that there is, the room. `GiveMem` gives the room
  back and `TakeMem` takes the largest block again; neither touches a
  panel. `Aexec0` makes the room as short as `ROOM_LEAST` and then as
  long as it goes. `ReDr00` and `ReDr50` of `read.c` have lost their
  halves that moved the other panel up and down, 60 lines: the list's
  limit is what the original has with memory enough, a paragraph short
  of 64K less two entries, and the old list is kept where the room
  begins.
- Beside the main data, in `main.c`: `Room`, where the room begins, and
  `RoomEnd`, where it ends, both `u8 *`. `RoomParas()` of `start.c` is
  the one function that says how long the room is, in paragraphs of 16
  bits. `ViewSeg`, `MemSeg` and `FreMem2` are in `struct MainDat` with
  "Not set or read here". Two pointers and not three: `MemSeg` was the
  block that all of it was in and has no successor, and `ViewSeg` no
  longer moves, so where the room begins is its block.
- Elsewhere `SEG_OF(Room)`, 22 places in `copy.c`, `edit.c`, `find.c`,
  `view.c`, `files.c` and `start.c`, by one line of a script.
- `dos32.c` keeps what it knows of its blocks in a table beside the
  memory, `OwnBlocks` with `OwnHeld`, where it kept a paragraph in front
  of each block in the memory itself. That paragraph holds the pattern
  now, F0h down to E1h, written when the block is taken, and the last
  paragraph of the memory holds it from the start. So the room is
  0x7FFD paragraphs, 524,240 bytes, on any machine: 640 KB and a
  paragraph, less two panels and four paragraphs of the pattern.

**What the plan had wrong or had not.**

- "The room equal where it was written" does not hold against the
  build before, and cannot. The original keeps the old list of a
  directory that is read behind the panel as it is moved up, some 64K
  beyond where the room begins afterwards; with nothing moved it is
  where the room begins. So the room's bytes differ from the build
  before in 142 of 155 scenarios, by where things lie in it and by what
  each has left under the next. See "The image" for what held it
  instead.
- `ReDr50` moves a panel too, down again, and was not named.
- The 16 bytes cost the hand-out its place for what it keeps of a
  block. The plan's 64 bytes are right only with that in a table, which
  F0's section had as F9's. It is F1's now, and `_blocks` of the
  image's tool reads it.
- `PANEL_PARAS` and `VIEW_PARAS` are gone as the record wants, and a
  count of paragraphs is still what the functions for the memory take:
  `PANEL_BLOCK`, `ROOM_LEAST` and `RoomParas` are in `WORDS` of
  `harness/words.py` for F9, which makes them bytes.
- What `Init11` answers where there is too little: the paragraphs that
  there were for the room, or for a panel that got none. On the bench
  neither can happen, the memory being the program's own.
- `copy-big` gets to its point again, and `editor-full` does not:
  below.
- The image's tool looked for the pattern in the images of the build
  before as soon as it was declared, and said of each that it was not
  there. A build is known to write the pattern by having the table.
- The clock's separator follows the hundredths of a second, and is in
  the program's copy of the screen: `clock-path` differed in that byte
  against the build before, which takes thousands of instructions more
  to a step for the panels that it moves, where `save --early` of the
  piece's own build did not differ. It is in `UNSTEADY` by reading
  `Input` of `input.c`, not by F0's trial, which is too fine for it.

**The image, and how it was read.** Three sets of images: of the build
before, saved before anything was changed; of the piece's build, as the
scenarios' test left them; and of a third build made for this alone, in
`build/` and not kept: the source before with its panels fixed at 64K,
three lines, `PANEL_PARAS` 0x1000, `OWN_PARAGRAPHS` 0x9FFD so that its
room is as long as the piece's, and in `ReDr50` the place that the
other panel is moved down to made `panel + 0x1000`. That build does
what the original's routines do and moves nothing, since everything is
where it would be moved to.

| Held against the piece's build | Declared | What was left |
|---|---|---|
| The third build | `FreMem2`, `MemSeg`, `the blocks`, `ViewSeg`, `dos32.OwnCount` | Two runs of bytes in one scenario, `view-commands`: three bytes at +8000 of the room, where the old hand-out wrote what it kept of the free block behind the room while a viewer of another's ran. Everything else equal in all 155: the main data, both panels in all of their 64K, all 524,240 bytes of the room, the variables. "2 differences, 467 more in what was declared as changed, 191 in what may differ" |
| The build before | `FreMem2`, `MemSeg`, `the blocks`, `ViewSeg` | 17,722 differences in 144 scenarios, of two kinds. The room, in 142 scenarios, for the reason above. And in the main data what follows the room's length: the editor's count of free bytes as digits, in `TmpErr` and at the start of `TmpBuf`, in 27 scenarios; `EdSpace` and `ViSpace`; `FndSize`, 8,064 lines for 9,873, in five; and in `copy-big` how much was read, the source's handle and a cell of the bar. No panel differs as far as the build before has it, and no variable. With those declared too, `behind the panels`, `TmpErr`, `TmpBuf`, `ErrBuf`: "0 differences, 18652 more in what was declared as changed, 209 in what may differ" |

What each declared name is: `FreMem2` names the end of the room, which
is another length; `MemSeg` is 0 where it was a segment; `the blocks`
are three where there was one; `ViewSeg`, in `start-memory` alone,
names a room that is given back when the image is taken, which the
build before has as the end of its block made short; `dos32.OwnCount`
is the third build's own.

So the judgement rests on the third build. Declaring the room and
`TmpBuf` as changed against the build before would have passed
anything.

**The pattern, tried.** A line at the end of `Init11` that writes a 0
behind the end of the first panel's block, not committed:
`portable_c-scenarios` failed for all 155, each with "the 16 bytes in
front of block 1 are not the pattern, from the byte at +0 of them on",
and `save` says the same. `python3 harness/dataimage.py check` catches
a byte in each kind of place in the piece's images and in those of the
build before.

**The scenarios that the plan names**, with the room at 524,240 bytes:

| Scenario | What it does |
|---|---|
| `start-memory`, `second-copy`, `command`, `enter-many` | Pass. `enter-many` holds the 256 files, which are cut as they were: question 4 is not touched |
| `copy-big` | Reads its file of 526,632 bytes in two parts again, the first of 523,776, and is skipped with not all of it read: `CpRead` and the source's handle in the image say so. It gets to its point, as it did not while the room was 641,824 bytes |
| `editor-full` | Passes, and is still never full: 130,270 bytes free at its end, where the build before had 247,854 and the original has none |
| `editor-big` | Not run, as before: its file of 381,338 bytes still fits |
| `view-window` | Passes: a megabyte is refused as it was |

Entry 13 of `DEVIATIONS.md` has the new figures and entry 1 the three
blocks.

**The records.** The words that were F1's are at nothing: `md.ViewSeg`
38, `md.MemSeg` 14, `md.FreMem2` 8, `PANEL_PARAS` 6, `VIEW_PARAS` 2.
Left to go, by piece: F2 496, where it was 536, since `read.c` and
`start.c` lost 40 mentions of a panel with what moved them; F3 277;
F5 45; F6 44; F7 67; F8 212; F9 13, new; F10 211, where it was 198,
`SEG_OF` being up by 22 and `MAIN_SEG` down by 8. `harness/int16.txt` has one place more,
`RoomParas`, with its reason. The host check's record did not move.

**How long it took**, in tool calls, some 95 in all: reading the brief,
the plan and F0's section 8; reading the code 9; the program and the
tool written, to a build without warnings, 12; the records and the
first run of the scenarios, which passed, 6; the images, with the third
build, 16; the pattern's trial 4; the tool's own check, the records and
the documents the rest. The change to the program is small: 185 lines
of `dos32.c`, 160 of `start.c`, 90 fewer in `read.c`, and the 22 places
of the bridge.

**What a sweep should do otherwise, F2 above all.**

- Save "before" first, and with the tool as it is at that commit. F1
  changed the tool and had to save a set again.
- A sweep moves nothing and changes no length, so its images are to be
  equal to the build's before with nothing declared but the four words
  that leave the main data, each by its `variable=` in `ADDRESSES`.
  Anything else that differs is a fault. No third build is needed
  where no layout changes; where one does, make the build that
  separates the layout from the code, as here.
- The bridge is as cheap as the plan thought: `SEG_OF(Room)` is exact
  and costs nothing to write. It is read anew at every use, which is
  what makes it safe with a room that may move; a sweep that keeps a
  panel's pointer in a variable is safe too, since a panel never moves
  from F1 on.
- The compiler holds a sweep both ways, tried with `wcc386 -wx -we`: a
  pointer handed where a number is taken and a number where a pointer
  is taken are W102, and a pointer held against a number is refused
  too. So F2 can go by a script and then by what the build refuses.
- What the plan feared most of F2, arithmetic on a panel and two
  panels held against each other by `<`, is gone with F1: it was all in
  `ReDr00`, `ReDr50`, `GiveMem` and `TakeMem`. What is left are twelve
  places that hold a panel against a word of the main data by `==` or
  `!=`, counted by `grep`, which are right of pointers.
- Count a table of the system's file in 32 bits: a `u16` index was six
  places for `portable_c-int16` to ask about. A difference of two
  pointers that is shifted is a place too, and wants its reason.
- `harness/dataimage.py check DIRECTORY` after any change to the tool.

**The pieces behind, from here.** Opinions, by what F1 cost.

- F2 is one agent's, if it goes by a script: its 496 places are of
  four patterns, and nothing in it needs thought once F1 has taken the
  arithmetic out. The reading is in the five places of kind i and in
  whatever held a panel under another name.
- F3 is the one to watch. It changes what `Dialog` and its like are
  handed in 21 files and teaches `harness/reads.py` a new form, and
  the test of the data judges it as well as the scenarios. If it is cut,
  then by `reads.py` first, alone, with the program unchanged.
- F4, F6, F7 and F8 look rightly sized. F4 and F6 meet the room: each
  takes `SEG_OF(Room)` out of its files and has `Room` itself.
- F5 is more than one agent holds, as the plan says: 29 `AT`, 24
  `AddressOf`, 21 `OFFSET_OF` and 13 `SEG_OF(Room)` in one file, and
  the arithmetic of `struct Address` is where C1's fault was. Cut it as
  the plan offers, `T50_00` to `T54_00` apart.
- F9 has less to do in the system's file than the plan has: the table
  of the blocks is there, and the tool reads it. It has `RoomParas`,
  `PANEL_BLOCK` and `ROOM_LEAST` to take out, 13 places.
### N0, the finder, the record, the types: done, and what a cluster goes by

**What there is.** `harness/width.py`, its record `harness/width.txt`
and the test `portable_c-width`; `bool` and `i32` in `vc.h`;
`FileSize`, `FileStep`, `ByteCount` and `MemSize` in `sys.h`, defined
and not used; the fields of the main data and of a panel sorted, in the
record; and the rule tried whole on `digits.c`, `mask.c`, `line.c`,
`mouse.c` and `blank.c`. Every scenario passes and the data's image of
each of the 155 is that of the build before, with no difference.

**The finder** is the finder of `harness/int16.py` with two things
more, and its top says all of it. It finds the numbers, everything of 8
or 16 bits that a file declares, and the cuts, where a number that may
not fit, by the types alone, is put into 8 or 16 bits: `cast`, `store`,
`step`, `handed`, `returned`. The plan said that the finder named every
store that cuts and had two things to learn. It named none: its kinds
are where a number is looked at before it is cut. So `store`, `cast`
and `returned` are new as `step` and `handed` are.

**The record.** A file, a function or a structure is in it as `not
sorted` with its counts, as sorted with an entry and a reason for every
number that is left and every cut that is left, as `laid out` with
whose it is, or `as a whole`, the files of tables and the system's
file. At its top is a table by cluster and by file: the numbers of 8 or
16 bits that there are, how many of them nobody has sorted, how many
are kept with a reason, the cuts, and how many numbers a file declares
as `u32`, as `bool` and with a sign. A cluster's commit shows in that
table what it widened, and the count that it takes to nothing is the
column `not sorted` of its row. It is 1,019 lines with five files and
the two structures sorted; a file that is sorted has some four lines
for every number and cut that is left, so the whole of it will be some
5,000 lines if half of the numbers stay as they did here, and fewer if
the clusters keep fewer.

Three things of the plan are otherwise:

- `harness/int16.py`, its record and its test stay beside it. What it
  finds is still where the int of the code as it was written shows, and
  a widening moves it: two places came and two went with the five
  files. Folding its record into this one would be a second kind of
  entry for no gain.
- A cast is in the record only where it cuts, by the finder's ranges:
  `(u16)*text` is no entry, `(u8)mode` is.
- A number that was widened or made a boolean is no entry. The record
  is what is narrow, and the table counts the rest.

**The types.** `bool`, `true` and `false` are of `<stdbool.h>`, which
Open Watcom has in the directory that the build names for `<stddef.h>`
and gcc and clang have without a library; a `bool` is one byte for all
three, which a `HOLDS` line holds. clang will want nothing else at step
6. `i32` is `int`, with a `HOLDS` line for its length and its sign. No
number of the five files wanted it. The program was the same bytes with
the types defined.

**The rule as it stood up**, by what the 66 numbers of the five files
became:

| | Numbers | Which |
|---|---|---|
| Widened to `u32` | 29 | Counts, indexes, lengths; what `TstHex`, `LnStr`, `DoublMo` and two static functions answer |
| Made `bool` | 3 | What `TstMas` and `InWind` answer, what `CurMous` is handed |
| Kept, as wide as it is by what it is | 34 | 15 a character, a colour or a cell; 9 a place, a row and a column in two bytes; 6 what the system answers in, the mouse's buttons, the keys that are held, the page, the timer's low word; 2 a time of DOS's; 2 a random number |
| Counted on to wrap all its life | 0 | |
| With a sign | 0 | |

And of the 41 cuts that the finder named before, 29 are left, each
with its reason: 12 are a cast that is written, of which 5 say where
the program counts on wrapping (`wraps:` in the record), and the others
are a number that what stands at the place keeps within.

- Half of the numbers stay. A character, a place or a colour that the
  translation has in 16 bits, since the assembly had it in a register,
  is kind 1 by the rule, and widening it gains nothing. The cost is a
  reason for each and one for every cut into it.
- Few are booleans, and the finder does not say which: a number is
  one when every value put into it and every use says so, and of what
  another file calls that is a `grep` for its calls. `CurMous` took
  "the low byte of `show`" and is handed 0 or 1 by all 13 calls.
  `DoublMo` answers 0, 1 or 2 and is a number.
- A number that wraps at one place: the variable is widened and the cut
  written there. As it was and as it is, in `LnStr` of `line.c`:

      u16 more;
      more = (second << 8 | second & 0x7F) - 1;

      u32 more;
      more = (u16)((second << 8 | second & 0x7F) - 1);

- A mask that meant "all but the low byte" of 16 bits cuts a number of
  32. Nothing names it, since it is no cut by the types. In the same
  routine:

      length = length & 0xFF00 | length + more & 0xFF;

      length = length & ~(u32)0xFF | length + more & 0xFF;

  Before a variable is widened, look at every mask of it that is
  written for 16 bits, `0xFF00`, `0xFFFF`, `0xFFF0`, at every `~` and
  `-` of it, and at every line that holds it against `0xFFFF`.
- A variable that takes two things in turn, `now` of `Blank`, the keys
  that are held and then the timer's low word, stays as it is with both
  said.

**Across clusters**, as "The rule for a number" has it, and it held.

- A return that is widened: `TstHex`, `LnStr` and `DoublMo` answer
  `u32`. No caller changed. The record counts a cut more in each file
  that keeps the answer in 16 bits, nine in nine files, and the commit
  shows them as counts that moved: the cluster of that file meets the
  cut when it sorts the file.
- A parameter that is widened: `python3 harness/width.py --show NAME`,
  before the change, lists every call in the tree that counts on the
  parameter to cut what it is handed. There was none for `HexByt` and
  `DoublMo`.
- What it did not say: a file that declares another file's function for
  itself. `edit.c` has `HexByt` so and `dialog.c` `LnStr`, and each
  line changes with the function, in a file that is another cluster's.
  The compiler stops for it, so it cannot be missed; it is a line of a
  file that someone else may have open.
- A number that is widened and then kept in a field of the main data
  is a cut with a reason, until M: `md.MousPos = cursor`.

**The fields of the main data and of a panel** are entries of the
record, under `vc.h`, each with one word and what is not plain: `setup`
46, which stay bytes; `flag` 16; `text` 22 and `cells` 5; `address` 16,
which a piece of the addresses takes out, or question 3 for the four
places of a panel; `small` 10; `system` 6; `unused` 7; `wraps` 2, the
random numbers; and `M` 5, which the piece M takes out to widen:
`ComSpec`, `ClstSiz`, `Total`, `FreeDsk`, `MousPos`. They were sorted
by name, by comment and by one `grep` of what the code puts into each,
not by reading every use, so a cluster may find one otherwise and
changes its line then. One thing was seen on the way and not looked
into: `ClstSiz` is a sector's bytes times its sectors in 16 bits, which
is 0 for a cluster of 64K. Whether the original shows something wrong
there is for the cluster that has the Info to see and the user to
decide.

Eleven structures of the headers are `laid out`. The others are the
program's own, what a function answers in, and are not sorted: each
goes with the cluster of the functions that answer in it, and those of
`sys.h` with S and the layer. No cluster owns them by the table of the
clusters, which is by file: 133 numbers.

**What one agent holds.** The five files, 66 numbers and 41 cuts in 738
lines, took some 12 tool calls of the piece, with the files read whole,
one script for the edits, one for the reasons, one build and one run of
the suite: nothing failed and nothing had to be read in another file
but the calls of eight functions. That is no measure of a cluster.
What it does say: the reading is cheap and the writing is not. 63
entries wanted a reason, six of every ten things that the finder
named, and a reason is a sentence that somebody has to have checked.
By that, a cluster is some 150 numbers and 100 cuts, 150 reasons, for
an agent that hands back after a hundred tool calls, and the counts of
the record's table are what to cut by, not the lines:

| Cluster | Numbers; cuts | |
|---|---|---|
| N3 and N4, `view.c` | 197; 211 | Two halves are right |
| N6 to N8, `edit.c` | 248; 278 | Three thirds are right, and no fewer |
| N17, `keys.c`, `commands.c` | 222; 58 | The largest by numbers. 116 are of `commands.c` with 13 cuts, most of them what a command is handed; F2 makes many of those pointers. Cut in two by file if it is still so after F8 |
| N19, five files | 163; 74 | `start.c` waits for F9 anyway: a cluster of its own, 49; 30 |
| N12, N13, N15 | 88; 113, 71; 101, 114; 108 | More cuts than numbers, `treebox.c`, `usermenu.c`, `panel.c`, `drive.c`: arithmetic on places. Right as they are, and the slow ones |
| The others | 90 to 140; 50 to 100 | Right |

**What a cluster does**, in this order:

1. `python3 harness/dataimage.py save` of the build that it starts
   from, into `build/`.
2. `python3 harness/width.py --show FILE`: the numbers and the cuts,
   function by function. Read the file with that beside it and say of
   each number which of the five kinds it is. For a parameter or a
   return of a function that other files call, `grep` its calls, and
   `--show NAME` for what is cut in being handed to it.
3. Change the types, in the file, in `vc.h` and in whatever file
   declares the function for itself; write the cut where the program
   counts on wrapping; look at the masks. Build.
4. `python3 harness/width.py --sort FILE`, or `FILE:FUNCTION` for a
   part, and write the reason under every entry that has none, in
   `harness/width.txt`. `python3 harness/int16.py`, and its record anew
   with the reason of whatever is new there.
5. clang's three warnings for the files ("Booleans"), the tests that
   start no machine, then the whole suite and the images held against
   those of step 1.
||||||| parent of 742d10a (Write down the three blocks of step 2, and what a later piece should know)

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
  the scenarios take the jump one way only. For the program of 32 bits
  the file fits, and the scenario is not run for it: "The editor's
  room: untested in the program of 32 bits".
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
