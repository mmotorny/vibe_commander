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
- The port ends with no assembly file in it: its code and its data are
  C. Where C can be written in more than one way, portable C comes before
  C that is tied to this compiler or this machine, that before the
  compiler's intrinsics, and those before assembly, which is written
  inline in a C file and only where nothing else works. That a thing
  would be smaller or faster is no reason to go down that list.
- The mouse is tested like the keys: the bench is to have one, and what
  the program does with it is held against the original in scenarios.
  The port is not done while the mouse has not been shown to work. This
  asks for no more coverage than is asked elsewhere: an instruction that
  no scenario can reach is still listed as out of reach with its reason.
  That the bench has no mouse is no longer such a reason.
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

### Moving the data of a file to C

Every routine is C. What a file of `port/asm/` still holds is texts,
boxes, dialogs, buttons and colours, which go to C a file at a time, and
tables of commands and code, which go otherwise: `TODO.md`, "The last of
the assembly", has what is left, who has which file, and the order.
`VCHELP.INC` went first, into `port/help_t.c`: read that file and
`port/tables.h` before anything else.

For one file, in this order:

1. Build, and run `python3 harness/datacheck.py build/shareware-layout
   build/port`. Under `assembly` it names every object of the file that
   has a name of the original, with its length. That is the list of what
   moves, and each is as the original now. Keep the output, in `build/`:
   step 5 holds the new one against it. To read the assembly, make a
   copy that an editor shows, `iconv -f cp866 port/asm/VCFIND.INC >
   build/VCFIND.txt`; change the file itself by a script that reads and
   writes bytes.
2. Write the file of constants, `port/NAME_t.c`, under the name that
   `TODO.md` has for it. It includes `tables.h` and holds constants and
   nothing else, in the order of the assembly:
   - A text is `const u8 Name_S[] = "text";`, with the zero byte of
     `DB 'text',0`. One that the assembly ends without a zero is an array
     of its length, `Name_S[9]`. A byte that is no character of ASCII is
     an octal escape, `\377\003`: the markup, and the frames, which are
     characters of CP866, `\263` for the upright line. The source stays
     ASCII; a comment may show the line as it looks. Take every byte
     from the line of the assembly, read as bytes. An escape has three
     digits always, since a digit may follow: `0FFh,3,'5 minutes'` is
     `"\377\0035 minutes"`. A number that the assembly works out is
     worked out: `0FFh,80h+16` is `\377\220`. `'\*.*'` is `"\\*.*"`,
     and `^V` and `^I` are a caret and a letter. Several `DB` under one
     label are one array: `'a',0` and `'b',0` are `"a\0" "b"`, the last
     zero being the string's own.
   - A text that the assembly ends without a zero and that is read as
     far as the next zero all the same, as a line of a box is, is
     written with what follows it up to that zero: `Kov_S`, four
     characters in front of `Empty_S`, which is a zero byte, is
     `"\"^V\""` with the string's zero, five bytes. `Empty_S` is a
     constant of its own besides. Two objects of C stand one behind the
     other as they are written, but nothing says that they must.
   - Buttons and colours are arrays of `u8`.
   - A box is `BOX(n)`, or `BOX1(n)` and `BOX2(n)` with texts put in; a
     dialog is `DIALOG(n)`. The assembly's `DB 4, 1`, `DW ErrBuf`, `DW
     TmpErr+1`, `DB 0, 55` and four lines are `BOX1(4)`: `{ colours },
     4, 1, MD_AT(ErrBuf), { { MD_AT(TmpErr[1]), 0, 55 } }, { lines }`. A
     dialog is written in the assembly as `BOXHD <...>` and `BOXMN
     <...>` or as `DW` and `DB` with the same values in the same order;
     `OFFSET Name` is `Name`. A word that names something is a pointer,
     written as the name. A place in the main data is `MD_AT(TmpBuf)`,
     and `MD_AT(TmpErr[1])` for `TmpErr+1`; a number in a word that
     names something elsewhere is `NUMBER(n)`. What `port/tables.h`
     lacks, a place in a panel's segment say, is added there.
   - A constant that only this file names is `static`. One that code or
     another file names has `#pragma aux Name "*"`.
   - What it names of another file, assembly or C alike, is `extern
     const u8 Name[];` with `#pragma aux Name "*"`. If that is of the
     assembly, it has to be `PUBLIC` there, in the file that defines it;
     the sixteen that several files name are declared in `VC.ASM`
     already. A routine that a dialog names is declared the same way,
     and is the stub.
   - Where the program copies more than an object has, the object is
     written as long as what is copied, with the bytes that the original
     has behind it: the test compares as many bytes as the object has.
     `TODO.md` says "copied with what follows it" of each such text.
3. Take the objects out of the assembly file, as bytes: CP866, CRLF, and
   the 1Ah at its end. Take their names out of every `PUBLIC` line; the
   assembler says "Symbol not defined" and the line. Where assembly
   still names one, a box that stays or a stub, that file says `EXTERN
   Name:BYTE` at its top: a file is included inside the code segment,
   and there the assembler takes the name for one of its group, which
   it is.
4. The C code that uses them does not change: `extern const u8 __far`
   with its `#pragma aux` reads a constant of C as it read the
   assembly's.
5. Build, and run the tests, `ctest --test-dir build -R data` first:
   it takes no time, and the build a minute. `data` has to name each
   object of step 1 under the new file, as long as it was or longer,
   and pass. The test does not notice an object that has left the
   assembly and not arrived: hold the names against those of step 1.
   The two counts in its last lines add up to what they added up to
   before, in objects, and in bytes but for what was written longer.
   Then `port` has to pass, two minutes.
6. A file with nothing left but comments goes: `git rm`, its `INCLUDE`
   in `VC.ASM`, its name in `port_asm` of `CMakeLists.txt`.

In what order the objects of a file go: all at once is the least work,
since what names what moves together. Where a file is done in pieces,
go from the top down: the dialogs, which only C code names, then the
boxes, then the texts and the buttons. A dialog of C that names a box
still in the assembly needs the box `PUBLIC` in its file and no more;
the other way round, a box of the assembly that names a text of C needs
an `EXTERN` line, which goes again a step later.

What names an object from elsewhere, and what to do:

- C code: nothing. It has `extern const u8 __far Name[]` and `#pragma
  aux Name "*"`, and the constant has `#pragma aux Name "*"`. If the
  linker says "undefined symbol Name", one of the two lacks the pragma,
  or the assembly lacks `PUBLIC`.
- A box of another assembly file: today only the sixteen that `VC.ASM`
  declares `EXTERNDEF`, and for those nothing is to do on either side.
  Should the assembler say "Symbol not defined" in a file that is not
  yours, that file wants `EXTERN Name:BYTE` at its top: add the line,
  and say so in the commit, since another agent has the file.
- A stub: `PtMnu_S` alone, which is one of the sixteen.
- A code file that has a copy of its own, `static ... CODE`, under the
  same name, as `Empty_S` and `Mask_S` are in several: leave it. The two
  do not meet at the linker, and the test holds both.

What the test `data` goes by is the name: an object is held against the
label of the original that has its name, and a word that holds an
address there against where the port has what that label names. What it
says when it fails, and what that means as a rule:

- "Name+N: the port has XXh, the original YYh". Byte N of the object,
  counted from 0, is another. With 00h in the port at the last byte, the
  string's zero is one that the assembly has not: give the array its
  length. With 00h behind a single byte, a structure is not packed. With
  everything from N on shifted, a byte is missing or one too many just
  before: an escape that ran on, a `BOX` that should be `BOX1`, a count
  of lines that is not the number of lines.
- "the port has XXXXh, which is not where it has Y, ...: the original
  names Y there". The word names the wrong thing: two lines exchanged, a
  name of another box. A number where a name belongs shows like this
  too.
- "the original names Y there, which the port has not by that name".
  Nothing in the port is called Y: a name written otherwise in C, or Y
  left in neither place.
- "the original has no data called X". A constant of C whose name the
  original has not: a name mistyped, as a rule.
- "X is listed in datacheck.py, and the C code has no constant of that
  name". An entry of a table there that has outlived its constant.

Where the port is meant to be otherwise, `harness/datacheck.py` has four
tables, each entry with its reason. A file of the assembly that moves as
it is needs no entry in any of them, and wanting one is a sign to look
again:

- `ELSEWHERE`, for a constant that the original has under another name
  or inside another object. It is there for the constants that code
  files had before the test was. A constant that moves keeps its name.
- `OWN`, for a constant that the original has not as data, which is
  then not compared. Right for a table that only the port has, as
  `Main_T`, `CmdNx_T`, `MnuNx_T` and `F35_T` will be when they are C.
  Wrong for anything that was in the published source: an entry here to
  quiet "no data called X" takes the object out of the test.
- `NAMED_OTHERWISE`, for a word that names something the port has in
  another way: a routine that has become a number in a table of
  functions. Texts, boxes and buttons are never named otherwise.
- `DIFFERENT`, for an object that is meant to differ from the original.
  It is empty. An entry is behaviour other than the original's: stop and
  ask, as "A run of batches" has it.

A difference that the test reports is mended in the C. If the C is as
the assembly of the port and the test still fails, the port's assembly
had drifted from the original, which the test said before the move, in
step 1: it does not start to fail by moving what was right.

What was met on the way:

- C code that names a constant of the compiler's group reads it through
  DS, which is on the main data: the wrong bytes, without a warning.
  That is why such constants have files of their own, and
  `tools/mkexe.c` stops the build for an object file with both code and
  constants. A constant that only code reads stays `static const ...
  CODE` in the file of the code.
- `__based(__segname("CONST2"))` does not put an object into the group:
  it makes a segment of that name and the class FAR_DATA outside it,
  and offsets count from there.
- A structure for a table needs `#pragma pack(push, 1)`, or a byte is
  followed by a gap.
- A text written as a string has a zero byte behind it whether the
  assembly has one or not. Five constants of the C code had one too
  many; one of them, `CrLf_S`, made the banner be copied shorter than
  the original copies it.
- A hex escape runs on into letters that are hex digits: `"\xC4File"`
  is not what it looks like. Octal escapes are three digits.
- `#pragma aux default "*"` does not take the underscore off the names
  of data; each name wants its own `#pragma aux`.
- `EXTERNDEF` wants the type that the definition has, `BYTE` for what is
  made with `DB`, `WORD` for `DW`, `BOXHD` for a dialog written as one,
  or it is "Symbol type conflict".
- A word that names a routine, `FncAddr` in the main data or the
  routine of a dialog, cannot become a pointer to a function: that is
  four bytes, and the layouts are the original's. It becomes the place
  of the function in a table of functions of one type, `static FncFn *
  const CODE Fnc_T[]` in `port/dirfunc.c`, and what called through the
  word calls `Fnc_T[md.FncAddr](attributes)`. The type has the registers,
  `#pragma aux FncFn parm [ax] value [ax] modify exact [ax]`; a function
  that is far only by its own `#pragma aux` is cast to it, `(FncFn *)`,
  or it is "Pointer type mismatch". The stubs of the routines go, with
  the entry that called through the word, their lines in the `EXTERN`
  list and their functions in `harness/ported.py`. Routines that take
  different things, as the commands do, are called from a `switch`
  instead. The routine that a dialog names for Enter and F10 went that
  second way: the word in its header is a number, `SUB_` of
  `port/tables.h`, and `Dialog` has a `switch` that calls `F38_10`,
  `M30_60` or `F35_61`, each of which takes and hands back something
  else. The test `data` is told of each such word in `NAMED_OTHERWISE`,
  with "is" and the number.
- The linker's map has the static objects too, `option static`, marked
  with an `s`, and that is how the test finds a constant that is
  `static`.
- No code may come within 4K of the stack: `tools/mkexe.c` stops the
  build. The constants stand between, `CONST2`, which `VC.ASM` declares
  in front of the code.

What the agents found who moved the other files, each in ten to fifteen
minutes by the steps above, and what those steps did not say:

- `Var_S`, `Title_S` and `Tree_S` are declared far in `port/vc.h`, as
  code reads them, and `tables.h` includes that. A box names its lines
  by near words, so `tables.h` has each of the three under a near name
  as well, `#define Var_S VarNear_S` with `#pragma aux VarNear_S
  "Var_S"`, and a file of constants writes `Var_S` in a box like any
  other line. The file that defines one of them writes no `#pragma aux`
  for it: the macro would be expanded there, and the object named
  `VarNear_S`.
- Take every text from the assembly's bytes by a script, never from a
  copy that an editor shows. A text may hold bytes that are no
  characters, the arrows 18h and 19h in `Cnf14_S`, 1Eh in a line of the
  editor's help, and a converted copy has them as something else or not
  at all. The script reads the `DB` lines as bytes and writes the C,
  with an octal escape for every byte outside 20h to 7Eh, for the quote
  and for the backslash.
- A byte that the assembly works out from an `EQU`, a length or the
  number of a colour, is better an array of bytes with the expression
  in it than an escape in a string: the reader sees what it is.
- Buttons that stand behind the dialog that names them want a
  declaration in front of the dialog, with the length: `static const u8
  Name_D[5];`, the dialog, then the buttons.
- The `mCtrl` of a box to tick holds a place: `NUMBER(MD_AT(field))`,
  and `NUMBER(WCB_AT(field))` for a field of a panel.
- `WinBuf`, `ErrBuf` and their like are arrays of words in `struct
  MainDat`: `WinBuf+2000` is `MD_AT(WinBuf[1000])`.
- An object that nothing names cannot be `static`: W202, "defined, but
  not referenced", stops the build. It is written without `static`,
  with the `#pragma aux` of one that others name.
- When blocks of `.CONST` and `.DATA` are cut out of a file, the `.CODE`
  behind the last of them has to stay where code follows: without it
  the code is assembled into the data.
- A text written longer than the assembly has it adds to the sum of
  bytes in the test's last lines by exactly the difference. Work the
  sum out before the build: a text that came out a byte long or short
  shows there.
- The test names an object of the assembly only as `assembly`, not by
  its file: the list of what moves, in step 1, is made from the labels
  of the file itself, by a script, and held against the test's list.
- The "all" row of the inventory in `TODO.md` is added up again from
  the rows, by a script, at every rebase: two agents each change a row
  and the sum.
- A comment in the code's C file that the move makes stale, "of the
  assembly" where the object is of a file of constants now, may be
  mended in the same commit: comments only.

What is read behind the end of an object. The test compares as many
bytes as an object has, and the program reads more of some: `MvzLine`
copies as many bytes as it is told whatever the text, 68 for a path;
`MainFil` and `ReadOne` copy a name as 13; a title is copied as 80; a
loop goes to a count. What lies behind the text in the original is then
read too, and the port has to have it there: the object is written as
long as what is read, with the original's bytes, and the test holds
them. The copy may be in another file than the text: `User_S` of
`port/keys_t.c` is copied as 80 bytes by Menu file edit, in
`port/commands.c`, and was found last for that.

- `python3 harness/reads.py build/port` lists every place where a file
  of code names a constant, with the bytes that are read of it there. A
  place that it knows by its form, a loop to a count, `MvzLine`,
  `MvsBw`, `MainFil`, a text read as far as its zero, a box handed on
  by its offset, needs nothing. Any other is in `REVIEWED` there, with
  the count and how it is known, by reading the code from where the
  constant is named to where the read stops, through the functions that
  it is handed to.
- The test `data` fails for an object that is shorter than what is read
  of it, for a text that is read as far as a zero byte and has none,
  whether code reads it or a box names it, and for a place that is of
  no known form and not in `REVIEWED`: so a new read is looked at when
  it is written.
- A table of colours is read at the place that the markup of its lines
  asks for: none the first, `^R` the second, `^B` the third, `^U` the
  fourth. A table of one or two colours is enough for lines that are
  constants without the other marks; a line that holds what the user
  typed or named wants four. The name of a file in the mini status is
  written by `MovDz0`, not as a line.
- What the tool cannot see is said at its top: a read that does not
  begin with the constant's name in a file of code, and a table read
  at an index that the program does not keep inside it. Of the second
  kind are the menu that is none, which `TODO.md` has, and the defaults
  of a panel, 88 bytes of which the original has 8: what lies behind
  those is addresses of routines, or is not data of the port at all,
  and no layout of the constants can make it the original's.

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
  procedure, or by a later label of it for what stands from there on:
  such a stretch may hold the end of one file and the start of the
  next, as `M60_00` of `VCMENU.INC` runs on into `F37_00` of `VCF1.INC`.
