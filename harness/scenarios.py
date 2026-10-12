"""Scenarios: what is done to Volkov Commander to see how it behaves.

A scenario starts the program on the test bench (see setups.py) and presses
keys, one step at a time. After the start and after every step the screen
is recorded, once the program has done all that the keys make it do; at the
end, so is everything on the hard disks. What the original records is kept
as the scenario's golden (see goldens.py), and a build is right if it
records the same.

The keys are pressed inside the machine, by a driver that DOS has loaded
before the program (see driver.py and DRIVER.ASM). It hands the keyboard
controller what the keyboard would send, one key when the program waits
for a key and has taken the one before, and writes the screen out when a
step has no key left and the program waits, with nothing that it will do
by itself after a while. The original and the port are driven alike, as
they are: nothing in either knows of the tests.

Steps are written as driver.py takes them: key names separated by
spaces, "Alt+F1" for keys held together, and text to type in double quotes.
Text may be Russian where the bench has the Russian keyboard, once
"Alt+RightShift" has selected its Cyrillic letters. That is pressed where
the program takes text: at the panels Alt by itself, down and up again,
brings up the menu bar.

What has been found out about driving the program, to save finding it
out again (show.py --changes is how):

  - In a dialog Tab goes to the next field, one at a time: four boxes to
    tick are four Tabs. Up and Down move within a group, Space ticks a box
    or chooses one of a group, Enter is the first button and Esc cancels.
  - Typing into a field that holds something replaces it.
  - A box that only reports, "Could not find the string", stays until a
    key is pressed: close it with Esc before the next step.
  - Shift+F3 and Shift+F4 ask for the name of a file to view or to edit;
    Ctrl+a is the dialog of attributes, date and time; GrayPlus and
    GrayMinus select and unselect by a mask; F4 in the viewer is hex.
  - Ctrl+y empties the field that the cursor is in.
  - In the dialog of attributes "Tab Tab Space" ticks Hidden and "Tab Tab
    Tab Space" System; from System two more Tabs go to the time. For a
    directory, or for selected files, the dialog is another, with a
    column to set and one to clear, and no date; and when a directory has
    been given attributes the panel is read without regard to where the
    cursor was.
  - The order of a panel is in its menu only: "F9 l n", x, m, s and u for
    name, extension, time, size and unsorted. Ctrl+F3 does nothing and
    Ctrl+F4 asks for the volume label. "F9 l f" is Full and "F9 l l" the
    filter: a mask, then two Tabs to "Executable files". Ctrl+h switches
    hidden files off and on. "Alt+F1 a" is another drive.
  - "F9 o f" switches Full screen, which is on: off, a panel has ten lines
    of files. In the Configuration dialog four Tabs go to "Menu bar
    always visible". Alt+F9 switches to 50 lines and back; a screen is
    recorded as its first 25 rows whatever it has.
  - F5 on a file and a name typed makes a copy next to it that keeps the
    time stamp, so copies are a way to more files: C:\\WORK lists nine
    entries. Ins selects and goes down. F8 on what is selected asks
    twice, and once more for every directory that is not empty: Enter
    each time.
  - A:\\ is the disk DOS started from: three files before a directory, of
    other dates than the bench's, one of them with a name of twelve
    characters. D:\\ has the directory of the program and nothing else.
  - Ctrl+l makes the right panel one of files, in C:\\; "F9 r" is its
    menu, as "F9 l" is the left one's. A mask and "Executable files" are
    both shown behind the path over a panel, so the filter is a way to a
    long one: C:\\STAMPS\\SUB, the longest on the bench, is 13 characters.
    "F9 o n" switches the mini status off.
  - In the Configuration dialog "Down Space" from where it opens chooses
    the laptop palette, in which a selected file has a mark.
  - Shift+F4, a name that is not there and Enter twice open the editor on
    a new file; a letter, F2 and Esc leave a file of one byte, which
    comes last in its panel. It has the time of day. To give it another,
    press Ctrl+a, the Tabs, the time and Enter in one step: the dialog
    shows the seconds, and a step shows only what is there at its end.
    Five Tabs go from the attributes past the date to the time, and
    "12:00:00p" is the time of the bench's files.
  - The editor empties the keyboard buffer after every key it takes, and
    once more before it draws: a key that arrives while it deals with the
    one before is lost, now and then, and more often in the port, which
    the tests run side by side. A scenario that types more than one key
    to a step there first switches that off: "F9 o o" is the dialog of
    advanced options, five times Down goes to "Clear keyboard buffer",
    Space and Enter. Tab types a tab there and Enter ends a line.
  - F7, a name and Enter make a directory and leave the cursor on it, so
    one more Enter goes into it: thirteen such steps make a tree thirteen
    levels deep. "F9 r t" shows the tree in the right panel; it is read
    from the disk again when directories have been made. Alt+F10 shows
    it in a box, to choose a directory from; Esc leaves that.
  - In the box of Alt+F10 a letter is the speed search: the cursor goes
    to the next directory whose name begins with what has been typed,
    from where it is; Ctrl+Enter goes to the one after that and round,
    Backspace takes a letter back, and a letter that fits no name is not
    taken. F2 and Ctrl+r read the disk again, Enter goes to the directory
    and Esc or F10 leaves. On a screen of 25 lines the box has 14 lines.
    The tree of a disk is kept in a file in its root, TREEINFO.NCD, when
    it has five entries or more, the root counted: C: has six, and A:
    five, FREEDOS with BIN, NLS and CONFIGS in it. "Down Ins Ins F8" and
    two Enters at the start delete DOCS and SRC, which leaves four. A
    directory of the file's name, F7 and "\\treeinfo.ncd", keeps the file
    from being written: that beeps, and the screen is a line further up.
    On A: it cannot be written either, and nothing says so.
  - Keys of one name, "F2+Esc", go down together and reach the keyboard
    buffer one behind the other, with nothing in between: the second is
    waiting when the program takes the first. That is how a key stops
    what the program does without waiting for one, the reading of the
    tree: pressed as a key of its own, in the same step or the next, it
    arrives when the reading is over.
  - The Info panel shows the file dirinfo of the directory of the other
    panel, once that has been read again, as it is after the editor.
    "F9 o m" switches Memory allocation, and the Info then says how much
    memory is free with all of the program loaded: that depends on the
    size of the program, so the port shows another number than the
    original. The scenario memory does it. deviations.py has the Info's
    free memory left out of the comparison, there and in every other
    scenario. Every disk of the bench
    has a volume label, and the program does not get one off: Ctrl+F4,
    Ctrl+y, Enter and Enter again ask whether to delete it and leave it
    where it is.
  - A text that is typed has no space in it: 'rd work' is written
    '"rd" Space "work"'.
  - At the panels the command line takes the keys that the panels do
    not: Ctrl+Left and Ctrl+Right go by words, Ctrl+Home and Ctrl+End to
    the ends, Ctrl+s and Ctrl+d by a character; Ctrl+g, Ctrl+t, Ctrl+w,
    Ctrl+k and Ctrl+y delete a character, a word forwards and backwards,
    the rest and all; Ctrl+q has the next key typed as it is. Ctrl+Enter
    puts the name under the cursor of the panel into the line, when the
    cursor of the line is at its start or behind a character that words
    end at, and looks in the history otherwise. The line holds 123
    characters and the screen shows some 70 of them: seven names of
    eleven make it move, twelve fill it. The beep of a full line does not
    scroll the screen, with the cursor in the line. Ctrl+b switches the
    key bar off and on.
  - "time", Space, "9:59" and Enter set the machine's clock by DOS's
    command. A scenario that shows the clock then does so within the
    minute that begins there, and makes no file afterwards: the harness
    takes a time stamp for one of the run only in the ten minutes from
    noon.
  - A line of an extension file that begins with a colon is for files
    without an extension. In the editor Ctrl+Left goes back to where a
    word begins and Ctrl+Backspace deletes back to there.
  - "F9 o d" switches Auto directory sizes: the mini status then has the
    size of a directory, of all files in it and below, once its list has
    been read. A new file comes last in an unsorted panel, so End goes to
    it. F5, "src\\" and Enter copy the file under the cursor into the
    directory SRC; F5, a name and Enter make a copy next to it.
  - Ctrl+r reads the directory again and puts the cursor at the top with
    nothing selected. What is selected stays so when the list is read
    again after F7 has made a directory. Ctrl+PgUp puts the cursor on the
    directory it came from. Ctrl+u has the panels change places. F8 on a
    directory that is not empty asks twice: Enter each time; the other
    panel, if it showed that directory, shows the current one then.
  - Enter on a file runs the command that the extension file has for
    its kind: the main one, of the program's directory, has zip, arj,
    lzh and asm, and a directory may have a vc.ext of its own, a line
    like "txt:", a tab and "echo !.!". Shift+F4, the name and one Enter
    open the editor on it. Ctrl+o shows what a command printed.
  - A directory named DIRINFO is what the Info panel cannot open: it
    then says that the disk has no label, which it has not looked for.
    Ctrl+F4 shows the label in a dialog, which Esc leaves.
  - The dialog of Alt+F1 offers A, C and D. The one floppy drive is B:
    as well, and DOS has its diskette be that of one letter at a time:
    the dialog offers the letter that it is. A file of the other letter
    named, Shift+F3 and "b:\\fdauto.bat", has the program ask for the
    diskette, "Insert diskette for drive B:", and Enter there makes it
    B:'s. From then on A: is a drive that is not ready: whatever reads
    it asks for its diskette, and Esc there fails. A panel of files that
    cannot be read is drawn empty and has the box "Can't read the disk
    in drive A:", which takes a letter, Enter to try the letter it
    shows and Esc or F10 to give up. Enter makes the diskette that of
    the letter shown, if the floppy drive has that letter; and given up,
    the diskette was A:'s again, the drive that had failed and the
    current one. So the diskette is made B:'s again before every
    failure. "F9 o d", Auto directory sizes switched on, reads both
    panels again, Ctrl+r the current one, and a panel that could not be
    read is read again when the viewer is left. See not-ready.
  - The driver hands the program a key only when AutoTim says that
    nothing is pending, and the main data is new memory when the
    program starts and after every command of DOS, until the main loop
    has set AutoTim once. A box that comes up before that, while the
    panels are drawn for the first time, gets no key and the step never
    ends: "b:" as a command leaves the program at "Insert diskette for
    drive B:" for the five minutes of STEP_TIME. To see where a step is
    stuck without waiting that long, run show.py with STEP_TIME set to
    a few seconds: the error has the screen.
  - The program run from its own command line, "d:\\vc\\vc", is a second
    copy of whichever program is tested, and what follows the name is a
    command that the copy runs before it shows its panels; F10 and Enter
    leave it. Its Info counts all the memory from where the copy is
    loaded, which follows the size of the first copy's part that stays:
    have the setup saved without an Info first. "F9 o Enter", four
    Tabs, Down and Space tick Auto save setup, and the setup is then
    saved whenever it has changed, the directories of the panels and
    which one is current apart. See second-copy.
  - In the viewer F2 switches wrapping, F4 hex, F7 asks for a text and
    Shift+F7 looks for it again; Right and Left move a line that is not
    wrapped by a column, with Ctrl by forty. Home in a file that has
    been paged down goes back to its start. A text typed for F7 in small
    letters finds capitals. The viewer has 65504 bytes for a file, less
    two for every line: A:\\FREEDOS\\BIN\\COMMAND.COM, 87772 bytes, is the
    one file on the bench that does not fit, and is read a part at a
    time. It has 25 line ends in all, so not wrapped a PgDn or a PgUp
    leaves the part: that is three steps to the reading of the next part
    and of the one before, where wrapped it is some twenty PgDn. The
    file of all bytes, data.bin, has no line end at all.
  - A search in the viewer begins at the first line of the screen, and
    once a text has been found, on the line behind it: in a file of one
    line, data.bin not wrapped, the search after one that found
    something finds nothing. A search that finds nothing has the next
    one begin at the place found last, not on the line behind. What is
    found off the screen moves the line: "xyz" is found as XYZ, in
    column 94 of data.bin. "backslash" is in the shell of A: once, at
    66317, behind what the viewer holds of it: the file is read on for
    it and the part around the place shown. Not wrapped, the fifth line
    of that file begins in the second half of the part, so four times
    Down read the next part; PgDn at the start does too, and Up at the
    start of that part reads the one before.
  - A key stops a search of the viewer where it goes on from what is
    in the memory to reading the file: "Enter+Esc" in the dialog of F7,
    as in the editor, with "Clear keyboard buffer" switched off first.
    The screen is the same as if Esc had closed the box "Could not find
    the string" afterwards; what ran says which.
  - The files of C:\\STAMPS have a size in the directory and no
    contents: DOS reads nothing of them, and the viewer shows zeros, a
    megabyte of them for noon.dat. Renamed to dirinfo, F6, that is a
    dirinfo too large for the editor: Tab makes the Info the active
    panel, F4 there edits the dirinfo in the lower part of the panel,
    and the box that offers View instead takes Enter. That is the one
    way to a viewer in a narrow window, without a status line; in hex
    it writes its lines over what is to the right of the window.
  - Shift+F5 with the name of a file of A:, Tab and a name makes a
    file without leaving C:\\WORK; the same again with another file and
    "p" appends. format.exe and xcopy.exe of A:\\FREEDOS\\BIN are 65109
    bytes together: less than 64K and more than the viewer holds, so
    End and Home there read parts of a file whose length has 16 bits.
  - The box "Can't find the file" of the viewer and the editor beeps,
    which scrolls the screen (see below); "Could not find the string"
    does not.
  - Enter on a file with the extension bat runs it: on A:\\ the second
    file is fdauto.bat, which starts the bench again.
  - A step may press any number of keys: each is handed over when the
    program waits for one and has taken the one before. Keys written as
    one name, "Enter+Home", go into the keyboard buffer together, and
    that holds 15.
  - The bench has a mouse, with FreeDOS's driver for it. The pointer is
    on every screen, a cell whose colours are exchanged: in the middle,
    column 40 of row 12, until a step moves it, and it stays where a
    step leaves it. "Click@3,5" moves it to column 3 of row 5 and
    presses the left button, "RightClick@3,6" the right one, and
    "Move@40,12" only moves it. The button stays down until the program
    has asked 20 times where the mouse is, or "Click@1,24*4" 4 times:
    what the program does while a button is held, it does that often.
    The left button on a file puts the cursor there and the right one
    selects it; on a name of the key bar it is that key; on the top
    line it brings up the menu bar with the menu under the pointer
    open; on an item of a menu it is the item, and beside a menu it
    closes it; in a dialog it is on a button or in a field. The
    scenarios mouse-panel, mouse-bars and mouse-dialog have each of
    these once. In the top left corner it switches the panels off and
    on; on the titles of a list and on its status line it moves the
    cursor up and down for as long as it is held, so to the end; below
    the last file it goes to that; in the other panel it makes that the
    current one, an Info too, and in the lower part of an Info that is
    current it edits dirinfo. Two clicks on an entry in two steps that
    follow each other are a double click, Enter there: mouse-list. No
    step presses two buttons at once. In the box of a drive that is not
    ready the right button is Enter and the left one beside the box
    gives up: not-ready.
  - More of the mouse. A box has a blank row above its frame and one
    below, and blank columns beside it, which are in the box: a click
    outside it is further out, and count the rows of a field from the
    screen, not from the frame. In a dialog a click on a field takes
    the cursor there and acts on it: a line of a group is chosen, a box
    is ticked or not, a line that is typed into has its cursor where
    the click is and keeps its text; a click in the box and on no field
    does nothing, one outside the box is Esc, and the right button
    anywhere is Enter, on the button that is chosen if the cursor is
    on the buttons. The right button on a key of the key bar is the key
    with Alt, and a key of the bar that has no name is no key: a click
    there is one outside the box that is open.
  - On the menu bar a click beside the names shows the bar for as long
    as the button is down; on a name with another menu open it opens
    that one; an item that is dark is none, and the menus close. The
    right button on an item of the Files menu is the item with Shift,
    as Shift+Enter is there, and Ctrl+Enter the item with Ctrl: Copy
    asks for the file too, or copies the one under the cursor whatever
    is selected. Alt by itself in an open menu closes the menu and
    leaves the bar.
  - In the user menu, the history and the map of the memory a click on
    a line takes the cursor there, a click outside the box leaves, and
    the right button outside the box does nothing; in the box of the
    drives a click on a letter is that drive, and the right button
    anywhere is Enter. A click on F1 or F10 of the key bar in the user
    menu is the help or leaves, also where the menu has a line for the
    key.
  - A click on the line that the click before was on is the second
    click of two, in the same step or in the next: the machine's clock
    goes by what the machine executes, and that is little between two
    clicks. The program allows 0.4 seconds, or up to 1.4, by the
    hundredths of the time of day at the first click. To have a click
    come late, take the pointer across the screen and back 64 times
    first: _A_WHILE below. The time of day is the same to the hundredth
    every time a scenario is run with the same program, but the port
    gets to a click some hundredths later than the original: nothing
    may depend on which side of a second a click is.
  - The pointer in the top right corner of the screen blanks it, and
    the stars that are shown then are not the same in the original and
    in the port: take it there and away in one step, "Move@79,0
    Move@40,12". The screen is drawn again from what the program has
    of it, which shows when a beep has moved the screen up a line.
  - The button of a click comes up when the program has asked for the
    mouse 20 times, and the first of those sees it down: "Click@1,24*2"
    has the program see the button once and find it up when it looks
    again, and "*1" not see it at all.
  - A button held where the program goes on doing something, the right
    one below a list, which selects the file that comes into the panel,
    does it nine times with "*12", in the original and in the port
    alike.
  - In the map of the memory a letter is taken by the buttons, whether
    it is one of theirs or not; Tab is a key that gets to the map and
    means nothing there. The one program that can be released is the
    ninth line, FDAPM once it has been run.
  - "Click@20,10>20,22" is a button held while the pointer moves: down
    on column 20 of row 10, to row 22 with the button down, and up
    there. More places may follow, and each may have its count:
    "Click@20,10*4>20,22*30>20,2". A list scrolls for a button that
    went down on one of its lines and is held above or below them.
  - What the middle button is: on a key of the key bar the key with
    Shift, on an item of the Files menu the item with Ctrl, and in a
    dialog, the user menu, the history, the box of the drives and the
    map of the memory the way out, wherever the pointer is:
    mouse-middle.
  - A button that is moved while it is down counts where it comes up.
    Let go beside the key of the key bar, the button of a dialog or the
    letter of a drive that it went down on, it is no key, no button
    and no drive, and back on it, it is; let go outside a box that it
    went down in, it is Esc, and the other way round nothing. In a
    line that is typed into the cursor follows it, in the user menu
    the cursor of the lines, and on the menu bar the menus, each name
    opening its own and each item getting the cursor; let go on an
    item it is that item. A second click that comes up beside its
    line chooses nothing. The right button moved over the files of a
    panel selects each, or takes each off, as it did with the first;
    the program is not asked on the way between two places, so every
    file wants a place of its own: "RightClick@3,5>3,6>3,7". The
    scenarios mouse-drag-dialog, mouse-drag-menu and mouse-drag-lists.
  - The program sees a button at the place it is moved to once more
    than that place's count says, since the look that ends the count
    of the place before finds the pointer there already: with "*1" at
    the second place it sees it there once. "Click@70,0*5>25,0*1" is
    how the menu bar is left with no menu, the button let go at the
    look after the one that found it on a name. Left, Down and Enter
    there open a menu that is none, from behind the table of the
    menus, and F1 a page of the help by a byte that is not the same
    in the original and in the port: do not press them.
  - "MiddleClick@40,12" is the middle button. The mouse driver takes a
    PS/2 mouse for one of two buttons and hands the third on only when
    it has been told, so the bench loads it with /3: that is in
    a:\\fdauto.bat, which find and find-drives list with its size, and
    drive-b and not-ready show in the viewer.
  - The row of a step may be any of 50: on a screen of 50 lines,
    Alt+F9, "Click@1,49" is F1 of the key bar. The mouse driver stops
    the pointer at the last row that the screen has.
  - A second click on the same line is one within 0.4 seconds of the
    first, by the machine's clock, which counts instructions: two
    clicks of one step are that, and so is the first click of the next
    step on a line that the step before clicked on. To click a line
    twice without that, click another in between.
  - In the help a click on a line of the index puts the cursor there
    and a second one shows its page. A button that went down on a line
    and is held above or below the lines moves the cursor, a line for
    every two times that the program asks where the mouse is:
    "Click@20,10>20,22*40" is 20 lines. On a page the same holds for a
    button in the first third of the text or the last, without moving
    the pointer: "Click@30,16*40". A click outside the window leaves,
    and so does the middle button anywhere; beside the text in the
    window a click does nothing. The scenario mouse-help has these.
  - The box of Alt+F7 has a margin around its frame, a row above and
    below and three columns beside, which is part of it: rows 2 to 22
    and columns 3 to 76, with the list on rows 4 to 17. When a search
    is over the list shows its end, so a
    button held below it only takes the cursor to the last line, and
    one held above it moves the list. A second click on a line is Go
    to; a click beside the list does nothing, one outside the box
    closes it, and so does the middle button. A click as short as
    "Click@20,14*3" is over before the list has asked for the mouse,
    and does nothing. The lines of the list are the directories, their
    files, and an empty line between two directories: "*.txt" in
    C:\\WORK is three lines. The scenario mouse-find has these.
  - The box of Alt+F10 has the same margin: rows 2 to 22, with the
    lines on rows 4 to 17. A button that went down
    on a line and is held above the lines takes the cursor to the line
    before the first one shown, and below them to the one after the
    last, each time the box asks: the lines move by one. A second click
    on a row, not on a directory, is Enter: the row is what is
    remembered. The middle button and a click outside close the box.
    On a tree panel the main loop takes the mouse as on a panel of
    files: a click makes the panel the current one and puts the cursor
    on the line, the files of the other panel follow, a second click is
    Enter, and a button held below or above the panel moves the lines.
    Eleven directories made in C:\\WORK are a tree of 17 lines, more
    than the box has and more than a panel of half the height. The
    scenario mouse-tree has these.
  - The viewer takes a button that is down in its window, any of the
    three, for a key as long as it is down: Up in the first third of
    the rows, Down in the last, and between them Left in the first
    quarter of the columns and Right in the last, which move a text
    that is not wrapped. Once it has been in the window the button
    counts outside it too, on the status line. a:\\freedos\\nls\\setup.en
    is a text of 7,948 bytes with long lines. The scenario mouse-view.
  - In the editor a click puts the cursor on the line and the column
    of the pointer, or at the end of a shorter line; in hex on the
    digit or the character. A click in the last column of the window
    on the cursor's own line is Right instead. A button that went down
    in the window and is held above it or below it is Up or Down. A
    click on the dirinfo that the Info panel shows edits it, in the
    window there: rows 11 to 21, columns 42 to 77. The scenario
    mouse-edit.
  - More of the blanker. The pointer has to have been elsewhere since
    the last time for the corner, columns 78 and 79 of row 0, to bring
    it up. It stays while the mouse moves within columns 76 to 79 of
    rows 0 and 1, and goes with a key, which it takes, with Shift, Ctrl
    or Alt, with a button, or with the mouse elsewhere. Its stars are
    drawn from random numbers that begin with whatever the memory held,
    which is why the port's are others: so the key or the movement that
    ends it is in the same step, "Move@79,0 x". The sky changes when
    the timer has ticked, 18 times a second, and the machine's time is
    counted in instructions: one movement of the mouse within the
    corner, which is 500 bytes of a script, lets about half a tick
    pass, and a script holds 64K at the most (the driver counts its
    place in 16 bits, and nothing says so when a script is longer: the
    steps come out wrong). That is some 60 turns of the sky, and a star
    is out for 88 at the least before it changes. For the same reason
    no scenario waits the minute after which the blanker comes up by
    itself. See blank.
  - The setup file is written to the program's directory whatever the
    panels show: with "del d:\\vc\\vc.ini" run as a command it is made
    anew, and with the directory renamed, F6 on it in a panel that shows
    D:\\ and has no Info beside it, saving fails. See setup-new.
  - Nothing sets the kind of a panel to anything but files, tree and
    Info: what the program does for a fourth kind of panel cannot be
    reached.
  - No panel may show D:\\VC: the program is VC.COM there or VC.EXE, of
    another size, and with it what the Info says is free on D:. A file
    of that directory is opened by its path: Shift+F4 and
    "d:\\vc\\vc.ext".
  - In the editor F4 switches between text and hex, where two digits
    are a byte and Tab goes to the characters and back; Shift+F2 saves
    under another name, which the editor then goes by; Esc with changes
    asks, Enter to save and "Right Enter" not to. Ctrl+y cuts the line
    and Ctrl+u puts it back, as often as it is pressed, each time as a
    line of its own; Del at the end of a line joins the next to it. So
    "Ctrl+y Ctrl+u Ctrl+u Up Up End Del" makes a line twice as long, and
    eight times that make 4096 characters of 16: more than the 3900
    that the editor keeps of a line at a time. data.bin is one line of
    1024 characters to it, with tabs; the lines of a:\\kernel.sys are
    longer than 3900, with tabs too, so the cursor stops short of the
    end of what the editor keeps.
  - The dialog of attributes opens on Read only: Space ticks it. Saving
    such a file in the editor asks first; "nul" as a name saves to a
    device; a name in a directory that is not there is an error with a
    box. An error beeps, and the BIOS of the machine scrolls the screen
    up a line for the beep, because the program has the cursor hidden
    below the last row: the editor's first row stays like that until it
    is drawn again.
  - The floppy A: is write-protected, and the hard disks have 32 MB
    free: the editor fills neither, copies do (see below). Writing to
    A: brings up the box of DOS's critical error, from inside the call
    of DOS, and that is 46 bytes deeper in the stack than a dialog of
    the routine that called: saving to A: in the editor left 4 bytes of
    the stack while that was still assembly and the stack 256 bytes. No
    scenario has it yet; with the stack of 4,096 bytes one can.
  - In the editor Ctrl+Right goes to the next word and Ctrl+t deletes up
    to it; Ctrl+k cuts off the line at the cursor. End in a line of 3900
    characters or more stops on the 3900th, and Del there deletes that
    one, or, in a longer line, joins the next line to those 3900 and
    drops the rest: lines to be joined by "End Del" are 3899 long at the
    most. "Home Ctrl+Right Ctrl+k" makes a line of 4096 characters
    without a space one of 3899. Seventeen such lines joined from the
    last upwards, "Up End Del" for each, are a line of more than 64K. In
    hex Up and Down go by 16 bytes, and F4 puts the cursor into the line
    of the text that its byte is in.
  - The editor has 358,752 bytes for a text on the bench, and the port
    fewer, by what it is larger. Ctrl+u puts back a line that Ctrl+y has
    cut as often as it is pressed, 14 times to a step, until the editor
    beeps: no room. The port gets there some presses before the
    original, so every such step ends where both show the same whatever
    is at the end of the text: Ctrl+Home, the top. A beep scrolls the
    screen, so the step also has to draw all of it again, which Ctrl+Home
    from the end of the text does.
  - More keys of the editor: Ctrl+Left is the word before, Ctrl+w and
    Ctrl+Backspace cut back to it, Ctrl+t cuts on to the next word, and
    Ctrl+u puts back what was last cut, a piece into the line as often
    as it is pressed, a line as a line. Ctrl+q makes the next key a
    character, whatever it is, and shows a quotation mark in the status
    line meanwhile. PgDn goes to the last line of the window first and
    then by pages. F7 asks for a text and Shift+F7 looks for it again;
    the cursor goes behind what is found, and in hex "4c 69" is two
    bytes. "Enter+Esc" in the dialog of F7 has Esc waiting when the
    search begins, which stops it.
  - d:\\vc\\vcview.ext has 23 lines, and the editor shows ten lines of
    help below an extension file or the menu file: a window of 13 rows,
    and so a text of two pages without making one.
  - F4 in a panel that shows the Info edits the file dirinfo there, in a
    window of 36 columns and 11 rows without a status line. "Ctrl+l" and
    then "F9 l i" leave the Info in the left panel, which stays the
    active one, and files in the right. With the Info in both panels a
    row has "Bytes Free" twice, and only the first is left out of the
    comparison.
  - The editor has room for 358,752 bytes of text, and the port for
    some 328,000 by now, less with every batch: a file that a scenario edits
    has to fit the port for long, and one that is to be too large is so
    for both. The original compares the low words of size and room for
    a file of 358,752 to 393,215 bytes only: four times COMMAND.COM and
    a:\\freedos\\bin\\country.sys behind them are 381,338. The files
    of C:\\STAMPS have no bytes behind their sizes: the editor opens
    night.dat as an empty text, and has no room for noon.dat.
  - The floppy of the bench has no SETUP.BAT.
  - The key bar is drawn when the program waits for a key and finds
    none, and only if it is another than the last one drawn: the bar of
    another mode, or of Shift, Ctrl or Alt held down. Every key of a
    step is given to the program when it waits, so the bar is drawn
    between any two keys as it would be between two steps. That was not
    so while the port's keys were pressed by the clock, and
    `editor-full` has "F4" as a step of its own from then.
  - In the help Up, Down, PgUp, PgDn, Home and End move the cursor in
    the index and the text of a page; Left and Right go from button to
    button, Enter presses the one the cursor is on and a button's first
    letter presses it: Help and Cancel under the index, Next, Previous,
    Index and Cancel under a page. Esc and F10 leave. The index has 62
    lines, of which a screen of 25 lines shows 13; its fourth page has
    eight lines, fewer than the window, its fifth twenty. F1 in a dialog
    or a menu shows the page of that: F7, F9. F5 on ".." opens no
    dialog, and Esc at the panels switches them off.
  - The help file, d:\\vc\\vc.hlp, has no line end in it: to the editor
    it is one line, End goes to its 3900th character, in the text of
    the second page, and a letter typed there puts every page behind it
    a byte further on than the index says. The index is read as before
    then, and a page behind the second is an error with a box. Ctrl+y
    there leaves a file with nothing in it, an error at once; "del" at
    the command line leaves none, and F1 does nothing.
  - Alt+F7 asks for a name and, a Tab further, for a text that the
    files are to contain; both fields keep what they had. A name with a
    path, "a:\\*.*", searches from there down, and "*:" for the drive
    searches the drives that are not removable, so not A:. When the
    search is over, "n" is New search, Enter is Go to, which puts the
    cursor of the panel on the file, and Esc or "q" closes the box; Up,
    Down, PgUp, PgDn, Home and End move in the list, which shows 14 of
    its lines. A:\\ has 83 files and directories in all.
  - A search takes keys while it runs: Esc and Ctrl+c stop it, F1 is
    the help, whose window covers all of the box that changes, Left,
    Right, Space and Backspace move in "Stop, Go to, Quit" and a first
    letter answers. The one way to give it a key is with the Enter that
    starts it, as one name: "Enter+Home", "Enter+Right+Left+F1". The
    keys go down together and the search finds them waiting when it
    first looks, before DOS is asked for a file. A key written
    behind that Enter as a name of its own gets to the program when
    the search is over. From the help over
    a search, "c+Esc" is Cancel for the help and Esc for the search,
    and "Ctrl+Esc+c" Esc for the help and Ctrl+c for the search; in the
    dialog Ctrl+m is Enter, so "Ctrl+m+c" stops a search as it begins.
    No key can be given to a search later than that, between two files
    or two parts of one.
  - The floppy is slow: a file of 46 KB takes most of a second to read.
    A:\\FREEDOS\\BIN\\COMMAND.COM, 87,772 bytes, is read in three parts
    by a search for a text; the three *.bat of that directory are
    small.
  - The box of F8 takes no key that was typed before it came up: "F8"
    and "Enter" are steps of their own, and a step "F8 Enter" leaves the
    box open. "Ctrl+a Space Enter" and 'F7 "name" Enter' do work as one
    step. In the boxes about a read-only file, "Delete Skip Cancel", and
    about a directory that is not empty, "Delete All Cancel", "Right
    Enter" is the second answer and "Right Right Enter" the third; in a
    box of two answers "Right Right" is the first again.
  - DOS deletes the files of a directory at one call only up to the
    first that is read-only: the others are then deleted one by one,
    once that one has been answered for with Skip or Delete.
  - Shift+F8 asks for a mask. "nul" is a file that DOS finds and cannot
    delete: the one way to "Can't delete the file" on the bench. A mask
    in a directory that is not there is "Can't find files"; a mask with
    a drive and no directory, "c:*.zzz", says nothing when no file is
    like it.
  - The directory under the cursor of a tree panel is the current one
    of DOS, once the files have followed there: F8 in the tree deletes
    the current directory. The tree leaves TREEINFO.NCD in the root
    when it is read from the disk; a panel lists the file once it is
    read again, Ctrl+r. Made read-only, it cannot be marked as out of
    date when a directory is made: a beep, and the tree stays as it
    was. Ctrl+r in a tree panel reads the tree again; with the file
    read-only it would make the panel one of a kind there is not, 5,
    which no scenario does.
  - A directory that F7 makes has the time of day, and the mini status
    shows it to the minute when the cursor is on it. Late in a scenario
    switch the mini status off first, "F9 o n".
  - On the floppy, "Alt+F1 a", setting an attribute brings up the box
    of DOS's critical error, "Retry Abort", which Esc leaves.
  - Dates are typed as the country writes them: "31.8.99" in Russia.
  - In a dialog End and PgDn go to the buttons and Home and PgUp to the
    first field; Up and Down past the ends of a group go to the field
    above or below, Left and Right to the one beside; Shift+Tab goes
    back a field. Space on the line that is chosen of a group chooses
    the next. A key that means nothing leaves the screen as it is, and
    the cursor, which a golden records. F10 closes a dialog as Esc does,
    but for the dialog of F5, where F10 and Alt+F10 show the tree to
    choose from: Enter there types the path of the directory into the
    line. "F9 r l" with the left panel active is the filter of the right
    one, in the right half of the screen, three Tabs from its buttons.
  - F5 and F6 are in a step of their own in the scenarios, the name
    and Enter in the next. That was needed while keys were pressed by
    the clock; 'F5 "name" Enter' as one step does the same now. An
    answer to a box that the copying brings up may follow in the same
    step too: a key of its own gets to the program when the copying
    has stopped at the box. Only a key written with the Enter that
    starts the copying, "Enter+Esc", is there while it copies, and is
    taken for a break.
  - With the Info next to it a panel's F5 offers the last target again,
    so "F5", "Enter" and an answer repeat a copy; with a panel of files
    next to it, that panel's directory. F6 offers the name of the file
    itself, which does nothing to a file. Do not press Enter on that
    for a directory: it is moved into itself, a directory deeper every
    time, until the path is too long, with a box and a beep for each.
    Shift+F5 and Shift+F6 ask for the source too: its name, Tab, the
    target.
  - "The following file exists" takes Enter to overwrite, "a" for all,
    "p" to append and "s" to skip; "Yes No" takes "Right Enter" for
    no; "Ok Cancel" Enter and Esc; "Abort Continue", of a full disk,
    Enter and "Right Enter"; "Delete Skip Cancel", of a read-only file
    that was moved, Enter, "s" and Esc.
  - Names that lead somewhere: "*." is the name of the source without
    its extension, which may be that of a directory; ".\\" thirty times
    and "src\\" are the 64 characters that a target may have, and name
    C:\\WORK\\SRC; "nul" is a device and "con" one that is refused, as
    target and, with Shift+F5, as source; "nul." is a device too.
    D:\\VC\\VC.HLP ends with the end-of-file character, 1Ah.
  - "F9 l t" makes the left panel the tree, and F6 there asks for a new
    name of the current directory. FreeDOS does not rename that one,
    nor remove it, so the program moves what is in it instead and says
    what it could not do.
  - A disk gets full by copies: COMMAND.COM of A: as two files, each
    appended to the other in turn, seven times, and nine copies of the
    3 MB that makes. The scenario copy-full does it and takes half a
    minute when a build is tested. Keep what a step copies small: 25 MB
    in one step took more than 20 seconds on a machine that other tests
    were using, and a test is as slow as its slowest step.
  - "Ctrl+a", "Down Space Enter" takes the Archive attribute off a
    file, which then has none.
  - Alt+F5 is the map of the memory: Up, Down, Home, End, PgUp and PgDn
    move the cursor, which shows in the colours only; Ins, GrayPlus and
    GrayMinus select programs that can be released, Enter releases
    them, or the one under the cursor, and "Right Enter", Esc and F10
    leave. A program can be released if it was started from here and
    stays in the memory: 'a:\\freedos\\bin\\fdapm' Space "apmdos"
    Enter at the command line is the one such program of the bench. It
    does not stay twice, and the map has eight lines with it, fewer
    than its window. deviations.py leaves out the sizes and the places
    that follow the size of the program.
  - Ctrl+F5 and Ctrl+F6 copy and rename the file under the cursor,
    whatever is selected. Tab makes the Info the active panel, and F5
    there asks for the source too, as Shift+F5 does and as F5 does with
    the panels off, which Esc switches. F5 and F6 on ".." do nothing.
  - In the dialog of F5 the keys go to the field that the cursor is in:
    text typed while it is on the buttons is not taken, and Home goes
    back to the line. Once F10 has put the path of a directory into the
    line, what is typed is added to it: Ctrl+y empties the line first.
    F10 shows the tree of the drive that the line names, "d:", and of
    the current one for a drive that is not there, "z:"; Alt+F10 and
    the button show the current drive's.
  - In the Configuration dialog three Tabs go to "Auto change
    directory". With that off the cursor of a tree panel moves without
    the current directory of DOS following, until Enter: so a directory
    that is not the current one can be renamed there, F6, which FreeDOS
    does also for one above the current one. After F6 or F5 there the
    tree is read again, in the order of the disk, and its cursor is on
    the current directory, not on what was renamed.
  - The floppy drive has a second letter, B:, which nothing on the bench
    offers but which may be typed: Shift+F8, "b:*.*" and two Enters
    bring up "Insert diskette for drive B:", and Esc there stops the
    command. That is the way to a disk that the user will not put in.
    Enter there makes the diskette B:'s: see above.
  - Shift+F8 with "*.*" or a full stop for a name, with a path or
    without, warns first, "Delete Cancel"; a name that no file has says
    "Can't find file". Ctrl+F8 deletes the file under the cursor
    whatever is selected. F8 on a panel that is off, Ctrl+F1 for the
    left one, is Shift+F8.
  - F7 in a tree panel asks for a name of twelve characters and makes
    the directory below the one the cursor is on; a name with a path is
    an error there. F7 with a name that is there already is an error
    too, and so is Shift+F7 with such a path; a path that can be made
    leaves the panel as it was.
  - Ctrl+F4 and a new label: FreeDOS reports no error and the disk keeps
    the label it had, WORK on C:. The name of a directory of the root,
    "work", cannot be made: "Could not set the volume label", with a
    beep.
  - Alt+F7 with "*:" and a name that has no path searches C: and D:
    from their roots. Keep the program's own file out of what is found
    on D:, by the mask: "*:*.hlp". An empty name closes the box. Two
    keys written as one name, "Enter+q", go down together: the second
    is waiting when the search first looks for a key, before it has
    found a file, so "Enter+q" is Quit and "Enter+Enter" Stop with no
    file found, the same in every run. That is how to answer a search
    that runs without pressing a key at no defined moment.
  - Go to in the list of Alt+F7, Enter, on a line of a directory, which
    Up gets to, shows that directory; with the tree in the current
    panel the other panel gets it. Go to a file of A: from C: changes
    the drive of the panel.
  - F2 shows the menu of the current directory, its file vc.mnu, or
    else the main one, of D:\\VC, which Shift+F2 shows at once: F1 is
    its line "Archives...", another menu, F2 "Format floppy disk..." and
    F4 "Edit file", whose command, ne, is not there. A key of a line
    comes before what the key means otherwise: F1 is the help and F10
    leaves only in a menu that has no such line. A menu file is made in
    the editor, Shift+F4 and its name: for vc.mnu, vc.ext and vcview.ext
    the editor opens at once, for another name it asks first, and Enter
    is "New-file". A line of the menu begins at the margin, "a:" or
    "f3:" and what is shown; its commands begin with Tab, and a comment
    is one only at the margin: behind a Tab it is run. A command that is
    the name of a menu file shows that menu.
  - The list file that "!@" stands for is D:\\VC\\vc.lst: the bench sets
    no TEMP. Run with the cursor on "..", or in the tree, it beeps. F3 on
    a file runs the command that a vcview.ext of the directory has for
    its kind, with the name as it was given, and Shift+F3 does for a
    name with a path; Alt+F3 is always the program's own viewer.
    "F9 o a" switches Auto menus: the menu comes back after a command.
  - Alt with a letter or a digit begins the speed search of a panel of
    files or of the tree; letters go on with it, Backspace takes one
    back, Ctrl+Enter goes to the next name, Esc ends it, and any other
    key ends it and is acted on. Ctrl+[ and Ctrl+] put the paths of the
    left and the right panel into the command line, and Ctrl+i the
    names of the selected files, or of the one under the cursor; Esc
    empties that,
    and with nothing in it switches the panels off and on, unless "F9 o
    o", "Down Space" has it otherwise. The command line takes 123
    characters; a beep there scrolls nothing, the cursor being on the
    screen.
  - Alt+F8 is the history: Enter runs the command under the cursor,
    Ctrl+Enter only puts it into the command line, a letter goes to the
    next command that begins with it. The box of Alt+F1 and Alt+F2 opens
    with the cursor on the drive that the panel has, so from C: "Right
    Enter" is D:. Do not have the Info show D:, as Ctrl+l does next to a
    panel on D: its free bytes follow the size of the program.
  - A:\\FREEDOS\\BIN has 44 entries: with "F9 o f" a panel shows 30 of
    them in brief and ten in full, so Left, Right, PgUp and PgDn move
    the list there. Seven directories made in C:\\WORK give a tree of
    thirteen lines, two more than the half-high panel shows. PgDn in a
    list of less than two panels moves the list to its end and the
    cursor no further than that; the next PgDn goes to the last entry.
  - With the right panel off, Ctrl+p or Ctrl+F2, what DOS has shown is
    behind it, and at the start that is the end of the program's
    banner, "ware" for the original and "are" for the port:
    deviations.py knows the banner only where a line begins with it. Run
    a command first, "ver": that scrolls the banner away.
  - Ctrl+z puts the tree into the other panel and Ctrl+l the Info; each
    pressed again brings back what the panel showed before, which is
    the other of the two once both have been there, and never files:
    "F9 l b", Brief in the menu of the panel, makes it one of files, in
    C:\\. With the Info it would say how much is free on D:, which
    depends on the size of the program: keep the Info away from D:.
    The tree of D: has two lines and is kept in no file, so Ctrl+r in
    it finds no file to mark and reads nothing.
  - Ctrl+m selects again what was selected before a command went
    through the selected files: "Ins Ins", "Ctrl+a", "Enter" is such a
    command, and sets nothing. In the Files menu "g", "n", "i" and "s"
    are the four commands of the selection; with the tree current they
    are switched off.
  - With the key bar off, Ctrl+b, a command leaves the prompt on the
    last line of the screen, and Ctrl+b then has everything move up a
    line. With the panels off Up and Down are the commands before and
    after, and Home, End, Left, Right and GrayStar keys of the command
    line.
  - On the menu bar Left and Right go round its five menus, a letter
    opens one, L, F, C, O or R, and Down or Enter the one that is
    marked; in a menu Up and Down go round its items, over the lines
    between them and the items that are dark, Home and PgUp go to the
    first and End and PgDn to the last, the capital letter of an item
    chooses it, and so does the key that it names, Ctrl+o for Panels
    on/off. Esc and F10 leave, but F10 in the Files menu is Quit.
    Shift+F10 opens the menu of the last item chosen, on that item, or
    on the next that is not dark. F1 is the help of the item, which Esc
    leaves for the menu. Ctrl+z makes the other panel the tree and
    Ctrl+l the Info, or what it was before; Ctrl+p switches the other
    panel off. Rename chosen from the Files menu with the panels off
    asks for the file by name and offers the last target: after F6 it
    may offer the name under the cursor instead, as the registers
    happen to be.
  - A panel on the left with none on the right shows, in the first row
    of the right half, the end of the banner that the program printed
    when it started: "are", and "ware" under the original, whose
    banner names another edition. deviations.py knows the banner by
    its beginning, so that row is held against the wrong letters, and
    the port test fails before anything is translated. Switch the left
    panel off instead, "Tab Ctrl+p": the beginning of the banner is
    known.
  - An item of the Files menu chosen with Shift or Ctrl held down,
    "Shift+Enter" or "Ctrl+Enter" on Copy, is the command that F5 has
    with that key. The program asks for Shift and Ctrl after it has
    taken the Enter, and the harness holds them for 20 ms for the port:
    it worked when tried, and is in no scenario.
  - Ctrl+c, "F9 c c", compares the directories of two panels of files
    and selects in each what the other has not or has older, by date
    and time; with nothing to select a box says that they are the
    same, and next to the Info the item is dark and the key does
    nothing. Files are listed in small letters and directories in
    capitals, and the names are compared as they are listed: a file
    and a directory are of one name only where it has no letter, a
    directory "1", F7, and a file "1", F5 and "src\\1". Ctrl+a with five
    Tabs and a time, or four and a date, makes a file newer than its
    copy.
  - "F9 c m" is Menu file edit, and "F9 o x", "F9 o v" and "F9 o e"
    are the three extension files: a box asks which, Enter for the
    main one, in D:\\VC, "Right Enter" for that of the current
    directory, which is a new file if there is none, and Esc or the
    third button for neither. The editor opens, and Esc leaves it.
  - Shift+F9, "F9 o s", asks whether to save the setup, and Enter
    writes D:\\VC\\VC.INI. The command 'a:\\freedos\\bin\\attrib +r
    d:\\vc\\vc.ini' makes that file read-only without a panel showing
    its directory: saving then ends in a box, with a beep that scrolls
    the screen.
  - With the panels off, Ctrl+o, Alt+F9 leaves the command line on its
    row in 50 lines, and Alt+F9 again finds it too far down for 25 and
    puts it on the second row. With panels it is below them, and
    moves up with what is shown.
  - The floppy drive under its second letter: 'Shift+F8 "b:*.zzz"
    Enter' brings up "Insert diskette for drive B:", and Enter there
    has DOS take the drive for B: from then on. Whatever goes to A:
    afterwards asks for its diskette first, and Esc refuses: that is
    how a command finds a drive that it cannot go to. A panel that
    shows A: is read again at once, so the question comes up as soon
    as the first is answered; Esc, and Esc again for the box that
    says the disk cannot be read, leave the panel empty. Enter at any
    such box makes the drive A: again.
  - A directory that a panel has, and that is gone when the panel is
    made one of files again: go into it, Ctrl+l for files on the
    right, "F9 l i" for the Info on the left, which leaves DOS in the
    other panel's directory, and remove it by a command, "rd" Space
    "work\\docs". Do not have the Info in both panels for it: a row
    then has "Bytes Free" twice, and the second number is compared.
  - Drive... of a panel's menu, "F9 l d", is the box of Alt+F1.
  - A key written with the one before it, "Right+Right", is lost where
    the program empties the keyboard buffer after a key, as the editor
    does while "Clear keyboard buffer" is on: the cursor goes one
    column. Behind the Enter that starts a copy, "Enter+a", a key that
    stops nothing is left where it is by the copy, which looks for Esc,
    Ctrl+c and F10 before every piece, and is typed into the command
    line afterwards; "Enter+F10" stops the copy before it has read
    anything, and no target is left.
  - "Click@40,15" in the Configuration, "F9 o c", ticks Left-handed
    mouse: from then on the right button puts the cursor on a file and
    the left one selects.
  - Enter on run.bat, "End Up" in C:\\WORK, runs it. "Shift+Enter" as
    one name has Shift held when the program asks for it. Ctrl+e and
    Ctrl+x go through the history, and Ctrl+Enter behind a letter looks
    there for a command that begins with what the line has. Five
    commands of 120 characters fill the history: "rem" and letters.
  - With "Quick execute commands" on, "F9 o o", nothing is run at all:
    the shell of the bench does not serve the interrupt that the
    program runs a command by then.
  - A tree keeps a directory that DOS has removed behind the program's
    back: with Auto change directory off, the cursor of the tree on
    DOCS and "rd docs" typed and run, Enter in the tree finds no such
    directory. With it on, the cursor is on the current directory, and
    DOS does not remove that.
  - The floppy by its other letter: Shift+F3, a file of B: and Enter
    ask for the disk of B:, Enter puts it in, and Esc leaves the viewer
    with the panels as they were. From then on whatever goes to A: asks
    for its disk, and Esc there refuses: Enter on a directory of a
    panel on A:\\, Ctrl+PgDn, Ctrl+\\. Do not get to B: by Shift+F8: that
    has both panels read again, and a disk refused there is "Can't
    read the disk in drive A:".
  - More files than a panel keeps while a command runs, 256: F7 and a
    name, Shift+F5 with "a:\\freedos\\bin\\v*.com" and the directory,
    GrayPlus with "*.com", and then "Ctrl+m F5", "*.0" and Enter, with
    another extension each time: Ctrl+m selects the same files again,
    and a star in the target keeps their names. A:\\FREEDOS\\BIN has no
    two programs of one name but fdisk.exe and fdisk.ini.
  - What the bench puts on its floppy may change with the harness:
    driver.com in A:\\FREEDOS\\BIN is the driver of the tests, and
    a:\\fdauto.bat starts it. Do not copy or view either, and keep the
    Info away from A:, which says what is free there. The programs of
    DOS do not change: "v*.com" is eighteen small ones.
  - A line of vcview.ext that is nothing but the name of a program
    with its extension has F3 run that program on the file; of
    vcedit.ext, Alt+F4, since the setup has the editor be the
    program's own. a:\\freedos\\bin\\vecho.com is one that ends at once.
    A name with a star, "a:\\freedos\\bin\\v*.com", is found and cannot
    be run; a file that begins with "MZ" and 30 more characters is a
    program that DOS has no memory for. A directory named vcview.ext
    cannot be opened, and F3 does nothing then; with the main file
    gone, "del d:\\vc\\vcview.ext", and none in the directory, F3 is
    the program's own viewer.
  - A program of a few bytes is typed in the editor, in hex: Shift+F4,
    its name and two Enters, as many characters as it has bytes, F4,
    Ctrl+Home, and the bytes as hex digits, which take the place of the
    characters; F2 and Esc. Home in hex goes to the start of the row of
    16 bytes, not of the file: a program of 17 bytes typed behind Home
    was 16 letters with the program behind them, which ran as code and
    went astray. Left and Right move by a digit, two to a byte, and Up
    and Down by a row: 27 times Down and 20 times Right are byte 1BAh.
    start-screen has three such programs, for what the bench has no
    command for: "m" and a digit sets that video mode, "m 1" 40 columns,
    "m 6" graphics, "m 7" monochrome and "m 3" the mode the program
    wants; "c" gives the cursor a shape and puts it below the screen;
    and k.com of start-memory stays in the memory with all of it but a
    number of paragraphs. A:\\FREEDOS\\BIN has vcursor, which is on the
    path: "vcursor" Space "hide".
  - A batch file is made the same way, without F4: its lines, F2, Esc.
    What "set" does in it holds for the commands that follow in it, so
    "set vc=c:\\work" and "d:\\vc\\vc" start a second copy that looks for
    its files in C:\\WORK, and "set comspec=" one that has no COMSPEC=.
    A second copy that finds no setup file there, or one that is not
    correct, starts with the defaults: the right panel only, on the
    current directory, with the mini status, the clock off. It saves
    its setup by itself, into C:\\WORK as it is left: a file of the
    defaults, which the disks at the end of start-setup have. Shift+F5
    with "d:\\vc\\vc.ini", Tab, "vc.ini", Enter, and Enter to
    overwrite, puts the setup of the bench there instead.
  - The setup file is 511 bytes, the last two their sum. A byte changed
    in the editor makes it "not correct"; another byte changed the
    other way makes the sum right again: 1BAh is Snow mode, 2, which no
    menu sets, and 1CAh the first of the reserved bytes. See
    start-setup. That is within the rule that a scenario sets up what
    it needs through the program: the rule is against files prepared
    outside and handed to a scenario, and this one is made inside it,
    in the program's own editor. Nothing else makes a setup file that
    is damaged, or that has a mode no menu sets.
  - With the screen in another mode than text of 80 columns when a
    command ends, the program asks for Enter, waiting in the BIOS, and
    the step ends there: Enter is the next step. The test reads the
    memory of the colour text screen whatever the mode is: 40 columns
    are seen as half as many rows of 80, graphics as characters that
    mean nothing, and in mode 7, monochrome, nothing of what the
    program draws is seen, only the cursor and its shape.
  - While the cursor is below the last row, every character that the
    BIOS writes as a teletype scrolls the screen a line and takes the
    cursor up a row. DOS writes six or more when a command ends, so a
    cursor that a command leaves on row 30 is back on the screen when
    the program looks, and one on row 255 is not.
  - The memory that a second copy has for its data can be made too
    little: its command, k.com, stays in the memory and leaves a number
    of paragraphs free above itself. The copy then loads its code
    again, into the gap of 72 KB that COMMAND.COM leaves below k.com,
    and asks for its data in what is free above: with 66 to 83 KB free
    the original gets its code back and has too little for the data,
    which wants 84; start-memory leaves 75. The port, which is larger,
    does not get as far as that and says the same, "Program too big to
    fit in memory". The copy ends and releases k.com.
Every scenario gets a machine of its own, in the state the bench was saved
in: the program as its distribution configures it but for the clock, which
is off, started in C:\\WORK. A scenario that wants something else arranges
it with its first steps, the way a user would. It may end anywhere, with
the program running or not.

The machine's clock is counted in the instructions that the machine
executes, 256 nanoseconds for each (see CLOCK_SHIFT in dos.py), and
nothing of the host's time gets into it: a scenario comes to each of its
steps at the same hundredth of a second of the time of day whenever it is
run, on whichever bench and however busy the host is. But the port takes
more instructions to do a thing than the original, so the two do not
agree on the time for long. Nothing on the bench shows the time of day, so
that does not matter. A scenario that does show it, by switching the
clock on or by listing a file it has made, has to be done with that
before the machine's first minute is over. The original shows its first
screen 9.6 seconds past noon and the port 9.9; `copy`, of 68 steps, ends
26.9 seconds past noon in the original and 28.1 in the port, and
`delete`, of 63, 25.8 and 28.5: the port has taken up to a sixth longer.
"""
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import coverage
import driver
import fat
import setups
from dos import MachineError

