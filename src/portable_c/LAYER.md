# The layer: what the program asks of the system, and how

**Draft: nothing in it is approved unless it says so, with the date.**

This is step 3 of the road (`CLAUDE.md`, "The portable C"): the design
of the abstraction layer, made with the user a group of functions at a
time. It was begun on 13 October 2026, while step 2 was still landing,
from the tree at 32827fb. It changes no code. What the user decides is
written into it with the date, and nothing is built on a part of it
before that part says it is approved.

`sys.h` is what there is until then: a seam that keeps the registers in
one file, with `dos32.c` behind it. This document starts from it, says
what of it stays, what changes shape and what goes back into the
program, and puts each choice that the whole layer turns on to the user
as a question.

Contents: 1, what the layer is for, with the principles proposed; 2,
the inventory of `sys.h` and of what else the program asks; 3, the
questions that cut across the groups; 4, the order of the groups; 5,
the first group, proposed in full.

## 1. What the layer is for

The program is a faithful translation of an assembly program for DOS,
with its names and its behaviour, bugs included until each is decided.
The layer is about what that program asks of the system it runs on, and
not about rewriting it. It is to let the same common code run on the
platforms in view:

- **DOS on a PC, under the extender DOS/32A**, built by Open Watcom's
  `wcc386` and judged on the bench by every scenario. It is the one
  implementation that runs, today `dos32.c`, and step 4 moves it under
  the layer with no change that a scenario, or the list of DOS's calls
  that `harness/trial32/calls.asm` makes, shows.
- **A stub** (step 5): every function there, each answering "failed"
  or "none", so that the common code builds without DOS. It is not
  meant to run.
- **POSIX with clang on Linux, for 64 bits** (step 6): the platform that
  the road names. The stub is built there first; what of the x86 or of
  DOS is still in the common code goes behind the layer then, in files
  `_dos.c` and `_posix.c` beside common headers, as Chromium has it.
  Each POSIX function is described here in a sentence, with its system
  call, to show that the shape can be answered; none is written.
- **Beyond that**, what the user has mentioned, iOS, is no target. It is
  a reason not to close doors: no part of the layer is to assume a
  terminal, a `fork`, a shell or a disk with drives, only to answer
  "none" where a system has no such thing.
- **An implementation for tests** is future work (`TODO.md`, "An
  implementation of the layer for tests: future work"), not to be begun
  without the user. Its section "What it asks of the layer's design" is
  taken into the principles below: low and narrow, deciding nothing,
  reading nothing of the program's data, the critical error as a call
  back, time only through the layer, memory in blocks that are
  pointers.

The principles proposed, each for the user to accept or change:

1. **The program decides; the layer answers.** Every behaviour that the
   scenarios judge, and every row of questions in the original's order,
   stays in the common code. A platform's function answers one question
   and decides nothing that a screen shows.
2. **One function is one thing asked**, as in `sys.h` today. A function
   that does two things on DOS is two functions.
3. **DOS's values where the program decides or shows by them, plain C
   where it only carries them.** Error numbers, attributes, a packed date
   and time, a key's word, a cell of the screen are the layer's own
   values, defined in its headers; a size, a count, a handle, yes or no,
   a pointer are numbers of a said width, `bool` and pointers. Another
   platform maps to them (question 1).
4. **What a platform lacks it answers with the value that means none**,
   which the function's comment says: no unit shared, no mouse, no
   device error, version 0. A function that only asks whether a thing is
   there is made only where the program does something else without it.
5. **A failure is an error number that comes back, 0 for none.** What a
   function answers besides is a member with a name. A thing that DOS
   happens to leave and that a caller goes on with is named at the
   function that answers it, or goes as a declared deviation (question
   10).
6. **What crosses is the caller's.** A name or a buffer is lent for the
   call and no longer. Where the layer is to keep a pointer, the record
   of a search, the program's function for a device error, its function
   says so.
7. **The layer reads and writes nothing of the program's data**, `md`
   and the panels, and the program nothing of the machine's.
8. **No register, segment, selector, interrupt or type of a platform**
   (`off_t`, `termios`, a DPMI handle) in a header of the common code.
   Numbers are `u8`, `u16`, `u32`, `i32`, `i64` or `bool` by the rules of
   `CLAUDE.md`, and a type is named only where its width differs by
   platform (question 16).
9. **Nothing arrives unasked but through a function of the program's
   that the layer was handed**: the device error during a call. Keys,
   time and everything else are polled.
10. **Time is what the layer says it is.** The program waits only through
    the layer: for a key, for a tick, by saying that it is idle.
11. **The DOS implementation is today's.** Step 4 is a move, not a
    rewrite: the same calls of DOS, in the same order, with what entry 6
    of `DEVIATIONS.md` has decided of the registers.

## 2. The inventory

Every function of `sys.h` at 32827fb, as `sys.h` groups them. "Calls" is
the number of places that call it, `grep` of `Name(` in the files of the
program, by file. "Everywhere" says whether every platform in view has
the thing. "Of DOS alone" says, where the thing is DOS's or the PC's,
what a user sees of it on DOS. "Goes to" is where it belongs in the
layer, as proposed: **layer** for a function that every platform makes,
**layer, none elsewhere** for one that another platform answers with
"none", **program** for what is a decision of the program's and goes
back above the layer, **DOS file** for what only the DOS implementation
needs and no caller of the common code. Nothing in that column is
decided; each is a proposal for its group.

### Files

All asked with the program's errors but the three Bare ones (question
9).

