# The layer: what the program asks of the system, and how

**Being redone from the platforms' side. Nothing in it is approved but
what "Decided" says, with the date.**

This is step 3 of the road (`CLAUDE.md`, "The portable C"): the design
of the abstraction layer, made with the user. A first draft was begun on
13 October 2026 from what the program asks today, `sys.h` and its
callers. The user judged that approach wrong the same day: a pragmatic
platform layer is designed from what the target platforms provide, area
by area, finding the common ground between them, and not from what one
program asks of one system. `fopen`, for example, is no abstraction for
this program, since it reaches only local files, and the user wants
iCloud and other file systems on the iPhone. The design is being made
anew that way, beginning with file systems.

What stays of the first draft: the user's answers, below, and what the
program needs, the inventory of `sys.h` and of what the program asks
outside it, with the counts and findings that went with it. What does
not stay is said in the last section, "Superseded: the first draft".

Running programs and the program's life are surveyed in `layer/programs.md`.
Language, text and time are surveyed in `layer/i18n.md`.
The screen and input are surveyed in `layer/console.md`.

## Decided

What the user has decided, in the user's words, with the date. Nothing
else in this document is decided.

- **The values of the layer** (question 1 of the first draft), 13
  October 2026: "The layer should speak standard C, unless Watcom C
  support is bad, then I would design our own, platform-agnostic
  vocabulary." No value of DOS's is the layer's: not its error numbers,
  its attributes, its packed dates, its key words or its cells. What
  Open Watcom gives of standard C is under "What Open Watcom gives".
- **Paths and drives** (question 2 of the first draft), 13 October 2026:
  "Drives survive, but their meaning will be platform-specific. For
  example, DOS can continue exposing A: and C:, but iPhone
  implementation will have ⛁: for local files and ☁️: for iCloud.
  Slashes should be platform-native: back on DOS and Windows, forward
  on mac and ios." A drive is a name that the platform gives, which may
  be a character beyond ASCII; the separator is the platform's.
- **Text** (question 4 of the first draft), 13 October 2026: "UTF-8
  inside."
- **File systems**, questions 1 to 4 of "Area: file systems", 13
  October 2026: paths under named roots; no Android, but archives and
  FTP some day; the drives of Linux, macOS and iOS. The answers are
  under "Decided in this area" there.
- **File systems**, questions 5 to 15 of "Area: file systems", and one
  that followed from 6, 13 and 14 October 2026: a Mac build outside the
  sandbox; F8 to the trash and Shift-F8 deleting outright; the
  platform's copy where it has one; files not downloaded listed with a
  mark and downloaded behind a box; the platform's attributes; the
  layer's own time and errors; directories watched; the program's loop
  on a thread of its own; names that are not UTF-8 kept as bytes; and
  the setup file one format of the program's own everywhere, DOS
  included, against the recommendation. The answers are under "Decided
  in this area" there.
- **Running programs and the program's life**, the nine questions of
  `layer/programs.md`, 13 and 14 October 2026: no command line on iOS;
  Vibe Commander a terminal program on Windows, Linux and macOS, with no
  native app on macOS, and a view of its own on iOS; a command's output
  on the real terminal first, and the panels over it later, by a shadow
  grid that libvterm keeps; the user's shell; the program's files in
  the platform's place for a user's settings, `VC` still overriding;
  Enter on a file that the extension file has no line for handed to the
  platform's opener; the box of a device that fails kept for what
  waiting can cure; the preferences and the session saved by the
  program itself, on every platform, DOS included, with "Save setup"
  and "Auto save setup" gone; Ctrl-Z a key in a terminal. The answers
  are under "Decided in this area" there; how the saving sits with
  decision 15 of the file systems is under "Decided in this area" of
  "Area: file systems" below.
- **Language, text and time**, the ten questions of `layer/i18n.md`, 14
  October 2026: the layer reports the country's facts and the program
  writes dates, times and numbers in its columns, DOS unchanged; a
  panel's names in the order of the platform's file manager off DOS;
  capitals beyond ASCII by each platform's own call off DOS, against
  the recommendation of a pinned table, DOS still asked for every byte;
  the code page asked of DOS once at the start, 6601h, a deviation; the
  program in English only for now; conventions from the environment on
  Linux and from the user's settings on macOS and iOS; a character's
  width answered by the layer; the ticks starting again at midnight on
  DOS as the original's; the setup's Country info kept off DOS without
  the code page's table; no Unicode library. The answers are under
  "Decided in this area" there.

Each of these stands in the design that is being made anew.

## What the program needs: the inventory

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

The column "Goes to" was written for the first draft and is kept as
what it was then, a proposal of that draft; the other columns are facts
of the program at 32827fb.

### Files

All asked with the program's errors but the three Bare ones.

| Function | Calls | What the program uses it for | Everywhere | Of DOS alone, and what a user sees | Goes to |
|---|---|---|---|---|---|
| `FileCreate` | 7 (stmain 1, copy 1, dirfunc 1, usermenu 1, edit 1, setup 1, treebox 1) | the setup file, a copy's target, the editor's file, the tree's file, the menu file, a label (attribute 8, `F54_00`) | yes | a label made by creating a file with attribute 8 | layer; the label to "Drives" |
| `FileOpen` | 4 (system 2, stmain 1, sthelp 1) | `OpenFil`, with its second try without sharing; the setup's help and file | yes | the sharing bits of the mode | layer |
| `FileOpenBare` | 1 (start 1) | the setup file at the start | yes | | layer, as `FileOpen` |
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
| `FcbParse` | 2 (dirfunc 2) | a name made an FCB for deleting by a mask and for the label | no | all of it: deleting every file of a mask at one call, `F38_70`, and the label, `F54_00` | layer, none elsewhere; or a "delete by mask" and a "label" of the layer whose DOS file makes the FCB |
| `FcbDelete` | 3 (dirfunc 3) | the same | no | as above, with the carry flag that DOS leaves (`flagged`) | as above |

### Directories

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `FindRecord` | 1 (system 1) | `SetDTA`: where a search puts what it finds, `md.DTA`, DOS's record | the state, yes | DOS's record of 43 bytes, kept by the system between two calls | layer, in another shape: a search answers its record |
| `FindFirst` | 10 (copy 1, dirfunc 3, read 2, treebox 1, view 1, find 1, enter 1) | reading a panel, the tree, the search, a copy, the labels, the viewer's next file | yes, `opendir` and a match | 8.3 names, the attributes as a filter, a label as attribute 8 | layer |
| `FindNext` | 6 (find 1, read 1, treebox 1, dirfunc 3) | the same | yes | | layer |
| `DirMake` | 3 (copy 2, dirfunc 1) | F7, a copy of a directory | yes | | layer |
| `DirRemove` | 4 (copy 2, dirfunc 2) | delete, move | yes | | layer |
| `DirChange` | 11 (copy 1, dirfunc 3, commands 1, enter 3, drive 1, find 1, keys 1) | a panel's directory made current, Enter, the tree, a command run there | yes, one for the process | one for each drive | layer |
| `DirCurrent` | 7 (find 1, drive 1, system 1, dirfunc 1, edit 1, main 1, read 1) | the path of a panel, of the editor's file, of the program's | yes | one for each drive, without its drive and first backslash | layer |
| `DirCurrentBare` | 2 (exec 1, start 1) | the prompt between commands; the start | yes | | layer, as `DirCurrent` |

### Drives

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DriveCurrent` | 7 (stmain 1, find 1, system 3, start 1, exec 1) | `TstDrv`, whether a drive exists; `SetPth`; the prompt; the search | as one drive | all drives | layer |
| `DriveSelect` | 10 (drive 1, keys 2, commands 1, copy 2, find 1, main 1, system 2) | `TstDrv`; a panel's drive made current; Go to | as one drive | | layer |
| `DriveSpace` | 2 (copy 1, read 1) | a copy's room; the Info panel's free space | yes, `statvfs` | in clusters and sectors, with `flagged` | layer, in bytes |
| `DriveLetter` | 2 (drive 1, system 1) | A: and B: on one floppy drive: the drive menu, the prompt "Insert disk for drive B:" (`Fantom`) | no | the prompt and its setting, kept while DOS is a target (user, 13 October 2026) | layer, none elsewhere: "shares no unit" |
| `DriveSetLetter` | 5 (drive 1, system 3, main 1) | the same, and the box of a drive not ready | no | as above | layer, none elsewhere |
| `DriveRemote` | 1 (find 1) | which drives the file search goes through for "all drives" | as a mount's kind | | layer, or one function "what kind of drive" |
| `DriveFixed` | 1 (find 1) | the same | as above | | as above |
| `DriveCd` | 1 (find 1) | the same, by MSCDEX | as above | the CD-ROM's own question, INT 2Fh | as above |

### Memory

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MemTake` | 5 (start 5) | the panels and the room, at the start and after a command | yes | paragraphs and a segment until F9; "how large a block can be" | layer, in bytes and pointers |
| `MemGive` | 1 (start 1) | the room given back before a command | yes | | layer |
| `MemResize` | 3 (view 3) | the room cut and grown around a viewer of another's | yes | | layer |
| `MemSetSearch` | 1 (exec 1) | `SetStat`: DOS to look for the lowest block, as it does when it starts | no | none on the screen: where DOS puts a command | layer, none elsewhere, or DOS file |
| `MemSetUpper` | 1 (exec 1) | the same, the upper memory out | no | as above | as above |
| `MemOwn` | 1 (exec 1) | `Alloc3`: the program's block made as long as it is | no | "not enough memory" at the start | as above |

