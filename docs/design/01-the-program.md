# The program: its data, its loop, its screens

Vibe Commander is a two-panel file manager for an 80-column text
screen. Under its C it is still Volkov Commander 4.05, a DOS program of
1991 to 2000 written in 16-bit assembly for a machine with 640 KB, a
text buffer at `B800:0000` and BIOS calls for keys. It was translated
routine by routine into C, and the portable C, `src/portable_c/`, grew
out of that translation. So the program is laid out the way the
assembly laid it out:

- one block of global data, `struct MainDat md`, with the original's
  byte layout;
- one block per file panel, `struct WCBdef`, which the original reached
  through a segment register;
- tables of boxes and dialogs that name each other by address;
- a main loop that never returns, entered at several labels, and
  commands that are jumps rather than calls.

This chapter covers that skeleton: the files, the start, the data, the
loop, the input, the screen and what is drawn on it. The layer
underneath, `sys.h` and its implementations for DOS, Linux and the web,
has its own chapter. Here it is only the place where a key comes from
and the cells go to.

Everything below comes from `src/portable_c/`. Line counts, sizes and
offsets were read from the source on 16 October 2026. The structure
sizes are held by `LAID_OUT` lines in `vc.h`, which fail to compile if
they are wrong.

## The source tree

`src/portable_c/` holds 78 C files and headers, about 43,600 lines in
all. About 10,600 of those are the system layer: `sys_dos.c`,
`sys_posix.c`, `sys_web.c` and the `*_posix.c` files. The other 33,000
are the program, and it does not know which system it runs on.

There are three kinds of file:

- **Code**, `name.c`: routines translated from one or two of the
  assembly's include files. Every file's head comment says which,
  "Translated from VCSUB2.INC".
- **Tables**, `name_t.c`: the texts, boxes, dialogs and buttons that
  `name.c` uses, as constants and nothing else. The split comes from the
  translation, which moved the assembly's data to C one file at a time
  and checked every byte against the original's binary (the test
  `portable_c-data`, `harness/datacheck.py`).
- **The platform**, `sys_*.c`, `*_posix.c`, `stack_dos.c`: what is asked
  of the system, behind `sys.h`.

By role:

| Role | Files |
|---|---|
| What everything shares | `vc.h` (structures, prototypes, the commands), `tables.h` (how boxes and dialogs are written), `compiler.h` |
| Start, loop, leaving | `main.c`, `main_t.c`, `start.c`, `exec.c`, `setup.c` |
| The screen buffer and its primitives | `screen.c`, `window.c`, `box.c`, `line.c` |
| Text and numbers | `string.c`, `format.c`, `digits.c`, `mask.c`, `misc.c` |
| Keys, mouse, idling | `input.c`, `mouse.c`, `blank.c`, `system.c`, `system_t.c` |
| Bars, menus, dialogs | `bars.c`, `bars_t.c`, `menu.c`, `dialog.c`, `commands.c`, `commands_t.c` |
| The panels | `panel.c`, `place.c`, `files.c`, `read.c`, `info.c`, `tree.c`, `treebox.c`, `treebox_t.c`, `drive.c` |
| Commands of the panels | `keys.c`, `keys_t.c`, `enter.c`, `cmdline.c`, `usermenu.c`, `extfile.c` |
| Commands on files | `copy.c`, `copy_t.c`, `dirfunc.c`, `dirfunc_t.c`, `find.c`, `find_t.c` |
| Viewer and editor | `view.c`, `view_t.c`, `edit.c`, `edit_t.c` |
| Help | `help.c`, `help_t.c`, `help/vc-help.txt` |
| The setup, folded in | `stmain.c`, `stmain_t.c`, `stopts.c`, `stopts_t.c`, `stcolor.c`, `stcolor_t.c`, `sthelp.c`, `sthelp_t.c` |
| The system | `sys.h`; `sys_dos.c`, `stack_dos.c` for DOS; `sys_posix.c`, `machine_posix.c`, `emulate_posix.h`, `files_posix.c`, `names_posix.c`, `paths_posix.c`, `codepage_posix.c`, `command_posix.c`, `keyboard_posix.c`, `term_posix.c`, `time_posix.c` for Linux; `sys_web.c` and `web/` for the browser |

The dependencies are simple, because almost everything goes through
two shared things: the main data and the screen buffer.

```mermaid
graph TD
    main["main.c: Main, Round, Loop, Command"]
    start["start.c: Init00, Init11, IniData"]
    exec["exec.c: Alloc3, Exec, Exit00"]
    cmds["keys.c, enter.c, cmdline.c, commands.c, copy.c, dirfunc.c, find.c, view.c, edit.c, usermenu.c"]
    panels["panel.c, place.c, files.c, read.c, info.c, tree.c"]
    ui["dialog.c, menu.c, bars.c, help.c"]
    input["input.c, mouse.c, blank.c"]
    draw["screen.c, window.c, box.c, line.c"]
    tables["*_t.c: texts, boxes, dialogs"]
    md[("md: struct MainDat, ScrBuf inside it")]
    sys["sys.h"]
    impl["sys_dos.c, sys_posix.c, sys_web.c"]

    main --> start
    main --> exec
    main --> cmds
    main --> input
    cmds --> panels
    cmds --> ui
    ui --> input
    ui --> draw
    panels --> draw
    ui --> tables
    cmds --> tables
    draw --> md
    panels --> md
    cmds --> md
    input --> sys
    draw --> sys
    start --> sys
    exec --> sys
    panels --> sys
    sys --> impl
```

Names come from the assembly. A routine keeps its label: `F11_00` is
what Ctrl-O does, `Init50` draws the panels, `Func12` maps a key to its
command, `DialBox` draws a box. Data keeps its name too: `DosBuf`,
`TmpBuf`, `WinPath`. A comment marks the labels of the original that a
function passes through, `/* Func41 */`, so that a reader can hold the C
against `src/asm/` line by line. A thing that has stopped being what its
old name says gets a new name, with the old one in a comment beside it:
`CurrentPanel` was `WCBcrn`, `Room` was `ViewSeg`.

## How the program starts

Each system enters the program in its own place, and each ends up in
`Main` of `main.c`:

- On DOS the extender DOS/32A loads the linear executable and jumps to
  `Start` of `sys_dos.c`. `Start` notes the selectors of code and data,
  finds the PSP and the environment, takes a block below the megabyte
  for the calls that have to go through real mode, and calls `Main`.
- On Linux it is the C `main` of `sys_posix.c`, which takes the terminal
  (or a test channel), sets up the emulated machine and calls `Main`.
- On the web it is `sys_web.c`, under Emscripten.

`Main` never returns:

```c
void Main(void)
{
    struct Answer memory;

    Alloc3();
    Init00();
    memory = Init11();
    if (memory.failed)                  /* AlocEr */
        AlocEr(memory.value);
    Round();
}
```

