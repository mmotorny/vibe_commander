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

## Decided

What the user has decided, in the user's words, with the date. Nothing
else in this document is decided.

- **The values of the layer** (question 1 of the first draft), 13
  October 2026: "The layer should speak standard C, unless Watcom C
  support is bad, then I would design our own, platform-agnostic
  vocabulary." No value of DOS's is the layer's: not its error numbers,
  its attributes, its packed dates, its key words or its cells. What
  "standard C" is to mean here is question S1, with what Open Watcom
  gives in section 3.
- **Paths and drives** (question 2 of the first draft), 13 October 2026:
  "Drives survive, but their meaning will be platform-specific. For
  example, DOS can continue exposing A: and C:, but iPhone
  implementation will have ⛁: for local files and ☁️: for iCloud.
  Slashes should be platform-native: back on DOS and Windows, forward
  on mac and ios." A drive is a name that the platform gives, which may
  be a character beyond ASCII; the separator is the platform's.
  Section 4.
- **Text** (question 4 of the first draft), 13 October 2026: "UTF-8
  inside." Section 5.

Not decided: the principles, which the user is reading (3 and 5 of the
first draft were overtaken by the decision on values and are written
anew below), and every question in section 6.

Contents: 1, what the layer is for, with the principles proposed; 2,
the inventory of `sys.h` and of what else the program asks; 3, what
standard C and Open Watcom give; 4, drives and paths; 5, UTF-8 inside;
6, the questions still open; 7, the order of the groups; 8, the first
group, proposed in full.

## 1. What the layer is for

The program is a faithful translation of an assembly program for DOS,
with its names and its behaviour, bugs included until each is decided.
The layer is about what that program asks of the system it runs on, and
not about rewriting it. The decisions on text and on drives do reach
into the program: how it holds text and paths changes, though not what
it does with them (sections 4 and 5). The layer is to let the same
common code run on the platforms in view:

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
3. **The layer speaks standard C and a vocabulary of its own, not
   DOS's.** Its types are those of `<stdint.h>`, `<stddef.h>` and
   `<stdbool.h>`; its text is UTF-8; where standard C has no word, or
   Open Watcom's is bad (section 3), it has its own, named for what a
   thing is on any platform: an error, a drive, a key, a date. The DOS
   implementation maps DOS's values to it and from it; what of DOS's
   values the program goes by inside is the program's, mapped at the
   edge of the common code (written anew 13 October 2026, by the user's
   decision on values).
4. **What a platform lacks it answers with the value that means none**,
   which the function's comment says: no unit shared, no mouse, no
   device error, version 0. A function that only asks whether a thing is
   there is made only where the program does something else without it.
5. **A failure is an error of the layer's own list that comes back,
   `SYS_OK` for none.** What a function answers besides is a member
   with a name, defined where nothing failed. What DOS happens to leave
   is no part of an answer (question C5).
6. **What crosses is the caller's.** A name or a buffer is lent for the
   call and no longer. Where the layer is to keep a pointer, the record
   of a search, the program's function for a device error, its function
   says so.
7. **The layer reads and writes nothing of the program's data**, `md`
   and the panels, and the program nothing of the machine's.
8. **No register, segment, selector, interrupt or type of a platform**
   (`off_t`, `termios`, a DPMI handle) in a header of the common code.
   Numbers are the types of `<stdint.h>` and `bool`, signed by the rule
   of `CLAUDE.md` unless there is a reason; a type is named only where
   its width differs by platform (questions S2 and S3).
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

The columns "Of DOS alone" and "Goes to" were written under the first
draft, when the layer was to speak DOS's values; the facts stand, and
where a function goes is now by the decisions above and sections 3 to
5: a DOS value that a column names is the DOS file's to map, not the
layer's to answer.


### Files

All asked with the program's errors but the three Bare ones (question
9).

| Function | Calls | What the program uses it for | Everywhere | Of DOS alone, and what a user sees | Goes to |
|---|---|---|---|---|---|
| `FileCreate` | 7 (stmain 1, copy 1, dirfunc 1, usermenu 1, edit 1, setup 1, treebox 1) | the setup file, a copy's target, the editor's file, the tree's file, the menu file, a label (attribute 8, `F54_00`) | yes | a label made by creating a file with attribute 8 | layer; the label to "Drives" |
| `FileOpen` | 4 (system 2, stmain 1, sthelp 1) | `OpenFil`, with its second try without sharing; the setup's help and file | yes | the sharing bits of the mode | layer |
| `FileOpenBare` | 1 (start 1) | the setup file at the start | yes | | layer, as `FileOpen` (question C4) |
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
| `FcbParse` | 2 (dirfunc 2) | a name made an FCB for deleting by a mask and for the label | no | all of it: deleting every file of a mask at one call, `F38_70`, and the label, `F54_00` | layer, none elsewhere; or a "delete by mask" and a "label" of the layer whose DOS file makes the FCB (group 9) |
| `FcbDelete` | 3 (dirfunc 3) | the same | no | as above, with the carry flag that DOS leaves (`flagged`) | as above |

### Directories

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `FindRecord` | 1 (system 1) | `SetDTA`: where a search puts what it finds, `md.DTA`, DOS's record | the state, yes | DOS's record of 43 bytes, kept by the system between two calls | layer, in another shape: a search answers its record (question C1) |
| `FindFirst` | 10 (copy 1, dirfunc 3, read 2, treebox 1, view 1, find 1, enter 1) | reading a panel, the tree, the search, a copy, the labels, the viewer's next file | yes, `opendir` and a match | 8.3 names, the attributes as a filter, a label as attribute 8 | layer |
| `FindNext` | 6 (find 1, read 1, treebox 1, dirfunc 3) | the same | yes | | layer |
| `DirMake` | 3 (copy 2, dirfunc 1) | F7, a copy of a directory | yes | | layer |
| `DirRemove` | 4 (copy 2, dirfunc 2) | delete, move | yes | | layer |
| `DirChange` | 11 (copy 1, dirfunc 3, commands 1, enter 3, drive 1, find 1, keys 1) | a panel's directory made current, Enter, the tree, a command run there | yes, one for the process | one for each drive | layer (section 4) |
| `DirCurrent` | 7 (find 1, drive 1, system 1, dirfunc 1, edit 1, main 1, read 1) | the path of a panel, of the editor's file, of the program's | yes | one for each drive, without its drive and first backslash | layer (section 4) |
| `DirCurrentBare` | 2 (exec 1, start 1) | the prompt between commands; the start | yes | | layer, as `DirCurrent` |

