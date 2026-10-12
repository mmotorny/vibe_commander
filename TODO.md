# To do

Things that are known and not done. Standing instructions are in
`CLAUDE.md`; this is the list of loose ends.

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

- Of `VCSUB2.INC` three routines are still assembly: `Blank`, the
  screen blanker, 402 bytes, `LeaveTm`, 46, which tells DOS, Windows and
  DESQview that the program is idle, and the two bytes of `PutMnu3`,
  which load a 3 and run into the stub of `PutMnu`. `Input0`, which is C,
  calls the first two through entries. `Blank` runs when no key has been
  pressed for as many minutes as the setup says, one at the least, and
  the harness has no step that waits: it presses the next key as soon as
  the program waits for one. A step that lets the machine run until its
  BIOS has counted a minute of timer ticks would do: the driver of the
  tests counts them for the clock already, and has an event that lets
  rounds of the program pass. What `Blank` shows could still not be
  compared: where its stars are is drawn from random numbers that the
  program steps every time it goes round while it waits. That the screen
  comes back with a key could. `LeaveTm` is three calls of interrupts
  and little else; of its 17 instructions six, for DESQview, run in no
  scenario.
- Of `Input0` in C, four instructions of the original run in no
  scenario and are translated by reading: where the minute has come in
  which the screen is blanked, or midnight has passed before it. They
  are listed as out of reach in `harness/ported.py`. The blanking itself
  runs, in `mouse-keys`: the pointer of the mouse in the top right
  corner blanks the screen at once, and a step that takes it there and
  away again, "Move@79,0 Move@40,12", ends with the screen shown again.
  That shows on a screen which a beep has moved up a line: all of it is
  put back as it should be. A step that ended with the screen blank
  could not be compared, for the stars; one of two such steps tried had
  none yet.
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
  416K free under the original and 412K under the port, whose part that
  stays is 3,840 bytes longer by its stack. With those numbers left out,
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
    15; the C copies from the assembly, `Run2_S`, as long as that has
    them.
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
- Of the viewer's keys, 49 instructions have run in no scenario and are
  translated by reading, listed in `harness/ported.py`: the mouse, 36
  of them; Ctrl+Right at column 65455; Up to a line with none behind
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

- Of the help, what it does for the mouse has run in no scenario, 105
  instructions of `Help`: a button held above the text or below it, a
  click and a second one on a line of the index, the middle button, a
  click outside the window. The bench has no mouse. It is translated by
  reading, and listed as out of reach in `harness/ported.py`.
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
- "N files found." is put together from two texts of the assembly,
  each copied with the 15 bytes that begin with it: behind the full
  stop TmpBuf gets seven bytes of whatever follows `FnFl7_S` in the
  program. Nothing reads them; the C copies from the assembly, as long
  as that has them.
- What `FndMove` does for the mouse has run in no scenario, 96
  instructions: the middle button, a button held above the list or
  below it, a click outside the box, a click and a second one on a line.
  It is translated by reading, and so are the two places where
  `FindFil` runs out of memory, for its list and for the file it reads.
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
  which the stub of the menu bar jumps to the commands, the stubs of the
  commands, 28 of them with `M17_00`, which hands Drive... to the box of
  the drives, the boxes, dialogs and texts, which `DialBox` and `Dialog`
  read in the assembly, and one entry for the C, `SaveSet_a`. The stubs
  go when the loop that jumps to
  them is C, and with it `CmdM01`, `CmdM03` and `CmdM06` of
  `VCFUNC.INC`, by which the keys of the panels go on in three of the
  commands: `port/keys.c` can call the functions then.
- Of the commands of the menus, three instructions are listed as out
  of reach: one of EGA lines that keeps the pointer of the mouse on the
  screen when it is below the 25th row and 50 lines become 25, since a
  step of the mouse names a place among the first 25 rows, so it is
  translated by reading; one for a display that is neither EGA nor VGA;
  and one for a kind of panel that nothing asks for. A step that can
  put the pointer further down on a screen of 50 lines would reach the
  first.
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
- Of the menu bar, what it does for the mouse has run in no scenario, 94
  instructions, `F40_50` among them, which says whose name on the bar
  the mouse is at. It is translated by reading, and listed as out of
  reach in `harness/ported.py`. So are Alt by itself in an open menu,
  which only counts if it comes up within three ticks; EGA lines dark,
  for a display that is neither EGA nor VGA; an open menu whose items
  are dark down to the last; and
  an item of the Files menu chosen with Shift or Ctrl held down, three
  instructions, which the port did like the original when it was tried
  by hand, once: the harness holds the two keys for 20 ms for the port,
  and the program asks for them after it has taken the key.