```mermaid
sequenceDiagram
    participant S as Start or main()
    participant M as Main (main.c)
    participant X as exec.c
    participant I as start.c
    participant R as Round (main.c)
    participant L as MainLoop
    S->>M: call
    M->>X: Alloc3: DOS version, memory not used
    M->>I: Init00: read VC.INI or take defaults, banner, VC.EXT
    M->>I: Init11: two panel blocks and the room
    M->>R: Round, never returns
    loop for ever
        R->>I: IniData: screen, country, environment, interrupts
        R->>L: MainLoop until END_EXIT, END_EXEC or END_INIT11
        alt END_EXEC
            R->>I: GiveMem
            R->>X: Exec: run the commands in DosBuf
            R->>I: TakeMem
        else END_EXIT
            R->>X: Exit, Exit00
        end
    end
```

The three `Init` steps are the original's start, cut into functions.
What each one does:

- **`Init00`** looks for `VC.INI` in the program's directory, or in the
  one that the `VC=` variable names. It reads at most `INI_LEN + 1 =
  512` bytes into `MenuBuf`. The file counts only if it is exactly
  `INI_LEN = 511` bytes long, starts with `VVV` and ends with the right
  16-bit sum of the bytes before it. Then the first 330 bytes of the
  main data, `CmnIni`, are copied from it, and the two panels' 88 bytes
  each, `WCBini`, stay in `MenuBuf` until `Init11`. Otherwise
  `Init00` takes the defaults: the colour table `Palets`, both panels on
  the current directory, and the left panel switched off. Then it prints
  the banner through DOS, "Vibe Commander 0.1, based on Volkov Commander
  4.05" and the copyright line. It reads `VC.EXT`, the main extension
  file, into `ExtMain` and ends it with 1Ah, the way DOS text files end.
  It also sets the masks and notes Ctrl-C (`SetBrk`).
- **`Init11`** asks the system for two blocks of `PANEL_LEAST` bytes
  each, 32 + 65,536, one per panel. It copies the 88 bytes of each
  panel out of `MenuBuf` and sets the panel's places to an empty list.
  Then it takes the room, `TakeRoom`: all the rest of the memory, for
  the viewer, the editor and the copy buffer.
- **`IniData`** runs once now and again after every command, because a
  command may have left anything behind: another video mode, another
  country, another code page. It takes the critical-error and
  Ctrl-Break interrupts (`HookVct`). If the screen is not 80-column text
  it asks for Enter and sets the mode back. It reads the country and
  builds `LowTab`, a table of lower case for bytes 80h to FFh, by asking
  DOS for the capital of each one. It finds `COMSPEC=`, `VC=` and
  `TEMP=` in the environment. It counts the screen's rows, 25 or what an
  EGA or VGA reports, at most `MaxLins = 50`. Finally it finds DOS's
  text colour: it has DOS write a space and reads back the colour of
  that cell.

In the original, `Init11` is where the program comes back after every
command. The original gives its main data back to DOS while a command
runs. It keeps a copy of the setup and the panels in a small part that
stays in memory, and builds its data again from that copy. The
portable C stays in memory whole, a declared deviation. So only the
room is given back (`GiveMem`, `TakeMem`), and the loop goes back to
`IniData`. `exec.c` used to be `resident.c`, the part that stayed. It
is named for `Exec` now and is an ordinary file.

## The main data

### One structure, at offset 0

The original asks DOS for one block of memory, points DS at it and
keeps all of its state there. The C keeps that block as one structure
in `vc.h`, `struct MainDat`. It is `0xF48B` = 62,603 bytes. The one
instance is `md`, defined in `main.c` in a segment of its own:

```c
SEGMENT_BEGIN("MAINDATA", "MAIN")
struct MainDat md;
SEGMENT_END
```

The structure is `#pragma pack(1)` and field for field the `STRUC` of
`src/asm/VC.ASM`, comments included ("Data tramsnir area" and all). Its
main regions, with offsets that gcc worked out from `vc.h`:

```
 offset  size   field            what it is
 ------  -----  ---------------  ------------------------------------------
  0000     240  ColTab           colours: 40 entries x 6 (3 palettes x
                                 colour/mono)
  00F0      .   FulScr..DriveB   one byte per option of the setup
  0112       5  StarTab          the blanker's five shapes of a star
  0117      51  ResIni           reserved
  ------------------------------- CmnIni = 014Ah: end of what VC.INI saves
  014A       1  LoadRes
  015C     512  HstBuf           command history, newest first
  035C     512  ExtMain          VC.EXT, ended by 1Ah
  055C      14  EPB              DOS exec parameter block
  0578      68  CrnPath          current directory of the tree panel
  ...           ViewFil, ToFile, FindStr, SrchStr (65 each)
  06C5    1000  DosBuf           command line and dialog input lines
  ------------------------------- end of what the original keeps in memory
  0AAD      .   DosLin, DosCol.. command line on screen; Lines at 0AB3
  0AC9       8  WCBcrn..WCB2seg  the original's panel segments, unused
  0AED       2  FncAddr          per-file function of a group command
  0AF5       4  AutoTim          when FEh, "the time has come", is sent
  0B11      68  DosPth           current path
  0BDD      32  Country          DOS country info
  0BFD      44  DTA              DOS find-first record
  0C29     128  LowTab           lower case of 80h..FFh
  0CA9     256  TmpBuf           scratch, used by everything
  0DA9     256  TmpErr           texts put into boxes (^V at +1, ^W at +101)
  0EA9     128  Stars            32 stars of the blanker
  0F29    4000  MenuBuf          user menu; also VC.INI image, setup data
  1EC9    1406  InfBuf           Info panel text
  2447       1  Even             pads ScrBuf to an even address
  2448    8000  ScrBuf           the screen buffer, 80 x 50 cells
  4388    7840  UserScr          what DOS showed, 80 x 49 cells
  6228    7392  WinBuf           what a box covers, 77 x 48 cells
  7F08    7392  ErrBuf           second level of boxes
  9BE8    7392  ErrBuf1          third level (help, errors over boxes)
  B8C8    3000  HelpBuf          one help entry and its state
  C480   12299  TreeBuf          the directory tree, 768 entries
  F48B          end
```

### Why the layout stays

A modern program would split this into modules. This one keeps the
byte layout on purpose, and the code depends on it in four ways:

1. **The setup file is a dump of it.** `VC.INI` is three `V`s, the
   first 88 bytes of each panel, the first 330 bytes of `md`, and a
   16-bit sum. That is 511 bytes, the same format as the original's.
   `MakeIni` of `setup.c` builds the image, and `Init00` reads it back
   with a byte copy, `((u8 *)&md)[i] = image[3 + 2 * WCBini + i]`. A
   reordered field would make every existing `VC.INI` wrong.
2. **The tables name places in it by number.** A box says where to save
   what it covers as an offset into `md`, `MD_AT(WinBuf)`. A dialog
   field says where its value lives, `MD_AT(TmpBuf)`. Those numbers are
   16 bits, as in the original, and `MD_AT(field)` is just
   `offsetof(struct MainDat, field)`.