### Drives

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DriveCurrent` | 7 (stmain 1, find 1, system 3, start 1, exec 1) | `TstDrv`, whether a drive exists; `SetPth`; the prompt; the search | as one drive | all drives (section 4) | layer |
| `DriveSelect` | 10 (drive 1, keys 2, commands 1, copy 2, find 1, main 1, system 2) | `TstDrv`; a panel's drive made current; Go to | as one drive | | layer |
| `DriveSpace` | 2 (copy 1, read 1) | a copy's room; the Info panel's free space | yes, `statvfs` | in clusters and sectors, with `flagged` | layer, in bytes (group 3) |
| `DriveLetter` | 2 (drive 1, system 1) | A: and B: on one floppy drive: the drive menu, the prompt "Insert disk for drive B:" (`Fantom`) | no | the prompt and its setting, kept while DOS is a target (user, 13 October 2026) | layer, none elsewhere: "shares no unit" |
| `DriveSetLetter` | 5 (drive 1, system 3, main 1) | the same, and the box of a drive not ready | no | as above | layer, none elsewhere |
| `DriveRemote` | 1 (find 1) | which drives the file search goes through for "all drives" | as a mount's kind | | layer, or one function "what kind of drive" (group 3) |
| `DriveFixed` | 1 (find 1) | the same | as above | | as above |
| `DriveCd` | 1 (find 1) | the same, by MSCDEX | as above | the CD-ROM's own question, INT 2Fh | as above |

### Memory

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MemTake` | 5 (start 5) | the panels and the room, at the start and after a command | yes | paragraphs and a segment until F9; "how large a block can be" | layer, in bytes and pointers (question C13) |
| `MemGive` | 1 (start 1) | the room given back before a command | yes | | layer |
| `MemResize` | 3 (view 3) | the room cut and grown around a viewer of another's | yes | | layer |
| `MemSetSearch` | 1 (exec 1) | `SetStat`: DOS to look for the lowest block, as it does when it starts | no | none on the screen: where DOS puts a command | layer, none elsewhere, or DOS file (question C14) |
| `MemSetUpper` | 1 (exec 1) | the same, the upper memory out | no | as above | as above |
| `MemOwn` | 1 (exec 1) | `Alloc3`: the program's block made as long as it is | no | "not enough memory" at the start | as above |

### The time, the country, the system itself

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `TimeOfDay` | 6 (input 4, main 1, mouse 1) | the clock on the screen, the times of a key, a double click | yes | to the hundredth, packed in bytes | layer |
| `CountryInfo` | 1 (start 1) | how dates, times and numbers are written: `md.Country` | yes, from the locale, mapped | DOS's record, with a far address of its case map in it | layer, in a record of the layer's own (group 2) |
| `CaseMap` | 2 (start 1, system 1) | the table of capitals of 80h to FFh at the start; `UpCase`, every byte of a search for a text | yes, by the code page | DOS's routine through the extender, slow (user, 13 October 2026) | layer, the code page's capitals (question U4) |
| `SystemVersion` | 2 (start 1, exec 1) | `Alloc3`: refuse a DOS before 3; the version noted | no | the message and the end on an old DOS | layer, none elsewhere: "new enough" |
| `SystemTrueVersion` | 1 (start 1) | `VerDOS` noted, which nothing in the portable C reads | no | nothing | DOS file, or gone with `VerDOS` (a deviation of order, to ask) |
| `BreakCheck` | 1 (exec 1) | `SetBrk`: what DOS was set to, put back at the end | no | Ctrl-C checked at every call | layer, none elsewhere (question C14) |
| `SetBreakCheck` | 3 (exec 3) | the same, around commands | no | as above | as above |
| `Idle` | 1 (blank 1) | each time the program looks for a key and finds none: INT 28h | yes, as a short wait | other programs of DOS get a turn; the harness's driver knows a step is over | layer: the one place where a POSIX program sleeps |

### Other programs, and the end of this one

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ProgramRun` | 1 (view 1) | `Aexec0`: a viewer or an editor of another's | not on iOS | DOS's tail: a count byte, the text, a carriage return | layer (question C12) |
| `ProgramRunBare` | 1 (exec 1) | `Exec`: COMMAND.COM for the commands | as above | COMMAND.COM and `/C` built by the program | layer, as `ProgramRun` (question C12) |
| `ProgramEnd` | 3 (exec 3) | the program leaves | yes | | layer |
| `ProgramEndOld` | 1 (exec 1) | the end on a DOS before 3 | no | | layer, none elsewhere, or with group 9 |
| `ProgramTail` | 1 (main 1) | the path on the command line that the program was started with | yes, `argv` | the tail as DOS keeps it | layer, as the arguments (question C12) |
| `ProgramEnvironment` | 3 (exec 1, start 2) | `COMSPEC`; the program's own path behind the environment, read as 68 bytes by `MnPth0`, for where its files are | yes, `environ` and the path of the program | DOS's block, and the path behind it | layer, by name: a variable, the program's path (question C12) |

### What the system asks the program

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `DeviceErrorFn` | type | `FtErr1`, the box with Retry and Abort | no | the critical error: a drive without a disk, a disk that is protected | layer: the program's function, which another platform never calls |
| `DeviceErrorNote`, `DeviceErrorTake`, `DeviceErrorGive` | 1 (main 1), 1 (main 1), 1 (main 1) | `HookVct` and the end | no | | layer, none elsewhere |
| `IntOn` | 3 (box 1, window 2) | behind what goes to the screen: the box of the critical error gets its keys | no | | layer, none elsewhere, or DOS file (question C15) |
| `CtrlBreakNote`, `CtrlBreakTake`, `CtrlBreakGive` | 1 (main 1), 1 (main 1), 1 (main 1) | Ctrl-Break taken, so that nothing happens for it: the key 0 that it leaves is read as a stop by `TstBrk` | as SIGINT, in part | the key of a PC | layer (question C15) |
| `CtrlCTake` | 1 (exec 1) | `SetBrk` | as SIGINT | | layer (question C15) |
| `ProgramEndedBreakCheck`, `ProgramEndedTake`, `ProgramEndedGive` | 1 (exec 1), 1 (exec 1), 1 (exec 1) | DOS ending the program by another's hand: what DOS is to put back | no | the scenario `ended` | layer, none elsewhere; or DOS file entirely, said by `SetBreakCheck` (group 8) |

### The console

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ConsoleChar` | 1 (start 1) | the space that DOS writes at the start | yes, as the terminal before the screen is taken | | layer, with the screen (group 5) |
| `ConsoleText` | 2 (start 1, stmain 1) | the banner at the start; the setup's line for a file not written | as above | ended by `$` | as above, ended by a zero (group 5) |