- Code between procedures may be entered by a jump and left by jumps, as
  the editor is: the viewer's code jumps to `T00_00` and the editor jumps
  back to `V00_10`, `V13_00` or `V13_01`. Its stub keeps the label, calls
  the C function, and jumps by what that returns.
- Where every jump inside such code ends up back at one place, the wait
  for the next key, each label that is jumped to can be a function, with
  the registers that the jump carries as its arguments: a jump is a call
  and a return. `port/edit.c` has the editor's keys like that.
- When the callers of a stub become C, the stub goes, and so does a
  function that was only there for the stub: `T60_00_c` and `T61_00_c`
  were listed in `harness/ported.py` and ran no more. The test says "not
  executed in the port".
- A text or a table of the assembly that C code reads is at its offset
  in the segment that `AsmSeg_a` returns: `AT(AsmSeg(), OFFSET(text))`
  costs no relocation, where naming the text as a far object does.
- A table of colours for `NgLine` may be shorter than the four that the
  routine can ask for: `EdtH_P` is one byte, and what followed it was the
  editor's table of keys. With the table gone other bytes follow; the
  lines written with it use the first colour only.
- A table for `Case` whose entries are a character with any scan code,
  or a key without a character, is two `switch` statements, one on the
  character and one on the scan code; a key without a scan code is in
  neither.
- `LOOP` with CX 0 goes round 65,536 times: `do ... while (--count)` on
  a `u16`. The editor counts the rows of its window like that.
- A register may hold nothing that was meant for the routine called:
  the editor calls `T53_00` with DI, the number of the line, as the last
  key left it, where it deletes. `T53_00` goes by the number only where
  it is 0FFFFh, so the C hands it 0; read what the routine does with
  such a register before choosing.
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
  calls DOS, had an entry that took the registers from a structure on
  the stack, `struct DosRegs` of `port/vc.h`, DS among them, and put
  back what DOS returns, as the function that it is now does; `OpenFil`
  has one that takes the name as a far pointer. Such an entry puts ES on
  the main data, as `FromC` does.
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
  can stand next to the stub that needs it: `FromC T70_00` was in
  `VCVIEW.INC`, the routine in `VCEDIT.INC`, until both sides were C.
- Where code that is jumped to goes on to one of several places, and the
  next place takes registers, the function returns which place and the
  registers come back in room on the stack: the stub of `V00_02` goes on
  to `Func04`, to the editor at `T00_00` or to the viewer at `V00_10`.
  Code of another file may jump into the middle of such code, as the
  editor's stub does to `V00_10`, `V13_00` and `V13_01`: each of those
  labels is a stub of a function of its own.
- Two batches side by side may each keep a stub for the other: the
  editor kept those of `T70_00` and `T71_00` for the viewer's assembly,
  and wrote the viewer's `V80_00` out in C, while the viewer became C
  and kept `V80_00` and `V82_00` for the editor's. After the rebase,
  look for the callers of every stub of the batch and of every stub
  that the batch was the caller of, in all of `port/asm/`, and take out
  what has none; and have the C of the other batch call the function
  where it wrote one out.
- A new scenario may reach what an earlier batch listed as out of
  reach, and the `port` test then fails on the stale entry, "listed as
  out of reach, but executed": take it out of that batch's list.
  `harness/reach.py` shows it beforehand, as a routine with nothing
  left.
- A routine may read a panel through DS that no comment names: `PutPrm`
  takes the size of the panel in DS for where the prompt goes, and the
  viewer hands it the right panel's whichever is active.
- A register that a routine stores without having set it, as `V32_00`
  did DX where nothing found is on the screen, is handed to the C
  function like an argument. That is less than showing that nothing
  reads what was stored, which was done for `V32_00` when its callers
  became C: see there.
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
  which leaves the flags, and `JNE` past an `INC AX`. `Int16_a` did it
  with DX, while it was there. A stub makes the zero flag for its callers
  of a function that returns 1 or 0 with `CMP AX,1`: `CmpSym`.
- Nothing in the program tells the tests where it is. The driver of the
  tests (`harness/DRIVER.ASM`) watches it from inside the machine: it
  takes a call of `INT 28h` for the program's when it comes from the
  memory of the program that DOS has as the current one, and that is
  named as the file under test is. `LeaveTm` of `port/blank.c` makes that
  call, through `Int28` of `port/dos.c`, every time `Input0` has gone
  round, and `Blank` while it waits; whatever makes it has to do so with
  DS on the main data and on the program's own stack, as C code does. And the
  driver reads `AutoTim` and `Clock` in the main data, which
  `harness/driver.py` finds in the listing of the original,
  `build/shareware-layout/VC.LST`, for the port too: its main data is
  laid out as the original's. The port's stack it finds in the linker's
  map, `stack` of `harness/coverage.py`: it begins at `DataSav` of
  `port/resident.c` and is all of the segment `R_BSS`.
- What a routine reads in the assembly's code segment may be read by
  another file too, and then stays, `PUBLIC`, when the routine goes: the
  menu finds the menu that the mouse is on by `PtMnu_S`, the names that
  `PutMnu` writes. Look for every table and text that would go with a
  routine in all of `port/asm/` first.
- An entry that hands back a number of 32 bits is written out, with the
  high word in DX and the low one in AX, whatever the routine has them
  in, as `Timer_a` and `DosTime_a` were.
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
- A command is no routine: the main loop jumps to it by its table of
  keys, and it jumps back to one of several places of the loop, or on
  into another command or a routine of the menus. In C it is a function
  that returns where the program goes on, a number, and its stub jumps
  there by a table: `CmdC`, `CmdEnd` and `CmdNx_T` in `VCFUNC.INC`, with
  `port/keys.c`. Where the place it goes on at takes registers, a panel
  in DS and a byte in CL or CH for `M01_01`, `M03_00` and `M06_00`, the
  function returns them too, in DX and the high byte of AX, and the piece
  of the stub for that place loads them. Such a place may pop a DS that
  the command had pushed: the stub pushes it, before the key.
- A command is entered with the key in AX, and `F73_00`, where a key
  goes that is one of the command line, takes it there: the stub keeps
  it on the stack around the function. The menus enter a command with
  their own numbers in AX and CX instead.
- A label inside a command may be an entry of its own, as inside a
  procedure: the menu comes in at `F17_01` and `F76_01`, the mouse at
  `F00_05`, `F01_05` and `F00_60`, with other registers than the key's
  entry has. Each gets a stub and a function; what only the mouse reaches
  runs in the port all the same where the function for the key calls it.
- The stub of `PutFls` chooses between three C functions by the kind of
  the panel, so C code that wants a panel written makes that choice
  itself: `PtFs00` in `port/keys.c`.
- A routine may be called with DS somewhere that only happens to do:
  `F16_00` asks `BegBX` where a panel begins with DS on the main data,
  and takes the row, which is that of either panel.
- A label names code between two procedures from there to the next
  procedure, not to the next command: `F22_00` runs to `F42_10` and
  holds `F80_00`, `F30_00` and `F32_00` as well. A batch that takes the
  first commands of such a stretch and not the last answers in
  `harness/ported.py` for the instructions of both.
- An entry for C can be made for a label inside a procedure of another
  file: `FromC DelTr0` was in `VCSUB3.INC`, the label in `DelTree` of
  `VCF1.INC`, until `DelTree` became C. `OncZoom` hands back
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
- The BIOS was called through an entry too, `Int16_a` for the keyboard,
  which made a number of the zero flag. It is `Int16` of `port/dos.c`
  now: see "C for the port".
- The program has code in its data: `DirFunc` and what follows it,
  `FncSel` and `FncExt`, stand behind a `.DATA` with no `.CODE` after
  it. `harness/coverage.py` knows the procedures there, and code that
  is no procedure by a label that has an instruction, as far as the next
  procedure or the next data: `FncSel`, with `FncExt` in it.