| Function | Calls | What the program uses it for | Everywhere | Of DOS alone, and what a user sees | Goes to |
|---|---|---|---|---|---|
| `FileCreate` | 7 (stmain 1, copy 1, dirfunc 1, usermenu 1, edit 1, setup 1, treebox 1) | the setup file, a copy's target, the editor's file, the tree's file, the menu file, a label (attribute 8, `F54_00`) | yes | a label made by creating a file with attribute 8 | layer; the label to "Drives" |
| `FileOpen` | 4 (system 2, stmain 1, sthelp 1) | `OpenFil`, with its second try without sharing; the setup's help and file | yes | the sharing bits of the mode | layer |
| `FileOpenBare` | 1 (start 1) | the setup file at the start | yes | | layer, as `FileOpen` (question 9) |
| `FileClose` | 27 (sthelp 1, usermenu 3, copy 6, dirfunc 3, find 2, help 1, stmain 2, setup 1, read 2, treebox 2, edit 2, view 2) | every file opened | yes | | layer |
| `FileCloseBare` | 1 (start 1) | the setup file at the start | yes | | layer, as `FileClose` |
| `FileRead` | 15 (help 1, stmain 1, copy 2, edit 1, find 1, read 2, sthelp 2, treebox 1, usermenu 1, view 3) | the viewer, the editor, a copy, the help, the search, dirinfo, the tree, the menus, the extension files | yes | | layer |
| `FileReadBare` | 1 (start 1) | the setup file at the start | yes | | layer, as `FileRead` |
| `FileWrite` | 10 (setup 1, copy 2, dirfunc 1, edit 1, treebox 1, stmain 1, usermenu 3) | a copy, the editor, the setup file, the tree, the menu file, the list of files for a command | yes | a count of 0 cuts the file there | layer |
| `FileSeek` | 14 (sthelp 1, copy 6, find 1, help 1, view 5) | the viewer, a copy's size and an appended `^Z`, the help, the search going back 80 bytes | yes | what a failed seek leaves, which `view.c` and `copy.c` go on with | layer |
| `FileDelete` | 14 (copy 9, dirfunc 2, edit 1, treebox 1, usermenu 1) | delete, move, the editor's file after a save that failed, the tree's file, the list of files for a command | yes | | layer |
| `FileRename` | 3 (copy 3) | rename and move | yes | | layer |
| `FileAttributes` | 14 (setup 1, copy 8, dirfunc 2, edit 1, stmain 1, treebox 1) | a copy's source and target; the attributes dialog; the setup file's and the tree's attributes kept when they are made anew | yes, mapped | DOS's bits: hidden, system, archive | layer |
| `FileSetAttributes` | 14 (copy 8, dirfunc 3, edit 2, usermenu 1) | the attributes dialog, a copy, the editor, a read-only file deleted | yes, mapped | hidden, system and archive have no POSIX meaning | layer |
| `FileTime` | 3 (copy 2, dirfunc 1) | a copy keeps the source's stamp; the attributes dialog | yes | DOS's packed date and time | layer |
| `FileSetTime` | 2 (copy 1, dirfunc 1) | the same | yes | | layer |
| `FileDevice` | 4 (copy 3, edit 1) | a copy to or from a device, `CON`, `PRN`; the editor's target | yes, as "not a regular file" | DOS's device names in any directory | layer |
| `FileTrueName` | 2 (dirfunc 1, read 1) | leaving a directory before removing it; the name of a drive for its label | yes, as `realpath` | DOS's `\\server\` and `SUBST` forms | layer |

### Files by a file control block

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `FcbParse` | 2 (dirfunc 2) | a name made an FCB for deleting by a mask and for the label | no | all of it: deleting every file of a mask at one call, `F38_70`, and the label, `F54_00` | layer, none elsewhere; or a "delete by mask" and a "label" of the layer whose DOS file makes the FCB (group 10) |
| `FcbDelete` | 3 (dirfunc 3) | the same | no | as above, with the carry flag that DOS leaves (`flagged`) | as above |

### Directories

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `FindRecord` | 1 (system 1) | `SetDTA`: where a search puts what it finds, `md.DTA`, DOS's record | the state, yes | DOS's record of 43 bytes, kept by the system between two calls | layer, in another shape: a search answers its record (question 3) |
| `FindFirst` | 10 (copy 1, dirfunc 3, read 2, treebox 1, view 1, find 1, enter 1) | reading a panel, the tree, the search, a copy, the labels, the viewer's next file | yes, `opendir` and a match | 8.3 names, the attributes as a filter, a label as attribute 8 | layer |
| `FindNext` | 6 (find 1, read 1, treebox 1, dirfunc 3) | the same | yes | | layer |
| `DirMake` | 3 (copy 2, dirfunc 1) | F7, a copy of a directory | yes | | layer |
| `DirRemove` | 4 (copy 2, dirfunc 2) | delete, move | yes | | layer |
| `DirChange` | 11 (copy 1, dirfunc 3, commands 1, enter 3, drive 1, find 1, keys 1) | a panel's directory made current, Enter, the tree, a command run there | yes, one for the process | one for each drive | layer (question 2) |
| `DirCurrent` | 7 (find 1, drive 1, system 1, dirfunc 1, edit 1, main 1, read 1) | the path of a panel, of the editor's file, of the program's | yes | one for each drive, without its drive and first backslash | layer (question 2) |
| `DirCurrentBare` | 2 (exec 1, start 1) | the prompt between commands; the start | yes | | layer, as `DirCurrent` |

### Drives

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DriveCurrent` | 7 (stmain 1, find 1, system 3, start 1, exec 1) | `TstDrv`, whether a drive exists; `SetPth`; the prompt; the search | as one drive | all drives (question 2) | layer |
| `DriveSelect` | 10 (drive 1, keys 2, commands 1, copy 2, find 1, main 1, system 2) | `TstDrv`; a panel's drive made current; Go to | as one drive | | layer |
| `DriveSpace` | 2 (copy 1, read 1) | a copy's room; the Info panel's free space | yes, `statvfs` | in clusters and sectors, with `flagged` | layer, in bytes (group 4) |
| `DriveLetter` | 2 (drive 1, system 1) | A: and B: on one floppy drive: the drive menu, the prompt "Insert disk for drive B:" (`Fantom`) | no | the prompt and its setting, kept while DOS is a target (user, 13 October 2026) | layer, none elsewhere: "shares no unit" |
| `DriveSetLetter` | 5 (drive 1, system 3, main 1) | the same, and the box of a drive not ready | no | as above | layer, none elsewhere |
| `DriveRemote` | 1 (find 1) | which drives the file search goes through for "all drives" | as a mount's kind | | layer, or one function "what kind of drive" (group 4) |
| `DriveFixed` | 1 (find 1) | the same | as above | | as above |
| `DriveCd` | 1 (find 1) | the same, by MSCDEX | as above | the CD-ROM's own question, INT 2Fh | as above |

### Memory

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MemTake` | 5 (start 5) | the panels and the room, at the start and after a command | yes | paragraphs and a segment until F9; "how large a block can be" | layer, in bytes and pointers (question 13) |
| `MemGive` | 1 (start 1) | the room given back before a command | yes | | layer |
| `MemResize` | 3 (view 3) | the room cut and grown around a viewer of another's | yes | | layer |
| `MemSetSearch` | 1 (exec 1) | `SetStat`: DOS to look for the lowest block, as it does when it starts | no | none on the screen: where DOS puts a command | layer, none elsewhere, or DOS file (question 14) |
| `MemSetUpper` | 1 (exec 1) | the same, the upper memory out | no | as above | as above |
| `MemOwn` | 1 (exec 1) | `Alloc3`: the program's block made as long as it is | no | "not enough memory" at the start | as above |