### The keyboard

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `KeyWaiting` | 6 (blank 1, find 1, input 2, system 2) | `Input0`, `TstBrk`, `ClrKbd`, the blanker, the search | yes, by parsing the terminal | the BIOS's key word | layer (question C8) |
| `KeyRead` | 8 (blank 1, find 1, exec 1, input 2, start 1, system 2) | the same | yes | | layer |
| `KeysHeld` | 6 (blank 2, menu 1, bars 1, input 2) | the key bar of Shift, Ctrl and Alt held; the blanker; the menu | not in a terminal | the bits of the BIOS's data | layer, none elsewhere: "none held" (question C8) |
| `KeyForgetIns` | 1 (keys 1) | Ins held down selects file after file | no | the BIOS's Ins bit | layer, none elsewhere |

### The screen

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `ScreenState` | 11 (window 4, blank 3, main 1, stmain 1, start 2) | the mode, the columns, the page shown | as "colour or not" and a size | modes and pages | layer, as the screen's size and kind (group 5) |
| `ScreenSetMode` | 2 (start 2) | the start: mode 3 if the mode is not of text | no | | DOS file, or none elsewhere |
| `CursorWhere` | 8 (stmain 1, blank 1, main 1, start 2, view 1, window 2) | where the command line goes, the cursor kept and put back | yes | the shape in lines of a character | layer |
| `CursorTo` | 3 (blank 2, window 1) | the cursor of an input line; the blanker | yes | | layer |
| `CursorShape` | 8 (stcolor 1, stmain 1, blank 2, start 2, window 2) | hidden, a line or a block (Ins) | in part: shown or not, a bar or a block | lines of the character | layer |
| `ScreenScroll` | 2 (blank 1, window 1) | `Scroll` of a window by a line; the blanker clears | yes | the BIOS's call 6 and 7 | layer, or the program's own on its buffer (group 5) |
| `ScreenCell` | 2 (start 2) | the colour at the cursor when the program starts, `CrntCol` | only of what the program put | reading DOS's screen | layer (question C11) |
| `ScreenFill` | 2 (blank 1, start 1) | a star of the blanker; the cell put back at the start | yes | | layer, or `ScreenPut` (group 5) |
| `ScreenType` | 13 (exec 3, stmain 2, enter 2, main 1, start 4, system 1) | the command line, the prompt, the line feeds between commands, the bell | yes, as a terminal's output | the BIOS's teletype, with a page and a colour as they happen to be | layer (question C11) |
| `ScreenEga` | 1 (start 1) | whether the display may have more than 25 rows | no | | layer, as the rows of the screen (group 5) |
| `ScreenFont` | 1 (start 1) | how many rows the display has | no | the BIOS's font | as above |
| `ScreenPageStart` | 1 (window 1) | `ScrOffs`: where the cells of the page are | no | | DOS file |
| `ScreenMemory` | 1 (window 1) | the same | no | `md.Screen`, kept by `dos32.c` in the main data | DOS file |
| `ScreenPut` | 4 (window 4) | `Window`, `MovScr`: the program's buffer to the screen | yes | the display's memory | layer |
| `ScreenGet` | 1 (window 1) | `SavScr`: the screen read back after a command | only of what the program put | the screen that DOS and the commands share | layer (question C11) |

### The mouse

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `MouseFind`, `MouseReset` | 2 (mouse 1, stmain 1), 2 (mouse 1, stmain 1) | whether there is a mouse; the setup sets it anew | yes, in a terminal that reports one | the driver's answers, 21h for an old one | layer |
| `MouseShow`, `MouseHide` | 1 (mouse 1), 1 (mouse 1) | the pointer off while the screen is written | yes, as nothing to do where the terminal draws it | a count of hides | layer |
| `MouseWhere` | 2 (mouse 2) | the buttons and the place, polled | yes | in dots, 8 to a cell | layer (question C8) |
| `MouseTo`, `MouseRows` | 1 (mouse 1), 2 (mouse 1, stmain 1) | the pointer put in the middle; the rows it may go to | not in a terminal | | layer, none elsewhere |
| `MousePointer` | 2 (mouse 2) | the pointer's look, by the colours it turns | not in a terminal | | layer, none elsewhere |

### The timer

| Function | Calls | What for | Everywhere | Of DOS alone | Goes to |
|---|---|---|---|---|---|
| `Ticks` | 1 (system 1) | `Timer`: the blanker, a box that grows, the waits, double clicks | yes, from a monotonic clock | 18.2 a second, from midnight, read in the BIOS's data | layer (question C6) |

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
  asks of the layer's design"). Group 7.
- **Fields of the main data that the DOS file keeps.** `md.Screen`, where
  the cells are, written by `ScreenMemory`, and `md.ErrDat`, which
  `Ask` clears and reads around every call that is asked with the
  program's errors. Both break principle 7. `ErrDat` goes with question
  9, `Screen` into the DOS file with group 5.
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
  build defines (question C16).