### The time, the country, the system itself

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `TimeOfDay` | 6 (input 4, main 1, mouse 1) | the clock on the screen, the times of a key, a double click | yes | to the hundredth, packed in bytes | layer |
| `CountryInfo` | 1 (start 1) | how dates, times and numbers are written: `md.Country` | yes, from the locale, mapped | DOS's record, with a far address of its case map in it | layer, in a record of the layer's own |
| `CaseMap` | 2 (start 1, system 1) | the table of capitals of 80h to FFh at the start; `UpCase`, every byte of a search for a text | yes, by the code page | DOS's routine through the extender, slow (user, 13 October 2026) | layer, the code page's capitals |
| `SystemVersion` | 2 (start 1, exec 1) | `Alloc3`: refuse a DOS before 3; the version noted | no | the message and the end on an old DOS | layer, none elsewhere: "new enough" |
| `SystemTrueVersion` | 1 (start 1) | `VerDOS` noted, which nothing in the portable C reads | no | nothing | DOS file, or gone with `VerDOS` (a deviation of order, to ask) |
| `BreakCheck` | 1 (exec 1) | `SetBrk`: what DOS was set to, put back at the end | no | Ctrl-C checked at every call | layer, none elsewhere |
| `SetBreakCheck` | 3 (exec 3) | the same, around commands | no | as above | as above |
| `Idle` | 1 (blank 1) | each time the program looks for a key and finds none: INT 28h | yes, as a short wait | other programs of DOS get a turn; the harness's driver knows a step is over | layer: the one place where a POSIX program sleeps |

### Other programs, and the end of this one

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ProgramRun` | 1 (view 1) | `Aexec0`: a viewer or an editor of another's | not on iOS | DOS's tail: a count byte, the text, a carriage return | layer |
| `ProgramRunBare` | 1 (exec 1) | `Exec`: COMMAND.COM for the commands | as above | COMMAND.COM and `/C` built by the program | layer, as `ProgramRun` |
| `ProgramEnd` | 3 (exec 3) | the program leaves | yes | | layer |
| `ProgramEndOld` | 1 (exec 1) | the end on a DOS before 3 | no | | layer, none elsewhere |
| `ProgramTail` | 1 (main 1) | the path on the command line that the program was started with | yes, `argv` | the tail as DOS keeps it | layer, as the arguments |
| `ProgramEnvironment` | 3 (exec 1, start 2) | `COMSPEC`; the program's own path behind the environment, read as 68 bytes by `MnPth0`, for where its files are | yes, `environ` and the path of the program | DOS's block, and the path behind it | layer, by name: a variable, the program's path |

### What the system asks the program

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DeviceErrorFn` | type | `FtErr1`, the box with Retry and Abort | no | the critical error: a drive without a disk, a disk that is protected | layer: the program's function, which another platform never calls |
| `DeviceErrorNote`, `DeviceErrorTake`, `DeviceErrorGive` | 1 (main 1), 1 (main 1), 1 (main 1) | `HookVct` and the end | no | | layer, none elsewhere |
| `IntOn` | 3 (box 1, window 2) | behind what goes to the screen: the box of the critical error gets its keys | no | | layer, none elsewhere, or DOS file |
| `CtrlBreakNote`, `CtrlBreakTake`, `CtrlBreakGive` | 1 (main 1), 1 (main 1), 1 (main 1) | Ctrl-Break taken, so that nothing happens for it: the key 0 that it leaves is read as a stop by `TstBrk` | as SIGINT, in part | the key of a PC | layer |
| `CtrlCTake` | 1 (exec 1) | `SetBrk` | as SIGINT | | layer |
| `ProgramEndedBreakCheck`, `ProgramEndedTake`, `ProgramEndedGive` | 1 (exec 1), 1 (exec 1), 1 (exec 1) | DOS ending the program by another's hand: what DOS is to put back | no | the scenario `ended` | layer, none elsewhere; or DOS file entirely, said by `SetBreakCheck` |

### The console

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ConsoleChar` | 1 (start 1) | the space that DOS writes at the start | yes, as the terminal before the screen is taken | | layer, with the screen |
| `ConsoleText` | 2 (start 1, stmain 1) | the banner at the start; the setup's line for a file not written | as above | ended by `$` | as above, ended by a zero |

### The keyboard

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `KeyWaiting` | 6 (blank 1, find 1, input 2, system 2) | `Input0`, `TstBrk`, `ClrKbd`, the blanker, the search | yes, by parsing the terminal | the BIOS's key word | layer |
| `KeyRead` | 8 (blank 1, find 1, exec 1, input 2, start 1, system 2) | the same | yes | | layer |
| `KeysHeld` | 6 (blank 2, menu 1, bars 1, input 2) | the key bar of Shift, Ctrl and Alt held; the blanker; the menu | not in a terminal | the bits of the BIOS's data | layer, none elsewhere: "none held" |
| `KeyForgetIns` | 1 (keys 1) | Ins held down selects file after file | no | the BIOS's Ins bit | layer, none elsewhere |

### The screen

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ScreenState` | 11 (window 4, blank 3, main 1, stmain 1, start 2) | the mode, the columns, the page shown | as "colour or not" and a size | modes and pages | layer, as the screen's size and kind |
| `ScreenSetMode` | 2 (start 2) | the start: mode 3 if the mode is not of text | no | | DOS file, or none elsewhere |
| `CursorWhere` | 8 (stmain 1, blank 1, main 1, start 2, view 1, window 2) | where the command line goes, the cursor kept and put back | yes | the shape in lines of a character | layer |
| `CursorTo` | 3 (blank 2, window 1) | the cursor of an input line; the blanker | yes | | layer |
| `CursorShape` | 8 (stcolor 1, stmain 1, blank 2, start 2, window 2) | hidden, a line or a block (Ins) | in part: shown or not, a bar or a block | lines of the character | layer |
| `ScreenScroll` | 2 (blank 1, window 1) | `Scroll` of a window by a line; the blanker clears | yes | the BIOS's call 6 and 7 | layer, or the program's own on its buffer |
| `ScreenCell` | 2 (start 2) | the colour at the cursor when the program starts, `CrntCol` | only of what the program put | reading DOS's screen | layer |
| `ScreenFill` | 2 (blank 1, start 1) | a star of the blanker; the cell put back at the start | yes | | layer, or `ScreenPut` |
| `ScreenType` | 13 (exec 3, stmain 2, enter 2, main 1, start 4, system 1) | the command line, the prompt, the line feeds between commands, the bell | yes, as a terminal's output | the BIOS's teletype, with a page and a colour as they happen to be | layer |
| `ScreenEga` | 1 (start 1) | whether the display may have more than 25 rows | no | | layer, as the rows of the screen |
| `ScreenFont` | 1 (start 1) | how many rows the display has | no | the BIOS's font | as above |
| `ScreenPageStart` | 1 (window 1) | `ScrOffs`: where the cells of the page are | no | | DOS file |
| `ScreenMemory` | 1 (window 1) | the same | no | `md.Screen`, kept by `dos32.c` in the main data | DOS file |
| `ScreenPut` | 4 (window 4) | `Window`, `MovScr`: the program's buffer to the screen | yes | the display's memory | layer |
| `ScreenGet` | 1 (window 1) | `SavScr`: the screen read back after a command | only of what the program put | the screen that DOS and the commands share | layer |

### The mouse

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MouseFind`, `MouseReset` | 2 (mouse 1, stmain 1), 2 (mouse 1, stmain 1) | whether there is a mouse; the setup sets it anew | yes, in a terminal that reports one | the driver's answers, 21h for an old one | layer |
| `MouseShow`, `MouseHide` | 1 (mouse 1), 1 (mouse 1) | the pointer off while the screen is written | yes, as nothing to do where the terminal draws it | a count of hides | layer |
| `MouseWhere` | 2 (mouse 2) | the buttons and the place, polled | yes | in dots, 8 to a cell | layer |
| `MouseTo`, `MouseRows` | 1 (mouse 1), 2 (mouse 1, stmain 1) | the pointer put in the middle; the rows it may go to | not in a terminal | | layer, none elsewhere |
| `MousePointer` | 2 (mouse 2) | the pointer's look, by the colours it turns | not in a terminal | | layer, none elsewhere |

### The timer

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `Ticks` | 1 (system 1) | `Timer`: the blanker, a box that grows, the waits, double clicks | yes, from a monotonic clock | 18.2 a second, from midnight, read in the BIOS's data | layer |

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
  asks of the layer's design").
- **Fields of the main data that the DOS file keeps.** `md.Screen`, where
  the cells are, written by `ScreenMemory`, and `md.ErrDat`, which
  `Ask` clears and reads around every call that is asked with the
  program's errors. Both are the program's data, which the system's
  file reads or writes.
- **DOS's records in the program's data, filled by the system.**
  `struct DTAs` (`md.DTA`, by `FindRecord`), `struct CNTRY`
  (`md.Country`, with DOS's far address of its case map in it), and the
  environment's block read in place by `MnPth0` and `Exec`. `struct EPBs`
  is in the main data still, only as room that the layout keeps: the
  DOS file makes the block now.
- **The harness's knowledge of the program.** `harness/DRIVER.ASM` knows
  that a step is over by `Idle`'s INT 28h, and reads `AutoTim` and
  `Clock` at their places in the main data. Not the program asking, but
  a test asking the program: `TODO.md` proposes one call that says
  whether the program will do anything by itself. Not in this design
  unless the user wants it.
- **Open Watcom's own.** `#pragma aux ... aborts` on `Main`, `Exit00`,
  `AlocEr` and two more, which clang has as `_Noreturn`: a word that
  each compiler defines, the first of the macros that a platform's
  build defines.

## What Open Watcom gives

