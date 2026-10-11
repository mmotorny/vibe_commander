# Instructions for Claude

Standing instructions for working in this repository. Record new ones here,
not in Claude's memory: memory does not travel between computers.

## Working together

- Give an honest opinion, including disagreement, not compliance.

## Third-party software

- Never commit anything proprietary. That includes the shareware Volkov
  Commander distribution and any commercial tool.
- That also covers anything reconstructed from the shareware binaries: no
  decompiled code, no disassembly, none of their strings. Where they hold
  material that the published source lacks, the source reserves room with
  the `ShwCode` and `ShwData` macros of `src/VCLAYOUT.INC`, which emit
  filler only. Recording where such material sits and how long it is, is
  fine; reproducing it is not.
- Only fetch and run software that has a clean licence: open-source
  software, or software obtained from its rights holder on terms that allow
  this use. The Volkov Commander shareware distribution qualifies, because
  it comes from its author's site under his shareware terms. Copies of
  commercial software from mirrors or abandonware sites do not, however
  useful; Borland's TASM and TLINK, which the original build uses, are the
  standing example.
- The same goes for their output. Do not use reference binaries, hashes or
  other results obtained by running such a tool on the side, and do not
  shape code to match them. The binaries Volkov shipped are a different
  matter: they are his own output, obtained from his site, and matching
  them byte for byte is the project's test of fidelity.
- What the original does when it is run is not proprietary: the screens it
  shows and the files it leaves, recorded as goldens in `harness/goldens/`,
  are committed. They are derived by `harness/goldens.py` and never edited
  by hand.
- Naming such tools is fine. Documentation should say plainly what the
  original was built with and where a reference came from; the restriction
  is on fetching and running them, not on mentioning them.
- Anything the work downloads is fetched at build or run time from its
  official source and pinned, by SHA-256 (see `harness/deps.py`) or by git
  commit (see `CMakeLists.txt`).

## Naming

- The project is called Vibe Commander. The name of the checkout directory
  or of the remote may be temporary; never write either into source,
  documentation or commit messages.

## Committing and pushing

- Show the result of a piece of work, then wait for confirmation before
  committing it. A translated batch is the exception: see "A run of
  batches".
- Commit and push together. A commit is never left unpushed.

## History

- The history is meant to be read: one deliberate, self-contained commit per
  step, described accurately. Get a commit right before it is pushed rather
  than fixing it afterwards.

## The original source

- The files in `src/` are CP866 text with CRLF line endings. Edit them as
  bytes; do not let an editor or tool re-encode them or change line endings.
  The same goes for `port/asm/`, which starts as a copy of them.
- The first commit holds the source exactly as its author published it, so
  `git diff` against that commit shows every change made since.

## The port

- The program is translated to C for DOS first, and compared with the
  original there. Nothing for other operating systems, and no layer to hide
  DOS behind, goes in until that translation is complete and verified.
- Fidelity means the same behaviour, bugs included. Port a bug as it is and
  report it; whether to fix it is decided separately.
- Translated code keeps the names that routines and data have in the
  assembly, until the whole program is C.
- Size matters, but need not be chased: twice the original is fine, ten
  times would be a red flag.

## Translating a batch

Routines are translated in batches of ones that belong together. A batch
is as large as what belongs together: a whole dialog or a whole command
rather than a handful of leaves. `tools/routines.py` lists those that are
still assembly, with their sizes, their callers and what they call. For
each batch, in this order:

1. Run `tools/flagscan.py` on the routines: it follows the flags they
   leave to whatever reads them, through calls and returns. Read every
   caller that stays in assembly: which registers it passes and which it
   uses afterwards. That and the flags are the contract the stubs must
   keep.
2. Write scenarios that reach the routines. `harness/reach.py` lists the
   instructions of theirs that no scenario has run yet, by address in a
   line for each routine and with `--list` as the listing has them, and
   tries steps against that list; `harness/show.py` shows what keys do, with
   `--changes` only the rows that a step changed; what is known about
   driving the dialogs is at the top of `harness/scenarios.py`. Derive the
   goldens with `harness/goldens.py`.
3. List the routines in `harness/ported.py` and run the `port` test. It
   names every instruction of theirs that no scenario executed in the
   original. Add scenarios until none is left, or list an instruction as
   out of reach with the reason.