### The time, the country, the system itself

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `TimeOfDay` | 6 (input 4, main 1, mouse 1) | the clock on the screen, the times of a key, a double click | yes | to the hundredth, packed in bytes | layer |
| `CountryInfo` | 1 (start 1) | how dates, times and numbers are written: `md.Country` | yes, from the locale, mapped | DOS's record, with a far address of its case map in it | layer, in a record of the layer's own (group 2) |
| `CaseMap` | 2 (start 1, system 1) | the table of capitals of 80h to FFh at the start; `UpCase`, every byte of a search for a text | yes, by the code page | DOS's routine through the extender, slow (user, 13 October 2026) | layer, the code page's capitals (question 4) |
| `SystemVersion` | 2 (start 1, exec 1) | `Alloc3`: refuse a DOS before 3; the version noted | no | the message and the end on an old DOS | layer, none elsewhere: "new enough" |
| `SystemTrueVersion` | 1 (start 1) | `VerDOS` noted, which nothing in the portable C reads | no | nothing | DOS file, or gone with `VerDOS` (a deviation of order, to ask) |
| `BreakCheck` | 1 (exec 1) | `SetBrk`: what DOS was set to, put back at the end | no | Ctrl-C checked at every call | layer, none elsewhere (question 14) |
| `SetBreakCheck` | 3 (exec 3) | the same, around commands | no | as above | as above |
| `Idle` | 1 (blank 1) | each time the program looks for a key and finds none: INT 28h | yes, as a short wait | other programs of DOS get a turn; the harness's driver knows a step is over | layer: the one place where a POSIX program sleeps |

### Other programs, and the end of this one

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ProgramRun` | 1 (view 1) | `Aexec0`: a viewer or an editor of another's | not on iOS | DOS's tail: a count byte, the text, a carriage return | layer (question 12) |
| `ProgramRunBare` | 1 (exec 1) | `Exec`: COMMAND.COM for the commands | as above | COMMAND.COM and `/C` built by the program | layer, as `ProgramRun` (question 12) |
| `ProgramEnd` | 3 (exec 3) | the program leaves | yes | | layer |
| `ProgramEndOld` | 1 (exec 1) | the end on a DOS before 3 | no | | layer, none elsewhere, or with group 10 |
| `ProgramTail` | 1 (main 1) | the path on the command line that the program was started with | yes, `argv` | the tail as DOS keeps it | layer, as the arguments (question 12) |
| `ProgramEnvironment` | 3 (exec 1, start 2) | `COMSPEC`; the program's own path behind the environment, read as 68 bytes by `MnPth0`, for where its files are | yes, `environ` and the path of the program | DOS's block, and the path behind it | layer, by name: a variable, the program's path (question 12) |

### What the system asks the program

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DeviceErrorFn` | type | `FtErr1`, the box with Retry and Abort | no | the critical error: a drive without a disk, a disk that is protected | layer: the program's function, which another platform never calls |
| `DeviceErrorNote`, `DeviceErrorTake`, `DeviceErrorGive` | 1 (main 1), 1 (main 1), 1 (main 1) | `HookVct` and the end | no | | layer, none elsewhere |
| `IntOn` | 3 (box 1, window 2) | behind what goes to the screen: the box of the critical error gets its keys | no | | layer, none elsewhere, or DOS file (question 15) |
| `CtrlBreakNote`, `CtrlBreakTake`, `CtrlBreakGive` | 1 (main 1), 1 (main 1), 1 (main 1) | Ctrl-Break taken, so that nothing happens for it: the key 0 that it leaves is read as a stop by `TstBrk` | as SIGINT, in part | the key of a PC | layer (question 15) |
| `CtrlCTake` | 1 (exec 1) | `SetBrk` | as SIGINT | | layer (question 15) |
| `ProgramEndedBreakCheck`, `ProgramEndedTake`, `ProgramEndedGive` | 1 (exec 1), 1 (exec 1), 1 (exec 1) | DOS ending the program by another's hand: what DOS is to put back | no | the scenario `ended` | layer, none elsewhere; or DOS file entirely, said by `SetBreakCheck` (group 9) |

### The console

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ConsoleChar` | 1 (start 1) | the space that DOS writes at the start | yes, as the terminal before the screen is taken | | layer, with the screen (group 6) |
| `ConsoleText` | 2 (start 1, stmain 1) | the banner at the start; the setup's line for a file not written | as above | ended by `$` | as above, ended by a zero (group 6) |

### The keyboard

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `KeyWaiting` | 6 (blank 1, find 1, input 2, system 2) | `Input0`, `TstBrk`, `ClrKbd`, the blanker, the search | yes, by parsing the terminal | the BIOS's key word | layer (question 7) |
| `KeyRead` | 8 (blank 1, find 1, exec 1, input 2, start 1, system 2) | the same | yes | | layer |
| `KeysHeld` | 6 (blank 2, menu 1, bars 1, input 2) | the key bar of Shift, Ctrl and Alt held; the blanker; the menu | not in a terminal | the bits of the BIOS's data | layer, none elsewhere: "none held" (question 7) |
| `KeyForgetIns` | 1 (keys 1) | Ins held down selects file after file | no | the BIOS's Ins bit | layer, none elsewhere |

### The screen

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ScreenState` | 11 (window 4, blank 3, main 1, stmain 1, start 2) | the mode, the columns, the page shown | as "colour or not" and a size | modes and pages | layer, as the screen's size and kind (group 6) |
| `ScreenSetMode` | 2 (start 2) | the start: mode 3 if the mode is not of text | no | | DOS file, or none elsewhere |
| `CursorWhere` | 8 (stmain 1, blank 1, main 1, start 2, view 1, window 2) | where the command line goes, the cursor kept and put back | yes | the shape in lines of a character | layer |
| `CursorTo` | 3 (blank 2, window 1) | the cursor of an input line; the blanker | yes | | layer |
| `CursorShape` | 8 (stcolor 1, stmain 1, blank 2, start 2, window 2) | hidden, a line or a block (Ins) | in part: shown or not, a bar or a block | lines of the character | layer |
| `ScreenScroll` | 2 (blank 1, window 1) | `Scroll` of a window by a line; the blanker clears | yes | the BIOS's call 6 and 7 | layer, or the program's own on its buffer (group 6) |
| `ScreenCell` | 2 (start 2) | the colour at the cursor when the program starts, `CrntCol` | only of what the program put | reading DOS's screen | layer (question 6) |
| `ScreenFill` | 2 (blank 1, start 1) | a star of the blanker; the cell put back at the start | yes | | layer, or `ScreenPut` (group 6) |
| `ScreenType` | 13 (exec 3, stmain 2, enter 2, main 1, start 4, system 1) | the command line, the prompt, the line feeds between commands, the bell | yes, as a terminal's output | the BIOS's teletype, with a page and a colour as they happen to be | layer (question 6) |
| `ScreenEga` | 1 (start 1) | whether the display may have more than 25 rows | no | | layer, as the rows of the screen (group 6) |
| `ScreenFont` | 1 (start 1) | how many rows the display has | no | the BIOS's font | as above |
| `ScreenPageStart` | 1 (window 1) | `ScrOffs`: where the cells of the page are | no | | DOS file |
| `ScreenMemory` | 1 (window 1) | the same | no | `md.Screen`, kept by `dos32.c` in the main data | DOS file |
| `ScreenPut` | 4 (window 4) | `Window`, `MovScr`: the program's buffer to the screen | yes | the display's memory | layer |
| `ScreenGet` | 1 (window 1) | `SavScr`: the screen read back after a command | only of what the program put | the screen that DOS and the commands share | layer (question 6) |