- A routine that a command puts into `FncAddr`, as `F38_40` deletes and
  `M30_50` sets attributes, is called by `FncSel`, by `FncMas` and by
  `DirFunc`, with the attributes in AL, and answers in AX: 1, 0, or
  100h to stop. Its callers go on with BX, DX and SI, so its stub saves
  all but AX. C code calls what `FncAddr` names with `FncCall` of
  `port/vc.h`, whose entry keeps DS: `C00_00` does not. `FncAddr`
  holds a number since, and these stubs are gone: see "Moving the data
  of a file to C".
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
  `INT 21h` itself is `Int21d` of `port/dos.c` in C, which has the flag
  clear when DOS is called: `Fcb29` of `port/dirfunc.c`.
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
  too, as a number that is set when the program is loaded: it costs a
  relocation, four bytes of the file. The `port` test knows such a
  function all the same, by the file's table of relocations.
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
- A command is code between procedures that the main loop jumps to by
  its table of keys, with DS on the active panel, and that ends by
  jumping back: to `Func04` for the next key, to `Init50` to draw the
  panels anew, to `FncSel`, `FncMas` or `FncExt`. Its C function returns
  a number for where to go on, and the stub jumps by a table of its own,
  `F35_T` in `VCCOPY.INC`; what the place it jumps to takes in a
  register, the title in BX for `FncMas`, comes back in DX. A command
  may read a register that it has not set, as Shift-F6 does BX and DX:
  the stub hands them to the function as they come.
- The address of a routine of the assembly, for `FncAddr`, is that of
  an object like any other: `PUBLIC C00_00`, and in C `extern const u8
  __far C00_00_s[]` with `#pragma aux C00_00_s "C00_00"`, since the C
  function has the name already.
- The stub of a routine that a dialog names saves AX around the call:
  `Dialog` takes the answer from AL when the carry flag is clear, and
  reads nothing of AX when it is set. See `F35_60`, which also keeps
  the label that the other dialog names, `F35_61`, inside it.
- The commands of `VCF1.INC` and the file search's go on the same way,
  with numbers of their own, the `TO_` of `port/vc.h`: each stub is the
  label, the call and `JMP F1Next`, which is in `VCF1.INC` and jumps to
  `Func04`, `FncSel`, `FncExt` or `FncEx1`. `F67_00` returns the panel
  for `F24_02` in DX, and its stub sees to that. `F38_40`, `M30_50` and
  `FindFil` are public for `FncAddr` like `C00_00`.
- A routine that reads through ES what its caller has at DS takes the
  wrong bytes where the two differ, and the C has to hand it the same
  wrong address: `ChgDisk` reads the path at ES:DX, and Go to of the
  file search calls it with DS on the list. Read what a routine takes
  its argument through, not only which register.
- The header of a dialog has its bytes in the order of `BOXHD` in
  `port/asm/VC.ASM`, the key bar first: `DB 1, 0FFh, 32, 10h` is key
  bar 1, no menu item, help page 32, flags.
- DOS called with `INT 21h` itself, where the original did not go
  through `Intr21`, is `Int21d` of `port/dos.c`, which takes `struct
  DosRegs` as `Intr21` does. `Int2F` is next to it.
- When the last caller of a stub becomes C, the stub goes, with its
  line in the `EXTERN` list, and the function is static if only its own
  file calls it: `FndMove`, `FndCurs` and `F38_70`. Take such a function
  out of the batch's `functions` in `harness/ported.py`, which are those
  that the assembly calls.
- The commands of `VCFUNC.INC` from `F80_00` on have stubs that jump on
  by themselves, two ways each: `F32_00` to `Func04` or `F21_30`,
  `F68_00` to `Func04` or `F21_25`, `F80_00` to `Func05` or `Func01`,
  `F62_01` to `Func04` or `M06_10`, with the panel in DS and the item of
  the menu in CL for that. Where the place jumped to takes the current
  panel in DS, `Func01`, `Func05`, `Func12` and `F11_00`, the stub saves
  DS around the call. A label that others jump to, `F73_00`, `Run_00`,
  stays as such a stub.
- A name given to `harness/ported.py` takes all that stands from its
  label to the next procedure: `F80_00` takes `F30_00` and `F42_00` with
  it, `F81_00` the commands down to `Run_00`, and `F61_00` the history
  behind the box of the drives. Such a stretch is listed once all of it
  is C or stub; what is left of it in the assembly is said in `TODO.md`.
- A piece of a command that only hands registers to routines which are
  still assembly, and takes the panel in DS for them, can stay assembly
  as an entry that the C calls: `F77_10_a`, of the speed search, calls
  `Search` and `F00_50`.
- A routine that hands back the cursor of a line and the carry flag
  returns both as a number of 32 bits: `EdLin`, which is C, has the
  column in the low word and 1 in the high one for the flag, and its
  stub makes DX and the carry of them. Its entry did the same while it
  was assembly.
- A routine that reads DS, as `ClsLine` reads `CrntCol`, and that is
  once called with DS left elsewhere by a jump out of a loop, takes DS
  as a number: `ClsLine` has the segment as an argument, MAIN_SEG for
  the main data, and its stub hands over DS. `Run_00` leaves DS on the
  panel when the commands have no room for a name, and passes that.
- What a command leaves in the registers goes round the main loop to
  the next command, and a command that is C does not leave what the
  assembly left there. `Func05` loads BX, so a command that comes back
  to the loop there or before leaves nothing in BX and DX that
  Shift-F6, which goes by the two together, could tell. But the keys
  of the cursor come back at `Func07`, behind it, and their stub left
  both 0 where the original leaves an address or a size in DX: Rename
  behind Down offered a name that the original does not offer, and no
  scenario had the two keys one behind the other. Where a register
  goes round, look at every place that the code comes back to, not at
  the one that most commands use. The cursor's keys put what the
  original leaves into `struct Regs` of `port/vc.h` now.
- Code that ends in a jump by a table, as the menu bar jumps to the
  command of the item chosen, leaves the table and the jump in its stub:
  the C function returns what was chosen and which table has it, and the
  stub makes of that the registers as the code had them at the jump, all
  but BP there, which no command reads. Reading the commands for what
  they take is not enough where another batch has them in hand: Rename
  goes by BX and DX as they come, which showed only when its batch
  arrived with a stub that hands the two to its C function.
- A stub can put ES back on the main data itself, `PUSH DS` and `POP ES`
  before it restores DS, and the function then lists ES among what it
  modifies: see `F39_01` in `VCMENU.INC`.
- The address of a text in a file that another batch has in hand, the
  bar's `PtMnu_S` in `VCSUB2.INC`, can come to the C in a register of
  the stub, with `AsmSeg` for its segment: `F39_01` in `VCMENU.INC`.
  An entry made for a label there, as one was for `PtMnu1`, goes when
  that batch arrives with the routine in C: call the function then.
- The program is in three parts: the assembly's code segment, the main
  data, and the part that stays in the memory while a command runs,
  where the stack is, with `Counter`, `PrgList`, `RunPar` and the
  routines that `CallPSP` calls. The assembly reaches that part through
  SS; C code gets its segment from an entry, `ResSeg_a` in `VCMEM.INC`,
  and the offset of an object there like any other that it names. What
  that part keeps is one structure, `struct Stays` of `port/vc.h`, and a
  member has no name to the linker: `#define Counter STAYS(Counter)` at
  the top of a file makes `OFFSET(Counter)` the member's place.
- The part that stays is the segments that the top of
  `port/asm/VC.ASM` declares before `.MODEL`, so that the linker puts
  them first and in that order: `RHEAD`, what is assembly of it, behind
  the PSP; `RTEXT`, its compiled code; the four that the compiler makes
  for the data of `port/resident.c`, `RCONST`, `RCONST2`, `R_DATA` and
  `R_BSS`, the last of which is the stack; `RGAP`, the paragraph of the
  header of the next block of memory. The part that goes begins behind
  them, at `TrnBeg`, on a paragraph, and `tools/mkexe.c` stops the build
  if it does not. It begins with the constants of the C code, `CONST2`,
  some 8,900 bytes of them: QEMU is many times slower at a write to a
  page that has code which it has run, the stack ends just in front,
  and with `_TEXT` there, whose stubs of the main loop ran at every
  key, a scenario of the cursor's keys took 19.7 seconds where it takes
  5.6.
  Every offset of the assembly still counts from the PSP, and so does
  every one in the data of the part that stays, which is a group of its
  own (see below). Where the two parts begin and end is not known to
  the assembler, with compiled code between them: `TrnPar` is a word of
  the part that stays, `TrnPrW` of `port/resident.c`, which
  `tools/mkexe.c` fills in, read through SS, and it cannot be added to
  a constant where the program is assembled.