3. **The code reads across fields** the way the assembly did. `DialBox`
   copies 81 bytes of a text whatever its length, and the help reads a
   26-byte record into several fields at once. Such reads go through
   `MD_BYTES`, the structure as bytes, so that C is never asked what
   lies beyond a field.
4. **The tests compare it.** `harness/dataimage.py` dumps the main data
   at the end of every scenario and compares the dumps between builds,
   field by field, using the original's listing for the names.

Many fields say in their comment that they are "of the original. Not
set or read here". `WCBcrn`, `ViewSeg`, `TreAdr`, `ClstSiz` and the
others were 16-bit segments or offsets. In a flat 32-bit program they
are pointers or 32-bit counts, which do not fit in a word. So each
lives beside `md` under a name that says what it is:

```c
extern struct WCBdef *CurrentPanel;     /* WCBcrn */
extern u8 *Room;                        /* ViewSeg */
extern struct DIRENT *TreeCursor;       /* TreAdr */
extern u32 ClusterBytes;                /* ClstSiz */
extern i32 ClickLine;                   /* MousPos */
```

The old word keeps its place in `md` so that nothing after it moves.

Two macros show how literally the original is kept.
`MAIN_AS_PANEL`, `(struct WCBdef *)&md`, hands the main data to a
routine that expects a panel. In three places the original calls such
a routine with DS still on the main data, and reads colour bytes of
`ColTab` as if they were `Visible` or `WinPath[3]`. That is a bug, and
it is kept, because what the screen shows depends on it.
`PANEL_AS_MAIN` is the same thing the other way round, for one place in
`usermenu.c`.

## The panels

### `struct WCBdef`

A panel, a "window control block", is `0x1A62` bytes followed by its
list of files:

```c
struct WCBdef {
    u8  WinTyp;          /* 0 files, 1 tree, 2 info */
    u8  BrfFul;          /* brief or full */
    u8  Visible;
    u8  Hidden;
    u8  MasTyp;
    u8  SortTyp;         /* name, extension, time, size */
    u8  Ctrl_L;
    u8  DskMas[13];      /* the filter */
    u8  WinPath[LenPath];
    /* End of WCB part saved in VC.INI: 88 bytes */
    u16 CurAdr16, First16, EndBuf16;   /* the original's, unused */
    u8  ExtBuf[LenExt];  /* the directory's own extension file */
    u16 EndFul16;
    u8  ChgFlg;          /* to be read again */
    u8  SizDir;
    struct FILENT DskBuf[MinFls];      /* 256 entries, and on */
};
```

A file entry, `struct FILENT`, is 24 bytes: the name as 13 bytes
(8.3 and a zero), the attributes, DOS's packed time and date, a 32-bit
size, and `NumbF`. `NumbF` is the entry's place in the directory, which
nothing reads any more since the unsorted order went. Two high bits of
the attributes belong to the program: `ATTR_SELECTED` (40h) and "was
selected" (80h).

### One block per panel, places as offsets

In the original a panel is a segment, at most 64K. The list starts at
`DskBuf`, offset `0x262`, and the cursor, the first line shown and the
ends of the list are 16-bit offsets in that segment. The C keeps the
idea and gives up the limit:

```
  block (from the system, MemTake)
  +--------------------------+ <- block
  | struct Places  (32 bytes)|   Bytes, Unused[3],
  |                          |   CurAdr, First, EndBuf, EndFul  (i32 each)
  +--------------------------+ <- panel = (struct WCBdef *)(block + 32)
  | WCBdef header            |   0x000 .. 0x057  saved in VC.INI
  |                          |   0x058 .. 0x261  the rest
  +--------------------------+ <- panel + DSKBUF (0x262)
  | FILENT  ..               |   shown: DSKBUF .. EndBuf
  | FILENT  ..               |   hidden by the filter: EndBuf .. EndFul
  |   ...                    |
  +--------------------------+ <- block + Bytes
```

A place in the list is an `i32` offset from the panel's start, and
`ENTRY(wcb, place)` turns it into a pointer. `PLACES(wcb)` is the
`struct Places` just in front of the panel. The places stay offsets
because the block moves. `ReDir` of `read.c` reads a directory, and
calls `PanelGrow` of `start.c` whenever the list fills the block. That
may reallocate the panel elsewhere. At the end `PanelFit` trims the
block to the list, but never below `PANEL_LEAST`, which holds 2,703
entries, the original's ceiling. There is no ceiling now but memory,
and 65,536 entries, because `NumbF` is 16 bits. Both functions update
the four panel pointers. So the rule is that nothing keeps a pointer
into a panel across `ReDir`.

### Four pointers to two panels

```c
extern struct WCBdef *CurrentPanel;   /* WCBcrn */
extern struct WCBdef *OtherPanel;     /* WCBsec */
extern struct WCBdef *LeftPanel;      /* WCB1seg */
extern struct WCBdef *RightPanel;     /* WCB2seg */
```

`md.Active` says which side is current, and `IniData` sets the first
two from it. Tab (`F20_00`) swaps those two and inverts `md.Active`.
Ctrl-U (`F15_00`) swaps the other two, `LeftPanel` and `RightPanel`, so
that the panels change sides, and inverts `md.Active` too. The original kept the current
panel in DS during a command. Here it is the first argument of every
command, `struct Onward F11_00(struct WCBdef *panel)`.

### The tree

A panel of type 1 shows the directory tree. The tree is not in the
panel. It is in `md.TreeBuf`: the header of the original's
`TREEINFO.NCD`, then 16-byte `struct DIRENT` entries, root first, each
with its level. `TRELIST` points at the first entry, which starts
inside the header at `TreSum2`, as in the assembly. Some routines walk
a file list or the tree with the same code, `F00_50` of `keys.c` and
the speed search for example. They know a directory by
`TREE_OFFSET(entry)`, its 16-bit offset in the tree's list, just as
they know a file by its offset in the panel.

<!-- SCREENSHOT: the two panels as the program starts on the bench, files on both sides, key bar at the bottom; no keys -->

## The main loop

### What the original does

In the assembly the main loop is not a routine. The start-up code falls
into it at `Init40`. It falls through `Init50` (draw the panels),
`Func01` (draw one panel), `Func02` (copy the screen buffer out),
`Func04` (the menu bar), `Func05` (save the setup if it changed) and
`Func07` (wait for a key). At `Func12` it looks the key up in a table,
`FncTab`, through a routine called `Case`, and **jumps** to the
command:

```asm
Func12:	CMP	AH,0FCh
	JE	Func20
	CMP	ES:Quote,0
	JNZ	Func14
	MOV	SI,OFFSET FncTab
	CALL	Case
Func14:	JMP	F73_00
```

A command is code between labels. It ends by jumping back to one of
those labels, whichever fits, or on into another command. Ctrl-O ends
in `JMP Func03` when it switches the panels off. Registers go round:
whatever a command leaves in BX and DX, the next command finds there.
Rename reads them.