The user's condition for a vocabulary of our own is that Open Watcom's
support of standard C is bad. This is what it is, for the program of 32
bits under DOS/32A, found on 13 October 2026 in the pinned Open Watcom
(its headers, its libraries for DOS of 32 bits, `lib386/dos`) and by a
toy on the bench. The toy is not kept: `files.c` of a dozen calls of
standard C, linked with the library and its start-up, run under
`harness/trial32/calls.asm`, which writes down every call of DOS as DOS
gets it.

### Why the build has no C library

`-zl -zls` came over from the C translation, where a library's start-up,
data and stack checks would have broken the layout that is held against
the original byte for byte, and string literals were refused for the
same reason (`CLAUDE.md`, "C for the translation": "There is no C
library"). The trial of 32 bits built its toys the same way, and the
program of 32 bits kept the options, "as in c_compile above"
(`CMakeLists.txt`). It was never decided anew for 32 bits; nothing
written says that a library would be wrong there. What would be wrong
is what follows.

### What can be used with no library

A header that declares types and macros costs nothing at the link:

- `<stdint.h>` (`int32_t`, `int64_t`, `uint8_t`, `INT32_MAX`),
  `<stddef.h>` (`size_t`, `ptrdiff_t`, `NULL`, `offsetof`, which the
  tree uses), `<stdbool.h>` (which the tree uses), `<limits.h>`,
  `<inttypes.h>`'s macros, `<stdarg.h>`, and the macros of `<stdio.h>`,
  `SEEK_SET`, `SEEK_CUR`, `SEEK_END`, `EOF`. All are there.
- `memcpy`, `strlen` and `memcmp` are compiled in line with `-oi`;
  `memset` and `strcmp` are calls even so, under `-os`.
- Numbers of 64 bits: `wcc386` adds, subtracts, compares and shifts
  them in line, and calls `__I8M` and `__I8D` (and the `U` forms) to
  multiply and divide.

And a third way that the toy showed: the library's routines that need
no start-up can be linked without it. A file that calls `memset`,
`strcmp` and divides numbers of 64 bits, compiled with `-zl -zls` and
linked with `lib386/dos/clib3r.lib` and the program's own entry, takes
five modules of the library, `memset`, `i8m`, `i8d`, `_strcmp`, `__stos`,
and nothing of the start-up. So helpers for 64 bits need not be written.

### What the library would change of what DOS is asked

Linked whole, with its start-up, the library asks DOS, in this order:

| Where | What DOS is asked |
|---|---|
| The start-up, before `main` | 30h, the version; 6300h, the table of lead bytes of a double-byte code page; 6601h, the code page. The original asks none of these there |
| `fopen(name, "wb")` | 3D01h, the file opened to write if there is one; 3Ch, it is made; 4400h twice, what the handle is. The original asks 3Ch alone |
| `fwrite` of 600 bytes | 4400h, and no write: the bytes wait in the library's buffer |
| `fclose` | 40h with the 600 bytes, then 3Eh |
| `fopen(name, "rb")` | 3D00h; 4400h twice |
| `fread` of 10 bytes | 4400h; 3Fh twice, of the library's buffer, not 10 bytes |
| `fseek` to 300 | 4201h, where the file is, and nothing more: the place is in the buffer |
| `fread` of 10 more | nothing |
| `fclose` | 4201h, then 3Eh |
| `rename`, `remove` | 56h, 41h, as the original asks |
| `time` | 2Ah, 2Ch, 2Ah: the date, the time and the date again |
| `getenv` | nothing |

And in the registers that a call does not take, the library hands over
what its code left: 40h with 42h in AL, 56h with 77h, 41h with 83h.

So the standard library as the layer is a
program that asks DOS other things, in other sizes and another order,
than the original: a deviation for every file that the program reads or
writes, which the scenarios' screens may not show but `calls.asm` does,
and which entry 6 of `DEVIATIONS.md` would need rows for. Unbuffered
calls would come nearer, but `open`, `read` and `lseek` are of POSIX,
not of standard C, and the library's `open` asks 4400h too.

### Where Open Watcom's standard C is bad

Concretely, for what the layer is for:

- **Text.** No `<uchar.h>`, no `char32_t`, no `mbrtoc32`. `wchar_t` is
  16 bits, so a character beyond the first 65,536 is two of them. The
  locale is "C": `toupper` knows no Cyrillic, and nothing converts
  between CP866 or 437 and UTF-8.
- **Time.** `time_t` is an `unsigned long` of 32 bits: no time before
  1970, none after 2106, and of another sign and width than on Linux,
  where it is a signed 64. `struct tm` is standard and usable.
- **The place in a file.** `fseek`, `ftell` and `off_t` are `long`, 32
  bits with a sign: no file of 2 GB or more, where DOS has 4 GB less a
  byte. The forms of 64 bits are `_fseeki64` and `_lseeki64`, Watcom's
  own.
- **Errors.** ISO C names three errors, `EDOM`, `ERANGE`, `EILSEQ`.
  Watcom's `<errno.h>` has POSIX's names, with other numbers than
  POSIX's (`ENOENT` is 1). None of them says what the program tells
  apart: a drive not ready, a file that another has open, the end of a
  search, a drive that is not there, the current directory.
- **What standard C has not at all**, on any compiler: a directory
  listed, attributes, a file's date, drives, the room of a disk, the
  current directory, a program run and come back from but by
  `system`, the keyboard, the screen, the mouse, how much memory there
  is. Watcom's `<dirent.h>`, `<direct.h>`, `<dos.h>` and `<conio.h>`
  have some of it in DOS's shapes, with `NAME_MAX` 12.

What is good: the types of `<stdint.h>`, `<stddef.h>` and
`<stdbool.h>`, numbers of 64 bits with the library's helpers, the
macros, `struct tm`, and the string routines.

These are findings, for the design that is made from the platforms'
side to take or leave.

## What the program does with drives and paths

Counted in the files of the program, comments left out, `grep`'s counts
and not read one by one:

| What | Places | Files |
|---|---|---|
| A backslash as a character, `'\\'` | 48 | 16: `copy.c` 9, `dirfunc.c` 6, `find.c` 5, `treebox.c` 5, `read.c` 4, `string.c` 4, `tree.c` 4, `edit.c` and `usermenu.c` 2, `cmdline.c`, `exec.c`, `input.c`, `main.c`, `start.c`, `stmain.c`, `system.c` 1 |
| A backslash in a text | 15 | 7, `stcolor_t.c`, `system_t.c` and other tables |
| A colon as a character, `':'` | 38 | 15, most in `dirfunc.c` 7, `copy.c` 6, `usermenu.c` 5 |
| A drive's letter worked out, `- 'A'`, `+ 'A'`, `'A' - 1`, `'@'`, `'Z'` | 46 | 19, most in `find.c` 6, `copy.c`, `main.c` and `system.c` 4 |
| The drive as the first byte of a path, `WinPath[0]`, `DosPth[0]` | 42 | |
| `Fantom`, the drive B: that shares the floppy, by a letter | 10 | `enter.c` 3, `read.c` 3, `commands.c`, `keys.c`, `main.c`, `system.c` |
| `ChgDisk`, a path's drive held against B: before a call | 45 | 12, `copy.c` 13, `dirfunc.c` 9 |
| A drive by its number: `DriveSelect`, `DriveCurrent`, `DirCurrent`, `DriveSpace` and the four of the floppy and the kind | 41 | 12 |
| The letters A to Z gone through | 2 | `drive.c` (the drive menu, `TstDrv` of each), `find.c` (all drives searched) |
| A letter typed held to A to Z | 4 | `dialog.c`, `main.c`, `menu.c`, `view.c` |
| `LenPath`, 68, and the lengths 64 and 67 | some 100 | 20 |
| A name of 8.3 in 13 bytes: `NameF` of a panel's list, `DirName` of the tree, `FilName` of a search, `DTA` | throughout | the panels, the tree, the search, copy, the setup |

What is stored with a drive in it: the path of each panel and the
current path (`WinPath`, `DosPth`, 68 bytes each, and in the setup file,
which is held against the original's byte for byte), the drive of the
file search (`FndDrv`, a letter), the history, the tree's file
`TREEINFO.NCD` at the root of each drive, and every name that a dialog
takes.

## What UTF-8 inside touches

- **The screen's cells.** `ScrBuf` holds a cell as 16 bits, a byte of
  CP866 or 437 and a colour. A cell has to hold a character, a code
  point of 21 bits, and a colour: 32 bits, `ScrBuf` twice as long. Every
  routine that writes text into the cells counts a byte as a column
  today: `line.c`, `box.c`, `panel.c`, `dialog.c`, `menu.c`, `bars.c`,
  `help.c`, `info.c`, `tree.c`, `input.c` and the input lines, `view.c`,
  `edit.c`, `find.c`, the setup's.
- **How wide a character is.** One cell on DOS. On iOS and macOS a
  character may take two, the user's `☁️` among them.
- **The texts of the tables.** Every `_t.c` holds CP866 bytes, held
  against the original's by `portable_c-data`. What is a character
  there beyond ASCII is the frames, B3h to DFh, which CP437 and CP866
  have alike, and a few others, `·`, `√`, `■`, the arrows. What looks
  like a byte beyond ASCII is often no character: FFh and a count with
  80h added are the markup of `line.c`, and stay bytes; FFh never
  occurs in UTF-8, the counts have to move below 80h or be escaped.
  The test of the data would hold a text against the original's
  converted by the code page.
- **The setup's table of characters**, `StInf05_S` and `StInf09_S` of
  `stopts_t.c`: 80h to AFh and E0h to FEh, shown as whatever the code
  page has there. In UTF-8 that is no text but a question to the layer,
  "the characters of the upper half of your code page", which only DOS
  has.
- **Capitals and sorting.** `CaseMap` asks DOS for the capital of each
  byte of 80h and above, the user having decided that a search asks
  every byte as the original does. Names are sorted by their bytes
  after capitals. In UTF-8 the order of the bytes is another: CP866 has
  `Ё` behind `я` and the frames between `п` and `р`; Unicode has `Ё`
  before `А`. So the order and the capital of a character are asked of
  the layer, which on DOS asks DOS and goes by the code page's bytes,
  as today.
- **Names and paths**, typed and found: the panels' lists, the tree,
  the history, the search, every dialog that takes one. The program
  keeps them in fixed fields; a name of 12 characters is up to 48
  bytes of UTF-8. On DOS a character of the name is one of the code
  page, two bytes of UTF-8 for Cyrillic.
- **The keys' characters.** The BIOS gives a byte of the code page
  (the Russian keyboard's driver on the Russian bench); the layer gives
  a character.
- **Files that the program reads and writes** are not its text: a file
  that the viewer shows, the editor edits, or `VC.MNU`, `VC.EXT`,
  `DIRINFO`, `TREEINFO.NCD` and the help file hold, is bytes in the
  encoding that the file has.
- **The setup file**, held byte for byte against the original's: it is
  the main data's first bytes as they lie, paths of 68 bytes among
  them. With UTF-8 inside it cannot be a copy of the memory any more;
  it is written field by field, with the paths converted to the code
  page on DOS. It will not be held against the original's either: by
  decision 15 of the file systems it is a format of the program's own on
  every platform, DOS included, and by decision 7 of `layer/programs.md`
  the program writes it by itself; see "Decided in this area" of "Area:
  file systems".

## Area: file systems

**Draft, 13 October 2026. It is a survey and a proposal, with the
questions for the user at its end. What the user has decided of it is
under "Decided in this area", and nothing else here is decided.**

What this section rests on. Nothing was tried on any platform but DOS;
everything about the others is from their documentation. The pages
that were read for this survey, on 13 October 2026, are listed at its
end, under "Sources", and what a cell says from one of them is what the
page says, in its words where it is quoted. The pages "linked, not read
again" there are the platform's documentation for what the cell says as it is
generally known; they were not read for this survey, and a cell that
rests on one alone is that much less sure. What is marked **inferred** is
a conclusion drawn here and in no documentation; what is marked
**third-party** comes from writers outside the platform's maker. The DOS
column is what the program asks today, in `sys.h` and `dos32.c`, and
what DOS does with it.

The platforms are those of the brief: DOS under DOS/32A; Windows, as a
desktop program of Win32; Linux and POSIX in general; macOS; iOS and
iPadOS, which are one platform here. One platform not on the list would
change the picture: Android, whose shared storage is reached by
documents that have URIs and no paths (**inferred** from general
knowledge, not read). The user does not plan it: see below.

### Decided in this area

The user's answers to the first four questions, 13 October 2026, as
the session passed them on; the words in quotation marks are the
user's.

1. **The shape:** paths under named roots, shape A below.
2. **Android:** not planned. But "archives/FTP some day": so the layer
   allows a root that the program itself serves, an archive opened as a
   drive or a remote site, besides the roots that the platform mounts.
   What that asks of the shape is under "Roots that the program
   serves". Nothing of it is built now.
3. **Linux and macOS:** `/`, the home folder, and each mounted
   removable or network volume, each a drive.
4. **iOS:** ⛁: the app's Documents, ☁️: its iCloud folder, and each
   folder the user picks as a drive of its own.

The user's answers to questions 5 to 15, and to one that followed from
6, 13 and 14 October 2026, as the session passed them on:

5. **The Mac build:** outside the App Store, not sandboxed; paths reach
   everything. The layer still allows a sandboxed build later.
6. **Delete:** both, by two keys. Asked next which key does which: **F8
   moves to the platform's trash where it has one, Shift-F8 deletes
   outright**, as Far Manager has it. On DOS both delete. That is a
   deviation from the original, to be declared when it is built.
7. **Copy:** the platform's own copy where it has one, `CopyFile` on
   Windows, `copyfile` or `clonefile` on Apple's, keeping streams,
   extended attributes and clones; the program's own loop on DOS and
   Linux. The progress and the prompts stay the program's.
8. **Files not yet downloaded** (iCloud, OneDrive): listed with a mark.
   Opening, viewing or copying one downloads it behind a box that Esc
   gives up: the critical error's box, widened.
9. **Attributes off DOS and Windows:** the dialog shows and changes what
   the platform has, read-only and hidden on a Mac, permissions on
   POSIX; DOS's four on DOS and Windows.
10. **Time:** the layer's own instant, seconds since 1970 in UTC as 64
    bits, and nanoseconds; shown in local time.
11. **Errors:** the layer's own list of some fifteen kinds that the
    program tells apart, with the platform's own number kept for a
    message.
12. **Changes made by others:** the layer watches the panels'
    directories, and the program reads one again when it is told,
    polled with the keys. Nothing on DOS, where no call is added.
13. **Long waits:** the program's loop runs on a thread of its own. A
    call of the layer may wait, and asks the program to show a box that
    Esc gives up.
14. **Names that are not UTF-8:** kept as bytes, shown with a mark, and
    handed back to the system exactly as they came.
15. **The setup file off DOS:** **one format everywhere**, chosen
    against the recommendation. The setup file becomes a format of the
    program's own on every platform, DOS included. On DOS that is a
    deviation from the original, and every scenario that saves the
    setup or reads a saved one will show it: it is to be declared, held
    by patches, and asked about in its details when it is built, what
    the format is and whether DOS can still read an old `VC.INI`.

What the design must settle for 15: the format, whether text, in which
encoding, and how roots, paths and the bookmarks of iOS are kept in it;
and what becomes of the scenarios that hold the setup file byte for
byte against the original's.

Decision 7 of `layer/programs.md`, of 14 October 2026, sits on top of
15: the program saves its preferences and its session by itself, when
they change and when it is put away, on every platform, DOS included,
and "Save setup" and "Auto save setup" go. The one format of 15 is then
the format of what is saved, and the preferences and the session may be
two files of it: the setup that the user sets once, and the panels'
directories and cursors, the sort modes, the command line and its
history, which change at every key. Of the scenarios, the comparison of
the disks leaves the setup file out by one rule of the harness, but in
those that are about the setup itself (`CLAUDE.md`, step 3 of the
road). What the design must settle besides what 15 asks: whether they
are one file or two, and which field is in which, the panels' kinds
and views among them; where each lives, a user's settings or,
for the session, the platform's place for state where it has one
(`$XDG_STATE_HOME` on Linux), and on DOS beside the program under names
of eight and three; how long a change waits before it is written, and
how a write is made so that an end in the middle of it leaves the file
before it whole; what two copies of the program running at once do
with one file; and what the scenarios that are about the setup hold,
now that the file is written when the setup changes, not by a command.

### The survey

#### 1. Volumes and roots

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| What a root is | A drive letter, A: to Z:, each with a current directory of its own | A drive letter, or a share of the network, `\\server\share` ([naming][w-naming]) | One tree, `/`; a file system is mounted at a directory of it | One tree; volumes under `/Volumes`; iCloud Drive and File Provider locations are directories in the home (**third-party**: [Sonoma][x-sonoma]) | No tree the app sees whole. The app's own container (`Documents`, `Library`, `tmp`), its iCloud container ([url(forUbiquity…)][a-ubiq]), and folders the user picks ([directories][a-dirs]) |
| How they are listed | Each letter tried: `DriveSelect` and `DriveCurrent` (`TstDrv`) | [GetLogicalDriveStrings][w-drives]; shares are not listed, only reached | [getmntent][p-getmntent], `/proc/self/mountinfo`; a desktop mounts removable and network media under `/media`, `/run/media` or `/run/user/…/gvfs` | [mountedVolumeURLs][a-volumes]: "may block if I/O is required" | `mountedVolumeURLs` "returns nil on platforms other than macOS" ([page][a-volumes]). There is nothing to list: the app knows the roots it was given |
| Kind of a root | `DriveFixed`, `DriveRemote`, `DriveCd` (MSCDEX) | [GetDriveType][w-drivetype]: removable, fixed, remote, CD-ROM, RAM disk | `statfs`'s type, or the mount's options | `volumeIsRemovableKey`, `volumeIsLocalKey`, `volumeIsInternalKey` ([keys][a-keys]) | Of the app's container, the iCloud container, or a picked folder; a picked folder may be on a USB disk, a server, or another app's File Provider (**inferred** from the picker's reach) |
| Label | A file with attribute 8 at the root, made by an FCB | [GetVolumeInformation][w-volinfo], `SetVolumeLabel` | None common | `volumeLocalizedNameKey`; renaming a volume is no file call | None |
| What may be reached at all | Everything | Everything the user may | Everything the user may | Everything outside an App Sandbox; the system asks the user once for Desktop, Documents, Downloads, removable and network volumes (**inferred** from macOS's privacy settings, not read). A sandboxed app reaches only what the user picked, as on iOS | Only the app's containers and what the user picked in a [document picker][a-dirs], for which the app has to call `startAccessingSecurityScopedResource`, which "can fail" |

#### 2. How a file is named, and what lasts between runs

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| A name in a call | A path, `C:\DIR\FILE.EXT`, 67 characters at most, 8.3 names, code page bytes | A path in UTF-16; `/` taken for `\` (not read); 260 characters before Windows 10 1607, and after it unless `\\?\` or an opt-in ([naming][w-naming], [CopyFileEx][w-copyfileex]); `CON`, `PRN`, `AUX`, `NUL`, `COM1`, `LPT1` and their like are devices in any directory | A path of bytes, any but `/` and NUL; `NAME_MAX` 255 bytes, `PATH_MAX` 4,096 on Linux and no hard limit | A path, or a file URL; Foundation works in URLs | A file URL. What a picked folder holds is reached by URLs below the folder's, which are "also security-scoped" ([directories][a-dirs]); that this holds for iCloud and every File Provider alike is **inferred** |
| Handles to directories | None | A directory can be opened as a handle | `openat` and its kin, relative to a directory's descriptor | The same, below Foundation | The same, below Foundation, inside the granted scope (**inferred**) |
| What lasts between runs | The path | The path | The path | The path; a [bookmark][a-dirs] for what a sandboxed app was given | A bookmark, `bookmarkData`, resolved again with `URL(resolvingBookmarkData:bookmarkDataIsStale:)`; the user can take the access back in Settings, and the app "must be ready to handle failures" ([directories][a-dirs]). A path alone gives no access next time |
| Case | Not kept, names are capitals | Kept; "Do not assume case sensitivity" ([naming][w-naming]) | Kept and compared as bytes | Kept; a volume says whether it compares case (`volumeSupportsCaseSensitiveNamesKey`, [keys][a-keys]); a Mac's own compares without, and NFC and NFD as one (**inferred**, not read) | The same key; the device's own volume compares case (**inferred**, not read) |

#### 3. Listing a directory

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| Call | `FindFirst` (4Eh) with a pattern and the attributes to include, then `FindNext` (4Fh), into the DTA | [FindFirstFileEx][w-findfirst] with `FindExInfoBasic` and `FIND_FIRST_EX_LARGE_FETCH`, then `FindNextFile`; or `GetFileInformationByHandleEx` for a batch | [readdir][p-readdir], then [fstatat][p-stat] or Linux's [statx][l-statx] for each entry | [enumerator(at:includingPropertiesForKeys:…)][a-enum], which pre-fetches the keys asked for; `getattrlistbulk` below | The same, inside a coordinated read for anything outside the app's container ([directories][a-dirs]) |
| What one entry has | Name 8.3, attributes, time, date, size of 32 bits | Name, short name, attributes, three times, size of 64 bits: all with the name, no second call | Name and inode; `d_type` is not POSIX and may be "unknown"; the rest is a call per entry | Any key asked: name, size, dates, kind, hidden, readable, writable, iCloud state ([keys][a-keys]) | As macOS; an item not downloaded may show as a hidden `.name.icloud` file (**third-party**: [forums][x-icloud-forum], [article][x-fatbobman]); Apple says to read its state with `NSMetadataQuery` ([coordinate][a-coord-read]) |
| Pattern | Matched by DOS, by DOS's rules for `*` and `?` in 8.3 | Matched by Windows, by its own rules | None: the program matches | None in Foundation | None |
| Hidden | Attribute bit, a filter of the call | Attribute bit | A name that begins with a dot, by custom | Either the dot or the `UF_HIDDEN` flag, `isHiddenKey` | As macOS; `skipsHiddenFiles` |
| Names | Code page bytes, 12 characters | UTF-16, 255 units a component | Bytes, 255; UTF-8 by custom, not by rule, so a name may not be UTF-8 | UTF-8, 255 (**inferred** for APFS) | As macOS |
| What it costs | A call per entry, of DOS | Few calls for many entries | A call per entry for its metadata | Batched below; one object per entry | As macOS, plus coordination and the scope |

#### 4. Reading and writing a file

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| Open, read, write, seek, close | 3Dh, 3Fh, 40h, 42h, 3Eh; a place of 32 bits | `CreateFile`, `ReadFile`, `WriteFile`, `SetFilePointerEx` (64 bits) | [open][p-open], `read`, `write`, `lseek`, `pread`; `off_t` of 64 bits | The same; `FileHandle` above | The same, inside a coordinated read or write when the file is iCloud's or the user's ([directories][a-dirs]) |
| Others writing | Sharing modes, where SHARE is loaded: error 20h | Sharing modes: `ERROR_SHARING_VIOLATION` | Locks only advise | As POSIX, and [file coordination][a-coord] among apps that coordinate | Coordination is required, not offered |
| A file not yet on the device | None | A placeholder of the [cloud files API][w-cfapi]: reading it "will hydrate without additional code changes"; a toast shows progress; the user can forbid an app to download | A network or FUSE file system may wait as long as it likes | Since macOS 14 iCloud Drive keeps "dataless" files (**third-party**: [Sonoma][x-sonoma]); whether a read downloads one is a policy of the process ([getiopolicy_np][a-iopolicy]), whose default the man page gives as off, which is not how apps are seen to behave (**unsure**) | A coordinated read "blocks (potentially for a long time) while the file is downloaded", and fails if it cannot be ([coordinate][a-coord-read]); or the app calls [startDownloadingUbiquitousItem][a-startdl] and watches its state |

#### 5. Making, renaming, moving, copying, deleting

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| Create | 3Ch, which empties a file that is there | `CreateFile`, with the choice | `open` with `O_CREAT`, `O_EXCL` | The same | The same, coordinated |
| Rename | 56h, on one drive; fails if the target is there | [MoveFileEx][w-movefileex]: fails if the target is there unless asked; across volumes only if asked to copy | [rename][p-rename]: one step, and **replaces** a target that is there; `EXDEV` across file systems; Linux's `renameat2` with `RENAME_NOREPLACE` does not replace | `rename`, and `renamex_np` with `RENAME_EXCL` or `RENAME_SWAP` | As macOS, coordinated |
| Copy | None: the program reads and writes, in `copy.c` | [CopyFileEx][w-copyfileex]: progress, cancel, and it keeps attributes, streams, extended attributes and security | None in POSIX; Linux's `copy_file_range` and `FICLONE` copy the bytes only | [copyItem][a-copy], which on APFS makes a clone ([APFS][a-apfs]); `copyfile(3)` keeps metadata | `copyItem`, coordinated |
| Delete | 41h; by a mask, 13h with an FCB | `DeleteFile` | `unlink` | `removeItem` | `removeItem` |
| Trash | None | The Recycle Bin, through `IFileOperation` with `FOFX_RECYCLEONDELETE` | The freedesktop.org [Trash specification][p-trash], by hand or through GIO | [trashItem][a-trash] | [trashItem][a-trash], from iOS 11 |
| Directories | 39h, 3Ah; 3Ah fails for a directory that is not empty or is current (error 10h) | `CreateDirectory`, `RemoveDirectory` | `mkdir`, `rmdir` (`ENOTEMPTY`) | The same; `removeItem` removes a tree | As macOS |
| One step, all or nothing | Rename on one drive (**inferred**) | Rename on one volume | Rename in one file system | Rename; and the swap | As macOS |

#### 6. Attributes and times

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| Attributes | Read-only, hidden, system, archive, directory, label; 4300h and 4301h | The same bits, with the same values, and more: reparse point, offline, and those of placeholders, `RECALL_ON_OPEN`, `RECALL_ON_DATA_ACCESS`, `PINNED`, `UNPINNED` ([attributes][w-attrs]) | The mode: kind, read, write and run for owner, group and others; owner and group; Linux's flags such as immutable | The mode, and flags: `UF_HIDDEN`, user and system immutable; read in keys ([keys][a-keys]) | As macOS; the app is not the owner of what it was given (**inferred**) |
| Times | One, the last write, local time, to two seconds; by handle, 5700h and 5701h | Created, last access, last write: 100 ns since 1601, UTC | Last access, last change of the data, last change of the inode, to the nanosecond, UTC; when made only by `statx` | Created, changed, opened, attributes changed ([keys][a-keys]) | As macOS. Reading a file's times or a volume's free space is an API that Apple requires a reason for, declared in the app: "display file timestamps to the person using the device" is one ([required reasons][a-reasons]) |

#### 7. Free space and the size of a volume

| DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|
| 36h: clusters, sectors, bytes; of 16 bits each | [GetDiskFreeSpaceEx][w-diskfree]: free to this user, total, total free, in bytes, for any path | [statvfs][p-statvfs]: blocks free to this user, and the block's size | `volumeAvailableCapacityKey`; `…ForImportantUsageKey`, "for storing important resources", which the page does not explain further ([key][a-capacity]). Volumes of one APFS container share their space ([APFS][a-apfs]) | As macOS. A reason must be declared: "display disk space information to the person using the device" ([required reasons][a-reasons]) |

#### 8. Being told of changes

| DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|
| None. The program reads a panel again after what it did itself, after a command, and on Ctrl-R | [ReadDirectoryChangesW][w-rdcw], with a buffer that can overflow, then everything has to be read again; `FindFirstChangeNotification` says only that something changed | None in POSIX; Linux's [inotify][l-inotify] for each directory watched, not for changes made on another machine of a network file system | FSEvents for a tree, `kqueue` for a descriptor; and [NSFilePresenter][a-presenter], which is "not notified about changes made directly using low-level read and write calls" | [NSFilePresenter][a-presenter], which must be taken off while the app is in the background; `NSMetadataQuery` for iCloud's items and their download state |

#### 9. Errors

| | DOS | Windows | POSIX | macOS, iOS |
|---|---|---|---|---|
| How | AX and the carry; 59h says more. In hex, as the program writes them | `GetLastError`, in decimal: the low numbers are DOS's, 15h is 21 ([codes][w-errors]) | `errno` | `NSError`, with a domain and a code; POSIX's `errno` below |
| Not there: file, path | 2, 3 | 2, 3 | `ENOENT`, `ENOTDIR` | `NSFileNoSuchFileError`, `ENOENT` |
| Refused | 5 | 5 | `EACCES`, `EPERM`, `EROFS` | `NSFileReadNoPermissionError`, `NSFileWriteNoPermissionError` |
| In use by another | 20h | 32 | None but `EBUSY`, `ETXTBSY` | None; coordination waits instead |
| Not ready | The critical error, the program's box: Retry or Abort; 15h, and 53h when given up | 21; the box of the system unless `SetErrorMode` says not | `EIO`, or a wait | Download failed; not reachable |
| End of a listing | 12h | 18 | `readdir` returns none | The enumeration ends |
| No such drive | 0Fh | 15 | None: no drives | None |
| No room | None from DOS: a write that comes short, which the program calls FFh | 39, 112 | `ENOSPC`, `EDQUOT` | `NSFileWriteOutOfSpaceError` |
| Not empty, current, another drive | 5, 10h, 11h | 145, 16, 17 | `ENOTEMPTY`, `EBUSY`, `EXDEV` | Their errors |
| The user gave up | 53h after the box | `ERROR_REQUEST_ABORTED`, `ERROR_CANCELLED` | `EINTR` | `NSUserCancelledError` |

What the program tells apart today is the DOS column: some fifteen
numbers, of which `ERR_NOT_READY` is named most, some forty times by
`grep`, then no access, no file, no path, and the end of a listing, the
disk full, a file shared, the current directory and no drive.

#### 10. What may take long, and must not stop the program

| | DOS | Windows | POSIX, Linux | macOS | iOS |
|---|---|---|---|---|---|
| What can wait | A floppy, a network drive, the critical error's box | A placeholder being hydrated, a network share, a drive without a disk | A network or FUSE mount, without end where the mount is hard | A dataless file; the system's question for a folder (**inferred**); a coordinated access that waits for another app | A coordinated read that downloads; [url(forUbiquityContainerIdentifier:)][a-ubiq]: "Do not call this method from your app's main thread"; coordination that waits for another app |
| Where the system asks the user | The critical error: the system calls the program's `DeviceErrorFn` in the middle of the call | A message box of its own for a drive not ready, unless turned off | Nowhere | Privacy questions; the progress of a download | Nowhere during a call; the document picker is a screen the app shows |
| What a program that waits may do | Show the box, read keys | Wait or use a thread | Wait or use a thread | Wait off the main thread | Wait off the main thread only: the main thread draws, and a coordinated request that is waiting when the app is suspended "is canceled" ([coordinator][a-coord]) |

### The differences that matter

1. **On every platform but iOS, a path reaches everything the user may
   reach.** On iOS a path is not enough: what lies outside the app's
   container is reached by a URL that the user gave, inside a scope that
   is started and stopped, by reads and writes that are coordinated.
   Once given, though, the items are file URLs, paths below a root, and
   their children are reached by appending names
   ([directories][a-dirs]). **Inferred:** the difference is one of
   access, not of naming, and it is a difference of the root.
2. **What lasts is a path everywhere but iOS** (and a sandboxed Mac),
   where it is a bookmark: bytes from the system that the program
   stores and cannot read.
3. **Waits.** DOS and Windows already stop in the middle of a call and
   ask: DOS asks the program, by the critical error. iOS waits for
   downloads and for other apps, and must not wait on the thread that
   draws. A program that is one loop of keys and calls fits that only if
   the loop is not the thread that draws, and the wait can be shown and
   given up, as the box of the critical error is today.
4. **Rename replaces on POSIX** and fails on DOS and Windows when the
   target is there. The layer has to say which, and do it everywhere.
5. **Copy:** Windows and Apple copy for the program and keep what the
   program cannot see: Windows its streams, extended attributes and
   security; Apple makes a clone on APFS, which takes no room. POSIX
   copies only bytes. DOS has nothing.
6. **Attributes:** DOS and Windows share the bits, with the same values.
   POSIX and Apple have permissions and owners instead; hidden is a name
   on POSIX and a flag or a name on Apple; system and archive mean
   nothing there.
7. **Times:** DOS has one, local, to two seconds; all others have UTC
   instants with three or four kinds of time and nanoseconds.
8. **Names:** 8.3 in a code page on DOS; up to 255 of UTF-16 on Windows;
   bytes on Linux, not always UTF-8; UTF-8 with normalisation on Apple.
9. **Patterns** are matched by DOS and Windows, and by the program
   everywhere else.
10. **Being told of changes** exists everywhere but DOS, in four shapes,
    none of which sees everything: inotify misses the network, a file
    presenter misses what was not coordinated.

### Three shapes for the layer's file systems

#### A. Paths under named roots

A place is a path, text in UTF-8 with the platform's separator, that
begins with a root: `C:\` on DOS; `C:\` or `\\server\share\` on Windows;
`/`, the home folder and each mounted removable or network volume on
Linux and macOS; `⛁:/`, `☁️:/` and one for each folder the user picked
on iOS (decisions 3 and 4); and, some day, a root that the program
serves itself (decision 2). A name that is not UTF-8 is kept as its
bytes and handed back as it came (decision 14). The layer lists the
roots, says of each its kind and whether it is ready, and takes and
answers paths. An open file and a listing under way are handles, as
today.

- **DOS:** the paths are today's, in the code page; the DOS file turns
  UTF-8 into the code page and back, and asks the same calls in the
  same order, but where the setup file is written and read in the
  program's own format (decision 15).
- **Windows:** UTF-8 to UTF-16, `\\?\` in front of a long path. Close
  to DOS: the same drives, bits and errors.
- **Linux, macOS:** the path as it is. Mounts are roots by choice. The
  Mac build is not sandboxed (decision 5).
- **iOS:** the layer keeps a table of roots: its name, the base URL,
  the bookmark. It turns a path into a URL by appending the components,
  starts the scope, coordinates the read or write, and asks the program
  when it has to wait (difference 3).
- **Cannot express:** a place that is no file URL, such as Android's
  documents by their URIs, which is not planned; an item followed when
  it is moved while the program holds it, which a bookmark does. A
  server that no system has mounted, or an archive, is a root that the
  program serves (below).
- **What the program changes:** fixed fields of 68 bytes and names of 13
  become text of a length the layer says; the backslash and the colon
  as characters (86 places) become the layer's separator and the root's
  length; a drive by its letter (46 places, and 41 by number) becomes a
  root by its name or index. The code that walks a path stays. The
  setup file, a copy of the main data today, becomes a format of the
  program's own everywhere (decision 15).

#### B. Opaque handles to places

The program never builds a path: it opens a root, lists it, and gets a
handle for each entry, which it opens in turn. A handle can be turned
into text to show, and into bytes to keep (a path, or a bookmark).
POSIX's `openat`, Windows' handles, Apple's URLs and bookmarks and
Android's document URIs all fit it.

- **DOS:** a handle holds a path; DOS is asked the same calls. Opening
  a directory is no call.
- **Elsewhere:** a natural fit to each system's own objects, and the
  scope of iOS rides on the handle.
- **Cannot express:** what the user types: a path in a dialog, the
  target of a copy, the command line, the history. Each needs the
  layer to parse text into a handle, which is shape A again under it.
- **What the program changes:** every place that makes a path, which is
  most of `copy.c`, `dirfunc.c`, `find.c`, `tree.c`, the panels and the
  dialogs; the setup file keeps bytes of no fixed size.

#### C. Locations as URLs

A place is a URL in UTF-8, `file:///C:/DIR/` or `icloud:/Docs/`, and
shown to the user in the platform's way. A scheme leaves room for file
systems that no system mounts, an archive or a server.

- **DOS:** a URL turned into a path before every call; the same calls.
- **Elsewhere:** a URL is a path with a scheme on Apple; on Windows and
  Linux a conversion before every call.
- **Cannot express:** nothing more than A, but with two texts for one
  place, the URL and what the user sees, and the program has to keep
  them apart everywhere it shows or takes one.
- **What the program changes:** what A asks, and the two texts.

#### Recommendation: A, with the waits of iOS as the critical error

The user chose A (decision 1). The reasons, as proposed: every platform on the list reaches everything it can reach by
a path below a root, iOS included once the root is given; the program
is built around paths that the user reads and types; and the DOS file
keeps today's calls in their order. The weight of iOS goes into its
roots, which the layer keeps, and into one thing that the program
already has: when the system has to wait or fails in the middle of a
call, it asks the program, which shows a box and lets the user try again
or give up. On DOS that is `DeviceErrorFn`; on iOS and Windows it is a
download under way; on Linux, a mount that does not answer, where the
layer can only wait.

B would have been the choice if Android were to come; it is not
planned. C's room for file systems that no system mounts, an archive or
a server, A has as roots that the program serves. C adds nothing else
to A that this program asks.

Inside A, these principles, each a proposal:

- **One question, one function, asked when the program asks it.** The
  kind of a root, its label and its space are each asked by themselves,
  so the DOS file asks DOS what the program asks today and when.
- **The layer says where it has nothing.** Copying, the trash, labels
  and the floppy's second letter are functions that answer "none here",
  and the program goes its own way then: it copies by reading and
  writing, as on DOS.
- **Rename does not replace,** on any platform.
- **The layer's own types where standard C on Open Watcom is short**
  (below).

#### Roots that the program serves

An archive opened as a drive, or a remote site, is a root like any
other to the rest of the program: it has a name in the drive menu,
paths below it with the platform's separator, and listings, files,
attributes and times of the same shapes. What differs is who answers.
Under a root that the platform mounts, the platform's file answers,
`dos32.c` on DOS. Under a root that the program serves, code of the
program answers, common to every platform: a reader of an archive, a
client of a protocol. So the file-system functions that the program
calls go first to a table of roots, which sends each call to whoever
serves the root of its path. That table is common code, above the
platform's file and not in it, and each server answers the same
functions that the platform's file does, as a table of functions of
its own.

- **Listing, reading, writing** under such a root are the server's. It
  does its own reading and writing of the archive through the layer's
  file functions, under the root that holds the archive; a client of a
  protocol needs the network, which is another area of the layer and
  not surveyed here.
- **What a server lacks it answers "none here"**, as a platform does:
  the trash, a label, free space perhaps, the platform's copy. Copying
  between two roots, of which one is the program's, is the program's
  loop of reads and writes, as on DOS. Rename stays within one root.
- **Waits** of a remote site are asked of the program as any other
  wait is, by the widened critical error.
- **What lasts** is what opens the root again: the archive's path, the
  site's address, kept by `RootKeep` as a platform's root is.
- **The current directory** of such a root is the table's, as on iOS.
  A command run from the command line needs a directory of the
  platform's; what it gets under the program's own root is a question
  for when such a root is built.
- **On DOS** nothing changes: every root there is the platform's, and
  the table sends every call on to `dos32.c` as it comes, so DOS is
  asked the same calls in the same order. The table and the servers
  are not built now; what is built now is only that every function of
  this area takes a path that begins with its root, and that each
  function that a root may lack can answer "none here".

### How the program's needs map onto shape A

Proposed names; the names are not decided. What a row does by a
decision of "Decided in this area" names the decision. "DOS" says what
the DOS file asks, which is what the program asks today.

| What the program does | Today | Proposed | DOS |
|---|---|---|---|
| The drive menu, the drives to search | `TstDrv` for each letter; `DriveFixed`, `DriveRemote`, `DriveCd` | `RootNext(index)`: the next root, its name; `RootKind(root)` | The same calls for each letter, in the same order; the kind asked only where the search asks it |
| A drive's label | `FileTrueName`, `FindFirst` with attribute 8; an FCB to set it | `RootLabel(root)`, `RootSetLabel(root, label)`: none on POSIX and iOS | The same calls |
| Free space | `DriveSpace`, in clusters | `RootSpace(root)`: free and total, `int64_t` bytes | 36h, multiplied |
| The current directory of each drive | `DirCurrent`, `DirChange`, `DriveSelect`, `DriveCurrent` | `DirCurrent(root)`, `DirChange(path)`; the layer keeps one for each root where the system keeps one for the process | The same calls |
| Listing a panel, the tree, the search | `SetDTA`, `FindFirst(pattern, 37h)`, `FindNext` | `DirOpen(path, pattern, what)` answering the first entry, `DirNext`, `DirClose`; the pattern matched by the layer, by DOS's rules, where the system does not | 1Ah, 4Eh, 4Fh; `DirClose` asks nothing |
| One entry | 13 bytes of name, attributes, time, date, 32 bits of size | Name in UTF-8, or its bytes where it is not UTF-8, shown with a mark (decision 14); size `int64_t`; the time of the last write; the layer's flags (below); whether the content is on the device, shown with a mark (decision 8) | The DTA, converted |
| Open, read, write, seek, close | 3Dh, 3Fh, 40h, 42h, 3Eh with a `u16` handle | The same functions, with `size_t` counts and `int64_t` places; the handle the layer's | The same calls; a place of 4 GB or more is refused |
| Make, delete, rename | 3Ch, 41h, 56h | `FileCreate`, `FileDelete`, `FileRename` (which does not replace) | The same calls |
| Delete by a mask | 29h and 13h with an FCB | `FileDeleteMatching(dir, pattern)`, which lists and deletes where no system does it at once | The same FCB calls |
| Copy | The program's loop of reads and writes | `FileCopy(from, to, progress)`: the platform's copy on Windows and Apple's, answering "none here" on DOS and Linux, where the program's loop copies; the progress and the prompts the program's (decision 7) | Nothing: the loop, as today |
| Trash | None | `FileTrash(path)`, for F8, "none here" on DOS; Shift-F8 is `FileDelete` (decision 6) | Nothing: F8 deletes, as Shift-F8 does |
| Attributes | 4300h, 4301h, DOS's bits | The layer's flags: read-only, hidden, system, archive, directory, link, not on the device; the permissions of POSIX; and which of them a root has and lets the program set: DOS's four on DOS and Windows, read-only and hidden on a Mac, the permissions on POSIX (decision 9) | The same calls; the flags are DOS's bits, renamed |
| A file's time | 5700h, 5701h, packed | `FileTime(handle)`, `FileSetTime(handle, time)` in the layer's time (decision 10; below) | The same calls. DOS knows no time zone: its local time is taken for the instant and shown as it is, so that a copy keeps the stamp to the bit |
| A device, `CON` | 4400h | `FileKind(handle)`: a file, or something else | The same call |
| A path made whole | 60h | `PathTrue(path)`, as `realpath` | The same call |
| A wait, a drive not ready | `DeviceErrorTake(FtErr1)` | The same, widened: the system asks the program about a wait or a failure that can be tried again, with what it is waiting for, a download among them; the program shows a box that Esc gives up, its loop being on a thread of its own (decisions 8 and 13) | The same: the critical error |
| Being told of changes | None | `DirWatch(path)` for each panel's directory, and `DirChanged(watch)` polled in the loop of keys: "read again" (decision 12) | Never changed, no call |
| Granting a root | None | `RootGrant()`: on iOS, and on a Mac if it is ever sandboxed (decision 5), the picker; "none here" elsewhere | Nothing |
| Keeping a root | The path, in the setup file | `RootKeep(root)`: bytes to store; `RootRestore(bytes)`; kept in the setup file, of the program's own format (decision 15) | The path, in that format: no longer the original's file |
| A root that the program serves (decision 2; not built now) | None | The same functions, sent by the table of roots to the root's server | Nothing: no such root on DOS |

#### What the DOS file could not keep

The calls and their order stay under every shape. What the program
reads that is no answer does not: `value` where nothing failed, which is
AX as DOS left it, and which some callers go on with (entry 6 of
`DEVIATIONS.md` has the one met, `ReDr00`), and what a seek that failed
leaves, which `view.c` and `copy.c` go on with. A layer of standard
values has no place for either. Each such caller either gets a field of
the DOS file's that no other platform fills, or is a deviation to be
declared. The places are to be counted before the design is approved.

Nor does the DOS file keep two things that the user has decided
otherwise. The setup file is written and read in the program's own
format on DOS too (decision 15), so it is no longer the original's, and
every scenario that saves the setup or reads a saved one shows that: a
deviation to be declared and held by patches. And delete by two keys,
F8 and Shift-F8, both deleting on DOS (decision 6), is a deviation from
the original too. Both are declared when they are built.

#### Standard C and Open Watcom, for this area

- **Enough:** `int64_t` and `<stdint.h>` for sizes and places, with the
  library's helpers for 64 bits linked without its start-up (the
  section above); `size_t` for counts; `char` strings in UTF-8, which
  need nothing of `<uchar.h>`; `struct tm` to show a date.
- **Not enough, so the layer's own:** time, since `time_t` is 32 bits
  without a sign on Open Watcom and a signed 64 elsewhere: a structure
  of `int64_t` seconds since 1970 in UTC and nanoseconds (decision 10).
  Errors, since `errno`'s numbers are not POSIX's on Open Watcom and say
  nothing of a drive not ready or a file shared: an enumeration of the
  layer's, with the platform's own number kept beside it for a message
  (decision 11).
  The place in a file, since `fseek` is 32 bits with a sign. Names of
  a length that differs by platform: the longest name and path are the
  layer's constants.
- **Not wanted in any case:** `fopen` and its kin, which reach only
  local files and, on Open Watcom, ask DOS other things in another
  order (the section above).

### Questions for the user

Each with the ways, the recommended one first. All are answered,
1 to 4 on 13 October 2026, 5 to 15 on 13 and 14 October 2026: see
"Decided in this area". They are kept here as they were put, and 5 to
15 are each marked with the answer.

1. **The shape.** (a) Paths under named roots, text in UTF-8 with the
   platform's slash; iOS keeps its roots behind it. (b) Opaque handles,
   the program never building a path. (c) URLs. Recommended: (a), since
   every platform here reaches its files by paths below a root, and DOS
   keeps its calls.
2. **Android, or file systems no system mounts** (archives, FTP)?
   (a) Not planned; shape (a) of question 1 holds. (b) Some day; then
   opaque handles, since Android's documents are no paths.
   Recommended: (a), unless you know of such plans.
3. **Drives on Linux and macOS.** (a) `/` as one drive, and the home
   and each mounted removable or network volume as further drives.
   (b) `/` alone. (c) Every mount. Recommended: (a); (c) lists dozens of
   system mounts on Linux.
4. **Drives on iOS.** (a) ⛁: the app's own Documents, ☁️: the app's
   own iCloud folder, and each folder the user picks as a drive of its
   own, added from the drive menu. (b) Only ⛁: and ☁️:. Recommended:
   (a): the rest of iCloud Drive, USB disks and servers are reached no
   other way.
5. **The Mac build.** (a) Outside the App Store, not sandboxed: paths
   reach everything. (b) In the App Store, sandboxed: roots granted as
   on iOS. Recommended: (a); the layer allows (b) later without change.
   **Answered:** (a); see decision 5.
6. **Delete.** (a) To the trash where the platform has one, deleted on
   DOS. (b) Deleted everywhere, as the original. (c) Both, by two keys.
   Recommended: (a); a file manager that deletes past the trash
   surprises users of Windows, macOS and iOS.
   **Answered:** (c): F8 to the trash, Shift-F8 deletes outright, both
   delete on DOS; see decision 6.
7. **Copy.** (a) The platform's copy where it has one, keeping streams,
   extended attributes and clones; the program's loop on DOS and Linux.
   (b) The program's loop everywhere. Recommended: (a); the loop loses
   what it cannot see. **Answered:** (a); see decision 7.
8. **Files not downloaded (iCloud, OneDrive).** (a) Listed with a mark;
   opening one downloads it behind a box that Esc gives up. (b)
   Refused until downloaded elsewhere. (c) Everything downloaded when a
   panel is read. Recommended: (a), the critical error's box widened.
   **Answered:** (a); see decision 8.
9. **Attributes off DOS and Windows.** (a) The dialog shows what the
   platform has: read-only, hidden on a Mac, permissions on POSIX.
   (b) Only DOS's four, mapped. Recommended: (a); system and archive
   mean nothing there. **Answered:** (a); see decision 9.
10. **Time.** (a) The layer's own: 64-bit seconds since 1970, UTC, and
    nanoseconds; shown in local time. (b) `struct tm` in local time.
    Recommended: (a); a local time is ambiguous at the change of
    summer time, and `time_t` is unusable on Open Watcom.
    **Answered:** (a); see decision 10.
11. **Errors.** (a) The layer's own list of some fifteen kinds, with the
    platform's number kept for a message. (b) `errno`'s names.
    Recommended: (a); `errno` has no "not ready" or "in use", and Open
    Watcom's numbers are not POSIX's. **Answered:** (a); see decision
    11.
12. **Changes made by others.** (a) The layer watches a panel's
    directory and the program reads it again when told, polled with the
    keys. (b) Read again only as now. Recommended: (a), nothing on DOS,
    where no call is added. **Answered:** (a); see decision 12.
13. **Long waits.** (a) The program's loop runs on a thread of its own;
    the layer's calls may wait, and ask the program to show a box that
    Esc gives up. (b) Every call of the layer asynchronous, with events.
    Recommended: (a); it keeps the program a loop, as on DOS.
    **Answered:** (a); see decision 13.
14. **Names that are not UTF-8** (bytes on Linux, lone halves on
    Windows). (a) Kept as bytes, shown with a mark, and handed back as
    they came. (b) Left out of the panel. Recommended: (a); a file one
    cannot see cannot be renamed or deleted. **Answered:** (a); see
    decision 14.
15. **The setup file off DOS.** (a) DOS's file stays as it is, byte for
    byte; elsewhere a file of the program's, text, with roots and paths
    and the bookmarks iOS needs. (b) One format everywhere.
    Recommended: (a); the DOS file is held against the original's.
    **Answered:** (b), against the recommendation: the program's own
    format on every platform, DOS included; see decision 15.

### What this survey is unsure of

- Whether a read of a dataless file on macOS downloads it for an
  ordinary app: the man page's default says no; what users see says
  yes. It decides whether a Mac needs coordinated reads as iOS does.
- How iOS lists iCloud items that are not downloaded today: a hidden
  `.icloud` file, by reports of outside writers, and reports of change
  in iOS 18.4. The proposal asks the state by resource keys and does
  not go by names.
- Whether the root of iCloud Drive itself can be picked as a folder on
  iOS, or only folders inside it.
- The macOS privacy questions for removable and network volumes, and
  whether iCloud Drive and `~/Library/CloudStorage` ask too.
- What the program's callers read beyond the answer (the section above,
  "What the DOS file could not keep"): not counted.

#### Sources

Read, for this survey: on Apple's side [directories][a-dirs],
[NSFileCoordinator][a-coord], [coordinate(readingItemAt:…)][a-coord-read],
[NSFilePresenter][a-presenter], [url(forUbiquityContainerIdentifier:)][a-ubiq],
[startDownloadingUbiquitousItem][a-startdl], [mountedVolumeURLs][a-volumes],
[enumerator][a-enum], [URLResourceKey][a-keys], [the capacity for important
usage][a-capacity], [the required reasons][a-reasons], [copyItem][a-copy],
[About Apple File System][a-apfs], [trashItem][a-trash], and xnu's
[getiopolicy_np][a-iopolicy]; on Microsoft's [CopyFileEx][w-copyfileex] and
[the cloud files API][w-cfapi] whole, and for the sentence that a cell
quotes [naming][w-naming], [MoveFileEx][w-movefileex],
[ReadDirectoryChangesW][w-rdcw], [FindFirstFileEx][w-findfirst] and
[GetDiskFreeSpaceEx][w-diskfree]; of POSIX and Linux [rename][p-rename],
[readdir][p-readdir] and [inotify][l-inotify], for the same; and the
third-party pages [on Sonoma][x-sonoma], [on placeholders][x-fatbobman] and
[a forum thread][x-icloud-forum].

Linked, not read again: [GetLogicalDriveStrings][w-drives],
[GetDriveType][w-drivetype], [GetVolumeInformation][w-volinfo], [the file
attributes][w-attrs], [the error codes][w-errors], [getmntent][p-getmntent],
[fstatat][p-stat], [open][p-open], [fstatvfs][p-statvfs], [statx][l-statx],
and the [trash specification][p-trash], of which only the contents were
seen. What a cell says without a link, `IFileOperation`, `renamex_np`,
`renameat2`, `copyfile(3)`, `SetErrorMode`, FSEvents and the error names
of Foundation, is from general knowledge of the platforms and no page.

[w-naming]: https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file
[w-drives]: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getlogicaldrivestringsw
[w-drivetype]: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getdrivetypew
[w-volinfo]: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getvolumeinformationw
[w-findfirst]: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-findfirstfileexw
[w-copyfileex]: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-copyfileexw
[w-movefileex]: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw
[w-attrs]: https://learn.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
[w-diskfree]: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getdiskfreespaceexw
[w-rdcw]: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
[w-errors]: https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
[w-cfapi]: https://learn.microsoft.com/en-us/windows/win32/cfapi/build-a-cloud-file-sync-engine
[p-getmntent]: https://man7.org/linux/man-pages/man3/getmntent.3.html
[p-readdir]: https://pubs.opengroup.org/onlinepubs/9799919799/functions/readdir.html
[p-stat]: https://pubs.opengroup.org/onlinepubs/9799919799/functions/fstatat.html
[p-open]: https://pubs.opengroup.org/onlinepubs/9799919799/functions/open.html
[p-rename]: https://pubs.opengroup.org/onlinepubs/9799919799/functions/rename.html
[p-statvfs]: https://pubs.opengroup.org/onlinepubs/9799919799/functions/fstatvfs.html
[p-trash]: https://specifications.freedesktop.org/trash/latest/
[l-statx]: https://man7.org/linux/man-pages/man2/statx.2.html
[l-inotify]: https://man7.org/linux/man-pages/man7/inotify.7.html
[a-ubiq]: https://developer.apple.com/documentation/foundation/filemanager/url(forubiquitycontaineridentifier:)
[a-dirs]: https://developer.apple.com/documentation/uikit/providing-access-to-directories
[a-volumes]: https://developer.apple.com/documentation/foundation/filemanager/mountedvolumeurls(includingresourcevaluesforkeys:options:)
[a-keys]: https://developer.apple.com/documentation/foundation/urlresourcekey
[a-enum]: https://developer.apple.com/documentation/foundation/filemanager/enumerator(at:includingpropertiesforkeys:options:errorhandler:)
[a-coord]: https://developer.apple.com/documentation/foundation/nsfilecoordinator
[a-coord-read]: https://developer.apple.com/documentation/foundation/nsfilecoordinator/coordinate(readingitemat:options:error:byaccessor:)
[a-startdl]: https://developer.apple.com/documentation/foundation/filemanager/startdownloadingubiquitousitem(at:)
[a-iopolicy]: https://cs.iossec.tech/xnu/latest/source/bsd/man/man3/getiopolicy_np.3
[a-copy]: https://developer.apple.com/documentation/foundation/filemanager/copyitem(at:to:)
[a-apfs]: https://developer.apple.com/documentation/foundation/about-apple-file-system
[a-trash]: https://developer.apple.com/documentation/foundation/filemanager/trashitem(at:resultingitemurl:)
[a-presenter]: https://developer.apple.com/documentation/foundation/nsfilepresenter
[a-capacity]: https://developer.apple.com/documentation/foundation/urlresourcekey/volumeavailablecapacityforimportantusagekey
[a-reasons]: https://developer.apple.com/documentation/bundleresources/app-privacy-configuration/nsprivacyaccessedapitypes/nsprivacyaccessedapitype
[x-sonoma]: https://eclecticlight.co/2023/10/25/macos-sonoma-has-changed-icloud-drive-radically/
[x-icloud-forum]: https://developer.apple.com/forums/thread/722654
[x-fatbobman]: https://fatbobman.com/en/posts/advanced-icloud-documents/

## Superseded: the first draft

The first draft (c367b1b, revised in 8d86aa7) had three parts that are
not carried on. They are in the history for whoever wants them.

- **Its principles.** Eleven lines, among them "the program decides,
  the layer answers", "one function is one thing asked", and at first
  "DOS's values where the program decides or shows by them". They were
  drawn from `sys.h` and the program's calls, so they described a seam
  around one program on one system rather than the ground that the
  platforms have in common, and the user judged that the wrong starting
  point.
- **Its cross-cutting questions.** Eighteen, then some thirty after the
  first answers: values, paths and drives, names, text, the screen,
  keys, errors, time, programs, memory, what only DOS has, events, the
  types, the files and the names of the layer. Each was posed as how to
  carry today's DOS shape onto other systems. Three were answered and
  are under "Decided"; the rest are to be asked anew from what the
  platforms provide, area by area, where they still arise.
- **Its first group, files by their handle.** Create, open, close,
  read, write and seek, with the errors above or under the layer, first
  in DOS's values and then in standard C's types with an error list of
  the layer's own. It was shaped as a handle and a local path, which is
  `fopen`'s model: it reaches no file system but the local one, and the
  user wants iCloud and others. The file systems are the first area of
  the new survey.
