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
- DOS is a stage on the way: the aim is to carry the program to modern
  operating systems, where a few kilobytes of memory or of disk mean
  nothing. So the port may be larger than the original and use more
  memory, the part that stays in memory while a command runs included,
  as long as it behaves the same. Spend no effort on making it small.
  Ten times the original's size would still be a red flag.

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

The test can only see what a scenario shows. `harness/notice.py` breaks
a branch in a copy of the port and says whether any scenario notices, but
it is not run for a batch: over three batches it took a sixth of the time
and found no translation that was wrong. A batch is done when every
instruction has run in the original and the `port` test passes.

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
- Code that stands between two procedures belongs to none: `PutTre` and
  `PutInf` stood behind `PutFls`, and `RdInfo` stands before it still.
  `tools/routines.py` does not list such code. `harness/reach.py` and
  `harness/ported.py` take it by its first label, as far as the next
  procedure.
- A routine that answers in the carry flag and nothing else gets a stub
  that makes the flag of what the function returns: `FilePan` returns 1
  or 0, and `NEG AX` sets the carry for the 1. `POP` leaves flags alone.
- A table that the assembly names by CS:BP goes to C as a far pointer in
  `[es si]`. The stub puts ES on its own segment and exchanges SI and BP
  around the call, which takes no stack: C code keeps both. `CallC` does
  not serve there, since it takes DS from ES; see `PutTree`.
- A stub may choose between C functions where the assembly jumped from
  one piece to the next: that of `PutFls` goes to `PutFls_c`, `PutTre_c`
  or `PutInf_c` by the kind of the panel. Chosen in C, by one function
  that calls the next, the registers the first one saves lie under the
  second: 12 bytes of the stack less, in every scenario.
- A stub may also call several C functions one after the other, where
  one would have too much on the stack when it calls something deep:
  that of `ReDir` calls `ReDir_c`, `ReDr45_c` and `ReDr50_c`, and has
  what the first leaves for the last in room on the stack, as a
  structure that BX points at. As one function `ReDir` left 12 bytes of
  the stack in every scenario, 46 less than the assembly.
- The deepest that most scenarios get is the filter of a panel: `SetMask`
  calls `Convert`, that `UpCase`, and that the routine of DOS for the
  capital letters of the country, which runs on the caller's stack and
  takes 62 bytes of it. `Convert` takes 110 bytes of the stack from
  where C code calls it, `SetMask` 146. What counts in a routine that
  calls either is what stands on the stack at that call.
- A routine that answers in the carry flag gets an entry for C that
  makes a number of it: `FromC Fantom, <SBB AX,AX>`. `Intr21`, which
  calls DOS, has an entry that takes the registers from a structure on
  the stack, `struct DosRegs` of `port/vc.h`, DS among them, and puts
  back what DOS returns; `OpenFil` has one that takes the name as a far
  pointer. Such an entry puts ES on the main data, as `FromC` does.
- `MvzLine` copies what follows a text along with it, 68 bytes in all,
  so a name that is copied with it is the assembly's own, made `PUBLIC`,
  and not a constant of the C file: `Mask_S` and `Ext_F`. Once what
  follows them in the assembly is translated, those bytes are others.
- A flag may come back through several returns. `V30_00` answered, for
  a screen that is full, with the carry that `V32_00` left, and that was
  the one of an `ADD` in `Color`, always clear. `tools/flagscan.py` names
  the instruction that reads such a flag; follow it back to the one that
  set it, in `src/`, before deciding what the stub makes.
- A place that the assembly keeps in two registers and answers for in
  the carry flag as well, as `NextLin` does with SI and BX, goes to C as
  a structure: the stub pushes the registers, the function gets BX on
  them and returns the flag in AX, and the stub pops them and makes the
  carry with `NEG AX` before it pops AX.