- With the mouse the menu bar can come to have no menu at all, the
  number 0, while it goes on as if it had one: a button pressed on the
  bar next to the names and moved onto one leaves 0 for a round, and
  the page of the help is then read by a word in front of the table of
  the menus; a button let go at that moment, or the mouse leaving the
  bar between two looks at it after a menu was closed that way, has the
  keys work on menu 0, and a menu opened then is taken from 2,040 bytes
  behind the table. Ported like that as far as it can be: the C has the
  tables in its own place, so what it reads there is something else. The
  bench has no mouse.
- In an open menu the key of an item that is dark, Ctrl+a in the Files
  menu of a tree, is not dropped: the number of the item is looked for
  among the letters of the items, where it is not. And the table of
  those keys has F1 for the Help item of the Files menu, which `Input`
  never hands on: it shows the help of the item under the cursor
  itself. Both ported as they are.
- `harness/deviations.py` knows the banner that the program prints at
  its start by its beginning. With a panel on the left and none on the
  right, the first row shows the end of it behind the panel, "are", or
  "ware" under the original, whose banner names an edition with a
  letter more: that is held against the golden as it is, and the port
  fails there without anything translated. No scenario shows it now;
  the comparison would have to know the end of the banner by its place.
- Of the dialogs, what they do for the mouse runs in `mouse-dialog`,
  `mouse-fields`, `mouse-line` and `mouse-filter`, but for two
  instructions that are listed as out of reach in `harness/ported.py`:
  the middle button, or two buttons at once, which leave a dialog as Esc
  does, and a button held on the button that is chosen and moved off it.
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
- Of the box in which a directory is chosen from the tree, what it does
  for the mouse has run in no scenario, 93 instructions of `ChTree`: a
  button held on a line, above the lines or below them, a second click
  on a line, a click outside the box. It is translated by reading, and
  listed as out of reach in `harness/ported.py`. So are 17 instructions
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
- `VCCOPY.INC` is C but for its boxes and texts, which `DialBox` and
  `Dialog` read in the assembly, and the stubs: of the commands, of what
  their dialogs call for the tree, and of `C00_00`, which `FncSel` and
  `FncMas` call. Those two, in `VCF1.INC`, are C as well.
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
- Of the map, 187 instructions of 913 have run in no scenario and are
  translated by reading: the mouse, 136; a map of more lines than its
  window has, 13 on a screen of 25 lines, with the line that scrolls in,
  22; a DOS before version 4 or with a version of two digits, 15; a
  block of an unknown owner, 4; and a second program that stays in the
  memory and is not released with the first, 10. The bench has one such
  program, FDAPM of the floppy, which does not stay twice.
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
    puts the cursor on a third digit, which is taken for the second;
    and a digit typed on the second digit of the byte behind the last
    one is cleared again. By reading: the bench has no mouse.
  - While a file is read, and before and after a search, the keyboard
    is looked at for Esc, Ctrl-C and F10. While it is read that cannot
    be seen: the viewer's code looks first, and takes the key.