# How long the driver may write nothing before the program counts as
# stuck, in seconds: the longest a step may take. It is long on purpose:
# it is there for a program that hangs, and a machine that many tests
# share must not look like one.
STEP_TIME = 300


@dataclass(frozen=True)
class Scenario:
    description: str
    steps: tuple = ()
    country: str = "us"           # see setups.COUNTRIES


@dataclass
class Run:
    """What a scenario showed and left behind."""
    screens: list                 # after the start and after each step
    disks: dict                   # drive letter -> fat.read_tree() of it
    # Offset and length in the program file of every instruction that ran,
    # if the run was traced.
    executed: dict = field(default_factory=dict)
    # The fewest bytes of its stack that the program had never used, of
    # all the steps that ended with it waiting for a key; None if its stack
    # was not looked at. And the segment its stack was in.
    stack_left: int = None
    stack_segment: int = None


# From C:\WORK into C:\STAMPS, whose files have sizes and time stamps of
# every kind (see setups.py), with the left panel showing them: Full in its
# menu.
_STAMPS = (
    "Ctrl+PgUp",
    "Down Down Enter",
    "F9",
    "Enter",                              # the menu of the left panel
    "Down Enter",                         # Full
)

_STAMPS_STEPS = _STAMPS + (
    "Down Down Down",                     # the status line of a file
    "Ctrl+a",                             # its date and time in full
    "Esc",
    "Down",                               # a date and a time before ten
    "Ctrl+a",
    "Esc",
    "Down Down Down",                     # the last date DOS can express
    "Ctrl+a",
    "Esc",
    "Ins Ins",                            # the size of the selected files
    "Alt+F6",                             # the sizes of directories
    "Alt+F7",                             # the file search and its list
    "Enter",
    "Esc",
)

