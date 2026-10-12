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
  or `PutInf_c` by the kind of the panel.
- A stub may also call several C functions one after the other: that of
  `ReDir` calls `ReDir_c`, `ReDr45_c` and `ReDr50_c`, and has what the
  first leaves for the last in room on the stack, as a structure that BX
  points at. That was done to save stack when it was short, and need not
  be done again.
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
- The assembler does not tell capitals from small letters. `Help_c` is
  `Help_C` to it, the number of a colour, and it says "Symbol
  redefinition": the C function of `Help` is `Help00`.
- A routine that jumps on by a table of keys, with `Case`, has the table
  as a `switch` in C. `Case` takes no key without a scan code, and an
  entry of the table without one takes the character with any.
- `Case` jumps with SI behind the entry of its table that it found, and
  what it jumps to may go on with that: `ChTr55` calls `DelTr0`, which
  takes SI for the address of a path in the main data. With the table
  gone, the C hands on the address that the table has in the original.
  The listing has a table of `.DATA` by its offset in that segment only;
  where it is in the program shows in `build/shareware-layout/VC.COM`, by
  looking for its bytes.
- An entry for C can be made for a label inside a procedure of another
  file: `FromC DelTr0` was in `VCSUB3.INC`, the label in `DelTree` of
  `VCF1.INC`, until `DelTree` became C. `Search` takes an argument in
  BP, and its entry, written out, takes it in CX; `OncZoom` hands back
  three registers as `DialBox` does, into three words on the stack, and
  takes the include line in DX: a function that has none to give leaves
  DX out of its `parm`.
- The `port` test can run before anything is translated: with the batch
  listed in `harness/ported.py` without functions, it says whether the
  new scenarios hold on the port as it is, and so tells a scenario that
  cannot stay from a translation that is wrong.
- What a routine of the assembly reads at an offset in its code segment
  stays in the assembly and is made `PUBLIC`: the box that `Dialog`
  shows, the buttons and the colours that `MinMnu` takes. Texts and
  colours that only C code hands on, as a far pointer to `NgLine`, go to
  the C file.
- An entry for C can hand back a second register in DX, `FromC GetMous,
  <MOV DX,CX>`, and make a number of two flags: that of `DoublMo` loads
  AX with `MOV`, which leaves the flags, between its jumps. `MinMnu`
  takes its colours at CS:BP and has an entry that takes them in DI;
  `PgDn` has one that takes in BX what it has in BP.
- `Input` calls `Help` for F1, so every routine that takes keys can have
  the help on the stack above it, and the help takes its own keys with
  `MinMnu`, which calls `Input`: F1 in the help enters `Help` again,
  which returns at once because `HlpPage` is 0FFh then.
- The files of `port/asm/` end with a byte 1Ah behind the last line. A
  script that takes one apart by lines keeps it.
- A stub that only the code between procedures calls runs when a
  scenario has that code call it: `FndAdr` is called from the assembly
  by "Go to" of the file search alone, and the `port` test said "not
  executed in the port: FndAdr_c" until a step did that.
- A jump on a flag behind two calls may never be taken. `FndBack` has
  `JC` behind `MnMn40` to go back for the next key, and `MnMn40` ends in
  `Window`, which always leaves the carry clear. The C does what the
  assembly did then, not what it meant: it goes on to `FndMove` with
  what `MnMn40` left in AX, and the entry `FromC MnMn40` hands that back.
- Two batches that run side by side may each make an entry for the same
  routine, under the same name: the help and the file search both made
  those of the mouse and of `PgUp` and `PgDn`. Git shows no conflict
  where each put its entry in a file of its own, and the name is there
  twice. After a rebase, look for the entries that have arrived, `FromC`
  and `_a` in `port/asm/`, keep the one next to the routine, and go by
  its registers and answers: those of `DoublMo` were the other way round.
- The BIOS is called through an entry too: `Int16_a` in `VCFIND.INC`
  for the keyboard, which makes a number of the zero flag.
- The program has code in its data: `DirFunc` and what follows it,
  `FncSel` and `FncExt`, stand behind a `.DATA` with no `.CODE` after
  it. `harness/coverage.py` knows the procedures there; code between
  procedures it names in the code only.
- A routine that a command puts into `FncAddr`, as `F38_40` deletes and
  `M30_50` sets attributes, is called by `FncSel`, by `FncMas` and by
  `DirFunc`, with the attributes in AL, and answers in AX: 1, 0, or
  100h to stop. Its callers go on with BX, DX and SI, so its stub saves
  all but AX. C code calls what `FncAddr` names with `FncCall` of
  `port/vc.h`, whose entry keeps DS: `C00_00` does not.