- Of the editor, 81 instructions have run in no scenario and are
  translated by reading, listed in `harness/ported.py`: 74 for the
  mouse; two for a file that DOS will not read; three where the window
  would move up by fewer lines than it does, or begin above line 0;
  and two for a cursor behind all that the buffer has of a line, which
  only a click of the mouse leaves there.
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
- The bench has a mouse now, and three scenarios use it: `mouse-panel`,
  `mouse-bars` and `mouse-dialog`. With them 349 of the instructions
  that `harness/ported.py` listed as out of reach for the mouse run in
  the original, and their C in the port, and are off the lists. Some 800
  are still listed, with reasons that end in "the bench has no mouse",
  which is no reason any more: each place wants a scenario, or the true
  reason why none gets there. By the file of the port that has them:
  - `treebox.c`, 93: `ChTree`, a button held on a line of the tree,
    above the lines or below them, a second click on a line, a click
    outside the box.
  - `menu.c`, 19: what is left of the menu bar, a button held on the bar
    or on a menu and let go, a menu open with no item under the cursor.
  - `view.c`, 36: a button held in the viewer's window.
  - `edit.c`, 38: what is left of `T00_71`, a click in the editor's
    window, a button held above it or below it. `mouse-list` runs 36
    of its instructions: the click that opens the editor on dirinfo
    has the button still down when the editor asks.
  - `help.c`, 105: a button held in the text, above it or below it, a
    click on a line of the index and one outside the window.
  - `find.c`, 96: `FndMove`, the list of the file search.
  - `dialog.c`, 2: `mouse-fields`, `mouse-line` and `mouse-filter` have
    the fields of a dialog. Left are the middle button, and a button
    moved off the button of a dialog that it went down on.
  - `usermenu.c`, `cmdline.c` and `drive.c`, 114: the rest of the user
    menu with `F32_70`, the box of the drives, the history.
  - `memory.c`, 136: `F65_00`, the map of the memory, either button on
    its lines and buttons, held above or below, the middle button, a
    second click.
  - `input.c`, none: `mouse-keys` has the right button on the key bar, a
    key of the bar that has no name, and the corners of the screen.
  A second click is two clicks in one step; a button held is a click
  with a count behind it, "Click@3,5*200". There is no step yet for a
  button that goes down in one place and comes up in another, nor for
  the middle button: `harness/driver.py` has the packets, and the names
  would be two lines there.
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
- `VCF1.INC` is C but for its boxes and texts and the stubs: `FncSel`,
  which goes through the selected files of a panel, and `FncExt`, where
  a command ends that may have changed the disk, are functions of
  `port/dirfunc.c`, and their stubs are what `F1Next` and the table of
  Copy and Rename jump to. They were code in the program's data, behind
  `DirFunc`; `harness/coverage.py` names such code by its label now.
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
  the stubs, the table by which they jump on, `CmdNx_T`, boxes and
  texts. Enter, `F21_00`, saves what the program needs to start over,
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
  command through the table of keys there, `FncTab`, because the
  commands are labels of the assembly. When they are functions, the
  table is a `switch` in the loop and the structure of registers goes.
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
- What the keys of the panels do for the mouse has run in no scenario:
  26 instructions of `F00_60`, which select the files that the mouse
  passes over with its right button held, and the three places where the
  mouse comes in, `F00_05`, `F01_05` and `F00_60` with buttons. They are
  translated by reading. So is what Tab does when DOS will not go to
  the directory of the other panel, or has not told the current one,
  18 instructions of `F20_02`: whatever removes or renames a directory
  has both panels read again, and the drives of the bench are ready.
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

- What the user menu, the history and the box of the drives do for the
  mouse has run in no scenario, 164 instructions: all of `F32_70`, which
  the menu and the history share, and the box's own, a button held on a
  letter and let go on it or elsewhere. They are translated by reading
  and listed as out of reach in `harness/ported.py`, with a box of the
  drives too wide for its title, 15 letters or more, a drive that is not
  ready, and a list file that DOS will not give attributes to or write.
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
- A box that is drawn before the main loop has set `AutoTim` gets no
  key from the driver of the tests, and its step never ends. The main
  data is memory newly asked for when the program starts and after
  every command of DOS, and `AutoTim` there is whatever that memory
  holds until `Func10` writes it, once the panels have been drawn. The
  driver reads `AutoTim` every time the program idles, and while its low
  word is not 0FFFFh it takes the program to have something pending,
  counts no idle round and hands over no key. So it was with `b:` run
  as a command from the program's command line: back from DOS the
  panels are drawn, the current drive is the floppy's second letter,
  and "Insert diskette for drive B:" stands on the screen with the
  driver silent, until the 300 seconds of `STEP_TIME` are over; Esc or
  Enter put behind the command in the same step are not handed over
  either. The same would hold for the box of a drive that is not ready
  and for a critical error of DOS while the panels are first drawn.
  A box that comes up when the panels are drawn again later, as in
  `not-ready`, is answered like any other: `AutoTim` is 0FFFFh from
  the last time round.