- A label of one segment less a label of another is no number to the
  assembler, also within one group: "Constant expected".
- The compiled code of the part that stays is `port/resident.c`, a file
  of `port_c_resident` in `CMakeLists.txt`: compiled into `RTEXT` with
  `RESIDENT` defined. DS is the PSP's segment in all of it, not the main
  data, and `md` means nothing there. Its data is its own, static data
  of C, which `tools/mkexe.c` lets that file have; a place in the PSP
  is a near pointer made of its offset. Nothing of the
  part that goes can be called from it, since that is not in the memory
  while a command runs: `port/dos.c` is compiled a second time into
  `RTEXT`, as `dos_r.obj`, and `port/vc.h` names the functions of that
  copy with `_r` where the first has `_c`, by a macro in their `#pragma
  aux`, which the compiler expands. `Int21e` there calls DOS with ES as
  the structure has it, for a block of memory in ES.
- The assembly of the part that stays calls a function of
  `port/resident.c` near: `EXTERN GetStat_r:NEAR` in the list at the
  top of `VC.ASM`, and `#pragma aux GetStat "*_r" __near` in the C.
  Both are in the one group, so the call needs no relocation.
- An `__interrupt` function of the compiler saves AX, CX, DX, BX, SP,
  BP, SI, DI, DS, ES and two words more, clears the direction flag and
  loads DS with its group, by a number that has to be relocated: the
  group is the assembly's, so in `RTEXT` it is the PSP. Its parameters
  are what lies on the stack: a structure of those twelve words, and
  behind it one of three words for each return address that an
  interrupt has left, `struct Saved` and `struct Frame` of
  `port/resident.c`. Writing to them changes the registers that come
  back and where the function returns.
- A relocation in the part that stays is applied by the code that
  starts the program, once it has the check sum of the file: `tools/
  mkexe.c` puts those behind the others in the table and counts them in
  `ResCnt`. One in the data of that part, or one that names a segment
  of the part that goes, stops the build.
- A function without the compiler's entry and exit is `__declspec(naked)`
  with its instructions in `_asm { }`. A jump there to an `__interrupt`
  function is a far one, with a relocation, whatever is written: such a
  function is far, and `near ptr` is "Operator is expected".
- A statement of C in a `__declspec(naked)` function is compiled to
  nothing, without a warning: such a function is its `_asm` and no more.
  `_asm` takes labels and jumps to them, and `jmp dword ptr cs:Vect21`
  is a far jump through a word of the code segment of the part that
  stays: `Vect21` is a variable declared `CODE`, which C code reads
  through CS as well, and no member of that part's data. `_asm` expands
  the macros of C: `mov sp,offset DataSav + LenStk`.
- Nothing that the compiler makes passes an interrupt on with the stack
  as it found it and no register changed. An ordinary function with the
  call in AX, `modify exact []`, and the handler before called through a
  pointer to a function type that is `aborts`, makes a frame, changes AL
  to compare, reads the pointer through DS or an ES that it loads, and
  jumps, `jmp dword ptr`, with BP still pushed. So INT 21h is told apart
  in assembly, in `New21`: the calls that the program does not deal
  with go on from there, before anything is pushed, with the flags of
  the original's comparisons. Only the others reach the interrupt
  function.
- An interrupt function that goes on in another handler enters it with
  the flags of `on`, and those are made what the assembly had there:
  `Left` of `port/resident.c`, with `Added`, `Taken` and `Doubled` for
  what `ADD`, `SUB` or `CMP`, and `SHL` leave of two numbers. A routine
  of that file that the assembly left with flags that go on to DOS
  returns them: `AddProg`.
- The `port` test knows code that ran by its bytes in the file, and a
  stretch with a relocated word in it does not count as run, but for a
  far call or jump: an `__interrupt` function begins with one. List in
  `functions` of `harness/ported.py` the naked entry in front of it,
  `New21`, not the function.
- The part that goes is called from the part that stays through a
  pointer to a far function that is kept in memory, `TrnAdr`, in
  `Stay` of `port/resident.c`: it comes back with every register
  changed but SS and SP, BP too, which the compiler takes to be kept
  whatever a `modify` list says. A pointer made on the stack, or a
  variable of `Stay`, would be reached through BP after the call. Read
  `Stay_r` and `Stay11_` in `build/port/resident.lst` after any change
  there: no instruction between the call and the next may name BP or a
  register.
- The program is entered at a C function, `Alloc0` of
  `port/resident.c`: the linker is told `option start=Alloc0`, and
  `tools/mkexe.c` takes the address from the linker's header. DOS enters
  it with DS on the PSP, which is what that file takes DS to be, and on
  the stack that the header names. The direction flag is as DOS leaves
  it: the first thing called is `IntOn`, which clears it. Until the
  relocations of the part that stays have been applied there, nothing
  may be called that needs one: an `__interrupt` or `__loadds` function,
  or a far function of another segment. A far call within `RTEXT` needs
  none, the linker makes `push cs` and a near call of it.
- The numbers that `tools/mkexe.c` fills in are seven words of
  `port/resident.c`, each of which it finds by its name in the linker's
  map, as it finds what they say: `TrnBeg`, `Stays`, `DataSav` and the
  segment `R_BSS`.
- The data of the part that stays is in a group of its own, `R_GROUP`,
  which the compiler makes for a file that is compiled with `-nd=R` and
  reads through DS without loading a segment. Its four segments are
  `RCONST` for string literals, `RCONST2` for what is `const`, `R_DATA`
  for what has a value and `R_BSS` for what has none, all of the class
  `FAR_DATA` and in that order. What has no value the compiler lays out
  in an order of its own; what has one, in the order it is written. So
  what needs an order is one structure, `Stays`, written last and with
  a value, zeros, and the stack is the one thing without: the end of
  the group, by the segments. A variable added to that file needs a
  value, `= 0`, or `tools/mkexe.c` stops the build for what is in the
  segment of the stack. An interrupt function and a `__loadds` one load
  DS with the group, a number that is relocated.
- The compiler's option `-g=NAME` puts the code segment of a file into
  a group of that name. With the name that `-nd` gives the group of the
  data, `-nd=R -g=R_GROUP`, the object file has two groups of one name
  and the linker makes one of them: code and data in one group, which
  begins with the code. Not used yet: see `TODO.md`, the last item of
  step 4.
- A layout at the number 0, `(*(struct MainDat __near *)0)`, is not the
  main data. Read and written through it, it is; but an address in it
  is a number to the compiler, and of a near pointer that is a number
  it makes a far one with the segment 0, not with DS, without a
  warning: `MvzLine(to, md.DosPth, ...)` would copy from the bottom of
  the memory. `__based(__segname("_DATA"))` in its place makes every
  address of a field far, "Pointer truncated" wherever one is kept in a
  near pointer. `md` is an `extern` object for that reason, and its
  place is the linker's to say. A `#pragma aux` can name it as another
  object, `#pragma aux md "Psp"`: the linker takes the offset of that
  in its own group.
- The linker's command can say the order too: `order clname RCODE clname
  DATA segment CONST segment CONST2 clname CCODE`, classes and the
  segments within one, whatever the order of the object files. Its
  help does not list the directive for DOS; it works.
- The linker places segments by their class, in the order in which it
  first meets each, and a group may have segments of other groups, or
  of none, between its own: a group is where its first segment is. A
  segment without contents counts for that. `R_GROUP` begins at the PSP
  because `port/asm/VC.ASM` declares an empty segment, `RPSP`, first of
  all, and says that it is of that group. The linker has no word of its
  own for a group or for a class: both come from object files.
- An `__interrupt` function of the part that goes has DS on the
  compiler's group when its first statement runs, and that group is the
  assembly's: its segment, where `SecDat` is, a variable of
  `port/main_t.c`. `FatErr` of `port/main.c`
  reads `SecDat` there as a near word and puts DS on the main data by
  `DataSeg`, a `#pragma aux` of one instruction, `mov ds,ax`, with
  nothing in its `modify` list: DS in a `parm` or a `modify` list is
  "Illegal register modified". Everything behind it is C as anywhere,
  `md` and the functions of other files.
