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
  page on DOS.

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
