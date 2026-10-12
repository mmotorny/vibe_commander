"""Scenarios: what is done to Volkov Commander to see how it behaves.

A scenario starts the program on the test bench (see setups.py) and presses
keys, one step at a time. After the start and after every step the screen
is recorded, once the program has done all that the keys make it do; at the
end, so is everything on the hard disks. What the original records is kept
as the scenario's golden (see goldens.py), and a build is right if it
records the same.

Steps are written as Machine.press() takes them: key names separated by
spaces, "Alt+F1" for keys held together, and text to type in double quotes.
Text may be Russian where the bench has the Russian keyboard, once
"Alt+RightShift" has selected its Cyrillic letters. That is pressed where
the program takes text, and in a step that types some: at the panels Alt
by itself brings up the menu or not, as the timing has it, and a step of
nothing but Shift and Alt leaves the program nothing to signal.

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
  - The dialog of Alt+F1 offers A, C and D: there is no drive B to be
    asked for, and no drive that is not ready.
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
  - Enter on a file with the extension bat runs it: on A:\\ the second
    file is fdauto.bat, which starts the bench again.
  - A step may press about 16 keys. The harness waits for each to reach
    the keyboard buffer, which holds 15, and fails when the program does
    not take them fast enough.
  - There is no mouse on the bench, and nothing sets the kind of a panel
    to anything but files, tree and Info: what the program does for a
    mouse or for a fourth kind of panel cannot be reached.
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
  - Keys that follow, in the same step, the Enter that starts a search
    are taken while it runs: Esc and Ctrl+c stop it, F1 is the help,
    whose window covers all of the box that changes, Left and Right
    move in "Stop, Go to, Quit" and a first letter answers. They arrive
    a few milliseconds after the Enter, before DOS is first asked for a
    file or after, as it happens; a key with Ctrl arrives 20 ms later
    than one without. So a step may depend on a key arriving while the
    search runs, if the search reads the floppy, but not on when.
  - The floppy is slow: a file of 46 KB takes most of a second to read.
    A search by contents there stands still on "Scanning:" for longer
    than the original is waited for, and the step is recorded in the
    middle of it, where the port's ends with the search. Such a step has
    to end stopped, or in the help: of A:\\FREEDOS\\BIN\\COMMAND.COM,
    87,772 bytes, which the search reads in three parts, "Enter Right
    F1" and then "Esc Ctrl+c". A search by contents of a few small files
    there is fast enough: the three *.bat of that directory.
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
  - F5 and F6 go in a step of their own, the name and Enter in the
    next: 'F5 "name" Enter' as one step works at first, but once a box
    with an error has been shown the dialog is slow to come up, the
    keys behind F5 fill the keyboard buffer, and the step fails with
    "the key t did not arrive", whatever the sixteenth key is. An
    answer to a box that the copying brings up is a step of its own
    too: Esc typed behind the Enter that starts the copying is taken
    for a break, if it gets there in time.
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
    in one step took more than the 20 seconds a step may, on a machine
    that other tests were using.
  - "Ctrl+a", "Down Space Enter" takes the Archive attribute off a
    file, which then has none.
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
    Enter there has not been tried.
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

Every scenario gets a machine of its own, in the state the bench was saved
in: the program as its distribution configures it but for the clock, which
is off, started in C:\\WORK. A scenario that wants something else arranges
it with its first steps, the way a user would. It may end anywhere, with
the program running or not.

The machine's clock runs while the harness waits, and it waits longer for
the original than for a build that signals. Nothing on the bench shows the
time of day, so that does not matter. A scenario that does show it, by
switching the clock on or by listing a file it has made, has to be done
with that before the machine's first minute is over.
"""
import re
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

import coverage
import fat
import setups
import watch
from dos import MachineError

# How long the screen must stand still to count as finished, in seconds,
# where that is all there is to go by: for a program that neither signals
# nor is the original. It has to cover the longest the program waits before
# it does something by itself, 0.6 s for following the cursor to another
# directory, and both states of the blinking colon of its clock.
SETTLE = 0.8
# How long a step may take before the program counts as stuck, in seconds.
STEP_TIME = 20


@dataclass(frozen=True)
class Scenario:
    description: str
    steps: tuple = ()
    country: str = "us"           # see setups.COUNTRIES
    # Step number -> how many of its last keys are pressed while the
    # program is busy with what the key before them began, and are meant
    # to find it so. The original is given any other key only when it
    # waits for one (see watch.py).
    ahead: dict = field(default_factory=dict)