- A function that does not return is `#pragma aux name aborts`; the
  compiler then ends the caller there.
- A far function of `port/resident.c` that the part that goes calls
  through `CallPSP`, with DS anywhere, is `__far __loadds`: `SetBrk`.
  Its first instructions load DS by a relocated number, so it cannot be
  listed in `functions` of `harness/ported.py`.
- Where the program has no stack, behind INT 2Eh, the instructions that
  set one are a function `__declspec(naked)` with `_asm`, which ends in
  a jump to a C function that does not return: `Int2E`. `offset
  DataSav + LenStk` there is the end of an array of the file, the
  stack. A parameter of a naked function that is only in
  its register wants `(void)name;`, or it is "not referenced".
- `_disable` and `_enable` are known only where a `#pragma intrinsic`
  names them, which is in `port/dos.c`: "No prototype found" elsewhere.
  Call `IntOff` and `IntOn`.
- A command that is C whole and ends at `Func04` is a stub of two
  lines: `CallC` and the jump. See `F65_00`.
- A label that gets a stub takes what the stub takes, and whatever
  jumps to the label has to bring that: a table too, not only `JMP` and
  `CALL`. Drive... of a panel's menu went by the bar's table to `F62_01`
  with the panel in DS and the menu in CL, as the original has it, after
  the stub there had come to take them in AX and DX: the box came up for
  a panel that was none, and no scenario chose the item. When a stub
  changes what a label takes, look for the label in every table of
  `port/asm/`, and have a scenario come in by each.
- A command that finds a register pushed for it and takes it off on
  every way out has a stub that takes it off: `PanC` and `PanEnd` in
  `VCMENU.INC`, for the commands of a panel's menu, which find the
  current panel on the stack. Where the assembly popped it to use it,
  the C needs to know what it was: there `WCBcrn`, since every way into
  a command has DS on the current panel. Look for what could change it
  between the push and the pop: `F20_02` makes another panel the current
  one, and `ReDir` may move a panel to another segment.
- A command may come to a place with DS on the main data where the
  place reads a panel: `M60_02` of the menus writes the prompt and tests
  `Visible`, and the configuration and EGA lines jump there after a
  `PUSH ES`, `POP DS`. The C takes the segment as a number, MAIN_SEG for
  those, and reads there what the assembly read.
- The BIOS called more than once with registers that go from one call
  to the next, as EGA lines calls it for the font and the mode, takes
  AX, BX, CX and DX from four words on the stack and puts them back:
  `Int10` of `port/dos.c`, with `struct BiosRegs` of `port/vc.h`, which
  was an entry, `Int10_a`, while the commands of the menus were
  translated. `Int10m` is another thing: it takes the mouse pointer off
  the screen around the call.
- Code that is entered by jumps at several labels, falls from one into
  the next, and leaves by jumps, never by a return, as the main loop
  does, is one function that takes the number of the label and returns
  the number of the place where the program goes on: `MainLoop` of
  `port/main.c`, a `switch` whose cases fall into one another, in a
  loop for what jumps back. Every label is a stub that pushes a word of
  room and all the registers, as a structure that the function gets in
  BX, and jumps to one piece of code that calls the function, puts the
  address of the place into the room, pops the registers and returns
  there: `MainC` and `MainGo` in `port/asm/VC.ASM`. So what goes round
  in registers from one command to the next is what the function makes
  of the structure.
- A macro stands before its first use. JWasm says "Syntax error" at a
  use that comes first, and names the macro.
- An entry for a routine that takes a panel in DS is written out, with
  the panel in a register: `ReDir_a` in `port/asm/VC.ASM`.
- A routine that only C code calls any more leaves nothing in the
  assembly but what it read there: `ErrRdy` left its box, `NoRd_P`,
  made `PUBLIC`, and the table of its keys went.
- The shared routines of `VCSUB1.INC` are C, and their entries for C
  are gone: `port/vc.h` declares the functions, under the names the
  entries had and with the same registers. A C file that still declares
  one with `"*_a"` does not link, "undefined symbol", and the cure is to
  take the declaration out. So are `SetPth`, `Fantom`, `ChgDisk` and
  `OpenFil` of `VCSUB3.INC`. `GetMous`, `InWind`, `DoublMo`, `TstBrk`,
  `TstDrv` and these four answer as their entries did, in numbers, 1
  where an entry had 0FFFFh for a flag; `Int10m` returns DX
  in the low word and CX in the high one; `OncZoom` always takes the
  include line; `InvVar` takes a far pointer, since the assembly hands
  it a byte of a panel too.
- A routine that found the main data by `CS:SecDat`, as `Intr21` did,
  has a stub that loads DS from there: its callers have DS anywhere.
  The stub of `Intr21` pushes the registers in the order of
  `struct DosRegs`, hands the carry flag it was called with to the C as
  a number, `SBB AX,AX` behind the pushes, which leave the flags, and
  pops what DOS returned, ES among it: the structure has a word for
  ES as DOS leaves it, the last, and the stub pushes ES first.
- Registers that a routine leaves as the BIOS left them, and that a
  caller goes on with, come back in a structure on the stack: the code
  that starts the program takes the mode in AL, the columns in AH and
  the page in BH after `ChkMode`. The stub pushes BX as well, and the C
  changes its high byte only.
- A function that needs nothing of the main data, as those that only
  call the BIOS, is called from its stub by `CALL FAR PTR` with DS as it
  is. `ScrOffs` and `ChkSum` read through the caller's DS, which is the
  main data wherever they are called, and are called so too.
- A stub of a routine that answers in two flags makes them with a jump:
  see `DoublMo`, which is CY for 2, NZ for 1 and ZR for 0.
- When two batches land one behind the other, the stubs of the second
  may have lost their callers to the first: after the rebase onto the
  main loop, `ClrKbd`, `Beep`, `TypMous`, `DoublMo`, `TstDrv`, `ResUser`
  and `SavScr` had none left in the assembly. Count the callers of
  every stub of the batch again then, in all of `port/asm/`, and take
  out the stub, its line in the `EXTERN` list and the function's name
  in `functions` of `harness/ported.py`.
- The places of `CmdNx_T` in `VCFUNC.INC`, by which a command of the
  keys says where the program goes on, have names in two files: those
  that `keys.c` returns in an `enum` of its own, and those of `enter.c`
  and `view.c` as `NEXT_` in `port/vc.h`. A name in `port/vc.h` is seen
  by every C file, so it must not be that of a routine: `keys.c` calls
  three places `M01_01`, `M03_00` and `M06_00`, and a function of
  those names could not be declared next to them. A new place to go on
  at is a new entry at the end of the table and a new `NEXT_`.
- A command that does not come back, as Enter does not when it runs
  something, still returns to its stub, and the stub jumps: `CmdExe`
  and `CmdIni`, to the part of the program that stays in the memory
  and to `Init11`. Neither sets the stack pointer, so an entry called
  from inside the C would leave the frames of the C on the stack, and
  more of them with every command run.
- A label in the middle of a command may be where the start of the
  program comes in: `Init45`, of the main loop, goes to `F21_31`, in
  Enter, to save the setup it has read where a command leaves it, and
  start over from there. Before taking code out, look for every label of it
  in all of `port/asm/`, not only for those that look like entries;
  the assembler finds the rest, "Symbol not defined".
- C code goes on after the main data has been given back to DOS, as
  the assembly did: `F21_31` frees it and then reads the copy in the
  part that stays, through far pointers made with `ResSeg`. DS stays
  where it was, and nothing has taken the memory yet.
- A register may reach the BIOS with what it was last used for: Enter
  has the BIOS write the end of a line with BX on the saved setup,
  where the BIOS takes a page and a colour. The C hands it the address
  that the original has there, from the listing of
  `build/shareware-layout/`, as for `DelTr0`.