## 3. What standard C and Open Watcom give

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

So the standard library as the layer, way (b) of question S1, is a
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

So, by the user's condition, the layer's vocabulary is standard C's
types and macros, and its own words for errors, text, time, drives,
keys and the screen. Question S1.

## 4. Drives and paths

### What the program does with drives and paths today

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

### What is proposed

- **A drive is a name**, UTF-8, that the platform gives: `A` to `Z` on
  DOS, `⛁` and `☁️` on iOS, `/` alone or the mounts on Linux (question
  D1). A path is the drive's name, a colon, and the rest, with the
  platform's separator: `C:\WORK\A.TXT`, `☁️:/Notes/a.txt`. The colon
  stays the program's mark of where the drive's name ends, so that the
  program can find it in a path without asking.
- **The layer lists the drives**, in its order, each with its name and
  its kind (floppy, fixed, network, CD-ROM, cloud), and answers whether
  one is there. On DOS the list is what `TstDrv` finds, A to Z, by the
  same calls in the same order: the program asks drive after drive, as
  it does, and the layer says what the next one is called.
- **The current directory of a drive** stays a question of the layer,
  as DOS has one for each drive. A platform with one for the whole
  program keeps a table of its own (question D2).
- **The separator is a macro that each platform defines**, `SYS_SEP`,
  `'\\'` on DOS and Windows, `'/'` on macOS, iOS and Linux, Chromium's
  way. Joining, splitting and the parent of a path are a few functions
  of the common code that go by it; no system is asked (question D3).
- **The limits**: a path and a name are as long as the platform has
  them, `SYS_PATH_MAX` and `SYS_NAME_MAX` by the platform's build, 67
  and 12 on DOS, so that the DOS program's fields, screens and setup
  file are what they are (question D4). Lifting them on DOS too, with
  long names by DOS's calls for them (`doslfn3r.lib` is in the
  download), would be a deviation of its own.