_CLOCK_STEPS = (
    "F9 o",                               # the Options menu: not ticked
    "l",                                  # the clock, over the panels
    "Ctrl+o",                             # over what DOS shows
    "Ctrl+o",
    "F9 o",                               # on the menu bar, and ticked
    "l",                                  # off again
)

# Files named by hand: with spaces around the name, by paths too long for
# where they are shown, and with a letter that is neither capital nor
# small.
_NAMES_STEPS = (
    "Shift+F3",                           # a file to view
    'Space Space "c:hello.txt" Space',    # on the current drive
    "Enter",                              # C: in capitals, the name small
    "Esc",
    "Shift+F3",                           # by a long path
    '"c:\\work\\..\\work\\..\\work\\..\\work\\readme.txt"',
    "Enter",                              # shortened behind C:\
    "Esc",
    "Shift+F3",                           # and by one from here
    '"..\\work\\..\\work\\..\\work\\..\\work\\hello.txt"',
    "Enter",                              # shortened from its start
    "Esc",
    "Down Down Down",                     # onto a file
    "F5",                                 # a copy of it
    '"under_sc.tx"',                      # an underscore, a short extension
    "Enter",                              # the panel lists it
)

# The path over the right panel makes room for the clock: it moves to the
# left by half of what it has beyond 28 characters, or 30 where the clock
# has 24 hours and is a character wider, and no further than a path of 32
# or 33 would. The path over the left panel stays, and so does the right
# one when the menu bar is above it.
_CLOCK_PATH_STEPS = (
    "Ctrl+PgUp",
    "Down Enter",                         # the left panel in C:\\STAMPS
    "F9 l l",
    '"????????.???"',                     # a mask, and only programs:
    "Tab Tab Space",                      # both are shown with the path,
    "Enter",                              # which is 30 characters now
    "Ctrl+l",                             # the Info panel off: two panels
    "Tab",
    "Down Enter",                         # the right one in C:\\STAMPS
    "Down Enter",                         # and in its directory SUB
    "F9 o l",                             # the clock: this path is short
    "F9 r l",
    '"?????.txt"',
    "Tab Tab Space",
    "Enter",                              # longer: it moves over
    "F9 r l",
    '"????????.???"',                     # longer than there is room for
    "Enter",
    "F9 o Enter",
    "Tab Tab Tab Tab",
    "Space",                              # the menu bar always visible
    "Enter",                              # and the clock in it
)

# The diskette made that of B:. The floppy drive is A: and B: both, and
# DOS has it be one of them at a time: a file of B: viewed has the program
# ask for the diskette of B:, and from then on A: is the drive whose disk
# is not there.
_DISK_B = (
    "Shift+F3",
    '"b:\\fdauto.bat"',
    "Enter",                              # Insert diskette for drive B:
    "Enter",                              # Ok: the file is shown
    "Esc",
)

# The time of day set by DOS's command, and the clock switched on: the
# scenario shows it within the minute that begins then.
_CLOCK_EARLY_STEPS = (
    '"time" Space "9:59"',
    "Enter",
)

# The pointer of the mouse taken across the screen and back 64 times: three
# seconds or more of the machine's time, which is counted in what the
# machine executes. A click on the line that the click before was on comes
# too late then to be the second click of two.
_A_WHILE = " ".join(["Move@79,24"] * 64)