### What the C does

C has no computed jump into the middle of a loop, so the loop is two
functions and a driver:

```c
static i32 MainLoop(void)
{
    struct Regs regs;
    struct Regs *r = &regs;
    i32 entry = INIT40;
    ...
    for (;;) {
        command = Loop(entry, r);
        entry = Command(command, r->ds, r);
        if (entry >= END_INIT11)        /* no place of the loop */
            return entry;
    }
}
```

- `Loop(entry, r)` is a `switch` on the entry label, whose cases fall
  through into each other in the original's order. It returns the
  number of a command, a `CMD_` of `vc.h`.
- `Command(command, panel, r)` is a `switch` of about a hundred cases. It
  calls the command's function and turns what that returns into the
  next entry of `Loop`, or into an `END_`: start over, run DOS
  commands, or exit.

The entries are the original's labels:

```c
enum {
    INIT40,   /* find the command line, keep what DOS shows, draw all */
    INIT50,   /* draw the panels, reading those that changed */
    FUNC01,   /* draw the panel in DS */
    FUNC02,   /* put the screen buffer on the screen */
    FUNC04,   /* draw the menu bar */
    FUNC05,   /* save the setup if it changed and autosave is on */
    FUNC07,   /* wait for a key */
    FUNC12,   /* go to the command of the key in AX */
    END_INIT11, END_EXEC, END_EXIT
};
```

```mermaid
flowchart TD
    INIT40["INIT40: find command line, SavScr, IniMous"] --> INIT50
    INIT50["INIT50: Init50, read and draw panels"] --> FUNC02
    FUNC01["FUNC01: Panel, PutPath, PtFs00"] --> FUNC02
    FUNC02["FUNC02: Window, whole screen"] --> FUNC04
    FUNC04["FUNC04: PutMnu, clear flags"] --> FUNC05
    FUNC05["FUNC05: MakeIni, compare with IniSave, SaveSet"] --> FUNC07
    FUNC07["FUNC07: Cursor1, AutoTim, mouse or Input"] --> FUNC12
    FUNC07 -- "button down" --> MOUSE
    FUNC07 -- "Alt+letter" --> SPEED["CMD_F77_00 speed search"]
    FUNC12["FUNC12: Func12 key to CMD_"] --> CMD
    MOUSE["Func20 / Func21: mouse at the panels"] -- "a command" --> CMD
    MOUSE -- "nothing" --> FUNC07
    SPEED --> CMD
    CMD["Command: call the command"] --> ENDS["CmdEnd, MnuEnd, F1Next, F35_90"]
    ENDS -- "a place" --> INIT40
    ENDS -- "a place" --> INIT50
    ENDS -- "a place" --> FUNC01
    ENDS -- "a place" --> FUNC04
    ENDS -- "a place" --> FUNC05
    ENDS -- "a place" --> FUNC07
    ENDS -- "END_" --> ROUND["MainLoop returns to Round"]
```

`Loop` keeps three `goto`s, to `Func02`, `Func20` and `Mouse`. Those
are jumps from inside one case to a label inside another, which a
`switch` cannot do any other way.

Two things in `Loop` are easy to miss:

- **The setup is saved by comparison.** At `FUNC05`, every time the loop
  comes round, `MakeIni` writes the setup image into `MenuBuf`, and the
  loop compares it with `IniSave`, the image as last saved or loaded.
  The panels' paths, `Active` and `Ctrl_O` are copied across first, so
  that they never count as a change. If anything else differs and Auto
  save is on, `SaveSet` writes `VC.INI`. There is no "dirty" flag
  anywhere in the program. The comparison takes the place of one.
- **`AutoTim`** is an alarm. With Auto change dir on and the cursor
  moved in a tree panel, `FUNC07` sets `md.AutoTim` to now plus
  `DelayTm`, as hours:minutes in the low word and seconds:hundredths in
  the high word. When that moment passes, `Input0` returns the
  pseudo-key FE00h, and `Func12` maps it to `CMD_F75_00`, which changes
  the directory. A low word of FFFFh means never.

### The registers that go round: `struct Regs`

The registers that pass from one command to the next are a structure:

```c
struct Regs {
    u16 ax, bx, cx, dx;
    i32 si, di;                         /* the numbers of two entries */
    u16 bp;
    struct WCBdef *ds;                  /* the current panel */
    i32 last;                           /* and of the last entry */
};
```

Most commands take only the panel. The `struct Regs` is passed where
something really is read:

- the mouse hands its commands the entry under the pointer in `si` and
  the first entry shown in `di`, its buttons in `bp`;
- a key that goes to the command line arrives in `ax`;
- Rename reads `bx` and `dx` as the previous command left them. The
  cursor keys go back to `FUNC07`, behind the point where the original
  reloads BX, so they must leave the original's values there. `F00_50`
  returns them in a `struct Left`.

The comments in `Loop` record, again and again, which register the
assembly leaves somewhere that nothing reads: "The assembly has SI and
DI behind what it has compared and copied ... nothing reads SI or DI as
this leaves them." Every register that does go round was found by
reading every command.

### How a command says where to go on: `struct Onward` and the four families

A command function returns where the loop goes on. The commands come
in four families, by the file of the assembly they live in. Each family
has its own set of places, and `main.c` has one function per family
that turns a place into a loop entry:

| Family | Files | Returns | Places | Turned into an entry by |
|---|---|---|---|---|
| Keys of the panels | `keys.c`, `enter.c`, `view.c` | `struct Onward` | `NEXT_FUNC07` .. `NEXT_EXIT`, 20 of them | `CmdEnd` |
| Menu commands | `commands.c` | `i32` | `MNU_FUNC04`, `MNU_INIT50`, `MNU_F24_01`, `MNU_V00_01` | `MnuEnd` |
| Group file commands | `dirfunc.c`, `find.c` | `i32` / `struct Searched` | `TO_FUNC04` .. `TO_F24_02` | `F1Next` |
| Copy and rename | `copy.c` | `struct Copied` | `GO_KEYS` .. `GO_DONE` | `F35_90` |

```c
struct Onward {
    u8  place;                 /* a NEXT_ */
    u8  item;                  /* for M03_00 and M06_00 */
    struct WCBdef *panel;      /* the panel a menu routine is for */
};
```

Some places are not loop labels but other commands, because in the
original a command could jump into another. `NEXT_M01_01` goes on into
the menu routine that sets a panel's view, `NEXT_EARLIER` into `F25_00`
(Ctrl-E), `NEXT_VIEW` into the viewer. `CmdEnd` calls those itself and
then goes on from what they return. Ctrl-O in both versions makes the
pattern plain. The assembly, `VCFUNC.INC`:

```asm
F11_00:	CMP	DS:Visible,0		; ^O
	PUSH	DS
	MOV	DS,ES:WCBsec
	JZ	F11_01
	MOV	AL,DS:Visible		; Panels Off
	MOV	ES:Ctrl_O,AL
	MOV	DS:Visible,0
	POP	DS
	MOV	DS:Visible,0
	CALL	BegBX
	CALL	SizDX
	...
	CALL	ResUser
	JMP	Func03
```

The C, `keys.c`:

```c
struct Onward F11_00(struct WCBdef *panel)
{
    struct WCBdef *second = OtherPanel;
    ...
    if (panel->Visible != 0) {
        md.Ctrl_O = second->Visible;
        second->Visible = 0;
        panel->Visible = 0;
        begin = BegBX(panel) & 0xFF00;
        size = (SizDX() & 0xFF00) | 80;
        ResUser(begin, size);
        Window(begin, size);            /* Func03 */
        return To(NEXT_FUNC04);
    }
    ...
    return To(NEXT_INIT50);
}
```

`JMP Func03` became an inlined `Window` and a return to `FUNC04`, the
label behind `Func03`. DS (the panel) and ES (the main data) became
`panel` and `md`.

### From a key to a command

`Func12` is the original's `FncTab` written as two `switch`es. The
first matches whole BIOS words, scan code and character: 4800h is Up,
3F00h is F5, 6E00h is Alt-F7. The second matches characters that count
with any scan code, the Ctrl letters: 0Fh is Ctrl-O, 15h is Ctrl-U.
Anything else goes to the command line, `CMD_F73_00`. The order matters
for keys like Tab (0F09h), whose character 09h is also Ctrl-I. The
table had the full key first, so the first `switch` does too. Alt-F5
and Alt-F9 are missing: the memory map and EGA lines are gone from the
portable C.

```mermaid
sequenceDiagram
    participant U as user
    participant S as sys.h (KeyRead)
    participant I as Input0 (input.c)
    participant L as Loop (main.c)
    participant F as Func12
    participant C as Command
    participant K as F11_00 (keys.c)
    participant E as CmdEnd
    U->>S: Ctrl-O
    S-->>I: BIOS word 180Fh
    I-->>L: struct Key, not clicked
    L->>F: key 180Fh
    F-->>L: CMD_F11_00
    L-->>C: CMD_F11_00, r
    C->>K: F11_00(panel)
    K-->>C: To(NEXT_INIT50)
    C->>E: CmdEnd
    E-->>C: INIT50
    C-->>L: Loop(INIT50, r): panels read and redrawn
```

A menu item takes another road to the same `Command`. F9 is
`CMD_F39_00`, and `F39_01` runs the bar, `F39_00` of `menu.c`. The bar
returns which menu, which item and which table. There are six tables
in `commands_t.c`: `Menu1_T` for the Left and Right menus, then Files,
Commands, Options, and Shift-Files and Ctrl-Files. The command number
is read there. `F39_01` also loads AX, BX, CX and DX the way the
original's jump left them, and calls `Command` again, recursively. The
setup is `CMD_SETUP`, the last number, and `Command` handles it as
`StSetup(); return END_INIT11;`.

## Input

### A key is a BIOS word

Everything that waits for a key calls `Input(bar)`, or `Input0(bar)`
where F1 is not to bring up help. A key is a 16-bit BIOS word, scan
code in the high byte and character in the low byte. That is the form
on every platform: `keyboard_posix.c` turns terminal escape sequences
and the kitty keyboard protocol into the same words. `Input0` adds
three scan codes of its own:

| High byte | Meaning |
|---|---|
| FCh | a mouse button is down; the low byte says which |
| FDh | Alt was tapped by itself (opens the menu bar if `AltBar` is set) |
| FEh | the moment in `AutoTim` has come |

`Input` itself is short. F1 shows help and keeps waiting:

```c
u16 Input(i32 bar)
{
    u16 key;

    for (;;) {
        key = Input0(bar).key;
        if (key != KEY_F1)
            return key;
        if (StOpen != 0)
            StHelp();
        else
            Help00();
    }
}
```

So any routine that takes keys can have the help on the stack above
it. While the help is up, `md.HlpPage` is FFh, and a second F1 does
nothing.

### What `Input0` does while it waits

`Input0` is a busy loop of the DOS kind. Each time round it:

1. redraws the key bar for the Shift, Ctrl and Alt keys now held
   (`PutKey(bar)`). `PutKey0` draws only if the row it picks differs
   from `md.BarPrev`;
2. calls `LeaveTm()`, which calls `Idle()` of `sys.h`. On DOS that is
   INT 28h, "DOS is idle". On Linux and the web it also draws the
   screen and waits up to `IDLE_WAIT`, 50 ms, for input, so the loop
   does not spin the CPU;
3. checks whether the blank delay has passed, and if so calls `Blank()`;
4. draws the clock if `md.Clock` is set;
5. checks `AutoTim`;
6. steps the random numbers, `Rnd()`, so that the blanker's sky differs
   every time;
7. checks for a key, then the mouse buttons, then a tap on Alt. A tap is
   Alt down and up within 3 timer ticks, with nothing else changing in
   between;
8. checks the mouse in the corners. In the top right corner it blanks
   the screen at once. In the bottom right corner it holds the blanker
   off.

```mermaid
flowchart TD
    A["Input0(bar)"] --> B{"key waiting?"}
    B -- "yes" --> Z["Inpt41: key bar click?"]
    B -- "no" --> C["PutKey(bar), LeaveTm, Idle"]
    C --> D{"blank time passed?"}
    D -- "yes" --> BL["Inpt12: Blank, redraw"] --> C
    D -- "no" --> E["clock: draw if separator changed"]
    E --> F{"AutoTim due?"}
    F -- "yes" --> RT["key FE00h"] --> Z
    F -- "no" --> G["Rnd, then key waiting?"]
    G -- "yes" --> Z
    G -- "no" --> H{"mouse button?"}
    H -- "yes" --> RM["key FC00h or buttons"] --> Z
    H -- "no" --> J{"Alt tapped alone?"}
    J -- "yes" --> RA["key FD00h"] --> Z
    J -- "no" --> K["mouse in a corner?"] --> C
    Z --> R["return struct Key"]
```

### The mouse

`mouse.c` asks `sys.h` where the mouse is, in pixels, eight to a cell.
`GetMous` turns that into a cell, row in the high byte and column in
the low byte, and the buttons (left 1, right 2, middle 4, left and
right swapped for `LftMous`). `DoublMo` tells a second click on the
same line from a first one, using `ClickLine` and `MousTim`. Three
places act on the mouse:

- **`Inpt41`** of `input.c`, on the key bar. A button held on a key
  shows the bar with Shift (middle button) or Alt (right button). The
  pointer gets a check mark, FBh, while it is on a key. Letting go
  there returns that key, F1 to F10 plus the column divided by 8.