- What the viewer keeps of a file, `ViewDat`, lies in `TmpBuf` from its
  byte 200 on, and what the editor keeps, `EdData`, lies over it: `EdBase`
  is `ViBase`, `EdSize` is `ViSize`, `EdTxHex` is `ViTxHex`. `port/view.c`
  has the viewer's as a structure there. The text is in a segment of its
  own, which the assembly has in DS and C gets as a number, like a panel;
  the table of its lines is at the top of the same segment.
- An entry for C to a routine of a file that another batch has in hand
  can stand next to the stub that needs it: `FromC T70_00` is in
  `VCVIEW.INC`, the routine in `VCEDIT.INC`.
- A register that a routine stores without having set it, as `V32_00`
  does DX where nothing found is on the screen, is handed to the C
  function like an argument. That is less than showing that nothing
  reads what was stored.
- A caller may go on with a register as the routine happens to leave it,
  not only with what it returns: CX is 0 after `T70_00` and `T75_00`,
  because a `REP` ended there, and `T21_00` and `T00_53` compare with CL
  as if they had cleared it. Read in every caller what becomes of each
  register that the routine changes. With `T75_00` the test showed it
  only by chance, in a status line drawn where the original leaves it:
  what went wrong was the count of free bytes, which is not compared.
- BP cannot carry an argument to an entry for C. `DialBox` and `Dialog`
  take their box at CS:BP, so their entries take its offset in AX. C code
  has the offset of an object of the assembly as
  `(u16)(u32)(const void __far *)object`, which compiles to the offset
  alone and costs no relocation. `PUBLIC` for such an object can stand in
  `VC.ASM`, whichever file defines it.
- The box of DOS's critical error, `FatErr`, comes up inside a call of
  DOS and is 46 bytes deeper in the stack than a dialog of the routine
  that called DOS. What a C function has on the stack when it calls
  `Intr21` is on top of that: see `TODO.md` for the editor's saving.
- A routine that answers in the zero flag gets an entry for C that is
  written out, where `FromC` has room for one instruction: `MOV AX,0`,
  which leaves the flags, and `JNE` past an `INC AX`. See `CmpSym_a` in
  `VCEDIT.INC`. A stub makes the zero flag for its callers of a function
  that returns 1 or 0 with `CMP AX,1`.
- Registers that follow from what a function returns are made by the
  stub: `T55_00` returns where the cursor gets to in the line, and its
  stub makes SI of that, DI of where the cursor was, and the zero flag
  by comparing the two, which is what `T23_00` goes on with.
- An entry for C goes when its routine becomes C, and the C code that
  used it calls the function: `T50_00_a`, `T51_00_a` and `T53_10_a` went
  like that. A function that had the entry's convention, a result in SI
  say, then has another: read every place that calls it.

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
- A far pointer in a pair of general registers has its segment in DX
  whichever is written first: `[bx dx]` and `[dx bx]` both put the
  offset in BX. Read the call in the listing.
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
  one routine uses it. The call is near, two bytes, and the function
  saves every register it uses but AX. So what is deep can be kept out
  of the frame of the function that calls something deeper: `T80_00`
  has only the place of its box, and calls `Dialog` with 22 bytes on the
  stack where the assembly had 12; the registers for DOS, 14 bytes, are
  in static functions that have returned by then.
- The room a function takes for its variables is the `sub sp` at its
  start in the listing. `PutFls_c` takes 46 bytes, and 14 for registers,
  where the assembly pushed 16; no scenario is at its deepest there. A
  routine that is, wants its variables few and its structures small.
- Dividing a 16-bit number by another, or taking the remainder, needs no
  helper routine.
- A structure among the variables of a function is in its frame whole,
  and a function with a dozen variables keeps most of them there too:
  `ReDir_c` takes 44 bytes and 14 for registers. A static function that
  gets a pointer to the caller's structure, as `GetVol0` does to the
  registers of `RdInfo`, has no second one on the stack.
- The structures of `port/vc.h` are the assembly's, but a field that the
  assembly declares with `0 DUP`, a name for a place, is not in them:
  `TreList` is where `TreSum2` is, two bytes before the end of `DIRTREE`.
  Read the `STRUC` in `port/asm/VC.ASM` before taking an offset from a
  C structure.
