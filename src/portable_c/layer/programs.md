# The layer: running programs, and the program's life

**Draft of 13 October 2026. Nothing in it is decided.** It is one area
of step 3 of the road (`CLAUDE.md`, "The portable C"), surveyed from the
platforms' side, as the user asked: what each target platform provides,
where they agree, where one has another shape, what common ground there
is, and only then how the program's needs map onto it. `LAYER.md` has the
user's decisions so far and the inventory of what the program asks; this
file takes both as given.

How to read it:

- A cell or a sentence that rests on a page of documentation that was
  read for this draft has that page among "Sources" at the end, marked
  **read**. One that rests on what is generally known of a platform and
  was not looked up this time is marked †. A sentence that begins "So"
  or "That means" is an inference, and says so.
- Nothing was tried on any platform but DOS. What is said of DOS is read
  in the program, `sys.h`, `dos32.c` and its callers, at f7ae44e.

## What the program does in this area today

From `LAYER.md`, "Other programs, and the end of this one" and "What the
system asks the program", and from the code.

- **A command line.** What the user types is run by `Exec` of `exec.c`:
  for each command of `DosBuf`, COMMAND.COM by the name that `COMSPEC`
  has in the environment, with `/C` and the command for its tail, DOS
  4B00h. Between two commands the program writes a prompt and the
  command on the screen itself, as a shell would. The program does not
  read a command's exit status; it reads only whether DOS could run the
  shell, and says "not found" (with a retry) or "cannot run" otherwise.
- **Enter on a file.** A directory is entered. A file that DOS runs by
  its name, `.com`, `.exe`, `.bat`, is typed into the command line and
  run. Any other file is run by the line that the extension file,
  `VC.EXT`, has for its extension; with none, nothing happens.