- What ran in the original is not recorded the same every time for a
  step that clicks on an entry of a panel. `DoublMo` adds 0.4 seconds
  to the time of day, and three instructions of it, `INC DH`, `CMP
  DH,60` and `JB DblMo2` at 9506h, run when the hundredths are 60 or
  more. Of five derivations of `mouse-list`, one had them not run and
  four had them run, with the same screens every time; the golden of
  `mouse-panel` has them as run too. The hundredths at which a step
  happens are not the same from one run of a machine to the next: the
  machine's clock follows the computer's while the BIOS waits for a
  disk. `SUB DL,100` in `Func07` came and went the same way when the
  clock was changed over. It matters when `DoublMo` is translated and
  its instructions have to have run. The double click of `mouse-list`
  itself, two steps within those 0.4 seconds, came out the same in
  every derivation and in the port; how much room it has was not
  measured.
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
  - The start-up from `Init00` to `Init40`, 1,266 bytes, with `MainPth`,
    `ReadOne`, `Expand`, `CurNl` and `OutStr1`: plain code, the next
    batch. `Init00` to `Init11` runs before there is a main data
    segment, with DS on the part that stays in the memory, where the
    setup file and the main extension file are read to: C functions
    that take that segment as far pointers and touch nothing of `md`.
    From `Init11` on it is the main data. It asks much of the BIOS and
    of DOS: the video mode set, the cursor's shape, the country, the
    font's rows, DESQview, for which `Int10` and `Int21d` of
    `port/dos.c` do, and the address of a virtual screen (INT 10h,
    AH=FEh), which answers in ES:DI and has no function there yet. It
    calls the routine of DOS that makes capitals through the far
    pointer of the country information, with a character in AL: a far
    function pointer with a `#pragma aux`. It writes two vectors into
    the table at 0:0 with interrupts off, and looks for snow by reading
    port 3DAh in a loop that is timed by its instructions: `inp()`,
    `_disable()` and `_enable()` are intrinsics, and how many times the
    loop has to see the bit clear will want measuring again in C.
    `MainPth` is not reached for a `VC=` in the environment and
    `CurNl` not for a cursor on the last line; a second copy of the
    program run after `set vc=` and after enough lines of output would
    reach both.
  - `Reloc`, which the original has not: applies the linker's
    relocations to the copy of the program that has just been loaded.
    It cannot be called far before it has run, since a far call
    between two segments is itself relocated. It is C once what calls
    it is C in the same file: a static function is called near.
  - The entry of the program, `Alloc0` to `Alloc9`, 256 bytes: adds up
    the check sum of the file; as the second copy, loaded to bring the
    program back after a command, moves its code into the first copy's
    memory and ends; otherwise shrinks its block, moves everything
    from `Init00` on above the part that stays, over itself, makes a
    block of DOS of that by writing its header, and jumps there. DS and
    ES are on the PSP and the stack is the one of the EXE header, in
    memory that the move overwrites, so `MovPars` uses no stack. In C
    this is a function of the part that stays, which is not moved: the
    stack has to be set first, SS and SP, two instructions that C
    cannot say, and the move is then a loop over far pointers. It needs
    a code segment of its own for the part that stays, placed first in
    the file, with its own notion of DS: there is no main data yet.
  - `Exec`, 470 bytes, in the part that stays: runs the commands of
    `DosBuf` one after the other, by COMMAND.COM or by INT 2Eh, which
    comes back with SS and SP destroyed, and loads the program again by
    running its file as a second copy. The stack set again after INT
    2Eh is inline assembly, three instructions. The rest is C of the
    part that stays, where DS, SS and CS are one segment and the main
    data does not exist: what the commands need is in the copy that
    `F21_31` keeps there, `ComnIni`.
  - The jumps between the two parts: `F21_31` to `Exec`, `Exec15` to
    `Init11`, `Exit` to `Exit00`, `Init12` to `AlocEr`, and `CallPSP`,
    by which the code that goes calls routines of the part that stays.
    Each is a far address made of a segment known when the program
    runs and an offset: in C a call through a far function pointer. A
    jump is not a call, though: a call that never returns would leave
    its return address on the stack with every command run. The way
    out that needs no assembly is to turn the jumps round. The part
    that stays has the outermost loop: it calls what `Init11` begins,
    by a far pointer, and that returns, through the main loop, when a
    command is to be run or the program left. That asks that every
    way to `F21_31` and to `Exit` be a return from the main loop,
    which they are once the commands are functions: they return
    numbers already.
  - `Exit00` to `Exit05`, 106 bytes: gives the terminate address of the
    PSP back, releases the programs that have stayed in the memory,
    gives INT 21h and INT 27h back and ends the program. `Exit01` is
    also what the PSP names as its terminate address while the program
    runs, so DOS can jump to it with no stack of the program's: a stack
    set by inline assembly at that entry, or the address in the PSP
    made that of a handler written like one for an interrupt.
  - `New21` and `New27`, 113 bytes, with `ExeProg`, `AddProg`, `FreeBlk`
    and `ChkBlk`, 354 bytes: the program's hook on DOS, which notes
    every program that is run and every one that stays in the memory,
    with the vectors it has changed, and answers the question of
    other programs for the program's data (AH=2Bh, CX='VC', DX='OM').
    They run on the stack of whoever called DOS, know only CS, and
    hand the call on to DOS with every register and the flags as they
    came. `__interrupt` functions of Open Watcom save the registers
    and give them to the C as a structure; they load DS with a segment
    that the linker names, which here has to be the part that stays
    and has to be relocated by `Reloc`. To go on in the old handler
    they need `_chain_intr`, which is of the C library that the
    program does not have: a few instructions of inline assembly that
    put the registers back and jump far. `VCapi` returns with the
    flags as it sets them, `RET 2`: an `__interrupt` function does
    that by changing the flags in its structure. The table of
    vectors is copied with interrupts off: `_disable()`.
  - `Release`, 104 bytes, `GetStat` and `SetStat`, 68 bytes, `SetBrk`
    and `SetBrk0`, 39 bytes, `NotFind` and `OutStr0`, 54 bytes: plain
    routines that happen to be of the part that stays, called far from
    the rest through `CallPSP`. `GetStat` and `SetStat` keep what they
    found in BP from the one to the other: an argument in C. They wait
    for the code segment of that part.
  - `FatErr`, 220 bytes: what DOS calls for a critical error. It is
    entered with nothing but CS known, takes its error in AX and DI
    and the device in BP:SI, shows a dialog, and returns the answer in
    AL by IRET. It finds the main data by a word that the start-up
    code writes into the code segment, `SecDat`, which `Intr21` reads
    too. An `__interrupt` function gets and sets those registers, but
    C code has no word in its code segment to write, and nothing in C
    sets DS to a segment known only when the program runs: loading DS
    from where the program keeps the segment is inline assembly, two
    or three instructions, here and wherever else the program is
    entered from outside.
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