### The mouse

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MouseFind`, `MouseReset` | 2 (mouse 1, stmain 1), 2 (mouse 1, stmain 1) | whether there is a mouse; the setup sets it anew | yes, in a terminal that reports one | the driver's answers, 21h for an old one | layer |
| `MouseShow`, `MouseHide` | 1 (mouse 1), 1 (mouse 1) | the pointer off while the screen is written | yes, as nothing to do where the terminal draws it | a count of hides | layer |
| `MouseWhere` | 2 (mouse 2) | the buttons and the place, polled | yes | in dots, 8 to a cell | layer (question 7) |
| `MouseTo`, `MouseRows` | 1 (mouse 1), 2 (mouse 1, stmain 1) | the pointer put in the middle; the rows it may go to | not in a terminal | | layer, none elsewhere |
| `MousePointer` | 2 (mouse 2) | the pointer's look, by the colours it turns | not in a terminal | | layer, none elsewhere |

### The timer

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `Ticks` | 1 (system 1) | `Timer`: the blanker, a box that grows, the waits, double clicks | yes, from a monotonic clock | 18.2 a second, from midnight, read in the BIOS's data | layer (question 11) |

### What the program asks that is not in `sys.h`

What the brief named, the screen's memory written directly, the BIOS's
data read for the ticks and the mouse, is in `sys.h` now: `ScreenPut`
and `ScreenGet` write and read the display's memory, `Ticks` reads the
BIOS's count at 46Ch, the mouse is its eight functions. No file but
`dos32.c` has an address of the machine, an interrupt or a register:
`grep` for `0x0040`, `0xB800`, `_asm` and `#pragma aux` with registers
finds nothing outside it, and `tools/calls.py` counts no call of the old
way. What is left that is the system's and not a function of `sys.h`:

- **The entry.** `Start` of `dos32.c` is where the extender enters; it
  settles the extender, the PSP, the block below the megabyte and the
  640 KB, and calls `Main` of `main.c`, declared in `vc.h`, which does
  not return. A platform owns its entry and calls `Main`; a test would
  want to call the program and have it come back (`TODO.md`, "What it
  asks of the layer's design"). Group 8.
- **Fields of the main data that the DOS file keeps.** `md.Screen`, where
  the cells are, written by `ScreenMemory`, and `md.ErrDat`, which
  `Ask` clears and reads around every call that is asked with the
  program's errors. Both break principle 7. `ErrDat` goes with question
  9, `Screen` into the DOS file with group 6.
- **DOS's records in the program's data, filled by the system.**
  `struct DTAs` (`md.DTA`, by `FindRecord`), `struct CNTRY`
  (`md.Country`, with DOS's far address of its case map in it), and the
  environment's block read in place by `MnPth0` and `Exec`. `struct EPBs`
  is in the main data still, only as room that the layout keeps: the
  DOS file makes the block now. Each becomes a record of the layer's,
  or a question by name, in its group.
- **The harness's knowledge of the program.** `harness/DRIVER.ASM` knows
  that a step is over by `Idle`'s INT 28h, and reads `AutoTim` and
  `Clock` at their places in the main data. Not the program asking, but
  a test asking the program: `TODO.md` proposes one call that says
  whether the program will do anything by itself. Not in this design
  unless the user wants it.
- **Open Watcom's own.** `#pragma aux ... aborts` on `Main`, `Exit00`,
  `AlocEr` and two more, which clang has as `_Noreturn`: a word that
  each compiler defines, the first of the macros that a platform's
  build defines (question 17).

## 3. The questions that cut across

Each question is short enough to be put in a dialog; the ways are
lettered, the recommendation is the first way, with its reason, and
what follows from each way is behind the question. The answers are to
be written here with their date.

### Q1. In whose values does the layer speak?

> **A (recommended)**: DOS's values where the program decides or shows
> by them (error numbers, attributes, packed dates, key words, screen
> cells), as the layer's own; plain C for sizes, counts, handles, yes or
> no. POSIX maps to them. **B**: neutral values throughout, DOS maps from
> them, the program is rewritten on them. **C**: DOS's values for
> everything, as `sys.h` has today.

Why A: the program's tables, comparisons and screens go by DOS's values
in hundreds of places, and the scenarios judge them; but a width that
only DOS has, `u16` for a count or paragraphs for memory, is no value
the program decides by. What follows: with A, each group says of every
member which kind it is. With B, steps 4 to 6 rewrite much of the
translation, and every rewritten comparison is held by the scenarios
alone. With C, the stub and POSIX carry DOS's widths and leftovers.

### Q2. Paths and drives

> **A (recommended)**: the program keeps DOS's model, drives A: to Z:, a
> current directory for each, backslashes, 67 characters. POSIX shows
> its tree as drive C:, `\` made `/` at the edge. **B**: a neutral path
> model in the layer (root, separator, parent) and the program rewritten
> on it. **C**: A, with POSIX's mounts as further drives.

Why A: the program's paths are logic that the scenarios judge, some 150
places in 16 files that write a backslash, a colon or a drive's letter;
the stub does not run, and how POSIX shows drives can be widened later,
which is C. What follows: with A, `DirChange`, `DirCurrent`,
`DriveSelect` and `DriveCurrent` keep their shape, and the POSIX file
keeps one current directory and one drive. With B, the panels, the
tree, the command line, Enter and copy are rewritten: a deviation of
its own, not a step of the road.

### Q3. Names that DOS cannot have

> **A (recommended)**: decided when POSIX is to run. Until then a search
> answers a record of the layer's with the name and its length, which
> DOS fills with at most 12, and the program copies what fits. **B**:
> POSIX makes short names, `LONGNA~1`, and maps them back, as Windows
> did. **C**: names that do not fit 8.3 are not listed.

Why A: nothing runs on POSIX before step 6, and a record that carries a
name of any length closes no door and costs DOS a copy. What follows:
`FindRecord` and DOS's record of 43 bytes leave the common code; the
program fills `md.DTA` from the layer's record, which keeps the main
data's layout. B is a table of aliases in the POSIX file; C hides files
from the user.

### Q4. Text: code page or Unicode

> **A (recommended)**: all the program's text is bytes of one code page,
> 437 or 866 as DOS has it. POSIX chooses the page at the start and
> converts to and from UTF-8 at the screen, the keys and the names; a
> character that the page lacks is a later decision. **B**: the program
> becomes UTF-8 within. **C**: bytes pass unconverted.

Why A: a cell is a byte and a colour, the capitals, the sort and the
Russian scenarios are the code page's bytes, and conversion at the edge
is what a layer is for. What follows: with A, `CaseMap` and the country
are the code page's, and the POSIX file has the two pages' tables; with
B, every cell, every comparison of letters and the setup file change; C
shows a Russian name as other letters.

### Q5. The screen

> **A (recommended)**: a grid of cells, a character and a colour each,
> that the program puts and reads as today, with a cursor; a platform
> draws it, a terminal by escape sequences in the 16 colours of a PC.
> The BIOS's modes, pages, EGA and font become one question: the rows
> and columns. **B**: a higher-level interface, text and boxes, that the
> program is rewritten on.

Why A: the program draws into its own buffer, `ScrBuf`, and copies
windows of it; a terminal, or a view on iOS, can draw a grid, and a
test reads one. What follows: with A, group 6 takes the BIOS's own
functions out of the common code, and the rows of the screen come from
the system, which entry 12 of `DEVIATIONS.md` waits for. With B, the
drawing of every box, panel and line is rewritten, under the layer.

### Q6. The screen that the commands share

> On DOS the commands write on the program's screen, and the program
> reads it back (`SavScr`; Ctrl-O shows it). A terminal cannot be read.
> **A (recommended)**: POSIX has the panels on the terminal's second
> screen and the commands on the first; reading answers what the
> program last put; Ctrl-O switches screens. **B**: commands run under
> a terminal emulated in the layer, as Midnight Commander's subshell.
> **C**: decided when POSIX runs.

Why A: it is what full-screen programs of a terminal do, at no cost to
DOS; B is a terminal emulator under the layer, thousands of lines that
only POSIX has. What follows: with A, `ScreenGet`, `ScreenCell` and
`ScreenType` stay, and say that another platform answers only what the
program put and writes the console where the commands are; what Ctrl-O
shows on POSIX is a deviation when POSIX runs.

### Q7. Keys and the mouse

> **A (recommended)**: the BIOS's key word, scan code and character, is
> the layer's key; POSIX parses the terminal's sequences into it, and
> answers "none held" for Shift, Ctrl and Alt. The mouse stays in dots,
> eight to a cell. **B**: neutral key events, a key, its modifiers and
> its text, and the program's tables rewritten on them.

Why A: the program's keys are `switch` statements on the BIOS's words in
hundreds of cases, and a terminal tells fewer keys apart than a PC, so
B gains nothing there. What follows: with A, the layer's header names
the key words that it may answer, and a terminal's key without a BIOS
word is not answered; the key bar of a held Shift shows only on DOS.

### Q8. Error numbers

> **A (recommended)**: DOS's numbers, named in the layer's header,
> `SYS_ERR_NO_FILE` 2, `SYS_ERR_NO_ACCESS` 5, `SYS_ERR_NOT_READY` 53h
> and the rest; POSIX maps `errno` to them. **B**: the layer's own
> codes, which DOS maps its numbers to, and the program's comparisons
> rewritten.

Why A: the program's messages and ways go by the numbers, some
forty-five comparisons, with a `#define ERR_` of their own in eleven
files; with A those become the header's names, and nothing changes.
What follows: the POSIX file has a table of `errno` to DOS's numbers,
and where `errno`
does not tell two of DOS's errors apart, a file from a path, it asks
once more (section 5).

### Q9. The program's errors: above the layer or under it

> Most calls are "asked": the box for a drive not ready, the error 53h
> of a call given up, DOS's extended error behind a failure. `dos32.c`
> does that, reading `md.ErrDat`. **A (recommended)**: it goes up into
> the program: the layer has one plain function per thing and one for
> the extended error. **B**: it stays under the layer, which is handed
> the program's flag.

Why A: what is done about a failure is the program's decision, and
under the layer every platform would repeat it; the five Bare twins go,
and DOS is asked the same calls in the same order. What follows: with
A, twenty-three small functions of the program, under today's names,
wrap the layer's (`TODO.md` weighed this at S2, "one function more in
`sys.h`, for AH 59h");
the 59h call is handed 0 in the registers that it does not take, a row
of entry 6. With B, `sys.h`'s two kinds stay, and principle 7 has an
exception.

### Q10. What DOS leaves that callers go on with

> A failed call answers an error, 0 for none. Some callers go on with
> what DOS left where nothing failed, or with what a failure left
> (`FileSeek`'s place). **A (recommended)**: each such thing is named at
> its function, DOS answers as today and others as written there.
> **B**: each goes, a declared deviation that no scenario shows.

Why A: nothing changes that a user could see, and each fix stays the
user's to decide later, as a bug of the original's does. What follows:
the members are renamed when a group is built, so that the compiler
names every line that read them, and each line is looked at; groups 1, 3
and 4 have such things (sections 2 and 5).

### Q11. Time and dates

> **A (recommended)**: DOS's forms: a file's date and time packed as DOS
> packs them, the time of day to the hundredth, the timer's 18.2 ticks a
> second from midnight. POSIX converts from `time_t` and a monotonic
> clock. **B**: neutral: seconds since 1970 for a file, milliseconds
> for the timer, the program packing and counting for itself.

Why A: the panels keep, sort and show packed stamps, a copy hands them
on, and the blanker and the waits count ticks, all judged by the
scenarios. What follows: with A, a POSIX date before 1980 or after 2107,
and a time's odd second, are a later decision; the ticks wrap at
midnight on every platform, as on DOS.

### Q12. Other programs, the shell, the environment

> **A (recommended)**: the layer runs a file with its arguments as one
> text, and answers by name what the program reads of its start: its
> arguments, a variable, the path of its own file, the shell and its
> switch (`COMMAND.COM /C`, `sh -c`). DOS's tail, environment block and
> EPB stay in the DOS file. **B**: as today: DOS's blocks cross, and
> POSIX makes them.

Why A: DOS's layouts are not the program's to know, and the program
reads only a few things of them, which the DOS file answers byte for
byte, the 68 bytes of the path that `MnPth0` reads included. What
follows: with A, `Exec` asks for the shell and its switch where it
reads `COMSPEC`; iOS answers that it runs nothing. With B, POSIX builds
DOS's environment block for the program to read.

### Q13. Memory

> **A (recommended)**: blocks as pointers, sized in bytes (`i32`): take,
> give back, resize, and how large a block can be. DOS asks the extender
> (F9); POSIX uses `malloc` and `realloc`, and answers for "how large"
> a figure that it sets when it starts. **B**: one arena from the layer
> at the start, the program's own allocator handing out blocks in it.

Why A: the program's room is "the largest block there is", which a
platform has to say; blocks of their own let a sanitizer see one panel
run into the next. What follows: group 7 waits for F9, which gives the
three functions that shape; with B, the 640 KB of step 1 come back as
the program's code.

### Q14. What only DOS has

> **A (recommended)**: each stays a function of the layer that another
> platform answers with the value that means none: no unit shared, no
> CD-ROM, version new enough, nothing to set. A question "is it there"
> only where the program does otherwise without it. **B**: each goes
> into the DOS file, called from a hook at the start or around a
> command.

Why A: the program's rows of calls stay as they are, so DOS is asked in
the same order; B moves DOS's calls, which `calls.asm` would show, and a
hook decides in the platform. What follows: with A, the layer has some
thirty functions that POSIX makes empty or answers with none; each group
may still take one into the DOS file where nothing in the common code
needs it, as `ScreenMemory`.

### Q15. What the system brings in

> **A (recommended)**: the device error stays a function of the
> program's that the layer calls in the middle of a call it was asked,
> and POSIX never calls; Ctrl-Break and Ctrl-C are taken as today
> (POSIX: the terminal raw, `SIGINT` ignored); nothing else arrives
> unasked. **B**: events in a queue that the program reads with the
> keys.

Why A: it is today's shape, it works for DOS and for a terminal, and a
test can call the program's function. What follows: with A, a
terminal's new size or its loss is answered later as a key, and
`IntOn` stays a function that only DOS fills. With B, the box of the
critical error, which DOS waits for in the middle of its call, cannot be
a queued event on DOS.

### Q16. The types that cross

> **A (recommended)**: the size of a file, a place and a step in one,
> the room of a disk: `i64` on every platform (signed, and 32 bits with
> a sign hold 2 GB where DOS's files reach 4), so no name of their own.
> A count of bytes per call, a size in memory, a handle: `i32`. Yes or
> no: `bool`. **B**: `FileSize`, `i64` on POSIX and `i32` on DOS, files
> of 2 GB or more a deviation there.

Why A: by the user's rule a type is named only where its width differs,
and signed sizes make the width the same everywhere. What follows: with
A, Open Watcom multiplies and divides 64 bits by helper routines, which
the tree has no C library for: they are written and tested as `__U4M`
and `__U4D` were for 16 bits, and the panel needs room for more digits
only where a size passes ten. With B, the one named type, and sizes
past 2 GB refused on DOS.

### Q17. The files of the layer

> **A (recommended)**: a header for each group, `sys_file.h`,
> `sys_dir.h` and so on, with `sys.h` including them; for each group a
> file of each platform, `sys_file_dos.c`, `sys_file_posix.c`,
> `sys_file_stub.c`, chosen by the build; `sys_dos.h` shared by the DOS
> files alone; the platform's macros in `sys_config_dos.h` and the like.
> **B**: one header, one file per platform: `sys_dos.c`, `sys_posix.c`,
> `sys_stub.c`.

Why A: it is Chromium's way, which the road names, and a group is
designed, approved and moved on its own. What follows: with A, step 4
cuts `dos32.c` into a dozen files, its interrupts and real mode in
`sys_dos.c` behind `sys_dos.h`, and `CMakeLists.txt` takes the files of a
platform by their suffix; the macros are few: `NORETURN`, and one per
platform, `SYS_DOS`, `SYS_POSIX`, `SYS_STUB`. With B, `dos32.c` is
renamed and keeps its 2,000 lines.

### Q18. The names of the layer

> **A (recommended)**: every function, type and constant of the layer
> begins with `Sys` or `SYS_`: `SysFileOpen`, `struct SysOpened`,
> `SYS_ERR_NO_FILE`. The program's wrappers of question 9 keep today's
> names, so its callers do not change. **B**: today's names for the
> layer, and the wrappers renamed.

Why A: a call that leaves the program is seen to, and the program's
364 calls of `sys.h` keep their names, which the history and the notes use.
What follows: `harness/reads.py`'s `NAMED`, which knows the functions
that take a name, is told the new names; `harness/ported.py` finds a
function under its name in the map.

## 4. The order of the groups

To be designed with the user one at a time, from the group that teaches
the most of the questions above at the least risk:

1. **Files by their handle**, with the error model: create, open,
   close, read, write, seek, and the extended error. It settles
   questions 8, 9, 10, 16, 17 and 18, which every later group uses; it
   needs no answer on paths, names or text, a name being lent and passed
   on; and it is the most called group, so a wrong shape shows early,
   while its DOS functions are among the plainest of `dos32.c`.
2. **The time, the country and idle**: `TimeOfDay`, `Ticks`, `Idle`,
   `FileTime`, `FileSetTime`, `CountryInfo`, `CaseMap`. Small; settles
   question 11, the country's record, and what `Idle` is to a POSIX
   program and to a test.
3. **Names and directories**: the search and its record, attributes,
   delete, rename, make, remove, the current directory, the true name.
   Questions 2, 3 and 4, the largest of the model.
4. **Drives**: current, selected, their room, the shared unit, their
   kind. Rests on 3; question 14 met first.
5. **Keys and the mouse**. Question 7.
6. **The screen and the console**. Questions 5 and 6; entry 12 of
   `DEVIATIONS.md` waits for the rows.
7. **Memory**, when F9 has landed. Question 13.
8. **Other programs, the start and the end**: running, the shell, the
   arguments, the environment, the entry, the end. Question 12.
9. **What the system brings in**: the device error, Ctrl-Break, Ctrl-C,
   the end by another's hand, `IntOn`. Question 15.
10. **What is left of DOS's own**: the FCB, the version, the Ctrl-C
    check, how DOS looks for memory, `MemOwn`. Question 14, item by
    item.

## 5. The first group, proposed in full: files by their handle

Chosen by section 4: it carries the error model and the types, which
every other group takes, and asks nothing of the path model.

What it covers of `sys.h` today: `FileCreate`, `FileOpen`,
`FileOpenBare`, `FileClose`, `FileCloseBare`, `FileRead`,
`FileReadBare`, `FileWrite` and `FileSeek`, nine functions, and the
part of `Ask` of `dos32.c` that is the extended error. Their calls: 80 in 14
files; the section above has them by function and by file.

Piece S of step 2 (`TODO.md`, "The pieces") changes these first: counts
become `i32`, read and write answer a structure of their own, the step
of a seek an `i32`. The proposal is written for where S goes, and goes
one step further only where the layer's types do, the place of a file.

It is written as if questions 1, 8, 9, 10, 16, 17 and 18 were answered
as recommended. Where an answer goes another way, what changes is said
at the end.

### The header, `sys_file.h`

```c
/*
 * Files, by the handle that opening one gives. A name is a path that
 * ends in a zero byte, in the program's code page and with its drive
 * and backslashes (see sys_dir.h); it is lent for the call. A buffer
 * is the caller's, and the layer keeps no pointer to it.
 *
 * A function that may fail answers with `error`: 0 if it did not fail,
 * else one of SYS_ERR_ of sys.h, the number that DOS has for it. What
 * it answers besides is defined only where `error` is 0, unless its
 * comment says otherwise. Where the program wants DOS's extended error
 * behind a failure, it asks SysLastError.
 */

/* A handle: what opening a file answers, and what the others take. */
/* (an i32; see question G2) */

/* How a file is opened: the access in the low bits, and what others
   may do meanwhile, as DOS has them. A platform without sharing goes
   by the access alone. */
#define SYS_OPEN_READ        0x00
#define SYS_OPEN_WRITE       0x01
#define SYS_OPEN_BOTH        0x02
#define SYS_SHARE_DENY_WRITE 0x20
#define SYS_SHARE_DENY_NONE  0x40

/* From where a seek counts. */
#define SYS_FROM_START 0
#define SYS_FROM_HERE  1
#define SYS_FROM_END   2

struct SysOpened {
    i32 error;
    i32 handle;
};

struct SysMoved {
    i32 error;
    i32 count;      /* how many bytes were read or written */
};

struct SysPlace {
    i32 error;
    i64 place;      /* counted from the start of the file; if it
                       failed, see SysFileSeek */
};

/* Make a file with these attributes (SYS_ATTR_ of sys_dir.h), or make
   the one there is empty, and open it to read and write. */
struct SysOpened SysFileCreate(const u8 *name, u32 attributes);

/* Open a file: `mode` as above. */
struct SysOpened SysFileOpen(const u8 *name, u32 mode);

/* Close a file. Answers the error, 0 if there was none. */
i32 SysFileClose(i32 handle);

/* Read up to `count` bytes to `to`, from where the file is at: fewer at
   its end, 0 there. */
struct SysMoved SysFileRead(i32 handle, u8 *to, i32 count);

/* Write `count` bytes from `from` at where the file is at: fewer on a
   disk that is full, which is no error. A count of 0 cuts the file off
   there. */
struct SysMoved SysFileWrite(i32 handle, const u8 *from, i32 count);

/* Go to `step` bytes from the start, from where the file is at, or from
   its end; a step from where it is or from its end may be below 0.
   Answers the place then. If it failed, `place` is what the platform
   says here, which on DOS is the error in the low 16 bits and the low
   16 bits of `step` above them: see question G1. */
struct SysPlace SysFileSeek(i32 handle, i32 from, i64 step);

/* The system's own account of the last call that failed, DOS's extended
   error: a SYS_ERR_. Asked behind a failure, before anything else is
   asked of the system. */
i32 SysLastError(void);
```

`SYS_ERR_` and the attributes are in `sys.h` and `sys_dir.h`, of
groups that come later; this group names the errors that its callers
compare: `SYS_ERR_NO_FILE` 02h, `SYS_ERR_NO_PATH` 03h,
`SYS_ERR_NO_ACCESS` 05h, `SYS_ERR_SHARED` 20h, `SYS_ERR_NOT_READY` 53h,
with the rest of DOS's list beside them.

### What the program keeps above it: the asked kind

In `system.c`, beside `OpenFil`, the program's own functions under
today's names, which do what `Ask` of `dos32.c` does today:

```c
/* What a call that is asked with the program's errors answers: Intr21 of
   the original. A device that FtErr1 was let fail makes it the error
   53h; another failure is asked about, and DOS's extended error comes
   back. `value` is the error if it failed, as the callers read it. The
   members are as wide as piece S leaves them. */
static struct Answer Asked(i32 error, i32 value)
{
    struct Answer answer;

    if (md.ErrDat != 0) {
        answer.value = SYS_ERR_NOT_READY;
        answer.failed = 1;
    } else if (error != 0) {
        answer.value = SysLastError();
        answer.failed = 1;
    } else {
        answer.value = value;
        answer.failed = 0;
    }
    return answer;
}

struct Answer FileOpen(const u8 *name, u16 mode)
{
    struct SysOpened opened;

    md.ErrDat = 0;
    opened = SysFileOpen(name, mode);
    return Asked(opened.error, opened.handle);
}
```

and so `FileCreate`, `FileClose`, `FileRead`, `FileWrite` and
`FileSeek`, each a few lines. `struct Answer` becomes the program's,
declared in `vc.h`; its callers do not change, and S's structures for
the counts are the program's in the same way. The three Bare calls of
`start.c` call the layer itself, with a line each that makes the
answer the program's.

This is the same calls of DOS in the same order: the call, and behind a
failure that no device error has given up, 59h. One thing is handed
otherwise: today 59h gets CX, DX, SI and DI as the failed call left
them, and `SysLastError` hands 0 there, as every function does in a
register that its call does not take. That is a row of entry 6, which
the user decided as a kind on 13 October 2026.

### What each function does, on each platform

| Function | DOS, from `dos32.c` today | POSIX | The stub |
|---|---|---|---|
| `SysFileCreate` | DOS 3Ch, CX the attributes, the name translated by the extender | `open(name, O_RDWR \| O_CREAT \| O_TRUNC, mode)`, `mode` 0444 for the read-only bit and 0666 else, less the `umask`; the other bits have no meaning there; attribute 8, a label, is `SYS_ERR_NO_ACCESS` | error 1, handle -1 |
| `SysFileOpen` | DOS 3Dh, AL the mode | `open` with `O_RDONLY`, `O_WRONLY` or `O_RDWR` by the low bits; the sharing bits are no question there, so 20h never comes | as above |
| `SysFileClose` | DOS 3Eh | `close` | error 1 |
| `SysFileRead` | DOS 3Fh, as many calls as a count needs, each of at most what DOS takes (S) | `read`, again on `EINTR`, as many times as a count needs, until the end | error 1, count 0 |
| `SysFileWrite` | DOS 40h, likewise; a count of 0 is one call of 0 | `write`, likewise; a write cut short by a full disk answers the count and no error; a count of 0 is `ftruncate` at `lseek(handle, 0, SEEK_CUR)` | as above |
| `SysFileSeek` | DOS 42h, AL `from`, CX:DX the low 32 bits of `step`; place DX:AX | `lseek` with `SEEK_SET`, `SEEK_CUR` or `SEEK_END`, in 64 bits; a place below 0 is `SYS_ERR_` 1, with `place` -1 | error 1, place -1 |
| `SysLastError` | DOS 59h, BX 0; AX | the `errno` of the last call that failed, by the table below, kept by each function where it failed | 1 |

Where `errno` does not say what DOS would, the POSIX file asks once more:
`ENOENT` is `SYS_ERR_NO_PATH` 03h if a directory of the path is missing,
which a `stat` of the directory part tells, and `SYS_ERR_NO_FILE` 02h if
not. The rest, a proposal held by a test of the POSIX file when there is
one: `EMFILE` and `ENFILE` 04h; `EACCES`, `EPERM`, `EROFS`, `EISDIR`
05h; `EBADF` 06h; `ENOMEM` 08h; `EBUSY`, `ETXTBSY` 20h; `ENOSPC` 27h;
`EEXIST` 50h; `EIO` 1Fh; anything else 1Fh, the general failure.

### Which callers change, and how much

- With question 9 answered A: the files that call these nine change
  nothing at their calls, but for `start.c`, three calls of the Bare
  ones. `system.c` has six functions more, and `dos32.c` loses nine for
  the six of `sys_file_dos.c` and `SysLastError`. `Ask` stays under the
  functions of the groups still to come, which are asked with the
  program's errors, and goes with the last of them.
- What a caller reads of an answer stays: `value` is the handle or the
  count, or the error where `failed` is set, as today. Two things are
  looked at by reading every caller, since nothing else finds them,
  and were for this draft:
  - `value` of a close where nothing failed: no caller reads it. The
    one that went by it, `ReDr00`, goes by the read's answer since the
    user's decision of entry 6. `dirfunc.c` keeps a close's `value` in
    `error`, which is read only on the way where the close failed.
  - The place of a failed seek: `view.c` holds it against the place it
    asked for, twice, and `copy.c` takes it for a size, three times,
    without looking at the error. The DOS file answers what DOS left,
    as today; question G1.
- `md.ErrDat` is no longer read or written by the DOS file; the
  program's `FtErr1` sets it and its `Asked` reads it, as in the
  original, where `Intr21` and the box are of one program.

### The questions on it

- **G1. A failed seek's place.** `view.c` and `copy.c` go on with it
  without looking at the error. A, recommended: DOS answers what DOS
  left, the error and the step's low 16 bits, and another platform -1;
  the callers stay. B: every platform answers -1, a deviation that no
  scenario shows, since a seek on an open file fails only for a bad
  handle.
- **G2. The handle.** A, recommended: an `i32`, which the program keeps
  in its 16-bit fields with a cast until M widens them. B: a structure
  of one `i32`, `struct SysFile`, which the compiler holds apart from a
  count; every field and variable that keeps a handle changes, some
  twenty.
- **G3. The mode of opening.** A, recommended: DOS's mode byte with its
  sharing bits is the layer's, named; POSIX goes by the access. B: the
  access alone, and the sharing a DOS file's matter, which changes what
  DOS is handed and `OpenFil`'s second try: a deviation.
- **G4. A write of no bytes.** A, recommended: it cuts the file on every
  platform, as DOS has it, since a count that the program works out may
  be 0. B: a function of its own, `SysFileCut`, for the callers that
  mean it, and a write of 0 does nothing; each caller is then read for
  whether its count can be 0.
- **G5. The extended error.** A, recommended: `SysLastError` is asked
  behind a failure, as DOS's 59h is, and POSIX keeps the last `errno`
  for it. B: every function answers the extended error itself; DOS
  would then ask 59h behind the failures of today's Bare calls too,
  calls that the original does not make.

If question 9 is answered B, the layer keeps the asked and the bare
kinds, nine functions with `Ask` under them, and is handed `&md.ErrDat`
by `DeviceErrorTake`; G5 falls away. If question 16 is answered B, the
place and the step are `FileSize`. If question 18 is answered B, the
layer's functions take today's names and the wrappers of `system.c`
others. If question 17 is answered B, the header is `sys.h` and the
functions stay in `dos32.c`.

## What this draft is unsure of

- Whether FreeDOS answers 59h with the same number for a call of the
  extender's as for one made in real mode: `dos32.c` asks it for both
  today, and the toys did not look at it.
- The table of `errno` to DOS's numbers: written from what each means,
  not tried.
- That nothing but `view.c` and `copy.c` goes on with what a failed
  call of this group left: read in the callers, not held by a tool.
- That `wcc386` needs helper routines for numbers of 64 bits only to
  multiply, divide and take a remainder, and does the rest in line, as
  it did for 32 bits in 16: known of the compiler, not tried here.
- The counts of places in section 3, which are `grep`'s, not read.
- Whether a terminal's second screen does for Ctrl-O as users of the
  original expect: question 6 is a guess at what is wanted.