- A box may name a routine that `Dialog` calls when it is answered:
  `F38_10` and `M30_60`. It gets the answer in AL, with ES and DS on the
  main data, and hands back AL and the carry flag, set to go on with
  the dialog.
- The label inside a procedure that others call may skip a test at its
  start: `DelTr0` in `DelTree`. The stub keeps the test and the label,
  and the C function is what follows the label.
- A caller may go on with what a routine leaves in AX when it reports
  no error: `^R` in a tree takes AL after `DelTr0` for the kind of the
  panel, the 1 that DOS answers for one byte written. Hand back all of
  AX as DOS left it, also where nothing failed.
- A call of DOS that leaves the carry flag as it finds it comes back
  through `Intr21` with what the caller had, and `Intr21` takes a set
  flag for an error. The calls for an FCB are like that, 13h in
  `F38_70`; those from 38h to 4Fh and 56h to 60h clear it first. C code
  leaves the flag undefined, so such a call goes through `Intr21f`,
  which sets the flag as the assembly had it: read back from the call
  to the instruction that set it. A call that the assembly makes with
  `INT 21h` itself gets an entry of its own: `Fcb29_a`.
- `EXTERN` for a C function stands in the list at the top of `VC.ASM`,
  before any segment is open. In a file that is included inside the
  code, the assembler takes the function for one of that segment and
  makes of `CALL FAR PTR` a `PUSH CS` and a near call, without a word:
  some such calls arrive and some do not, and the program ends in
  "Invalid Opcode" somewhere else. `PUBLIC` can stand anywhere, and at
  the top of the batch's own file, as in `VCF1.INC`, it does not meet
  the line another batch adds.
- A box and a dialog are tables in the assembly's code segment, and
  `DialBox`, `Dialog`, `MinMnu`, `MnMn10` and `MnMn40`, which are C, take
  them as offsets there, from the assembly and from C alike. The C reads
  them through far pointers made of the offset and of the assembly's
  segment, which an entry hands it: `AsmSeg_a`, and `AT` in
  `port/dialog.c`. Naming something of the assembly gives the segment
  too, but as a number that is set when the program is loaded, and the
  `port` test knows code by its bytes in the file: a function that had
  such a number among its first instructions, before its first call or
  jump, was reported as "not executed in the port".
- Where the assembly has DS elsewhere than on the main data and the
  routine reads there, the stub hands DS to the C as a number: the filter
  of a panel is a dialog whose fields are in the panel's segment. C code
  that calls such a function passes `MAIN_SEG` of `port/vc.h`, which
  compiles to a copy of DS.
- A routine that the assembly names in a table and C code has to call,
  as `Dialog` calls what a dialog has for Enter and F10, gets an entry
  that takes its address in a register and calls that: `DialSub_a`, next
  to the stub of `Dialog`. It sets DS and BP as the routine expects them
  and makes a number of the carry flag. Such a routine may call the stub
  of the function that called it: `M30_60` and `F38_10` call `Dialog`
  from inside a dialog.
- With `modify exact [ax]` the compiled function saves the registers its
  arguments come in as well, BX, CX, DX, SI and DI, and ES: the stub of
  `InSt10` is the call and nothing else.
- C code may be entered again from what it calls: `C00_00` has `DirFunc`
  call it for everything in a directory. Nothing is needed for that, as
  long as the functions keep what they know on the stack or where the
  assembly kept it.
- Within one routine, too, a register may be read long after it was set,
  and not by what was meant to set it. `C20_00` takes CX for whether the
  name of the target is short enough, and `C10_00` jumps there with CX as
  comparing two names left it, or as DOS returned the attributes of a
  file: a file without attributes cannot be appended to. Where a piece
  of a routine becomes a function, read what every jump to it has in
  each register that it reads.
- A routine that is C whole, and whose helpers only it called, leaves
  one stub: `C40_00`, `C50_00`, `SetDate` and `PerCent` are static
  functions of `port/copy.c`. The boxes and the texts stay in the
  assembly, made `PUBLIC` where they are defined.

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
- The port's stack is 4,096 bytes where the original's is 256, so stack
  use needs no care. The `port` test reports how much each scenario
  leaves and fails when less than half is left.
- A static function is called, not expanded in place, also where only
  one routine uses it.
- Dividing a 16-bit number by another, or taking the remainder, needs no
  helper routine.
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
- DX of a call of DOS that is no pointer, a drive or a date, is written
  into the offset of `name` in `struct DosRegs`, which leaves DS as it
  was set: `*(u16 STACK *)&regs.name`, as `DX_OF` in `port/dirfunc.c`.
- `INC AX` and `CBW` on what DOS left in AL is
  `(u16)(signed char)(u8)(value + 1)`.

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