SCENARIOS = {
    "start": Scenario(
        "The program as it starts"),

    "panels": Scenario(
        "The file panel in its other modes, the status line and the "
        "drive menu",
        steps=(
            "Down Down Down",             # onto a file: the status line
            "Down Down Down",             # onto the 1 KB file
            "End",
            "Ctrl+l",                     # the Info panel off: two panels
            "Alt+F1",                     # the drive menu of the left panel
            "Esc",
            "Tab",
            "Ctrl+F1",                    # the left panel off and on again
            "Ctrl+F1",
            "Ctrl+l",                     # the Info panel on the left
            "Ctrl+l",
            # Two keys at once: the second is waiting when the program
            # comes to ask for the next key, which only happens like this.
            "Up+Up",
        )),

    "help": Scenario(
        "The help: its index and pages, whose text is bold, underlined "
        "and reversed in places",
        steps=(
            "F1",                         # the index
            "Down Enter",                 # the keyboard reference
            "PgDn",
            "Enter",                      # the next page
            "Esc",
        )),

    "help-keys": Scenario(
        "The help by all its keys: the index and pages scrolled by lines "
        "and by pages, from page to page and back to the index, for a "
        "dialog and a menu, and on a screen of 50 lines",
        steps=(
            "F1",                         # the index
            "F1",                         # in the help: nothing
            '"5"',                        # a key that means nothing
            "End",                        # the last pages of 62
            "PgUp",                       # the cursor in the last line
            "Down",                       # scrolled by a line
            "Home",
            "Home",                       # where it is already
            "Down Down Down Enter",       # a page of eight lines
            "Down",                       # nothing more to show
            "Up",
            "Enter",                      # Next: a page of twenty lines
            "Down",                       # scrolled by a line
            "Up",                         # and back
            "PgDn",
            "PgUp",
            "End",
            "Home",
            "Right Enter",                # Previous
            '"i"',                        # the index, by its letter: the
                                          # cursor on that page
            "Home Enter",                 # the first page
            "Right Enter",                # Previous: the last one
            "Enter",                      # and the one before it
            "Left Enter",                 # Next
            "Enter",                      # Next: the first one again
            "Right Right Enter",          # the index, by its button
            "Right Enter",                # Cancel
            "F7",
            "F1",                         # the page of a dialog
            '"c"',                        # Cancel, by its letter
            "Esc",
            "F9",
            "F1",                         # the page of the menus
            "Esc",
            "Esc",
            "Alt+F9",                     # 50 lines
            "F1",                         # an index of 38 lines
            "End",
            "F10",                        # leaves it too
            "Alt+F9",
        )),

    "help-broken": Scenario(
        "A help file that cannot be read: a page that is not where the "
        "index says, a file that is empty and one that is not there",
        steps=(
            "Shift+F4",                   # the help file in the editor:
            '"d:\\vc\\vc.hlp"',           # one line of 68836 characters
            "Enter",
            "End",                        # the 3900th of them, in the text
            '"x"',                        # of the second page: every page
            "F2",                         # behind it is a byte further on
            "Esc",
            "F1",                         # the index is as it was
            "End Enter",                  # its last page is not: an error
            "Enter",                      # which ends the help
            "F7",
            "F1",                         # nor is the page of a dialog
            "Enter",
            "Esc",
            "Shift+F4",
            '"d:\\vc\\vc.hlp"',
            "Enter",
            "Ctrl+y",                     # the one line cut: nothing left
            "F2",
            "Esc",
            "F1",                         # an error
            "Enter",
            '"del" Space "d:\\vc\\vc.hlp"',
            "Enter",
            "F1",                         # no file: nothing
        )),

    "configuration": Scenario(
        "The Configuration dialog, with its frames inside the frame",
        steps=(
            "F9 o Enter",
            "Esc",
        )),

    "hex": Scenario(
        "A file in hex, and bytes looked for in it by their hex digits",
        steps=(
            "Down Down Down Down Down Down",   # onto the file of all bytes
            "F3",
            "F4",                         # in hex
            "F7",
            '"4a" Space "4B"',            # two bytes, digits small and big
            "Enter",
            "F7",
            '"4:"',                       # no digits: looked for as text,
            "Enter",                      # which is not there
            "Esc",
            "F7",
            '"g1"',                       # nor is this, behind F
            "Enter",
            "Esc",
            "F7",
            '"/1"',                       # nor this, before 0
            "Enter",
            "Esc",
            "Esc",
        )),

    "attributes": Scenario(
        "The date and the time of a file, set by hand",
        steps=(
            "Down Down Down",             # onto a file
            "Ctrl+a",
            "Tab Tab Tab Tab",            # past the attributes, to the date
            '"1-2-99"',
            "Tab",
            '"3:04:05"',
            "Enter",                      # set, the seconds made even
            "Ctrl+a",
            "Tab Tab Tab Tab",
            # A year too large for a number is refused, be it by the
            # tenfold or by the digit added to it. Cut to 16 bits these
            # two would be 1999 and 1.
            '"1-2-67535"',
            "Enter",
            "Esc",
            "Esc",
            "Ctrl+a",
            "Tab Tab Tab Tab",
            '"1-2-65537"',
            "Enter",
            "Esc",
            "Esc",
        )),

    "select": Scenario(
        "Files selected and unselected by a mask",
        steps=(
            "GrayPlus",
            '"*.txt"',
            "Enter",
            "GrayMinus",
            '"h?llo.*"',
            "Enter",
        )),

    "sort": Scenario(
        "The files of a panel in each order: by name, extension, time and "
        "size, with system files first, and unsorted again",
        steps=(
            "Down Down Down Down",        # onto hello.txt
            "F5",                         # a copy with the same name and
            '"hello.doc"',                # another extension
            "Enter",
            "Down",                       # onto notes.doc
            "Ctrl+a",
            "Tab Tab Tab Space",          # a system file,
            "Tab Tab",
            '"3:04:05"',                  # of the same day at another time
            "Enter",
            "Down Down",                  # onto run.bat
            "Ctrl+a",
            "Tab Tab Tab Space",          # another system file
            "Enter",
            "F9 l n",                     # by name
            "F9 l x",                     # by extension
            "F9 l m",                     # by time
            "F9 l s",                     # by size
            "Alt+F1 a",                   # a disk with files before a
                                          # directory, of different days
            "F9 l u",                     # unsorted: as they are on disk
        )),

    "filter": Scenario(
        "The files of a panel chosen: without the hidden ones, by a mask, "
        "and only programs",
        steps=(
            "Down Down Down Down",        # onto hello.txt
            "F5",                         # copies of it that are programs
            '"hello.com"',
            "Enter",
            "F5",
            '"hello.exe"',
            "Enter",
            "F5",                         # and one without an extension
            '"noext"',
            "Enter",
            "Ctrl+h",                     # hidden files off: none is hidden
            "Up",                         # onto readme.txt
            "Ctrl+a",
            "Tab Tab Space",              # hidden: it goes, and the cursor
            "Enter",                      # to the file behind it
            "End",                        # onto the last file
            "Ctrl+a",
            "Tab Tab Space",              # hidden too: the cursor goes to
            "Enter",                      # the file before it
            "Ctrl+h",                     # hidden files on again
            "Home",
            "Down Down Down Down Down Ins",   # notes.doc selected
            "F9 l l",
            '"*.t?t"',                    # by a mask
            "Enter",
            "F9 l l",
            "Ctrl+y",                     # no mask,
            "Tab Tab Space",              # only programs
            "Enter",
            "F9 l l",
            "Tab Tab Space",              # all files again: one that was
            "Enter",                      # left out is no longer selected
        )),

    "scroll": Scenario(
        "Panels of other sizes, and a list of files longer than its panel",
        steps=(
            "F9 o Enter",
            "Tab Tab Tab Tab",
            "Space",                      # the menu bar always visible
            "Enter",
            "Alt+F9",                     # 50 lines, of which the first 25
                                          # are recorded
            "Alt+F9",
            "F9 o f",                     # panels half the screen high
            "F9 l f",                     # and of one column: ten files
            "Down Down Down Down",        # onto hello.txt
            "F5",
            '"hello.doc"',
            "Enter",
            "F5",
            '"hello.bak"',
            "Enter",                      # eleven files now
            "End",
            "Up Up Up Up Up Up Up Up Up Up",              # scrolled back
            "Down Down Down Down Down Down Down Down Down Down",
            "Home",
            "Down Ins",                   # a directory selected
            "Down Down Ins",              # and a file
            "F8",                         # counted
            "Esc",
        )),

    "cursor": Scenario(
        "The cursor moved through a list longer than its panel, by lines, "
        "columns and pages, in a panel of files and in the tree",
        steps=(
            "F9 o f",                     # panels half the screen high
            "Alt+F1 a",
            "Down Down Down Enter",       # A:\\FREEDOS
            "Down Enter",                 # and its BIN: 44 entries, in
                                          # three columns of ten
            "Right",                      # a column on
            "Right",
            "Right",                      # from the last column the list
                                          # moves by one
            "Right",                      # no three columns are left of
                                          # it: the end
            "Left",
            "Left",
            "Left",                       # from the first column the list
                                          # moves back
            "Left",                       # less than a column is left
                                          # before it: the start
            "Down Down Down",
            "Left",                       # in the first column of the
                                          # list: its first entry
            "PgDn",                       # as far as the list can move
            "PgDn",                       # the end
            "PgUp",
            "PgUp",                       # the start
            "F9 l f",                     # Full: one column
            "End",
            "PgUp",                       # a panel back,
            "PgUp PgUp PgUp",             # and to where the list begins
            "PgUp",
            "PgDn",
            "PgDn PgDn PgDn PgDn",
            "Home",
            "Ins Ins",                    # nothing above is selected
            "Alt+F1 c",
            "F9 l b",                     # Brief, and the mini status
            "F9 o n",                     # off: no time of day is shown
            *(f'F7 "{letter}" Enter'      # seven directories more: the
              for letter in "abcdefg"),   # tree has thirteen
            "PgDn",                       # a list shorter than its panel
            "PgUp",
            "F9 l t",                     # the tree, eleven lines of it
            "Down Down Down Down Down",
            "Down Down Down Down Down",   # the tree moves a line up,
            "Down",                       # and another
            "Up Up Up Up Up Up",
            "Up Up Up Up Up",             # and a line down
            "PgDn",
            "PgUp",
            "Right",                      # not a key of the tree: it
                                          # goes to the command line
            "Home",
        )),

    "panel-keys": Scenario(
        "Files selected by masks and what was selected restored; the "
        "panels switched off and on, swapped, of another kind, and the "
        "key bar; the keys of the panels with the panels off",
        steps=(
            "Down Down Down Ins Ins",     # two files selected
            "GrayStar",                   # the selection turned round,
            '"*.txt"',                    # for a mask
            "Enter",
            "GrayPlus",
            '"*.zzz"',
            "Enter",                      # no file like that
            "Enter",
            "Ins Ins",                    # two files again
            "Ctrl+a",                     # their attributes: none is set,
            "Enter",                      # and they are selected no more
            "Ctrl+m",                     # but they are again
            "F9 f i",                     # turned round, from the menu:
            "Esc",                        # not after all
            "Ctrl+b",                     # the key bar off
            "Ctrl+o",                     # the panels off
            '"ver"',
            "Enter",                      # the prompt is on the last line
            "Up",                         # the command before
            "Home",                       # the cursor in the line
            "Right",
            "End",
            "Left",
            "GrayStar",                   # a character of the line
            "Down",                       # no command after it
            "PgDn",                       # not without the panels,
            "Ctrl+l",                     # nor this,
            "Ctrl+u",
            "Ctrl+p",
            "Ctrl+b",                     # the key bar: the prompt goes up
            "Ctrl+o",                     # the panels, read again
            "Ctrl+p",                     # the other panel off: the banner
                                          # of the program has gone from
                                          # what is behind it
            "Tab",                        # nowhere to go
            "Ctrl+p",                     # and on
            "Ctrl+F2",                    # the right panel off
            "Ctrl+F2",
            "Ctrl+u",                     # the panels change places
            "Ctrl+z",                     # the tree for the Info,
            "Ctrl+l",                     # the Info for the tree,
            "Ctrl+l",                     # the tree again, which it was
            "Ctrl+z",                     # and the Info, which it was
            "F9 l b",                     # the Info a panel of files
            "Tab",
            "Enter",                      # both in C:\\WORK
            "Tab",                        # there is nowhere to change to
            "Alt+F2 d",                   # the right panel on D:, whose
            "Ctrl+z",                     # tree is kept in no file
            "Tab",
            "Ctrl+r",                     # so there is none to mark as
                                          # out of date
        )),

    "emptied": Scenario(
        "A disk with nothing left on it to list",
        steps=(
            "Ctrl+PgUp",                  # the root, with two directories
            "Ins Ins",
            "F8",
            "Enter",
            "Enter",                      # both of them
            "Enter",                      # though they are not empty
            "Enter",
        )),

    "editor": Scenario(
        "The editor, as it opens a file",
        steps=(
            "Down Down Down",             # onto a file
            "F4",
            "Esc",
        )),

    "palettes": Scenario(
        "The other two palettes, black and white and laptop, in which a "
        "box throws another shadow",
        steps=(
            "F9 o Enter",
            "Up Space",                   # black and white
            "Enter",
            "F10",                        # a box and its shadow
            "Esc",
            "F9 o Enter",
            "Down Down Space",            # laptop
            "Enter",
            "F10",
            "Esc",
        )),

    "tree": Scenario(
        "The tree of directories, which the other panel follows a moment "
        "after the cursor has come to rest",
        steps=(
            "Ctrl+l",                     # the Info panel off: two panels
            "F9 l t",                     # the tree in the left one
            "Down",                       # the right one follows
            "Down Down",                  # only to where the cursor stops
            "End",
        )),

    "command": Scenario(
        "DOS commands: the program gives up most of its memory while one "
        "runs, and loads itself again afterwards",
        steps=(
            '"ver"',
            "Enter",
            "Ctrl+o",                     # what the command printed
            "Ctrl+o",
            '"mem"',
            "Enter",
            "Ctrl+o",
            "Ctrl+o",
            "Ctrl+PgUp",                  # and the program still works
        )),

    "command-edit": Scenario(
        "The command line typed into and edited, longer than the screen "
        "is wide and until it is full; the key bar off; and a file run by "
        "what the extension file says for its kind",
        steps=(
            "Down Down Down",             # onto readme.txt
            " ".join(["Ctrl+Enter"] * 7), # its name seven times: the line
                                          # is too long and moves to the left
            " ".join(["Ctrl+Left"] * 7),  # back by words, ^A
            " ".join(["Ctrl+Left"] * 6),  # until the line moves back
            "Ctrl+Right Ctrl+Right",      # on by words, ^F at the panels
                                          # is the filter
            "Ctrl+Home",
            "Ctrl+d Ctrl+d Ctrl+d",       # right
            "Ctrl+t",                     # the rest of the word goes
            "Ctrl+End",
            "Ctrl+w",                     # and the word before the cursor
            "Ctrl+s Ctrl+s Ctrl+g",       # left, and a character deleted
            "Ctrl+q Ctrl+a",              # the next key as it is: ^A
            "Ctrl+q Left",                # which a key without a character
                                          # is not
            "Ctrl+End",
            " ".join(["Ctrl+Enter"] * 5), # until the line is full: a beep
            "x",                          # and another
            "Ctrl+y",
            "Ctrl+b",                     # the key bar off
            "Ctrl+b",                     # and on
            "Down Down",
            "Enter",                      # notes.doc: no command for doc
            "F5",
            '"noext" Enter',              # a file without an extension
            "F9 o n",                     # the mini status off: it would
                                          # show the time of the file made
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",
            '"vc.ext"',
            "Enter",                      # a command for no extension
            '":" Tab "echo" Space "!.!" Space "x.y"',
            "Ctrl+Left",                  # in the editor: back a word
            "Ctrl+Backspace",             # and the word before deleted
            "F2",
            "Esc",
            "End Up",
            "Enter",                      # noext is run by it
            "Ctrl+o",                     # what the command printed
            "Ctrl+o",
        )),

    "lost": Scenario(
        "The program file gone when the program wants to load itself "
        "again after a command",
        steps=(
            '"del" Space "d:\\vc\\vc.*"',  # the program, with what it reads
            "Enter",                      # it asks to try again
            "Space",                      # which is not what it asked for
            "Enter",                      # it fails again
            "Esc",                        # and gives up: the DOS prompt
        )),

    "quit": Scenario(
        "Leaving the program, declined once",
        steps=(
            "F10",
            "Esc",
            "F10",
            "Enter",                      # back at the DOS prompt
        )),

    "not-ready": Scenario(
        "A drive whose disk is not there: the one floppy drive is A: and "
        "B:, and the panels of the letter that it is not at the moment "
        "cannot be read. The box that says so, with another drive chosen "
        "there, with another try, and given up; and what the panels show "
        "then",
        steps=(
            "Ctrl+l",                     # two panels of files
            "Alt+F2",
            "a",                          # the right one on A:
            "Alt+F1",
            "a",                          # and the left one, the current
        ) + _DISK_B + (
            "F9 o d",                     # both are read again: Auto
                                          # directory sizes. DOS wants
                                          # the diskette of A:
            "Esc",                        # not given: the left panel
                                          # cannot be read
            "Esc",                        # no other drive: the right one,
                                          # on the same drive, is not tried
            "Click@5,15",                 # the left button beside the box
                                          # gives up: both are empty
            "Ctrl+l",                     # the Info: A: is read, the
                                          # diskette is A:'s again once
                                          # the box has been given up
        ) + _DISK_B + (
            "F9 o d",
            "F9 o d",                     # read again
            "Esc",
            "F10",                        # given up: an Info without
                                          # figures
            "F9 r t",                     # the tree
        ) + _DISK_B + (
            "F9 o d",
            "F9 o d",
            "Esc",
            "Esc",                        # a tree of the root alone
        ) + _DISK_B + (                   # a panel that was not read is
                                          # read when the viewer is left
            "Esc",
            "1 z c",                      # no letter, no such drive, and
                                          # a drive: the box shows it
            "Click@30,8",                 # the left button in the box does
                                          # nothing
            "RightClick@30,8",            # the right one is Enter: C: is
                                          # the current drive now
            "Alt+F1",                     # the diskette is still B:
            "b",
            "Shift+F3",
            '"a:\\fdauto.bat"',
            "Enter",
            "Enter",                      # and is A: now
            "Esc",
            "Ctrl+r",                     # so B: cannot be read
            "Esc",
            "Enter",                      # try again: the box has made
                                          # the diskette that of B:
            "F9 r b",                     # the right panel shows files
                                          # again, of A:
            "Esc",                        # which cannot be read
            "c",
            "Enter",                      # so it shows C: instead
        )),

    "second-copy": Scenario(
        "The setup saved by itself when it has changed, and the program "
        "started with a command to run: a second copy of it, run from "
        "the command line of the first",
        steps=(
            "F9 o Enter",                 # the Configuration
            "Tab Tab Tab Tab Down Space",  # Auto save setup
            "Enter",                      # which is a change: saved
            "Ctrl+l",                     # two panels of files, saved too:
                                          # the second copy starts so, and
                                          # shows no Info, whose count of
                                          # all the memory follows where
                                          # the copy is loaded
            # A Tab in the command line, typed as it is after ^Q, comes
            # to the second copy as it is, and is a space in its command.
            '"d:\\vc\\vc" Space "ver" Ctrl+q Tab "/r"',
            "Enter",                      # the second copy runs the
                                          # command and shows its panels
            "Ctrl+o",                     # what the command printed
            "Ctrl+o",
            "F10",
            "Enter",                      # the second copy left: the first
            "Ctrl+PgUp",                  # still works
        )),

    "user-menu": Scenario(
        "The user menu, of F2 and Shift+F2: the main one and that of a "
        "directory, its keys, lines of every kind, a menu that names "
        "another, and menu files that are not there, empty, or no file",
        steps=(
            "F9 o n",                     # no mini status: no time of day
            "F2",                         # no menu here: the main one
            "Down", "Down", "Down",       # round the end
            "Up", "Up",                   # and round the top
            "End", "Home", "PgDn", "PgUp",
            "Home",                       # where it is already
            "x",                          # the letter of no line
            "F10",                        # leaves: no line has that key
            "Shift+F2",                   # the main menu, asked for
            "F1",                         # a line's key: another menu
            "F1",                         # no line's key there: the help
            "Esc",
            "Esc",
            "F9 o o",
            "Down Down Down Down Down Space Enter",   # keys are kept
            "F7", '"d.mnu" Enter',        # a directory named like a menu
            "Shift+F4",                   # a menu of every kind of line
            '"l.mnu" Enter', "Enter",
            '"f11:abcdefghijkl"',
            "Ctrl+y Ctrl+u Ctrl+u Up Up End Del",  # a line of 32 characters
            "Ctrl+y Ctrl+u Ctrl+u Up Up End Del",   # 64: too wide
            "Down Home",
            'Tab "d.mnu" Enter',          # its command: the directory
            '"\'c" Enter',                # a comment
            '"f10:ten" Enter',
            'Tab "x.mnu" Enter',          # a menu that is not there
            '"F12:" Space "twelve" Enter',
            'Tab "nul.mnu" Enter',        # one with nothing in it
            '"f2:" Tab "two" Tab "x" Enter',  # tabs in a line
            'Tab Enter',                  # an empty command, and
            '"\'no" Enter',               # a comment, before the command
            'Tab "vc.mnu" Enter',
            '"f13:no" Enter',             # no keys, these
            '"g:" Enter',
            '"end"',                      # the file ends in its last line
            "F2 Esc",
            "Shift+F4", '"vc.mnu" Enter',  # the menu of this directory
            '"l:b" Enter Tab "l.mnu" Enter',  # narrower than a menu gets
            '"n" Enter',                  # a line without a command
            "F2 Esc",
            "F2",                         # found here now
            "F1",                         # the help
            "Esc",
            "Down Enter",                 # the line without a command
            "F2",
            "l",                          # the other menu, by its letter
            "End",
            "Enter",                      # the last line: nothing to run
            "F2 Enter",
            "F10",                        # a menu that is not there
            "Enter",
            "F2 Enter",
            "F12",                        # an empty one
            "Esc",
            "F2 Enter",
            "Enter",                      # a directory: nothing is said
            "F2 Enter",
            "F2",                         # back to the first menu
            "Esc",
            "F9 f Down Enter",            # by the menu bar
            "Esc",
            "Alt+F9",                     # 50 lines
            "F2 Enter",
            "Esc",
            "Alt+F9",
            "Home Down Enter",            # into DOCS
            "F7", '"vc.mnu" Enter',       # a directory where a menu is
            "F2",                         # looked for: the main one
            "Esc",
            '"del" Space "d:\\vc\\vc.mnu"',
            "Enter",
            "F2",                         # nor is that there
            "Esc",
        )),

    "user-menu-run": Scenario(
        "Commands of the user menu: what stands for the file, its drive "
        "and its path, the list of the selected files, with the tree in "
        "the panel and with the panels off, the menu that comes back by "
        "itself, and the command of a viewer for a kind of file",
        steps=(
            "F9 o n",                     # no mini status: no time of day
            "F9 o o",
            "Down Down Down Down Down Space Enter",   # keys are kept
            "Shift+F4", '"vc.mnu" Enter',
            '"a:" Tab "all" Enter',
            'Tab "echo" Space "!.!" Space "!!"',
            'Space "!:" Space "!\\" Space "!" Enter',
            'Tab "echo" Tab "t" Enter',   # a second one, with a tab
            '"f3:list" Enter',
            'Tab "@echo" Space "!@" Enter',
            '"\'no" Enter',
            'Tab "ver" Enter',
            "F2 Esc",
            "F2",
            "a",                          # on "..": the name is two dots
            "Ctrl+o", "Ctrl+o",
            "Down Down Down",             # onto a file
            "F2 a",
            "Ctrl+o", "Ctrl+o",
            "F2 F3",                      # the list file, of this one
            "Ctrl+o", "Ctrl+o",
            "Up Up Ins",                  # a directory selected,
            "Down Ins Ins",               # and two files
            "F2 F3",                      # the list of them
            "Ctrl+o", "Ctrl+o",
            '"type" Space "d:\\vc\\vc.lst"',
            "Enter",
            "Ctrl+o", "Ctrl+o",
            "Ctrl+m",                     # what was selected is so again
            "F9 o a",                     # Auto menus
            "F2 a",                       # the menu comes back by itself
            "Esc",
            "F9 o a",
            "Shift+F2",                   # the main menu
            "F4",                         # its command is not there
            "Ctrl+o", "Ctrl+o",
            "Shift+F4",                   # a viewer for a kind of file
            '"vcview.ext" Enter',
            '"txt:" Tab "echo" Space "!.!" Space "!"',
            "F2 Esc",
            "Up Up",
            "F3",                         # the file, as it is viewed
            "Ctrl+o", "Ctrl+o",
            "Shift+F3",                   # and one named with a path
            '"..\\work\\hello.txt" Enter',
            "Ctrl+o", "Ctrl+o",
            "F9 l t",                     # the tree: no file to name
            "F2 a",
            "Ctrl+o", "Ctrl+o",
            "F2 F3",                      # and none to list: a beep
            "Ctrl+o", "Ctrl+o",
            "F9 l Enter",                 # files again
            "Ctrl+o",                     # the panels off
            "F2",                         # the menu over what DOS shows
            "a",
            "Ctrl+o",
        )),

    "command-line": Scenario(
        "The command line: typing into it, Esc, the selected files and "
        "the paths of the panels put into it, and the speed search, which "
        "Alt and a letter start in a panel of files and in the tree",
        steps=(
            '"dir"',                      # typed into the command line
            "Backspace",                  # and edited there
            "Esc",                        # which Esc empties
            "Esc",                        # nothing in it: the panels go
            "Esc",                        # and come back
            "Ctrl+i",                     # the file of the cursor: not ".."
            "Down Ctrl+i",                # a directory, with a mask
            "Down Down Ctrl+i",           # a file
            "Ins Ins",
            "Ctrl+i",                     # the two that are selected
            "Ctrl+i Ctrl+i Ctrl+i Ctrl+i Ctrl+i Ctrl+i Ctrl+i Ctrl+i Ctrl+i",
            "Ctrl+m",                     # until no room; selected again
            "Esc",
            "Home",
            "Ctrl+[",                     # the path of the left panel
            "Ctrl+]",                     # the right one is the Info
            "Esc",
            "Ctrl+l",                     # two panels of files
            "Ctrl+]",                     # the root: no backslash more
            "Ctrl+[",
            "Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[",
            "Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[",
            "Ctrl+[",                     # no room for it: a beep
            "Esc",
            "Alt+r",                      # the speed search: readme
            "u",                          # run
            "x",                          # no such name
            "Backspace",                  # r again
            "Ctrl+Enter",                 # the next of them, and round
            "Ctrl+Enter",
            "Alt+e",                      # with Alt as without
            "Esc",                        # leaves it
            "Alt+d",                      # DOCS
            "Alt+a",                      # data
            "Up",                         # leaves it and is acted on
            "Alt+s",
            "Enter",                      # likewise: into SRC
            "Alt+z",                      # nothing there by that name
            "Esc",
            "Ctrl+PgUp",
            "F9 l t",                     # the tree
            "Ctrl+i",                     # has no files for the command line
            "Alt+s",                      # SRC is the next from here
            "Ctrl+Enter",                 # STAMPS
            "Ctrl+Enter",                 # SUB
            "t",                          # ST: STAMPS
            "Esc",
            "Ctrl+[",                     # the directory of the tree
            "Esc",
            "F9 o o",
            "Down Space Enter",           # Esc no longer hides the panels
            "Esc",
        )),

    "history-drives": Scenario(
        "The history of commands in its box, and the box that asks for "
        "the drive of a panel",
        steps=(
            "Alt+F8",                     # the history: nothing in it yet
            "Esc",
            '"ver"', "Enter",
            '"echo" Space "one"', "Enter",
            '"echo" Space "two"', "Enter",
            '"echo" Space',
            "Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[ Ctrl+[",
            "Enter",                      # a command of 77 characters
            "Alt+F8",                     # wider than the box gets
            "Up", "Up", "Up", "Up",       # round the top
            "Down", "Down",               # and round the end
            "Home", "End", "PgUp", "PgDn",
            "Home", "Home",
            "v",                          # the next that begins with it
            "e", "e", "e",                # and round
            "x",                          # none does
            "Ctrl+Enter",                 # into the command line
            "Esc",
            "Alt+F8",
            "Up Up",
            "Enter",                      # run again
            "Ctrl+o", "Ctrl+o",
            "Alt+F8",
            "F10",
            "Alt+F9",                     # 50 lines
            "Alt+F8",
            "Esc",
            "Alt+F9",
            "Alt+F1",                     # the drive of the left panel
            "Right", "Right", "Right",    # round the end
            "Left", "Left",               # and round the start
            "Home", "End", "Space", "Backspace",
            "z",                          # no such drive
            "Esc",
            "Alt+F1",
            "F10",
            "Alt+F1",
            "Enter",                      # the drive it has
            "Alt+F1",
            "a",                          # by its letter
            "Alt+F2",                     # the right panel: the Info goes
            "Right Enter",                # D:
            "Tab",
            "Alt+F1",                     # of the panel that is not current
            "c",
            "Alt+F2",
            "a",                          # the right one on A: as well
            "Ctrl+l",                     # the Info again, on the left
            "Alt+F1",                     # it becomes files
            "Enter",
        )),

    "enter": Scenario(
        "Enter on a file that DOS can run, with Shift held, with nothing "
        "to run, and in a tree whose directory is gone; the commands "
        "before and after, one looked for by how it begins, and more of "
        "them than are kept; the keys that change the directory, on "
        "another drive, and on one whose disk the user will not put in",
        steps=(
            "End Up",                     # onto run.bat
            "Enter",                      # its name is the command
            "Ctrl+o", "Ctrl+o",           # what it printed
            '"ver" Shift+Enter',          # with Shift: all of the program
                                          # stays in the memory this once
            "Ctrl+o", "Ctrl+o",
            '"ver" Enter',                # the same again is kept once
            "Ctrl+o",                     # the panels off, and nothing
            "Enter",                      # to run: a new line
            "Ctrl+o",
            "Ctrl+e",                     # the command before: ver
            "Ctrl+e",                     # and before that
            "Ctrl+e",                     # there is none
            "Ctrl+x",                     # the one after
            "Ctrl+x",                     # after the last: an empty line
            "Ctrl+x",
            '"R" Ctrl+Enter',             # the one that begins like this
            "Ctrl+y",
            '"zz" Ctrl+Enter',            # none does
            "Ctrl+y",
            # Five commands of 120 characters: the history keeps 512
            # bytes, and the oldest of what it has is cut off.
            *(f'"rem" Space "{letter * 116}" Enter' for letter in "abcde"),
            "Ctrl+e", "Ctrl+e", "Ctrl+e", "Ctrl+e",
            "Ctrl+e",                     # four are left
            "Ctrl+y",
            "F9 o Enter",
            "Tab Tab Tab Space Enter",    # Auto change directory off
            "F9 l t",                     # the tree
            "Down",                       # onto DOCS: DOS does not follow
            '"rd" Space "docs"',
            "Enter",                      # removed, and the tree has it
            "Enter",                      # still: nowhere to go
            "Down Enter",                 # SRC is there
            "F9 l b",                     # its files
            "Ctrl+PgUp",                  # up, onto where it came from
            "Ctrl+PgDn",                  # and into it again
            "Ctrl+PgUp Ctrl+PgUp",        # the root
            "Ctrl+PgUp",                  # which has nothing above it
            "Down Ctrl+PgDn",             # into STAMPS
            "Down Down Ctrl+PgDn",        # a file is no directory
            "Ctrl+\\",                    # the root at once
            "Ctrl+l",                     # the Info off: it would say
                                          # what is free on the floppy
            "Alt+F1 a",
            '"ver" Enter',                # a command on another drive
                                          # than the program started on
            "Shift+F3",                   # the floppy by its other letter
            '"b:\\freedos\\version.fdi" Enter',   # for the viewer:
            "Enter",                      # the disk is "put in"
            "Esc",
            "Down Down Down",             # onto FREEDOS, of A:
            "Enter",                      # whose disk is asked for now
            "Esc",                        # and refused
            "Ctrl+PgDn", "Esc",
            "Ctrl+\\", "Esc",
            "Enter",
            "Enter",                      # put in: into the directory
        )),

    "enter-many": Scenario(
        "A command run from a directory of more files than are kept "
        "while it runs, with the cursor on the last of them",
        steps=(
            'F7 "big" Enter',             # in the first minute: its time
            "Enter",                      # shows in it
            "Shift+F5",                   # small programs of DOS
            '"a:\\freedos\\bin\\v*.com" Tab "c:\\work\\big" Enter',
            'GrayPlus "*.com" Enter',
            # Eighteen files, fifteen times over: 288 with the first
            # ones, and 256 are kept.
            *(f'Ctrl+m F5 "*.{number:x}" Enter' for number in range(15)),
            "F9 l f",                     # one to a line
            "End",
            '"ver" Enter',                # the last one is kept, and the
                                          # cursor is on it afterwards
            "Up",
            "Home",
            "End",
        )),

    "stamps": Scenario(
        "Sizes, dates and times of files, wherever they are shown",
        steps=_STAMPS_STEPS),

    "stamps-russia": Scenario(
        "The same where the day comes first and the clock has 24 hours",
        country="ru", steps=_STAMPS_STEPS),

    "names": Scenario(
        "Names of files as they are typed: tidied up, shortened to fit, "
        "and with a letter beyond Z",
        steps=_NAMES_STEPS),

    "russian": Scenario(
        "A file with a Russian name, typed on the Russian keyboard",
        country="ru",
        steps=(
            "Down Down Down",             # onto a file
            "F5",                         # a copy of it
            # The keyboard's Cyrillic letters are selected here, where Alt
            # by itself means nothing: at the panels it brings up the
            # menu, if the program sees it pressed and let go in time.
            'Alt+RightShift '
            '"\u043f\u0438\u0441\u044c\u043c\u043e_1.\u0434\u043e\u043a"',
            "Enter",                      # the panel lists it
            "End",                        # the status line names it
            "F3",                         # and so does the viewer
            "Esc",
            "Shift+F3",                   # typed again, in capitals
            '"\u041f\u0418\u0421\u042c\u041c\u041e_1.\u0414\u041e\u041a"',
            "Enter",
            "Esc",
        )),

    "clock": Scenario(
        "The clock in the corner of the screen, which the bench has off",
        steps=_CLOCK_STEPS),

    "clock-russia": Scenario(
        "The same where the clock has 24 hours",
        country="ru", steps=_CLOCK_STEPS),

    "clock-path": Scenario(
        "The path of the right panel, which moves over for the clock",
        steps=_CLOCK_PATH_STEPS),

    "clock-path-russia": Scenario(
        "The same where the clock has 24 hours and is wider",
        country="ru", steps=_CLOCK_PATH_STEPS),

    "clock-morning": Scenario(
        "The clock before ten in the morning, a minute before the hour",
        steps=_CLOCK_EARLY_STEPS + ("F9 o l",)),

    "clock-night-russia": Scenario(
        "The clock after midnight where it has 24 hours",
        country="ru",
        steps=('"time" Space "0:59"',) + _CLOCK_EARLY_STEPS[1:]
        + ("F9 o l",)),

    "laptop": Scenario(
        "The marks of selected and of hidden files in the laptop palette, "
        "which has no colour to tell them by",
        steps=(
            "F9 o Enter",
            "Down Space",                 # laptop
            "Enter",
            "Down Down Down",             # onto readme.txt
            "Ctrl+a",
            "Tab Tab Space",              # hidden: a mark before its
            "Enter",                      # extension
            "Ins",                        # selected too: another mark
            "Ins",                        # hello.txt only selected
            "Up",                         # the cursor on the one
            "Up",                         # and on the other
        )),

    "columns": Scenario(
        "A list of files longer than a column of its panel, what is "
        "selected counted, and the panel without its status line",
        steps=(
            "F9 o f",                     # panels half the screen high:
                                          # ten files to a column
            "Down Down Down Down",        # onto hello.txt
            "F5",
            '"hello.doc"',
            "Enter",                      # the tenth entry
            "Shift+F4",                   # a new file, made in the editor
            '"one.txt"',
            "Enter",
            "Enter",                      # it is not there: a new one
            '"a"',                        # of one byte
            "F2",
            "Esc",                        # the eleventh, in a column of
            "End",                        # its own
            # It has the time of day. It gets the time of the bench's
            # files in one step, so that no screen shows its seconds.
            'Ctrl+a Tab Tab Tab Tab Tab "12:00:00p" Enter',
            "Ins",                        # 1 byte in 1 selected file
            "Home",
            "Down Ins",                   # a directory too: not counted
            "Alt+F6",                     # until its size is worked out
            "F9 o n",                     # the mini status off
        )),

    "dirinfo": Scenario(
        "The Info panel, with what a directory says about itself in its "
        "file dirinfo",
        steps=(
            # The editor empties the keyboard buffer when it has taken a
            # key and again before it draws, so of keys pressed one behind
            # the other it may lose one. Not with this option off.
            "F9 o o",                     # the advanced options
            "Down Down Down Down Down Space",     # Clear keyboard buffer
            "Enter",
            "Down Enter",                 # C:\\WORK\\DOCS: 0 files
            "Shift+F4",                   # a new file, made in the editor
            '"dirinfo"',
            "Enter",
            "Enter",
            '"ab" Tab "c" Enter',         # a tab: on to the eighth column
            'Tab Tab Tab Tab Tab "x" Enter',  # further than the line goes
            '"0123456789012345"',         # a line longer than the 36
            '"6789012345678901"',         # columns that are shown of it
            '"23456789" Enter',
            '"last"',                     # and one that does not end
            "F2",
            "Esc",                        # 1 file; the Info shows the text
            "Ctrl+F1",                    # no panel of files next to it:
                                          # the Info is as it was, until
            "F9 o f",                     # it is drawn again, in panels
                                          # half the screen high, without
                                          # the files and with three lines
            "Ctrl+F1",                    # of the text
        )),

    "memory": Scenario(
        "The Info panel with Memory allocation off: the memory that is "
        "free is what all of the program leaves, a number that is not "
        "compared",
        steps=(
            "F9 o m",                     # off: the number is another
            "F9 o",                       # and the menu has it without
                                          # its mark
        )),

    "menu": Scenario(
        "The menu bar and its menus by every key: along the bar and round "
        "it, up and down a menu past its lines and its ends, the help of "
        "an item, the keys that the items name, the last item again; and "
        "the items that a menu has dark, for a tree, with the panels off, "
        "and next to a panel of each kind",
        steps=(
            "End",                        # a file under the cursor
            "Shift+F10",                  # nothing chosen yet: the bar
            "z Home End Up PgUp PgDn Left",   # keys that mean nothing on
                                          # the bar, and round to Right
            "Right",                      # round to Left
            "Left Left",                  # Options
            "F1",                         # the help of that menu
            "Esc",
            "Down",                       # the menu, with its ticks
            "Up",                         # round to the last item
            "Up",                         # over the line between
            "End Down",                   # round to the first
            "PgDn",
            "PgUp Home",
            "Down Down Down Down Down",   # over the line, to Auto menus
            "F1",                         # the help of that item
            "Esc",
            "Right",                      # the Right menu, of the Info:
                                          # no filter, Ctrl-F2 and Alt-F2
            "Right",                      # round to the Left menu
            "Ctrl+F2 Ins z",              # the key of another menu, a key
                                          # of none, a letter of none
            "F10",                        # leaves the menu
            "Shift+F10 Enter",            # still nothing chosen: the bar,
                                          # and Enter opens the menu
            "Esc",
            "F9 Right Enter",
            "Enter",                      # Help, in the Files menu
            "Esc",
            "Shift+F10",                  # the Files menu, on Help again
            "a",                          # the attributes of the file
            "Esc",
            "Ctrl+z Tab",                 # the tree on the right, active
            "Shift+F10",                  # the Files menu of a tree: five
                                          # items dark, attributes among
                                          # them, so the cursor is on Quit
            "Ctrl+a",                     # the key of a dark item
            "Up",                         # over the dark items, to Delete
            "Right",                      # Commands, of a tree
            "Right Right",                # the Right menu, of a tree
            "Right",                      # the Left one, next to a tree:
                                          # no Tree
            "f",                          # Full, for the panel that is
                                          # not the active one
            "Tab F9 c",                   # Commands, next to a tree
            "Ctrl+o",                     # the key of an item: panels off
            "F9",                         # without panels: Left
            "c",                          # Commands, three items dark
            "Left",                       # Files, as of a tree
            "r",                          # Rename asks for a name, and
                                          # offers the last target, none:
                                          # it goes by what the bar left
                                          # in two registers
            "Esc",
            "F9 l",                       # Left: off, and no filter
            "Esc",
            "Ctrl+o",
            "F9 r b",                     # files on the right
            "F9 c",                       # Commands, nothing dark
            "Esc Tab Ctrl+p",             # the left panel off
            "F9 c",                       # no comparing
            "Esc",
        )),

    "menu-panel": Scenario(
        "The commands of a panel's menu where they have something to "
        "mend: a panel made one of files whose directory is gone, the "
        "filter by its key, the drive from the menu, a panel switched on "
        "while the current one is off, and 50 lines left with the command "
        "line high up",
        steps=(
            "Down Enter",                 # the left panel in C:\\WORK\\DOCS
            "Ctrl+l",                     # files on the right, in C:\\
            "F9 l i",                     # the Info on the left: DOS goes
                                          # where the other panel is
            '"rd" Space "work\\docs" Enter',  # the directory that the
                                          # left panel had is gone
            "F9 l b",                     # files again: where DOS is
            "Ctrl+f",                     # the filter, by its key
            "Esc",
            "F9 l d",                     # the drive, from the menu
            "Esc",
            "F9 r d",                     # and that of the right panel,
            "a",                          # which shows files then
            "Ctrl+F2",                    # the right panel off,
            "Ctrl+F1",                    # and the left one, the current
            "Ctrl+F2",                    # the right one on, and current
            "Ctrl+F1",
            "Ctrl+o",                     # no panels: the command line
            "Alt+F9",                     # stays where it is in 50 lines,
            "Alt+F9",                     # and is too far down for 25
            "Ctrl+o",
        )),

    "compare": Scenario(
        "Two directories compared: a file that the other has not, one "
        "that is newer by its time and one by its date, one that is a "
        "directory there, and two directories that are the same",
        steps=(
            "Ctrl+c",                     # not with the Info next to it
            'F7 "1" Enter',               # a directory named 1
            'Home Down Down Down F5 "src\\" Enter',  # readme.txt into SRC,
            'F5 "src\\1" Enter',           # and again as a file named 1
            'Down F5 "src\\" Enter',       # hello.txt, and here an hour
            'Ctrl+a Tab Tab Tab Tab Tab "1:00:00p" Enter',     # later
            'Down F5 "src\\" Enter',       # notes.doc, and here a day
            'Ctrl+a Tab Tab Tab Tab "6-17-00" Enter',          # later
            "Ctrl+l",                     # two panels of files
            "Tab Enter",                  # both in C:\\WORK
            "Tab Ctrl+c",                 # the same: a box says so
            "Enter",
            "Tab Down Down Enter",        # the right one in SRC
            "F9 c c",                     # five files here that are not
                                          # there or are newer, and one
                                          # there that is a directory here
            "Tab",
            "Ctrl+o Ctrl+c",              # not without panels
            "Ctrl+o",
        )),

    "menu-files": Scenario(
        "The menu file and the extension files edited from the menus, "
        "the main ones and those of a directory, and the setup saved, "
        "also where its file cannot be written",
        steps=(
            "F9 c m",                     # the menu file: which one?
            "Esc",
            "F9 c m",
            "Enter",                      # the main one, in the editor
            "Esc",
            "F9 c m",
            "Right Enter",                # that of the directory: none yet
            "Esc",
            "F9 o x",                     # the extension file
            "Enter",
            "Esc",
            "F9 o v",                     # that of the viewers,
            "Right Enter",                # of the directory
            "Esc",
            "F9 o e",                     # that of the editors:
            "Right Right Enter",          # not after all
            "F9 o e",
            "Enter",
            "Esc",
            "F9 o p",                     # the prompt without its path
            "Shift+F9",                   # save the setup?
            "Esc",
            "Shift+F9 Enter",             # saved
            '"a:\\freedos\\bin\\attrib" Space "+r" Space "d:\\vc\\vc.ini" '
            'Enter',                      # its file read-only
            "F9 o s",
            "Enter",                      # not saved: a box says so
            "Enter",
        )),

    "phantom": Scenario(
        "The floppy drive under its second letter: a command of a "
        "panel's menu that has DOS go to a directory of A: asks for the "
        "diskette, and does nothing when that is refused",
        steps=(
            "Alt+F2 a",                   # the right panel on A:
            'Shift+F8 "b:*.zzz" Enter',   # delete nothing on B: the
            "Enter",                      # diskette is B: now, and A: is
                                          # asked for to read the panel
            "Esc",                        # no: the disk cannot be read
            "Esc",                        # and is not
            "F9 l t",                     # the tree on the left: DOS is
                                          # to go where the other panel is
            "Esc",                        # no: no tree
            "F9 r i",                     # the Info on the right,
            "Tab",                        # current,
            "F9 r b",                     # and files again: in A:\\
            "Esc",                        # no: the Info stays
            "F9 r b",
            "Enter",                      # yes
        )),

    "memory-map": Scenario(
        "The map of the memory, Alt+F5: its lines and the cursor, and a "
        "program that stays in the memory selected and released",
        steps=(
            "Alt+F5",                     # the cursor on the first line
            "Down Down",
            "Up",
            "End",
            "Down",                       # no further
            "Home",
            "PgDn",
            "PgUp",
            "Ins",                        # nothing to release there: down
            "GrayPlus",                   # nor anywhere
            "GrayMinus",
            "Enter",                      # Release: nothing
            "F1",                         # the help
            "Esc",
            "Right",                      # Cancel
            "Enter",
            "Alt+F5",
            "F10",
            "F9 o m",                     # Memory allocation off: all of
            "Alt+F5",                     # the program stays
            "Esc",
            "F9 o m",
            "F9 o c",
            "Tab Tab Space Enter",        # Ins moves down: off
            '"a:\\freedos\\bin\\fdapm" Space "apmdos" Enter',
            "Alt+F5",                     # a program that stays, marked
            "Ins",                        # DOS is not selected
            "End Up",                     # onto the program
            "Ins",                        # selected
            "Ins",                        # and not
            "GrayPlus",
            "GrayMinus",
            "Enter",                      # released without that
            "Alt+F5",                     # gone
            "Esc",
            '"a:\\freedos\\bin\\fdapm" Space "apmdos" Enter',
            "Alt+F5",
            "GrayPlus",
            "Enter",                      # released as selected
            "Alt+F5",
            "Esc",
        )),

    "memory-map-russia": Scenario(
        "The map of the memory with Russian numbers",
        country="ru",
        steps=(
            "Alt+F5",
            "End",
            "Esc",
        )),

    "tree-deep": Scenario(
        "A tree thirteen directories deep, which is drawn narrower and has "
        "no room for the last of them, and longer than its panel",
        steps=(
            "Ctrl+l",                     # the Info panel off: two panels
            *(f'F7 "{letter}" Enter Enter'    # a directory, and into it
              for letter in "abcdefghijklm"),
            "F9 r t",                     # the tree in the right panel
            "Ctrl+PgUp",                  # the files go up a directory, and
                                          # the cursor of the tree with
                                          # them: twelve levels deep
            "Tab",
            "Up",                         # eleven
            "Up",                         # and ten
            "F9 o f",                     # panels half the screen high:
                                          # the tree begins further down
            "Home",
            "End",
            "Ctrl+l",                     # the Info next to the tree
            "Alt+F10",                    # the tree in a box, to choose a
            "Up",                         # directory from
            "Esc",
        )),

    "tree-box": Scenario(
        "The box of the tree with more directories than it has lines: the "
        "keys that move in it, the speed search, and the disk read again, "
        "once with a key that stops it",
        steps=(
            "Ctrl+l",                     # the Info panel off: two panels
            *(f'F7 "{letter}" Enter Enter'    # a directory, and into it
              for letter in "abcdefghijklm"),
            "Alt+F10",                    # 19 directories, read from the
                                          # disk and saved: the cursor on
                                          # the current one, the last lines
            "Home",
            "Home",                       # nothing to do
            "Up",
            "PgDn",                       # the lines turn, the cursor stays
            "PgDn",                       # no more lines: to the last one
            "Down",
            "PgUp",
            "PgUp",                       # the first lines: to the first one
            "Down Down Down Down Down Down Down "
            "Down Down Down Down Down Down Down",     # one more than fit:
                                          # the screen moves up a line
            "PgUp",                       # and down a line
            "End",
            "Up",
            '"d"',                        # the next one that begins so
            "Ctrl+Enter",                 # and the one after it
            "F2+Esc",                     # read again, and Esc waiting when
                                          # the disk is asked: only the root
            "Ctrl+r",                     # read again, all of it
            "F2",                         # and again: the same
            "F10",
        )),

    "tree-choose": Scenario(
        "A directory chosen from the tree, of a disk with few directories "
        "and of one whose tree cannot be saved",
        steps=(
            "Down Ins Ins F8",            # DOCS and SRC
            "Enter",
            "Enter",                      # deleted: three directories left
            "Alt+F10",                    # too few for a tree file
            '"s"',                        # the next one with an s: STAMPS
            "Ctrl+Enter",                 # SUB
            "Ctrl+Enter",                 # and round to STAMPS
            '"u"',                        # SUB
            '"x"',                        # there is no SUX
            "Backspace",
            "Enter",                      # into it
            'F7 "\\treeinfo.ncd" Enter',  # a directory where the tree
                                          # file would be
            "Alt+F10",                    # five to save now: the file
                                          # cannot be made, and that beeps
            "Esc",
            "Alt+F1 a",                   # a disk that cannot be written
            "Alt+F10",                    # five directories too: no beep
            "Down Enter",                 # A:\\FREEDOS
            "Ctrl+l",                     # the right panel one of files,
            "F9 l t",                     # the left one the tree
            "Alt+F10",                    # the box next to it
            "F10",
        )),

    "reread": Scenario(
        "A directory read again: what a panel keeps of the list it had, "
        "and where it goes when its directory is gone",
        steps=(
            "Down Down Down Ins",         # a file selected
            'F7 "dirinfo" Enter',         # the list is read again, with a
                                          # directory more: the file stays
                                          # selected. The Info cannot open
                                          # a directory for its text, and
                                          # says nothing of the label
            "Home Down Ins Up Enter",     # a directory selected, and into
                                          # it: its list has nothing of
                                          # the one before
            "Ctrl+PgUp",
            'F7 "work" Enter',            # C:\\WORK\\WORK
            "Ctrl+PgUp",
            "Ins Up Enter",               # C:\\WORK selected, and into it:
                                          # the WORK in there is not
            "Ctrl+F4",                    # the label of the disk, to change
            "Esc",
            "Down Down Down",
            'F5 "src\\" Enter',           # a file into C:\\WORK\\SRC
            "Ctrl+l",                     # two panels of files
            "Ctrl+u",                     # which change places: the one
            "Ctrl+r",                     # that is first in the memory is
            "Tab",                        # the right one now
            "Ctrl+r",
            "Tab",
            "Home Down Down Enter Down",  # the right one in C:\\WORK\\SRC,
            "Tab",                        # on the file
            "Enter",
            "Down Down",
            "F8",                         # the left one deletes SRC with
            "Enter",                      # what is in it: the right one
            "Enter",                      # shows the current directory
                                          # instead, from its top, though
                                          # it has a file of that name
        )),

    "sizes": Scenario(
        "The sizes of directories worked out as a panel is read, and an "
        "extension file of the directory's own",
        steps=(
            "F9 o d",                     # Auto directory sizes
            "Down",                       # a directory with nothing in it
            "Ctrl+PgUp",                  # one with files
            "Down",                       # and one with more than 4 GB
            "Up Enter",
            "Down Down Down",
            'F5 "a.zip" Enter',           # a file of a kind that the main
            "End Enter",                  # extension file has a command
            "Ctrl+o",                     # for, which DOS does not find
            "Ctrl+o",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of the file made
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",
            '"vc.ext"',
            "Enter",
            '"txt:" Tab "echo" Space "!.!"',
            "F2",
            "Esc",                        # the directory is read again,
            "Home Down Down Down Enter",  # and its vc.ext with it
            "Ctrl+o",                     # what the command printed
            "Ctrl+o",
        )),

    "view": Scenario(
        "The viewer: lines wrapped and not, a line longer than the screen "
        "moved sideways, and a text found in it",
        steps=(
            "Down Down Down Down Down",   # onto notes.doc, of three lines
            "F3",
            "F2",                         # not wrapped: the same
            "Down",                       # all of it is shown: no further
            "Esc",
            "Down",                       # the file of all bytes: they do
            "F3",                         # not end a line anywhere
            "F2",                         # not wrapped: one line, that goes
            "Right",                      # on to the right; and to the left
            "Ctrl+Right",                 # once it is moved, by one column
            "Left",                       # or by forty
            "Ctrl+Left",
            "F7",
            '"abc"',                      # a text, small letters for the
            "Enter",                      # capitals there: moved into view
            "F2",                         # wrapped: from where it was found
            "Up",                         # a line at a time
            "Down",
            "F4",                         # in hex: found in the column of
            "Up",                         # characters
            "Down",
            "PgUp",
            "Shift+F7",                   # and found once more, where the
            "Esc",                        # bytes repeat
        )),

    "view-parts": Scenario(
        "A file larger than the memory the viewer has for it, read a part "
        "at a time, forwards and backwards",
        steps=(
            "Shift+F3",                   # the shell that DOS started from
            '"a:\\freedos\\bin\\command.com"',
            "Enter",                      # the first 64K of its 87772 bytes
            "Down",
            "Up",
            "PgDn",
            "PgUp",
            "PgDn",
            "Home",
            "F2",                         # not wrapped: the lines of that
                                          # part fill no screen
            "PgDn",                       # so the next part is read, which
                                          # begins half a part further on
            "PgUp",                       # and the one before it again
            "End",                        # the end of the file: half a part
            "PgUp",                       # the part before it, by half
            "PgUp",                       # and the first
            "F4",                         # in hex
            "PgDn",
            "Up",
            "End",
            "Esc",
        )),

    "view-keys": Scenario(
        "The viewer: a file that is not there, its keys where they have "
        "nothing to do, the last lines of a file, a text found off the "
        "screen, and a file of one byte",
        steps=(
            "Shift+F3",
            '"nosuch.txt"',
            "Enter",                      # no such file: a box
            "Enter",
            "Shift+F4",                   # the editor says so too, where
            '"nodir\\x.txt"',             # it is the directory that is
            "Enter",                      # not there
            "Enter",
            "Down Down Down Down Down",   # onto notes.doc, of three lines
            "F3",
            "Home",                       # at the start already
            "Shift+F7",                   # nothing to look for yet
            "F7",
            "Esc",                        # nor now
            "F7",
            '"three"',
            "Enter",                      # found in the last line
            "Shift+F7",                   # and no line behind that to look
            "Esc",                        # in: not found, a box
            "Esc",
            "Down",                       # the file of all bytes
            "F3",
            "F2",                         # not wrapped: one line
            "Right",
            "Ctrl+Left",                  # back by forty: to the first
            "F7",                         # column, not beyond it
            '"xyz"',
            "Enter",                      # found to the right of the
            "F7",                         # screen: the line is moved
            '"qq"',
            "Enter",                      # not found, so the next search
            "Esc",                        # begins where the last one found
            "Ctrl+Right Ctrl+Right",      # something
            "F7",
            '"xyz"',
            "Enter",                      # found to the left of the screen
            "F4",                         # in hex
            "F2",                         # no wrapping there
            "Home",
            "End",                        # the last 23 lines of 64
            "Home",
            "PgDn",
            "PgDn",                       # no further than the end
            "Up",
            "Down",                       # the last line comes up again
            "Esc",
            "Shift+F4",                   # make a file of one byte
            '"one"',
            "Enter",
            "Enter",
            '"a"',
            "F2",
            "Esc",
            "Shift+F3",                   # which the status line counts as
            '"one"',                      # 1 Byte
            "Enter",
            "Esc",
            "Shift+F5",                   # and one of 65109 bytes, of two
            '"a:\\freedos\\bin\\format.exe" Tab "big" Enter',
            "Shift+F5",                   # programs: less than 64K, and
            '"a:\\freedos\\bin\\xcopy.exe" Tab "big" Enter',
            "p",                          # more than the viewer holds
            "Shift+F3",
            '"big"',
            "Enter",
            "End",                        # the end, read from the middle on
            "Home",                       # and the start again
            "Esc",
        )),

    "view-window": Scenario(
        "A dirinfo that is too large for the editor, viewed in the window "
        "of the Info panel, which is narrow and has no status line",
        steps=(
            "Home Enter",
            "Down Down Enter",            # C:\\STAMPS
            "Down Down Down Down Down",   # a megabyte, by its entry in
            "F6",                         # the directory, and no contents
            '"dirinfo" Enter',
            "Tab",                        # to the Info panel
            "F4",                         # its dirinfo: too large to edit
            "Enter",                      # View
            "Down",
            "F2",
            "F4",                         # in hex, a line is wider than
            "End",                        # the window
            "Home",
            "Esc",
        )),

    "view-search": Scenario(
        "A text looked for in a file larger than the memory the viewer "
        "has for it, found in a part that is read for it, and a search "
        "stopped by a key",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # a key that is to wait
            "Enter",
            "Shift+F3",                   # the shell that DOS started from
            '"a:\\freedos\\bin\\command.com"',
            "Enter",
            "F2",                         # not wrapped
            "Down Down Down Down",        # the fifth line begins in the
                                          # second half: the next part is
                                          # read
            "Home",                       # and the first again
            "PgDn",                       # the next part, from its start
            "Up",                         # the one before it
            "F7",
            '"backslash"',                # behind what is in the memory:
            "Enter",                      # the file is read on, and the
                                          # part around the place shown
            "Home",
            "F7",
            '"qqqq"',                     # nowhere: Esc is waiting when the
            "Enter+Esc",                  # file is to be read on, and stops
            "Esc",                        # the search
        )),

    "view-commands": Scenario(
        "F3, F4 and their kin: a name asked for and not given, the "
        "program's own viewer and editor whatever the setup, extension "
        "files that are missing or cannot be opened, and a program "
        "named in one as the viewer or the editor: run, on a disk that "
        "is not put in, and one that DOS will not run",
        steps=(
            "F9 o n",                     # no mini status: no time of day
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space Enter",   # the editor
            "Shift+F3",                   # which file? none
            "Esc",
            "Down Down Down",             # onto readme.txt
            "Alt+F3",                     # the viewer, without a look at
            "Esc",                        # the extension file
            "Alt+F4",                     # the editor is the program's
            "Esc",                        # own, so Alt asks vcedit.ext
            'F7 "vcview.ext" Enter',      # a directory of that name
            "Shift+F3",
            '"readme.txt" Enter',         # cannot be opened: no viewer
            "F8", "Enter",                # the directory goes
            '"del" Space "d:\\vc\\vcview.ext"',
            "Enter",                      # and the main file
            "Home Down Down Down",
            "F3",                         # no extension file: the viewer
            "Esc",
            "Shift+F4",                   # what DOS takes for a program
            '"nomem.exe" Enter',          # that needs six megabytes
            "Enter",
            '"MZ0000000000000000000000000000000000"',
            "F2",
            "Esc",
            "Shift+F4",
            '"vcview.ext" Enter',
            '"txt:" Tab "a:\\freedos\\bin\\vecho.com" Enter',
            '"doc:" Tab "b:\\x.com" Enter',
            '"bin:" Tab "a:\\freedos\\bin\\v*.com" Enter',
            '"exe:" Tab "c:\\work\\nomem.exe" Enter',
            "F2 Esc",
            "Shift+F4",
            '"vcedit.ext" Enter',
            '"txt:" Tab "a:\\freedos\\bin\\vecho.com" Enter',
            "F2 Esc",
            "Home Down Down Down",
            "F3",                         # readme.txt: the program is run
            "Alt+F4",                     # and as the editor
            "Down Down",
            "F3",                         # notes.doc: its viewer is on B:
            "Esc",                        # and the disk is not put in
            "Down",
            "F3",                         # data.bin: DOS finds a file for
            "Enter",                      # the mask and can run none
            "End Up Up",
            "F3",                         # nomem.exe: no memory for it
            "Enter",
            "Ctrl+o",                     # what the programs printed is
            "Ctrl+o",                     # not kept
        )),

    "editor-hex": Scenario(
        "The editor on a file that is one line of all the characters "
        "there are, as text and in hex",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of the file saved
            "Down Down Down Down Down Down",      # onto data.bin
            "F4",                         # 1024 characters, tabs among
                                          # them: an arrow where it goes on
            "End",                        # and one where it began
            "Home F4",                    # in hex, a zero as a dot
            '"4a"',                       # the two digits of a byte
            'Tab "z"',                    # and a byte as a character
            "Down Down End",
            "F4",                         # text again, the cursor where
                                          # it was
            "Esc",                        # changed: asks
            "Enter",                      # and saves
        )),

    "editor-long": Scenario(
        "A line longer than the editor keeps of one at a time",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",                   # a new file
            '"long.txt" Enter',
            "Enter",
            '"0123456789abcdef"',
            # The line cut, put back twice and the two joined: twice as
            # long, and that eight times. 4096 characters.
            *("Ctrl+y Ctrl+u Ctrl+u Up Up End Del "
              "Ctrl+y Ctrl+u Ctrl+u Up Up End Del",) * 4,
            "End",                        # as far as the cursor goes, the
                                          # 3900th column
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
        )),

    "editor-save": Scenario(
        "What the editor meets when it saves: a file that is read-only, a "
        "device, a directory that is not there, and the main extension "
        "file",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of the file saved
            "Down Down Down Down",        # onto hello.txt
            "Ctrl+a",
            "Space",                      # read-only
            "Enter",
            "F4",
            '"x"',
            "F2",                         # asks whether to overwrite it
            "Enter",                      # yes
            "Shift+F2",
            '"nul" Enter',                # saved to a device
            '"y"',
            "Shift+F2",
            '"nodir\\x" Enter',           # no such directory: an error
            "Enter",
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
            "Shift+F4",                   # the main extension file, in the
            '"d:\\vc\\vc.ext" Enter',     # directory of the program
            '"dat:" Tab "echo" Space "!.!" Enter',
            "F2",                         # saved, and from now on what the
            "Esc",                        # program goes by
            "End Enter",                  # empty.dat: the command just
            "Ctrl+o",                     # written for its kind
            "Ctrl+o",
        )),

    "editor-words": Scenario(
        "The editor from line to line and from word to word, as text and "
        "in hex, in a text with empty lines",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",                   # a new file
            '"words.txt" Enter',
            "Enter",
            '"one" Space "two" Enter Enter Enter',
            '"bcdefghijk"',
            'Up "xy"',                    # up past two empty lines
            'Up Up Up "q"',               # to the top, and no further
            "Ctrl+Right",                 # the next word
            "Home Ctrl+t",                # a word deleted
            "F4",                         # in hex: the last line is short
            "Down",
            'Up Up "4142"',               # up, and no further than the top
            "Down Right F4",              # text again, the cursor in the
                                          # line that its byte is in
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
        )),

    "editor-full": Scenario(
        "The editor on a line of more than 64K, and with all of its "
        "memory full",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",                   # a new file
            '"full.txt" Enter',
            "Enter",
            '"0123456789abcdef"',
            # The line cut, put back twice and the two joined: twice as
            # long, and that eight times. 4096 characters.
            *("Ctrl+y Ctrl+u Ctrl+u Up Up End Del "
              "Ctrl+y Ctrl+u Ctrl+u Up Up End Del",) * 4,
            # To the next word, of which there is none: as far as the
            # editor keeps the line, and the last column but one. The line
            # cut off there, 3899 characters, and cut, and put back twelve
            # times.
            "Home Ctrl+Right Ctrl+k Ctrl+y "
            + " ".join(["Ctrl+u"] * 12),
            # Twenty lines. The sixteenth and the seventeenth joined, and
            # then the one above with what that makes, fifteen times: 17
            # lines in one, 66283 characters.
            " ".join(["Ctrl+u"] * 8) + " Up Up Up Up Up End Del",
            " ".join(["Up End Del"] * 5) + " Home",
            " ".join(["Up End Del"] * 5),
            " ".join(["Up End Del"] * 5) + " Home",
            "Down",                       # the line after it
            "F4",                         # in hex,
            "Up",                         # 16 bytes back: in the long line,
            "F4",                         # more than 64K from its start
            # A line of 3899 cut, and put back at the end of the text, as
            # often as a step has keys for: after five such steps there is
            # no room for another, and the editor beeps. Each ends at the
            # top of the text, which is the same however much was put in.
            "Down Ctrl+y Ctrl+End " + " ".join(["Ctrl+u"] * 12)
            + " Ctrl+Home",
            *("Ctrl+End " + " ".join(["Ctrl+u"] * 14) + " Ctrl+Home",) * 5,
            '"ab"',                       # two characters more than there
                                          # is room for: none is free
            "F4",                         # in hex: a beep for the two
            # A byte typed behind the last one has no room either: a
            # beep, at the end of the text, which the port has elsewhere.
            # The key bar is gone with it; F4 brings another.
            'Ctrl+End "4" Ctrl+Home F4',
            "Del Del",                    # the two taken out again
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
        )),

    "editor-keys": Scenario(
        "The keys of the editor in a text: by characters, words, lines "
        "and pages, deleting and putting back, and those it has no use "
        "for",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",                   # 23 lines, in a window of 13
            '"d:\\vc\\vcview.ext" Enter',
            '"alpha" Space "beta" Space "gamma"',
            # A line of its own; back in it, a character deleted before
            # the cursor and one under it.
            "Enter Up End Left Left Left Backspace Del",
            "Ctrl+Left Ctrl+Left Ctrl+w", # two words back; one cut
            "Ctrl+w Left Backspace",      # nothing before the first line
            "Ctrl+u End Ctrl+u Ctrl+k",   # the word put back, twice; at
                                          # the end nothing to cut off
            "Ctrl+Left Ctrl+k Home Ctrl+u",   # the end cut off, and put
                                              # back at the start
            "Ctrl+q",                     # the next key is a character:
            "Ctrl+y",                     # that of Ctrl-Y
            "Ctrl+q Up F5 Ctrl+b",        # and none of these is one
            "End Right",                  # on to the next line,
            "Left",                       # and back to the end of this
            "Down Home Backspace",        # the next line joined to it
            "Ctrl+End Right End Del",     # nothing behind the last line
            "Ctrl+Home PgDn",             # to the last line of the window
            "PgDn",                       # a page, as far as there is one
            "PgDn",
            "PgUp",                       # to the first line of the window
            "Backspace",                  # joined to the line above it,
                                          # which the window did not show
            "PgUp",                       # a page, as far as there is one
            "PgUp",
            "Ctrl+y",                     # a line cut: one more comes up
            "End Del",                    # and with two lines joined
            "Shift+F2",                   # another name asked for,
            "Esc",                        # and not given
            "Esc",                        # changed: asks
            "Right Right Enter",          # go on editing
            "Esc",
            "Right Enter",                # not saved
        )),

    "editor-hex-keys": Scenario(
        "The keys of the editor in hex: from digit to digit and byte to "
        "byte, bytes added behind the last and taken off again, pages",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Down Down Down Down",        # onto hello.txt, 15 bytes
            "F4",
            "F4",                         # in hex
            "Right Right Right",          # the second digit of a byte
            "Left",                       # its first
            "Left",                       # the second of the one before
            "Left Left Backspace",        # no further than the first byte
            "Tab Right Right Left",       # by characters
            "Backspace",
            "Ctrl+End",                   # behind the last byte
            '"z"',                        # a sixteenth: the next line
            'Tab "4"',                    # the first digit of another,
            '"1"',                        # and the second
            "Backspace",                  # taken off again,
            "Backspace",                  # and the one before: back in
                                          # the first line
            "Left",
            "Home Up",
            # Keys for a text.
            "Ctrl+Left Ctrl+Right Del Ctrl+y Ctrl+k Ctrl+u Ctrl+t Ctrl+w",
            "Ctrl+q Tab",                 # no digit, whatever comes first,
            '"g" Enter',                  # and neither are these
            "Tab Ctrl+q",                 # but characters:
            "Enter",                      # the one of Enter, as it is,
            "Enter",                      # with Ctrl-Q or without
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
            "Down Down",                  # data.bin: 64 lines of 16
            "F4",
            "F4",
            "PgDn",                       # to the last line of the window
            "PgDn",                       # a page
            "PgDn",                       # as far as there is one: the
            "PgDn",                       # line behind the last byte
            "PgUp",
            "PgUp",
            "PgUp",
            "PgUp",
            "Ctrl+End",
            "Up End",                     # the last byte of a line,
            "Down",                       # and below it a line of none
            "Esc",
        )),

    "editor-search": Scenario(
        "The editor looks for a text, and in hex for bytes",
        steps=(
            "Down Down Down Down Down",   # onto notes.doc, three lines
            "F4",
            "Shift+F7",                   # nothing to look for yet
            "F7",
            "Esc",                        # and nothing given
            "F7",
            '"line" Enter',               # behind the first of three
            "Shift+F7",
            "Shift+F7",
            "Shift+F7",                   # no fourth
            "Esc",
            "F4",                         # in hex
            "Ctrl+Home",
            "F7",
            '"4c" Space "69" Enter',      # two bytes, among the digits
            "Shift+F7",
            "F7",
            '"two" Enter',                # a text, among the characters
            "Shift+F7",                   # not twice
            "Esc",
            "F7",
            '"three" Enter+Esc',          # Esc is waiting when the search
                                          # begins, and stops it
            "Esc",
        )),

    "editor-buffer": Scenario(
        "A line that the editor keeps a part of, and more typed and put "
        "into that part than it holds",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Shift+F4",                   # a new file
            '"buffer.txt" Enter',
            "Enter",
            '"0123456789abcdef"',
            # The line cut, put back twice and the two joined: twice as
            # long, and that eight times. 4096 characters, of which the
            # editor keeps 3900, and has room for 4000.
            *("Ctrl+y Ctrl+u Ctrl+u Up Up End Del "
              "Ctrl+y Ctrl+u Ctrl+u Up Up End Del",) * 4,
            'Ctrl+Home "ABCDEFGHIJKLMNOP"',
            # Those sixteen cut and put back six times: 3996.
            "Ctrl+w " + " ".join(["Ctrl+u"] * 6),
            '"wxyz!"',                    # the fifth is one too many
            # 3999, and no room for the two that end a line.
            " ".join(["Ctrl+u"] * 6) + ' "ab" Enter',
            " ".join(["Ctrl+u"] * 7),     # nor for sixteen, the seventh
                                          # time
            "End Del Right",              # the last character it keeps
            "Home Del End",               # one less, and to the end
            "Home Space End Ctrl+Left",   # a word of 3899 characters back
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
        )),

    "editor-dirinfo": Scenario(
        "The editor in the Info panel, on the file dirinfo: a window of "
        "36 columns without a status line",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "Ctrl+l",                     # files in the right panel, and
            "F9 l i",                     # the Info in the left one
            "F4",
            '"ab" Space "cd" Space "ef" Space "gh" Space "ij" Space',
            '"kl" Space "mn" Space "op" Space "qr" Space "st" Space',
            '"uv" Space "wx" Space "yz" Space "0"',   # 40 characters
            " ".join(["Ctrl+Left"] * 12), # back to the first word that is
                                          # shown: one column more
            "Ctrl+Left",                  # a word before that
            "Ctrl+Left",                  # and the first
            "End " + " ".join(["Enter"] * 12),    # eleven lines are shown
            "F4 Up",                      # no hex in so narrow a window
            "Ctrl+Home",
            "F2",
            "Esc",                        # the Info shows the text
        )),

    "editor-big": Scenario(
        "The editor on a file of 87K, on one that it has no room for, and "
        "on one that is shorter than its directory entry says",
        steps=(
            "F9 o n",                     # the mini status off: it would
                                          # show the time of a file made
            "Shift+F4",
            '"a:\\freedos\\bin\\command.com" Enter',    # 87,772 bytes
            "F7",
            '"open" Space "watcom" Space "c" Enter',  # more than 32K in
            "Shift+F7",                   # and once more, 50 bytes on
            "Esc",
            # Four times that file and country.sys behind them: 381,338
            # bytes, within 64K of what the editor has room for.
            "Shift+F5",
            '"a:\\freedos\\bin\\command.com" Tab "b"',
            "Enter",
            *("Shift+F5",
              '"a:\\freedos\\bin\\command.com" Tab "b"',
              "Enter",
              "p") * 3,
            "Shift+F5",
            '"a:\\freedos\\bin\\country.sys" Tab "b"',
            "Enter",
            "p",
            "End",                        # onto it
            "F4",                         # too large
            "Enter",                      # view it, then
            "Esc",
            "F4",
            "Esc",                        # or not
            "Ctrl+PgUp",
            "Down Enter",                 # C:\\STAMPS
            "Down Down Down",             # night.dat says 9 bytes and
            "F4",                         # has none
            "Esc",
            "Down Down",                  # noon.dat says 1 MB: too large
            "F4",                         # by more than that
            "Right Enter",                # Cancel
        )),

    "find": Scenario(
        "Looking for files by name and by what they contain, with keys "
        "that a search finds waiting when it begins",
        steps=(
            "Alt+F7",
            'Tab "hello" Enter',          # of C:\\WORK, the one that has it
            "1 n",                        # a key that means nothing, and
                                          # New search
            # Keys written as one name go down together, and those behind
            # the Enter are waiting when the search first looks for a key:
            # Home, so that the list does not follow its end.
            '"a:\\*.*" Tab Ctrl+y Enter+Home',    # more than the box shows
            "Home PgDn Up",               # the list a line down
            "Down Down",
            "End Down",
            "PgUp",
            "Home",
            "n",                          # the last of three files has it:
                                          # Home comes before the first line
            '"a:\\freedos\\bin\\*.bat" Tab "startup" Enter+Home',
            "n",
            # What the menu of a search that runs takes, a letter that is
            # no answer, and F1: the help, over the search.
            '"a:\\freedos\\bin\\command.com" Tab "qzqz" '
            'Enter+Right+Left+Space+Backspace+x+F1',
            "c+Esc",                      # Cancel leaves the help, and Esc
                                          # is waiting: stopped
            "n",
            "Enter+F1",                   # once more,
            "Ctrl+Esc+c",                 # and Ctrl+C stops it
            "n",
            "Enter",                      # to its end: a file read in
                                          # three parts
            "q",                          # Quit
            "Alt+F7",
            '"hello.*" Tab Ctrl+y Enter',
            "Enter",                      # Go to: the cursor on the file
        )),

    "delete": Scenario(
        "Files and directories deleted: read-only ones, a directory with "
        "files in it, the current one from the tree, by a mask, and what "
        "cannot be deleted",
        steps=(
            "Down Down Down",             # readme.txt
            "F8",
            "Enter",                      # gone
            "Ctrl+a Space Enter",         # hello.txt read-only
            "F8",
            "Enter",                      # which is asked about
            "Right Enter",                # Skip
            "F8",
            "Enter",
            "Right Right Enter",          # Cancel
            "F8",
            "Enter",
            "Enter",                      # Delete
            'F5 "docs\\" Enter',          # notes.doc and data.bin into DOCS
            "Down",
            'F5 "docs\\" Enter',
            "Down",
            'F5 "src\\" Enter',           # and run.bat into SRC
            "Home Down Enter",
            "Down",
            "Ctrl+a Space Enter",         # DOCS\\notes.doc read-only
            "Ctrl+PgUp",                  # the cursor on DOCS
            "F8",
            "Enter",                      # it is not empty
            "Right Right Enter",          # Cancel: nothing is deleted
            "F8",
            "Enter",
            "Enter",                      # Delete: DOS deletes no file of
                                          # it, the first being read-only,
                                          # and that one is asked about
            "Right Right Enter",          # Cancel: that stops it all
            "F8",
            "Enter",
            "Right Enter",                # All
            "Right Enter",                # Skip: data.bin goes, and the
                                          # directory stays
            "F8",
            "Enter",
            "Enter",
            "Enter",                      # Delete: both are gone
            "F9 r t",                     # the tree, which leaves its file
            "Tab",                        # on the disk
            "Ctrl+r",                     # read again: the file is marked
                                          # as out of date first, and the
                                          # byte written says that this
                                          # is a tree
            "Down",                       # SRC, which the files follow to:
                                          # it is the current directory
            "F8",
            "Enter",
            "Enter",                      # deleted all the same, from its
                                          # parent, and the tree's file is
                                          # marked as out of date
            "Tab",
            "Shift+F8",                   # by a mask
            '"nosuch\\*.txt"',            # in a directory that is not there
            "Enter",
            "Esc",
            "Shift+F8",
            # No file is like this either, and nothing says so: a mask
            # with a drive and no directory. The files are deleted by a
            # call of DOS that leaves the carry flag as it was, the flag
            # was set by comparing the colon with a backslash, and Intr21
            # takes it for an error of DOS.
            '"c:*.zzz"',
            "Enter",
            "Shift+F8",
            '"nul"',                      # a device: DOS finds it, and
            "Enter",                      # cannot delete it
            "Esc",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of what is made
            "Ctrl+PgUp",                  # C:\\
            'F7 "new" Enter',             # a directory: the tree has no
                                          # file to mark, and writes one
            "Ctrl+r",                     # which the panel lists when it
            "End",                        # is read again
            "Ctrl+a Space Enter",         # read-only
            'F7 "more" Enter',            # and cannot be marked: a beep
        )),

    "make-directory": Scenario(
        "Directories made from the tree and by a path, those that cannot "
        "be made, and commands that have nothing to work on",
        steps=(
            "F9 o n",                     # the mini status off: it would
                                          # show the time of what is made
            "F9 l t",                     # the tree, on C:\\WORK
            'F7 "new" Enter',             # a directory below that one,
                                          # which the tree then is on
            'F7 "a\\b" Enter',            # no path is taken there
            "Esc",
            "F9 l b",                     # files again
            'F7 "docs" Enter',            # which is there already
            "Esc",
            'Shift+F7 "..\\other" Enter', # by a path: the panel does not
                                          # list it
            "Ctrl+F1",                    # the panel off
            "F8",                         # asks for a mask then
            "Esc",
            "Ctrl+F1",
            "Ctrl+a",                     # on "..": nothing
        )),

    "delete-mask": Scenario(
        "Shift+F8 with a name that no file has, with masks that mean "
        "every file, and on a drive whose disk is not put in",
        steps=(
            "Down Down Down Ins",         # readme.txt selected
            "Ctrl+F8",                    # is for the file under the
            "Enter",                      # cursor all the same: hello.txt
            "Shift+F8",
            '"nosuch.txt"',               # no mask: "Can't find file"
            "Enter",
            "Esc",
            "Shift+F8",
            '"src\\."',                   # all files of a directory
            "Enter",                      # WARNING, with the path
            "Esc",
            "Shift+F8",
            '"*.*"',
            "Enter",                      # "in the current directory"
            "Enter",                      # the five files are gone
            "Shift+F8",
            '"b:*.*"',                    # the floppy drive under its
            "Enter",                      # other letter
            "Enter",                      # "Insert diskette for drive B:"
            "Esc",                        # no: that stops it
            "Shift+F8",
            '"docs\\*.*"',                # an empty directory
            "Enter",
            "Enter",                      # "Can't find files"
            "Esc",
        )),

    "label": Scenario(
        "The volume label: a new one, none, and one that cannot be made",
        steps=(
            "Ctrl+F4",
            '"newlab"',
            "Enter",                      # FreeDOS says nothing, and the
                                          # disk keeps the label it had
            "Ctrl+F4",
            "Ctrl+y Enter",               # no label: asked about
            "Esc",
            "Ctrl+F4",
            "Ctrl+y Enter",
            "Enter",                      # and not got off the disk
            "Ctrl+F4",
            '"work"',                     # the name of a directory of the
            "Enter",                      # root: "Could not set"
            "Esc",
        )),

    "find-drives": Scenario(
        "The file search as a command: left at once, without a name, "
        "over every fixed drive, answered while it runs, and Go to with "
        "nothing found, from a tree and to another drive",
        steps=(
            "Alt+F7",
            "Esc",
            "Alt+F7",
            "Ctrl+y Enter",               # no name: nothing is searched
            "Alt+F7",
            '"*:*.hlp" Enter',            # C: and D:, from their roots
            "n",                          # New search
            '"nosuch.zzz" Enter',         # 0 files
            "Enter",                      # Go to, with no line to go to
            "Alt+F7",
            # Two keys that go down together: the second is waiting when
            # the search first looks for a key.
            '"hello.*" Enter+q',          # Quit
            "Alt+F7",
            "Enter+Enter",                # Stop: nothing found yet
            "n",
            '"*.txt" Enter',
            "Enter",                      # Go to: the cursor on the file
            "Ctrl+l",                     # two panels of files,
            "F9 l t",                     # and the left one the tree
            "Alt+F7",
            '"*.doc" Enter',
            "Up Enter",                   # Go to the directory: the other
                                          # panel shows it
            "Alt+F7",
            '"a:\\*.bat" Enter',          # another drive
            "Enter",                      # the other panel on the file,
                                          # and the tree read from A:
        )),

    "attributes-directory": Scenario(
        "Dates that are refused or need working out, the attributes of "
        "what is in a directory, and of a file on a disk that cannot be "
        "written",
        steps=(
            "Down Down Down",             # readme.txt
            "Ctrl+a",
            "Tab Tab Tab Tab",            # to the date
            '"5"',                        # a month and nothing else
            "Enter",                      # refused
            "Esc",
            'Ctrl+y "2-29-00"',           # a year of two digits below 80,
            "Enter",                      # and a leap year: 2000
            'F5 "docs\\" Enter',          # a copy in DOCS
            "Home Down",
            "Ctrl+a",                     # a directory: what to set and
            "Space",                      # what to clear. Read only, set
            "Enter",                      # for every file in it
            "Enter",
            "Down",
            "F8",                         # as deleting it shows
            "Enter",
            "Right Right Enter",
            "Alt+F1 a",                   # the floppy, write-protected
            "Ctrl+a",
            "Space",
            "Enter",                      # DOS's critical error, in a box
            "Esc",                        # Abort: nothing more is said
        )),

    "attributes-russia": Scenario(
        "A date set where the day comes first, in a month after July",
        country="ru",
        steps=(
            "Down Down Down",
            "Ctrl+a",
            "Tab Tab Tab Tab",
            '"31.8.99"',
            "Enter",
        )),

    "dialog-keys": Scenario(
        "The keys of a dialog: from field to field by every key that "
        "goes to one, round a group and a line of buttons, and F10",
        steps=(
            "F9 o Enter",                 # Configuration: on the colours
            "x",                          # a key that means nothing there
            "Space",                      # on the one chosen: the next
            "Space",                      # round to the first
            "Space",
            "End",                        # the buttons
            "Home",                       # the first field, its first line
            "Down Down Down",             # past its last: the next group
            "Up",                         # back, on the last line
            "Right",                      # the box next to it
            "x Space",                    # no key there either; ticked off
            "Left",                       # the colours, the first line
            "Down Down Home",
            "PgDn",                       # the buttons
            "Shift+Tab",                  # the field before them
            "Alt+F10 Space",              # nothing; ticked
            "End",                        # the buttons, the first of them
            "Left",                       # round to the last
            "Right",                      # and to the first
            "End",                        # the last
            "Home",                       # the first field
            "Shift+Tab",                  # round to the buttons
            "1",                          # a key that means nothing
            "Tab",                        # round to the first field
            "F10",                        # as Esc: nothing is changed
            "Ctrl+l",                     # the right panel one of files
            "F9 r l",                     # its filter, in the right half
            "Tab Tab Tab",                # the buttons there
            "Esc",
            "Down Down Down F5",          # a copy: the tree is F10 here
            "Left",                       # at the start of the field
            '"ab" Left',                  # in it
            "F10",                        # the tree, to choose from
            "Down Enter",                 # a directory: its path typed
            "Alt+F10",                    # the tree again
            "Esc",                        # nothing chosen
            "Esc",
        )),

    "copy": Scenario(
        "A file copied: where the name it is given leads, onto a file "
        "that is there, onto one that is read-only, onto itself, to a "
        "device and from one",
        steps=(
            "Ctrl+l",                     # the right panel in C:\\
            "Down Down Down",             # onto readme.txt
            "F5",                         # offers the other directory
            "Enter",
            "F5",
            '"docs" Enter',               # a directory, named without \\
            "F5",
            '"nul" Enter',                # a device
            "F5",
            '"con" Enter',                # the console: refused
            "Enter",
            "F5",
            '"*.bak" Enter',              # a mask: readme.bak
            "F5",
            '"readme.bak" Enter',         # which is there now: asks
            "Enter",                      # overwrite
            "F5",
            '"readme.bak" Enter',
            "p",                          # append
            "F5",
            '"readme.bak" Enter',
            "s",                          # skip
            "F5",
            '"readme.bak" Enter',
            "Esc",
            "F5",
            '"readme.txt" Enter',         # onto itself
            "Enter",
            "F5",
            '"c:readme.txt" Enter',       # the same with its drive
            "Esc",
            "F5",
            '"\\work\\readme.txt" Enter', # and by its path
            "Enter",
            "F5",
            '".." Enter',                 # the directory above, where it
            "Enter",                      # is already: overwrite
            "End",                        # readme.bak
            "Ctrl+a",
            "Space Enter",                # read-only
            "Home",
            "Down Down Down",
            "F5",
            '"readme.bak" Enter',
            "p",                          # append: asks once more
            "Right Enter",                # no
            "F5",
            '"readme.bak" Enter',
            "p",
            "Enter",                      # yes: it stays read-only
            "F5",
            '"readme.bak" Enter',
            "Enter",                      # overwrite: asks once more
            "Right Enter",                # no
            "F5",
            '"readme.bak" Enter',
            "Enter",
            "Enter",                      # yes
            "Ins Ins",                    # two files into DOCS, where
            "F5",
            '"docs" Enter',               # the first is already
            "a",                          # all
            "Shift+F5",                   # a file named by hand
            '"nul" Tab "x"',              # a device that gives nothing
            "Enter",
            "Shift+F5",
            '"con" Tab "x" Enter',        # the console: refused
            "Enter",
        )),

    "move": Scenario(
        "A file renamed or moved: onto itself, onto a file that is "
        "there, onto a directory, to another drive, and one that is "
        "read-only",
        steps=(
            "Down Down",                  # onto SRC, which is empty
            "F6",
            '"d:\\" Enter',               # made there and removed here
            "F6",                         # readme.txt: offers its name
            "Enter",                      # onto itself: nothing
            "F6",
            '"first.txt" Enter',          # renamed
            "F6",
            '"hello.txt" Enter',          # onto a file that is there
            "Enter",                      # overwrite
            "F6",
            '"notes.doc" Enter',
            "p",                          # append
            "F6",
            '"data.bin" Enter',
            "s",                          # skip
            "F6",
            '"data.bin" Enter',
            "a",                          # all
            "Down",                       # run.bat
            "Ctrl+a",
            "Space Enter",                # read-only
            "Up",
            "F6",
            '"run.bat" Enter',
            "Enter",                      # overwrite: asks once more
            "Right Enter",                # no
            "F6",
            '"run.bat" Enter',
            "Enter",
            "Enter",                      # yes
            "F5",
            '"docs.txt" Enter',
            "Home Down Down",             # onto docs.txt
            "F6",
            '"*." Enter',                 # DOCS, which is a directory
            "Esc",
            "F5",                         # a copy named so: asks whether
            '"*." Enter',                 # to overwrite it
            "Enter",                      # and cannot
            "Enter",
            "F5",
            '"con" Enter',                # the console, and cancelled
            "Esc",
            "F5",
            '"nul." Enter',               # a device with a full stop
            "End",                        # empty.dat
            "Ctrl+a",
            "Down Space Enter",           # no attribute at all
            "Home Down Down",
            "F6",
            '"empty.dat" Enter',          # appended to that: taken for a
            "p",                          # name too long
            "Enter",
            "F6",
            '"nodir\\x" Enter',           # no such directory
            "Esc",
            "F6",
            '"d:\\" Enter',               # to another drive
            "Ctrl+a",                     # run.bat
            "Space Enter",                # read-only
            "F6",
            '"d:\\" Enter',               # copied; asks before deleting
            "s",                          # skip: it stays
            "F6",
            '"d:\\" Enter',               # it is there now, read-only
            "Enter",                      # overwrite
            "Enter",                      # yes
            "Esc",                        # not deleted here
            "F6",
            '"d:\\" Enter',
            "Enter",
            "Enter",
            "Enter",                      # deleted
            "Shift+F5",                   # the help file ends with the
            '"d:\\vc\\vc.hlp" Tab "h"',   # character for the end of a
            "Enter",                      # file
            "Shift+F5",
            '"d:\\vc\\vc.mnu" Tab "h"',   # appended to it: over that
            "Enter",                      # character
            "p",
            "F9 l t",                     # the tree in the left panel
            "F6",                         # asks for a name only
            '"work2" Enter',
            "Enter",
            "Enter",
            "Enter",
        )),

    "copy-ask": Scenario(
        "What Copy and Rename ask before anything is copied: for the "
        "file under the cursor, for the selected ones, for a source "
        "that is typed, with the panels off, and with the tree to "
        "choose the target from",
        steps=(
            "F5",                         # on ..: nothing
            "Ctrl+F5",                    # nor for that one entry
            "Down Down Down",             # onto readme.txt
            "Ctrl+F6",                    # offers its own name
            '"r.txt" Enter',
            "Ins Ins",                    # two files selected
            "Ctrl+F5",                    # the file under the cursor all
            '"one.txt" Enter',            # the same
            "F5",                         # the two
            "Esc",
            "F6",                         # offers the last target
            "Ctrl+y Enter",               # none: nothing is done
            "Shift+F6",
            "Esc",
            "Shift+F5",
            '"*.zzz" Tab "x" Enter',      # no such files
            "Enter",
            "Tab",                        # the Info is the active panel
            "F5",                         # asks for the source too
            "Esc",
            "Tab",
            "Esc",                        # the panels off
            "F5",                         # the same
            "Esc",
            "F6",
            "Esc",
            "Esc",                        # and on
            "F9 r t",                     # the tree next to the files
            "F5",                         # offers its directory
            '"d:" F10',                   # the tree of the drive typed
            "Home Enter",                 # its root
            "Enter",                      # the two files go there
            "F5",
            "F10",                        # the drive of what is offered
            "Esc",
            "Alt+F10",                    # the current drive
            "Down Enter",
            "Tab Right Enter",            # the button
            "Esc",
            'Home Ctrl+y "z:" F10',       # no such drive: the current one
            "Esc",
            "Esc",
        )),

    "copy-tree": Scenario(
        "A directory of a tree panel copied and renamed, and the "
        "current directory of DOS taken along",
        steps=(
            "F9 o c",
            "Tab Tab Tab Space Enter",    # Auto change directory off
            "F9 l t",                     # the tree in the left panel
            "Home",                       # the root
            "F5",                         # is not copied
            "F6",                         # nor renamed
            "Down Down",                  # DOCS
            "F5",
            '"docs2" Enter',              # copied with what is in it
            "Down",                       # SRC
            "F5",                         # offers the last target
            '"x2" Enter',
            "F6",
            '"src1" Enter',               # renamed; the cursor goes to
            "Down",                       # the current directory: DOCS
            "F6",
            '"a\\b" Enter',               # a path is no name
            "Enter",
            "F6",
            "Esc",
            "Down Down Enter",            # DOCS2 is the current directory
            "Up Up",                      # DOCS, whose path that of DOCS2
            "F6",                         # begins with
            '"x" Enter',                  # the current one is X2 now
            "Home Down",                  # WORK
            "F6",
            '"work2" Enter',              # the current one follows
        )),

    "copy-dirs": Scenario(
        "Directories copied, renamed and moved, with what is in them, "
        "and names too long for it",
        steps=(
            "Down Enter",                 # into DOCS
            'F7 "subdir.dir" Enter',      # the time of day, in the first
            "Ctrl+PgUp",                  # minute
            "Down Down",                  # onto readme.txt
            "F5",
            '"docs\\" Enter',
            "Up Up",                      # onto DOCS
            "F5",
            '"hello.txt" Enter',          # a file of that name: an error
            "Enter",
            "F5",
            '"nodir\\x" Enter',           # no such directory
            "Enter",
            "F5",
            '"nul" Enter',                # its files to a device
            "F5",                         # into SRC by 64 characters:
            '"' + ".\\" * 30 + 'src\\"',  # what is in it gets a name
            "Enter",                      # too long
            "Enter",
            "Enter",
            "F6",                         # the same moved: the directory
            '"' + ".\\" * 30 + 'src\\"',  # is there, so what is in it is
            "Enter",                      # renamed one by one
            "Enter",
            "Enter",
            "F6",
            '"papers" Enter',             # renamed
            "F5",
            '"src\\" Enter',              # into SRC, with what is in it
            "F5",
            '"src\\" Enter',              # again: the file is there
            "Enter",
            "F6",
            '"src\\" Enter',              # moved there: file by file
            "s",                          # skip: the directory stays
            "F6",
            '"src\\" Enter',
            "Enter",                      # overwrite: it goes
            "F5",
            '"d:\\" Enter',               # SRC to another drive
            "F6",
            '"d:\\" Enter',               # moved: the file is there
            "s",                          # skip: PAPERS stays, and its
                                          # file is then taken for one of
            "Enter",                      # SRC, where there is none
            "F6",
            '"d:\\" Enter',
            "Enter",                      # overwrite: all of it goes
        )),

    "copy-big": Scenario(
        "A file that is read in several pieces, and one that the memory "
        "does not hold",
        steps=(
            "Shift+F5",
            '"a:\\freedos\\bin\\command.com" Tab "big"',
            "Enter",
            "End",                        # onto it
            "F5",
            '"huge" Enter',
            *("F5", "Enter", "p") * 5,    # itself appended five times
            "End",
            "F5",
            '"huge2" Enter',              # read and written in two parts
            "F5",
            '"huge2" Enter',
            "s",                          # skip: not all of it was read
        )),

    "copy-full": Scenario(
        "A disk that gets full while a file is copied to it",
        steps=(
            "Shift+F5",
            '"a:\\freedos\\bin\\command.com" Tab "a"',
            "Enter",
            "End",                        # onto it
            "F5",
            '"b" Enter',
            # Each appended to the other in turn, seven times: b is 3 MB
            # then, and the disk has 28 free.
            *("F5", '"b" Enter', "p", "Down",
              "F5", '"a" Enter', "p", "Up") * 3,
            "F5",
            '"b" Enter',
            "p",
            "Down",                       # onto b
            # Nine copies of it. No step moves more than 3 MB, to keep
            # the test quick on a machine that others use too.
            *(step for number in range(1, 10)
              for step in ("F5", f'"c{number}" Enter')),
            "F5",
            '"d" Enter',                  # the tenth: the disk is full
            "Right Enter",                # go on: what is there of d is
            "Enter",                      # in the way, overwrite it;
                                          # full again
            "Enter",                      # abort: d is deleted
            "Ctrl+a",
            "Space Enter",                # b read-only, and so its copies
            "F5",
            '"d" Enter',                  # full
            "Enter",                      # abort: what is there of d is
                                          # deleted, read-only as it is
            "F5",
            '"docs\\d" Enter',            # full once more, in a directory
            "Right Enter",                # go on
            "s",                          # skip: the part stays
            "Shift+F5",                   # nothing is free now
            '"hello.txt" Tab "x"',
            "Enter",                      # not a byte written
            "Enter",                      # abort
            "Shift+F5",
            '"hello.txt" Tab "x"',
            "Enter",
            "Right Enter",                # go on: the same again
            "Enter",
        )),
    "mouse-panel": Scenario(
        "The mouse on a panel of files: the left button puts the cursor "
        "on a file, the right one selects it",
        steps=(
            "Click@3,5",                  # the cursor onto readme.txt
            "Click@3,8",                  # and onto data.bin
            "RightClick@3,6",             # hello.txt selected
            "RightClick@3,6",             # and not
            "Move@40,12",                 # the pointer by itself
        )),

    "mouse-list": Scenario(
        "The mouse at the panels otherwise: in the corner, which switches "
        "them off and on, above and below a list, which move the cursor "
        "that way while the button is held, twice on a directory, which "
        "is Enter, in the other panel, and in the lower part of an Info, "
        "which edits dirinfo",
        steps=(
            "Click@0,0",                  # the corner: the panels off
            "Click@0,0",                  # and on
            "Click@3,15",                 # below the last file: onto it
            "Click@3,1",                  # the titles: up, to the top
            "Click@3,21",                 # the status line: down again
            "Click@3,3",                  # DOCS
            "Click@3,3",                  # at once again: into it
            "Click@45,5",                 # the Info is the current panel
            "Click@60,5",                 # its upper part: nothing
            "Click@60,15",                # its lower part: a dirinfo
            "Esc",
            "Click@3,5",                  # the left panel again
        )),

    "mouse-bars": Scenario(
        "The mouse on the key bar, on the top line, which brings up the "
        "menu bar, and on an item of a menu",
        steps=(
            "Click@1,24",                 # F1 of the key bar: the help
            "Click@45,20",                # its button Cancel
            "Click@10,0",                 # the top line: the Files menu
            "Click@12,3",                 # its item User menu
            "Click@60,12",                # beside the menu: it closes
        )),

    "mouse-dialog": Scenario(
        "The mouse on the buttons of a dialog",
        steps=(
            "Click@3,5",
            "Click@45,24",                # F6 of the key bar: Rename
            "Click@55,10",                # its button Cancel
            "Click@37,24",                # F5: Copy
            "Click@40,10",                # the button in the middle
        )),

    "keys-waiting": Scenario(
        "A key that waits behind another: taken from the keyboard and "
        "lost where the buffer is cleared, left there by a copy that it "
        "does not stop, and F10, which stops one",
        steps=(
            "Down Down Down",             # onto a file
            "F4",                         # the editor clears the buffer
            "Right+Right",                # after a key: one column, not two
            "Esc",
            "F5",
            '"zz"',
            "Enter+a",                    # no key that stops a copy: it
                                          # stays, for the command line
            "Backspace",
            "F5",
            '"zy"',
            "Enter+F10",                  # stops it: no zy
        )),

    "drive-b": Scenario(
        "The disk of a drive that shares the floppy drive with another, "
        "put in when the program asks for it: B:, and A: again",
        steps=(
            "Shift+F3",
            '"b:fdauto.bat"',
            "Enter",                      # "Insert diskette for drive B:"
            "Enter",                      # Ok: the file of that disk
            "Esc",
            "Shift+F3",
            '"a:fdauto.bat"',
            "Enter",                      # and for drive A:
            "Enter",
            "Esc",
        )),

    "mouse-left-hand": Scenario(
        "The mouse for a left hand: the two buttons the other way round",
        steps=(
            "F9 o c",                     # the Configuration
            "Click@40,15",                # Left-handed mouse, ticked
            "Enter",
            "RightClick@3,5",             # the cursor onto readme.txt
            "Click@3,6",                  # hello.txt selected
            "Click@3,8",                  # and data.bin
        )),

    "mouse-select": Scenario(
        "Files selected with the right button of the mouse held: below "
        "a list and above it, where the list moves under it, and on a "
        "file",
        steps=(
            "F9 o f",                     # panels half the screen high
            "Alt+F1 a",
            "Down Down Down Enter",       # A:\\FREEDOS
            "Down Enter",                 # and its BIN: 44 entries
            "F9 l f",                     # in one column, ten at a time
            "RightClick@3,3",             # a file: selected
            "RightClick@3,12*12",         # held below the list: it moves,
                                          # and what comes is selected
            "RightClick@3,1*8",           # held above it: back again
            "RightClick@3,5*40",          # held on a file that is
                                          # selected: not any more, once
        )),

    "mouse-fields": Scenario(
        "The mouse on the fields of a dialog: a line of a group, a box to "
        "tick, a click on no field and from the buttons to a field, and "
        "the right button for Enter",
        steps=(
            "F9 o c",                     # Configuration
            "Click@18,5",                 # a line of a group: Black & White
            "Click@18,5",                 # the line that is chosen: nothing
            "Click@43,5",                 # a box that is ticked: it is not
            "Click@43,5",                 # and is again
            "Click@30,17",                # in the box, on no field: nothing
            "Click@18,13 Click@18,14",    # two lines of the other group
            "PgDn",                       # the buttons
            "Click@18,7",                 # from them to a group: Laptop
            "RightClick@30,17",           # the right button is Enter
        )),

    "mouse-line": Scenario(
        "The mouse in a line that is typed into: a click puts the cursor "
        "there, no further than the text goes, and what is typed then "
        "does not replace the text",
        steps=(
            'F7 "abcdef"',
            "Click@10,8",                 # into the text
            "Click@10,8",                 # where the cursor is: nothing
            "Click@40,8",                 # behind the text: to its end
            "x",
            "Click@8,8 y",                # at its start
            "Click@40,10",                # in the box, below the line
            "RightClick@40,10",           # Enter: the directory is made
            "Down Down Down Down",
            "F6",                         # offers the name
            "Click@10,8 x",               # a click, and the name is kept
            "Click@40,3",                 # outside the box: Esc
        )),

    "mouse-filter": Scenario(
        "The mouse in a dialog of the right half of the screen, the "
        "filter of the right panel: from its buttons to a field, a line "
        "that is full, and its button Ok",
        steps=(
            "Ctrl+l",                     # files on the right
            "F9 r l",                     # their filter
            '"abcdefgh.xyz"',             # as long as the line
            "Click@57,8",                 # its first character
            "Click@70,8",                 # behind the line: no field
            "Click@68,8",                 # its last character
            "PgDn",                       # the buttons
            "Click@57,12",                # between two of them: nothing
            "Click@50,10",                # from them to a box to tick
            "PgDn Right",                 # Cancel
            "Click@60,8",                 # from there to the line
            "Click@62,12",                # Cancel, by a click
            "F9 r l",
            "Click@50,10",                # Executable files
            "Click@52,12",                # Ok
            "F9 r l",
            "Click@50,10",                # and not
            "PgDn Right",                 # Cancel
            "RightClick@50,7",            # the right button is Enter, on
                                          # the button that is chosen
        )),

    "mouse-keys": Scenario(
        "The mouse on the key bar with the right button, on a key without "
        "a name, with the bar off, and in the corners of the screen: the "
        "top right one blanks it",
        steps=(
            "RightClick@1,24",            # Alt+F1: the drive of the left
            "Esc",
            "Click@1,24*2",               # let go at once: nothing
            "F7",
            "Click@12,24",                # F2 has no name there: a click
                                          # outside the box
            "Ctrl+b",                     # no key bar
            "Click@12,24",                # the last row is no key
            "Ctrl+b",
            "Move@79,24",                 # the corner that keeps the
                                          # screen from being blanked
            "Shift+F3",
            '"nofile" Enter',             # a beep: the screen moves up
            "Esc",                        # and stays there
            "Move@79,0 Move@40,12",       # blanked and shown again, all
                                          # of it as it should be
        )),

    "mouse-menu": Scenario(
        "The mouse on the menu bar: beside its names, on a name with "
        "another menu open, on an item that is dark, the right button "
        "on an item of the Files menu, and Alt, Ctrl and Shift there",
        steps=(
            "Click@70,0",                 # the top line beside the names:
                                          # the bar while the button is down
            "Click@10,0",                 # on a name: the Files menu
            "Click@25,0",                 # on another: Commands instead
            "Click@30,12",                # an item that is dark: none
            "Click@25,0",
            "Click@70,0",                 # beside the names: no menu
            "F9",                         # the bar by its key
            "Click@40,12",                # a click below it: gone
            "Down Down Down",             # readme.txt
            "Click@10,0",
            "RightClick@12,6",            # Copy with the right button, as
            "Esc",                        # with Shift: Shift+F5
            "F9 f",
            "Alt",                        # Alt by itself closes the menu
            "f Down Down Down Down",      # to Copy again
            "Ctrl+Enter",                 # with Ctrl held: Ctrl+F5
            "Esc",
            "F9 f Down Down Down Down",
            "Shift+Enter",                # with Shift held
            "Esc",
        )),

    "mouse-user-menu": Scenario(
        "The mouse in the user menu: a click on a line, a second one "
        "that comes late and one that comes in time, the right button, "
        "and clicks beside the lines and outside the box",
        steps=(
            "F2",
            "Click@35,8",                 # a line: the cursor goes there
            _A_WHILE,
            "Click@35,8",                 # again, late: nothing more
            "Click@40,6",                 # the frame of the box: nothing
            "RightClick@35,3",            # the right button outside it:
                                          # nothing either
            "RightClick@35,9",            # and on a line: the cursor
            "Click@1,24",                 # F1 of the key bar is the help,
                                          # where the key is the first line
            "Esc",
            "Click@35,7 Click@35,7",      # twice on a line: its command,
                                          # another menu
            "Click@35,11",                # the last line there
            "Click@35,11",                # and once more, at once: its
                                          # command, the main menu
            "Click@35,3",                 # outside the box: it is gone
            "F2",
            "Click@75,24",                # F10 of the key bar leaves, where
                                          # the key would be a line's
        )),

    "mouse-history": Scenario(
        "The mouse in the history of commands",
        steps=(
            '"ver" Enter',
            '"echo" Space "one" Enter',
            '"echo" Space "two" Enter',
            "Alt+F8",
            "Click@35,10",                # a line: the cursor goes there
            "Click@35,9",                 # the frame: nothing
            "RightClick@35,11",           # the right button on a line
            "RightClick@35,3",            # and outside the box: nothing
            "Click@35,12 Click@35,12",    # twice on a line: it is run
            "Ctrl+o",                     # as this shows
            "Ctrl+o",
            "Alt+F8",
            "Click@35,3",                 # outside the box: it is gone
        )),

    "mouse-drive": Scenario(
        "The mouse in the box that asks for the drive of a panel",
        steps=(
            "Alt+F1",
            "Click@12,7",                 # in the box, on no letter
            "Click@18,8",                 # the letter of the cursor: it
                                          # is chosen
            "Alt+F1",
            "Click@14,8",                 # another letter: A
            "Alt+F1 Right",               # the cursor on C
            "RightClick@30,15",           # the right button is Enter,
                                          # wherever it is
            "Alt+F1",
            "Click@30,15",                # outside the box: Esc
        )),

    "mouse-memory": Scenario(
        "The mouse in the map of the memory: either button on a line, "
        "two clicks on a program that can be released and on one that "
        "cannot, its buttons, and clicks beside all of them",
        steps=(
            '"a:\\freedos\\bin\\fdapm" Space "apmdos" Enter',
            "Alt+F5",
            "Click@30,8",                 # a line: the cursor goes there
            "Click@30,17",                # below the last line: onto it
            "RightClick@30,7",            # the right button on a line
                                          # that cannot be released
            "RightClick@30,14",           # and on the program: selected
            "RightClick@30,14",           # and not
            "Click@30,4",                 # in the box, on no line: nothing
            "RightClick@30,2",            # the right button outside it:
                                          # nothing
            "Click@30,7 Click@30,7",      # twice on such a line: nothing
            "Tab",                        # a key that means nothing here
            "Click@47,20",                # the button Cancel
            "Alt+F5",
            "Click@30,14 Click@30,14",    # twice on the program: released
            "Alt+F5",                     # gone
            "Click@30,1",                 # outside the box: it closes
        )),

    "mouse-middle": Scenario(
        "The middle button of the mouse: on the key bar the key with "
        "Shift, on an item of the Files menu the item with Ctrl, and "
        "the way out of a dialog, the user menu, the history, the box of "
        "the drives and the map of the memory",
        steps=(
            "Down Down Down",             # readme.txt
            "MiddleClick@37,24",          # F5 of the key bar: Shift+F5
            "MiddleClick@30,8",           # in a dialog, wherever: Esc
            "F2",
            "MiddleClick@35,8",           # on a line of the user menu
            "Alt+F8",
            "MiddleClick@35,12",          # in the history
            "Alt+F1",
            "MiddleClick@14,8",           # on a letter of a drive
            "Alt+F5",
            "MiddleClick@30,8",           # on a line of the map
            "Click@10,0",                 # the Files menu
            "MiddleClick@12,6",           # its Copy: Ctrl+F5
            "MiddleClick@55,10",          # and on the dialog's button
        )),

    "mouse-drag-dialog": Scenario(
        "A button of the mouse that is moved while it is down, in a "
        "dialog, on the key bar and in the box of the drives: it counts "
        "where it is let go",
        steps=(
            'F7 "abcdef"',
            "Click@8,8>12,8",             # along the line: the cursor too
            "Click@40,3>40,10",           # down outside the box and up in
                                          # it: nothing
            "Click@40,10>40,3",           # the other way: Esc
            "Down Down Down",
            'F6 "moved.txt" PgDn',        # a new name, and to the buttons
            "Click@25,10>25,12",          # off the button that is chosen:
                                          # it is not pressed
            "Click@25,10>25,12>25,10",    # off it and on again: it is
            "Click@37,24>37,20",          # off a key of the key bar: no key
            "Click@37,24>45,24>37,24",    # to the next key and back: F5
            "Esc",
            "Alt+F1",
            "Click@14,8>14,6",            # down on a letter, which gets the
                                          # cursor, and up beside it: no
                                          # drive is chosen
            "Click@12,7>18,8",            # down beside the letters and up
                                          # on one: that drive
            "Alt+F1",
            "Click@14,8>14,2",            # up outside the box: Esc
        )),

    "mouse-drag-menu": Scenario(
        "A button of the mouse held and moved along the menu bar and "
        "down a menu, and the bar that is left with no menu",
        steps=(
            "Click@70,0>25,0",            # down beside the names and onto
                                          # one: its menu
            "Click@25,0>40,0>10,0",       # along the bar: each in turn
            "Click@10,0>12,3",            # down to an item: User menu
            "Esc",
            "Click@25,0>30,4>60,15",      # to an item and off the menu:
                                          # none
            "Click@70,0*5>25,0*1",        # let go as soon as it is on a
                                          # name: the bar with no menu
            "Right",                      # and the keys go by that: the
                                          # menu behind none is the first
            "Esc",
        )),

    "mouse-drag-lists": Scenario(
        "A button of the mouse that is moved while it is down, over the "
        "files of a panel, in the user menu and in the map of the memory",
        steps=(
            "RightClick@3,5>3,6>3,7>3,8",  # four files: selected
            "RightClick@3,8>3,7>3,6",     # back over three: not any more
            "F2",
            "Click@35,8 Click@35,8>35,3",  # a second click that comes up
                                          # beside its line: nothing
            "Click@35,7>35,9",            # down the lines: the cursor too
            "Click@35,8>35,3",            # up outside the box: it is gone
            '"a:\\freedos\\bin\\fdapm" Space "apmdos" Enter',
            "Alt+F5",
            "Click@30,14>30,3*4",         # from a line to above the
                                          # lines: the cursor goes up
            "Click@30,8>30,19*6",         # and to below them: down
            "RightClick@30,13>30,14>30,15",  # the right button over the
                                          # program: selected
            "Click@30,14 Click@30,14>30,10",  # a second click that comes
                                          # up on another line: nothing
                                          # is released
            "Click@30,4>47,20",           # from beside the lines onto a
                                          # button: Cancel
            "Alt+F5",
            "Click@30,4>30,1",            # up outside the box: it closes
        )),

    "mouse-help": Scenario(
        "The mouse in the help: a click on a line of the index and a "
        "second one, a button held above the lines and below them, in "
        "the first and the last third of a page, beside the text and "
        "outside the window, the right button and the middle one, and "
        "the help on a screen of 50 lines",
        steps=(
            "F1",
            "Click@20,10",                # the cursor onto a line
            "RightClick@30,12",           # and with the right button
            "Click@20,14*3",              # let go when asked next
            "Click@20,10>20,22*40",       # held below the lines: 20 down
            "Click@20,12>20,3*12",        # and above them: six up
            "Click@20,4",                 # on the title: nothing
            "RightClick@2,2",             # the right one outside: nothing
            "Click@30,9 Click@30,9>75,9", # let go beside the line: nothing
            "Click@30,11 Click@30,11",    # a second click: the page
            "Click@30,16*40",             # the last third: down to the end
            "Click@30,8*10",              # the first third: up
            "Click@30,12",                # between them: nothing
            "Click@30,12>30,21*4",        # held below the text: down
            "Click@30,12>30,2*8",         # and above it: up
            "MiddleClick@30,12",          # the middle button leaves
            "F1 Right",                   # the cursor on Cancel
            "Click@30,9 Click@30,9",      # a second click is Cancel then
            "F1",
            "Click@2,2",                  # a click outside the window
            "Alt+F9",                     # 50 lines
            "Click@1,49",                 # F1 of the key bar
            "Click@20,30",                # a line below the 25th row
            "Click@20,10>20,48*20",       # held below the lines
            "Click@20,43 Click@20,43",    # the page of the last line shown
            "Click@30,12>30,47*8",        # held below its text
            "MiddleClick@70,40",
            "Move@40,40",                 # the pointer on the 41st row
            "Alt+F9",                     # which 25 lines have not
        )),

    "mouse-find": Scenario(
        "The mouse in the list of the file search: a click on a line and "
        "a second one, which goes to the file, a click below the last "
        "line and on a list with none, a button held below the list and "
        "above it, beside the list and outside the box, the right button "
        "and the middle one",
        steps=(
            "Alt+F7",
            '"*.zzz" Enter',              # no file
            "Click@20,8",                 # a click on the empty list
            "n",
            '"*.txt" Enter',              # three lines
            "Click@20,6",                 # the cursor onto the last
            "Click@20,4",                 # and onto the first
            "RightClick@20,12",           # below the last line: the last
            "Click@20,3",                 # on the frame: nothing
            "Click@3,10",                 # beside it in the box: nothing
            "RightClick@1,1",             # the right one outside: nothing
            "Click@1,10",                 # the left one there closes it
            "Alt+F7",
            '"a:\\*.*" Enter',            # 85 lines, the last 13 shown
            "Click@20,8",                 # the cursor onto a line
            "RightClick@30,10",           # and with the right button
            "Click@20,14*3",              # let go before the list asks
            "Click@20,11>20,21*8",        # held below the list: down
            "Click@20,12>20,2*40",        # and above it: the list moves
            "Click@20,7>20,19*40>20,3*4", # down and up without letting go
            "Click@30,9 Click@30,9>75,9", # let go beside the line: nothing
            "MiddleClick@30,12",          # the middle button closes it
            "Alt+F7 Enter",               # the same search again
            "Click@30,9 Click@30,9",      # a second click: Go to
        )),

    "mouse-tree": Scenario(
        "The mouse on the tree, in the box where a directory is chosen "
        "and in a panel: a click on a line and a second one, a button "
        "held below the lines and above them, beside them and outside, "
        "the right button and the middle one",
        steps=(
            *(f'F7 "{letter}" Enter' for letter in "abcdefghijk"),
            "Alt+F10",                    # 17 lines, three more than fit
            "Click@30,8",                 # the cursor onto a line
            "RightClick@30,10",           # and with the right button
            "Click@30,12>30,20*8",        # held below the lines: to the end
            "Click@30,7>30,2*8",          # and above them: to the start
            "Click@30,6>70,10*6",         # beside the lines: nothing more
            "Click@30,3",                 # on the frame: nothing
            "RightClick@1,1",             # the right one outside: nothing
            "Click@30,9 Click@30,9>70,9", # let go beside the line: nothing
            "MiddleClick@30,12",          # the middle button closes it
            "Alt+F10",
            "RightClick@30,8 RightClick@30,8",    # a second click: chosen
            "Alt+F10",
            "Click@1,1",                  # a click outside closes it
            "F9 o f",                     # panels of half the height
            "F9 r t",                     # the tree on the right: 11 lines
            "Click@50,5",                 # a line: the panel is the current
                                          # one, and the files follow
            "Click@50,7 Click@50,7",      # a second click: Enter
            "Click@50,4>50,14*6",         # held below the panel: down
            "Click@50,6>50,0*4",          # and above it: up
            "RightClick@50,8",            # the right button
            "Click@50,9>50,5>50,7",       # the cursor goes with the pointer
            "Click@70,6",                 # beside the name, on its row
            "Tab",                        # the files are the current panel
            "Click@50,5",                 # and the tree again
        )),

    "mouse-view": Scenario(
        "The mouse in the viewer: a button held in the last third of the "
        "window and in the first, which move the text, in the middle, on "
        "the right and on the left of a text that is not wrapped, on the "
        "status line, from there into the window and back, the other "
        "buttons, and the same in hex",
        steps=(
            "Shift+F3",
            '"a:\\freedos\\nls\\setup.en" Enter',     # 7,948 bytes of text
            "Click@40,20*8",              # the last third: down
            "Click@40,3*4",               # the first third: up
            "Click@40,12",                # the middle: nothing
            "Click@75,12*6",              # wrapped, the right: nothing
            "F2",                         # not wrapped
            "Click@75,12*6",              # the right quarter: columns
            "Click@5,12*4",               # the left quarter: and back
            "Click@40,0",                 # on the status line: nothing
            "Click@40,0>40,20*4",         # from there into the window
            "Click@40,12>40,0*4",         # and from the window onto it: up
            "RightClick@40,20*4",         # the right button does the same
            "MiddleClick@40,3*4",         # and the middle one
            "F4",                         # hex
            "Click@40,20*4",              # down
            "Click@75,12*4",              # the right: nothing
            "Click@5,12*4",               # the left: nothing
            "Click@40,2*8",               # up
            "Click@73,24",                # F10 of the key bar
        )),

    "mouse-edit": Scenario(
        "The mouse in the editor: a click that puts the cursor into the "
        "text, behind the end of a line and into the last column, a "
        "button held above the window and below it, the digits and the "
        "characters in hex, the cursor taken behind what the editor "
        "keeps of a long line, and the window of the Info panel",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # the steps that type
            "Enter",
            "Shift+F4",
            '"a:\\freedos\\nls\\setup.en" Enter',
            "Click@10,5",                 # the cursor into a line
            "Click@60,3",                 # and into another
            "RightClick@30,9",            # behind a short line: to its end
            "Click@79,9*6",               # the last column there: nothing
            "Click@78,5",                 # the end of what shows of a line
            "Click@79,5*10",              # its last column: to the right
            "Click@40,12>40,0*12",        # held above the window: up
            "Click@40,12>40,24*40",       # and below it: the text moves
            "MiddleClick@5,20",           # the middle button is one too
            "Click@40,0",                 # on the status line: nothing
            "F4",                         # hex
            "Click@13,5",                 # the second digit of a byte
            "Click@14,5",                 # the space behind it
            "Click@12,5",                 # the first digit
            "Click@4,7",                  # the address: the first byte
            "Click@22,6",                 # the last byte of four
            "Click@23,6",                 # and the space behind it
            '"7"',                        # a digit typed there
            "Click@70,8",                 # a character
            "Click@79,9",                 # behind the characters
            "Click@30,10>30,24*40",       # held below the window
            "Click@30,10>30,0*8",         # and above it
            "Esc",
            "Right Enter",                # not saved
            "Shift+F4",
            '"a:\\kernel.sys" Enter',     # lines of more than 3900
            "End",                        # the last column that keys go to
            # The last column of another line is a column further on each
            # time: with the button down from one line to the other and
            # back, until the cursor is behind what the editor has of
            # the line.
            "Click@79,2*2" + "".join(f">79,{3 - place % 2}*2"
                                     for place in range(90)),
            "Click@79,4",                 # behind that of the next line
            "Click@79,4",                 # and its last column
            "Esc",
            "Shift+F4",
            '"dirinfo" Enter',
            "Enter",                      # a new file
            '"0123456789012345678901234567890123456789012345" Enter',
            " ".join(f'"line{number}" Enter' for number in range(1, 15)),
            "F2 Esc",                     # the Info shows it
            "Click@50,15",                # a click on it there edits it
            "Click@45,14",                # the cursor into a line
            "Click@10,5",                 # outside the window: nothing
            "Click@45,13>45,5*6",         # held above the window: up
            "Click@76,11",                # a line longer than the window
            "Click@77,11*8",              # its last column: to the right
            "Click@45,14>45,23*20",       # held below it: the text moves
            "Esc",
        )),

    "setup-new": Scenario(
        "The setup saved where its file is gone, which makes it anew, and "
        "where the directory of the program is gone, which is an error",
        steps=(
            '"del" Space "d:\\vc\\vc.ini"',
            "Enter",                      # no setup file any more
            "Shift+F9",
            "Enter",                      # saved: a new one
            "Ctrl+l",                     # no Info, which says what D: has
            "Alt+F1 d",                   # D:\, with the directory VC
            "F6",
            '"vx" Enter',                 # under another name
            "Shift+F9 Enter",             # no such path: a box says so
            "Enter",
            "F6",
            '"vc" Enter',                 # and back
        )),

    "blank": Scenario(
        "The screen blanker, brought up by the mouse in the top right "
        "corner and ended by a key, which is lost, by the mouse leaving "
        "the corner, by Shift, and by a button of the mouse after the "
        "mouse has moved about in the corner for a while",
        steps=(
            "Move@79,0 x",                # the sky, and a key: the panels
            "x",                          # that one is typed
            "Esc",
            "Move@40,12",
            "Move@78,0 Move@40,12",       # the sky, and the mouse away
            "Move@79,0 Shift",            # the sky, and Shift
            "Move@40,12",
            # The sky for some twenty turns of it: stars come out.
            "Move@79,0 " + "Move@76,1 Move@79,0 " * 20 + "Click@77,0",
            "x",                          # the panels take keys again
        )),

    "start-setup": Scenario(
        "The program started where VC= in the environment says its files "
        "are: without a setup file, with one that has Snow mode switched "
        "off, which no menu sets, and with one whose sum is wrong; and "
        "without COMSPEC=",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of a file made
            "Ctrl+l",                     # two panels of files, and that
            "Shift+F9 Enter",             # saved: a copy that starts from
                                          # this setup shows no Info
            "Shift+F4",
            '"s.bat" Enter',
            "Enter",
            '"set" Space "vc=c:\\work" Enter '
            '"set" Space "comspec=" Enter "d:\\vc\\vc"',
            "F2 Esc",
            '"s" Enter',                  # no setup file in C:\WORK: the
                                          # defaults, one panel, the right
            "Ctrl+o",                     # the banner and nothing else
            "Ctrl+o",
            "F10",
            "Enter",                      # it has left a setup file there
            "Shift+F5",                   # the setup of D:\VC over that
            '"d:\\vc\\vc.ini" Tab "vc.ini" Enter',
            "Enter",                      # overwrite
            "Shift+F4",
            '"vc.ini" Enter',
            " ".join(["F4"] + ["Down"] * 27 + ["Right"] * 20),  # byte
                                          # 1BAh: Snow mode
            '"00"',                       # off, and the sum wrong by 2
            "Down Left Left",             # byte 1CAh, the first that is
            '"02"',                       # reserved: the sum is right
            "F2 Esc",
            '"s" Enter',                  # the setup as it was saved, and
                                          # the screen tested for snow
            "Ctrl+o",
            "Ctrl+o",
            "F10",
            "Enter",
            "Shift+F4",
            '"vc.ini" Enter',
            " ".join(["F4"] + ["Down"] * 28 + ["Right"] * 20),  # the
            '"03"',                       # reserved byte again: the sum
                                          # is wrong
            "F2 Esc",
            '"s" Enter',                  # the defaults again
            "Ctrl+o",                     # "VC.INI is not correct."
            "Ctrl+o",
            "F10",
            "Enter",                      # and the setup file that it
                                          # leaves is that of the
                                          # defaults, on the disk at the
                                          # end
        )),

    "start-screen": Scenario(
        "The program brought back from a command, and started, with the "
        "screen as it does not want it: 40 columns, graphics, a "
        "monochrome screen, and a cursor of no shape or below the last "
        "line. The commands are programs of a few bytes, typed in hex",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of a file made
            "Ctrl+l",                     # two panels of files, and that
            "Shift+F9 Enter",             # saved: the second copy shows
                                          # no Info
            "Shift+F4",                   # m.com: the video mode that its
            '"m.com" Enter',              # command line names by a digit
            "Enter",
            '"aaaaaaaaaa" F4 Ctrl+Home',       # ten bytes, then in hex
            '"a08200240fb400cd10c3"',
            "F2 Esc",
            "Shift+F4",                   # c.com: a cursor from line 0
            '"c.com" Enter',              # to line 67h, on row 255
            "Enter",
            '"aaaaaaaaaaaaaaaaa" F4 Ctrl+Home',
            '"b401b96700cd10b402b700ba00ffcd10c3"',
            "F2 Esc",
            "Shift+F4",                   # t.bat: the program started on
            '"t.bat" Enter',              # a screen of 40 columns
            "Enter",
            '"m" Space "1" Enter "d:\\vc\\vc"',
            "F2 Esc",
            '"m" Space "1" Enter',        # 40 columns: Enter is asked for,
            "Enter",                      # on two lines
            '"m" Space "6" Enter',        # graphics of 80 columns: on one
            "Enter",                      # line, which the test does not
                                          # see as text
            '"m" Space "7" Enter',        # monochrome: the screen is in
                                          # another place than the test
                                          # looks at, the cursor's shape
                                          # is seen
            "F7",                         # and in a line that is typed
            "Esc",                        # into
            '"m" Space "3" Enter',        # colour again
            '"vcursor" Space "hide" Enter',   # of A:\FREEDOS\BIN: no
                                          # cursor, and it has one again
            '"c" Enter',                  # a cursor of another shape,
                                          # far below the screen: on the
                                          # last line and as it should be
            "Ctrl+o",                     # what DOS had shown, a line
            "Ctrl+o",                     # further up
            '"t" Enter',                  # the second copy sets the mode
            "Ctrl+o",
            "Ctrl+o",
            "F10",
            "Enter",
        )),

    "start-memory": Scenario(
        "The program without the memory for its data when it comes back "
        "from a command: a second copy of it, whose command keeps all "
        "but 75 KB of the memory",
        steps=(
            "F9 o o",                     # Clear keyboard buffer off, for
            "Down Down Down Down Down Space",     # typing in the editor
            "Enter",
            "F9 o n",                     # the mini status off: it would
                                          # show the time of a file made
            "Shift+F4",                   # k.com stays in the memory with
            '"k.com" Enter',              # all of it but 1250h paragraphs
            "Enter",
            '"aaaaaaaaaaaaaaaaa" F4 Ctrl+Home',
            '"8b1602008cc82bd081ea5012b80031cd21"',
            "F2 Esc",
            '"d:\\vc\\vc" Space "k" Enter',   # the second copy runs it,
                                          # cannot come back and ends,
                                          # which releases k.com
            "Ctrl+o",                     # what it said
            "Ctrl+o",
            "Ctrl+PgUp",                  # the first copy still works
        )),

}