- There is no C library. String literals and static variables are refused
  by `tools/mkexe.c`. A text may be written as a string where it gives an
  array declared `CODE` its contents: `static const u8 CODE Name_S[] =
  "text";` is in the code segment, with its zero byte.
- For 32-bit numbers the compiler calls helper routines, near, in its own
  code segment. `__U4M`, which multiplies, and `__U4D`, which divides
  unsigned numbers, are at the end of `port/asm/VC.ASM`; of `__U4D` the
  scenarios run only the half for a divisor of 16 bits. A helper that is
  missing shows as an undefined symbol when the port is linked: signed
  division would be `__I4D`. Shifting a 32-bit number by a constant needs
  none. `/` and `%` of the same numbers are two calls: take the remainder
  by multiplying back where it is known to fit 16 bits.
- `REP MOVSB` from one segment to another is a loop over two far
  pointers: a far pointer steps its offset only, as SI and DI do, down as
  well as up. Where the assembly divides a 32-bit count into pieces for
  that, comparing and subtracting in the loop needs no helper routine.
- The figures of the stack that the `port` test reports differ from one
  run to the next, by 12 bytes or so in a scenario or two: an interrupt
  that arrives when the calls are deep. What a batch costs shows in every
  scenario that draws what was translated, and the same in each.

## A run of batches

Batches are translated without waiting for the user between them, and
several at a time.

- Each batch is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not
  translate: it starts each agent, checks what the agent left, which is
  the commit, the push and `LastTest.log`, and keeps its report. An
  improvement to the tools is handed to an agent in the same way.
- An agent knows only what the repository tells it. What a batch learns
  that the next one needs is written down before the batch is committed:
  about the compiler and the stubs here, about driving the dialogs at the
  top of `harness/scenarios.py`, about lost time in `TODO.md`.
- Batches that run at the same time each have a checkout of their own, a
  plain clone of the same origin with its own build, and a part of the
  program that the others leave alone. The session names both: the part
  is a file of the source, as a rule.
- A batch is committed as soon as the `port` test passes, together with
  the scenarios and goldens made for it. It gets onto `main` by
  `git pull --rebase`, a build, the `port` test once more, now with the
  other batches under it, and `git push`. If the push is refused, another
  batch got there first: again from the pull. The agent settles a
  conflict of its batch itself and keeps what both sides added.
- The stack is one budget for all the batches. What counts is what the
  test says is left after the rebase.
- The screens of the original cannot be recorded while other machines
  run: goldens came out different when they were. `harness/turn.py` has
  every run of the harness wait for its turn, a run that records the
  original by itself, tests of the port together. So, while other batches
  run, derive the goldens of the scenarios at hand and never all of
  them; and when the `port` test fails where the change cannot explain
  it, derive again the golden of the scenario that failed.
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
  instructions listed as out of reach, the size and the stack; and what
  was improved in the tools, with the time it took and the time it is
  saving.
- Stop and ask, with the batch at hand left uncommitted, before any of
  these. An agent that meets one hands its batch back as it is, and the
  session asks the user:
  - behaviour that differs from the original's, a fix of its bug
    included;
  - a change to what the comparison leaves out, other than a figure of
    the kind that "Scenarios" has left out, or to a limit that the `port`
    test holds the build to;
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
- A figure on the screen that follows the time of day or the size of the
  program, such as the clock or a count of free memory, is left out of
  the comparison, in `harness/deviations.py`. The exception is a scenario
  that is made to test that figure.
- The countries tested are the United States and Russia.
- Russian text is typed on the Russian keyboard of the Russian bench. Its
  Cyrillic letters are selected in a step that types text into an input
  line, never at the panels: there Alt by itself may bring up the menu,
  depending on timing.
- The tests run the build of the port that signals where it is. The build
  that is shipped is not run by them.
