# Volkov Commander 4.05: the design

A short book on how Vsevolod Volkov fitted a two-panel file manager for
DOS, with a viewer, an editor, a file search, a tree of the disk, a
mouse, a help and a manager for resident programs, into a `.COM` file
of 65,090 bytes that keeps 12,690 bytes of itself in memory while the
user's command runs. It reads the original assembly, `src/asm/`, a
routine at a time.

The reader it is written for knows a modern toolchain and may remember
a little DOS: the [preface](00-preface.md#a-dos-refresher) has what the
chapters take for granted.

## Contents

### [Preface](00-preface.md)

What the program was, the limits it was written to, the question of the book, a DOS refresher, and how to read the rest.

- [What Volkov Commander was](00-preface.md#what-volkov-commander-was)
- [The constraints](00-preface.md#the-constraints)
- [The question](00-preface.md#the-question)
- [A DOS refresher](00-preface.md#a-dos-refresher)
- [How to read this book](00-preface.md#how-to-read-this-book)

### [1. The program: one segment, one loop, one screen buffer](01-the-program.md)

The skeleton: the source and the file, the start, the main data and the panels, the main loop and its key tables, input, the screen, boxes, dialogs and menus as tables, the help, the clock and the blanker.

- [The source: one file and twelve includes](01-the-program.md#the-source-one-file-and-twelve-includes)
- [The parts of `VC.COM`](01-the-program.md#the-parts-of-vccom)
- [The start: from one block to four](01-the-program.md#the-start-from-one-block-to-four)
  - [`Init00`: the setup file, read in place](01-the-program.md#init00-the-setup-file-read-in-place)
  - [`Init11`: the data block, cut in paragraphs](01-the-program.md#init11-the-data-block-cut-in-paragraphs)
- [A command runs in 12,690 bytes](01-the-program.md#a-command-runs-in-12690-bytes)
- [The main data](01-the-program.md#the-main-data)
  - [`MainDat`: one `STRUC`, 62,603 bytes](01-the-program.md#maindat-one-struc-62603-bytes)
  - [Saving the setup without a "dirty" flag](01-the-program.md#saving-the-setup-without-a-dirty-flag)
- [The panels](01-the-program.md#the-panels)
  - [`WCBdef`: a header and a list, in a segment of its own](01-the-program.md#wcbdef-a-header-and-a-list-in-a-segment-of-its-own)
  - [A panel is its segment](01-the-program.md#a-panel-is-its-segment)
- [The main loop](01-the-program.md#the-main-loop)
  - [Labels, not a procedure](01-the-program.md#labels-not-a-procedure)
  - [`Func07`: an alarm and an injected key](01-the-program.md#func07-an-alarm-and-an-injected-key)
  - [`Func12` and `Case`: a key table walked by 37 bytes](01-the-program.md#func12-and-case-a-key-table-walked-by-37-bytes)
  - [The mouse at the panels](01-the-program.md#the-mouse-at-the-panels)
- [Input](01-the-program.md#input)
  - [`Input` and `Input0`](01-the-program.md#input-and-input0)
  - [The mouse driver](01-the-program.md#the-mouse-driver)
- [The screen](01-the-program.md#the-screen)
  - [`ScrBuf`, and places in one register](01-the-program.md#scrbuf-and-places-in-one-register)
  - [`Window` and `MovScr`](01-the-program.md#window-and-movscr)
  - [What a box covers](01-the-program.md#what-a-box-covers)
  - [Colours](01-the-program.md#colours)
- [Boxes, dialogs and menus as tables](01-the-program.md#boxes-dialogs-and-menus-as-tables)
  - [A line and its markup: `NgLine`](01-the-program.md#a-line-and-its-markup-ngline)
  - [`DialBox`: a box is 8 bytes and its lines](01-the-program.md#dialbox-a-box-is-8-bytes-and-its-lines)
  - [`Dialog`: a header of 10 bytes, a field of 11](01-the-program.md#dialog-a-header-of-10-bytes-a-field-of-11)
  - [The menu bar: `MnuTab` and a table of commands](01-the-program.md#the-menu-bar-mnutab-and-a-table-of-commands)
  - [The key bar](01-the-program.md#the-key-bar)
- [The help: `VC.HLP`, one entry at a time](01-the-program.md#the-help-vchlp-one-entry-at-a-time)
- [The clock and the blanker](01-the-program.md#the-clock-and-the-blanker)
  - [The clock](01-the-program.md#the-clock)
  - [The blanker](01-the-program.md#the-blanker)
- [The economies, in one list](01-the-program.md#the-economies-in-one-list)
- [Where to read next](01-the-program.md#where-to-read-next)

### [2. Working with files: the commander's features](02-the-features.md)

What the program does with files: reading and sorting a directory, the tree, the walkers that every group command shares, delete, copy, the viewer, the editor, find file, the user menu and Enter.

- [The room](02-the-features.md#the-room)
- [A panel: `WCBdef`](02-the-features.md#a-panel-wcbdef)
- [Reading a directory: `ReDir`](02-the-features.md#reading-a-directory-redir)
  - [Growing the panel, and the 2,703](02-the-features.md#growing-the-panel-and-the-2703)
  - [One entry, from the DTA](02-the-features.md#one-entry-from-the-dta)
  - [The old list, kept in the room](02-the-features.md#the-old-list-kept-in-the-room)
  - [The extension file comes with the directory](02-the-features.md#the-extension-file-comes-with-the-directory)
  - [The filter: `SetMask`](02-the-features.md#the-filter-setmask)
  - [Five orders, one sort](02-the-features.md#five-orders-one-sort)
  - [Selection](02-the-features.md#selection)
- [The tree and `TREEINFO.NCD`](02-the-features.md#the-tree-and-treeinfoncd)
- [Walking the files: `FncAddr`, `FncSel`, `FncMas`, `DirFunc`](02-the-features.md#walking-the-files-fncaddr-fncsel-fncmas-dirfunc)
  - [One DTA, and the search found again](02-the-features.md#one-dta-and-the-search-found-again)
  - [Directory sizes](02-the-features.md#directory-sizes)
- [Delete and attributes](02-the-features.md#delete-and-attributes)
- [Copy, move and rename](02-the-features.md#copy-move-and-rename)
  - [The copy loop: all of the room](02-the-features.md#the-copy-loop-all-of-the-room)
  - [The bar: 48-bit arithmetic without `MUL`](02-the-features.md#the-bar-48-bit-arithmetic-without-mul)
  - [A disk that fills up](02-the-features.md#a-disk-that-fills-up)
- [The viewer: a 64K window over any file](02-the-features.md#the-viewer-a-64k-window-over-any-file)
  - [Search: `V81_00` and `V82_00`](02-the-features.md#search-v81_00-and-v82_00)
- [The editor: the whole text, and one line apart](02-the-features.md#the-editor-the-whole-text-and-one-line-apart)
  - [Save](02-the-features.md#save)
- [Find file](02-the-features.md#find-file)
  - [Searching inside files](02-the-features.md#searching-inside-files)
- [The user menu and the extension files](02-the-features.md#the-user-menu-and-the-extension-files)
  - [`Run_00`: the macros](02-the-features.md#run_00-the-macros)
- [Enter, and running a program](02-the-features.md#enter-and-running-a-program)
  - [Running it](02-the-features.md#running-it)
- [How the features share the main data](02-the-features.md#how-the-features-share-the-main-data)
- [Where to read next](02-the-features.md#where-to-read-next)

### [3. Talking to DOS and the PC: the system](03-the-system.md)

The conversation with DOS and the PC: the error wrapper, the critical error, Ctrl-Break, the display, the keyboard, the mouse, and the resident part that runs a command and loads the commander again.

- [`SecDat`: the main data from anywhere](03-the-system.md#secdat-the-main-data-from-anywhere)
- [`Intr21`: 62 bytes for every error](03-the-system.md#intr21-62-bytes-for-every-error)
- [The critical error: `FatErr`](03-the-system.md#the-critical-error-faterr)
  - [Taking the interrupt](03-the-system.md#taking-the-interrupt)
  - [Building the message](03-the-system.md#building-the-message)
  - [Asking](03-the-system.md#asking)
  - [Two flags that stop the boxes piling up](03-the-system.md#two-flags-that-stop-the-boxes-piling-up)
  - [The drives DOS would ask about itself](03-the-system.md#the-drives-dos-would-ask-about-itself)
- [Ctrl-Break and Ctrl-C: one `IRET`](03-the-system.md#ctrl-break-and-ctrl-c-one-iret)
- [The screen](03-the-system.md#the-screen)
  - [The display's address](03-the-system.md#the-displays-address)
  - [CGA snow, and Volkov's own question marks](03-the-system.md#cga-snow-and-volkovs-own-question-marks)
  - [What goes through the BIOS](03-the-system.md#what-goes-through-the-bios)
- [The keyboard: `Input0`](03-the-system.md#the-keyboard-input0)
  - [Waiting is not idle](03-the-system.md#waiting-is-not-idle)
  - [Giving time away](03-the-system.md#giving-time-away)
- [The mouse: eight calls of `INT 33h`](03-the-system.md#the-mouse-eight-calls-of-int-33h)
- [The resident part: 12,690 bytes that stay](03-the-system.md#the-resident-part-12690-bytes-that-stay)
  - [Why there is one, and when it is all that stays](03-the-system.md#why-there-is-one-and-when-it-is-all-that-stays)
  - [What stays, byte by byte](03-the-system.md#what-stays-byte-by-byte)
  - [`ORG LastRes`: the start-up runs where the vectors will go](03-the-system.md#org-lastres-the-start-up-runs-where-the-vectors-will-go)
  - [`Alloc0`: a sum, a version, a move and a hand-made MCB](03-the-system.md#alloc0-a-sum-a-version-a-move-and-a-hand-made-mcb)
  - [Entering the transient part](03-the-system.md#entering-the-transient-part)
  - [Going: `F21_31`](03-the-system.md#going-f21_31)
  - [Running: `Exec`, `DosBuf` and four bytes in front of each command](03-the-system.md#running-exec-dosbuf-and-four-bytes-in-front-of-each-command)
  - [Coming back: `VC.COM` loads itself](03-the-system.md#coming-back-vccom-loads-itself)
  - [Where DOS puts the blocks: `GetStat` and `SetStat`](03-the-system.md#where-dos-puts-the-blocks-getstat-and-setstat)
  - [The TSR manager: `New21`](03-the-system.md#the-tsr-manager-new21)
  - [The ways out](03-the-system.md#the-ways-out)
- [What it adds up to](03-the-system.md#what-it-adds-up-to)
- [Where to read next](03-the-system.md#where-to-read-next)

### [4. The setup program: `VCSETUP.COM`](04-the-setup.md)

`VCSETUP.COM`: its segment and menus, `VC.INI` as a dump of the main data, the private call by which it hands a running commander its settings, what it copies of the commander, and its help.

- [What the program is](04-the-setup.md#what-the-program-is)
  - [The segment](04-the-setup.md#the-segment)
  - [Its conversation with DOS and the PC](04-the-setup.md#its-conversation-with-dos-and-the-pc)
  - [The menus](04-the-setup.md#the-menus)
- [`VC.INI`: a dump of the main data](04-the-setup.md#vcini-a-dump-of-the-main-data)
  - [Why a dump](04-the-setup.md#why-a-dump)
  - [The format, byte by byte](04-the-setup.md#the-format-byte-by-byte)
  - [Reading it: the commander](04-the-setup.md#reading-it-the-commander)
  - [Reading it: the setup program](04-the-setup.md#reading-it-the-setup-program)
  - [The defaults, and 64 bytes that are not](04-the-setup.md#the-defaults-and-64-bytes-that-are-not)
  - [Writing it](04-the-setup.md#writing-it)
- [The private call: `INT 21h` AH 2Bh, `'VC'`, `'OM'`](04-the-setup.md#the-private-call-int-21h-ah-2bh-vc-om)
  - [The problem](04-the-setup.md#the-problem)
  - [The call](04-the-setup.md#the-call)
  - [What the setup program does with it](04-the-setup.md#what-the-setup-program-does-with-it)
- [What it shares with `VC.COM`, by copy](04-the-setup.md#what-it-shares-with-vccom-by-copy)
- [Its help: pages 55 to 61 of `VC.HLP`](04-the-setup.md#its-help-pages-55-to-61-of-vchlp)
  - [One file for two programs](04-the-setup.md#one-file-for-two-programs)
  - [The file](04-the-setup.md#the-file)
  - [Read once, at start](04-the-setup.md#read-once-at-start)
- [What it adds up to](04-the-setup.md#what-it-adds-up-to)
- [Where to read next](04-the-setup.md#where-to-read-next)

## The screens and the source

The pictures in `img/` are screens of the shipped program, recorded as
it ran and drawn in the VGA font. The source that the book quotes is
in [`src/asm/`](../../src/asm/), CP866 text with CRLF line endings, as
Volkov published it.