4. Translate. The C goes into `port/`, the routines leave `port/asm/`, and
   those that assembly still calls leave a stub behind.
5. The `port` test must pass. Nothing else decides whether a translation is
   right. What it reports, the size, the stack and what ran, is in
   `build/Testing/Temporary/LastTest.log` afterwards: read it there rather
   than run the test again.

The test can only see what a scenario shows. Where a branch of a routine
has run in the original but its effect cannot be pointed to in a golden,
`harness/notice.py` breaks it in a copy of the port and says whether any
scenario notices. If none does, make one. Do not use it on the registers
that a stub hands back: those are settled by reading the callers.

When the `port` test fails where the change at hand cannot explain it,
derive the goldens again before anything else. If `git diff` then shows
them changed, it is the machine that differs, QEMU or DOS, and not the
port.

### Stubs in the assembly

- A stub hands back exactly the registers its remaining callers use, and
  no caller may rely on flags: C code leaves them undefined.
- C code needs DS on the main data segment, where assembly often has ES
  there and DS elsewhere; the stub arranges it and restores DS.
- C code may destroy ES. A stub whose callers need ES saves it, or the
  function's `#pragma aux` leaves ES out of what it modifies.
- The direction flag must be clear when C is entered. C leaves it clear.
- C code calls a routine that is still assembly through an entry made
  with `FromC`, declared in `port/vc.h`. Registers go in and come back as
  the routine has them. Flags do not come back: the entry of a routine
  that answers in one turns the answer into a register.
- ES is not always on the main data: `ReDir` calls `FnFile` and `NearFl`
  with ES on the old list of files. A function that touches nothing of
  the main data, only far pointers and constants, needs no DS, and its
  stub leaves DS alone. A panel goes to C as its segment in a 16-bit
  register, and `PANEL` of `port/vc.h` makes the pointer.
- A routine that hands back two words can return them as a `u32` in
  `value [dx ax]`; one that hands back more gets room on the stack and a
  `STACK` pointer to it in BX, and the stub pops the registers. Where the
  flag a routine answers in follows from what it returns, the stub makes
  it by comparing: `FnFile` has found a file if SI is below `EndBuf`.
- A label inside a procedure may be an entry of its own that others call:
  `TreNum` in `FilNum`, `NumAd1` in `NumAdr`. `tools/routines.py` does not
  show such callers; the assembler does, with "Symbol not defined", once
  the procedure is a stub. What they call stays, as a procedure of that
  name.
- A routine that saves every register gets a stub that saves AX only,
  around the call: with `modify exact [ax]` the compiler saves what the
  function uses.
- A routine may be C in part: `PutFls` draws a panel of files in C, and
  for the tree and the Info its stub pushes what the assembly behind it
  pops, and jumps there.
- A batch may leave a stub without a caller, as `PutFls` did that of
  `SzDtTm`: `tools/routines.py` shows it with 0 callers, and it goes.
- Data of the assembly that C code needs is made `PUBLIC` there and
  declared in `port/vc.h` as `extern const u8 __far`, with a `#pragma
  aux` that keeps its name: `Title_S`, `Var_S`, `Tree_S`.

### C for the port

`port/vc.h` says what the program asks of C code. What Open Watcom does
with it is less obvious:

- A byte register in the `parm` list of `#pragma aux` is ignored without a
  warning, and the argument goes on the stack. Pass bytes in 16-bit
  registers and mask them in C.
- A 32-bit argument in a pair of registers does not arrive with its halves
  in the order written. Pass the halves as two 16-bit arguments.
- DS cannot carry an argument: a far pointer in `[ds si]` is refused. The
  stub moves the segment to another register.
- A function that returns nothing may change AX all the same, whatever
  `modify exact` says. List AX there and have the stub save it.
- A pointer variable to a constant in the code segment makes the compiler
  load the segment by name, which costs a relocation. Index the constant
  array directly: that is addressed through CS. Handing the constant to a
  function that takes a far pointer, or assigning it to a far pointer in
  a structure, costs none either: the segment is taken from CS. Choose
  between two such constants with `if` and two assignments: of `?:` the
  compiler makes a pointer into the main data, without a warning.