- Of the shared routines of `VCSUB1.INC` two are still assembly. `Case`,
  37 bytes, jumps by a table of keys to a label of whoever called it, and
  takes that caller's return address off the stack: one caller is left,
  `MainKy` in `VC.ASM`, where the main loop, which is C, has the key
  looked up in the table of the commands, whose labels are stubs. It
  goes when that table is a `switch`.
  `CallPSP`, 6 bytes, calls a routine in the part of the program that
  stays in the memory, far, by way of the stack; `OutStr1` and
  `Release_a` call it. Of `VCSUB3.INC` four are still assembly, 232
  bytes, the next batch there with `LeaveTm` and `Blank` of
  `VCSUB2.INC`: `SaveSet`, `MakeIni` and `MkIni1`, which save the setup,
  and `DiSi10`, which `DirFunc` calls for the sizes of directories, by
  its address. `SaveSet` has run in no scenario: the bench has the setup
  saved before the scenarios begin.
- When `LeaveTm` becomes C its `INT 28h` goes into `port/dos.c`, with
  the calls of `INT 2Fh` and `INT 15h` behind it. The driver of the
  tests takes a call of `INT 28h` for the program's when it comes from
  the memory of the program that DOS has as the current one and that
  program is named `VC`, so the call may stand anywhere in the port and
  `harness/driver.py` needs no change for it. What the call needs is DS
  on the main data, where the driver reads `AutoTim` and `Clock`, and
  the program's own stack: any C function has both.
- Entries by which C code calls DOS are still in the assembly, each a
  call of `INT 21h` that the original made without `Intr21`: `SelDrv_a`
  in `VCCOPY.INC`, `Fcb29_a` in `VCF1.INC`, and `Dos5802_a`, `Dos5803_a`,
  `Dos52_a`, `Dos4452_a` and `Dos2523_a` in `VCMEM.INC`. They belong in
  `port/dos.c`, or go through `Int21d` there: `struct DosRegs` has ES
  as DOS leaves it, for `Dos52_a`, and DS with DX, for `Dos2523_a`. The
  two that the main loop and the commands of the menus had made, for
  the BIOS and for where an interrupt goes, went that way: `Int10` and
  `Int21d`.
- Of the mouse's routines, which are C, ten instructions of the
  original have run in no scenario and are translated by reading: six of
  `IniMous`, for a mouse driver that does not know the call 21h and for
  no mouse at all, which every machine of the bench has now, and four of
  `DoublMo`, where a first click falls into the last 40 hundredths of a
  minute. The rest ran when the bench got its mouse, and the C did what
  the original does in every scenario of the mouse the first time it
  was tried. Three more instructions of `DoublMo`, at 9506h, where the
  hundredths of the first click are 60 or more, ran or did not from one
  derivation of the goldens to the next when the main loop was
  translated, by the time of day of the click. They are counted as run:
  if a derivation comes out without them, the `port` test will say "not
  executed" of them, and that is the clock, not the port.
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

## Ideas put off

- Keeping the booted bench from one run of a tool to the next. Booting DOS
  for the two countries and making the bench's setup file take about five
  seconds of every run. What is kept would have to be thrown away whenever
  the harness, a download or QEMU changes.
- A file or a directory that a scenario makes gets the time of day, to
  the second, and the golden lists it; deriving is slower than testing, so
  the two differ. No scenario makes one yet. Such a time stamp could be
  recorded only as made during the run.