- **What the DOS implementation does so that every scenario shows what
  it shows**: its drives are A to Z by the calls of today; its
  separator is `\`; its limits are 67 and 12; and it hands DOS the same
  bytes, the names converted from UTF-8 to the code page (section 5),
  which give back the same bytes that came. The floppy that A: and B:
  share stays, with its prompt, as the user decided: the layer answers
  for a drive which other one shares its unit, by name.

### How large

The drive's arithmetic, `- 'A'` and its like, is some 90 places in 20
files, each written anew as a lookup of the drive's name or a call; the
separator, some 60 places of the code and the tables, made `SYS_SEP`;
the colon stays. The fields of 13 and 68 bytes keep their sizes on DOS
by the macros. A reading of each place, a few days of work, judged by
the scenarios, which see every one of them through the screens.

## 5. UTF-8 inside

### What it touches

- **The screen's cells.** `ScrBuf` holds a cell as 16 bits, a byte of
  CP866 or 437 and a colour. A cell has to hold a character, a code
  point of 21 bits, and a colour: 32 bits, `ScrBuf` twice as long. Every
  routine that writes text into the cells counts a byte as a column
  today: `line.c`, `box.c`, `panel.c`, `dialog.c`, `menu.c`, `bars.c`,
  `help.c`, `info.c`, `tree.c`, `input.c` and the input lines, `view.c`,
  `edit.c`, `find.c`, the setup's.
- **How wide a character is.** One cell on DOS. On iOS and macOS a
  character may take two, the user's `☁️` among them (question U3).
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
  as today (question U4).
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
  encoding that the file has (question U2).
- **The setup file**, held byte for byte against the original's: it is
  the main data's first bytes as they lie, paths of 68 bytes among
  them. With UTF-8 inside it cannot be a copy of the memory any more;
  it is written field by field, with the paths converted to the code
  page on DOS.

### Where the conversion is, on DOS

At the layer's edge, by one table of the code page, CP866 or 437, which
the DOS implementation takes from DOS's code page when it starts. The
table maps all 256 bytes to characters and back, the bytes below 20h
and 7Fh to the glyphs that a PC's display shows for them (`☺`, `♪`,
`⌂`), so that it is one to one: a byte that DOS hands over comes back
as the same byte. Then the DOS program shows the same cells, types the
same characters, hands DOS the same names and writes the same setup
file, and every golden holds.

A character that the code page lacks can come only from the program's
own texts or from another platform's files, never from DOS. It becomes,
on DOS: on the screen, the code page's character for an unknown one,
`?`, in the cell's colour; in a name handed to DOS, an error of the
layer's, "a name that this system cannot hold", and DOS is not asked
(question U1).

### How large, and where on the road

It is a rewrite of how the common code holds text, not of what it does:
the cell, every routine that writes text, every buffer of a name or a
path, the tables' texts, the markup, the setup file, the input lines'
editing, which moves by characters and not bytes. Some 40 of the 66
files, two to four weeks of pieces by agents at the pace that step 2
has had, each judged by the scenarios, whose goldens do not change. It
is larger than any piece of step 2.

Step 4 moves the DOS implementation under the layer, so the common code
has to speak the layer by then: the work falls between the design's
approval and step 4, as a step of its own (question U5). It can be done
in stages that each leave the program whole: the cell holds a character
and the layer converts at the screen; then the keys; then the program's
texts and names; then the setup file.

## 6. The questions still open

Each is short enough for a dialog; the recommendation is first, with its
reason. The first draft's questions 1, 2 and 4 are decided and gone; 3,
8, 10, 11 and 16 are changed by the decisions; S, D and U are new.

### Standard C

**S1. Which standard C.**

> A (recommended): the layer's types and macros are standard C's
> (`int32_t`, `int64_t`, `bool`, `SEEK_SET`), its functions are ours,
> and the DOS build links the library's routines that need no start-up
> (`memset`, `strcmp`, 64-bit helpers). B: the standard library is the
> layer where it reaches (`fopen`, `remove`, `time`, `getenv`), ours
> elsewhere. C: as A, linking nothing.

Why A: B changes what DOS is asked at every file, by the library's
buffers and start-up (section 3), and reaches only files, time and the
environment; A keeps DOS's calls and uses what of Watcom is good.

**S2. Counts and sizes in standard C's types.**

> A (recommended): a count of bytes and a size in memory are `int32_t`,
> signed by the project's rule; a size or place of a file, and a disk's
> room, `int64_t`; no `size_t` in the layer. B: `size_t` for counts and
> sizes in memory, as the standard library has them, unsigned and as
> wide as the platform. C: `ptrdiff_t`, signed and as wide as the
> platform.

Why A: the program never asks for 2 GB at once, and a signed count of
one width everywhere is what the user's rule asks; `size_t` is the one
standard type whose width differs, and it is unsigned.

**S3. The tree's own names of the types.**

> A (recommended): the layer's headers say `int32_t` and `uint8_t`; the
> common code keeps `u8`, `u16`, `u32`, `i32` as they are, and adds
> `i64`, all made of `<stdint.h>`'s in `vc.h`. B: the common code moves
> to `<stdint.h>`'s names too, a sweep of every file.

Why A: the names are the same types, so nothing is to gain at a call;
B is a sweep of thousands of lines for the look of it, which can come
any time.

### Drives and paths

**D1. How the layer names a drive.**

> A (recommended): by its name, UTF-8 text that the platform gives
> (`C`, `⛁`, `☁️`); a path begins with it and a colon, and the program
> stores and shows it as text. B: by its number in the layer's list,
> with a name to show; a path keeps the number.

Why A: a drive's name is what a path, the setup file and the history
hold and what the user sees; a number moves when drives come and go.

**D2. The current directory of each drive.**

> A (recommended): the layer keeps one for each drive, as DOS does:
> DOS's on DOS, a table of the implementation's on a platform that has
> one for the whole program. B: the program keeps them itself, and the
> layer has one.

Why A: DOS is asked for each drive's directory today, in the original's
order; with B those calls go, a deviation.

**D3. Joining and splitting paths.**

> A (recommended): functions of the common code that go by the
> platform's `SYS_SEP` and the colon; no system is asked. B: functions
> of the layer, each platform's own.

Why A: a path is text with a separator, the same on every platform but
for the character, and functions of the common code are tested once.

**D4. The limits of a path and a name.**

> A (recommended): as long as the platform has them, `SYS_PATH_MAX` and
> `SYS_NAME_MAX` of its build, 67 and 12 on DOS, so the DOS program is
> as it is. B: lifted everywhere now, DOS refusing what it cannot hold,
> a deviation on DOS.

Why A: the setup file, the panels' fields and their screens stay the
original's on DOS; the sizes are what a platform macro is for.

### Text

**U1. A character that DOS's code page lacks.**

> A (recommended): on DOS's screen it is `?` in the cell's colour; in a
> name handed to DOS it is an error of the layer's, "a name this system
> cannot hold", and DOS is not asked. B: the name is handed with `?`
> in its place, and DOS answers what it answers.

Why A: such a character can only come from the program's own texts or
another platform's files, and a name that is not the one meant must not
reach DOS.

**U2. What is text and what is bytes.**

> A (recommended): the program's texts, names, paths and keys are
> UTF-8; a file's contents are the file's bytes, which the viewer and
> the editor show by the platform's code page, CP866 on DOS. The setup
> file on DOS keeps the original's bytes. B: every file is converted to
> UTF-8 when it is read.

Why A: the viewer's hex mode and the editor's counts are of the file's
bytes, and the scenarios judge them; a file's encoding is the file's.

**U3. Characters wider than a cell.**

> A (recommended): a cell holds one character; text is measured in
> columns by one function of the common code, one column a character
> until a platform with wide characters is made, which changes that one
> function. B: wide characters now, with a cell that continues the one
> before.

Why A: DOS has none, and with every measure in one place they cost
nothing to add later; the user's `☁️` needs them on iOS.

**U4. Capitals and the order of names.**

> A (recommended): asked of the layer, a character's capital and which
> of two characters comes first: DOS asks DOS's case map and goes by the
> code page's bytes, as today; POSIX by Unicode. B: tables of Unicode in
> the common code, the same on every platform.

Why A: the user decided that a search asks DOS for every capital as the
original does, and the order of names in a panel is the code page's,
which the scenarios see.

**U5. Where UTF-8 inside goes on the road.**

> A (recommended): a step of its own, after the layer's design is
> approved and before step 4, in stages: the cell holds a character and
> the layer converts at the screen; the keys; the texts and names; the
> setup file. B: it is part of step 4.

Why A: it is larger than any piece of step 2, and each stage can land
with the goldens as they are.

### The other questions, brought up to date

**C1 (was 3). Names longer than 8.3.**

> A (recommended): settled by D4 on DOS; on another platform the
> program's fields are `SYS_NAME_MAX` long, and a search answers a
> record of the layer's with the whole name. B: POSIX gives short
> aliases, `LONGNA~1`, as Windows did.

Why A: with the limits a platform's, nothing about DOS changes, and no
platform after it needs aliases.

**C2 (was 8). Errors.**

> A (recommended): an enumeration of the layer's own, a member for each
> error that the program tells apart (no file, no path, no access, not
> ready, shared, no more files, no drive, the current directory, disk
> full, bad name, other); the DOS file maps DOS's numbers to it. B:
> POSIX's names of `errno`, which Watcom has with other numbers, and
> ours for what POSIX has not.

Why A: the program goes by distinctions that `errno` does not make, a
drive not ready above all, and the layer's list says exactly which.

**C3. The program's own numbers of errors.**

> A (recommended): the program's wrappers map the layer's error back to
> the DOS number that the program compares, some 45 places, which stay
> as they are until the program is rewritten there. B: those places
> compare the layer's enumeration now.

Why A: it is a table of a dozen lines in the common code, and the
comparisons by range, `<= 3`, stay as the original has them.

**C4 (was 9). The program's errors: above the layer or under it.**

> A (recommended): the box for a drive not ready, the error of a call
> given up and the extended error behind a failure go up into the
> program: the layer has one plain function a thing, and one that says
> the last error. B: they stay under the layer, handed the program's
> flag.

Why A: what is done about a failure is the program's; under the layer
every platform repeats it.

**C5 (was 10). What DOS leaves that callers go on with.**

> A (recommended): it goes. Each caller that reads what a failed call
> left, or what a call that answers nothing left, looks at the error
> first: a deviation of each, which no scenario shows. B: the DOS file
> answers it in a member that only DOS fills.

Why A: a member that only DOS fills is DOS's vocabulary in the layer,
which the decision on values excludes. The places are few: the seek of
`view.c` and `copy.c` (section 8).

**C6 (was 11). Time.**

> A (recommended): a date and time of the layer's, year, month, day,
> hour, minute, second, millisecond, for a file and for the time of
> day; a monotonic clock in milliseconds, `int64_t`. DOS packs and
> unpacks, and works milliseconds from its ticks. B: `time_t` and
> `struct tm` of standard C.

Why A: Watcom's `time_t` is unsigned and of 32 bits, so standard C's
time is bad there; `struct tm` is near A and could be taken in its
place.

**C7. The program's waits in milliseconds.**

> A (recommended): the program's waits, the blanker, a double click,
> a growing box, are counted in milliseconds, each written so that on
> DOS it falls on the same tick as today, by reading each. B: the
> program keeps its ticks and converts the layer's milliseconds back,
> at one place.

Why A: a tick is DOS's; B is cheaper and keeps the tick inside the
program, and is right if A's reading is more than it is worth.

**C8 (was 7). Keys.**

> A (recommended): a key of the layer's own: which key (an enumeration,
> F1 to F12, arrows, Ins and so on), what was held with it, and its
> character. The program makes of it the BIOS word that its tables go
> by, in one function of the common code. B: the program's tables are
> rewritten on the layer's keys.

Why A: hundreds of cases stay as they are, and the BIOS word lives in
the common code alone.

**C9 (was 7). The mouse.**

> A (recommended): in cells, row and column. DOS multiplies and divides
> by eight. B: in the platform's own units, with the size of a cell.

Why A: the program goes by cells; a text screen's driver answers in
eights, which is to be seen on the bench before it is built.

**C10 (was 5). The screen.**

> A (recommended): a grid of cells, a character and a colour each, put
> and read, a cursor, and the rows and columns that the platform has; a
> colour is one of the 16 of a PC, as the program's tables have them.
> B: a higher-level interface, text and boxes.

Why A: the program draws into its own buffer and copies it; a grid is
what a terminal and a view on iOS can draw.

**C11 (was 6). The screen that commands share.** Unchanged:

> A (recommended): on POSIX the panels are on the terminal's second
> screen and the commands on the first; reading answers what the
> program last put; Ctrl-O switches. B: commands under a terminal
> emulated in the layer. C: decided when POSIX runs.

**C12 (was 12). Other programs, the shell, the environment.**

> A (recommended): the layer runs a file with its arguments as one text
> and answers by name the arguments, a variable, the program's own path,
> and the shell with its switch; DOS's blocks stay in the DOS file. B:
> DOS's blocks cross, and POSIX makes them.

Why A: the layouts are DOS's, which the decision on values keeps out.

**C13 (was 13). Memory.**

> A (recommended): blocks as pointers, sized in bytes, `int32_t`: take,
> give back, resize, as `malloc`, `free` and `realloc` have them, and
> how large a block can be. B: one arena from the layer, the program's
> own allocator in it.

Why A: the program's room is "the largest block there is", which a
platform has to say; separate blocks let a sanitizer see a panel run
over.

**C14 (was 14). What only DOS has.** Unchanged:

> A (recommended): each stays a function of the layer that another
> platform answers with none, so DOS's calls keep their order. B: each
> goes into the DOS file, called from a hook.

**C15 (was 15). What the system brings in.** Unchanged:

> A (recommended): the device error is a function of the program's that
> the layer calls during a call it was asked; Ctrl-Break and Ctrl-C are
> taken as today; nothing else arrives unasked. B: events in a queue
> read with the keys.

**C16 (was 17). The files of the layer.** Unchanged:

> A (recommended): a header a group, `sys_file.h` and so on, a file a
> group and platform, `sys_file_dos.c`, `sys_file_posix.c`,
> `sys_file_stub.c`, the platform's macros, `SYS_SEP` and the limits
> among them, in `sys_config_dos.h` and its like. B: one header, one
> file a platform.

**C17 (was 18). The names of the layer.** Unchanged:

> A (recommended): `Sys` and `SYS_` in front of everything of the layer;
> the program's wrappers keep today's names. B: today's names for the
> layer.

The first draft's question 16, the types, is S2 and S3.

## 7. The order of the groups

To be designed with the user one at a time, from the group that teaches
the most of the questions above at the least risk:

1. **Files by their handle**, with the errors: S1 to S3, C2 to C5, C16,
   C17, and of text U1 for the names it takes.
2. **The time, the country and idle**: C6, C7, and of text U4.
3. **Names, directories and drives**: D1 to D4, C1, U2.
4. **Keys and the mouse**: C8, C9.
5. **The screen and the console**: C10, C11, U3; the rows that entry 12
   of `DEVIATIONS.md` waits for.
6. **Memory**, when F9 has landed: C13.
7. **Other programs, the start and the end**: C12.
8. **What the system brings in**: C15.
9. **What is left of DOS's own**: C14, item by item.

Directories and drives are one group now: a drive is no number any
more, and the two are one model.

## 8. The first group, proposed in full: files by their handle

Chosen by section 7: it carries the errors and the types, which every
other group takes, and asks of paths only that a name is lent as text.
It is written as if S1 to S3, C2 to C5, C16 and C17, D4 and U1 were
answered as recommended; where an answer goes another way, the end of
the section says what changes.

What it covers of `sys.h` today: `FileCreate`, `FileOpen`,
`FileOpenBare`, `FileClose`, `FileCloseBare`, `FileRead`,
`FileReadBare`, `FileWrite` and `FileSeek`, nine functions with 80
calls in 14 files (section 2), and the part of `Ask` of `dos32.c` that
is the extended error. Piece S of step 2 changes their counts to
`i32` first; this goes on from there.

### The header, `sys_file.h`

```c
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>      /* SEEK_SET, SEEK_CUR, SEEK_END: macros only */