- The stub of code that stands in the data can stay there: `FncSel`.
- The start of the program runs before there is main data, and is C
  all the same: `Init00_c` and `Init11_c` of `port/start.c` get the
  segment of the part that stays in the memory as a number, SS in their
  stubs, and read and write through far pointers only. A function that
  touches nothing of `md` can be entered with DS anywhere, and so can
  what it calls if that reads none either: `ChkMode`, `Cursor`, `Int10`,
  `Int21d`, `CutPath`, `MvzLine` and `AddFile` do not, `CaseMap`,
  `PathLin`, `ScrOffs` and `HookVct` do. `Init11_c` returns the segment
  of the main data that it has asked DOS for, its stub puts DS there,
  and the rest is a third function, `IniData_c`.
- A routine of the part that stays is called from C through a pointer
  to a far function, made of that part's segment and the routine's
  offset, with a `#pragma aux` on the type of the function that says
  which registers it changes: `StaysFn` in `port/start.c`, for `SetBrk`.
  It asks nothing of the assembly but `PUBLIC`.
- A routine may know a thing by its offset alone. `MainPth` took the
  text of `NcPath` where it was to write at the offset of `TempDir`,
  whatever the segment, and its first call writes into another segment,
  at an offset 12h short of that in the original and 0Bh short in the
  port. The C has an argument for it. Where the assembly compares an
  address, look at every call for the segment it is made with.
- A copy that runs on behind its table takes what follows the table in
  the assembly. The defaults of a panel are 88 bytes of which the
  original has 8, and the rest is its table of the keys, with the
  addresses of the commands. Before calling such a thing a bug that
  shows, find out where the bytes get to and what reads them: these lie
  behind the end of two texts, in the memory, until `MkIni1` clears
  them before the first key, and the setup file is made without them.
  So the C has the 8 bytes as a table and zeros for the rest, now that
  the table of the keys is a `switch` and has no bytes to take.
- A text of the assembly may hold bytes that are no characters, and a
  tool that shows the file as text leaves them out: the table of stars
  is five bytes between two quotes, two letters and 07h, 04h and 0Fh,
  and read as text it is two, with three bytes of the next table
  seemingly taken along. Take the bytes of a table that goes to C from
  the listing, `build/port/VC.LST`, or hold the C against the assembly
  by a script before the table leaves it. The `port` test passed with
  the three bytes wrong: no screen shows the stars of the blanker. What
  showed it was the setup file that a program started on the defaults
  saves, held against the original's; `start-setup` ends with that
  file on its disk for this reason.
- When the last callers of a stub in another file become C, the stub
  goes there too: the start-up was the only caller left of `ChkMode`,
  `Cursor`, `ScrOffs`, `AddFile`, `MvzLin0` with `MvzLine`, `CutPath`
  and `PathLin` in `VCSUB1.INC` and of `HookVct`. `tools/routines.py`
  run on the tree before and after shows them, in the lines that go from
  some callers to none; look for each name in all of `port/asm/` before
  taking it out.

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
- A constant array of two dimensions in the code segment is not read
  through CS: the compiler takes the address of a row for a pointer into
  the main data, without a warning, and reads there. Give such a table
  one dimension and add the row's place to the index: `MnuHlp` of
  `port/menu.c`. In the listing every access to a constant has `cs:`.
- Texts that follow one another in one array can be written as a macro
  of string literals, each ended by `\0`, with octal escapes for bytes
  that are no characters: `sizeof` of the macro, less one, is where the
  next one begins, and costs nothing in the program. See `MenuLn` there.
- The option `-nd` makes the compiler load a segment for every access to
  the main data. Do not use it for a file of the part that goes.
- The port's stack is 4,096 bytes where the original's is 256, so stack
  use needs no care. The `port` test reports how much each scenario
  leaves and fails when less than half is left.
- A static function is called, not expanded in place, also where only
  one routine uses it.
- The compiled code has two segments: CTEXT, and CTEXT2 for the files
  that `port_c_second` of `CMakeLists.txt` names, `keys.c` the first. A
  file that calls `__U4M` or `__U4D` stays in CTEXT, where those are.
  A segment may end at F000h at the most, its offset in the linker's map
  and its size together, not at 64K: `tools/mkexe.c` reads the map and
  stops the build with a message when one is fuller. Name a file in
  `port_c_second` then.
  The limit is QEMU's, not the program's. The linker makes the far call
  between two files of one segment into `push cs` and a near call, and
  between the end of a segment and its beginning a near call goes the
  short way, around the end of the 64K. A processor does that. QEMU, as
  of 11.1 and of its source in October 2026, leaves the wrapping out
  when the call and where it would go without wrapping are in one 4K
  page of memory, and goes on there, 64K off, in whatever lies next to
  the segment. With CTEXT at FC7Ch `Run_00_c` at F6B8h called `DosCom_c`
  at 0471h so, and `command-edit` ended in "Invalid Opcode" after a
  command of an extension file; in a trial with CTEXT at FC19h the same
  call went astray, happened to come back, and the scenario passed. It
  takes one end of the call at F010h or beyond, wherever DOS loads the
  program.
  A program that fails where no translation explains it, and runs right
  when QEMU is also given `-d nochain`, has met this or something like
  it: that keeps QEMU from the short cut that is wrong. All scenarios
  take a third longer with it.
- A function with a `#pragma aux` that calls another of the same file
  does so by `push cs` and a near call, and may end by jumping into the
  end of another: `F01_05_c` goes on in `F00_05_c`. With `modify exact
  [ax dx]` it saves ES as well, also for the static functions it calls,
  which do not.
- Dividing a 16-bit number by another, or taking the remainder, needs no
  helper routine.
- The structures of `port/vc.h` are the assembly's, but a field that the
  assembly declares with `0 DUP`, a name for a place, is not in them:
  `TreList` is where `TreSum2` is, two bytes before the end of `DIRTREE`.
  Read the `STRUC` in `port/asm/VC.ASM` before taking an offset from a
  C structure.
- There is no C library. String literals and static variables are refused
  by `tools/mkexe.c`, but for the data of `port/resident.c`, which has
  DS on it, and for a variable with a value in a file that has no code:
  `SecDat` of `port/main_t.c`, the one. A text may be written as a
  string where it gives an array declared `CODE` its contents: `static const u8 CODE Name_S[] =
  "text";` is in the code segment, with its zero byte. Constants that
  are named by their offsets, boxes and dialogs with their texts, are in
  files of their own, without `CODE`: see "Moving the data of a file to
  C".
- For 32-bit numbers the compiler calls helper routines, near, in its own
  code segment. `__U4M`, which multiplies, and `__U4D`, which divides
  unsigned numbers, are functions of `port/arith.c`, written without
  what they are there for; of `__U4D` the scenarios run only a division
  by 16 bits. They are held against Python's arithmetic by the
  test `arith`, `harness/arithcheck.py`, which runs their bytes on a
  model of the processor, and with `--pairs 200000` as many numbers as
  they were written against: do that after a change there or to the
  compiler. A helper
  that is missing shows as an undefined symbol when the port is linked:
  signed division would be `__I4D`. Shifting a 32-bit number by a
  constant needs none. `/` and `%` of the same numbers are two calls:
  take the remainder by multiplying back where it is known to fit 16
  bits.
- A function may have a name that C cannot give it, `#pragma aux U4M
  "__U4M"`, and be near in a segment of far ones, `__near` there. A
  number of 64 bits comes back in four registers, AX, BX, CX and DX from
  its highest word down, whichever order `value` has them in: that is
  how `U4D` returns the quotient in DX:AX and the remainder in CX:BX,
  as a union of such a number and its four words. Assigning the words
  needs no helper; arithmetic on 64 bits would.
- `REP MOVSB` from one segment to another is a loop over two far
  pointers: a far pointer steps its offset only, as SI and DI do, down as
  well as up. Where the assembly divides a 32-bit count into pieces for
  that, comparing and subtracting in the loop needs no helper routine.
- DX of a call of DOS that is no pointer, a drive or a date, is written
  into the offset of `name` in `struct DosRegs`, which leaves DS as it
  was set: `*(u16 STACK *)&regs.name`, as `DX_OF` in `port/dirfunc.c`.
- `INC AX` and `CBW` on what DOS left in AL is
  `(u16)(signed char)(u8)(value + 1)`.
- A text without its zero byte is an array of its length:
  `static const u8 CODE Hist_S[9] = " History ";`.