- Arithmetic on `u8` is done as signed `int`, with a signed division. Use
  `u16` for anything that is divided or counted.
- The option `-nd` makes the compiler load a segment for every access to
  the main data. Do not use it.
- The program's stack is 256 bytes, and compiled code uses more of it
  than the assembly did. A static function that two routines share is
  called, not expanded in place, and a multiplication has DX saved around
  it. In a routine at the bottom of deep calls, write the sharing out and
  shift instead of multiplying, and read what the compiler made of it,
  in the listing next to the object, `build/port/*.lst`.
  The `port` test says how much of the stack each scenario leaves
  untouched, and fails below a limit. Its run in step 3 has the figures
  of the new scenarios with the routines still assembly.
- A static function is called, not expanded in place, also where only
  one routine uses it.
- The room a function takes for its variables is the `sub sp` at its
  start in the listing. `PutFls_c` takes 46 bytes, and 14 for registers,
  where the assembly pushed 16; no scenario is at its deepest there. A
  routine that is, wants its variables few and its structures small.
- Dividing a 16-bit number by another, or taking the remainder, needs no
  helper routine.
- The structures of `port/vc.h` are the assembly's, but a field that the
  assembly declares with `0 DUP`, a name for a place, is not in them:
  `TreList` is where `TreSum2` is, two bytes before the end of `DIRTREE`.
  Read the `STRUC` in `port/asm/VC.ASM` before taking an offset from a
  C structure.
- There is no C library. String literals and static variables are refused
  by `tools/mkexe.c`. The first 32-bit multiplication or division will
  need the compiler's helper routines written, in assembly, in the C code
  segment.

## A run of batches

Batches are translated one after another, without waiting for the user
between them.

- Each batch is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not
  translate: it starts each agent, checks what the agent left, which is
  the commit, the push and `LastTest.log`, and keeps its report. An
  improvement to the tools is handed to an agent in the same way.
- An agent knows only what the repository tells it. What a batch learns
  that the next one needs is written down before the batch is committed:
  about the compiler and the stubs here, about driving the dialogs at the
  top of `harness/scenarios.py`, about lost time in `TODO.md`.
- A batch is committed and pushed as soon as the `port` test passes,
  together with the scenarios and goldens made for it. The next batch
  starts at once.
- The tools and the harness are improved during a run, between batches
  and without waiting for confirmation, each improvement a commit of its
  own. What decides is the total cost: an improvement is made when it
  saves more time over the batches still to come than it takes to make.
- The time it saves is time that a batch has actually lost, not time one
  might lose. Note what a batch lost and to what, and go by those notes.
  What does not pay yet stays in `TODO.md` with what it has cost so far.
- The run ends when the number of batches the user asked for is done,
  when the program is all C, or when something below stops it. Then
  report: what was translated, the bugs found in the original, the
  instructions listed as out of reach, the branches that no scenario
  notices, the size and the stack; and what was improved in the tools,
  with the time it took and the time it is saving.
- Stop and ask, with the batch at hand left uncommitted, before any of
  these. An agent that meets one hands its batch back as it is, and the
  session asks the user:
  - behaviour that differs from the original's, a fix of its bug
    included;
  - a change to what the comparison leaves out, or to a limit that the
    `port` test holds the build to;
  - a golden that comes out changed when it is derived again;
  - anything new to download;
  - a change to these instructions, other than a note on what was
    learned about the compiler or the stubs;
  - a `port` test that still fails once the batch has been made smaller.

## Scenarios

- A scenario starts from the program as its distribution configures it, on
  a machine of its own, and sets up whatever else it needs through the
  program's own menus. It does not rely on prepared configuration files.
- The one thing the test bench changes is the clock, which is off. It has
  the original switch the clock off in its menu and save its setup, and
  gives every scenario the file that leaves.
- A scenario that shows the time of day, with the clock switched on or in
  the time stamp of a file it has made, does so within the first minute of
  its machine: a golden is recorded more slowly than a build is tested.
- The countries tested are the United States and Russia.
- Russian text is typed on the Russian keyboard of the Russian bench. Its
  Cyrillic letters are selected in a step that types text into an input
  line, never at the panels: there Alt by itself may bring up the menu,
  depending on timing.
- The tests run the build of the port that signals where it is. The build
  that is shipped is not run by them.