- **`Func21`** of `main.c`, at the panels. The top left corner toggles
  the panels. The top row opens the menu bar. A click in the other
  panel makes it current. Inside a list, `Func40` works out the entry
  under the pointer, scrolls when the pointer is above or below the
  list, and a double click is Enter, `CMD_F21_10`. The arithmetic keeps
  the original's 8-bit quirks. A row of 128 or more on a 48-line screen
  in brief mode is taken as a signed byte, and the cursor jumps 256
  entries back. The comment says so.
- **`Dialog`**, `MinMnu` and `F39_00`, each with its own loop for
  buttons, fields and menu items.

## The screen

### A buffer of cells

The program never draws on the screen directly. It draws into
`md.ScrBuf`, 80 x 50 cells of 16 bits each, character in the low byte
and attribute in the high byte, exactly the format of the PC's text
memory. Then `Window(begin, size)` copies a rectangle of it out.
Positions are packed the assembly's way, row in the high byte and
column in the low byte, so that `0x0528` is row 5, column 40. A size is
packed the same way, rows and columns.

```c
u16 *BasAdr(u16 place)
{
    i32 row = place >> 8;

    /* 80 cells to a row, without a multiplication: see IN_TABLE. */
    return md.ScrBuf + (row << 6) + (row << 4) + (place & 0xFF);
}
```

The shifts stand in for `* 80` because on a 286 a multiply cost more
than shifts. The comments also mention "the program's short stack",
which was 256 bytes in the original. The portable C has `LenStk =
0x4000`, but it keeps the arithmetic as written.

`Window` copies one row cell by cell, or several rows as whole 80-cell
rows, through `ScreenPut` of `sys.h`. On DOS `ScreenPut` is a loop that
stores into the video memory at the segment the BIOS mode implies
(`md.Screen`). On Linux and the web it copies into an emulated video
memory, and `Idle()` turns that into terminal output or HTML spans.
The mouse pointer is hidden around every copy (`CurMous(0)` and
`CurMous(1)`), as the DOS mouse driver needs.

```mermaid
graph LR
    R["routines: panel.c, line.c, box.c, dialog.c ..."] -- "write cells" --> SB["md.ScrBuf 80x50"]
    SB -- "Window(begin, size)" --> SP["ScreenPut (sys.h)"]
    SP --> DOS["DOS: video memory B800h / B000h"]
    SP --> LNX["Linux / web: emulated cells, drawn at Idle"]
    SB -- "SavWin" --> SAVE["WinBuf / ErrBuf / ErrBuf1 / MenuBuf+500 ..."]
    SAVE -- "ResWin" --> SB
    US["md.UserScr: what DOS showed"] -- "ResUser" --> SB
    DOS -- "SavScr at start and after commands" --> SB
```

### Keeping what is under a box

There is no window system. A box saves the cells it is about to cover
and puts them back when it closes:

- `SavWin(begin, size, buffer)` copies a rectangle of `ScrBuf` into a
  buffer of the main data;
- `ResWin(begin, size, buffer)` copies it back, and `Window` then
  redraws that part of the screen.

There are three buffers of 77 x 48 cells, so boxes nest three deep.
`WinBuf` is for an ordinary box or menu. `ErrBuf` holds the menu bar's
row while the bar is up, or a second box. `ErrBuf1` is for the help
and the error boxes that can appear over anything. Each box names its
buffer in its table by its place in `md`, `MD_AT(WinBuf)`. A buffer
can begin at an odd address (`MenuBuf+500` is one), so cells are
stored with `PUT16` and read with `GET16`, byte by byte. That keeps
the alignment check of the Linux sanitizer happy, and it does not
depend on byte order.

`UserScr` is different. It holds what DOS had on the screen when the
program started or a command ended, which `Init40` reads in with
`SavScr`. Switching a panel off shows that again (`ResUser`), and so
does Ctrl-O, which shows the output of the last command under the
panels.

### Colours

A colour is a number, not an attribute. `Color(n)` looks up entry `n`
of `md.ColTab`. Each entry has six bytes: black-and-white, colour and
laptop palettes, each for a colour and a monochrome display. The
numbers are named in `tables.h`: `Box_C` 16, `Clock_C` 34, `Sky1_C` 36,
`ShadFl` 38 and so on. The setup edits that table. The shadow of a box
is not a colour but a rule. `ShadCol` either leaves a cell alone, or
computes `(colour & 8Fh) ^ 8`, or makes it grey on black, depending on
the `ShadFl` entry.

### Lines with markup

Most text on the screen is written by `NgLine` or `CntLine` of `line.c`
(centred), in the markup of Norton Guides:

| Markup | Meaning |
|---|---|
| `^N ^R ^B ^U` | normal, reversed, bold, underlined: the caller's four colours |
| `^V`, `^W` | the texts at `TmpErr+1` and `TmpErr+101` |
| `^I` | the "include line" the caller passes |
| `^Axx`, `^Cxx` | colour xx, character xx (hex) |
| `FFh n` | n spaces; `FFh 80h+n c`: n copies of c |

`LnStr` counts the cells a line will take, so a box knows how wide to
be. The original's `NgLine` had `^A` and `^C` the wrong way round
compared with its own `LnStr`. The portable C puts that right, which
the harness declares.

## Boxes, dialogs and menus

### Tables that name tables

A box is a constant in a `_t.c` file. The quit dialog of F10 is in
`keys_t.c`:

```c
static const u8 Quit1_S[] = " The ^I ";
static const u8 Quit2_S[] = "Do you want to quit the ^I?";
const u8 YesNo_S[] = "Yes\377\003No";
const u8 YesNo_D[] = {
    5, 0,                               /* Yes */
    4, 6,                               /* No */
    0
};
static const BOX(3) Quit_P = {
    { Box_C, BoxCu_C, BoxCu_C, 0 },
    3, 0,
    MD_AT(WinBuf),
    { Quit1_S, Quit2_S, YesNo_S }
};
const DIALOG(1) Quit_B = {
    { &Quit_P, 0x528, 0, 1, 2, 26, 0 },
    { { 4, 0, 0, 0, 0, 0x323, MD_AT(TmpBuf), YesNo_D } },
    0
};
```

The layouts, from `tables.h`:

```
BOX(n)                                 DIALOG(n)
+--------------------------+           +----------------------------------+
| u8  colors[4]   colours  |           | struct BOXHD head                |
| u8  lines       n        |           |   const void *pDiBox  -> BOX     |
| u8  texts       0..2     |           |   u16 BoxBase   middle of top    |
| u16 saved   MD_AT(buf)   |           |   u16 SubFunc   SUB_ routine     |
+--------------------------+ BOXTOP    |   u8 bKeyBar, bMenu, bHelp, bFlags|
| struct BOXTX text[texts] |           +----------------------------------+
|   u16 from, u8 how, len  |           | struct BOXMN field[n]            |
+--------------------------+           |   u8  mType   1 input 2 select   |
| const u8 *line[n]        |           |               3 tick  4 buttons  |
|   line[0] = title        |           |   u8  mUp mDown mLeft mRight     |
|   0 = a rule across      |           |   u16 mBase   place in the box   |
+--------------------------+           |   u16 mPoint  MD_AT of its value |
                                       |   const void *mCtrl buttons/NUMBER|
                                       +----------------------------------+
                                       | u8 0                             |
                                       +----------------------------------+
```

