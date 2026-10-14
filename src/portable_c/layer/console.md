# The layer: the screen and input

**Draft of 13 October 2026. What the user has decided of it is under
"Decided in this area", and nothing else here is decided.** It is one
area of step 3 of the road (`CLAUDE.md`, "The portable C"), surveyed
from the platforms' side as the user asked on 13 October 2026: what
each target platform gives for drawing a text screen and for taking
keys and the mouse, where they agree, where one has another shape, the
common ground, and only then how the program's needs map onto it. The
program's needs are those of `../LAYER.md`, "What the program needs:
the inventory", the groups "The console", "The keyboard", "The screen",
"The mouse" and "The timer".

What is read and what is inferred. Every cell of the tables that has a
source behind it, `[W1]` and the like, is what that page says, read on
13 October 2026; the sources are listed at the end. A cell marked *(i)*
is inferred: general knowledge of the platform, or a conclusion from
what was read, not checked in documentation. Nothing was tried on any
platform. What is said of DOS is read in `dos32.c` and in the program,
which the scenarios judge; nothing was run for this survey there
either.

## Decided in this area

The user's answers to the ten questions at the end, 13 and 14 October
2026, as the session passed them on. Each question is marked there with
its answer.

1. **The shape: A.** The program keeps its grid of cells, each a code
   point and a colour, names what has changed, and the layer presents
   it. Input is asked for as now: is a key waiting, which modifiers are
   held, where the pointer is; and one wait besides. DOS keeps its
   calls.
2. **The screen of commands outside DOS: answered by
   `programs.md`, decision 2.** The shadow grid comes later, with
   libvterm. Until then a command has the real terminal, and Ctrl-O
   shows the terminal's own screen.
3. **The tap on Alt: the kitty keyboard protocol, or Windows Terminal's
   win32-input-mode, where the terminal offers them.** Elsewhere there
   is no tap and no key bar of modifiers, and F9 opens the menu. A
   deviation, to be declared when it is built.
4. **Terminal or window: a terminal on Windows, Linux and macOS**, as
   Midnight Commander runs (`programs.md`, decisions 2 and 9); iOS draws
   a view of its own. No window of the program's own on a desktop: no
   native app is planned on macOS.
5. **The size of the screen: 80 columns and any number of rows from 25
   up**, as now. A wider screen shows the 80 at the left or centred; a
   smaller one is refused.
6. **The 16 colours in a terminal: the PC's exact colours in 24 bits**
   where the terminal has them, the named 16 otherwise.
7. **A character's width: answered by `i18n.md`, decision 7.** The
   layer answers each character's width, and the panels count cells.
8. **Touch on iPhone and iPad:** a tap is the left button, a long press
   the right; the key bar is tapped for F1 to F10; a row of Esc, Tab,
   Ctrl, Alt and arrows stands above the on-screen keyboard; on an
   iPhone, landscape only.
9. **What names a key: named keys and the character only**, chosen by
   the user against the recommendation of the key's physical place. So
   Alt with a Cyrillic letter searches by that Cyrillic letter, not by
   the Latin key under it. What follows:
   - A key event is a named key (F1, the arrows, Enter, Esc and the
     like) or a character, with the modifiers held. The layer has no
     scan code and no USB HID usage; the kitty protocol's base layout
     key [K1] is not asked for.
   - The speed search is Alt with a character, any character, where it
     is Alt with one of 38 keys today. `AltSr_T` of `keys_t.c`, by
     which the main loop starts it, and `AltSr_T` and `Ascii_T` of
     `cmdline.c`, by which it turns the key into a letter, are scan
     codes of a US keyboard and leave the common code. The three are
     constants of the original's, which `portable_c-data` holds and
     `harness/reads.py` lists: where they go is declared there when it
     is built. The program's other comparisons of keys, the `case`
     labels on scan codes, become comparisons of named keys and
     characters as shape A has it.
   - A point to settle, DOS: the BIOS reports Alt with a letter key as
     the scan code with 0 for the character. The program starts the
     speed search only for a key with 0 there, and `command-line` shows
     it starting on the bench. So DOS's implementation has no character
     to report for it and has to make one of the scan code, and the
     only table it has is the US order of `Ascii_T`: Alt with a letter
     is then a Latin letter on DOS whatever the layout, as in the
     original, and the decision holds off DOS only. Whether FreeDOS's
     KEYB with the Russian layout, on the Russian bench with its
     Cyrillic letters selected, gives a character with Alt was not
     tried; DOS has no call that tells a program the layout of the
     keyboard *(i)*. No scenario would show
     a change on DOS: the one scenario that presses Alt with a letter,
     `command-line`, runs on the US bench, where the table gives the
     letter that the original searches by, and no scenario of the
     Russian bench presses Alt with a letter at all. What is to settle
     is whether that stands: DOS keeping the original's Latin letter,
     with nothing declared, or a way sought for DOS to give the letter
     of the layout too.
10. **Function keys where the keyboard has none** (iPad, some Macs):
    Cmd with a digit for F1 to F10, and the key bar tapped or clicked.

## What the program does today

Read in the program at 859b90d:

- **It draws into a grid of its own.** `md.ScrBuf` is 80 cells to a row,
  `MaxLins` (50) rows, each cell a byte of the code page and a colour
  byte. Every routine that draws writes there, and `Window` of
  `window.c` puts a rectangle of it on the screen through `ScreenPut`,
  from some 130 places. Whatever a box covers is kept
  and put back in the program's own buffers (`SavWin`, `ResWin`): no
  system is asked to save a region.
- **It also writes the real screen outside the grid**, as a terminal
  does: the prompt and the command at a run, carriage returns and line
  feeds (`ScreenType`, in `exec.c`, `enter.c`, `main.c`, `start.c`,
  `stmain.c`), the bell (`Beep`, `ScreenType(7)`), the banner through
  DOS (`ConsoleText`), a star of the blanker (`ScreenFill`). And it
  scrolls the real screen by a line (`Scroll`, seven places: the
  panels, the help, the editor, the tree, the file search, the setup's
  colours and the setup's help), after it has written every row into the grid, and
  then puts only the new row on the screen: the scroll is a shortcut
  of the drawing, not a thing of its own.
- **It reads the screen back.** After a command, `SavScr` reads what
  the command left on the screen into `UserScr`, and with the panels
  off (Ctrl-O) or one of them off (Ctrl-F1, Ctrl-F2) the program shows
  that (`ResUser`, seven places). At the start it reads the cell at the
  cursor to learn the colour that DOS writes in (`ScreenCell`, `start.c`).
- **It polls.** `Input0` of `input.c` goes round: is a key waiting
  (INT 16h 01h), which mouse buttons are down (INT 33h 03h), which of
  Shift, Ctrl and Alt are held (INT 16h 02h), the ticks, the time of
  day for the clock and the blanker, and `Idle` (INT 28h). A tap on Alt
  alone, which opens the menu, is seen there: Alt went down and up, and
  nothing else changed between two rounds less than three ticks apart.
  The key bar shows the keys of Shift, Ctrl or Alt while one is held. A
  double click is two presses close in time, measured by the program
  (`mouse.c`). Some loops wait for a button to be let go by polling
  alone, with no `Idle`: `while (GetMous(0).buttons != 0);` in `Input0`.
- **A key is the BIOS's word**: the character of the code page in the
  low byte, the scan code in the high one. The program goes by the
  scan code for every key that has one (`Case` "takes no key without a
  scan code"), and by the physical key for Alt with a letter: the speed
  search turns the scan code of Alt-Q into `q` by a table in the order
  of a US keyboard (`AltSr_T` and `Ascii_T` of `cmdline.c`). The BIOS's
  call 00h drops the keys of an enhanced keyboard [D1], so the program
  never sees F11 or F12.
- **The mouse** counts in dots, eight to a cell; the driver draws the
  pointer by turning bits of the cell under it, and the program sets
  that look (a check mark on the key bar), hides the pointer while it
  writes the screen, puts it in the middle and limits its rows.
- **Colours** are the PC's 16 for the text and 8 for the background,
  with the top bit for blinking: the setup's colours offer cells that
  blink (`BLINKS` of `stcolor.c`).

So the program is already the shape that the common ground below
turns out to be: a grid that it writes and has presented, and input
that it asks for. What is of DOS in it is the values (the code page,
the attribute byte, the key word, dots), the polling of state that only
DOS keeps, the read-back, and the screen it shares with commands.

## The survey

Columns: DOS as `dos32.c` drives it (the BIOS, the display's memory,
the mouse driver); a terminal, which is what a program in a POSIX
terminal gets and also what Windows Terminal and macOS Terminal give
over VT sequences; the Windows console's own API; a window of the
program's own on a desktop (Win32, AppKit); and iOS and iPadOS, where
there is no terminal and the program draws in a view of its own.

### Drawing

| Operation | DOS | Terminal | Windows console API | Own window (desktop) | iOS, iPadOS |
|---|---|---|---|---|---|
| Put cells | write the display's memory at B800h, a byte and an attribute a cell; seen at once | move the cursor (CUP), set colours (SGR), write UTF-8: a stream that the terminal interprets [X1][W3] | `WriteConsoleOutputW`, a rectangle of `CHAR_INFO`; off Microsoft's roadmap but supported "for the indefinite future" [W4]; or VT [W3] | draw glyphs in a grid (DirectWrite, Core Text) *(i)* | draw glyphs in a `UIView` *(i)* |
| Characters | the 256 of the code page | all of Unicode that the font has | all, as UTF-16 [W4] | all | all |
| Colours | 16 on 8, a bit to blink (or 16 on 16) | 8+8 named (SGR 30–37, 90–97), 256, 24-bit [W3]; the named 16 are the user's theme *(i)*; blinking not everywhere *(i)* | attributes in the PC's own layout, no blink [W4]; VT colours of 24 bits are taken to the nearest of the console's 16 [W3] | any | any |
| Width of a character | one cell, always | the terminal's own idea, which differs between terminals [U1]; characters of ambiguous width (box drawing, Cyrillic, `■`, `√`) are two cells in a terminal set up for East Asian text *(i, from [U3])* | the console's own rules *(i)* | the program's to decide | the program's to decide |
| Cursor | BIOS 01h, 02h, 03h: a place, a shape in lines of the character | shown or hidden (DECTCEM), block, underline or bar (DECSCUSR) [W3] | `SetConsoleCursorInfo` (a size in percent, shown) *(i)*, or VT | drawn by the program *(i)* | drawn by the program *(i)* |
| Scroll a rectangle | BIOS 06h, 07h | rows by a scrolling region (DECSTBM) and SU, SD [W3]; a column margin not everywhere *(i)*; or redraw | `ScrollConsoleScreenBuffer` [W3], or VT | redraw | redraw |
| Read back what is shown | yes, the display's memory | **no**; a checksum of a rectangle in some (DECRQCRA) [X1]; on the Linux console itself (not a terminal emulator) `/dev/vcsa` [L1] | `ReadConsoleOutput`, off the roadmap; Microsoft calls such "wrong-way verbs" and is converging away from them [W2] | only its own copy | only its own copy |
| A second screen | none: one screen, shared with the commands that the program runs | the main screen and the alternate one (`?1049h`), which has no scrollback [X1][W3]; what a command writes on the main screen cannot be read | screen buffers of its own (classic) *(i)*, or the alternate screen (VT) [W3] | none; a command's output needs a pseudo-terminal and a terminal emulator of the program's *(i)* | none, and no commands: an app cannot run other programs *(i)* |
| The bell | BIOS teletype 07h: the speaker | BEL; heard, seen or nothing by the user's settings [X1] | BEL, or `Beep` *(i)* | the system's beep *(i)* | no beep; a sound or haptics *(i)* |
| Size | 80 by 25, 43 or 50, by the mode and the font | any; told by a signal, `SIGWINCH`, and `TIOCGWINSZ` *(i)*, or asked, `CSI 18 t` [X1] | asked, `GetConsoleScreenBufferInfo`; told, an input record [W2] | any; the window's | any; rotation, Split View, Stage Manager *(i)* |
| When it shows | at once | when the stream is flushed; may tear mid-update unless the terminal knows synchronized output, mode 2026 (xterm does, st not [L4]) *(i)* | at once (classic) | at the next frame *(i)* | at the next frame, drawn on the main thread only *(i)* |

### Keys

| Operation | DOS | Terminal | Windows console API | Own window (desktop) | iOS, iPadOS |
|---|---|---|---|---|---|
| How keys arrive | polled: INT 16h 01h, read with 00h [D1] | bytes on standard input, in raw mode *(i)*; read with a timeout | records, `ReadConsoleInputW`, a key down and a key up each [W1] | events on the window's loop | `pressesBegan` and `pressesEnded` on the main thread [A2] |
| Which physical key | the scan code | **none**: a character or a named key; with the kitty protocol's flag 4, the key of a US layout as well ("base layout key": Ctrl+С on a Cyrillic layout reports `c`) [K1] | the scan code and a virtual key [W1] | Windows: the scan code; macOS: Apple's own key numbers *(i)* | `keyCode`, a USB HID usage [A1][A4] |
| The character | a byte of the code page (the Russian driver's CP866 on the Russian bench) | UTF-8 | UTF-16 [W1] | the system's text input *(i)* | `characters` [A1]; an input method wants `UITextInput` *(i)* |
| Modifiers with a key | the BIOS's flags, polled | in the sequence of a named key (2 Shift, 3 Alt, 5 Ctrl, ...) [X1]; Ctrl with a letter is a control character, so Ctrl and Ctrl-Shift are one; Alt is ESC in front [W3]; others with xterm's modifyOtherKeys [X1] or the kitty protocol [K1] | left and right Ctrl and Alt, Shift, the locks [W1] | with the event | `modifierFlags` [A1] |
| A modifier alone, a key let go | the flags, polled (INT 16h 02h) | **no**; yes with the kitty protocol's flags 2 and 8 [K1], or Windows Terminal's win32-input-mode [W5] | yes, but "the ALT key when pressed and released without combining with another character ... is not passed through to the application" [W1] | yes: `flagsChanged` [A10], key-up messages *(i)* | a key let go: yes [A2]; a modifier alone: not found in the documentation, to be tried |
| Repeat | the BIOS repeats the key | the key again; marked as a repeat only with the kitty protocol [K1] | `wRepeatCount` [W1] | a flag on the event *(i)* | not found in the documentation, to be tried |
| Keys the system keeps | Ctrl-Alt-Del; Ctrl-Break goes to INT 1Bh | what the desktop and the terminal bind (Alt-F4, Alt-F7 under some window managers; Cmd on macOS) *(i)*; Esc and Alt-letter alike begin with ESC, told apart by a timeout unless the protocol says *(i)* | Ctrl-C in processed mode [W1]; Alt-Enter in some hosts *(i)* | Cmd shortcuts; macOS gives Ctrl-arrows to Mission Control by default *(i)* | Cmd and Globe shortcuts; since iOS 15 text input and focus get keys before the app's key commands unless it asks [A3] |
| Function keys | F1 to F10 (call 00h drops F11, F12 [D1]) | F1 to F12 [W3][X1]; a Mac keyboard sends media keys unless Fn is held *(i)* | all | all | many iPad keyboards have no row of function keys and no Esc *(i)*; the on-screen keyboard has none |
| An on-screen keyboard | none | none | none | none | the system's, for a responder that takes text [A8]; a row of the app's own keys above it, `inputAccessoryView` [A9] |
| Typing Russian | the keyboard driver, CP866 bytes | the terminal's input method, UTF-8 | the system's layout, UTF-16 | the system's | the system's |

### The mouse and touch

| Operation | DOS | Terminal | Windows console API | Own window (desktop) | iOS, iPadOS |
|---|---|---|---|---|---|
| Where and which buttons | polled, INT 33h 03h, in dots, 8 to a cell | reported, in cells: press and release (`?1000h`), with motion while a button is down (`?1002h`) or always (`?1003h`), in SGR form (`?1006h`), with Shift, Alt and Ctrl [X1]; Shift is usually kept by the terminal for selecting text *(i)* | a record with the cell, the buttons, double clicks and the wheel, once quick edit is off *(i)* | events, in pixels | touches; on iPad with a mouse or trackpad, buttons [A5] and hover [A6] |
| The pointer | the driver draws it by bits of the cell, set by the program | the terminal's | the console's | the system's; any image | the system's, shaped by the app [A7]; none for a finger |
| Double click | measured by the program | measured by the program *(i)* | flagged by the console *(i)* | flagged by the system | a tap count *(i)* |

## The differences that matter

These are where a platform has another shape, not another spelling.

1. **Reading back.** Only DOS, the Windows console's classic API and the
   Linux console itself can say what a cell of the screen shows. A
   terminal cannot [X1], and Microsoft is moving the Windows console the
   same way [W2]. A window of the program's own knows only what it drew.
   What the program reads back is what commands wrote: everywhere but
   DOS, that is the program's to keep if it is to be shown again.
2. **The screen of commands.** On DOS the panels and the commands share
   one screen. A terminal has two, of which the program can write both
   and read neither. A window of its own has none: to show a command's
   output there, the program runs the command in a pseudo-terminal and
   is the terminal emulator itself *(i)*. On iOS there are no commands.
   Two file managers of the same lineage show the choice: Midnight
   Commander uses the terminal's two screens and, on the Linux console,
   a helper that reads the console's memory, and "the panels cannot be
   toggled" inside GNU screen [L2]; far2l has a terminal emulator of its
   own, "far2l's VT" [L3] (that its commands' output behind the panels
   comes from it is my reading, not checked).
3. **Width.** On DOS a byte is a cell. A terminal decides a character's
   width itself, and terminals disagree: a sequence of three code points
   is 2, 4, 5 or 6 cells in fourteen terminals measured [U1]. Box
   drawing and Cyrillic are of ambiguous width in Unicode [U3]. A
   program that writes one character and moves on by its own count of
   cells loses its place where the two disagree; one that places the
   cursor at the start of each run loses at most a cell *(i)*. In a
   window of its own the program decides.
4. **A modifier alone, and a key let go.** DOS polls the BIOS's flags.
   Every platform with events of its own reports them (the Windows
   console says it keeps Alt alone to itself [W1]). A terminal reports
   neither, unless it speaks the kitty protocol (kitty, Alacritty, foot,
   Ghostty, iTerm2, WezTerm, Windows Terminal, xterm.js and others by
   the protocol's own list [K1], not macOS Terminal by that list) or
   win32-input-mode [W5]. So the tap on Alt and the key bar of Shift,
   Ctrl and Alt are possible everywhere but in a plain terminal.
5. **The physical key.** DOS, Windows and iOS name the key that was
   pressed; iOS by the USB HID usage [A4], which is also what SDL takes
   as its "scancode", a key by its place "independent of language or
   keyboard mapping" [S1]. A terminal names only the character, or a
   few named keys, unless the kitty protocol adds the key of a US
   layout [K1]. Alt with a Cyrillic letter for the speed search depended
   on that; the layer goes by the character (decision 9).
6. **Polling and events.** Only DOS is polled. Everywhere else input
   arrives as events, and on macOS and iOS the program cannot hold the
   main thread in a loop: drawing and events belong to it *(i)*. State
   that DOS answers when asked, which modifiers are held, where the
   mouse is, the others answer by the events seen so far.
7. **Colours.** The PC's 16 are the Windows console's own [W4] and any
   window's. In a terminal the 16 named colours are the user's theme
   and may be anything; colours of 24 bits are exact where the terminal
   has them, and the Windows console takes them to its 16 [W3].
   Blinking is not everywhere *(i)*.
8. **Size.** DOS has a few fixed sizes; everything else has any size and
   says when it changes. The program has 80 columns throughout and its
   rows from the mode.
9. **Keys that are missing or taken.** Function keys are absent from
   many iPad keyboards and from every on-screen keyboard *(i)*; desktops
   and terminals keep some combinations. The program's key bar, which
   can be clicked, is a way to every function key on a screen that can
   be touched.

Whether the list of platforms is complete: nothing found changes the
picture for this area. Two distinctions within it matter. The Linux
console is not a terminal emulator: it can be read back [L1] and has
no kitty protocol *(i)*. A terminal may be remote, over SSH, where a
window of the program's own cannot go. Android, were it added, would
be iOS's shape: a view of its own, touch, no terminal *(i)*.

## Candidate shapes

### A. A grid that the program writes and the layer presents, input asked for

The program keeps its grid, as it does now, in the layer's values: a
cell is a character (a code point) and a colour of the layer's own (one
of 16 for the text, one of 16 for the background, blinking or not). It
hands the layer the grid once and then says which rectangle has
changed (`Window` today). The layer shows it its way. Input is asked
for by functions that answer at once, as the program asks DOS today: is
an event waiting, take it, which modifiers are held, where is the
pointer and which buttons are down; and one function that waits until
something happens or a time has passed (`Idle` today). A platform with
events keeps a queue and the state that the events say, filled when
the program asks or by a thread of its own.

- DOS: the same calls in the same order. The grid's character goes to a
  byte of the code page by a table when the cells are copied to B800h,
  which is a copy as today; the key word becomes an event by a table,
  with no call more; the mouse's dots become cells by a division. See
  "DOS under the recommendation".
- Terminal: the layer keeps a copy of what it last sent, sends only the
  cells that changed (with the cursor placed at the start of each run),
  takes raw input and parses sequences, and asks for the kitty protocol
  and mouse reporting where the terminal has them *(i)*. Some 1,000 to
  2,000 lines of C, like the core of a small curses library *(i)*.
- Windows: the same terminal implementation over VT, which Microsoft
  recommends [W2], with the console's input records for keys where they
  are better [W1]; or the classic API, which is a grid already [W4].
- A window of its own, iOS: a view that draws the grid with a
  monospaced font; the program runs on a thread of its own, asks the
  queue, and its "present" asks the main thread to redraw *(i)*.
- What the program changes: the cell's type and every routine that
  writes characters into cells (`../LAYER.md`, "What UTF-8 inside
  touches", lists them); keys compared as events instead of BIOS words
  (some 190 `case` labels in five files, besides other comparisons); loops that poll without waiting
  wait; the screen of commands asked of the layer instead of read back.
  The structure of the program, its drawing and its loop, stays.

### A2. The same grid, input as events that the program waits for

As A, but the program is turned round: the platform calls the program
with each event, or the program waits for the next event, instead of
polling state.

- DOS: the layer would have to poll the BIOS in its own order, and the
  program's order of calls (key, mouse, modifiers, ticks, time, idle)
  would no longer be the DOS file's to keep: `Input0` today asks INT 21h
  2Ch for the time between its key and mouse questions. This is the
  shape where DOS could **not** keep today's calls.
- Elsewhere: the most natural shape on iOS and macOS, a little less
  work for the layer there than A.
- What the program changes: `Input0`, the help, the menus, the dialogs,
  the viewer, the editor, every place that loops on a key, turned into
  handlers or into waits. Much more than A.

### B. A stream of text and VT sequences

The program writes UTF-8 with escape sequences, as to a terminal, and
reads key sequences back; the layer is a pipe on a terminal and a
terminal emulator everywhere else.

- DOS: the layer would interpret the sequences and write the display's
  memory, an ANSI.SYS of its own; the BIOS's teletype calls and the
  scroll calls would go, and with them today's calls. Nothing can be
  read back through a stream, so `UserScr` has no source.
- Terminal: almost nothing to write.
- Windows: VT is Microsoft's recommendation [W2]; the Windows console
  takes it.
- A window of its own, iOS: a whole terminal emulator, both ways, under
  the program, where A needs a grid drawn.
- What the program changes: every drawing routine, from cells in a
  buffer to text in a stream, and the saving and restoring of boxes,
  which the program does in its grid today, kept in a grid anyway.

### C. A higher interface: panels, menus, dialogs as the platform's own

The layer offers menus, dialogs and lists, and each platform makes
them of its own controls. It would be another program: the screens that
the scenarios hold against the original's would be gone, and DOS would
need all of it written for text mode. Named for completeness; not
proposed.

## The recommendation: A

A is the common ground. Every platform can show a grid of characters in
colours and say which keys and buttons came; only a terminal cannot
read a screen back, and only DOS has to be polled. A asks the polled
questions, which every platform can answer from its events, and so the
DOS file keeps today's calls in today's order. The program already is
this shape, and a test can be a platform of its own: a grid in memory
and a script of events, which A2 and B make harder *(i)*. The user
chose A (decision 1).

What it does not settle, and the questions below asked (decisions 2
and 3 answer them):

- The screen of commands. Under A it is a second grid that the layer
  keeps and can say it does not know: on DOS the screen itself, read
  as today; in the Windows console's classic API, read back; on the
  Linux console, read from `/dev/vcsa`; in a terminal, unknown unless
  the program runs commands in a pseudo-terminal and keeps their screen
  with an emulator of its own, which is the shadow grid that comes
  later, with libvterm; on iOS, empty.
- The tap on Alt and the key bar of modifiers in a terminal that does
  not report modifiers: the layer says it cannot, and the program does
  without (F9 still opens the menu).

### What the layer would offer under A

Names are placeholders; the types are the layer's own and of standard C
as the user decided. "None elsewhere" means that another platform has
the function and does nothing.

| Function | What it does | DOS (today's call) | Elsewhere |
|---|---|---|---|
| the grid given | where the program's cells are, the columns and rows | notes it | notes it |
| `ScreenSize` | columns, rows, whether colours are shown | BIOS 0Fh, 1130h, 12h (`ScreenState`, `ScreenFont`, `ScreenEga`) | the terminal's size, the view's |
| `ScreenShow(rectangle)` | the cells of a rectangle shown | copies to B800h, characters through the code page (`ScreenPut`) | sends the changes, or redraws |
| `ScreenMove(rectangle, lines, colour)` | what a rectangle shows moved up or down, the program's grid already written so | BIOS 06h, 07h (`ScreenScroll`) | shows the rectangle from the grid |
| `CellsOf(character)` | how many cells a character takes: 1 or 2 | always 1 | the platform's answer, see decision 7 |
| `CursorTo`, `CursorLook` | a place; hidden, a line or a block | BIOS 02h, 01h (`CursorTo`, `CursorShape`) | DECSCUSR, DECTCEM; drawn |
| `CursorWhere` | the place and the look | BIOS 03h | what was set |
| `Bell` | the bell | BIOS 0Eh with 07h (`ScreenType`) | BEL, a sound |
| `CommandsType(character)` | a character written on the screen of commands as a terminal writes it | BIOS 0Eh (`ScreenType`) | written to the terminal's main screen, or to the emulator |
| `CommandsGet(rectangle)` | the screen of commands, if known | the display's memory (`ScreenGet`) | see above; "unknown" from a plain terminal |
| `CommandsColour` | the colour that commands write in | BIOS 08h around DOS's space (`ScreenCell`, `ConsoleChar`) | the terminal's default colours, a colour of their own in the cell's vocabulary (decision 6) |
| `InputWaiting`, `InputTake` | the next event, if any; and taken | INT 16h 01h, 00h (`KeyWaiting`, `KeyRead`) | the queue |
| `ModifiersHeld` | Shift, Ctrl, Alt held, the locks; or "not known" | INT 16h 02h (`KeysHeld`) | from the events; "not known" from a plain terminal |
| `PointerState` | the cell, the buttons down; or "no pointer" | INT 33h 03h (`MouseWhere`) | from the events |
| `PointerShow`, `PointerHide`, `PointerTo`, `PointerRows`, `PointerLook` | the pointer | INT 33h 01h, 02h, 04h, 08h, 0Ah | none elsewhere, or the system's pointer |
| `InputWait(until)` | nothing to do until an event or a time | INT 28h once (`Idle`) | blocks on the queue or input until the time |
| `ConsoleCan` | what this platform can: modifiers alone, keys let go, a pointer, the screen of commands, blinking | answers all but keys let go | its answers |

An event is a key pressed, repeated or let go (a named key or the
character it types, by decision 9, and the modifiers), a button, the
size changed, or the program's own (the timer, the tap on Alt, as
`Input0` makes today). `KeyForgetIns`, the BIOS's bit for Ins held, is
of DOS alone and stays in the DOS file, said by the key's repeats
elsewhere *(i)*.

### DOS under the recommendation

Every function above is one of today's calls, made where the program
makes it today, so the scenarios see the same order of calls of DOS and
the BIOS. What is new on DOS is arithmetic between the call and the
program: a code point to a byte of the code page and back, a key word
to an event, dots to cells. Three places need care, and none of them
changes a call:

- The table of the code page is the country's, as `CaseMap` is: a
  character that the code page lacks is shown as a substitute.
- `InputWait` makes INT 28h once and returns, as `Idle` does; the time
  it is given is not waited out on DOS.
- `CommandsColour` keeps the order of `start.c`: the cell read, DOS's
  space, the colour read, the cell written back.

The loops that poll the mouse without `Idle` would call `InputWait` on
other platforms only, or call it on DOS too, which is an INT 28h more
in those loops: a deviation of the calls, to be asked if wanted.

### What "UTF-8 inside" bears here

- Text is UTF-8 in the program's strings; the grid holds one character
  in each cell, as a number, since a cell has to have a fixed size
  *(i)*. A character of two cells takes its cell and the next one,
  which holds a mark that the layer skips.
- On DOS a character becomes a byte of the code page when it is shown,
  and a typed byte becomes a character when it is read, by the
  country's table: what DOS shows and what the scenarios see stays the
  same.
- How wide a character is, is the platform's answer, not the program's
  table: a terminal decides it, and only the platform can know which
  rule it follows. Characters of ambiguous width, box drawing and
  Cyrillic among them [U3], are one cell except in a terminal set up
  for East Asian text *(i)*; the layer can say so by `CellsOf` and the
  program then cannot draw its frames, which is that user's case to
  decide on later.
- The drive names that the user gave for the iPhone, `⛁` and `☁️`, are
  drawn by the program's own view there, which decides their width.
  `☁` alone is of narrow width in Unicode and `☁️`, with the selector
  of emoji presentation behind it, is shown as an emoji, which
  terminals count as one or two cells [U1][U3] *(i)*.
- What the speed search makes of Alt with a letter is the physical key
  on DOS (a US letter whatever the layout); elsewhere the character
  typed, Cyrillic too (decision 9, with its point to settle for DOS).

## How the program's needs map onto it

| Today, `sys.h` | Under A |
|---|---|
| `ConsoleChar`, `ConsoleText` | the banner before the screen is taken: written to standard output elsewhere; on DOS as today |
| `KeyWaiting`, `KeyRead` | `InputWaiting`, `InputTake` |
| `KeysHeld` | `ModifiersHeld`, which may answer "not known" |
| `KeyForgetIns` | DOS file |
| `ScreenState` | `ScreenSize`; the mode and the page are the DOS file's |
| `ScreenSetMode`, `ScreenPageStart`, `ScreenMemory`, `ScreenEga`, `ScreenFont` | DOS file, behind `ScreenSize` and the grid given |
| `CursorWhere`, `CursorTo`, `CursorShape` | `CursorWhere`, `CursorTo`, `CursorLook` |
| `ScreenScroll` | `ScreenMove`; the blanker's clearing is the program writing its grid |
| `ScreenCell`, `ScreenFill` | `CommandsColour`; the blanker's star is a cell of the grid shown |
| `ScreenType` | `CommandsType`, and `Bell` |
| `ScreenPut` | `ScreenShow` |
| `ScreenGet` | `CommandsGet` |
| `MouseFind`, `MouseReset` | `ConsoleCan`; the reset is the DOS file's |
| `MouseShow`, `MouseHide`, `MouseTo`, `MouseRows`, `MousePointer` | the pointer's functions, none elsewhere |
| `MouseWhere` | `PointerState`, in cells |
| `Ticks` | the timer's area, not this one |
| `Idle` | `InputWait` |

## Questions for the user

Each with its ways, the recommendation first. All are answered, on 13
and 14 October 2026: see "Decided in this area". They are kept here as
they were put, each marked with the answer.

1. **The shape of the screen and input.** (a) The program keeps its
   grid and asks for input as it does now; the layer presents and
   answers; DOS keeps its calls. (b) The same grid, and the program
   waits for events; DOS's order of calls changes. (c) A stream of
   text and terminal sequences. Recommended: (a). **Answered:** (a);
   see decision 1.
2. **The screen of commands, outside DOS.** (a) In a terminal, panels
   on the alternate screen, commands on the main one; Ctrl-O shows the
   main one; behind half the panels nothing, as a terminal cannot be
   read back. (b) Commands in a pseudo-terminal with an emulator of our
   own, as on DOS. Recommended: (a) now, (b) with a window of our own.
   **Answered:** (a) now and (b) later, with libvterm, in a terminal
   and not with a window of our own; see decision 2.
3. **The tap on Alt, in a terminal that cannot report it.** (a) Use the
   kitty protocol or win32-input-mode where the terminal has them;
   elsewhere no tap and no key bar of modifiers, F9 opens the menu; a
   declared deviation. (b) Make Esc, tapped, open the menu there.
   Recommended: (a). **Answered:** (a); see decision 3.
4. **Terminal or window, on each desktop.** (a) A terminal on Linux,
   macOS and Windows first, one implementation over VT sequences; a
   view of its own written for iOS, used on macOS later through Mac
   Catalyst. (b) A window of its own on each desktop from the start.
   Recommended: (a). **Answered:** a terminal on all three, with no
   native app on macOS later; iOS draws a view of its own; see
   decision 4.
5. **The size of the screen.** (a) 80 columns, any number of rows
   from 25 up, as now; a larger screen shows the 80 at the left, or
   centred, and a smaller one is refused. (b) Columns follow the
   screen too: every layout of the program changes. Recommended: (a).
   **Answered:** (a); see decision 5.
6. **The 16 colours in a terminal.** (a) The PC's exact colours in 24
   bits where the terminal has them, the named 16 otherwise. (b) The
   terminal's named 16 always: the user's theme. Recommended: (a).
   **Answered:** (a); see decision 6.
7. **Characters two cells wide**, in names of files and in the iPhone's
   drives. (a) The layer says each character's width, and the panels
   count cells. (b) Every character one cell; one that a platform shows
   wider is replaced by a substitute. Recommended: (a). **Answered:**
   (a); see decision 7.
8. **Touch, on iPhone and iPad.** (a) A tap is the left button, a long
   press the right one; the key bar is tapped for F1 to F10; a row of
   Esc, Tab, Ctrl, Alt and arrows above the on-screen keyboard; on an
   iPhone, landscape only. (b) The same, portrait too, with a small
   font. Recommended: (a). **Answered:** (a); see decision 8.
9. **What names a key.** (a) Its place on the keyboard, as a USB HID
   usage, which iOS and SDL use, and the character it typed; Alt with a
   Cyrillic letter searches by the Latin key, as on DOS. (b) Only named
   keys and the character; such a search by the Cyrillic letter.
   Recommended: (a). **Answered:** (b), against the recommendation;
   see decision 9, with a point to settle for DOS.
10. **Function keys where the keyboard has none** (iPad, Mac). (a) Cmd
    with a digit for F1 to F10, and the key bar tapped or clicked. (b)
    The key bar only. Recommended: (a). **Answered:** (a); see
    decision 10.

## What is unsure

- Whether UIKit reports a modifier pressed alone, and a held key's
  repeats, to `pressesBegan`: not in the documentation found; to be
  tried on a device. If not, the tap on Alt on iOS needs another way,
  and repeats a timer of the layer's.
- Whether the Windows console really keeps Alt alone from the program,
  as its documentation says [W1], or only in some hosts.
- How macOS Terminal reports the mouse and which keys it passes; it is
  not on the kitty protocol's list [K1].
- The estimates of work: none was measured.

## Sources

[D1]: https://www.ctyme.com/intr/rb-1754.htm "Ralf Brown's Interrupt List, INT 16h AH=00h (read in a mirror of the list)"
[W1]: https://learn.microsoft.com/en-us/windows/console/key-event-record-str "KEY_EVENT_RECORD"
[W2]: https://learn.microsoft.com/en-us/windows/console/classic-vs-vt "Classic Console APIs versus Virtual Terminal Sequences"
[W3]: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences "Console Virtual Terminal Sequences"
[W4]: https://learn.microsoft.com/en-us/windows/console/writeconsoleoutput "WriteConsoleOutput"
[W5]: https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md "Improved keyboard handling in ConPTY (win32-input-mode), known from a search, not read whole"
[X1]: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html "XTerm Control Sequences"
[K1]: https://sw.kovidgoyal.net/kitty/keyboard-protocol/ "Comprehensive keyboard handling in terminals (the kitty protocol)"
[U1]: https://mitchellh.com/writing/grapheme-clusters-in-terminals "Grapheme Clusters and Terminal Emulators"
[U3]: https://www.unicode.org/reports/tr11/ "UAX #11, East Asian Width (the values checked with Python's unicodedata, Unicode 17.0)"
[L1]: https://man7.org/linux/man-pages/man4/vcs.4.html "vcs(4)"
[L2]: https://manpath.be/f7/1/mc "mc(1), known from a search, not read whole"
[L3]: https://github.com/elfmz/far2l "far2l"
[L4]: https://lists.suckless.org/dev/2608/36006.html "st and DECSET 2026, known from a search"
[A1]: https://developer.apple.com/documentation/uikit/uikey "UIKey"
[A2]: https://developer.apple.com/documentation/uikit/handling-key-presses-made-on-a-physical-keyboard "Handling key presses made on a physical keyboard"
[A3]: https://developer.apple.com/documentation/uikit/uikeycommand/wantspriorityoversystembehavior "wantsPriorityOverSystemBehavior"
[A4]: https://developer.apple.com/documentation/uikit/uikeyboardhidusage "UIKeyboardHIDUsage"
[A5]: https://developer.apple.com/documentation/uikit/uievent/buttonmask-swift.property "UIEvent.buttonMask"
[A6]: https://developer.apple.com/documentation/uikit/uihovergesturerecognizer "UIHoverGestureRecognizer"
[A7]: https://developer.apple.com/documentation/uikit/uipointerinteraction "UIPointerInteraction"
[A8]: https://developer.apple.com/documentation/uikit/uikeyinput "UIKeyInput"
[A9]: https://developer.apple.com/documentation/uikit/uiresponder/inputaccessoryview "inputAccessoryView"
[A10]: https://developer.apple.com/documentation/appkit/nsresponder/flagschanged(with:) "flagsChanged(with:)"
[S1]: https://wiki.libsdl.org/SDL3/SDL_Scancode "SDL_Scancode"

- [D1] Ralf Brown's Interrupt List, INT 16h 00h; read in a mirror of the
  list, `rb-1754.htm`.
- [W1] to [W4] Microsoft Learn, the Windows console: `KEY_EVENT_RECORD`;
  "Classic Console APIs versus Virtual Terminal Sequences"; "Console
  Virtual Terminal Sequences"; `WriteConsoleOutput`.
- [W5] Windows Terminal's spec 4999, win32-input-mode: known from a
  search's account of it, not read whole.
- [X1] XTerm Control Sequences.
- [K1] The kitty keyboard protocol.
- [U1] Mitchell Hashimoto, "Grapheme Clusters and Terminal Emulators",
  October 2023.
- [U3] Unicode Standard Annex 11, East Asian Width; the values of the
  characters named here taken from Python's `unicodedata`, Unicode 17.0.
- [L1] `vcs(4)`. [L2] `mc(1)`, from a search. [L3] far2l's README.
  [L4] The suckless mailing list on mode 2026, from a search.
- [A1] to [A10] Apple's documentation, UIKit and AppKit.
- [S1] SDL3's `SDL_Scancode`.