- **The user's menu** (F2) and the extension file make commands with
  `!`, `!.!`, `!:`, `!\` and `!@`, the last a file listing the selected
  files, written into the directory that `TEMP` names.
- **A viewer or an editor of another's** (`Aexec0` of `view.c`): run by
  `ProgramRun` with a tail that is the file's name and a block of bytes
  in Norton Commander's form (`ViewPar`, with the palette and the mouse),
  zero bytes among them.
- **The screen behind the panels.** Before a command the panels go, the
  screen is as DOS left it; after it, the program reads the screen back
  (`SavScr`, `ScreenGet`) and draws its panels over that. Ctrl-O and the
  half-panels show what the commands wrote.
- **What the program is started with**: a path in its command tail
  (`ProgramTail`); the environment, of which it reads `COMSPEC`, `VC`
  (the directory of its files) and `TEMP`; and the path of its own file,
  which DOS keeps behind the environment, for where its files are when
  `VC` is not set: `VC.INI`, `VC.MNU`, `VC.EXT`, `VCVIEW.EXT`,
  `VCEDIT.EXT`, `VC.HLP`.
- **Stops.** Ctrl-Break and Ctrl-C are taken from DOS and the BIOS so
  that nothing happens for them; a long piece of work, a copy or a
  search, looks at the keys now and then (`TstBrk`, 15 places) and
  stops for Ctrl-C, Esc, F10 or what Ctrl-Break leaves.
- **The critical error.** DOS asks the program what to do about a device
  that fails in the middle of a call, and the program answers by a box
  with Retry and Abort, `FtErr1`, inside that call.
- **The end.** By itself, F10; or by DOS at another program's word, when
  nothing of the program runs and the system puts back what the program
  said beforehand (`ProgramEnded...`).
- **Idle.** Each time the program looks for a key and finds none it says
  so, INT 28h; the harness's driver knows by that that a step is over.

Under every shape below, the DOS implementation does exactly what it does
today: the same calls of DOS, in the same order, with the same bytes.

## What the platforms provide

### Running a command and waiting for it

| | DOS | POSIX (Linux, a terminal program on macOS) | Windows | macOS, a native app | iOS |
|---|---|---|---|---|---|
| The call | 4B00h: a file, a tail, an environment | `fork`/`exec`, or `posix_spawn` (argv, envp; a directory to go to since POSIX 2024) | `CreateProcessW`: a file, one command-line string | as POSIX; `Process` (`NSTask`) on top of it | none: no child processes for an app (Apple, forum answer) |
| A command line through the shell | `COMSPEC` with `/C` and the command; the shell is the user's | `sh -c`; ISO C's `system` is that, and ignores SIGINT and SIGQUIT while it waits | `cmd.exe /c`, needed for a batch file; `COMSPEC` names it †; the CRT's `system` uses it † | as POSIX, but with no terminal to hand over | none; `system` is not in the SDK (ios_system) |
| Waiting | the call returns when the program has ended | `waitpid` | `WaitForSingleObject`, `GetExitCodeProcess` | as POSIX | none |
| What it may run | anything that DOS loads | anything with the right to execute | anything; a document by its association only through `ShellExecuteEx` | as POSIX; inside the App Sandbox a child inherits the sandbox | only code in the app's bundle; nothing downloaded (Guideline 2.5.2) |
| Memory | the caller has to give memory back first | none to give | none | none | none |

### The terminal, the command's output and its input

| | DOS | POSIX | Windows | macOS, a native app | iOS |
|---|---|---|---|---|---|
| Who has the screen while a command runs | the command, which writes where the cursor is | the command, once the terminal is its (`tcsetpgrp`) and the program's modes are put back (`tcsetattr`) | the command, which inherits the console with flags 0 | nobody: there is no terminal; the app has to host one, a pseudo-terminal and something that draws it | none |
| What the program has of the output afterwards | the screen, read back from the display's memory | nothing: no sequence of a terminal gives the characters back (xterm's list); the Linux console alone can be read, `/dev/vcsa` | the console's buffer, `ReadConsoleOutput`: supported indefinitely, off the roadmap, "the client application is expected to remember its own drawn state" | what its pseudo-terminal gave, as text and escape sequences, which it has to interpret | none |
| Keeping both | the program draws over what it read | the alternate screen, `1049`: the panels on one screen, the output on the other, one or the other shown, never both; Midnight Commander's Ctrl-O is that | a second screen buffer of the console, and the first read back † | the app keeps it, being the terminal | none |
| Hosting the command | | a pseudo-terminal (`posix_openpt` †), its output read by the program | ConPTY: pipes of text and escape sequences, which the host draws; threads advised; Windows 10 1809 † | that is the only way | none |
| The terminal left as it was | DOS does not change modes | a command may leave the terminal "in an odd state": the shell saves and puts back the modes (glibc) | a command may change the console's modes; put them back † | the app's own | none |

### The exit status

| DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|
| 4Dh, a byte; the program never asks | `WEXITSTATUS`, a byte, or a signal; 127 where the shell or the child could not start | a 32-bit number, `GetExitCodeProcess` | as POSIX | none; a Shortcut answers with text or an error (below) |

### Running a file by its kind

| | DOS | POSIX | Windows | macOS | iOS |
|---|---|---|---|---|---|
| The system's way | none: `.com`, `.exe`, `.bat` run by name, all else is the program's extension file | none in POSIX; on a desktop, `xdg-open` (exit 0, or 1 to 4 for an error) | `ShellExecuteEx`, verb "open", "edit", "openas" for a chooser; may start no process, and gives no handle to wait on then | `NSWorkspace.open`, in the default app or a named one; outside the sandbox of the program | `UIDocumentInteractionController`: a preview, "Open in" another app; the share sheet, `UIActivityViewController`; `UIApplication.open` for a URL |
| How the kind is known | the extension | the extension or the contents, by the desktop's database † | the extension, by the registry † | Uniform Type Identifiers, from the extension or the file | Uniform Type Identifiers |
| Does the program wait | yes | no † | only where a process was started and handed back | no | no: the user goes to the other app |
| "Runnable" | the three extensions | the execute permission | `PATHEXT` † | the execute permission, or an `.app` | nothing |

### The environment and the arguments

| | DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|---|
| Arguments | the command tail, one text of at most 126 characters | `argv`, texts, no limit that matters | one command line of up to 32,767 characters; `argv` by the C runtime's rules † | `argv` from a terminal; from the Finder, documents opened by an event † | none; a URL or a document the app is opened with † |
| The environment | DOS's block, `NAME=value` with a zero byte each; the path of the program behind it | `environ`, `getenv` (ISO C) | a block of the same form, in UTF-16 with `CREATE_UNICODE_ENVIRONMENT`, and `=C:` variables for each drive's directory | `environ`; an app started by the Finder gets little of the user's † | `getenv` works, the variables are the system's † |
| Handed to a child | a copy, or a block that is named | `envp` | `lpEnvironment`; with a block of its own the `=C:` variables have to be put in by hand | as POSIX | none |
| The shell's name | `COMSPEC` | `SHELL` for the user's; `sh` is what `system` uses | `COMSPEC` † | as POSIX | none |

### The program's own path, and where it keeps its files

| | DOS | POSIX | Windows | macOS | iOS |
|---|---|---|---|---|---|
| The program's file | behind the environment | Linux: `/proc/self/exe`, Linux's alone; others differ † | `GetModuleFileNameW` † | `_NSGetExecutablePath` †; an app's bundle | the bundle, read-only |
| Settings | `VC.INI` beside the program, or where `VC` says | `$XDG_CONFIG_HOME`, `~/.config` | `%APPDATA%`, `SHGetKnownFolderPath` † | `~/Library/Application Support/<id>`, a container if sandboxed | `Library/Application Support`, backed up |
| History, state | in memory only | `$XDG_STATE_HOME`, `~/.local/state`: "command history" belongs there | `%LOCALAPPDATA%` † | Application Support | Application Support |
| Files the user edits, the menus | beside the program | config | AppData, or beside a portable program † | Application Support | `Documents`, which the Files app shows, or the app's iCloud container |
| A temporary file | `TEMP`, else the program's directory | `TMPDIR`, `/tmp` † | `GetTempPathW` † | `NSTemporaryDirectory` † | `tmp`, purged when the app is not running |

### Being interrupted: the stop keys

| | DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|---|
| Ctrl-C | DOS calls INT 23h at a call that looks for it; the program takes it | a key, byte 03h, where the terminal is raw (`ISIG` off) †; else SIGINT, which ends a program by default | a key where `ENABLE_PROCESSED_INPUT` is off †; else `CTRL_C_EVENT`, which ends it by default | a key | a key from a hardware keyboard † |
| Ctrl-Break | INT 1Bh of the BIOS, taken | none | `CTRL_BREAK_EVENT`, whatever the mode; a handler that answers TRUE keeps it from ending the program | none | none |
| While a command runs | DOS's: Ctrl-C ends the command, as DOS is set | the command's, being in the foreground; the program ignores SIGINT, as `system` does | every process of the console gets the event; the program's handler ignores it † | the hosted command's, by what the app sends it | none |
| Stopped and continued | none | SIGTSTP stops a program by default, Ctrl-Z where `ISIG` is on; SIGCONT goes on | none | none | none |
| The size of the screen changes | never | SIGWINCH, ignored by default | an event in the console's input † | the app's window | the app's scene |

### Being ended, and the app's life

| | DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|---|
| By itself | 4Ch | `exit`; the terminal put back first | `ExitProcess` | as POSIX | an app does not end itself † |
| Asked to end | never; DOS ends it by another's hand, with nothing of it run | SIGTERM; SIGHUP when the terminal goes; both end by default | `CTRL_CLOSE_EVENT`, then ended after 5 seconds | the app is asked to quit † | not asked: in the background it is suspended, and a suspended app is ended without a word to free memory |
| What it has to do first | say beforehand what the system is to put back | put the terminal back | what it can in 5 seconds | save | save when it leaves the foreground: about 5 seconds, more by `beginBackgroundTask`; come back where it was by state restoration (`NSUserActivity`) |
| Started again | afresh | afresh | afresh | afresh, or with its windows restored † | the user expects to find it as it was |

### Errors put to the user's face

| DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|
| INT 24h, inside the call that failed: a drive not ready, a disk protected, a printer without paper; the handler answers retry, fail, ignore or abort | none: the call fails with `errno`, `EIO`, `EROFS`, Linux's `ENOMEDIUM` † | the system shows its own box for a disk not in the drive unless the program says `SEM_FAILCRITICALERRORS`, which Microsoft calls best practice; then the call fails, `ERROR_NOT_READY` † | the call fails; the Finder may ask for a disk † | the call fails; an iCloud file that is not yet downloaded, or a provider that is offline, is a condition that waiting can cure † |

### Idle

| DOS | POSIX | Windows | macOS, native | iOS |
|---|---|---|---|---|
| INT 28h, once each time no key is found: other programs get a turn | wait for the terminal or a time, `poll` † | wait for the console's input or a time, `WaitForSingleObject` † | the app's run loop has the time, and gives the program an event † | the same; the app does not own its loop † |

## The differences that matter

1. **iOS has no command line to run.** No child process, no shell, no
   `system`, and no code from outside the app's bundle. What it has is
   handing a file or a URL to another app, without waiting and without
   an answer, and running a Shortcut by name with a text, which answers
   with a text by `x-success` and `result`, by way of the Shortcuts app
   coming to the front and going back. A command line on iOS is either
   gone or something other than a command line.
2. **A native app on macOS has processes and no terminal.** Running a
   command there means hosting it: a pseudo-terminal, and something that
   reads what the command writes, escape sequences and all, into cells.
   A terminal program on macOS is POSIX and has neither problem.
3. **What a command wrote stays readable on DOS, on the console of
   Windows and on the Linux console, and nowhere else.** In a terminal
   emulator, which is where a program for Linux or macOS runs, nothing
   gives the characters back. The alternate screen keeps the two apart,
   panels or output, one at a time. The original's half-panels over the
   output, and its panels drawn over what the commands left, need the
   cells: in a terminal that means hosting the command too (finding 2),
   at the cost of a terminal emulator in the program.
4. **The shell exists everywhere but iOS, and has the same shape:**
   `COMSPEC /C` on DOS and Windows, `sh -c` on POSIX. ISO C's `system`
   is exactly that on POSIX and on Windows. So a "run this command line
   through the user's shell, and wait" is common ground, and standard C,
   as the user asked, where a platform has it.
5. **The terminal is handed over and taken back.** On DOS nothing is
   asked for that. On POSIX and Windows the program puts back the modes
   that it changed for its own keys before the command, and its own
   after it, and ignores the stop keys while the command has them.
6. **Running a file by its kind is the program's on DOS, and the
   system's elsewhere.** DOS has no associations; the extension file is
   the program's own. Every other platform has an opener, which does not
   wait and on iOS takes the user away.
7. **The stop keys can be plain keys everywhere.** In a raw terminal, a
   console without processed input, a native app, Ctrl-C arrives as a
   key, as `TstBrk` reads it on DOS. What a platform does about the
   keys is only to keep the system from ending the program for them:
   INT 23h and 1Bh on DOS, a handler for Ctrl-Break on Windows, nothing
   on POSIX while the terminal is raw.
8. **The critical error is DOS's shape, but its question is not.** DOS
   asks inside the call and repeats it itself on Retry. Windows has the
   same box and lets a program have the error instead. The other
   platforms only fail the call. Conditions that waiting can cure, a
   disk not in its drive, a file of iCloud not yet here, are on every
   platform, and only the program can ask the user about them.
9. **Being ended goes from "never asked" to "always without a word".**
   DOS never asks, but runs nothing of the program; POSIX and Windows
   ask with a little time; iOS suspends and then ends without asking,
   and expects the app to come back as it was.
10. **The program's files belong somewhere else on each platform**, and
    only DOS has them beside the program.
11. **The program owns its loop on DOS, POSIX and Windows, and not in a
    native app.** There the system's run loop has the time; a program
    that waits for keys in a loop of its own would run on a thread of
    its own and be given events. That is of the keyboard's area as much
    as this one.

## The candidate shapes

Three shapes for the part of the layer that runs things. A and B differ
in what the program has of a command's output; C is how the interface is
asked, and goes with either.

### A. Hand the terminal over: the shell runs the command line

The layer has: run a command line through the user's shell and wait,
answering whether the shell could be run and how the command ended; open
a file by its kind; the arguments and the environment by name; where the
program's files go. While a command runs the command has the terminal;
the layer of the screen gives it up before and takes it back after.

- **DOS.** Today's calls. Run is `Exec02`'s row: 3301h with what DOS had,
  4B00h of `COMSPEC` with `/C`, 3300h, 3301h, 25h for INT 23h; the tail
  as the program has it in `DosBuf`, with the 128 bytes that DOS copies.
  The screen is read back after, as today.
- **POSIX in a terminal.** `system`, or `posix_spawn` of `$SHELL -c` with
  SIGINT ignored and the terminal's modes put back around it. The output
  is on the terminal's main screen, the panels on the alternate one:
  Ctrl-O switches, and no panel stands over the output.
- **Windows.** `CreateProcessW` of `cmd /c`, the console inherited; the
  program's screen in a second buffer of the console, so the output can
  be read back with `ReadConsoleOutput` as on DOS, an interface that is
  kept but not developed.
- **macOS native, iOS.** None. Run answers "cannot here".
- **What the program changes**: the prompt and the echo of the command
  stay the program's; the half-panels over the output are DOS's and
  Windows's only.
- **Cost**: least. One function of a few lines on POSIX and Windows.

### B. Host the command: the layer reads its output into cells

As A, but where a terminal emulator can read the output the layer runs
the command on a pseudo-terminal it owns, passes what the command writes
on to the real terminal as it comes, if there is one, and keeps it as
cells too, so the program has the screen behind its panels as on DOS.

- **DOS.** As A: the display's memory is the cells.
- **POSIX in a terminal.** `posix_openpt`, a child on its slave side, a
  loop that copies the master to the terminal and into an emulator's
  cells, the window's size passed on with SIGWINCH. A full-screen program,
  an editor, runs on the real terminal as before, since its output is
  passed on; where the emulator falls short, only the picture behind the
  panels is wrong.
- **Windows.** ConPTY, with the same emulator, or the console's buffer as
  in A.
- **macOS native.** This is the only way to run a command there.
- **iOS.** No processes; a Shortcut's text could be shown in the cells,
  if commands on iOS are Shortcuts (question 1).
- **What the program changes**: nothing beyond A; Ctrl-O and the
  half-panels work everywhere there are processes.
- **Cost**: a terminal emulator in the program, a VT parser of some
  thousand lines at the least; a loop that copies while waiting, threads
  on Windows; job control (Ctrl-Z) inside the hosted terminal.

### C. Ask by what the user wants, with what the platform can do

The interface names what the user asks for, not processes: run a command
line, run a program with arguments, open a file by its kind, open it with
a named application, hand it to another app. The program asks first which
of these the platform has, and shows only those: on iOS no command line,
no user menu of commands, no Shift-F2 of the extension file; Enter on a
file opens it in another app or a preview. Each request answers one of:
ended, with a status; handed on, no waiting; not here; failed, with the
layer's error. Under it, run is A or B.

- **DOS.** Every request DOS has is today's calls; "open by kind" and
  "hand to another app" answer "not here", which is what the program does
  today for a file the extension file does not name.
- **Others.** As A or B for running, and the platform's opener for the
  rest.
- **What the program changes**: menus and keys that it has look at what
  the platform has; on iOS a large part of the program is not reached.
- **Cost**: small in the layer; the program has places to make
  conditional, and nothing is shown on DOS for them.

### What each costs on each platform

| | A | B | C |
|---|---|---|---|
| DOS | today | today | today |
| Linux terminal | small; output and panels one at a time | an emulator; output behind panels | as A or B |
| Windows console | small; read-back as on DOS | ConPTY and the emulator, or as A | as A or B |
| macOS terminal | as Linux | as Linux | as A or B |
| macOS native | nothing runs | the only way | as B |
| iOS | nothing runs | Shortcuts' text at most | the only shape that says what iOS can do |

## Recommendation

**C's interface, with A underneath first, and B when a platform without a
terminal comes.**

Why: the user's iOS shows that the program cannot assume that a command
can be run at all, so the interface must say what a platform can do, and
C does that at little cost, and nothing of it shows on DOS. Running a
command line through the user's shell is the one thing that DOS, POSIX
and Windows share, and on two of them it is standard C's `system`. A is
what a Linux terminal program does as a matter of course, and it is the
next platform on the road. B is needed the moment the program is a
native app on macOS, and it is the only way to keep the original's
half-panels over the output in a terminal; it costs a terminal emulator,
which is a project of its own, and should not hold up the first port.
The functions are the same under A and B: what differs is whether the
screen's layer can answer "what is behind the panels" on that platform,
so the interface says that it may not.

## The common ground, with its trade-offs

- **Run a command line through the user's shell, in a directory, and
  wait** (DOS, POSIX, Windows). Answers whether the shell could be run,
  "not found" apart from other failures, as the program asks today, and
  the command's status, which the program does not read but every
  platform has. Trade-off: the shell is the platform's (`COMSPEC`, `sh`
  or `$SHELL`), so what the user types means what it means there, which
  is what the user expects of a command line.
- **Run a program with arguments and wait** (DOS, POSIX, Windows).
  Arguments as a list of texts on POSIX and Windows; on DOS one tail.
  Trade-off: the original's tail for a viewer of another's has bytes
  that are no text (`ViewPar`), which only DOS can hand on.
- **Open a file by its kind, without waiting** (Windows, Linux desktop,
  macOS, iOS; DOS none). Trade-off: the user leaves the program for
  another; on iOS there is no coming back by itself.
- **What a platform can do**, asked once: run, open, the screen behind
  the panels. Trade-off: the program has places that look at it.
- **The arguments and the environment by name** (all; iOS's are the
  system's). `getenv` is ISO C. The program's own file and where its
  files go are not standard C anywhere, and each platform has its own
  places: one function "where the program's settings are", one "where a
  temporary file goes".
- **The stop keys as keys** (all). The layer only keeps the system from
  ending the program for them. Ctrl-Break, the PC's key, is a key of DOS
  and Windows alone.
- **A question to the user from inside a call**: "a device needs the
  user: retry or fail?", a function that the program hands the layer.
  DOS calls it from INT 24h, as today; Windows from the layer, after
  `SEM_FAILCRITICALERRORS`, for a drive not ready; iOS for a file of
  iCloud not downloaded or a provider offline; POSIX for a medium that is
  missing, where the platform says so. Trade-off: elsewhere than on DOS
  the layer decides which errors are of that kind, and repeats the call
  itself on Retry.
- **The program asked to end, or about to be ended**: an event in the
  program's loop, with "save now" for iOS's background. DOS has neither,
  and keeps `ProgramEnded...` in its own file. Trade-off: the program
  decides what it saves, which is a matter for the user (question 7).
- **Idle** becomes "nothing to do until a key, an event or a time": INT
  28h and a return on DOS, as today; a wait elsewhere. The keyboard's
  area has the same question.

## How the program's needs map onto the recommendation

"Today" is the function of `sys.h`; "DOS" is what its implementation does
under the recommendation, which is what it does now.

| Today | In the layer | DOS | Elsewhere |
|---|---|---|---|
| `ProgramRunBare` of `COMSPEC` with `/C`, in `Exec02`, with `SetBreakCheck` before and `SetBrk` after | run a command line through the shell | the same row: 3301h, 4B00h, 3300h, 3301h, 25h; the tail in place in `DosBuf`, the 128 bytes behind it read as DOS reads them, said at the declaration as the 68 bytes of the environment are | `system`-like, A; hosted, B; "not here" on iOS |
| `md.ComSpec`, the shell's place in the environment, and `NotFind` with "COMMAND.COM" | the shell's name, or none | the environment's `COMSPEC`, as now | `$SHELL` or `sh` (question 3); `COMSPEC` |
| `ProgramRun` of `Aexec0`, the viewer or editor of another's | run a program with arguments | the tail with `ViewPar`, made in a file of DOS's, `view_dos.c` | the file's name as the one argument; the Norton block is DOS's alone, a removal of the kind that is declared without asking |
| Enter on `.com`, `.exe`, `.bat` | "runnable", asked of the platform | the three extensions, as now | the execute permission; `PATHEXT`; none on iOS |
| Enter on another file with no line in `VC.EXT` | open by its kind | not here: nothing, as now | the platform's opener (question 5) |
| `ProgramTail` | the arguments, as texts | the tail, as now | `argv`, converted to UTF-8 |
| `ProgramEnvironment`, read for `COMSPEC`, `VC`, `TEMP` and the path behind it | a variable by name; where the program's files are; where a temporary file goes | DOS's block, read as now | `getenv`; the platform's places (question 4) |
| `ProgramEnd`, `ProgramEndOld`, `SystemVersion` | end, with a number | as now; the old DOS stays DOS's | `exit` with the terminal put back |
| `ProgramEndedBreakCheck`, `Take`, `Give` | none in the common code | DOS's own file | the system's "asked to end" event |
| `DeviceErrorFn`, `Note`, `Take`, `Give`, `IntOn` | a function that the program hands the layer: retry or fail | INT 24h, as now; `IntOn` into the DOS file's screen writes | called by the layer for the conditions it knows (question 6) |
| `CtrlBreakNote`, `Take`, `Give`, `CtrlCTake`, `BreakCheck`, `SetBreakCheck` | the stop keys as keys | INT 1Bh and 23h, and 3300h and 3301h of DOS, as now, in the same places | a raw terminal; a console handler; nothing |
| `Idle` | wait for something to do | INT 28h, as now | `poll`; a wait; the run loop |
| `SavScr` and `ScreenGet` after a command | the screen behind the panels, which a platform may not have | the display's memory | the console's buffer; the emulator's cells; the alternate screen where there are none |

## Questions for the user

Each with its ways, the recommended one first.

1. **The command line on iOS**, where nothing can be run. (a) None: the
   command line, the user menu's commands and the extension file are
   hidden; Enter opens a file in another app or a preview. (b) A command
   runs a Shortcut by name; its text shows behind the panels. (c)
   Commands built into the app.
2. **A command's output behind the panels, in a terminal** (Linux, macOS).
   (a) For now, the command has the real terminal; Ctrl-O shows the
   terminal's own screen; no panel stands over the output. Hosting comes
   with the native apps. (b) Host commands now, with a terminal emulator
   in the program, so it looks as on DOS.
3. **The shell on Linux and macOS.** (a) The user's, `$SHELL -c`, and
   `/bin/sh` if none is set: the command line means what it means at the
   user's prompt. (b) Always `/bin/sh -c`, as C's `system` does: the same
   on every machine.
4. **Where the setup, menus, extension files and help live off DOS.** (a)
   The platform's place for a user's settings: `~/.config` on Linux,
   AppData on Windows, Application Support on Apple's; `VC` still
   overrides. (b) Beside the program, as on DOS.
5. **Enter on a file that `VC.EXT` has no line for.** It does nothing in
   the original. (a) Hand it to the platform's opener where there is one;
   DOS has none, so nothing changes there. This is a deviation, declared.
   (b) Nothing, as the original.
6. **The box of a device that fails, off DOS.** (a) Keep it, Retry and
   Abort, for what waiting can cure: a disk not in its drive, an iCloud
   file not yet downloaded. (b) DOS only; elsewhere such a failure is an
   ordinary error.
7. **iOS ends the program in the background without a word.** (a) It
   comes back where it was: panels, directories, command line, by the
   system's state restoration; nothing is saved into the setup that the
   user did not save. (b) It starts afresh, as on DOS.
8. **Ctrl-Z on Linux and macOS.** (a) A key, as on DOS: the program is
   not stopped by it; a stop that another program sends stops it with
   the terminal put back, and it redraws when continued. (b) It stops the
   program, as in most terminal programs.
9. **macOS: a terminal program or a native app?** (a) A terminal program
   first, as Linux, at little cost; a native app later, sharing iOS's
   work. (b) A native app from the start, with commands hosted.

## What is not sure

- Every cell marked †. The Apple pages that were read are summaries of
  the pages' text, and two of them, iOS's processes and macOS's sandbox,
  rest on answers of Apple's engineers in Apple's forum and on a third
  party's library, not on reference documentation.
- That `ReadConsoleOutput` gives the output back under Windows Terminal,
  through ConPTY, as it does in the classic console window. Inferred, not
  read.
- How large a terminal emulator good enough for shape B is; the figure
  of some thousand lines is a guess.
- Whether the Shortcuts app's round trip, out of the program and back,
  is bearable for a command line (question 1, b).
- Whether the list of platforms is complete. Android is not on it: it
  has processes, unlike iOS, and limits what an app may execute; if it
  ever comes, it sits between Linux and iOS here. And iPhone and iPad
  differ in what matters to a program of function keys, a keyboard,
  which is the keyboard's area.

## Sources

Read for this draft:

- POSIX: [`posix_spawn`](https://pubs.opengroup.org/onlinepubs/9799919799/functions/posix_spawn.html),
  [`system`](https://pubs.opengroup.org/onlinepubs/9799919799/functions/system.html).
- glibc, [Foreground and Background](https://sourceware.org/glibc/manual/latest/html_node/Foreground-and-Background.html),
  of "Implementing a Job Control Shell".
- Linux: [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html),
  [/proc/pid/exe](https://man7.org/linux/man-pages/man5/proc_pid_exe.5.html),
  [vcs(4)](https://man7.org/linux/man-pages/man4/vcs.4.html).
- freedesktop: [XDG Base Directory](https://specifications.freedesktop.org/basedir/latest/),
  [xdg-open](https://portland.freedesktop.org/doc/xdg-open.html).
- xterm, [Control Sequences](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html).
- Midnight Commander's FAQ on Ctrl-O, quoted in
  [Ubuntu's page on mc](https://help.ubuntu.com/community/mc).
- Windows: [`CreateProcessW`](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw),
  [`HandlerRoutine`](https://learn.microsoft.com/en-us/windows/console/handlerroutine),
  [`ReadConsoleOutput`](https://learn.microsoft.com/en-us/windows/console/readconsoleoutput),
  [Creating a Pseudoconsole session](https://learn.microsoft.com/en-us/windows/console/creating-a-pseudoconsole-session),
  [`SHELLEXECUTEINFOW`](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-shellexecuteinfow),
  [`SetThreadErrorMode`](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setthreaderrormode).
- Apple: [Managing your app's life cycle](https://developer.apple.com/documentation/uikit/managing-your-app-s-life-cycle),
  [Extending your app's background execution time](https://developer.apple.com/documentation/uikit/extending-your-app-s-background-execution-time),
  [Restoring your app's state](https://developer.apple.com/documentation/uikit/restoring-your-app-s-state),
  [`UIApplication.open`](https://developer.apple.com/documentation/uikit/uiapplication/open(_:options:completionhandler:)),
  [`UIDocumentInteractionController`](https://developer.apple.com/documentation/uikit/uidocumentinteractioncontroller),
  [`UIActivityViewController`](https://developer.apple.com/documentation/uikit/uiactivityviewcontroller),
  [Uniform Type Identifiers](https://developer.apple.com/documentation/uniformtypeidentifiers),
  [`NSWorkspace`](https://developer.apple.com/documentation/appkit/nsworkspace),
  [File System Programming Guide](https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html),
  [Embedding a helper tool in a sandboxed app](https://developer.apple.com/documentation/xcode/embedding-a-helper-tool-in-a-sandboxed-app),
  [App Review Guidelines](https://developer.apple.com/app-store/review/guidelines/), 2.5.2 and 4.7,
  Shortcuts: [Run a shortcut from a URL](https://support.apple.com/guide/shortcuts/apd624386f42/ios),
  [Use x-callback-url](https://support.apple.com/guide/shortcuts/apdcd7f20a6f/ios).
- Apple's forum, an engineer's answer that iOS apps may not start child
  processes: [thread 747499](https://developer.apple.com/forums/thread/747499).
- [ios_system](https://github.com/holzschu/ios_system), a third party's
  replacement for `system` on iOS, which says that iOS refuses `system`.
- DOS: Ralf Brown's Interrupt List, [INT 21h, 4Bh](https://mirror.cs.msu.ru/oldlinux.org/Linux.old/docs/interrupts/int-html/rb-2939.htm),
  read on a mirror; INT 23h, 24h, 1Bh and 28h as the program and
  `dos32.c` use them.