In the assembly every reference here is a 16-bit offset in the one
segment. Here a reference to another table is a real pointer, four
bytes on DOS and eight on Linux. That makes `BOXHD` 12 bytes rather
than the original's 10, and `BOXMN` 13 rather than 11, and the data
test compares them field by field. A reference into the main data
stays a 16-bit offset, `MD_AT(...)`, because the main data keeps its
layout. Each shape, `BOX(3)` or `DIALOG(1)`, is its own tagged struct,
`struct Box3`, made by `BOX_SHAPE(3)`. That lets a table file declare
an object before it defines it, as clang's
`-Wmissing-variable-declarations` asks. A file of code sees only the
common head: `extern const struct BOXHD Quit_B;`.

A `struct BOXTX` copies a text into `TmpErr` before the box is drawn,
from an offset in whatever data the caller passes, the main data or a
panel. `how` says how to format it: 4 trims spaces, 2 upper-cases, 1
keeps a file name at the end as it is. `length` cuts the text from the
front. That is how a box shows a path that is too long for it.

### Drawing a box: `DialBox`

```mermaid
flowchart TD
    A["DialBox(place, head, include, data, where)"] --> B["DiBx30: copy BOXTX texts to TmpErr+1, +101"]
    B --> C["LnStr every line: width = longest"]
    C --> D["size = lines,width + 040Ch: margins, frame, shadow"]
    D --> E["SavWin: what it covers, to md at head.saved"]
    E --> F["MinBox: frame and shadow, zooming if ZoomWin"]
    F --> G["CntLine each line, or BoxLine for a rule"]
    G --> H["caller: Window(begin, size)"]
```

A box has three cells of margin on each side of the frame, an empty
row above it, and a shadow two cells wide and one row deep. On a
screen taller than 25 rows the box moves down by half the extra rows.
With `ZoomWin` set, `MinBox` grows the frame from the middle in steps,
timed by the timer, before drawing it full size.

### `Dialog`

`Dialog(head, include, begin, size, data)` is one function of about
330 lines. It draws the menu bar item, the key bar and the box, and it
fills in each field from `data` (the main data or a panel). Then it
loops: the field under the cursor gets the keys through its own
routine, `InpStr` for an input line, `Select` for a group, `FlipFl` for
a tick box and `MinMnu` for buttons, and Up, Down, Left and Right move
to the field that the `mUp` .. `mRight` links name. Enter or F10 can
call a routine that the dialog names by number in `SubFunc`, for
example `SUB_M30_60`, which checks a typed date. In the original that
word held the routine's address. Here a `switch` dispatches on the
number. The answer is the button chosen, or FFh for Esc.

A line being typed in lives in `DosBuf`, from offset 124 + 10, behind
the command line. It is three bytes and then the text: the cursor
column and a "typed yet" byte. The first key typed replaces the text.

<!-- SCREENSHOT: the F10 quit dialog over the panels, Yes highlighted; keys from the start: F10 -->

### The menu bar

`menu.c`'s `F39_00` runs the bar. While it is up, the top row is saved
in `ErrBuf` and the open menu's box in `WinBuf`. The five menus are
rows of `MnuTab`, each with its box position, its size, where its lines
start in `MenuLn` and where its help pages start in `MnuHlp`. The lines
are one string literal made of concatenated macros, so that `sizeof`
of each part gives the offset of the next at no cost:

```c
static const u8 MenuLn[sizeof(LEFT_S FILES_S COMMANDS_S OPTIONS_S) - 1]
    NONSTRING =
    LEFT_S FILES_S COMMANDS_S OPTIONS_S;
```

The first capital letter of a line chooses it. `MnuKey`, "LFCOR",
opens the menus from the bar. `KeyTab` lets an open menu take a
command's own hot key: F5 in the Files menu chooses Copy. The bar
returns a `struct Picked`, and `F39_01` in `main.c` turns that into a
command.

`F39_00` is one of the functions where the assembly's `goto`s remain,
50 of them. The bar is a state machine whose states are labels:
`F39_10` waits for input, `F39_41` moves to another item, `F39_85`
leaves. The original enters loops in the middle and crosses paths
between them. The project's rule is that such a function keeps its
`goto`s until it is rewritten for another reason. The labels keep the
assembly's names, so the C can still be held against `VCMENU.INC`.
Across the tree, nine functions still have `goto`s, about 440 in all:

| Function | File | `goto`s |
|---|---|---|
| `V00_10` | `view.c` | 98 |
| `C20_00` | `copy.c` | 67 |
| `F32_00` | `usermenu.c` | 65 |
| `F39_00` | `menu.c` | 50 |
| `Run_00` | `usermenu.c` | 35 |
| `FileLst` | `usermenu.c` | 17 |
| `Loop` | `main.c` | 4 |
| `V82_00` | `view.c` | 3 |
| `FncSel` | `dirfunc.c` | 2 |

Everywhere else the jumps became `if`, loops, `break`, `continue` and
`return`. Where a label went, its name stayed behind as a comment, as
in `/* Func41 */`.

<!-- SCREENSHOT: the Options menu open from the menu bar, " More setup..." at its bottom; keys from the start: F9 O -->

### The key bar

The bottom row is ten keys of eight cells: the number, six cells of
name and a space. `PutKey0(bar, shift)` picks a row of `KeyB_T` by the
bar's number and the shift state (none, Shift, Ctrl, Alt). That row
gives an index into `KeyC_T`, ten name numbers, which index `Key_S`,
the six-letter names. While the setup is open (`StOpen`), the setup's
own tables `StKeyB_T`, `StKeyC_T` and `StKey_S` are used instead. Every
routine that waits for keys passes a bar number to `Input`: 0 for the
panels, 1 for a box, and so on.

## The help

`help.c` reads `VC.HLP` from the program's directory. The help file
is the project's own text, `help/vc-help.txt`, which
`tools/makehelp.py` turns into the original's binary format at build
time. Nothing of the shareware help is in it. The format:

- a header with a version string;
- at 194h, the file offset of the first entry, which is the index;
- each entry is 26 bytes of description (mode, text length, line count,
  the index line it belongs to, the offset of the index) followed by
  its text. All of it is XORed with 1Ah, which `DeCode` undoes.

Only one entry is in memory at a time, in `md.HelpBuf`, laid over by
`struct HlpData` (3,000 bytes, held by `LAID_OUT`). Its lines use the
same markup as everything else. In the index, each line carries six
extra bytes, four of which are the file offset of its page. Every
dialog and menu says which page it wants in `md.HlpPage`: 26 for the
quit dialog, the `bHelp` of its header. `Help00` opens the file on F1,
shows the page in a window that begins at row 2, column 4 (`BEGIN`,
0204h), 74 columns wide, and puts back what it covered from `ErrBuf1`.