/*
 * Files, by the handle that opening one gives. A name is a path of
 * the platform's, UTF-8 text ended by a zero byte, with its drive and
 * the platform's separator (sys_path.h); it is lent for the call. A
 * buffer is the caller's, and the layer keeps no pointer to it.
 *
 * A function that may fail answers `error`: SYS_OK if it did not, else
 * an error of enum SysError (sys.h). What it answers besides is
 * defined only where `error` is SYS_OK.
 */

/* The errors, each one that the program tells apart. */
enum SysError {
    SYS_OK = 0,
    SYS_ERR_NO_FILE,        /* no such file */
    SYS_ERR_NO_PATH,        /* a directory of the path is not there */
    SYS_ERR_TOO_MANY,       /* no more files can be open */
    SYS_ERR_NO_ACCESS,      /* refused: read-only, a directory, ... */
    SYS_ERR_BAD_HANDLE,
    SYS_ERR_NO_DRIVE,       /* the drive is not there */
    SYS_ERR_CURRENT_DIR,    /* the current directory cannot go */
    SYS_ERR_NO_MORE,        /* the end of a search */
    SYS_ERR_SHARED,         /* another has the file open */
    SYS_ERR_NOT_READY,      /* a device not ready, given up */
    SYS_ERR_DISK_FULL,
    SYS_ERR_EXISTS,
    SYS_ERR_BAD_NAME,       /* a name that this system cannot hold */
    SYS_ERR_UNSUPPORTED,    /* a thing that this system has not */
    SYS_ERR_OTHER
};