- `far` and `near` are words of the compiler: a variable of that name
  is refused with "Invalid declarator", and every use of it with a
  dozen errors more. What `MvsBw` and `MvsColr` return is a far pointer;
  kept in a near one it is "Pointer truncated", which stops the build.
- Warnings stop the build, and a parameter that is not read is one. A
  function gets of the registers that its stub has only those it reads:
  `parm [cx]` alone where the panel in AX is of no use to it.
- A far call from one of the two code segments to the other costs a
  relocation, four bytes of the file, where one within a segment costs
  none: `port/commands.c`, 2,479 bytes of code in CTEXT2 that calls the
  dialogs and the bars in CTEXT, made the program 2,848 bytes longer.
- The scratchpad directory of a session is one for all the agents that
  run side by side: a file there may be another agent's by the same
  name. Keep temporary files in the checkout's `build/`.
- A far pointer whose segment is the constant 0 is compiled as a near
  one, without a warning: the table of interrupt vectors was read from
  the main data. Take the segment from a `volatile` variable, as
  `F65_31` of `port/memory.c` does.
- CTEXT has under a thousand bytes left below its limit. A new file
  goes into CTEXT2, by its name in `port_c_second`, unless it multiplies
  or divides numbers of 32 bits.
- An address that the main data keeps as two words, offset and segment,
  `ErrVct`, is a `u32` there and a far pointer by a cast, which costs
  nothing: `(const u8 __far *)md.ErrVct`.
- DOS, the BIOS and the mouse driver are called in `port/dos.c` and
  nowhere else, and that file alone has inline assembly, but for the
  entries of the interrupts and INT 2Eh in `port/resident.c` and one
  instruction that loads DS in `port/main.c`: `Int21d` and
  `Int21f` for DOS, `Int10` and `TopView` for the screen, `Int16` for
  the keyboard, `Int2F`, `Int33` and `MousAt` for the mouse, and `Int28`
  and `DvPause`, by which the program says that it is idle, all
  declared in `port/vc.h`. Each is a line of C around a `#pragma aux`
  that holds the `INT` and as little else as will do: the registers are
  its `parm` and its `value`, and the compiler loads them. C code that
  needs a call of DOS fills a `struct DosRegs` and calls `Intr21` of
  `port/system.c`, which looks for DOS's critical error, or `Int21d`,
  where the original called DOS itself; an address that DOS answers
  with in ES:BX is in `es` and `bx` there afterwards. `Int21d` calls
  DOS with ES where DS is, on the main data; a call that takes a
  block of the memory in ES which is not the main data goes through
  `Int21e`, which takes ES from the structure too. A call of the
  BIOS for the screen fills a `struct BiosRegs` for `Int10`, and the
  registers stay in it from one call to the next, as they did in the
  assembly. `TopScr` asks where the buffer is that TopView shows for
  the screen, an address in ES:DI both ways. A call that these do
  not cover gets a function there, with a comment that says what it is
  for and which services go through it; nothing is added that hides
  DOS, as long as the translation goes on. `CaseMap`, `InPort`, `IntOff`
  and `IntOn` are there too: DOS's routine for capitals, a port, and the
  interrupt flag, for which the compiler has intrinsic functions. What
  is in the memory, the BIOS's data and the screen, is read and written
  through far pointers in plain C: `Timer` of `port/system.c`, `Window`
  of `port/window.c`. No entry of the assembly calls an interrupt for C
  code any more: a call of DOS that an entry made is a few lines around
  `Int21d` in the C file that wants it, as `Dos52` of `port/memory.c`.
- Calls of an interrupt between which no compiled code may run are one
  piece of inline assembly: DESQview is asked for a stack of its own,
  for a pause and for the program's stack back, and the compiler may use
  the stack between any two calls of C. `DvPause` of `port/dos.c`.
- A number of 16 bits times another, as a number of 32, is a call of
  `__U4M`, which a file of the second segment cannot reach. Where one
  factor is below 256 the product can be had in halves of 16 bits:
  `Below` of `port/blank.c`.
- A table for `Case` that has keys with their scan codes besides
  characters that count with any, as the main loop's had, is a `switch`
  on the key and then one on the character, if every character of the
  second kind stood behind the keys that have it too: Tab before ^I.
  `Func12` of `port/main.c`.
- A stub is taken out when nothing of the assembly names it, and what
  says so is the assembler, not `tools/routines.py`, which counts calls
  and misses a jump to a routine and a routine named in a table. With
  the stub go its line in the `EXTERN` list and its function in
  `harness/ported.py`.
- A number of 32 bits in a pair of registers has its halves by the
  registers, not by the order they are written in: `[dx cx]` has DX for
  the low word and CX for the high one, `[dx ax]` and `[bx ax]` have AX
  for the low one. Read the listing.
- Inline assembly is a `#pragma aux` with the instructions in quotes
  and `parm`, `value` and `modify exact` behind them, for a function
  declared `static`: it is put in place where it is called. `bp` in its
  `modify` list is accepted and not saved: where the BIOS may lose BP,
  the instructions push and pop it. Flags come out by `pushf` and a
  `pop` into the register of the value, as in `Int16`; a structure on
  the stack is written by `mov ss:[di],bx`, as in `MousAt` and `Int10`.
- `inp`, `_disable` and `_enable` are the compiler's own once a
  `#pragma intrinsic` names them, without a header: `IN`, `CLI`, and
  `STI` with a `CLD` behind it.
- A far routine that is no function of the program, DOS's for capitals,
  is called through a pointer to a function type that has a `#pragma
  aux` of its own: `CaseMapFn` in `port/dos.c`.
- A far pointer to a fixed place of the machine's memory is a constant
  with its segment: `(const u32 __far *)0x0040006CUL` is the BIOS's
  count of timer ticks, and costs no relocation.

## A run of batches

Batches are translated without waiting for the user between them, and
several at a time.

- Each batch is the work of an agent of its own, started fresh, so that a
  run does not fill one context. The session that runs them does not
  translate: it starts each agent, checks what the agent left, which is
  the commit, the push and `LastTest.log`, and keeps its report. An
  improvement to the tools is handed to an agent in the same way.
- An agent is given no time limit for work that is going well. The
  session watches what its agents are doing, and turns one round that is
  spending its time on something of little use.
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
- Nothing in the harness takes turns: recordings of the original and
  tests of the port run at the same time, in as many checkouts as there
  are, and other machines only make them slower. The original and the
  port are driven from inside their machine, by a driver that acts when
  the program waits (`harness/DRIVER.ASM`), not by the clock. Where a
  recording or a test
  does come out different under load, that is a flaw to report, not
  something to wait out.
- A routine of the assembly that C code calls has one entry for C,
  named after it with `_a`, that takes and hands back the registers as
  the routine has them, and one declaration in `port/vc.h`. A batch that
  wants its arguments otherwise wraps the entry in its own C file. Look
  for an entry before making one, and again after a rebase: two batches
  have made the same entry with different arguments, and git saw no
  conflict in it.
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
  its machine. The machine's clock is counted in the instructions it
  executes, and the port takes more of them than the original to get to
  the same step: up to a sixth more. The first screen is at about ten
  seconds past the minute, so such a scenario is done showing the time
  by about fifty seconds past in the original.
- A figure on the screen that follows the time of day or the size of the
  program, such as the clock or a count of free memory, is left out of
  the comparison, in `harness/deviations.py`. The exception is a scenario
  that is made to test that figure.
- The countries tested are the United States and Russia.
- Russian text is typed on the Russian keyboard of the Russian bench. Its
  Cyrillic letters are selected in a step that types text into an input
  line, never at the panels: there Alt by itself brings up the menu.
- The tests run the build of the port that is shipped; no build is made
  for them. The original and the port are driven alike, by a program of
  ours that stays resident in the machine, `harness/DRIVER.ASM`: it puts
  the keys of a step in through the keyboard controller, so that the BIOS
  makes of them what it makes of a keyboard's, and writes the screen out
  when the program idles with none left. It watches and feeds, and
  answers nothing in the program's place or the BIOS's.
- A test does not put a stand-in where something is that the project does
  not own: DOS, the BIOS, the machine. What a test knows of them it gets
  from the real ones. A part of the program itself may be replaced in a
  test build, since its source says what it is supposed to do.