<!-- SCREENSHOT: the help window on the quit dialog's page; keys from the start: F10 F1 -->

## The setup, folded in

In the original, `VCSETUP.COM` is a separate program with copies of
the shared routines. It edits `VC.INI` on disk and then signals a
running `VC` through a private DOS call. The portable C makes it one
item of the Options menu, " More setup...", `CMD_SETUP`. The code is
the `st*.c` files. Every routine and object there carries the
assembly's name with `St` in front (`StMain00`, `StF05_00`,
`StIniEr_B`), because many names clash with the main program's. A
routine the two programs share word for word is written once, the main
program's version.

`StSetup` of `stmain.c` works like this:

1. It makes the screen look as it does when a command runs: `Quit`
   restores DOS's screen, and a line feed is typed.
2. It copies the live setup, the first `CmnIni` bytes of `md`, into
   `sd.Aside`. `sd` is `struct StData`, laid over `MenuBuf`. A `HOLDS`
   check proves it fits in the 4,000 bytes.
3. It reads `VC.INI` into the main data (`StFile`, `StInit15`). Now the
   main data is what the setup program had in its one segment. The
   shared routines, the boxes, dialogs, bars and colours, edit it in
   place, and the setup's colour pictures show the new colours as they
   change.
4. It sets `StOpen = true`. Three shared routines check this flag:
   `PutKey0` uses the setup's key bars, `Dialog` draws no menu bar and
   keeps typed lines in `TmpErr`, and `Input` sends F1 to `StHelp`.
5. It runs the setup's menu, `StMain00`, then `StExit`. That either
   writes the file and keeps the edits, or puts `sd.Aside` back.
6. It returns, and `Command` answers `END_INIT11`. The program carries
   on as after a DOS command: `IniData` looks at the screen again and
   the panels are read again.

The setup has its own help, `sthelp.c`: seven pages of `VC.HLP`, from
line 55 of the index, all held in memory, with the same `DeCode`.

<!-- SCREENSHOT: the setup's own menu, entered from Options; keys from the start: F9 O, then Up and Enter on " More setup..." -->

## The clock and the blanker

### The clock

With `md.Clock` set, `Input0` keeps the time in the top right corner:
six cells at `ScrBuf + 74`, in colour 34. It saves those cells first
(`ClkBuf`) and puts them back when a key arrives. The separator blinks.
It is the country's time separator while the hundredths of a second
are 50 or more, and a space otherwise. The clock is redrawn only when
the separator changes, so the screen is written twice a second, not
every time round the loop. With a 12-hour country format (`TimeFmt` 0)
the clock starts a cell further left and ends in `a` or `p`.

### The blanker

`Blank` of `blank.c` runs when the minutes in `md.BlDelay` pass without
input, or when the mouse moves into the top right corner. It draws
straight to the display through `sys.h` (`ScreenScroll`, `CursorTo`,
`ScreenFill`) and not through `ScrBuf`. So the buffer keeps the real
screen, and `Inpt12` gets it back with one `Window` of the whole
screen.

The sky follows these rules:

- Each timer tick, a star may come out at a random cell. The odds and
  the place come from `Rnd`, a recurrence of two 16-bit words,
  `Rand1` and `Rand2`, which `Input0` also steps while it waits.
  Nothing seeds them, so the first sky depends on what memory held.
- A star lives in one of 32 slots, `md.Stars[NmbStrs]`. Each slot has a
  16-bit `StarMod`: a countdown of 100 to 255 in the low byte and a kind
  bit in the high byte.
- As the countdown passes 12, 9, 6 and 3 (`FLARE_1` .. `FLARE_4`), the
  star takes the next shape from `md.StarTab`, five bytes of the setup.
  A star of kind 0 flares four times. A star of kind 1 goes out at 9.
- Any key, a change of the shift keys, a mouse button, or a mouse move
  outside a 4 x 2 corner ends the sky. The key is consumed.

`Blank` waits between ticks by calling `LeaveTm`, so even the blanker
idles politely.

## Where the assembly shows through

Some of the original's shape stays even where C would have done it
differently:

- **Names.** Routine labels (`F11_00`, `Init50`, `Func12`, `DialBox`)
  and data names (`DosBuf`, `TmpErr`, `WinPath`) are the original's. A
  renamed thing says what it was: "`CurrentPanel` ... `WCBcrn` of the
  original".
- **Registers as structures.** `struct Regs` goes round the loop.
  `struct LineArgs` is the AX/BX, DI, DS:SI, CS:BP and CS:DX that a line
  routine took. `struct Place` is SI, DI, DX and AX from `AdrNum`. A
  function returns a small structure where the original returned two
  registers and a flag: `struct Key` is the key and the carry, and
  `struct Answer` is AX and the carry.
- **Commands as jumps.** A command returns a place, and the place is a
  label of the loop or of another command. `Loop` falls through its
  cases where the original fell through labels.
- **Packed coordinates.** Row in the high byte, column in the low byte,
  everywhere. `ADD_LOW` adds to the low byte without a carry, as `ADD
  BL,..` did.
- **Byte arithmetic.** Wherever the original counted in 8 or 16 bits
  and the screen shows the result, the C counts the same way, with a
  `(u8)` or `(u16)` cast and a comment that says it "wraps".
- **Faithful bugs.** "Ported as it is" or "Ported as they are" opens a
  paragraph that describes a bug of the original kept on purpose: the
  key bar's 81st cell, the signed row of a mouse click, `MAIN_AS_PANEL`.
  A bug that was fixed is a numbered entry of `DEVIATIONS.md`, and the
  code points to it, "(DEVIATIONS.md, 28)".
- **`goto`** remains in nine functions, where the original's control
  flow is not structured.

## Where to read next

- `src/portable_c/vc.h`: `struct MainDat`, `struct WCBdef`,
  `struct Places`, and the section "The commands" with the `CMD_`,
  `NEXT_`, `MNU_`, `TO_` and `GO_` numbers.
- `src/portable_c/main.c`: from `Main` at the bottom upward through
  `Round`, `MainLoop`, `Command`, `Loop` and `Func12`.
- `src/portable_c/start.c`: `Init00`, `Init11` and `IniData`, and
  `PanelGrow` and `PanelFit` for the panel blocks.
- `src/portable_c/input.c`: `Input0` and `Inpt41`.
- `src/portable_c/window.c` and `screen.c`: `Window`, `SavWin`,
  `ResWin`, `BasAdr` and `Color`.
- `src/portable_c/tables.h` with `keys_t.c`, then `dialog.c`:
  `DialBox`, then `Dialog`.
- `src/portable_c/menu.c`: `F39_00`, and `bars.c`: `PutKey0`.
- `src/portable_c/stmain.c`: `StSetup`.
- `src/asm/VC.ASM`, from the label `Init40` on, to see the loop the C
  replaced.