/* How a file is opened: one of the three, and what others may do
   meanwhile, which a platform without sharing does not look at. */
#define SYS_OPEN_READ       0x01
#define SYS_OPEN_WRITE      0x02
#define SYS_OPEN_BOTH       0x03
#define SYS_SHARE_ALL       0x10    /* others may read and write */
#define SYS_SHARE_READ      0x20    /* others may read */

struct SysOpened {
    enum SysError error;
    int32_t file;                   /* the handle */
};

struct SysMoved {
    enum SysError error;
    int32_t count;                  /* how many bytes were read or
                                       written */
};

struct SysPlace {
    enum SysError error;
    int64_t place;                  /* from the start of the file */
};

/* Make a file, or make the one that there is empty, and open it to
   read and write. `read_only` makes it read-only once it is closed. */
struct SysOpened SysFileCreate(const char *name, bool read_only);

struct SysOpened SysFileOpen(const char *name, int32_t how);

enum SysError SysFileClose(int32_t file);

/* Up to `count` bytes to `to`, from where the file is: fewer at its
   end, 0 there. */
struct SysMoved SysFileRead(int32_t file, uint8_t *to, int32_t count);

/* `count` bytes from `from` at where the file is: fewer on a disk that
   is full, which is no error. A count of 0 cuts the file off there. */
struct SysMoved SysFileWrite(int32_t file, const uint8_t *from,
                             int32_t count);

/* Go `step` bytes from SEEK_SET, SEEK_CUR or SEEK_END; the place then. */
struct SysPlace SysFileSeek(int32_t file, int64_t step, int whence);

/* What the system says of the last call that failed, more closely than
   that call's answer: DOS's extended error. Asked behind a failure,
   before anything else is asked. */
enum SysError SysLastError(void);
```

`SysFileCreate` takes `read_only` and no other attribute: the program
creates with 20h, archive, or with the attributes that a file had,
which group 3 is to say how a platform keeps; and with 8, a label,
which is a call of "the drives" there, not of files. Until group 3,
this takes whatever the attributes are with a member more (question
F3).

### What the program keeps above it

In `system.c`, the program's own functions under today's names, as in
the first draft: `md.ErrDat` cleared, the layer asked, and behind a
failure that no device error has given up, `SysLastError`. The answer
is `struct Answer` of today, with the error made the DOS number that
the program compares, by a table of the common code (question C3), so
that no caller changes:

```c
/* The DOS number that the program compares, for an error of the
   layer's: what DOS's extended error has for it. */