@dataclass
class Run:
    """What a scenario showed and left behind."""
    screens: list                 # after the start and after each step
    disks: dict                   # drive letter -> fat.read_tree() of it
    # Offset and length in the program file of every instruction that ran,
    # if the run was traced.
    executed: dict = field(default_factory=dict)
    # The fewest bytes of its stack that the program had never touched,
    # of all the times it waited for a key; None for a program that does
    # not mark its stack.
    stack_left: int = None


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
            "Esc",                        # changed: asks
            "Right Enter",                # not saved
        )),

    "find": Scenario(
        "Looking for files by name and by what they contain, with keys "
        "pressed while the search runs",
        steps=(
            "Alt+F7",
            'Tab "hello" Enter',          # of C:\\WORK, the one that has it
            "1 n",                        # a key that means nothing, and
                                          # New search
            '"a:\\*.*" Tab Ctrl+y Enter Home',    # more than the box shows
            "Home PgDn Up",               # the list a line down
            "Down Down",
            "End Down",
            "PgUp",
            "Home",
            "n",                          # the last of three files has it:
                                          # Home comes before the first line
            '"a:\\freedos\\bin\\*.bat" Tab "startup" Enter Home',
            "n",                          # a file read in three parts
            '"a:\\freedos\\bin\\command.com" Tab "qzqz" Enter Right F1',
            "Esc Ctrl+c",                 # out of the help, and stopped
            "n",
            "Enter q",                    # the same again, and Quit
            "Alt+F7",
            '"hello.*" Tab Ctrl+y Enter',
            "Enter",                      # Go to: the cursor on the file
        ),
        # Right and F1 while the search reads the floppy, and Ctrl+c when
        # it goes on behind the help.
        ahead={13: 2, 14: 1}),

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
            # Nine copies of it. No step moves more than 3 MB: a step
            # may take 20 seconds, on a machine that others use too.
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
}



def run(scenario, program, bench, signals, trace=False):
    """Run `scenario` on `program` (a setups.Program) and return the Run.

    `bench` is the setups.Bench for the scenario's country. With `signals`
    the program is taken to be a build that signals where it is, and each
    screen is recorded the moment it says that it waits for a key and has
    nothing left to do by itself. The original is watched from outside for
    the same moment (see watch.py). For any other program the moment is
    guessed: when the screen has stood still for a while.
    """
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, program, log) as machine:
            watched = None
            if signals:
                wait = _signalled
            elif watch.available(program):
                watched = watch.Watch(program)
                wait = _watched(watched)
            else:
                wait = _settled
            screens, left = [], []
            for number, step in enumerate(("",) + scenario.steps):
                if number:
                    machine.resume()
                    machine.press(step, watched,
                                  scenario.ahead.get(number, 0))
                where = f"step {number}, {step}" if number else "the start"
                screen, waiting = wait(machine, where)
                screens.append(screen)
                if waiting:
                    left.append(_stack_left(machine))
            disks = machine.disks
        # The machine has been shut down: the disk images are complete.
        codepage = setups.codepage(scenario.country)
        result = Run(screens, {drive: fat.read_tree(path, codepage)
                               for drive, path in disks.items()})
        if trace:
            result.executed = coverage.executed(log, program.contents)
        if left:
            result.stack_left = min(left)
    return result


# What the build that signals fills its stack with when it has set it up:
# StkMark in port/asm/VC.ASM, low byte first. The stack grows down from its
# top, so what it has never reached is still marked, from the bottom up.
STACK_MARK = b"\x5A\xA5"
CONVENTIONAL_MEMORY = 0xA0000     # where DOS programs live, below this


def _stack_left(machine):
    """How many bytes of its stack the program has never touched: the
    longest run of the mark in the machine's memory. That is 0 once the
    stack has been used to its bottom, or beyond."""
    memory = machine._memory(0, CONVENTIONAL_MEMORY)
    runs = re.finditer(b"(?:%s){2,}" % re.escape(STACK_MARK), memory)
    return max((len(run.group()) for run in runs), default=0)


def _settled(machine, where):
    """Wait for the screen to stand still and return it, and that it is
    not known whether the program waits for a key."""
    try:
        return machine.wait_for_screen(SETTLE, STEP_TIME), False
    except MachineError as error:
        raise MachineError(f"after {where}: {error}") from None


def _watched(watched):
    """What waits for the original to have finished a step, with the
    watch.Watch on its machine, and returns the screen, and that it is not
    asked how much of its stack the program has left."""
    def wait(machine, where):
        try:
            return watched.wait(machine), False
        except MachineError as error:
            raise MachineError(f"after {where}: {error}") from None
    return wait


def _signalled(machine, where):
    """Wait for the program to signal that it waits for a key, or that it
    has ended, and return the screen and which of the two: whether it
    waits. Leaves the machine paused if it does."""
    heard = ""

    def idle():
        nonlocal heard
        if not machine.keys_taken():
            return False
        # Every key has been taken, and a key that is taken says K at once.
        # So if the last word now is W, the program waits with nothing in
        # hand.
        heard += machine.signals()
        return heard.endswith(("W", "X"))

    try:
        machine.until(idle, STEP_TIME, "")
    except MachineError:
        machine.pause()
        shown = "\n".join(machine.screen().lines())
        raise MachineError(
            f"after {where}: the program did not come to wait for a key "
            f"within {STEP_TIME} s; it signalled {heard!r} and shows\n"
            f"{shown}") from None
    if heard.endswith("X"):
        # The program has ended; DOS then shows its prompt.
        return machine.wait_for_prompt(STEP_TIME), False
    machine.pause()
    return machine.screen(), True