def run(scenario, program, bench, trace=False, blinking=False,
        stack=False):
    """Run `scenario` on `program` (a setups.Program) and return the Run.

    `bench` is the setups.Bench for the scenario's country. The driver of
    the tests, in the machine, presses the keys of each step when the
    program is ready for them and writes the screen out when the program
    waits for a key with every key of the step taken and nothing left to
    do by itself (see driver.py and DRIVER.ASM). The original and a build
    of the port are driven alike, and neither is changed for it.

    With `trace`, what the program executed is recorded. With `blinking`,
    a screen with the program's clock on is recorded in both its states,
    which takes a second of the machine's time for each. With `stack`, how
    much of its stack the program never used is recorded (see
    driver.stack_left).
    """
    steps = ("",) + scenario.steps
    known = driver.facts(program, stack)
    script = driver.script(scenario.steps, known, blinking)
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, program, script, log) as machine:
            machine.resume()
            try:
                machine.wait_for_end(STEP_TIME)
            except MachineError:
                done = len(driver.read(machine.written())[0])
                shown = "\n".join(machine.screen().lines())
                raise MachineError(
                    f"after {_where(steps, done)}: the program did not "
                    f"come to wait for a key within {STEP_TIME} s; it "
                    f"shows\n{shown}") from None
            done, ended = driver.read(machine.written())
            disks = machine.disks
        if not ended:
            raise MachineError(
                f"after {_where(steps, len(done))}: the machine stopped: "
                "it was reset, or QEMU ended")
        if len(done) != len(steps):
            raise MachineError(f"the driver wrote {len(done)} screens for "
                               f"{len(steps)} steps")
        # The machine has been shut down: the disk images are complete.
        codepage = setups.codepage(scenario.country)
        result = Run([step.screen for step in done],
                     {drive: fat.read_tree(path, codepage)
                      for drive, path in disks.items()})
        if trace:
            result.executed = coverage.executed(log, program.contents)
        left = [driver.stack_left(step.stack, known) for step in done
                if step.stack is not None]
        if left:
            result.stack_left = min(left)
            result.stack_segment = next(
                step.stack_segment for step in done
                if step.stack is not None)
    return result


def _where(steps, done):
    """The step that was under way when `done` of `steps` had ended, the
    start being the first."""
    if done >= len(steps):
        return "the last step"
    return f"step {done}, {steps[done]}" if done else "the start"