static u16 DosNumber(enum SysError error)
{
    switch (error) {
    case SYS_ERR_NO_FILE:     return 0x02;
    case SYS_ERR_NO_PATH:     return 0x03;
    case SYS_ERR_TOO_MANY:    return 0x04;
    case SYS_ERR_NO_ACCESS:   return 0x05;
    case SYS_ERR_BAD_HANDLE:  return 0x06;
    case SYS_ERR_NO_DRIVE:    return 0x0F;
    case SYS_ERR_CURRENT_DIR: return 0x10;
    case SYS_ERR_NO_MORE:     return 0x12;
    case SYS_ERR_SHARED:      return 0x20;
    case SYS_ERR_DISK_FULL:   return 0x27;
    case SYS_ERR_EXISTS:      return 0x50;
    case SYS_ERR_NOT_READY:   return 0x53;
    case SYS_ERR_UNSUPPORTED: return 0x01;
    default:                  return 0x1F;
    }
}
```

The DOS file maps the other way, and for the program to go on as it
does, every number that the program tells apart has its own member: 1,
2, 3, 5, 0Fh, 10h, 12h, 20h and 53h, read from its comparisons, and
`<= 3` and `> 3` of them. A number of DOS's that has no member comes
back as 1Fh; the program shows no number, and decides alike for all
above 3 that it does not name, which is to be held by reading each
comparison when it is built (section "Unsure").

### What each function does, on each platform

| Function | DOS | POSIX | The stub |
|---|---|---|---|
| `SysFileCreate` | the name to the code page (U1); DOS 3Ch with the attributes that the program had, as F3 says | `open(name, O_RDWR \| O_CREAT \| O_TRUNC, read_only ? 0444 : 0666)` | `SYS_ERR_UNSUPPORTED`, file -1 |
| `SysFileOpen` | DOS 3Dh, AL made of `how`: the access and the sharing bits as DOS has them, 40h and 20h for the two | `open` with `O_RDONLY`, `O_WRONLY` or `O_RDWR`; the sharing is not looked at | as above |
| `SysFileClose` | DOS 3Eh | `close` | `SYS_ERR_UNSUPPORTED` |
| `SysFileRead` | DOS 3Fh, as many calls as the count needs, each at most what DOS takes | `read`, again on `EINTR` and until the count or the end | as above, count 0 |
| `SysFileWrite` | DOS 40h likewise; a count of 0 is one call of 0 | `write` likewise; a write cut short by a full disk answers its count and no error; a count of 0 is `ftruncate` at `lseek(file, 0, SEEK_CUR)` | as above |
| `SysFileSeek` | DOS 42h, AL `whence`, CX:DX the low 32 bits of `step`, the place DX:AX | `lseek` in 64 bits; a place below 0 is `SYS_ERR_OTHER` | as above, place -1 |
| `SysLastError` | DOS 59h, BX 0; the number mapped | the last `errno` that a function kept where it failed, mapped: `ENOENT` is `NO_PATH` where a `stat` of the directory part fails and `NO_FILE` else; `EMFILE`, `ENFILE` `TOO_MANY`; `EACCES`, `EPERM`, `EROFS`, `EISDIR` `NO_ACCESS`; `EBADF` `BAD_HANDLE`; `EBUSY`, `ETXTBSY` `SHARED`; `ENOSPC` `DISK_FULL`; `EEXIST` `EXISTS`; `ENAMETOOLONG`, `EILSEQ` `BAD_NAME`; anything else `OTHER` | `SYS_ERR_UNSUPPORTED` |

The DOS functions ask DOS what `dos32.c` asks today, the same calls in
the same order with the same registers, the extended error asked behind
a failure as `Ask` asks it. One thing is handed otherwise: today 59h
gets CX, DX, SI and DI as the failed call left them, and `SysLastError`
hands 0 there: a row of entry 6, of the kind that the user decided on
13 October 2026.

### Which callers change, and how much

- With C3 and C4 answered A: none at its call, but the three Bare ones
  of `start.c`, which call the layer and make its answer the program's.
  `system.c` has six functions and the table more.
- The places that go by what DOS left (C5): the seek of `view.c`, which
  holds the place of a failed seek against the place it asked for,
  twice, and of `copy.c`, which takes it for a size, three times. Each
  looks at the error first; on DOS a seek of an open file fails only for
  a bad handle, and no scenario shows it. No caller reads what a close
  left where it did not fail: `ReDr00` goes by the read's answer since
  the user's decision of entry 6, and `dirfunc.c` reads a close's value
  only where the close failed.
- The names that these functions take become UTF-8 when the program's
  names do (U5); until then the program's wrappers hand the layer the
  name as it is, CP866 being what the DOS file converts from and the
  same bytes going to DOS. This is the one place where the group waits
  for the text's step.
- `md.ErrDat` is read and written by the program alone.

### The questions on it

- **F1. The handle.** A (recommended): an `int32_t`, kept in the
  program's 16-bit fields with a cast until they are widened. B: a
  structure of one `int32_t`, which the compiler holds apart from a
  count; some twenty fields and variables change.
- **F2. The open mode.** A (recommended): the access and two words for
  sharing, which DOS maps to the bits that the original hands it and
  POSIX does not look at. B: the access alone, and the sharing gone: the
  original's second try without it, in `OpenFil`, goes, a deviation.
- **F3. Creating with attributes, until group 3.** A (recommended): the
  layer takes `read_only`, and a second member, "as the file had", that
  only the DOS file reads, until group 3 says what a file's attributes
  are on every platform. B: group 3 first, and files second.
- **F4. A write of no bytes.** A (recommended): it cuts the file on
  every platform, as DOS has it. B: `SysFileCut` of its own, and each
  caller read for whether its count can be 0.
- **F5. The extended error.** A (recommended): `SysLastError`, asked
  behind a failure, as DOS's 59h. B: each function answers it itself,
  and DOS is asked 59h behind today's Bare calls too, which the
  original does not.

If S1 is answered B, these become `fopen` and its kin and the toy's
list of section 3 is what DOS is asked. If C3 is answered B, the
callers compare `enum SysError` in some 45 places. If C4 is answered
B, the layer keeps the asked and the bare kinds and is handed
`&md.ErrDat`. If C5 is answered B, `SysPlace` has a member that DOS
alone fills.

## What this draft is unsure of

- That the library's start-up, linked whole, asks DOS the same on a DOS
  other than the bench's: the toy ran once, on FreeDOS.
- That the routines of the library linked without its start-up need
  nothing of it: five were linked, and none was run.
- That every DOS number that the program tells apart is in the list of
  `enum SysError`: read from the comparisons that `grep` finds, of
  `value` with a number or an `ERR_` name, not from every use.
- The table of `errno` to the layer's errors: written from what each
  means, not tried.
- That the mouse driver answers in multiples of eight in a mode of
  text, so that cells lose nothing (C9): the program puts the pointer
  back at `md.MouseX` and `md.MouseY` as it found them.
- The counts of section 4, which are `grep`'s, and the size of UTF-8
  inside, which is a judgment from the files it touches.
- That a terminal's second screen does for Ctrl-O as users of the
  original expect (C11).
- Whether FreeDOS answers 59h alike for a call that the extender
  translated and for one made in real mode.
- `SystemTrueVersion` fills `md.VerDOS`, which nothing in the portable
  C reads: whether to drop the call is to be asked in group 9.
