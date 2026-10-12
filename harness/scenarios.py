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
  - The Info panel shows the file dirinfo of the directory of the other
    panel, once that has been read again, as it is after the editor.
    "F9 o m" switches Memory allocation, and the Info then says how much
    memory is free with all of the program loaded: that depends on the
    size of the program, so the port shows another number than the
    original. Only the scenario memory does it, and deviations.py has
    the number left out of the comparison there. Every disk of the bench
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
from dos import MachineError

# How long the screen must stand still to count as finished, in seconds,
# where that is all there is to go by: for a program that does not signal,
# and for DOS. It has to cover the longest the program waits before it does
# something by itself, 0.6 s for following the cursor to another directory,
# and both states of the blinking colon of its clock.
SETTLE = 0.8
# The same for what DOS shows when the program has ended.
DOS_SETTLE = 0.3
# How long a step may take before the program counts as stuck, in seconds.
STEP_TIME = 20


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
}


def run(scenario, program, bench, signals, trace=False):
    """Run `scenario` on `program` (a setups.Program) and return the Run.

    `bench` is the setups.Bench for the scenario's country. With `signals`
    the program is taken to be a build that signals where it is, and each
    screen is recorded the moment it says that it waits for a key and has
    nothing left to do by itself. Without, the moment is guessed: when the
    screen has stood still for a while.
    """
    with tempfile.TemporaryDirectory() as workdir:
        workdir = Path(workdir)
        log = workdir / "trace.log" if trace else None
        with bench.start(workdir, program, log) as machine:
            wait = _signalled if signals else _settled
            screens, left = [], []
            for number, step in enumerate(("",) + scenario.steps):
                if number:
                    machine.resume()
                    machine.press(step)
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
        # The program has ended; what DOS then shows comes without signals.
        return machine.wait_for_screen(DOS_SETTLE, STEP_TIME), False
    machine.pause()
    return machine.screen(), True
